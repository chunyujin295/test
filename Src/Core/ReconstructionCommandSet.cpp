
#include "Core/ReconstructionCommandSet.h"
#include <Core/Logging.h>
#include "Core/ReconPerfLog.h"
#include "Core/Timer.h"
#include "Core/File.h"
#include "Core/Tiling.h"
#include "Core/TaskCommandSet.h"
#include "Core/Types.h"
#include "Util/TaskProcess.h"
#include "Core/KML.h"        
#include "Core/VectorFile.h"
#include <utility>
#ifdef USE_AI3D_PROJ
#include "Core/Proj/CoordinateReferenceSystem.h"
#endif

namespace AI3D
{
    namespace CORE
    {
        namespace
        {
            // SubmitReconstruction: memory is source of truth. Caller flushes with ExportBlockATData
            // so disk matches memory before reconstruct. Only read disk when a field is missing
            // in memory — never force ReloadCurrentATFromPersistedFilesForExportOrReconstruction.
            bool EnsureATReadyForReconstruction(BlockObject* block)
            {
                if (block == nullptr || block->GetCurrentAT() == nullptr)
                {
                    return false;
                }

                std::shared_ptr<ATData> at = block->GetCurrentATMutual();
                const bool has_registered_images =
                    at->HasRegImages() || !at->GetRegImageIds().empty();
                const bool tiepoints_in_memory =
                    block->GetTiepointStatus() && at->HasTiepoints();

                if (has_registered_images && tiepoints_in_memory)
                {
                    ReconPerfLog("[ReconPerf] SubmitReconstruction | EnsureATReady | path=in_memory (no disk read)");
                    return true;
                }

                if (!block->GetTiepointFullStatus())
                {
                    LOGE("SubmitReconstruction: block has no tiepoints on disk");
                    return false;
                }

                if (has_registered_images)
                {
                    ReconPerfStage perf_load_tp("SubmitReconstruction", "EnsureATReady_LoadTiepoints");
                    block->LoadTiepoints();
                    return block->GetCurrentAT() != nullptr && block->GetCurrentAT()->HasTiepoints();
                }

                ReconPerfStage perf_load_block("SubmitReconstruction", "EnsureATReady_LoadBlockATData");
                BlockObject::BlockImportOptions opts;
                opts.load_images_ = true;
                opts.load_tiepoint_ = true;
                opts.force_reload_tiepoints_from_disk_ = false;
                opts.suppress_update_complete_at_file_on_reload_ = true;
                if (!block->LoadBlockATData(at, opts))
                {
                    return false;
                }
                if (at->HasTiepoints())
                {
                    block->SetTiepointStatus(true);
                }
                return at->HasRegImages() && at->HasTiepoints();
            }
        }

        ReconstructionCommandSet::ReconstructionCommandSet()
        {

        };

        int ReconstructionCommandSet::ExportReconstructionViewBin(
            BlockObject* block, ReconstructionObject* reconstruction)
        {
            if (block == nullptr || reconstruction == nullptr)
            {
                return AI3D_FAILURE;
            }

            ReconPerfStage perf_total("ExportReconstructionViewBin", "total");

            std::string srsdef = reconstruction->GetATData().GetLocalSrs();
            srs_s srs = CoordinateDescriptor::GetSRSFromDefinition(srsdef);

            std::string outpath = File::EnsureTrailingSlash(
                File::EnsureUnifySlash(std::string(reconstruction->GetPath())));
            std::string srsFile = SRS_USE_BIN ? SRSBIN : SRSJSON;
            std::string localsrsfile = outpath + srsFile;

            {
                ReconPerfStage perf_srs("ExportReconstructionViewBin", "WriteLocalSrs");
                if (SRS_USE_BIN) {
                    block->GetTaskInfoMutual().WriteLocalBin(srs, localsrsfile);
                }
                else {
                    block->GetTaskInfoMutual().WriteLocalJson(srs, localsrsfile);
                }
            }

            BlockObject blocktemp(outpath);
            blocktemp.SetId(block->GetId());

            {
                ReconPerfStage perf_make_block("ExportReconstructionViewBin", "MakeBlockFromATData");
                blocktemp.MakeBlockFromATData(reconstruction->GetATData());
            }

            BlockObject::BlockExportOptions opt;
            opt.export_tiepoint_ = true;
            opt.export_not_registered_ = true;
            opt.export_controlpoint_ = true;
            {
                ReconPerfStage perf_export_bin("ExportReconstructionViewBin", "ExportATBinary");
                if (!blocktemp.ExportATBinary(outpath + PRODUCTIONVIEWIDSBIN))
                {
                    LOGE("ExportReconstructionViewBin: ExportATBinary failed");
                    return AI3D_FAILURE;
                }
            }

            ReconPerfLog(String::StringPrintf(
                "[ReconPerf] ExportReconstructionViewBin | done | path=%s",
                (outpath + PRODUCTIONVIEWIDSBIN).c_str()));
            return AI3D_SUCCESS;
        }

        int ReconstructionCommandSet::SubmitReconstruction(BlockObject* block, reconstruction_t& rid, const struct processing_settings_s& options)
        {
            ReconPerfStage perf_total("SubmitReconstruction", "total");
            if (block->GetCurrentAT() == nullptr)
            {
                return AI3D_FAILURE;
            }

            bool tiepointstatus = block->GetTiepointFullStatus();;

            if (block->GetStatus() != job_status_e::STATUS_COMPLETE || !tiepointstatus)
            {
                return AI3D_FAILURE;
            }

            {
                const std::shared_ptr<ATData> at_stats = block->GetCurrentATMutual();
                ReconPerfLog(String::StringPrintf(
                    "[ReconPerf] SubmitReconstruction | dataset | block=%s reg_images=%zu images=%zu tiepoints=%zu",
                    block->GetIdString().c_str(),
                    at_stats ? at_stats->GetRegImageIds().size() : 0u,
                    at_stats ? static_cast<size_t>(at_stats->GetNumImages()) : 0u,
                    at_stats ? at_stats->GetPoint3DIds().size() : 0u));
            }

            {
                ReconPerfStage perf_ready("SubmitReconstruction", "EnsureATReady");
                if (!EnsureATReadyForReconstruction(block)) {
                    LOGE("SubmitReconstruction: EnsureATReadyForReconstruction failed");
                    return AI3D_FAILURE;
                }
            }

            // Flush current in-memory AT to block folder so disk matches memory before reconstruct.
            {
                std::shared_ptr<ATData> at_flush = block->GetCurrentATMutual();
                if (at_flush && at_flush->HasConstraints()) {
                    ReconPerfStage perf_constraint("SubmitReconstruction", "SaveConstraint");
                    const std::string constraint_path =
                        File::EnsureUnifySlash(block->GetPath() + PATH_SEPARATOR_STR + CONSTRAINTFILE);
                    if (!at_flush->SaveConstraint(constraint_path)) {
                        LOGE("SubmitReconstruction: SaveConstraint failed");
                        return AI3D_FAILURE;
                    }
                }

                {
                    ReconPerfStage perf_export_block("SubmitReconstruction", "ExportBlockATData");
                    if (!block->ExportBlockATData()) {
                        LOGE("SubmitReconstruction: ExportBlockATData failed — could not sync AT to disk");
                        return AI3D_FAILURE;
                    }
                }
                if (block->GetTiepointStatus()) {
                    block->GetTaskInfoMutual().statisticinfo_.tiepointnum =
                        static_cast<int>(block->GetCurrentAT()->GetPoint3DIds().size());
                }
            }

            rid = -1;

            // Single working copy: block AT stays unchanged; reconstruction mutates its own ATData.
            ATData atdata_reconst;
            {
                ReconPerfStage perf_copy("SubmitReconstruction", "CopyBlockAT");
                atdata_reconst = *block->GetCurrentAT();
            }
           
           
           
            if(0)
            {
                
                
                

                

                std::string camparampath = block->GetPath() + "/cam.bin";
                File::EnsureUnifySlash(camparampath);
                // camparampath = GBK2UTF8(camparampath);
                block->SaveCamBin(camparampath);


                std::string undistortpath = block->GetPath() + "/"+ UNDISTORTPATH +"/";
                File::EnsureUnifySlash(undistortpath);
                // undistortpath = GBK2UTF8(undistortpath);
                atdata_reconst.UnditortData(undistortpath);
            }

            

            srs_s atlocalenu;
            {
                ReconPerfStage perf_enu("SubmitReconstruction", "GetDefaultEnuSRS");
                atlocalenu = atdata_reconst.GetDefaultEnuSRS();
            }

#ifdef USE_AI3D_PROJ
            {
                ReconPerfStage perf_crs("SubmitReconstruction", "AddCrs");
                srs_s newsrs = AI3D::PROJ::CoordinateReferenceSystem::AddCrs(atlocalenu.definition);
                (void)newsrs;
            }
#endif
            {
                ReconPerfStage perf_transform("SubmitReconstruction", "TransFormATData");
                atdata_reconst.TransFormATData(atlocalenu.definition);
            }
            {
                ReconPerfStage perf_depths("SubmitReconstruction", "ComputeDepths");
                atdata_reconst.ComputeDepths();
            }

            std::string cbfile = block->GetPath() + "/" + COLORBIN;
            cbfile = File::EnsureUnifySlash(cbfile);

            if (!File::ExistsFile(cbfile) && !atdata_reconst.ShouldCB())
            {
                ReconPerfStage perf_cb("SubmitReconstruction", "SaveCBBin");
                atdata_reconst.SaveCBBin(cbfile);
            }

            ReconstructionObject* reconstruction = nullptr;
            {
                ReconPerfStage perf_new_recon("SubmitReconstruction", "NewReconstructionObject");
                reconstruction = new ReconstructionObject(std::move(atdata_reconst), block->GetId());
            }

            {
                ReconPerfStage perf_add("SubmitReconstruction", "AddReconstruction");
                block->AddReconstruction(reconstruction);
            }

            {
                ReconPerfStage perf_files("SubmitReconstruction", "CreateReconDirAndCopyCB");
                File::CreateDirIfNotExists(reconstruction->GetPath(), true);
                if (File::ExistsFile(cbfile))
                {
                    std::vector<std::string> files(1, cbfile);
                    File::CopyFiles(files, reconstruction->GetPath(), false);
                }
            }

            // RB.bin + reconstruction SRS: deferred to first SubmitProduction (ExportReconstructionViewBin).

            rid = reconstruction->GetId();

            {
                ReconPerfStage perf_taskinfo("SubmitReconstruction", "UpdateBlockTaskInfo");
                auto& taskinfo = block->GetTaskInfoMutual();
                blk_recontruction_info_s info;
                reconstruction->ToTaskInfo(info);
                taskinfo.reconstructions_info_.push_back(info);
                taskinfo.isSaved = false;
            }

            ReconPerfLog(String::StringPrintf("[ReconPerf] SubmitReconstruction | done | reconstruction_id=%d", rid));
            return AI3D_SUCCESS;
        }

    



        
        int ReconstructionCommandSet::ResetSpatialFrameworkSRS(BlockObject* block, reconstruction_t reconstruction_id, const srs_s& srs)
        {
           
           
           
           

           
           
           
           
           
           
           
           
           
           

           
           
           
           

           
           
           
           
           
           
           
           
           
           
           
           
           
           
           
           
           
           
           


           

           
           
           
           
           
           
           
           
           


           

           
           
           
           
           
           
           
           
           
           
           
           

           
           
           
           
           


            return AI3D_SUCCESS;
        }
        int ReconstructionCommandSet::Get2DPolygon(const std::vector<Eigen::Vector3d>& polygon3D, std::vector<Eigen::Vector2d>& polygon2D)
        {

            polygon2D.resize(polygon3D.size());
            for (int i = 0; i < polygon3D.size(); i++)
            {
                polygon2D[i] = Eigen::Vector2d(polygon3D[i].x(), polygon3D[i].y());
            }
            return AI3D_SUCCESS;
        }

        
        int ReconstructionCommandSet::ResetBoundingBox(BlockObject* block, reconstruction_t reconstruction_id, const ABBox3d& box)
        {
            if (!block->GetReconstructionsMutual().count(reconstruction_id))
            {
                return AI3D_FAILURE;
            }
            ReconstructionObject* reconstruction = block->GetReconstructionsMutual().at(reconstruction_id);
            ABBox3d boxtemp = box;
            MakeBoundingBoxValid(boxtemp);
            

            reconstruction->GetBoundingBoxCustomMutual() = boxtemp;
         
            reconstruction->RunTiling();
           
           
                UpdateBlockInfo(block, reconstruction_id);
            return AI3D_SUCCESS;
        }

