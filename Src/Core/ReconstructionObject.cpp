
#include <Constants.h>
#include <sstream>
#include <glog/logging.h>
#include <pugixml.hpp>
#include "Core/ATData.h"

#include "Core/Alignment.h"
#include "Core/Types.h"
#include "Core/ReturnCode.h"
#include "Core/String.h"

#include "Core/Rapidjson.h"
#include "Core/ReconstructionObject.h"
#include <Core/Logging.h>
#include "Core/ReconPerfLog.h"
#include "Core/CoordinateSystem.h"
#include "Core/BlockObject.h"
#include "Core/File.h"
#include <filesystem>
#include "Core/KML.h"
#include <ogr_geometry.h>
#include <ogrsf_frmts.h>
#include <geos.h>
#include <geos/operation/buffer/BufferOp.h>
namespace AI3D
{
    namespace CORE
    {
        
       void ReconstructionObject::CopyAll(const ReconstructionObject& object)
        {
            id_ = kInvalidReconstructionId;
            name_ = object.GetName();

            atdata_ = object.GetATData();
            atdata_custom_ = object.GetATDataCustom();

            srs_base_ = object.GetBaseSrs();
            srs_custom_ = object.GetCustomSrs();


            
            boundingbox_custom_ = object.GetBoundingBoxCustom();


            tiling_mode_e mode = object.GetTilingDiscriptor()->GetParams().mode_;
            tiling_discriptor_ = TilingGenaratorFactory(mode);
            tiling_discriptor_->GetParamsMutual() = object.GetTilingDiscriptor()->GetParams();

            
            tiles_custom_ = object.GetTilesCustom();
            


            
            boundary_custom_ = object.GetBoundaryCustom();



           
            
            constraint_custom_ = object.GetConstraintCustom();
            
            processing_settings_ = object.GetProcessingSettings();
            block_id_ = object.GetBlockId();

        }

       void ReconstructionObject::CopyBase(const ReconstructionObject& object)
       {
           id_ = kInvalidReconstructionId;
           name_ = object.GetName();

           atdata_ = object.GetATData();
           atdata_custom_ = object.GetATDataCustom();

           srs_base_ = object.GetBaseSrs();
           srs_custom_ = object.GetCustomSrs();


          
           boundingbox_custom_ = object.GetBoundingBoxCustom();


           tiling_mode_e mode = object.GetTilingDiscriptor()->GetParams().mode_;
           tiling_discriptor_ = TilingGenaratorFactory(mode);
           tiling_discriptor_->GetParamsMutual() = object.GetTilingDiscriptor()->GetParams();

         
           tiles_custom_ = object.GetTilesCustom();
           


         
           boundary_custom_ = object.GetBoundaryCustom();


           
         
           constraint_custom_.clear();

          
           
           processing_settings_ = object.GetProcessingSettings();
           block_id_ = object.GetBlockId();

       }

        ReconstructionObject::ReconstructionObject(const ReconstructionObject& object)
        {
            id_ = kInvalidReconstructionId;
            name_ = object.GetName();

            atdata_ = object.GetATData();
            atdata_custom_ = object.GetATDataCustom();

            srs_base_ = object.GetBaseSrs();
            srs_custom_ = object.GetCustomSrs();

           
           
            boundingbox_custom_ = object.GetBoundingBoxCustom();
           
          
            tiling_mode_e mode = object.GetTilingDiscriptor()->GetParams().mode_;
            tiling_discriptor_ = TilingGenaratorFactory(mode);
            tiling_discriptor_->GetParamsMutual() = object.GetTilingDiscriptor()->GetParams();
            
           
            tiles_custom_ = object.GetTilesCustom();
            
           
            constraint_custom_ = object.GetConstraintCustom();
           
           
            boundary_custom_ = object.GetBoundaryCustom();


              
           
            
            processing_settings_ = object.GetProcessingSettings();
            block_id_ = object.GetBlockId();
          
        }

        ReconstructionObject::ReconstructionObject(block_t blockid)
        {
            id_ = kInvalidReconstructionId;
            name_ = "";
         
            path_ = "";

            
          
            boundary_custom_.clear();


              

           
            constraint_custom_.clear();
     
            processing_settings_ = processing_settings_s();
            block_id_ = blockid;
        }

        ReconstructionObject::ReconstructionObject(const ATData& atdata, block_t blockid)
        {
            id_ = kInvalidReconstructionId;
            name_ = "";

            atdata_ = atdata;
            std::string definition;
            Eigen::Vector3d orgin;
            
            atdata_custom_ = atdata;
            
            

            srs_base_ = CoordinateDescriptor::GetSRSFromDefinition(atdata_.GetLocalSrs());
            srs_custom_ = CoordinateDescriptor::GetSRSFromDefinition(atdata_custom_.GetLocalSrs());;
            clock_t t1, t2, t3;
            t1 = clock();
            atdata_custom_.ComputeTileBoundingBox(bb_scope_e::BB_SCOPE_TIEPOINTS);
            t2 = clock();
            t3 = t2 - t1;
            std::cout <<  " ComputeTileBoundingbox " << t3 * 0.001 << std::endl;
            ABBox3d box = atdata_custom_.GetTileAABBBox().cast<double>();
            
            
            
            
           
            boundingbox_custom_ = box;
          
            tiling_param_s param;
            
            param.mode_ = TILE_PALNAR_GRID;
           
            tiling_discriptor_ = TilingGenaratorFactory(param.mode_);
            auto scene_length = box.max() - box.min();

            double max_length = std::max(scene_length.x(), std::max(scene_length.y(), scene_length.z()));
            max_length = max_length < 100. ? max_length : 100.;
            tiling_discriptor_->GetParamsMutual().regular_params_.tilesize_ = max_length;
            
            
            
            
            
            t1 = clock();
            
            GetTilingDiscriptorMutual()->Run(atdata_custom_, box);
            t2 = clock();
            t3 = t2 - t1;
            std::cout << " Reconstruct Run Tiling " << t3 * 0.001 << std::endl;
            tiles_custom_ = GetTilingDiscriptorMutual()->GetTilesInfo();
         
           
           OrderTiles();
            
          
           boundary_custom_.clear();


             

          
            constraint_custom_.clear();
            


            processing_settings_ = processing_settings_s();
            block_id_ = blockid;

        }

