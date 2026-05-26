
#include "Core/Tiling.h"
#include "Core/TilingImpl.h"
#include "Core/ReconstructionOptions.h"
#include "Util/TaskProcess.h"
#include "Core/Application.h"

#define MAXRAMLIMIT 16
namespace AI3D
{
    namespace CORE
    {
       

        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        int CutBlockForTile(const ATData& data, tile_info_s& tile)
        {
           
            auto& g_images = data.GetImages();
            auto& g_points = data.GetPoints3D();
            auto g_cameras = data.GetCameras();

            if (g_images.empty() || g_points.empty() || g_cameras.empty())
                return AI3D_FAILURE;            
    
            std::vector<point3D_t> belong_ids(g_points.size(), -1);
            point3D_t ptcount = 0;
            for (auto point : g_points)
            {
                auto v = point.second.GetXYZ();

                if (tile.bb_.contains(v.cast<float>()))
                {
                    tile.point_ids_.insert(point.first);
                    for (auto& obs : point.second.GetTrack().GetElements())
                    {
                        tile.image_ids_.insert(obs.image_id);
                    }
                }

                ptcount++;
            }
         
            {
                std::vector<std::pair<int, int>> image_size;
                const int tilePointThreadHold = Application::Getinstance().ParseConfig().tile_point_threshold;
                if (tile.image_ids_.empty() || tile.point_ids_.empty() || tile.point_ids_.size() < tilePointThreadHold)
                {
                    tile.isempty = true;
                  
                }
                for (auto imgid : tile.image_ids_)
                {
                    auto image = data.GetImages().at(imgid);
                    image_size.emplace_back(image.GetWidth(), image.GetHeight());
                }
                if (image_size.empty())
                {
                    tile.isempty = true;
                  
                }
                if (!tile.isempty)
                {
                    float ram = GetExpectedMaxRamUsageForAJob(image_size);
                    if (ram <= 1e-6)
                    {
                        tile.isempty = true;
                      
                    }
                    else
                    {
                        tile.ram_estimated_ = ram;
                    }
                    
                }

            }
           
            return AI3D_SUCCESS;
          
        }
       
        float ExtimateTilesize(const ATData& data, const ABBox3d& box, float& ram)
        {

            auto definition_temp = data.GetLocalSrs();
            auto srs_temp = CoordinateDescriptor::GetSRSFromDefinition(definition_temp);
            double start_size = srs_temp.type == GEOGRAPHIC ? 0.0015 : 250;
            double step_size = srs_temp.type == GEOGRAPHIC ? 0.00025 : 25;
            auto lengthv = (box.max() - box.min());
            double len = std::max(lengthv.x(), std::max(lengthv.y(), lengthv.z()));
            start_size = len < start_size ? (int)(len * 0.5) : start_size;
            double a = start_size;
            float estimateram = ram;
           
         
            double max_memory = MAXRAMLIMIT;
            double max_memory_of_tiles = 0;
           
            double defaultsize = a;
            double step_size_raw = step_size;
            int loop = 0;
            std::vector<double> ramvec;
            clock_t t1, t2, t3;
            bool isDownstep = true;
            bool decrease = false;
            ABBox3f global_bbox = box.cast<float>();
            TilingImpl::params_s tilingparams(a, ex_dividemode_e::REGULAR_PLANAR_GRID);
            


            TilingImpl impl(data, global_bbox, tilingparams);
            do {
                auto tilesize = a;
               
                impl.GetParamsMutual().tile_size_ = tilesize;
                impl.Run(ram);
                max_memory_of_tiles = 0;

                EIGEN_STL_UMAP(std::string, tile_info_s) _tiles;
                _tiles.clear();
                _tiles = impl.GetTilesInfo();

                
                {
                    max_memory_of_tiles = ram;
                }
                defaultsize = tilesize;
                if (max_memory_of_tiles < max_memory)
                {
                    
                    
                    
                    
                    
                    
                    
                    
                    {
                        break;
                    }
                }
                ramvec.push_back(ram);

                if(loop > 0 )
                {
                    double lastmem = ramvec[loop-1];
                    if (max_memory_of_tiles > max_memory * 2 && !decrease)
                    {
                        if (a >= 3 * step_size_raw)
                        {
                            step_size = 2 * step_size_raw;
                        }
                        else if (fabs(max_memory_of_tiles - lastmem) < max_memory)
                        {
                            step_size = 2 * step_size_raw;
                        }
                        else
                        {

                            step_size = step_size_raw;

                        }


                        if (1)
                        {
                            double atemp = a - step_size;
                            {

                                if (atemp <= step_size_raw * 1e-3)
                                {
                                    step_size = decrease ? step_size : step_size_raw;
                                    decrease = true;

                                }
                                if (decrease)
                                    step_size = 0.5 * step_size;
                            }
                        }
                    }
                    
                } 
                a -= step_size;
                
                loop++;
            } while (a > 0);

            
            LOGI("Estimate size " + std::to_string(defaultsize ) + ".ram is " + std::to_string(max_memory_of_tiles));
            LOGI("Estimate tile size loop count " + std::to_string(loop));
            
            return  int(defaultsize) * 1.0;
            

        }
       