        int ReconstructionCommandSet::ResetBoundingBox(ReconstructionObject* reconstruction, const ABBox3d& box)
        {
            ABBox3d boxtemp = box;
            MakeBoundingBoxValid(boxtemp);
            
            reconstruction->GetBoundingBoxCustomMutual() = boxtemp;
            
            reconstruction->RunTiling();
           
            return AI3D_SUCCESS;
        }

        

        
        int ReconstructionCommandSet::ResetBoundaryByFile(BlockObject* block, reconstruction_t reconstruction_id, std::string file, std::string& msg)
        {
            if (!block->GetReconstructionsMutual().count(reconstruction_id))
            {
                return AI3D_FAILURE;
            }

            ReconstructionObject* reconstruction = block->GetReconstructionsMutual().at(reconstruction_id);
            
            std::vector<std::vector<Eigen::Vector3d> > points = ReadPolygonsFromVecFile(file);;
            
           
           
            auto  currentcustomsrs = reconstruction->GetCustomSrs();

            if (currentcustomsrs.type == LOCAL)
            {
                msg = "local srs is not support.";
                LOGI("local srs is not support.");
                return AI3D_FAILURE;
            }
            auto  currentbasesrs = reconstruction->GetBaseSrs();




            CoordinateTransformer::TransformPoints(points, GEO84SRS, currentcustomsrs.definition);

            auto  currentcustombox = reconstruction->GetBoundingBoxCustom();
            auto& boundary_custom = reconstruction->GetBoundaryCustomMutual();


            ABBox3d newbox;
            std::vector<std::vector<Eigen::Vector2d> > boundaries(points.size());
            for (int i = 0; i < points.size(); i++)
            {
                std::vector<Eigen::Vector2d> boundary(points[i].size());
                for (int j = 0; j < points[i].size(); j++)
                {

                    newbox.extend(points[i][j]);
                    boundary[j] = (Eigen::Vector2d{ points[i][j].x(),points[i][j].y() });
                }
                boundaries[i] = boundary;
            }
            newbox.min().z() = currentcustombox.min().z();
            newbox.max().z() = currentcustombox.max().z();
            
            bool insected = newbox.intersects(currentcustombox);
            if (!insected)
            {
                msg = "Warning:Invalid spatial framework,please adjust parameters.";
                return SPARTIAL_NOT_MATCH;
            }

            boundary_custom = boundaries;

            

             
             
            ResetBoundingBox(block, reconstruction_id, newbox);
            return AI3D_SUCCESS;
        }

       

        int ReconstructionCommandSet::ResetTilingMode(BlockObject* block, reconstruction_t reconstruction_id, tiling_mode_e mode)
        {
            if (!block->GetReconstructionsMutual().count(reconstruction_id))
            {
                std::cout << "canont count reconstruction_id" << std::endl;
                return AI3D_FAILURE;
            }
            ReconstructionObject* reconstruction = block->GetReconstructionsMutual().at(reconstruction_id);

            Tiling* tiling_raw = reconstruction->GetTilingDiscriptorMutual();
            tiling_param_s& param = reconstruction->GetTilingDiscriptorMutual()->GetParamsMutual();


            param.mode_ = mode;

            Tiling* tiling = TilingGenaratorFactory(param);
            reconstruction->SetTilingDisriptor(tiling);
            reconstruction->RunTiling();
           
            UpdateBlockInfo(block, reconstruction_id);


            return AI3D_SUCCESS;
        }

        int ReconstructionCommandSet::ResetTileSize(BlockObject* block, reconstruction_t reconstruction_id, float value)
        {
            if (!block->GetReconstructionsMutual().count(reconstruction_id))
            {
                return AI3D_FAILURE;
            }

            ReconstructionObject* reconstruction = block->GetReconstructionsMutual().at(reconstruction_id);
            Tiling* tiling_raw = reconstruction->GetTilingDiscriptorMutual();
            tiling_param_s& param = tiling_raw->GetParamsMutual();
            param.regular_params_.tilesize_ = value;
          
            clock_t t1, t2, t3;
            t1 = clock();
           
            
            reconstruction->RunTiling();
            t2 = clock();
            t3 = t2 - t1;
            t3 *= 0.001;
            std::cout << " tiling  finished. " << t3 << std::endl;
            
            t1 = clock();
            UpdateBlockInfo(block, reconstruction_id);
            t2 = clock();
            t3 = t2 - t1;
            t3 *= 0.001;
            std::cout << " tiling update finished. " << t3 << std::endl;

            return AI3D_SUCCESS;
        }

        int ReconstructionCommandSet::ResetTileMAXRamUsage(BlockObject* block, reconstruction_t reconstruction_id, float value)
        {
            if (!block->GetReconstructionsMutual().count(reconstruction_id))
            {
                return AI3D_FAILURE;
            }

            ReconstructionObject* reconstruction = block->GetReconstructionsMutual().at(reconstruction_id);
            Tiling* tiling_raw = reconstruction->GetTilingDiscriptorMutual();
            tiling_param_s& param = tiling_raw->GetParamsMutual();
            param.adaptive_params_.target_ram_used_ = value;

            
            
            reconstruction->RunTiling();
          
            UpdateBlockInfo(block, reconstruction_id);
            return AI3D_SUCCESS;
        }

        tiling_mode_e ReconstructionCommandSet::GetTilingMode(BlockObject* block, reconstruction_t reconstruction_id)
        {
            if (!block)
                return TILE_NONE;

            if (!block->GetReconstructionsMutual().count(reconstruction_id))
            {
                return TILE_NONE;
            }

            ReconstructionObject* reconstruction = block->GetReconstructionsMutual().at(reconstruction_id);
            Tiling* tiling_raw = reconstruction->GetTilingDiscriptorMutual();
            tiling_param_s& param = reconstruction->GetTilingDiscriptorMutual()->GetParamsMutual();

            return param.mode_;
        }

        float ReconstructionCommandSet::GetTileMAXRamUsage(BlockObject* block, reconstruction_t reconstruction_id)
        {
            if (!block)
                return 0.0;

            if (!block->GetReconstructionsMutual().count(reconstruction_id))
            {
                return 0.0;
            }

            ReconstructionObject* reconstruction = block->GetReconstructionsMutual().at(reconstruction_id);
            Tiling* tiling_raw = reconstruction->GetTilingDiscriptorMutual();
            tiling_param_s& param = tiling_raw->GetParamsMutual();

            if (param.mode_ == TILE_ADAPTIVE)
            {
                std::cout << "get adaptive ram:" << param.adaptive_params_.target_ram_used_ << std::endl;
                if (param.adaptive_params_.target_ram_used_ < 1.0 || param.adaptive_params_.target_ram_used_ > 1000.0)
                    return 64;
                return param.adaptive_params_.target_ram_used_;
            }
            else
                return 0.0;
        }

        float ReconstructionCommandSet::GetExpectedMaxRamUsageForAJob(ReconstructionObject* reconstruction)
        {
            if (!reconstruction)
                return 0.0;

           
           
            Tiling* tiling_raw = reconstruction->GetTilingDiscriptorMutual();
            tiling_param_s& param = tiling_raw->GetParamsMutual();

            return param.expected_max_ram_used_;
        }