        ReconstructionObject::ReconstructionObject(ATData&& atdata, block_t blockid)
        {
            ReconPerfStage perf_ctor("SubmitReconstruction", "ReconstructionObject_ctor");
            id_ = kInvalidReconstructionId;
            name_ = "";

            atdata_ = std::move(atdata);
            atdata_custom_ = atdata_;

            srs_base_ = CoordinateDescriptor::GetSRSFromDefinition(atdata_.GetLocalSrs());
            srs_custom_ = CoordinateDescriptor::GetSRSFromDefinition(atdata_custom_.GetLocalSrs());;
            ABBox3d box;
            {
                ReconPerfStage perf_bbox("SubmitReconstruction", "ComputeTileBoundingBox");
                atdata_custom_.ComputeTileBoundingBox(bb_scope_e::BB_SCOPE_TIEPOINTS);
                box = atdata_custom_.GetTileAABBBox().cast<double>();
            }

            boundingbox_custom_ = box;

            tiling_param_s param;

            param.mode_ = TILE_PALNAR_GRID;

            tiling_discriptor_ = TilingGenaratorFactory(param.mode_);
            auto scene_length = box.max() - box.min();

            double max_length = std::max(scene_length.x(), std::max(scene_length.y(), scene_length.z()));
            max_length = max_length < 100. ? max_length : 100.;
            tiling_discriptor_->GetParamsMutual().regular_params_.tilesize_ = max_length;

            {
                ReconPerfStage perf_point_views("SubmitReconstruction", "GeneratePointViewsForTiling");
                atdata_custom_.GeneratePointViews();
            }
            {
                ReconPerfStage perf_tiling("SubmitReconstruction", "InitialRunTiling");
                GetTilingDiscriptorMutual()->Run(atdata_custom_, box);
                ReconPerfLog(String::StringPrintf(
                    "[ReconPerf] SubmitReconstruction | InitialRunTiling | tiles=%zu tiling_points=%zu",
                    GetTilingDiscriptorMutual()->GetTilesInfo().size(),
                    atdata_custom_.GetPointsIDsTiling().size()));
            }
            tiles_custom_ = GetTilingDiscriptorMutual()->GetTilesInfo();

           OrderTiles();

           boundary_custom_.clear();

            constraint_custom_.clear();

            processing_settings_ = processing_settings_s();
            block_id_ = blockid;

        }
        ReconstructionObject::~ReconstructionObject()
        {
            boundary_custom_.clear();


            constraint_custom_.clear();

            tiles_custom_.clear();

            for (auto& block_ptr : productions_)
            {
                delete block_ptr.second;
            }
            if (tiling_discriptor_)
            {
                delete tiling_discriptor_;
            }
            tiling_discriptor_ = nullptr;
        }
        
        void ReconstructionObject::SetId(const reconstruction_t& id)
        {
            id_ = id;

        }
        bool ReconstructionObject::HasProductions()
        {
            return !productions_.empty();
        }

        const reconstruction_t& ReconstructionObject::GetId() const
        {
            return id_;

        }
        reconstruction_t& ReconstructionObject::GetIdMutual()
        {
            return  id_;
        }

        void ReconstructionObject::GetCustomTilingSrs(std::string& definition, Eigen::Vector3d& origin)
        {
            std::string local_srs_definition_ = atdata_.GetLocalSrs();
            auto srs_src = CoordinateDescriptor::GetSRSFromDefinition(local_srs_definition_);
            
           

            if (atdata_.HasControlPoints())
            {
                auto gcps = atdata_.GetControlPoints();
                for (auto gcp : gcps)
                {
                    if (gcp.second.GetSrs().type != LOCAL)
                    {
                        definition = gcp.second.GetSrs().definition;
                        break;
                    }
                }


            }
            else
            {

                if (srs_src.type != PROJECTION && srs_src.type != LOCAL)
                {



                    auto srsbase = atdata_.GetDefaultEnuSRS();
                    Eigen::Vector2d lonlat = CoordinateDescriptor::GetLatLonFromENUDefinition(srsbase.definition);

                    auto lon = lonlat.y();
                    auto lat = lonlat.x();
                    const int lon_zone = 1 + floor((lon + 180) / 6);
                    double lon_0 = (3 + 6 * (lon_zone - 1) - 180) * M_PI / 180.0;
                    int code = int(32700 - round((45 + lat) / 90) * 100 + round((183 + lon) / 6));
                    definition = ("epsg:" + std::to_string(code));
                }
                else
                {
                    definition= local_srs_definition_;
                }
            }

            
            Eigen::Vector3d position_offset = Eigen::Vector3d::Zero();
            if (srs_src.type != LOCAL)
            {
                atdata_.ComputePositionOffsetByAvgCenter(position_offset);
                CoordinateTransformer::Transform(1, &position_offset[0],
                    &position_offset[1], &position_offset[2],
                    local_srs_definition_, definition);

                auto srs_temp = CoordinateDescriptor::GetSRSFromDefinition(definition);
                if (srs_temp.type == GEOGRAPHIC)
                {
                    position_offset.x() = int(position_offset.x() * 1e6) * 1e-6;
                    position_offset.y() = int(position_offset.y() * 1e6) * 1e-6;
                }
                else
                {
                    position_offset.x() = int(position_offset.x());
                    position_offset.y() = int(position_offset.y());

                }
            }
            position_offset.z() = 0.;

            origin = position_offset;
        }
        