        Tiling::Tiling(tiling_param_s param)
        {
            params_ = param;
        }
            
      
        int Tiling::GetNumTiles()
        {
            return tiles_.size();
        }
        const tiling_param_s& Tiling::GetParams() const
        {
            return params_;
        }
        tiling_param_s& Tiling::GetParamsMutual()
        {
            return params_;
        }
      
        const  EIGEN_STL_UMAP(std::string, tile_info_s)& Tiling::GetTilesInfo() const
        {
            return tiles_;
        }

       
            NoneModeTiling::NoneModeTiling(tiling_param_s params):Tiling()
            {
                params_ = params;
            }
            int NoneModeTiling::RunDefault(const ATData& data, const ABBox3d& points)
            {
              
                tile_info_s info;
                info.name_ = "Model";
                info.bb_ = points.cast<float>(); 
                int ret = CutBlockForTile(data, info );
                if (ret == AI3D_SUCCESS)
                {
                    params_.expected_max_ram_used_ = info.ram_estimated_;
                    tiles_["Model"] = info;
                    return AI3D_SUCCESS;
                }
                         
                return AI3D_FAILURE;
            };
            
            int NoneModeTiling::Run(const ATData& data, const ABBox3d& points)
            {
                
                tile_info_s info;
                info.name_ = "Model";
                info.bb_ = points.cast<float>(); 
                int ret = CutBlockForTile(data, info);
                if (ret == AI3D_SUCCESS)
                {
                    params_.expected_max_ram_used_ = info.ram_estimated_;
                    tiles_["Model"] = info;
                    return AI3D_SUCCESS;
                }

                return AI3D_FAILURE;
            }

       
            Regular2DModeTiling::Regular2DModeTiling(tiling_param_s params) :Tiling()
            {
                params_ = params;
            }
            int  Regular2DModeTiling::RunDefault(const ATData& data, const ABBox3d& points)
            {
              
                ABBox3f global_bbox = points.cast<float>();

             
                unsigned long long ram_temp = MAXRAMLIMIT;
                float ram = float(ram_temp);
               
                clock_t t1, t2, t3;
                t1 = clock();
                float tilesize = ExtimateTilesize(data, global_bbox.cast<double>(), ram);
                if (tilesize <= 0.)
                {
                    return AI3D_FAILURE;
                }
                t2 = clock();
                t3 = t2 - t1;
                t3 *= 0.001;
                std::cout << "ExtimateTilesize tiling  run. " << t3 << std::endl;
                params_.regular_params_.tilesize_ = (float)tilesize;
                TilingImpl::params_s tilingparams(params_.regular_params_.tilesize_, ex_dividemode_e::REGULAR_PLANAR_GRID);
                TilingImpl impl(data, global_bbox, tilingparams);
                t1 = clock();
                impl.Run(params_.expected_max_ram_used_);

                tiles_ = impl.GetTilesInfo();
                t2 = clock();
                t3 = t2 - t1;
                t3 *= 0.001;
                std::cout << " tiling  run. " << t3 << std::endl;

                return AI3D_SUCCESS;
               
               
            };
            int Regular2DModeTiling::Run(const ATData& data, const ABBox3d& points)
            {
                
               

                
                {
                   
                    ABBox3f global_bbox = points.cast<float>();
                    if (params_.regular_params_.tilesize_ < 0)
                    {
                        return  RunDefault(data, points);
                    }
                    else
                    {

                        TilingImpl::params_s tilingparams(params_.regular_params_.tilesize_, ex_dividemode_e::REGULAR_PLANAR_GRID);
                        clock_t t1, t2, t3;
                        t1 = clock();

                        TilingImpl impl(data, global_bbox, tilingparams);
                        impl.Run(params_.expected_max_ram_used_);
                        tiles_ = impl.GetTilesInfo();

                        t2 = clock();
                        t3 = t2 - t1;
                        std::cout << t3 << " total run " << t3 * 0.001 << std::endl;

                    }
                }
                
                
                return AI3D_SUCCESS;
            }
          