        int ReconstructionCommandSet::DiscardEmptyTile(BlockObject* block, reconstruction_t reconstruction_id)
        {
            
            return AI3D_SUCCESS;
        }

       
        int ReconstructionCommandSet::StaticTilesIntersectionWithConstraint(const EIGEN_STL_UMAP(std::string, tile_info_s)& tiles_got
            , std::vector<constraint_info_s>& cinfos, std::map<int, std::set< int> >& polygon_to_use,
            std::map<std::string, std::pair<int, int>>& tiles)
        {

           
           
            int i = 0;
            for (auto& iter1 : cinfos)
            {
                int j = 0;
                for (auto& iter2 : iter1.polygons_)
                {
                    std::vector<Eigen::Vector2d> pts;
                    ABBox2d newbox,tiltbb;
                    for (auto& iter3 : iter2.points_)
                    {
                        Eigen::Vector2d pt(iter3.x(), iter3.y());
                        pts.push_back(pt);
                        newbox.extend(pt);
                    }
                    for (auto& iter : tiles_got)
                    {
                        auto bbtemp = iter.second.bb_.cast<double>();
                        tiltbb.max() = Eigen::Vector2d(bbtemp.max().x(), bbtemp.max().y());
                        tiltbb.min() = Eigen::Vector2d(bbtemp.min().x(), bbtemp.min().y());
                        bool insected = newbox.intersects(tiltbb);
                        if (insected)
                        {
                            tiles[iter.first] = std::make_pair(i, j);
                            polygon_to_use[i].insert(j);
                        }
                    }
                   
                    j++;
                }
                i++;
            }
            

            
            return AI3D_SUCCESS;



            if(0)
            {
                std::vector<std::vector<Eigen::Vector2d>> boundarybase;
                for (auto& iter1 : cinfos)
                {
                    for (auto& iter2 : iter1.polygons_)
                    {
                        std::vector<Eigen::Vector2d> pts;
                        for (auto& iter3 : iter2.points_)
                        {
                            Eigen::Vector2d pt(iter3.x(), iter3.y());
                            pts.push_back(pt);
                        }
                        boundarybase.push_back(pts);
                    }

                }

                for (auto& iter : tiles_got)
                {
                    OGRGeometry* boundary_2d = nullptr;
                    OGRGeometry* boundary_2d_tmp = nullptr;
                    boundary_2d = BoxToPolygon(iter.second.bb_.cast<double>());
                    {
                        OGRGeometry* geombb = BoxToPolygon(iter.second.bb_.cast<double>());
                        {

                           
                            for (auto& boundary : boundarybase)
                            {
                                OGRGeometry* poly_2d = nullptr;
                                if (boundary_2d)
                                {
                                    poly_2d = boundary_2d->clone();
                                }
                                if (boundary.size() > 3)
                                {
                                   
                                    OGRGeometry* specified_polygon = ToPolygon(boundary);
                                    if (poly_2d)
                                    {
                                        poly_2d = poly_2d->Intersection(specified_polygon);
                                    }
                                    else
                                    {
                                        poly_2d = new OGRPolygon(*static_cast<OGRPolygon*>(specified_polygon));
                                    }
                                    OGRGeometryFactory::destroyGeometry(specified_polygon);
                                    if (!poly_2d)
                                        continue;
                                    if (!poly_2d->IsValid())
                                        continue;
                                }
                                if (!boundary_2d_tmp)
                                {
                                    boundary_2d_tmp = poly_2d;
                                }
                                else
                                {
                                    boundary_2d_tmp = boundary_2d_tmp->Union(poly_2d);
                                    OGRGeometryFactory::destroyGeometry(poly_2d);
                                }
                               

                            }
                        }
                    }
                    boundary_2d = boundary_2d_tmp;
                    if (!boundary_2d)
                    {
                        continue;
                    }
                    if (boundary_2d->IsEmpty())
                    {
                        std::cout << " " << iter.second.name_ << std::endl;
                        if (boundary_2d)
                        {
                            OGRGeometryFactory::destroyGeometry(boundary_2d);
                        }
                    }
                }
            }

            std::vector< std::vector<OGRGeometry*> > metric_geo(cinfos.size());
            
            
            for (int idx = 0; idx < cinfos.size(); idx++)
            {
                auto cinfo = cinfos[idx];
                std::vector < std::vector<Eigen::Vector3d>> rings(cinfo.polygons_.size());
                for (int idx_p = 0; idx_p < cinfo.polygons_.size(); idx_p++)
                {
                    rings[idx_p] = cinfo.polygons_[idx_p].points_;
                }
                std::vector<OGRGeometry*> geoms;
              
                for (int i = 0; i < rings.size(); i++)
                {

                    OGRGeometry* geo = ToPolygon(rings[i]);
                   
                    geoms.push_back(geo);
                    OGRLinearRing* boundary_geometry = nullptr;
                    if (rings[i].size() > 2)
                    {
                        boundary_geometry = static_cast<OGRLinearRing*>(OGRGeometryFactory::createGeometry(wkbLinearRing));
                        for (int j = 0; j < rings[i].size(); j++)
                        {
                            boundary_geometry->addPoint(rings[i][j][0], rings[i][j][1]);


                        }
                        boundary_geometry->closeRings();
                    }
                   
                }
                

                metric_geo[idx] = geoms;
               
            }
            
            EIGEN_STL_UMAP(std::string, tile_info_s)  tiles_tobeused;
           
           
            
            for (auto& iter : tiles_got)
            {

                {
                    OGRGeometry* geombb = BoxToPolygon(iter.second.bb_.cast<double>());
                   

                    for (int i = 0; i < metric_geo.size(); i++)
                    {
                        auto boundaries_geo_metry = metric_geo[i];
                        for (int j = 0; j < boundaries_geo_metry.size(); j++)
                        {
                            auto geobound = boundaries_geo_metry[j];
                            if (geobound != nullptr)
                            {
                                if (geombb->Intersect(geobound))
                                {
                                    tiles_tobeused[iter.first] = iter.second;
                                    tiles[iter.first] = std::make_pair(i, j);
                                    polygon_to_use[i].insert(j);

                                }
                            }
                        }
                    }
                }
            }

            

            return AI3D_SUCCESS;
        }

        int ReconstructionCommandSet::ImportConstraintFile(BlockObject* block,
            reconstruction_t reconstruction_id,const std::vector<std::string>& files,
            std::vector<constraint_info_s>& cinfos, int& progress, std::string& msg,bool bNeedChineseMsg)
        {
            if (0)
            {
                
                if (bNeedChineseMsg)
                {
                    
                    
                    
                }
                return AI3D_FAILURE;
            }

            if (!block->GetReconstructionsMutual().count(reconstruction_id))
            {
                return AI3D_FAILURE;
            }

            ReconstructionObject* reconstruction = block->GetReconstructionsMutual().at(reconstruction_id);
            
            
           
            
            for (auto& file : files)
            {
                std::vector<std::vector<Eigen::Vector3d> > points = ReadPolygonsFromVecFile(file);;

                constraint_info_s cinfo;
               
                cinfo.name_ = File::GetFileNameWithoutExtension(file);

                int index = 0;
                for (auto& point : points)
                {
                    constraint_info_s::polygon_info_s polygon;
                    polygon.id_ = index;
                    polygon.points_ = point;
                 
                    cinfo.polygons_.push_back(polygon);;
                    index++;

                }

                cinfos.push_back(cinfo);
            }
            if (cinfos.empty())
            {
                if (bNeedChineseMsg)
                {
                    msg = "没有有效文件.";
                }
                else
                {
                    msg = "There are no valid files.";
                }
                LOGI(msg);
                return AI3D_FAILURE;
            }

            
            auto  currentcustomsrs = reconstruction->GetCustomSrs();

            if (currentcustomsrs.type == LOCAL)
            {
                if(bNeedChineseMsg)
                    msg = "不支持本地空间参考系统.";
                else
                    msg = "local srs is not support.";
                LOGI(msg);
                return AI3D_FAILURE;
            }
           
            
           

            reconstruction->TransformFromConstraint(cinfos,GEO84SRS, currentcustomsrs.definition);
            
            auto tiles_got = reconstruction->GetTilesCustom();
            
            std::map<int, std::set< int> > polygon_to_use;
                std::map<std::string, std::pair<int, int>> tiles;
            StaticTilesIntersectionWithConstraint(tiles_got, cinfos, polygon_to_use, tiles);
            if (polygon_to_use.empty())
            {
                if(bNeedChineseMsg)
                    msg = "没有相交瓦片.";
                else
                    msg = "There are no intersect tiles.";
                LOGI(msg);
                return AI3D_FAILURE;
            }
            
            std::vector<constraint_info_s> cinfos_touse(polygon_to_use.size());
            int pidx = 0;

            for (auto& iter1 : polygon_to_use)
            {
                cinfos_touse[pidx].name_ = cinfos[iter1.first].name_;
                cinfos_touse[pidx].polygons_.resize(iter1.second.size());
                int cidx = 0;
                for (auto& iter2 : iter1.second)
                {
                    cinfos_touse[pidx].polygons_[cidx].name_ = "constaint_" + std::to_string(pidx) + "_" + std::to_string(cidx);
                    auto points = cinfos[iter1.first].polygons_[iter2].points_;
                    cinfos_touse[pidx].polygons_[cidx].points_ = points;
                    {
                        ABBox3d newbox;
                        std::vector<std::vector<Eigen::Vector2d> > boundaries(points.size());
                        for (int ip = 0; ip < points.size(); ip++)
                        {

                            newbox.extend(points[ip]);

                        }
                        cinfos_touse[pidx].polygons_[cidx].box_ = newbox;
                    }
                    cidx++;
                }
                pidx++;
            }
          
           
            
            
            


            

            
           
            return AI3D_SUCCESS;
        }
        