        void ReconstructionObject::UpdateConstraint(const std::vector<constraint_info_s>& cinfos_touse)
        {
            auto& constraints = GetConstraintCustomMutual();
            constraints.insert(constraints.end(), cinfos_touse.begin(), cinfos_touse.end());
            
            
            
            for (int i=0; i< constraints.size();i++)
            {
                constraints[i].UpdatePolygonBoxes();
                constraints[i].UpdatePolgonNames(i);
            }
           
           
        }

        void  ReconstructionObject::RunTiling()
        {
            auto currentcustombox = GetBoundingBoxCustom();
            double bbXmin = currentcustombox.min().x();
            double bbXmax = currentcustombox.max().x();

            double bbYmin = currentcustombox.min().y();
            double bbYmax = currentcustombox.max().y();

            double bbZmin = currentcustombox.min().z();
            double bbZmax = currentcustombox.max().z();
            clock_t t1, t2, t3;
            t1 = clock();

            if (tiling_discriptor_->GetParams().mode_ == tiling_mode_e::TILE_ADAPTIVE)
            {
                GetATDataCustomMutual().GeneratePointViews();
           }
            tiling_discriptor_->Run(GetATDataCustom(), GetBoundingBoxCustomMutual());

            t2 = clock();
            t3 = t2 - t1;
            t3 *= 0.001;
            std::cout << " tiling  tiling_discriptor. " << t3 << std::endl;
            

           
            auto tiles_got = tiling_discriptor_->GetTilesInfo();
            EIGEN_STL_UMAP(std::string, tile_info_s)  tiles_tobeused;
           
            if (boundary_custom_.empty())
            {
                tiles_tobeused = tiles_got;
            }
            else
            {
                
               
                std::vector<OGRGeometry*> geoms;
                for (int i = 0; i < boundary_custom_.size(); i++)
                {
                    
                    OGRGeometry* geo = ToPolygon(boundary_custom_[i]);
                   
                    geoms.push_back(geo);
                 
                }
                

               
                for (auto& iter : tiles_got)
                {

                    OGRGeometry* boxgeometry = BoxToPolygon(iter.second.bb_.cast<double>());
                    for (const auto it1 : geoms)
                    {
                        if (it1->Intersects(boxgeometry))
                        {
                            tiles_tobeused[iter.first] = iter.second;
                           
                        }
                    }
                }                                                 
            }

            for (auto iter : tiles_tobeused)
            {

                iter.second.bb_.min().x() = iter.second.bb_.min().x() < bbXmin ? bbXmin : iter.second.bb_.min().x();
                iter.second.bb_.max().x() = iter.second.bb_.max().x() > bbXmax ? bbXmax : iter.second.bb_.max().x();

                iter.second.bb_.min().y() = iter.second.bb_.min().y() < bbYmin ? bbYmin : iter.second.bb_.min().y();
                iter.second.bb_.max().y() = iter.second.bb_.max().y() > bbYmax ? bbYmax : iter.second.bb_.max().y();

                iter.second.bb_.min().z() = iter.second.bb_.min().z() < bbZmin ? bbZmin : iter.second.bb_.min().z();
                iter.second.bb_.max().z() = iter.second.bb_.max().z() > bbZmax ? bbZmax : iter.second.bb_.max().z();


                tiles_tobeused[iter.first] = iter.second;
            }

            {
                const ATData& tile_at = GetATDataCustom();
                const auto& all_pts = tile_at.GetPoints3D();
                const int tilePointThreadHold = Application::Getinstance().ParseConfig().tile_point_threshold;
                for (auto& kv : tiles_tobeused)
                {
                    tile_info_s& tile = kv.second;
                    std::set<point3D_t> filtered;
                    for (point3D_t pid : tile.point_ids_)
                    {
                        auto itp = all_pts.find(pid);
                        if (itp == all_pts.end())
                            continue;
                        if (tile.bb_.contains(itp->second.GetXYZ().cast<float>()))
                            filtered.insert(pid);
                    }
                    tile.point_ids_.swap(filtered);
                    tile.isempty = tile.image_ids_.empty() || tile.point_ids_.empty()
                        || tile.point_ids_.size() < static_cast<size_t>(tilePointThreadHold);
                }
            }

            t1 = clock();
            SetTilesCustom(tiles_tobeused);
            
            
            OrderTiles();
            t2 = clock();
            t3 = t2 - t1;
            t3 *= 0.001;
            std::cout << " tiling  tiling_discriptor post. " << t3 << std::endl;
        }

        void ReconstructionObject::SetName(std::string name)
        {
            name_ = name;
        }
        void ReconstructionObject::ReName(std::string name)
        {
            name_ = name;
        }

        const std::string& ReconstructionObject::GetName() const
        {
            return name_;
        }