            int Regular3DModeTiling::Run(const ATData& data, const ABBox3d& points)
            {
               
               
                if (params_.regular_params_.tilesize_ < 0)
                {
                    return  RunDefault(data, points);
                }
                else
                {
                  
                    ABBox3f global_bbox = points.cast<float>();
                   
                    TilingImpl::params_s tilingparams(params_.regular_params_.tilesize_, ex_dividemode_e::REGULAR_VOLUMETRIC_GRID);
                    TilingImpl impl(data, global_bbox, tilingparams);
                    impl.Run(params_.expected_max_ram_used_);
                    tiles_ = impl.GetTilesInfo();
                   
                }
              
                
                return AI3D_SUCCESS;
            }
            Regular3DModeTiling::Regular3DModeTiling(tiling_param_s params) :Tiling()
            {
                params_ = params;
            }
            int Regular3DModeTiling::RunDefault(const ATData& data, const ABBox3d& points)
            {
                
               
                ABBox3f global_bbox = points.cast<float>();

               
               
                unsigned long long ram_temp = MAXRAMLIMIT;
                float ram = float(ram_temp);
                clock_t t1, t2, t3;
                t1 = clock();

                float tilesize = ExtimateTilesize(data, global_bbox.cast<double>(), ram);
                t2 = clock();
                t3 = t2 - t1;
                t3 *= 0.001;
                std::cout << " tiling  ExtimateTilesize. " << t3 << std::endl;

                if (tilesize <= 0.)
                {
                    return AI3D_FAILURE;
                }
                params_.regular_params_.tilesize_ = (float)tilesize;
                TilingImpl::params_s tilingparams(params_.regular_params_.tilesize_, ex_dividemode_e::REGULAR_VOLUMETRIC_GRID);
                TilingImpl impl(data, global_bbox, tilingparams);
                t1 = clock();
                impl.Run(params_.expected_max_ram_used_);
                tiles_ = impl.GetTilesInfo();
                t2 = clock();
                t3 = t2 - t1;
                t3 *= 0.001;
                std::cout << " tiling  run. " << t3 << std::endl;
                return AI3D_SUCCESS;
            }
           

      
            AdaptiveModeTiling::AdaptiveModeTiling(tiling_param_s params) :Tiling()
            {
                params_ = params;
            }

            int AdaptiveModeTiling::Run(const ATData& data, const ABBox3d& points)
            {
             
               
              
                unsigned long long ram_temp = (params_.adaptive_params_.target_ram_used_*1024.0*1024.0*1024.0);
                TilingImpl::params_s tilingparams(ram_temp, 10, 1, 0);
               
                
                ABBox3f global_bbox= points.cast<float>();

                TilingImpl impl(data, global_bbox, tilingparams);
                impl.Run(params_.expected_max_ram_used_);
                tiles_ = impl.GetTilesInfo();

                

            
               
                return AI3D_SUCCESS;
            }

            int AdaptiveModeTiling::RunDefault(const ATData& data, const ABBox3d& points)
            {               
              
                unsigned long long ram_temp = MAXRAMLIMIT;
                
                params_.adaptive_params_.target_ram_used_ = (float)ram_temp;
                return Run(data, points);
              
            }

            AI3D_API Tiling* TilingGenaratorFactory(tiling_param_s param)
            {

                switch (param.mode_)
                {
                case tiling_mode_e::TILE_NONE:
                {
                    NoneModeTiling* pGenarator = new NoneModeTiling(param);
                    return pGenarator;
                }

                break;
                case tiling_mode_e::TILE_PALNAR_GRID:
                {
                    Regular2DModeTiling* pGenarator = new Regular2DModeTiling(param);
                    return pGenarator;
                }

                break;
                case tiling_mode_e::TILE_VOL_GRID:
                {
                    Regular3DModeTiling* pGenarator = new Regular3DModeTiling(param);
                    return pGenarator;
                }

                break;
                case tiling_mode_e::TILE_ADAPTIVE:
                {
                    AdaptiveModeTiling* pGenarator = new AdaptiveModeTiling(param);
                    return pGenarator;
                }

                break;

                }
                return nullptr;
            }
    }
   
}