        int ReconstructionCommandSet::DeleteConstraintsPost(BlockObject* block, reconstruction_t reconstruction_id, 
            const std::vector<int>& indexs, std::map<std::string, bool> tilestoprocess)
        {
            if (!block->GetReconstructionsMutual().count(reconstruction_id))
            {
                return AI3D_FAILURE;
            }

            ReconstructionObject* reconstruction = block->GetReconstructionsMutual().at(reconstruction_id);
          
            std::vector<constraint_info_s> constraint_custom = reconstruction->GetConstraintCustomMutual();         
    
            std::vector<constraint_info_s> constraint_custom_new, constraint_base_new;
            
            for (int idx = 0; idx < constraint_custom.size(); idx++)
            {
                if (std::find(indexs.begin(), indexs.end(), idx) != indexs.end())
                {
                    continue;
                }
                constraint_custom_new.push_back(constraint_custom[idx]);
                
            }
            
            reconstruction->SetConstraintCustom(constraint_custom_new);

            auto productions = reconstruction->GetProductions();
            
            
            for (auto& iterpro : productions)
            {
                auto protiles = iterpro.second->GetTiles();


                for (auto& itertile : protiles)
                {

                   
                        if (tilestoprocess.count(itertile.first) && tilestoprocess.at(itertile.first))
                        {
                            itertile.second.status_ = jobsta_e::STATUS_CANCLE;
                            
                        }
                    
                }
            }

            return AI3D_SUCCESS;
        }

        
        
        
        
        
        int ReconstructionCommandSet::DeleteConstraintsPre(BlockObject* block, reconstruction_t reconstruction_id,
            const std::vector<int>& index,
            std::map<std::string, bool>& tilestocancle)
        {
            if (!block->GetReconstructionsMutual().count(reconstruction_id))
            {
                return AI3D_FAILURE;
            }

            ReconstructionObject* reconstruction = block->GetReconstructionsMutual().at(reconstruction_id);
          
            std::vector<constraint_info_s> constraint_custom= reconstruction->GetConstraintCustomMutual();
            if (index.size() > constraint_custom.size())
            {
                return AI3D_FAILURE;
            }
            
            std::vector<constraint_info_s> constraint_custom_delete;
          
            for (int idx = 0; idx < constraint_custom.size(); idx++)
            {
                if (std::find(index.begin(), index.end(), idx) != index.end())
                {
                    constraint_custom_delete.push_back(constraint_custom[idx]);
                }   
            }
            auto tiles_got = reconstruction->GetTilesCustom();
            std::map<int, std::set< int> > polygon_to_use;
            std::map<std::string, std::pair<int, int>> tiles;
            StaticTilesIntersectionWithConstraint(tiles_got, constraint_custom_delete, polygon_to_use,tiles);
            if (tiles.empty())
            {
                LOGE("there is not any tiles intersected with constraint(s)." );
                return AI3D_FAILURE;
            }
            
            auto productions = reconstruction->GetProductions();
            
            std::map<std::string,bool> tilestatus;
            
            for (auto& iterpro : productions)
            {
                auto protiles = iterpro.second->GetTiles();
             
                for (auto& itertile : tiles)
                {
                    
                    if (protiles.count(itertile.first))
                    {
                        if (tilestatus.count(itertile.first))
                        {
                            if (protiles.at(itertile.first).status_ == jobsta_e::STATUS_PENDDING ||
                                protiles.at(itertile.first).status_ == jobsta_e::STATUS_RUNNING)
                            {
                                tilestatus.at(itertile.first) = true;
                            }
                        }
                    }
                }
            }
            for (auto iter : tiles)
            {
                tilestocancle[iter.first] = tilestatus.count(iter.first);
            }
            
           bool bStatusInProduction = !tilestatus.empty();
          int  num_tiles = bStatusInProduction ? tilestatus.size() :tiles.size();
            
            
            
            
            
            
            
            
            
            
            
            
            
            return AI3D_SUCCESS;
        }

        int ReconstructionCommandSet::ResetBoudingBoxCalcMode(BlockObject* block, reconstruction_t reconstruction_id, bbox_calc_mode_e mode)
        {
            if (!block->GetReconstructionsMutual().count(reconstruction_id))
            {
                return AI3D_FAILURE;
            }

            ReconstructionObject* reconstruction = block->GetReconstructionsMutual().at(reconstruction_id);
            bbox_calc_mode_e mode_temp = mode;
            
            auto data = reconstruction->GetATData();
            return AI3D_SUCCESS;
        }

        std::string ReconstructionCommandSet::MakeProductionTileJobKey(block_t block_id, reconstruction_t reconstruction_id,
            production_t production_id, const std::string& tile_name)
        {
            return "B" + std::to_string(block_id) + "R" + std::to_string(reconstruction_id)
                + "P" + std::to_string(production_id) + tile_name;
        }

        std::string ReconstructionCommandSet::ResolveProductionTileJobStr(BlockObject* block,
            ReconstructionObject* reconstruction, ProductionObject* production,
            const std::string& tile_name, bool update_tile)
        {
            if (!production || tile_name.empty() || !production->GetTilesMutual().count(tile_name))
            {
                return "";
            }

            production_tileinfo_s& tile = production->GetTilesMutual().at(tile_name);
            if (!tile.jobstr_.empty())
            {
                return tile.jobstr_;
            }

            if (!block || !reconstruction)
            {
                return "";
            }

            const std::string key = MakeProductionTileJobKey(
                block->GetId(), reconstruction->GetId(), production->GetId(), tile_name);
            auto& jobs = block->GetTaskInfoMutual().reconstructionjobs_;
            if (!jobs.count(key))
            {
                return "";
            }

            if (update_tile)
            {
                tile.jobstr_ = jobs.at(key);
            }
            return jobs.at(key);
        }

        void ReconstructionCommandSet::SyncProductionTileJobStrs(BlockObject* block,
            ReconstructionObject* reconstruction, ProductionObject* production)
        {
            if (!block || !reconstruction || !production)
            {
                return;
            }
            for (auto& entry : production->GetTilesMutual())
            {
                ResolveProductionTileJobStr(block, reconstruction, production, entry.first, true);
            }
        }

        std::string ReconstructionCommandSet::GenerateTileFeedbackFile(BlockObject* block_object,ProductionObject* production_object,std::string&tile,std::string &job)
        {
            if (!production_object || tile.empty())
            {
                return "";
            }

            if (job.empty() && block_object)
            {
                ReconstructionObject* reconstruction = production_object->GetReconstructionObject();
                job = ResolveProductionTileJobStr(block_object, reconstruction, production_object, tile, true);
            }

            if (job.empty())
            {
                return "";
            }

            if (production_object->GetTiles().count(tile) <= 0)
                return "";

            const production_tileinfo_s& production_tileinfo = production_object->GetTiles().at(tile);
            std::string feedbackpath = production_object->GetPath() + "/" + production_tileinfo.name_ + "/";
            feedbackpath = AI3D::CORE::File::EnsureUnifySlash(feedbackpath);
            
            std::string feedback_file = "";
            if (JOB_FEEDBACK_USE_BIN) {
                feedback_file = MAKE_FEEDBAK_BIN_FILE(feedbackpath, job);
            }
            else {
                feedback_file = MAKE_FEEDBAK_JSON_FILE(feedbackpath, job);
            }

            if (File::ExistsFile(feedback_file))
            {
                std::ostringstream oss;
                oss << "Exists:" << feedback_file;
                
            }
            else
            {
                std::ostringstream oss;
                oss << "Doesn't exist:" << feedback_file;
                
                feedback_file = "";
            }

            return feedback_file;
        }
        
        std::string ReconstructionCommandSet::GenerateFeedbackFile(BlockObject* block, ReconstructionObject* reconstruction, ProductionObject* production, std::string tile,const std::string& lsMasterJobQueue, std::string fullPathJobName, int* pJobStatus)
        {
            

            if (pJobStatus)
                *pJobStatus = -1;

            if (block == nullptr || reconstruction == nullptr || production == nullptr || tile.empty())
            {
            
                return "";
            }

            

            std::string block_path = block->GetPath();
            
            {
               
                auto& tiles_for_process = production->GetTiles();

                if (tiles_for_process.size() <= 0)
                {
            
                    return "";
                }

                if (!tiles_for_process.count(tile))
                {
            
                    return "";
                }
                int ii = 0;
                int iii = 1;
                int iiiii = 3;
              
              
                BlockObject::Task_Info task_info;
                try
                {
                    task_info = (const_cast<BlockObject*>(block))->GetTaskInfo();
                }
                catch (std::exception& ex)
                {
                    
                    std::ostringstream oss;
                    oss << " inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " <<
                        ex.what();
                    LOGI(oss.str());
                    std::cout << oss.str() << std::endl;
                    return "";
                }

                std::string BRPID = std::string("B") + std::to_string(block->GetId()) + std::string("R") +
                    std::to_string(reconstruction->GetId()) + std::string("P") +
                    std::to_string(production->GetId()) + tile;

              
std::string BRPJob;



if (task_info.reconstructionjobs_.find(BRPID) != task_info.reconstructionjobs_.end())
{
    BRPJob = task_info.reconstructionjobs_.at(BRPID);
    if (!BRPJob.empty())
    {
        
        try
        {
            
            std::string feedPath = File::EnsureTrailingSlash(block_path) + reconstruction->GetIDString() + std::string("/Productions/") + production->GetIDString()
                + "/" + tile;
            std::string feedback_file = "";
            if (JOB_FEEDBACK_USE_BIN) {
                feedback_file = MAKE_FEEDBAK_BIN_FILE(feedPath, BRPJob);
            }
            else {
                feedback_file = MAKE_FEEDBAK_JSON_FILE(feedPath, BRPJob);
            }
            
            std::ifstream ifs = File::OpenIfstreamUtf8(feedback_file, std::ios::in);
            if (ifs.good())
            {
                

                return feedback_file;
            }
            else
            {
                

            }
        }
        catch (std::exception e)
        {
            
            return "";
        }
        
    }

    
                       
}

if (0)
{
    auto tiles_global_base = reconstruction->GetTilesCustom();
    if (tiles_global_base.count(tile))
    {
        
        tile_info_s tileinfo = tiles_global_base.at(tile);

        std::string first_tile;
        std::string first_job;

        for (auto& t : task_info.reconstructionjobs_)
        {
            first_tile = t.first;
            first_job = t.second;
            break;
        }

        if (!first_tile.empty() && !first_job.empty())
        {
            auto strsvec = AI3D::CORE::String::StringSplit(first_job, "_");
            
            if (strsvec.size() >= 6)
            {
                
                std::string missing_job = JOB_PREFIX + strsvec[1] + "_TILE_" + std::to_string(tileinfo.index_) + "_" +
                    strsvec[4] + "_" + strsvec[5] + "_" + tileinfo.name_;

                

                    
                int jobStatus = -1;

                if (!lsMasterJobQueue.empty())
                {
                    
                    bool bCheckResult = TaskCommandSet::CheckJobStatusInsideJobQueuePath(lsMasterJobQueue, missing_job, fullPathJobName, jobStatus);
                    
                    if (pJobStatus)
                        *pJobStatus = jobStatus;

                    if (bCheckResult && jobStatus >= 0)
                        return missing_job;
                }

                
            }
            
        }
    }
}

            }

            if(BlockObject::supportTempLogs())
            {
                std::ostringstream oss;
                oss << "generating feedback file failed:" << tile;

            }

            
            return "";
        }