        std::string& ReconstructionObject::GetNameMutual()
        {
            return name_;
        }

     
        const ABBox3d ReconstructionObject::ComputeGlobalBoxCustom()
        {
            bool imagechanged, tiepointchanged, gcpchanged;
            atdata_custom_.GetBoundingBox(imagechanged, tiepointchanged, gcpchanged);
            auto atbox = atdata_custom_.GetBox();
            auto anotherbox = boundingbox_custom_;
            if (!boundary_custom_.empty())
            {
                ABBox2d newbox;
               
                for (int i = 0; i < boundary_custom_.size(); i++)
                {
                   
                    for (int j = 0; j < boundary_custom_[i].size(); j++)
                    {

                        newbox.extend(boundary_custom_[i][j]);
                       
                    }
                    
                }
               
                anotherbox.min() = Eigen::Vector3d{ newbox.min().x(),newbox.min().y() ,boundingbox_custom_.min().z() };
                anotherbox.max() = Eigen::Vector3d{ newbox.max().x(),newbox.max().y() ,boundingbox_custom_.max().z() };
            }
            ABBox3d maxbox;
            maxbox.extend(anotherbox);
            maxbox.extend(atbox);
       
            return maxbox;
        }

        void ReconstructionObject::SetPath(std::string& path)
        {
            path_ = path;
        }

        std::string ReconstructionObject::GetPath()
        {
            return path_;
        }

        void  ReconstructionObject::SetBaseSrs(const srs_s& srs)
        {
            srs_base_ = srs;
        }
        const srs_s& ReconstructionObject::GetBaseSrs() const
        {
            return srs_base_;
        }
        srs_s& ReconstructionObject::GetBaseSrsMutual()
        {

            return srs_base_;
        }

        ProductionObject* ReconstructionObject::GetProductionMutual(production_t id)
        {
            return productions_[id];
        }

        ProductionObject* ReconstructionObject::GetProduction(production_t id)
        {
            for (auto it : productions_)
            {
                if (it.first == id)
                {
                    return it.second;
                }

            }
            return nullptr;
        }

        void  ReconstructionObject::SetCustomSrs(const srs_s& srs)
        {
            srs_custom_ = srs;
        }
        const srs_s& ReconstructionObject::GetCustomSrs() const
        {
            return srs_custom_;
        }
        srs_s& ReconstructionObject::GetCustomSrsMutual()
        {
            return srs_custom_;
        }

        
       

        void ReconstructionObject::SetBoundingBoxCustom(const ABBox3d& bb)
        {
            boundingbox_custom_ = bb;
        }
        const ABBox3d& ReconstructionObject::GetBoundingBoxCustom() const
        {
            return boundingbox_custom_;
        }
        ABBox3d& ReconstructionObject::GetBoundingBoxCustomMutual()
        {
            return boundingbox_custom_;
        }

       

        void ReconstructionObject::SetBoundaryCustom(const std::vector < std::vector<Eigen::Vector2d>>& bb)
        {
            boundary_custom_ = bb;
        }
        const std::vector < std::vector<Eigen::Vector2d>>& ReconstructionObject::GetBoundaryCustom() const
        {
            return boundary_custom_;
        }
        std::vector < std::vector<Eigen::Vector2d>>& ReconstructionObject::GetBoundaryCustomMutual()
        {
            return boundary_custom_;
        }
        bool ReconstructionObject::HasConstraints() const
        {
            return !constraint_custom_.empty();
        }


        bool ReconstructionObject::HasBoundary()
        {
            return !boundary_custom_.empty();
        }
        
        int ReconstructionObject::LoadBoundaryExternKml(const std::string& file)
        {
            return AI3D_SUCCESS;
        }
        int ReconstructionObject::SaveBoundaryExternKml(const std::string& file)
        {
            return AI3D_SUCCESS;
        }

       

        void ReconstructionObject::SetConstraintCustom(const std::vector<constraint_info_s>& bb)
        {
            constraint_custom_ = bb;
        }
        const std::vector<constraint_info_s>& ReconstructionObject::GetConstraintCustom() const
        {
            return constraint_custom_;
        }
        std::vector<constraint_info_s>& ReconstructionObject::GetConstraintCustomMutual()
        {
            return constraint_custom_;
        }

        int ReconstructionObject::GetNumTiles(bool discardempty)
        {
            int num = 0;
            if (!discardempty)
                return tiles_custom_.size();
            for (auto iter : tiles_custom_)
            {
                if (!iter.second.isempty)
                {
                    num++;
                }
            }
            return num;
        }

        
        int ReconstructionObject::StatisticsNumEmptyTiles()
        {
            return 1;
        }
        
        int ReconstructionObject::GetNumTiles()
        {
           
            return tiles_custom_.size();
        }
        

       

       

        production_t ReconstructionObject::GenerateValidProductionId()
        {
            production_t pro_id = kInvalidProductionId;

            std::set<production_t> pro_ids_;
            for (auto& it : productions_)
            {
                pro_ids_.insert(it.first);
            }
            if (!pro_ids_.empty())
            {
                pro_id = *pro_ids_.rbegin();
            }
            pro_id++;
           
            return pro_id;
        }
        bool ReconstructionObject::ExistProductionId(const production_t& id)
        {
            
            return productions_.find(id) != productions_.end();
        }

        bool ReconstructionObject::ExistsProduction(production_t pro_id)
        {
            
            return productions_.find(pro_id) != productions_.end();
        }

        void ReconstructionObject::SetProcessingSettings(const processing_settings_s& settings)
        {
            processing_settings_ = settings;
        }