        int ReconstructionCommandSet::CreateProductionJobFiles(std::string hostname, std::string jobpath, std::string projectpath,
            BlockObject* block, reconstruction_t reconstruction_id, production_t production_id ,std::vector<std::string> tiles_to_production)
        {
           
            
            if (!block->GetReconstructionsMutual().count(reconstruction_id))
            {
                return AI3D_FAILURE;
            }

            ReconstructionObject* reconstruction = block->GetReconstructionsMutual().at(reconstruction_id);
            ProductionObject* production = reconstruction->GetProductionsMutual().at(production_id);
           
            auto tiles_global_custom = reconstruction->GetTilesCustom();
            auto  settings = reconstruction->GetProcessingSettings();
          
            std::sort(tiles_to_production.begin(), tiles_to_production.end(), [&](std::string tile1, std::string tile2)
                {
                    return tiles_global_custom.at(tile1).index_ < tiles_global_custom.at(tile2).index_;
                });
           
            auto& tiles_in_production = production->GetTilesMutual();
            projectpath = File::EnsureUnifySlash(projectpath);
            
            std::string projectname = File::GetFileNameWithoutExtension(projectpath);
            
            std::string projectdir = File::GetParentDir(projectpath);
            
            auto& reconstructionjobs = block->GetTaskInfoMutual().reconstructionjobs_;
            int numoftileupdate = 0;
           
            auto global_bounbary_custom = reconstruction->GetBoundaryCustom();

            block_t blockid = reconstruction->GetBlockId();
            std::string prodir = "Block_" + std::to_string(blockid) + "/" + reconstruction->GetIDString() + "/" + PRODUCTION_DIR
                + "/" + production->GetIDString() + "/";
           
            prodir = File::EnsureUnifySlash(prodir);
            

            std::string BRPStr = "B" + std::to_string(blockid) + "R" + std::to_string(reconstruction->GetId()) +
                "P" + std::to_string(production->GetId());
            std::cout << tiles_in_production.size() <<"create jobfile ================" <<tiles_to_production.size() << std::endl;
#ifdef USE_OPENMP
#pragma omp parallel for
#endif 
            for (int index=0 ; index < tiles_to_production.size(); index++)
            {
                std::string tilename = tiles_to_production[index];

                if (tiles_in_production.at(tilename).status_ == jobsta_e::STATUS_COMPLETE 
                    || tiles_in_production.at(tilename).status_ == jobsta_e::STATUS_RUNNING)
                {
                    continue;
               }
                if (!tiles_global_custom.count(tilename))
                {
                    LOGE(" tile is not exist.");
                    continue;
                }
                tile_info_s tileinfo = tiles_global_custom.at(tilename);

                
                ProductionOptions def;
                std::string datetime = GetCurrentTimeStr();
                
                
               
                def.tilebb_ = tileinfo.bb_;
               
                def.tiling_srs_def_ = reconstruction->GetCustomSrs().definition;
                
                std::string blockitem = prodir + tilename;
                blockitem = File::EnsureUnifySlash(blockitem);
               
                
                def.item_path_ = blockitem;
                def.project_path_ = projectpath;
                

                std::string job = JOB_PREFIX + datetime + "_TILE_" + std::to_string(tileinfo.index_) + "_" +
                    projectname + "_" + BRPStr + "_" + tileinfo.name_;
               
               if(0) 
                {
                    
                    
                    std::string image_tilepath = projectdir + "/" + prodir + "indexes/";
                    File::CreateDirIfNotExists(image_tilepath,true);
                    for (auto& iter_idx : tileinfo.image_ids_)
                    {
                        std::string idexfile = image_tilepath + std::to_string(iter_idx) + "_" + tileinfo.name_;
                        std::ofstream stf = File::OpenOfstreamUtf8(idexfile, std::ios::out);
                        stf.close();
                    }
                    
                }

                def.job_ = job;
               
                def.boundary_custom_ = global_bounbary_custom;
                def.process_settings_ = settings;
               

                def.production_settings_ = production->GetOptions();
                
                   
                
                std::string productiontile_dir = projectdir + "/" + blockitem + "/";
                productiontile_dir = File::EnsureUnifySlash(productiontile_dir);
                File::MakeDirEmpty(productiontile_dir);             
                
                std::string taskdefdir = productiontile_dir + def.job_ + "/";
                
                std::string taskFirstfile = "";
                int firstId = 0;
                if (TASK_USE_BIN) {
                    
                    taskFirstfile = MAKE_TASK_BIN_FILE(taskdefdir, std::to_string(firstId));
                }
                else {
                    taskFirstfile = MAKE_TASK_JSON_FILE(taskdefdir, std::to_string(firstId));
                }
                File::CreateDirIfNotExists(taskdefdir, true);
                
                
                std::string taskdeffile = taskFirstfile;
                if (TASK_USE_BIN) {
                    def.saveBin(taskdeffile);
                }
                else {
                    def.save(taskdeffile);
                }
                
                numoftileupdate++;
                
                TaskCommandSet::CreateJobAndFeedbackFiles(jobpath, projectpath, blockitem,
                    hostname, datetime, productiontile_dir, job, true);

#ifdef USE_OPENMP
#pragma omp critical
#endif           
                reconstructionjobs[BRPStr + tileinfo.name_] = job;
                tiles_in_production.at(tilename).jobstr_ = job;
                tiles_in_production.at(tilename).status_ = jobsta_e::STATUS_PENDDING;

                {
                    std::ostringstream oss;
                    oss << "create job file :" << job << " for tile:" << tilename;
              
                }

            }

            
            if(numoftileupdate>0)
                block->GetTaskInfoMutual().isSaved = false;
            
            return AI3D_SUCCESS;
        }


        
       
        
         bool ReconstructionCommandSet::CanResubmitProduction(const ReconstructionObject& object, production_t production_id)
         {
             
             ReconstructionObject obj = object;
             std::vector<std::string> tiles_cancelled,tiles_failed;
             tiles_cancelled = obj.GetTilesByStatus(production_id, jobsta_e::STATUS_CANCLE);
             tiles_failed = obj.GetTilesByStatus(production_id, jobsta_e::STATUS_FAILURE);
             int num = tiles_cancelled.size() + tiles_failed.size();
             return (num>0);
  
         }

         bool ReconstructionCommandSet::CanCancelProduction(const ReconstructionObject& object, production_t production_id)
         {
             ReconstructionObject obj = object;
             std::vector<std::string> tiles_running, tiles_pending;
             tiles_running = obj.GetTilesByStatus(production_id, jobsta_e::STATUS_RUNNING);
             tiles_pending = obj.GetTilesByStatus(production_id, jobsta_e::STATUS_PENDDING);
             int num = tiles_running.size() + tiles_pending.size();
             return (num > 0);
           
          
         }

         bool ReconstructionCommandSet::CanDeleteProduction(const ReconstructionObject& object, production_t production_id)
         {
             
             return true;
         }

         bool ReconstructionCommandSet::CanDeleteReconstruction(const ReconstructionObject& reconstruction)
         {
             
             return true;
         }

         bool ReconstructionCommandSet::CanCloneReconstruction(const ReconstructionObject& reconstruction)
         {
             
             return true;
         }
         
        int ReconstructionCommandSet::ResubmitProductionJob(std::string hostname, std::string jobstr,
            std::string projectpath, BlockObject* block, reconstruction_t reconstruction_id, production_t production_id)
        {
            if (!block->GetReconstructionsMutual().count(reconstruction_id))
            {
                return AI3D_FAILURE;
            }

            ReconstructionObject* reconstruction = block->GetReconstructionsMutual().at(reconstruction_id);
            
            auto production = reconstruction->GetProductionMutual(production_id);
            std::vector<std::string> tiles_cancelled, tiles_failed, tiles_resubmit;
            tiles_cancelled = production->GetTilesByStatus( jobsta_e::STATUS_CANCLE);
            tiles_failed = production->GetTilesByStatus( jobsta_e::STATUS_FAILURE);
           
            
            tiles_resubmit.insert(tiles_resubmit.end(), tiles_cancelled.begin(), tiles_cancelled.end());
            tiles_resubmit.insert(tiles_resubmit.end(), tiles_failed.begin(), tiles_failed.end());
          
            
          
            for (auto& iter : tiles_resubmit)
            {
                CreateProductionJobFiles(hostname, jobstr, projectpath, block, reconstruction->GetId(), production->GetId(),tiles_resubmit);

            }

           
            return AI3D_SUCCESS;
        }


        int  ReconstructionCommandSet::ReNameReconstruction(BlockObject* block, reconstruction_t reconstruction_id, const std::string& name)
        {
            if (!block->GetReconstructionsMutual().count(reconstruction_id))
            {
                return AI3D_FAILURE;
            }

            ReconstructionObject* reconstruction = block->GetReconstructionsMutual().at(reconstruction_id);

            reconstruction->ReName(name);
            auto& taskinfo = block->GetTaskInfoMutual();

            blk_reconst_production_info_s info;

            auto recinfo = std::find_if(taskinfo.reconstructions_info_.begin(), taskinfo.reconstructions_info_.end(),
                [&](blk_recontruction_info_s a) { return reconstruction->GetId() == a.id_; });
            if (recinfo == taskinfo.reconstructions_info_.end())
            {
                return AI3D_FAILURE;

            }

          
            (*recinfo).name_ = name;

            taskinfo.isSaved = false;
            return AI3D_SUCCESS;
        }
        int  ReconstructionCommandSet::ReNameProduction(BlockObject* block, reconstruction_t reconstruction_id, production_t production_id, const std::string& name)
        {
            if (!block->GetReconstructionsMutual().count(reconstruction_id))
            {
                return AI3D_FAILURE;
            }
           
            ReconstructionObject* reconstruction = block->GetReconstructionsMutual().at(reconstruction_id);
            if (!reconstruction->GetProductionsMutual().count(production_id))
            {
                return AI3D_FAILURE;
            }
            ProductionObject* production = reconstruction->GetProductionsMutual().at(production_id);
            production->ReName(name);
            auto& taskinfo = block->GetTaskInfoMutual();

            blk_reconst_production_info_s info;
            
            auto recinfo = std::find_if(taskinfo.reconstructions_info_.begin(), taskinfo.reconstructions_info_.end(),
                [&](blk_recontruction_info_s a) { return reconstruction->GetId() == a.id_; });
            if (recinfo == taskinfo.reconstructions_info_.end())
            {
                return AI3D_FAILURE;

            }

            auto proindfo = std::find_if(recinfo->production_infos_.begin(), recinfo->production_infos_.end(),
                [&](blk_reconst_production_info_s a) { return production->GetId() == a.id_; });


            if (proindfo == recinfo->production_infos_.end())
            {
                return AI3D_FAILURE;
               
            }
           
            (*proindfo).name_ = name;

            taskinfo.isSaved = false;
            return AI3D_SUCCESS;
        }


        bool  ReconstructionCommandSet::CanSubmitProduction(const  ReconstructionObject& object)
        {
            
            return object.HasTiles();

        }
        void ReconstructionCommandSet::GetJobsToCancelled(const BlockObject& object, std::vector<std::pair<std::string, std::string> >& jobs_to_delete)
        {
            for (auto& iterrec : object.GetReconstructions())
            {
                for (auto& iterpro : iterrec.second->GetProductions())
                {
                    for (auto& itertile : iterpro.second->GetTiles())
                    {
                        if (itertile.second.status_ == jobsta_e::STATUS_RUNNING ||
                            itertile.second.status_ == jobsta_e::STATUS_PENDDING)
                        {
                            std::string jobstr = itertile.second.jobstr_;
                            

                            std::string feedbackpath = iterpro.second->GetPath() + "/" + itertile.second.name_ + "/";
                            feedbackpath = AI3D::CORE::File::EnsureUnifySlash(feedbackpath);
                            jobs_to_delete.push_back(std::make_pair(feedbackpath, (jobstr)));

                        }
                    }
                }
            }
        }
        void ReconstructionCommandSet::GetJobsToCancelled(const ReconstructionObject& object, std::vector<std::pair<std::string, std::string> >& jobs_to_delete)
        {
            for (auto& iterpro : object.GetProductions())
            {
                for (auto& itertile : iterpro.second->GetTiles())
                {
                    if (itertile.second.status_ == jobsta_e::STATUS_RUNNING ||
                        itertile.second.status_ == jobsta_e::STATUS_PENDDING)
                    {
                        std::string jobstr = itertile.second.jobstr_;
                       

                        std::string feedbackpath = iterpro.second->GetPath() + "/" + itertile.second.name_ + "/";
                        feedbackpath = AI3D::CORE::File::EnsureUnifySlash(feedbackpath);
                        jobs_to_delete.push_back(std::make_pair(feedbackpath, (jobstr)));

                    }
                }
            }
        }
        void ReconstructionCommandSet::GetJobsToCancelled(const ProductionObject& object, std::vector<std::pair<std::string, std::string> >& jobs_to_delete)
        {
            for (auto& itertile : object.GetTiles())
            {
                if (itertile.second.status_ == jobsta_e::STATUS_RUNNING ||
                    itertile.second.status_ == jobsta_e::STATUS_PENDDING)
                {
                    std::string jobstr = itertile.second.jobstr_;

                    
                    
                    std::string feedbackpath = object.GetPath() + "/" + itertile.second.name_ + "/";
                    feedbackpath = AI3D::CORE::File::EnsureUnifySlash(feedbackpath);
                    jobs_to_delete.push_back(std::make_pair(feedbackpath, (jobstr)));

                }
            }
        }

        
        int ReconstructionCommandSet::CancelProductionJob(std::string jobPath,BlockObject* block, reconstruction_t reconstruction_id, production_t production_id)
        {
            
            
         
            if (!block->GetReconstructionsMutual().count(reconstruction_id))
            {
                return AI3D_FAILURE;
            }
            ReconstructionObject* reconstruction = block->GetReconstructionsMutual().at(reconstruction_id);
            if (!reconstruction->GetProductionsMutual().count(production_id))
            {
                return AI3D_FAILURE;
            }
            ProductionObject* production = reconstruction->GetProductionsMutual().at(production_id);
            
            auto& tiles = production->GetTilesMutual();
            std::vector<std::string> tiles_pending, tiles_running,tilestocancel;
            tiles_pending = production->GetTilesByStatus( jobsta_e::STATUS_PENDDING);
            tiles_running = production->GetTilesByStatus( jobsta_e::STATUS_RUNNING);
            tilestocancel.insert(tilestocancel.end(), tiles_pending.begin(), tiles_pending.end());
            tilestocancel.insert(tilestocancel.end(), tiles_running.begin(), tiles_running.end());
            
            bool cancelResult = true;
            for (auto& iter : tilestocancel)
            {
                std::string feedbackfile = GenerateFeedbackFile(block,reconstruction,production,iter);
                std::string jobname = File::GetFileNameWithoutExtension(feedbackfile);
                std::string prefix = FEEDBACK_BIN_PREFIX;
                
                String::StringRemove(jobname, prefix);
                int code;
                bool itemCancel = TaskCommandSet::DoCancelJob(jobPath, feedbackfile, jobname, code);
                if (!itemCancel) {
                    cancelResult = false;
                    break;
                }
                tiles.at(iter).status_ = jobsta_e::STATUS_CANCLE;
            }
            if (cancelResult) {
                return AI3D_SUCCESS;
            }
            else {
                return AI3D_FAILURE;
            }
            
            
        }
        
        
        

        
        
        
        
        
        int ReconstructionCommandSet::DeleteReconstruction(BlockObject* block, reconstruction_t reconstruction_id)
        {
            


            if (!block->GetReconstructionsMutual().count(reconstruction_id))
            {
                return AI3D_FAILURE;
            }

            ReconstructionObject* reconstruction = block->GetReconstructionsMutual().at(reconstruction_id);
           
            block->DeleteReconstruction(reconstruction_id);

            auto& taskinfo = block->GetTaskInfoMutual();



            auto recinfo = std::find_if(taskinfo.reconstructions_info_.begin(), taskinfo.reconstructions_info_.end(),
                [&](blk_recontruction_info_s a) { return reconstruction->GetId() == a.id_; });
            if (recinfo == taskinfo.reconstructions_info_.end())
            {
                return AI3D_FAILURE;

            }
            else
            {
                taskinfo.reconstructions_info_.erase(recinfo);
            }

            

            
            std::string rptstring = "B" + std::to_string(block->GetId()) + "R" + std::to_string(reconstruction_id) ;

            for (auto it = taskinfo.reconstructionjobs_.begin(); it != taskinfo.reconstructionjobs_.end();)
            {
                std::string::size_type idx;

                idx = it->first.find(rptstring);
                if (idx == std::string::npos)
                {
                    ++it;
                }
                else
                {
                    taskinfo.reconstructionjobs_.erase(it);
                }
            }

            taskinfo.isSaved = false;

            return AI3D_SUCCESS;
        }

        int ReconstructionCommandSet::ResetBoundingboxToDefault(BlockObject* block, reconstruction_t reconstruction_id)
        {
            
            
            if (!block->GetReconstructionsMutual().count(reconstruction_id))
            {
                return AI3D_FAILURE;
            }
            ReconstructionObject* reconstruction = block->GetReconstructionsMutual().at(reconstruction_id);
            auto data = reconstruction->GetATDataCustom();
            data.ComputeTileBoundingBox(bb_scope_e::BB_SCOPE_TIEPOINTS);
            ABBox3d box = data.GetTileAABBBox().cast<double>();
           
            reconstruction->GetBoundaryCustomMutual().clear();
            ResetBoundingBox(block, reconstruction_id, box);
            return AI3D_SUCCESS;
        }

        int ReconstructionCommandSet::CloneReconstruction(BlockObject* block, const reconstruction_t reconstruction_id, reconstruction_t& clone_reconstruction_id)
        {
            
            block->CloneReconstruction(reconstruction_id, clone_reconstruction_id);
           
            return AI3D_SUCCESS;
        }
        