        void ReconstructionObject::AddProduction(ProductionObject* object)
        {
            
            if (object->GetIdMutual() == kInvalidProductionId)
            {
                auto id = GenerateValidProductionId();
                object->GetIdMutual() = id;
            }
            if(object->GetName()=="")
            {
                std::string name = PRODUCTION_PREFIX + std::to_string(object->GetId());
                object->SetName(name);
            }
            if (object->GetPath() == "" && path_!="")
            {
                std::string recpath = path_;
              
                recpath = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(recpath)));
                recpath += PRODUCTION_DIR;
                recpath = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(recpath)));
                recpath += object->GetIDString();
                object->SetPath(recpath);
            }
            productions_[object->GetId()] = object;
        }

        bool ReconstructionObject::LoadGlobalConstraintFile(const std::string& xml_file_path)
        {
            pugi::xml_document doc;
            LOGI("*********************load_file************************");
            if (doc.load_file(xml_file_path.c_str()).status != pugi::xml_parse_status::status_ok)
            {
                LOGE(String::StringPrintf("Load XML file: %s error!", xml_file_path.c_str()));
                return false;
            }
            
            pugi::xml_node rcnode = doc.child("ReconstructionConstraints");
            if (!rcnode)
            {
                LOGE("No ReconstructionConstraints Root !");
                return false;
            }

            
            constraint_custom_.clear();
            pugi::xml_node scnode = rcnode.child("SurfaceConstraint");
            if (scnode)
            {
                
                while (scnode)
                {
            
               
                    ABBox3d box;
                    
                    constraint_info_s cinfo;
                    std::string cinfoname = "";
                    if (scnode.child("Name"))
                    {
                        std::string name = scnode.child("Name").text().as_string();
                        if (!name.empty())
                        {
                            cinfoname = name;
                        }                       
                    }

                    pugi::xml_node meshnode = scnode.child("Mesh");
                    if (meshnode)
                    {
                       
                        while (meshnode)
                        {
                            constraint_info_s::polygon_info_s polygon;
                            pugi::xml_node bbnode = meshnode.child("BoundingBox");
                            if (bbnode)
                            {
                                pugi::xml_node bbxminnode = bbnode.child("XMin");
                                pugi::xml_node bbxmaxnode = bbnode.child("XMax");
                                pugi::xml_node bbyminnode = bbnode.child("YMin");
                                pugi::xml_node bbymaxnode = bbnode.child("YMax");
                                pugi::xml_node bbzminnode = bbnode.child("ZMin");
                                pugi::xml_node bbzmaxnode = bbnode.child("ZMax");
                                if (bbxminnode && bbxmaxnode && bbyminnode && bbymaxnode && bbzminnode && bbzmaxnode)
                                {
                                    box.min().x() = bbxminnode.text().as_double();
                                    box.max().x() = bbxmaxnode.text().as_double();
                                    box.min().y() = bbyminnode.text().as_double();
                                    box.max().y() = bbymaxnode.text().as_double();
                                    box.min().z() = bbzminnode.text().as_double();
                                    box.max().z() = bbzmaxnode.text().as_double();
                                    polygon.box_ = box;
                                }
                            }

                            pugi::xml_node pathnode = meshnode.child("MeshPath");
                            
                            if (pathnode)
                            {
                                polygon.name_ = pathnode.text().as_string();
                                
                                std::string constraintpath = AI3D::CORE::File::GetParentDir(xml_file_path);

                                std::string constraintfile = constraintpath + "/" + polygon.name_ + POLYGONEXT;
                                constraintfile = AI3D::CORE::File::EnsureUnifySlash(constraintfile);
                                polygon.ParsePoints(constraintfile);
                            }

                            if (polygon.name_ != "" && !polygon.points_.empty())
                            {

                                cinfo.polygons_.push_back(polygon);

                            }
                            meshnode = meshnode.next_sibling();
                        }
                    }
                    if(!cinfo.polygons_.empty())
                        cinfo.name_ = cinfoname;
                    constraint_custom_.push_back(cinfo);
                    scnode = scnode.next_sibling();
                }
            }
            return true;
        }
        bool ReconstructionObject::DeleteProduction(production_t id)
        {
            std::cout << "inside " << __FILE__ << " " << __LINE__ << std::endl;
                if (!ExistsProduction(id))
                {
                    std::cout << "inside " << __FILE__ << " " << __LINE__ << std::endl;
                    return false;
                }
                ProductionObject  production = *productions_.at(id);
                std::string production_dir = production.GetPath();
                std::cout << "inside " << __FILE__ << " " << __LINE__ << std::endl;
                
                try
                {
                    if (std::filesystem::exists(File::BoostPathFromUtf8(production_dir)))
                    {
                        
                        CHECK_OPTION(File::Remove(production_dir));
                    }
                }
                catch (const std::filesystem::filesystem_error& fse)
                {
                    std::ostringstream oss;
                    oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                    std::cout << "inside " << __FILE__ << " " << __LINE__ << std::endl;
                    LOGI(oss.str());
                }
                catch (std::exception& ex)
                {
                    std::ostringstream oss;
                    oss << "exception:" << ex.what();
                    std::cout << "inside " << __FILE__ << " " << __LINE__ << std::endl;
                    LOGI(oss.str());
                }

                productions_.erase(id);
                std::cout << "inside " << __FILE__ << " " << __LINE__ << std::endl;
                return true;
            
        }

        bool ReconstructionObject::SaveGlobalConstraintFile(const std::string& xml_file_path)
        {
            
            pugi::xml_document doc;
            pugi::xml_node declaration_node = doc.append_child(pugi::node_declaration);

            declaration_node.append_attribute("version") = "1.0";
            declaration_node.append_attribute("encoding") = "utf-8";

            pugi::xml_node ReconstructionConstraintsnode = doc.append_child("ReconstructionConstraints");
            for (auto& iter : constraint_custom_)
            {
                pugi::xml_node SurfaceConstraintnode = ReconstructionConstraintsnode.append_child("SurfaceConstraint");
                pugi::xml_node namenode = SurfaceConstraintnode.append_child("Name");
                namenode.append_child(pugi::node_pcdata).set_value(iter.name_.c_str());
                for (auto& iter2 : iter.polygons_)
                {
                    pugi::xml_node Meshnode = SurfaceConstraintnode.append_child("Mesh");
                    pugi::xml_node meshpathnode = Meshnode.append_child("MeshPath");
                    meshpathnode.append_child(pugi::node_pcdata).set_value((iter2.name_).c_str());
                    pugi::xml_node meshbb = Meshnode.append_child("BoundingBox");
                   
                    auto box = iter2.box_;
                    pugi::xml_node meshxminbb = meshbb.append_child("XMin");
                    meshxminbb.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(box.min().x(),9).c_str());
                    pugi::xml_node meshxmaxbb = meshbb.append_child("XMax");
                    meshxmaxbb.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(box.max().x(), 9).c_str());
                    pugi::xml_node meshyminbb = meshbb.append_child("YMin");
                    meshyminbb.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(box.min().y(), 9).c_str());
                    pugi::xml_node meshymaxbb = meshbb.append_child("YMax");
                    meshymaxbb.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(box.max().y(), 9).c_str());
                    pugi::xml_node meshzminbb = meshbb.append_child("ZMin");
                    meshzminbb.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(box.min().z(), 9).c_str());
                    pugi::xml_node meshzmaxbb = meshbb.append_child("ZMax");
                    meshzmaxbb.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(box.max().z(), 9).c_str());
                }
            }
            bool saveSucceed = doc.save_file(xml_file_path.c_str());
            if (!saveSucceed)
            {
                LOG(ERROR) << "saving xml failed！";
                return false;
            }

            return true;
        }

        
        
        bool ReconstructionObject::SaveConstraints()
        {
           

           
            std::string constraintpath = path_ + "/Constraint";
            constraintpath = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(constraintpath)));
           
            File::MakeDirEmpty(constraintpath);
            File::CreateDirIfNotExists(constraintpath);
            std::string  contraintbasefilename = constraintpath + "constraint.xml";
            
            SaveGlobalConstraintFile(contraintbasefilename);
            for (int i=0;i<constraint_custom_.size();i++)
            {
                constraint_custom_[i].SavePolygons(constraintpath);
              
            }
            return true;
        }

        const std::set<std::string> ReconstructionObject::GetTilesName(bool bdiscardempty )
        {
            std::set<std::string> tileset;
            if (!bdiscardempty)
            {
                for (auto& iter : tiles_custom_)
                {
                   
                    {
                        tileset.insert(iter.first);
                    }
                }
            }
            else
            {
                for (auto& iter : tiles_custom_)
                {
                    if (!iter.second.isempty)
                    {
                        tileset.insert(iter.first);
                    }
                }
            }
            return tileset;
        }

        void ReconstructionObject::WriteTiles(const std::string& path)
        {

            
            if (!constraint_custom_.empty())
            {

                SaveConstraints();
            }
            else
            {
                std::string constraintpath = path_ + "/Constraint";
                constraintpath = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(constraintpath)));

                File::Remove(constraintpath);
            }

            

            bool shoulddeletedir = true;
            
            bool hasproduction = HasProductions();
            
            auto dirs = File::GetDirList(path);
            

            auto tileset = GetTilesName(GetProcessingSettings().bdiscard_emptytiles_);
            int tiledircount = dirs.size();
            int atfilescount = 0;
            for (auto& dir : dirs)
            {
                
                std::string dirname = File::GetDirName(dir);
                if (strstr(dirname.c_str(), "Constraint") != NULL)
                {
                    tiledircount--;
                }
                if (strstr(dirname.c_str(), "Productions") != NULL)
                {
                    tiledircount--;
                }
                if (!tileset.count(dirname))
                {
                    continue;
                }

                if (!tiles_custom_.count(dirname))
                    continue;
                std::string atfile = dir + "/"+ PRODUCTIONVIEWIDSBIN;
                File::EnsureUnifySlash(atfile);
                if (File::ExistsPath(dir) && File::ExistsFile(atfile))
                {
                    atfilescount++;
                }
            }

            if (hasproduction && (atfilescount == tileset.size())&& tiledircount == atfilescount)
            {
                shoulddeletedir = false;
            }
            
            if (shoulddeletedir)
            {
                for (auto& dir : dirs)
                {
                    String::StringToLower(&dir);

                    if (strstr(dir.c_str(), "model") != NULL ||
                        strstr(dir.c_str(), "tile") != NULL)
                    {
                        File::Remove(dir);
                    }
                }



                

               auto config = Application::Getinstance().ParseConfig();
               bool to_gs = config.to_gs;
                std::vector<std::string> tilevec(tileset.begin(), tileset.end());
                if (to_gs)
                {
                    int cnt = 0;
                    std::map<image_t, std::set<std::string>> images_in_tiles;
#ifdef USE_OPENMP
#pragma omp parallel  for
#endif
                    for (int index = 0; index < tilevec.size(); index++)
                    {
                        auto iter = tilevec[index];
                        if (!tiles_custom_.count(iter))
                            continue;


                        auto tile = tiles_custom_.at(iter);
                        if (tile.image_ids_.empty())
                        {
                            continue;
                        }
                        std::string dir = path + "/" + iter;
                        File::CreateDirIfNotExists(dir, true);

                        std::string atfile = dir + "/" + PRODUCTIONVIEWIDSBIN;
                       
                        atfile = File::EnsureUnifySlash(atfile);

                        {
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            std::ofstream out = File::OpenOfstreamUtf8(atfile, std::ios::binary);
                            if (!out.is_open()) {
                                LOGE("Save SCSFR bin failed!");
                                continue;
                            }
                            ReViewsBinFile reViewsBinFile;
                            reViewsBinFile.num = tile.image_ids_.size();
                            reViewsBinFile.image_ids_.clear();
                            for (auto& iterid : tile.image_ids_)
                            {
                                reViewsBinFile.image_ids_.push_back(iterid);
                                
                            }

                            reViewsBinFile.Serialize(out);

                            out.close();
                        }                       
                        
                        ATData data;
                        std::set<point3D_t> ids;
                        if (tile.image_ids_.empty())
                        {
                            continue;
                        }

                        if (tile.point_ids_.empty())
                        {
                            
                            atdata_.ExtractATDataByImages(tile.image_ids_, data);
                        }
                        else
                        {
                            atdata_.ExtractATDataByTiepoints(tile.point_ids_, data);
                        }

                        
                         AI3D::CORE::BlockObject block(dir);
                         block.SetId(cnt);

                         
                         
                         
                         
                         
                         

                         block.MakeBlockFromATData(data);
                         AI3D::CORE::BlockObject::BlockExportOptions opt;
                         opt.export_tiepoint_ = true;
                         opt.srs_ = block.GetBlockSRS();
                         opt.srs_.ID = 0;
                         opt.export_not_registered_ = false;
                         opt.export_controlpoint_ = false;
                         std::string atxmlfile = dir + "/"+ tile.name_+".xml";
                         block.ExportATXML(atxmlfile, opt);
                         
#ifdef USE_OPENMP
#pragma omp critical
#endif
                        {
                            cnt++;
                            for (auto& imgid : tile.image_ids_)
                            {
                                images_in_tiles[imgid].insert(tile.name_);
                            }
                        }
                    }
                    


                
                    {

                        std::ofstream txtfile = File::OpenOfstreamUtf8(path + "images_vs_tiles.txt", std::ios::trunc);
                      
                        for (auto& iter1 : images_in_tiles)
                        {
                            txtfile << iter1.first << " " << iter1.second.size();
                            for (auto& iter2 : iter1.second)
                            {

                                txtfile << " " << iter2;
                            }
                            txtfile << std::endl;;
                        }
                        txtfile.close();
                    }
                }
                else
                {
                    
#ifdef USE_OPENMP
#pragma omp parallel  for
#endif
                    for (int index = 0; index < tilevec.size(); index++)
                    {
                        auto iter = tilevec[index];
                        if (!tiles_custom_.count(iter))
                            continue;


                        auto tile = tiles_custom_.at(iter);
                        if (tile.image_ids_.empty())
                        {
                            continue;
                        }
                        std::string dir = path + "/" + iter;
                        File::CreateDirIfNotExists(dir, true);

                        std::string atfile = dir + "/" + PRODUCTIONVIEWIDSBIN;

                        File::EnsureUnifySlash(atfile);
                        {
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            std::ofstream out = File::OpenOfstreamUtf8(atfile, std::ios::binary);
                            if (!out.is_open()) {
                                LOGE("Save SCSFR bin failed!");
                                continue;
                            }
                            ReViewsBinFile reViewsBinFile;
                            reViewsBinFile.num = tile.image_ids_.size();
                            reViewsBinFile.image_ids_.clear();
                            for (auto& iterid : tile.image_ids_)
                            {
                                reViewsBinFile.image_ids_.push_back(iterid);
                                
                            }

                            reViewsBinFile.Serialize(out);

                            out.close();
                        }
                        
                        std::string inputDir = dir + "/PCloudGS/";
                        File::CreateDirIfNotExists(inputDir, true);
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        std::string bboxCutFile = inputDir + TILEBBOXCUT;
                        bboxCutFile = File::EnsureUnifySlash(bboxCutFile);
                        
                        std::string bboxFile = inputDir + TILEBBOX;
                        bboxFile = File::EnsureUnifySlash(bboxFile);
                        {
                            std::ofstream obbox = File::OpenOfstreamUtf8(bboxFile, std::ios::out | std::ios::app);
                            std::ofstream obboxCut = File::OpenOfstreamUtf8(bboxCutFile, std::ios::out | std::ios::app);
                            if (!obbox.is_open() || (!obboxCut.is_open()))
                            {
                                
                                continue;
                            }
                            
                            bbox_s bb(tile.bb_.cast<double>());
                            obboxCut << std::setprecision(16) << bb.xmin_ << "," << bb.ymin_ << ",0" << std::endl;
                            obboxCut << std::setprecision(16) << bb.xmin_ << "," << bb.ymax_ << ",0" << std::endl;
                            obboxCut << std::setprecision(16) << bb.xmax_ << "," << bb.ymax_ << ",0" << std::endl;
                            obboxCut << std::setprecision(16) << bb.xmax_ << "," << bb.ymin_ << ",0" << std::endl;

                            double ratio = 0.1;
                            double deltaX = (bb.xmax_ - bb.xmin_) * ratio;
                            double deltaY = (bb.ymax_ - bb.ymin_) * ratio;
                            double newMinX = bb.xmin_ - deltaX;
                            double newMaxX = bb.xmax_ + deltaX;
                            double newMinY = bb.ymin_ - deltaY;
                            double newMaxY = bb.ymax_ + deltaY;
                            obbox << std::setprecision(16) << newMinX << "," << newMinY << ",0" << std::endl;
                            obbox << std::setprecision(16) << newMinX << "," << newMaxY << ",0" << std::endl;
                            obbox << std::setprecision(16) << newMaxX << "," << newMaxY << ",0" << std::endl;
                            obbox << std::setprecision(16) << newMaxX << "," << newMinY << ",0" << std::endl;

                            obbox.close();
                            obboxCut.close();

                        }
                        
                    

                    }
                }
                
            }
        }

        std::string ReconstructionObject::GetIDString()
        {
            return RECONSTRUCT_PREFIX + std::to_string(id_);
        }
        
        Tiling* ReconstructionObject::GetTilingDiscriptorMutual()
        {
            return tiling_discriptor_;
        }
       const Tiling* ReconstructionObject::GetTilingDiscriptor() const
        {
            return tiling_discriptor_;
        }
        void ReconstructionObject::SetTilingDisriptor(Tiling* tile)
        {
         
            tiling_discriptor_ = tile;
        }

       


        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        

        void ReconstructionObject::TransformFromATDataCustomToBase()
        {
            atdata_ = atdata_custom_;
            atdata_.TransFormATData(srs_custom_.definition);
        }
        void ReconstructionObject::TransformFromATDataBaseToCustom()
        {
            atdata_custom_  = atdata_;
            atdata_custom_.TransFormATData(srs_base_.definition);
        }
     
       
       
        std::vector<std::string> ReconstructionObject::GetOrderedTiles()
        {
            typedef std::pair<std::string, tile_info_s> tilepair;
            std::vector<tilepair> vec(tiles_custom_.begin(), tiles_custom_.end());
           
            std::sort(vec.begin(), vec.end(), 
                [&](const std::pair<std::string, tile_info_s> tile1, std::pair<std::string, tile_info_s> tile2) {
                    return tile1.second.index_< tile2.second.index_; });
            std::vector<std::string> names(vec.size());
            for (int i = 0; i < vec.size(); i++)
            {
                names[i] = vec[i].first;
            }
            return names;
        }
       
        struct TileComp
        {
            bool operator()(const std::string& left,const std::string& right) const
            {

                if (left.empty())
                    return false;

                if (right.empty())
                    return false;

                std::string::size_type leftn = left.find('_');
                std::string::size_type rightn = right.find('_');
                std::vector<std::string> leftstrs = String::StringSplit(left,"_");
                std::vector<std::string> rightstrs = String::StringSplit(right, "_");
                if (leftstrs.size() != rightstrs.size())
                    return false;
                if (leftstrs.size() <= 1)
                {
                    return false;
                }
                for (int i = 1;i< leftstrs.size();i++)
                {
                    int nLeftNumber = std::atoi(leftstrs[i].c_str());;
                    int nRightNumber = std::atoi(rightstrs[i].c_str());;
                    if (nLeftNumber < nRightNumber)
                        return true;
                }
                

                return false;
            }
        };


        void ReconstructionObject::OrderTiles()
        {
            

            std::set<std::string, TileComp> names;
                      
            for (auto& iter : tiles_custom_)
            {
               
                auto  ret = names.insert(iter.second.name_);
                if (!ret.second)
                {
                    std::cout << "== " <<iter.second.name_ << std::endl;
                }
            }

            int count = 0;
            for (auto& iter : names)
            {
                count++;
               
               
                tiles_custom_[iter].index_ = count;
                
            }

        }

        

        std::vector<std::string > ReconstructionObject::GetTilesByStatus(production_t production_id, const jobsta_e& status)
        {
            auto production = GetProduction(production_id);
            return production->GetTilesByStatus(status);

        }
        bool ReconstructionObject::HasTiles() const
        {
            return !tiles_custom_.empty();
        }

       
        void ReconstructionObject::TransformFromConstraint(std::vector<constraint_info_s>& cinfos, const std::string& src_srs,const std::string& des_srs)
        {
            {
                std::vector<double> x, y, z;


                for (int idx_i = 0; idx_i < cinfos.size(); idx_i++)
                {
                    for (int idx_j = 0; idx_j < cinfos[idx_i].polygons_.size(); idx_j++)
                    {
                        for (int idx_k = 0; idx_k < cinfos[idx_i].polygons_[idx_j].points_.size(); idx_k++)
                        {
                            auto point = cinfos[idx_i].polygons_[idx_j].points_[idx_k];
                            x.push_back(point.x());
                            y.push_back(point.y());
                            z.push_back(point.z());
                        }
                    }
                }
                CoordinateTransformer::Transform(x.size(), &x[0], &y[0], &z[0], src_srs, des_srs);
                int pointnum = 0;
               
                for (int idx_i = 0; idx_i < cinfos.size(); idx_i++)
                {
                  
                    for (int idx_j = 0; idx_j < cinfos[idx_i].polygons_.size(); idx_j++)
                    {
                     
                        for (int idx_k = 0; idx_k < cinfos[idx_i].polygons_[idx_j].points_.size(); idx_k++)
                        {
                            
                            auto& point = cinfos[idx_i].polygons_[idx_j].points_[idx_k];
                           
                            point.x() = x[pointnum];
                            point.y() = y[pointnum];
                            point.z() = z[pointnum];
                            pointnum++;
                        }
                    }
                     
                }
            }
        }
       
        
       


        void ReconstructionObject::ToTaskInfo(blk_recontruction_info_s& info)
        {
            info.id_ = GetId();
            info.name_ = GetName();
            info.boundary_custom_ = GetBoundaryCustom();
            info.boundingbox_custom_ = GetBoundingBoxCustom();
           
            info.processing_settings_ = GetProcessingSettings();
            info.srs_custom_ = GetCustomSrs();
            info.tiles_ = tiles_custom_;
            info.tile_params_ = tiling_discriptor_->GetParams();
           
            int count = 0;
            for (auto iterproduction : productions_)
            {
                blk_reconst_production_info_s prodinfo;
                prodinfo.id_ = iterproduction.second->GetId();
                prodinfo.name_ = iterproduction.second->GetName();
                prodinfo.options_ = iterproduction.second->GetOptions();
 
                info.production_infos_.push_back(prodinfo);
               
                count++;
            }
        }

        void ReconstructionObject::SetWidget(void* pWidget)
        {
            this->pWidget = pWidget;
        }

        void* ReconstructionObject::GetWidget()
        {
            return this->pWidget;
        }
    }

}