        int ReconstructionCommandSet::UpdateBlockInfo(BlockObject* block, reconstruction_t reconstruction_id)
        {
            {
                ReconstructionObject* reconstruction = block->GetReconstructionsMutual().at(reconstruction_id);
                auto& taskinfo = block->GetTaskInfoMutual();
                auto recindfo = std::find_if(taskinfo.reconstructions_info_.begin(), taskinfo.reconstructions_info_.end(),
                    [&](blk_recontruction_info_s a) { return reconstruction->GetId() == a.id_; });

                if (recindfo != taskinfo.reconstructions_info_.end())
                {
                    reconstruction->ToTaskInfo(*recindfo);
                }
                else
                {
                    blk_recontruction_info_s info;
                    reconstruction->ToTaskInfo(info);
                    taskinfo.reconstructions_info_.push_back(info);
                }
                taskinfo.isSaved = false;
            }

            return AI3D_SUCCESS;
        }
        
        
        int ReconstructionCommandSet::ResetBoundingboxScopeMode(BlockObject* block, reconstruction_t reconstruction_id, bb_scope_e mode)
        {
            bool bremoveoutliers = true;
            
            if (!block->GetReconstructionsMutual().count(reconstruction_id))
            {
                return AI3D_FAILURE;
            }

            ReconstructionObject* reconstruction = block->GetReconstructionsMutual().at(reconstruction_id);
           
            reconstruction->GetBoundaryCustomMutual().clear();
            auto prebox = reconstruction->GetBoundingBoxCustom();
            auto data = reconstruction->GetATDataCustom();
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            data.ComputeTileBoundingBox(mode, bremoveoutliers);
            ABBox3d box = data.GetTileAABBBox().cast<double>();
            
           
            
            ResetBoundingBox(block, reconstruction_id, box);

           
            return AI3D_SUCCESS;
        }

        
        bool ReconstructionCommandSet::GetSceneUnit(const ReconstructionObject& reconstruction)
        {
            
            return reconstruction.GetATData().GetSceneUnit();
        }

        void ReconstructionCommandSet::InitProductionOptions(BlockObject* block, ReconstructionObject* reconstruction, production_option_s& options)
        {
            {
            
              
                int produtionsnum = reconstruction->GenerateValidProductionId();
                options.id_ = produtionsnum;
                options.name_ = PRODUCTION_PREFIX + std::to_string(produtionsnum);
               
                auto tiles_to_production = reconstruction->GetTilesName(reconstruction->GetProcessingSettings().bdiscard_emptytiles_);;
                options.tiles_.assign(tiles_to_production.begin(), tiles_to_production.end());
               
                std::string desdir = options.destination_ + "Productions/" ;
                desdir = File::EnsureUnifySlash(desdir);
                
                
                
                std::string newname = options.name_;
                std::vector<std::string> dirnames = File::GetDirList(desdir);
               
                
               
                bool isnew = false;
                newname = File::AutoGeneratedFullFilePath(desdir, newname,isnew);
                options.destination_ =  File::EnsureUnifySlash(newname);
               
                

                
                
                auto atdata = block->GetCurrentAT();
                reconstruction->GetCustomTilingSrs(options.cs_.definition_, options.cs_.origin_);
                
                options.avgresolution_ = reconstruction->GetATDataMutual().ComputeAvgResolution();
               
                options.unit_ = reconstruction->GetATData().GetSceneUnit();;
            }
        }


        

        int ReconstructionCommandSet::DeleteProduction(BlockObject* block,const reconstruction_t& reconstruction_id, const production_t& production_id)
        {
           
            if (!block->GetReconstructionsMutual().count(reconstruction_id))
            {
                return AI3D_FAILURE;
            }

            ReconstructionObject* reconstruction = block->GetReconstructionsMutual().at(reconstruction_id);
            
            if (!reconstruction->GetProductionsMutual().count(production_id))
            {
                return AI3D_FAILURE;
            }
            ProductionObject* production = reconstruction->GetProductionsMutual().at(production_id);
            reconstruction->DeleteProduction(production_id);


          
            auto& taskinfo = block->GetTaskInfoMutual();

          
           
            auto recinfo = std::find_if(taskinfo.reconstructions_info_.begin(), taskinfo.reconstructions_info_.end(),
                [&](blk_recontruction_info_s a) { return reconstruction->GetId() == a.id_; });
            if (recinfo == taskinfo.reconstructions_info_.end())
            {
                return AI3D_FAILURE;

            }

            auto proindfo = std::find_if(recinfo->production_infos_.begin(), recinfo->production_infos_.end(),
                [&](blk_reconst_production_info_s a) { return production->GetId() == a.id_; });

            if (proindfo != recinfo->production_infos_.end())
            {
                recinfo->production_infos_.erase(proindfo);
            }
            else
            {
                return AI3D_FAILURE;

            }

            
            std::string rptstring = "B" + std::to_string(block->GetId()) + "R" + std::to_string(reconstruction_id) + "P" + std::to_string(production_id);

            for (auto it = taskinfo.reconstructionjobs_.begin(); it != taskinfo.reconstructionjobs_.end();)
            {
                std::string::size_type idx;

                idx = it->first.find(rptstring);
                if (idx == std::string::npos)
                {
                    ++it;
                }
                else
                {
                    taskinfo.reconstructionjobs_.erase(it);
                }
            }

            taskinfo.isSaved = false;

            return AI3D_SUCCESS;
        }

        
        int ReconstructionCommandSet::SubmitProduction(std::string hostname, std::string jobstr,
            std::string projectpath, BlockObject* block, reconstruction_t reconstruction_id,production_option_s options, production_t&production_id)
        {
            
            production_id = -1;
            if (!block->GetReconstructionsMutual().count(reconstruction_id))
            {
                return AI3D_FAILURE;
            }
          
            ReconstructionObject* reconstruction = block->GetReconstructionsMutual().at(reconstruction_id);

            {
                const std::string rb_path = File::EnsureUnifySlash(
                    reconstruction->GetPath() + "/" + PRODUCTIONVIEWIDSBIN);
                const bool first_production = !reconstruction->HasProductions();
                if (first_production || !File::ExistsFile(rb_path))
                {
                    ReconPerfLog(String::StringPrintf(
                        "[ReconPerf] SubmitProduction | ExportReconstructionViewBin | first=%d exists=%d",
                        first_production ? 1 : 0, File::ExistsFile(rb_path) ? 1 : 0));
                    if (ExportReconstructionViewBin(block, reconstruction) != AI3D_SUCCESS)
                    {
                        LOGE("SubmitProduction: ExportReconstructionViewBin failed");
                        return AI3D_FAILURE;
                    }
                }
            }

            ProductionObject *production = new ProductionObject(options,reconstruction);
            
            
            
            std::string productiontile_dir = File::GetParentDir(projectpath) + "/" + block->GetIdString() + "/" + reconstruction->GetIDString() + "/";
            std::string production_lock_dir = File::GetParentDir(projectpath) + block->GetIdString() + "/" + reconstruction->GetIDString() + "/" + "merge.lock";
            std::ofstream outFile = File::OpenOfstreamUtf8(production_lock_dir, std::ios::out); 

            if (!outFile) {
                std::cerr << "文件打开失败" << std::endl;
                outFile.close();
            }

            outFile << "Merge lock file" << std::endl;
            outFile.close(); 
       
            reconstruction->AddProduction(production);
            reconstruction->WriteTiles(productiontile_dir);
            
           
            
            File::CreateDirIfNotExists(production->GetPath(), true);
            File::CreateDirIfNotExists(options.destination_, true);
            

            std::string file = options.destination_ + "/metadata.xml";
            File::EnsureUnifySlash(file);
            options.cs_.CreateXml(file);
          

            
            
          
            CreateProductionJobFiles(hostname, jobstr, projectpath, block, reconstruction->GetId(), production->GetId(), options.tiles_);
            auto& taskinfo = block->GetTaskInfoMutual();

            blk_reconst_production_info_s info;
            info.id_ = production->GetId();
            info.options_ = options;
            info.name_ = production->GetName();
           
            auto recinfo = std::find_if(taskinfo.reconstructions_info_.begin(), taskinfo.reconstructions_info_.end(),
                [&](blk_recontruction_info_s a) { return reconstruction->GetId() == a.id_; });
            if (recinfo == taskinfo.reconstructions_info_.end())
            {
                return AI3D_FAILURE;

            }

            auto proindfo = std::find_if(recinfo->production_infos_.begin(), recinfo->production_infos_.end(),
                [&](blk_reconst_production_info_s a) { return production->GetId() == a.id_; });
            if (proindfo != recinfo->production_infos_.end())
            {
                *proindfo = info;
            }
            else
            {
                recinfo->production_infos_.push_back(info);
            }
            

            taskinfo.isSaved = false;

            production_id = production->GetId();
            
           
            
            
            return AI3D_SUCCESS;
        }

        void  ReconstructionCommandSet::GetProductionSetInformation(ProductionObject* production, std::vector<std::pair<std::string, std::string> >& infos,
            std::vector<std::string> &translated_infos)
        {
            auto options = production->GetOptions();
            std::string settings_str = production->GetOptions().settings_str_;
            std::cout << "---------settings_str_:" << settings_str << std::endl;
            rapidjson::Document doc;
            if (doc.Parse(settings_str.data()).HasParseError())
            {
                LOGE("Parse setting Json ERROR!");
                return;
            }
            std::pair<std::string, std::string> id_info = std::make_pair("Production ID", production->GetIDString());
            infos.push_back(id_info);
            if (BlockObject::isChineseVersion())
            {
                translated_infos.push_back("产品ID");
            }
            {
                
                infos.push_back(std::make_pair("Format", options.GetFormatString()));
                if (BlockObject::isChineseVersion())
                {
                    translated_infos.push_back("格式");
                }
            }
            
            std::pair<std::string, std::string> destination_info = std::make_pair("Destination", options.destination_);
            infos.push_back(destination_info);
            if (BlockObject::isChineseVersion())
            {
                translated_infos.push_back("输出目录");
            }
            
            auto& format_id = StringForProductionFormat.at(options.GetFormatString());
            if (PRODUCTION_MESH & format_id)
            {

                if (doc.HasMember("lod_type"))
                {
                    int type = doc["lod_type"].GetInt();
                    AI3D::CORE::mesh3d_lod_type_e lodtype = AI3D::CORE::mesh3d_lod_type_e(type);
                    std::string str = "";
                    if (lodtype == AI3D::CORE::mesh3d_lod_type_e::MESH3D_LOD_ADAPTIVETREE)
                    {
                        str = "Adaptive Tree";
                    }
                    else if (lodtype == AI3D::CORE::mesh3d_lod_type_e::MESH3D_LOD_QUADTREE)
                    {
                        str = "QuadTree";
                    }
                    infos.push_back(std::make_pair("Type of level of detail", str));
                    if (BlockObject::isChineseVersion())
                    {
                        translated_infos.push_back("LOD类型");
                    }
                    else {
                        translated_infos.push_back("LOD Type");
                    }
                }
                if (doc.HasMember("scope_mode"))
                {
                    int value = doc["scope_mode"].GetInt();
                    std::string str = "Tile-wise";
                    if (value == 1)
                    {
                        str = "Across-tile";
                    }
                    infos.push_back(std::make_pair("Scope of level of detail", str));
                    if (BlockObject::isChineseVersion())
                    {
                        translated_infos.push_back("LOD范围模式");
                    }
                    else {
                        translated_infos.push_back("LOD Range Mode");
                    }
                }

               
                if (doc.HasMember("tex_compression"))
                {
                    int value = doc["tex_compression"].GetInt();
                    std::string str = "75% quality JPEG";
                    if (value == 100)
                    {
                        str = "100% quality JPEG";
                    }
                    else if (value == 90)
                    {
                        str = "90% quality JPEG";
                    }
                    else if (value == 50)
                    {
                        str = "50% quality JPEG";
                    }
                    infos.push_back(std::make_pair("Texture compression quality", str));
                    if (BlockObject::isChineseVersion())
                    {
                        translated_infos.push_back("纹理压缩质量");
                    }
                    else {
                        translated_infos.push_back("Texture Compression Quality");
                    }
                }
                if (doc.HasMember("max_tex_size"))
                {
                    int value = doc["max_tex_size"].GetInt();
                    std::string str = std::to_string(value);

                    infos.push_back(std::make_pair("Maximum texture size", str));
                    if (BlockObject::isChineseVersion())
                    {
                        translated_infos.push_back("最大纹理尺寸");
                    }
                    else {
                        translated_infos.push_back("Maximum Texture Size");
                    }
                }
                if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
                {
                    if (doc.HasMember("tileoverlap_in_metersorunit"))
                    {
                        float value = doc["tileoverlap_in_metersorunit"].GetFloat();
                        std::string str = std::to_string(value);
                        infos.push_back(std::make_pair("Tile overlap in meters/units", str));
                        if (BlockObject::isChineseVersion())
                        {
                            translated_infos.push_back("块间重叠(米/单元)");
                        }
                    }

                    if (doc.HasMember("include_tex_maps"))
                    {
                        bool value = doc["include_tex_maps"].GetBool();
                        std::string str = value ? "True" : "False";
                        infos.push_back(std::make_pair("Include texture maps", str));
                        if (BlockObject::isChineseVersion())
                        {
                            translated_infos.push_back("输出纹理");
                        }
                        else {
                            translated_infos.push_back("Output Texture");
                        }
                    }
                    if (doc.HasMember("skirt_length"))
                    {

                        int value = doc["skirt_length"].GetFloat();
                        std::string str = std::to_string(value);
                        infos.push_back(std::make_pair("Skirt length in pixels", str));
                        if (BlockObject::isChineseVersion())
                        {
                            translated_infos.push_back("裙边");
                        }
                    }


                    if (doc.HasMember("texture_sharpening"))
                    {
                        bool value = doc["texture_sharpening"].GetBool();
                        std::string str = value ? "True" : "False";
                        infos.push_back(std::make_pair("Texture sharpening", str));
                        if (BlockObject::isChineseVersion())
                        {
                            translated_infos.push_back("纹理锐化");
                        }
                        else {
                            translated_infos.push_back("Texture Sharpening");
                        }
                    }
                }
            }
          
            {
                if (PRODUCTION_POINTCLOUD & format_id)
                {
                    if (doc.HasMember("point_sampling_distance"))
                    {
                        float value = doc["point_sampling_distance"].GetFloat();
                        std::string str = std::to_string(value);
                        infos.push_back(std::make_pair("Point sampling distance", str));
                        if (BlockObject::isChineseVersion())
                        {
                            translated_infos.push_back("点采样距离");
                        }
                    }

                    if (doc.HasMember("point_sampling_unit"))
                    {
                        int value = doc["point_sampling_unit"].GetInt();
                        std::string str = value == 0 ? "pixel" : "meter";
                        infos.push_back(std::make_pair("Point sampling unit", str));
                        if (BlockObject::isChineseVersion())
                        {
                            translated_infos.push_back("点采样单位");
                        }
                    }
                }
            }
            if (PRODUCTION_4D & format_id)
            {
                
                if (doc.HasMember("with_tdom"))
                {
                    auto value = doc["with_tdom"].GetBool();
                    std::string str = value ? "True" : "False";
                    infos.push_back(std::make_pair("Orthophoto enabled", str));
                    if (BlockObject::isChineseVersion())
                    {
                        translated_infos.push_back("输出TDOM");
                    }
                    if (doc.HasMember("tdom_format"))
                    {
                        auto value = doc["tdom_format"].GetInt();
                        std::string str;
                        if (value == AI3D::CORE::tdom_format_e::TDOM_FORMAT_TIFFGEOTIFF)
                            str = "TIFF/GeoTIFF";
                        else if (value == AI3D::CORE::tdom_format_e::TDOM_FORMAT_JPEG)
                            str = "JPEG";
                        else
                        {
                            
                        }
                        infos.push_back(std::make_pair("Orthophoto Format", str));
                        if (BlockObject::isChineseVersion())
                        {
                            translated_infos.push_back("TDOM格式");
                        }
                    }
                }
                if (doc.HasMember("sampling_distance"))
                {
                    float value = doc["sampling_distance"].GetFloat();
                    std::string str = std::to_string(value);
                    infos.push_back(std::make_pair("Sampling distance", str));
                    if (BlockObject::isChineseVersion())
                    {
                        translated_infos.push_back("采样距离");
                    }
                }
                if (doc.HasMember("with_dsm"))
                {
                    auto value = doc["with_dsm"].GetBool();

                    std::string str = value ? "True" : "False";
                    infos.push_back(std::make_pair("DSM enabled", str));
                    if (BlockObject::isChineseVersion())
                    {
                        translated_infos.push_back("输出DSM");
                    }
                    {
                        auto value = doc["dsm_format"].GetInt();
                        std::string str;
                        if (value == AI3D::CORE::dsm_format_e::DSM_FORMAT_TIFFGEOTIFF)
                            str = "TIFF/GeoTIFF";
                        else if (value == AI3D::CORE::dsm_format_e::DSM_FORMAT_XYZ)
                            str = "XYZ";
                        else
                        {
                            
                        }
                        infos.push_back(std::make_pair("DSM Format", str));
                        if (BlockObject::isChineseVersion())
                        {
                            translated_infos.push_back("DSM格式");
                        }
                    }
                }
               
            }

            if (doc.HasMember("srs_definition"))
            {
                auto value = doc["srs_definition"].GetString();
                auto srs_temp = CoordinateDescriptor::GetSRSFromDefinition(value);
                if (srs_temp.type != LOCAL)
                {
                    std::string str = value;
                    infos.push_back(std::make_pair("Spatial Reference System", str));
                    if (BlockObject::isChineseVersion())
                    {
                        translated_infos.push_back("输出坐标系");
                    }
                }
                
            }
            if (doc.HasMember("coordinate_origin"))
            {
                auto value = doc["coordinate_origin"].GetArray();
                if (value.Size() != 3)
                {
                    return;
                }
                Eigen::Vector3d xyz;
                xyz.x()= value[0].GetDouble();
                xyz.y() = value[1].GetDouble();
                xyz.z() = value[2].GetDouble();
                std::string str = std::to_string(xyz.x()) + "," + std::to_string(xyz.y()) + "," + std::to_string(xyz.z());
               
                infos.push_back(std::make_pair("Origin", str));
                if (BlockObject::isChineseVersion())
                {
                    translated_infos.push_back("原点");
                }
            }

           


        }

        void ReconstructionCommandSet::GetProductionSetInformation(ProductionObject* production, std::string& productionid,
            std::string& destination, std::string& srs_str, std::string& originstr)
        {
            productionid = production->GetIDString();
            auto options = production->GetOptions();
            destination = options.destination_;
            srs_str = options.cs_.definition_;
            auto xyz = options.cs_.origin_;
            originstr =std::to_string(xyz.x())+","+ std::to_string(xyz.y()) + ","+ std::to_string(xyz.z()) ;

            return ;
        }

         Mesh3DPurpose::mesh3d_formatoptions_s ReconstructionCommandSet::ResetMeshFormat(const mesh3d_format_e& format)
        {
            return Mesh3DPurpose::FormatOptions(format);
        }


        

        
        void ReconstructionCommandSet::InitProductionDefinitionName(ReconstructionObject* reconstruction, production_option_s& options)
        {
            int produtionsnum = reconstruction->GenerateValidProductionId();
            options.id_ = produtionsnum;
            options.name_ = PRODUCTION_PREFIX + std::to_string(produtionsnum);
        }
        

    }
}






       
