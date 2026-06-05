
#include "Core/TilingImpl.h"
#include <numeric>
#include "Core/BlockObject.h"
#include "Core/VectorFile.h"
#include "Core/KML.h" 
namespace AI3D
{
    namespace CORE
    {

        const int kDivisibleDimN = 3;
        const divisibleaxis kDivisibleDims[kDivisibleDimN] =
        { 
            divisibleaxis::X,divisibleaxis::Y,divisibleaxis::Z

        };

        int CutBlockForTileFrustum(std::map<int, std::string> indexes, divide_params_s& param, const ATData& data, EIGEN_STL_UMAP(std::string, tile_info_s)& tiles, float& maxram_tile)
        {
            if (tiles.empty())
                return AI3D_FAILURE;
            const int tilePointThreadHold = Application::Getinstance().ParseConfig().tile_point_threshold;
            auto g_images = data.GetImages();
           
            auto g_cameras = data.GetCameras();
            std::vector<Eigen::Vector3d> g_points;
            std::map<int, image_t> idx_to_id;
            int idxcount = 0;
            for (auto& iterimg : g_images)
            {
                auto image = iterimg.second;
                if (image.HasFrustum())
                {
                    g_points.insert(g_points.end(), image.GetFrustumMutual().begin(), image.GetFrustumMutual().end());
                    idx_to_id[idxcount] = image.GetImageId();
                    idxcount++;

                }
            }
            if (g_images.empty() || g_points.empty() || g_cameras.empty())
                return AI3D_FAILURE;

            int tilecount = 0;
            std::vector<std::string> tilenames(tiles.size());
            for (auto& tile : tiles)
            {
                tilenames[tilecount] = tile.first;
                tile.second.image_ids_.clear();
                tile.second.point_ids_.clear();
                tile.second.ram_estimated_ = 0.0;
                tilecount++;
            }


            auto paramdiv = param;
            std::vector<point3D_t> belong_ids(g_points.size(), -1);
            point3D_t ptcount = 0;
            for (auto iterid : idx_to_id)
            {
                int idx = iterid.first;
                image_t imgid = iterid.second;
                std::vector<Eigen::Vector3d> frustum(g_points.begin() + idx * 4, g_points.begin() + idx * 4 + 3);
                
              
                
                std::vector<point3D_t> fids(4);
                for (int idxy = 0; idxy < 4; idxy++)
                {
                    auto v = frustum[idxy];
                    if (paramdiv.divide_mode_ == ex_dividemode_e::REGULAR_PLANAR_GRID || paramdiv.divide_mode_ == ex_dividemode_e::REGULAR_VOLUMETRIC_GRID)
                    {
                        Eigen::Vector3d global_idx = (v - paramdiv.tile_origin_) / paramdiv.tile_size_;
                        for (int n = 0; n < 3; n++)
                        {
                            global_idx[n] = floor(global_idx[n]);

                        }


                        Eigen::Vector3i local_idx = global_idx.cast<int>() - paramdiv.tile_grid_.start;
                        point3D_t belong_id;
                        
                        if (paramdiv.divide_mode_ == ex_dividemode_e::REGULAR_PLANAR_GRID)
                        {
                            belong_id = local_idx[1] * paramdiv.tile_grid_.dims[0] + local_idx[0];
                        }
                        else
                        {
                            belong_id = local_idx[2] * paramdiv.tile_grid_.dims[0] * paramdiv.tile_grid_.dims[1]
                                + local_idx[1] * paramdiv.tile_grid_.dims[0] + local_idx[0];
                        }
                        fids[idxy] = belong_id;
                    }
                    bool bintile = false;
                    for (auto& belong_id : fids)
                    {

                        if (belong_id >= 0 && belong_id < tilenames.size())
                        {

                            if (indexes.count(belong_id))
                            {
                                bintile = true;

                                std::string name = indexes.at(belong_id);
                                tiles.at(name).image_ids_.insert(imgid);
                                 
                                  
                                break;
                            }
                        }
                    }

                }
            }



            float maxram = 0.f;
            for (int tilecnt = 0; tilecnt < tilenames.size(); tilecnt++)
            {
                auto& tile = tiles.at(tilenames[tilecnt]);

                std::vector<std::pair<int, int>> image_size;
                if (tile.image_ids_.empty())
                {
                    tile.isempty = true;
                    continue;
                }
                tile.point_ids_.clear();
                for (const auto& pt : data.GetPoints3D())
                {
                    if (tile.bb_.contains(pt.second.GetXYZ().cast<float>()))
                    {
                        tile.point_ids_.insert(pt.first);
                    }
                }
                if (tile.point_ids_.empty()
                    || tile.point_ids_.size() < static_cast<size_t>(tilePointThreadHold))
                {
                    tile.isempty = true;
                    continue;
                }
                for (auto imgid : tile.image_ids_)
                {
                    auto image = data.GetImages().at(imgid);
                    image_size.emplace_back(image.GetWidth(), image.GetHeight());
                }
                if (image_size.empty())
                {
                    tile.isempty = true;
                    continue;
                }
                if (!tile.isempty)
                {
                    float ram = GetExpectedMaxRamUsageForAJob(image_size);
                    if (ram <= 1e-6)
                    {
                        tile.isempty = true;
                        continue;
                    }
                    tile.ram_estimated_ = ram;
                    if (ram > maxram)
                    {
                        maxram = ram;
                    }
                }

            }
            maxram_tile = maxram;
            return AI3D_SUCCESS;

        }


        int CutBlockForTile(std::map<int, std::string> indexes,divide_params_s& param,const ATData& data, EIGEN_STL_UMAP(std::string, tile_info_s)& tiles, float& maxram_tile)
        {
            if (tiles.empty())
                return AI3D_FAILURE;
            const int tilePointThreadHold = Application::Getinstance().ParseConfig().tile_point_threshold;
            auto& g_images = data.GetImages();
            auto& g_points = data.GetPoints3D();
            auto g_cameras = data.GetCameras();

            if (g_images.empty() || g_points.empty() || g_cameras.empty())
                return AI3D_FAILURE;

            int tilecount = 0;
            std::vector<std::string> tilenames(tiles.size());
            for (auto& tile : tiles)
            {
               
                tilenames[tilecount] = tile.first;
                tilecount++;
            }
            std::vector<point3D_t> points_vec;
            const std::set<point3D_t>& tiling_point_ids = data.GetPointsIDsTiling();
            if (!tiling_point_ids.empty())
            {
                points_vec.reserve(tiling_point_ids.size());
                for (point3D_t ptid : tiling_point_ids)
                {
                    if (g_points.count(ptid))
                    {
                        points_vec.push_back(ptid);
                    }
                }
            }
            else
            {
                points_vec.reserve(g_points.size());
                for (const auto& point : g_points)
                {
                    points_vec.push_back(point.second.GetId());
                }
            }

            auto paramdiv = param;
            std::vector<int64_t> belong_ids(points_vec.size(), -1);
           
#ifdef USE_OPENMP
#pragma omp parallel  for
#endif
           
        
            for (int64_t i = 0; i < points_vec.size(); i++)
            {
               
                auto ptid = points_vec[i];
                auto point = g_points.at(ptid);
                auto v = point.GetXYZMutual();
                if (paramdiv.divide_mode_ == ex_dividemode_e::REGULAR_PLANAR_GRID || paramdiv.divide_mode_ == ex_dividemode_e::REGULAR_VOLUMETRIC_GRID)
                {
                    Eigen::Vector3d global_idx = (v - paramdiv.tile_origin_) / paramdiv.tile_size_;
                    for (int n = 0; n < 3; n++)
                    {
                        global_idx[n] = floor(global_idx[n]);

                    }


                    Eigen::Vector3i local_idx = global_idx.cast<int>() - paramdiv.tile_grid_.start;
                    int64_t belong_id;
                  
                    if (paramdiv.divide_mode_ == ex_dividemode_e::REGULAR_PLANAR_GRID)
                    {
                         belong_id = static_cast<int64_t>(local_idx[1]) * paramdiv.tile_grid_.dims[0] + local_idx[0];
                    }
                    else
                    {
                        belong_id = static_cast<int64_t>(local_idx[2]) * paramdiv.tile_grid_.dims[0] * paramdiv.tile_grid_.dims[1]
                            + static_cast<int64_t>(local_idx[1]) * paramdiv.tile_grid_.dims[0] + local_idx[0];
                    }

                    if (belong_id >= 0 && belong_id < static_cast<int64_t>(tilenames.size()))
                    {
                        
                        if (!indexes.count(belong_id))
                        {
                            continue;
                        }
                        belong_ids[i] = belong_id;
                    }
                }
               
                
            }
            for (size_t i = 0; i < points_vec.size(); ++i)
            {
                if (belong_ids[i] < 0)
                {
                    continue;
                }
                const point3D_t ptid = points_vec[i];
                const auto& point = g_points.at(ptid);
                const std::string name = indexes.at(static_cast<int>(belong_ids[i]));
                tiles.at(name).point_ids_.insert(ptid);
                for (const auto& obs : point.GetTrack().GetElements())
                {
                    tiles.at(name).image_ids_.insert(obs.image_id);
                }
            }
            float maxram = 0.f;
            for (int tilecnt = 0; tilecnt < tilenames.size(); tilecnt++)
            {
                auto& tile = tiles.at(tilenames[tilecnt]);
                
                std::vector<std::pair<int, int>> image_size;
                if (tile.image_ids_.empty() || tile.point_ids_.empty()
                    || tile.point_ids_.size() < static_cast<size_t>(tilePointThreadHold))
                {
                    tile.isempty = true;
                    continue;
                }
                for (auto imgid : tile.image_ids_)
                {
                    auto image = data.GetImages().at(imgid);
                    image_size.emplace_back(image.GetWidth(), image.GetHeight());
                }
                if (image_size.empty())
                {
                    tile.isempty = true;
                    continue;
                }
                if (!tile.isempty)
                {
                    float ram = GetExpectedMaxRamUsageForAJob(image_size);
                    if (ram <= 1e-6)
                    {
                        tile.isempty = true;
                        continue;
                    }
                    tile.ram_estimated_ = ram;
                    if (ram > maxram)
                    {
                        maxram = ram;
                    }
                    
                }

            }
            maxram_tile = maxram;
            return AI3D_SUCCESS;
         
        }


       
        static ABBox3f ExtendBoxToInfiniteInNonDivisibleDimensions(const ABBox3f& bbox)
        {
            ABBox3f extend_box = bbox;
            for (int dim = 0; dim < 3; ++dim)
            {
                bool is_non_divisibel_dim = true;
                for (auto divisible_dim : kDivisibleDims)
                {
                    if (dim == int(divisible_dim))
                    {
                        is_non_divisibel_dim = false;
                    }
                }
                if (is_non_divisibel_dim)
                {
                    extend_box.min()[dim] = -std::numeric_limits<float>::max();
                    extend_box.max()[dim] = std::numeric_limits<float>::max();
                }
            }
            return extend_box;
        }


        void TilingImpl::CutRecursive(std::vector<divisable_tile_info_s>& ret, 
             divisable_tile_info_s& block, int depth) const
        {
            if (block.point_count_ < 100 && block.pixel_count_ < 100000)
            {
                return;
            }
            std::vector<std::pair<int, int>> image_sizes;
            for (const auto& it : block.image_and_rect_)
            {
                image_sizes.emplace_back(it.second.width_, it.second.height_);
            }
            const unsigned long long  memory_needed = EstimateMemoryUsage(image_sizes, params_.resolution_level_);

            bool should_accept = memory_needed <= params_.usable_memory_in_bytes_ || depth >= params_.max_depth_;

            if (!should_accept)
            {
                std::pair<divisable_tile_info_s, divisable_tile_info_s> tile_pair;
                divisibleaxis cut_dim;
                if (FindBestCut(block, tile_pair, cut_dim))
                {
                    CutRecursive(ret,tile_pair.first,depth + 1);
                    CutRecursive(ret, tile_pair.second, depth + 1);
                }
                else
                {
                    should_accept = true;
                }
            }
            if (should_accept)
            {
                block.memory_need_cpu_ = memory_needed;
                divisable_tile_info_s tile_to_store = block;

                tile_to_store.name_ = "Tile_" + std::to_string(ret.size());
                ret.push_back(tile_to_store);
           }
            
        }


        struct viewrectbuckets_s
        {
            int view_count_, interval_count_;
            std::vector<rect_s> rect_buckets_;
            viewrectbuckets_s(int view_count, int interval_count) :
                view_count_(view_count), interval_count_(interval_count),
                rect_buckets_(view_count* interval_count) {}

            int get_id(int view_id, int interval_id) const {
                return view_id * interval_count_
                    + interval_id;
            };

            rect_s& rect(int view_id, int interval_id)
            {
                return rect_buckets_[get_id(view_id, interval_id)];
            }

            const rect_s& rect(int view_id, int interval_id) const
            {
                return rect_buckets_[get_id(view_id, interval_id)];
            }
        };

        struct omplock_s
        {
            omp_lock_t lock_ = nullptr;
            void lock() { omp_set_lock(&lock_); }

            void unlock() { omp_unset_lock(&lock_); }

            omplock_s() { omp_init_lock(&lock_); };
            ~omplock_s() { omp_destroy_lock(&lock_); };

        };

       

        TilingImpl::TilingImpl(const ATData& data, const ABBox3f& box, const params_s& param) : data_(data),
        params_(param)
        {
            global_bbox_ = box;
            imageids_.clear();
            imageids_ = data_.GetImageIDsTiling();
            point3dids_.clear();
            point3dids_ = data_.GetPointsIDsTiling();
          
            tiles_.clear();
        }

        TilingImpl::~TilingImpl()
        {
            imageids_.clear();
            point3dids_.clear();
            tiles_.clear();
        }
       
       
        
        void TilingImpl::CreateTilesInRegular(divide_params_s& params,std::map<int, std::string>& localid_to_tile)
        {
            tiles_.clear();
            ex_tilegrid_s& grid = params.tile_grid_;
            auto box = global_bbox_;
            double tile_size = params.tile_size_;
            ex_dividemode_e mode = params.divide_mode_;

            ABBox2d box2d;
            box2d.min() = Eigen::Vector2d{ box.min().x(),box.min().y() };
            box2d.max() = Eigen::Vector2d{ box.max().x(),box.max().y() };
            double min_z = box.min().z();
            double max_z = box.max().z();
            Eigen::Vector3d origin = params.tile_origin_;
            

            grid.start[0] = std::floor((box2d.corner(ABBox2d::BottomLeft)[0] - origin[0]) / tile_size);
            grid.start[1] = std::floor((box2d.corner(ABBox2d::BottomLeft)[1] - origin[1]) / tile_size);
            grid.start[2] = mode == ex_dividemode_e::REGULAR_PLANAR_GRID ? 0 :std::floor((min_z -origin[2])/tile_size);
            std::array<int, 3> end;
            end[0] = std::floor((box2d.corner(ABBox2d::TopRight)[0] - origin[0]) / tile_size);
            end[1] = std::floor((box2d.corner(ABBox2d::TopRight)[1] - origin[1]) / tile_size);
            end[2] = mode == ex_dividemode_e::REGULAR_PLANAR_GRID ? 0 : std::floor((max_z - origin[2]) / tile_size);;

            grid.dims[0] = end[0] - grid.start[0] + 1;
            grid.dims[1] = end[1] - grid.start[1] + 1;
            grid.dims[2] = end[2] - grid.start[2] + 1;
           
            std::set<std::string> namesset;
            
            int localidx = 0;
            for (int z = grid.start[2]; z <= end[2]; z++)
            {
                for (int y = grid.start[1]; y <= end[1]; y++)
                {
                    for (int x = grid.start[0]; x <= end[0]; x++)
                    {
                        tile_info_s tileinfo;
                        ABBox3d tilebb;
                        char name[256];
                        if (mode == ex_dividemode_e::REGULAR_PLANAR_GRID)
                        {
                            std::string str = "Tile";
                            std::vector<int> xy(2);
                            xy[0] = x;
                            xy[1] = y;
                            for (int idx = 0; idx < xy.size(); idx++)
                            {
                                if (xy[idx] < 0)
                                {
                                    str += "_-%03d";
                                    xy[idx] = -xy[idx];
                                }
                                else
                                {
                                    str += "_+%03d";
                                }
                            }

                            sprintf(name, str.c_str(), xy[0], xy[1]);
                          
                            
                             auto bbmin= Eigen::Vector3f(origin[0] + x * tile_size, origin[1] + y * tile_size, box.min().z());
                          auto bbmax = Eigen::Vector3f(origin[0] + (x + 1) * tile_size, origin[1] + (y + 1) * tile_size, box.max().z());
                           ABBox3f box3dtemp, box3dnew;
                           box3dtemp.min() = bbmin;
                           box3dtemp.max() = bbmax;
                          
                           if (!box.intersects(box3dtemp))
                               continue;
                           


                           bbmin.x() = bbmin.x() < box.min().x() ? box.min().x() : bbmin.x();
                            bbmax.x() = bbmax.x() > box.max().x() ? box.max().x() : bbmax.x();

                            bbmin.y() = bbmin.y() < box.min().y() ? box.min().y() : bbmin.y();
                            bbmax.y() = bbmax.y() > box.max().y() ? box.max().y() : bbmax.y();
                            box3dnew.min() = bbmin;
                            box3dnew.max() = bbmax;
                           if(bbmin.x() == bbmax.x() || bbmin.y() == bbmax.y() )
                               continue;
                               
                          
                            tilebb.min() = bbmin.cast<double>();
                            tilebb.max() = bbmax.cast<double>();
                            
                        }
                        else if (mode == ex_dividemode_e::REGULAR_VOLUMETRIC_GRID)
                        {
                           
                           
                            std::string str = "Tile";
                            std::vector<int> xy(3);
                            xy[0] = x;
                            xy[1] = y;
                            xy[2] = z;
                            for (int idx = 0; idx < xy.size(); idx++)
                            {
                                if (xy[idx] < 0)
                                {
                                    str += "_-%03d";
                                    xy[idx] = -xy[idx];
                                }
                                else
                                {
                                    str += "_+%03d";
                                }
                            }

                            sprintf(name, str.c_str(), xy[0], xy[1], xy[2]);

                            double z_min_temp = origin[2] + (z ) * tile_size;
                            double z_max_temp = origin[2] + (z + 1) * tile_size;
                            z_min_temp = z_min_temp < min_z ? min_z : z_min_temp;
                            z_max_temp = z_min_temp > max_z ? max_z : z_max_temp;
                           auto bbmin = Eigen::Vector3f(origin[0] + x * tile_size, origin[1] + y * tile_size, z_min_temp);
                           auto bbmax = Eigen::Vector3f(origin[0] + (x + 1) * tile_size, origin[1] + (y + 1) * tile_size, z_max_temp);


                            ABBox3f box3dtemp;
                            box3dtemp.min() = bbmin;
                            box3dtemp.max() = bbmax;
                            if (!box.intersects(box3dtemp))
                                continue;

                            bbmin.x() = bbmin.x() < box.min().x() ? box.min().x() : bbmin.x();
                            bbmax.x() = bbmax.x() > box.max().x() ? box.max().x() : bbmax.x();

                            bbmin.y() = bbmin.y() < box.min().y() ? box.min().y() : bbmin.y();
                            bbmax.y() = bbmax.y() > box.max().y() ? box.max().y() : bbmax.y();
                            if (bbmin.x() == bbmax.x() || bbmin.y() == bbmax.y() || bbmin.z() == bbmax.z())
                                continue;
                            tilebb.min() = Eigen::Vector3d{ bbmin.x(),bbmin.y(),z_min_temp };
                            tilebb.max() = Eigen::Vector3d{ bbmax.x(),bbmax.y(),z_max_temp };

                        }
                        tileinfo.bb_ = tilebb.cast<float>();
                        
                        tileinfo.name_ = name;
                       
                        tiles_[name] = tileinfo;
                        localid_to_tile[localidx] = name;
                        localidx++;
                    }
                }
            }


        }
        AI3D::CORE::TilingImpl::params_s TilingImpl::GetParams()
        {
            return params_;
        }
        AI3D::CORE::TilingImpl::params_s& TilingImpl::GetParamsMutual()
        {
            return params_;
        }
        
        bool TilingImpl::Run(float& maxram_for_tiles)
        {
            const int tilePointThreadHold = Application::Getinstance().ParseConfig().tile_point_threshold;
            clock_t  trun1, trun2, trun3;
            tiles_.clear();
            trun1 = clock();
            clock_t t1, t2, time_consume;
            t1 = clock();
            float maxram = 0.f;
            
            ABBox3f global_bbox;
            if (0)
            {
                for (auto& view : imageids_)
                {
                    global_bbox.extend(data_.GetImage(view).GetPosition().cast<float>());
                }
                {

                    for (auto iter : point3dids_)
                    {
                        auto point = data_.GetPoint3D(iter);
                        global_bbox.extend(point.GetXYZ().cast<float>());
                    }
                    global_bbox = ExtendBoundingBox(global_bbox, global_bbox.diagonal().norm() / 50.0f);
                }
            }
            else
            {
                global_bbox = global_bbox_;
            }

            if (params_.mode_ == ex_dividemode_e::ADAPTIVE_TILING)
            {

                std::vector<divisable_tile_info_s> tiles;
                const bool ret = CutSfmInBlocks(tiles, global_bbox);


              
                for (int i = 0; i < tiles.size(); ++i)
                {

                    std::string tilename = "Tile_" + std::to_string(i + 1);
                    tile_info_s tileinfo;
                    tileinfo.name_ = tilename;
                    tileinfo.bb_ = tiles[i].box_.intersection(global_bbox_);
                    if (!tiles[i].ptx_idx_.empty())                  
                    {
                        std::set<point3D_t> idset(tiles[i].ptx_idx_.begin(), tiles[i].ptx_idx_.end());

                        tileinfo.point_ids_ = idset;
                    }
                    for (auto iterimg : tiles[i].image_and_rect_)
                    {
                        tileinfo.image_ids_.insert(iterimg.first);
                  }
                    
                    tileinfo.isempty = (tiles[i].point_count_ < tilePointThreadHold);
                    tileinfo.ram_estimated_ = tiles[i].memory_need_cpu_ / 1024.0 / 1024.0 / 1024.0;

                    if (tileinfo.ram_estimated_ > maxram)
                    {
                        maxram = tileinfo.ram_estimated_;
                    }
         
                    tiles_[tilename] = tileinfo;


                }
               
            }
            else
            {
                divide_params_s divide_param;
                divide_param.divide_mode_ = params_.mode_;
                divide_param.tile_size_ = params_.tile_size_;
                divide_param.tile_origin_ = global_bbox.min().cast<double>();
                std::map<int, std::string> localid_to_tile;
                clock_t t11, t21, t31;
              
                t11 = clock();
             
                CreateTilesInRegular(divide_param, localid_to_tile);
                t21 = clock();
                t31 = t21 - t11;
                std::cout << t31  << " create tile " <<t31*0.001<< std::endl;
                t11 = clock();
                EIGEN_STL_UMAP(std::string, tile_info_s) tiles = tiles_;
                
                auto level = 1;
                if (level != 2)
                {
                    CutBlockForTile(localid_to_tile, divide_param, data_, tiles, maxram);
                    t21 = clock();
                    t31 = t21 - t11;
                    std::cout << t31 << " cut block " << t31 * 0.001 << std::endl;
                }
                else
                {
                    t11 = clock();
                    CutBlockForTileFrustum(localid_to_tile, divide_param, data_, tiles, maxram);

                    t21 = clock();
                    t31 = t21 - t11;
                    std::cout << t31 << " cut block  frustum" << t31 * 0.001 << std::endl;
                }
                tiles_ = tiles;
               

            }
            maxram_for_tiles = maxram;


          

            t2 = clock();
            time_consume = (t2 - t1);
            trun2 = clock();
            trun3 = (t2 - t1) * 0.001;
            std::cout << "CreateTilesInRegular time consume s " << time_consume*0.001 << std::endl;
            std::cout << "Total Tiles Num: "<<tiles_.size() << std::endl;
            std::cout << "trun3: " << trun3 << std::endl;
            return true;

        }

        const  EIGEN_STL_UMAP(std::string, tile_info_s) TilingImpl::GetTilesInfo()
        {
            return tiles_;
        }
       
        void TilingImpl::FindBestCutAlongAxis(const divisable_tile_info_s& tile,const divisibleaxis axis,const int interval_count,
            const float cut_extreme_ration, float& best_cut, std::pair<divisable_tile_info_s, divisable_tile_info_s>& child_tile) const
        {
            
            const float interval_range = tile.tight_box_.max()[int(axis)] - tile.tight_box_.min()[int(axis)];
            
            const float min_value = tile.tight_box_.min()[int(axis)] + cut_extreme_ration * interval_range;
            const float max_value = tile.tight_box_.max()[int(axis)] - cut_extreme_ration * interval_range;
            
            const float bucket_width = (max_value - min_value) / static_cast<float>(interval_count - 1);

            
            std::vector<float> var(interval_count);
            for (size_t var_id = 0; var_id < var.size(); ++var_id)
            {
                var[var_id] = min_value + bucket_width * var_id;
            }
            std::map<int, image_t> image_idx_to_id;
            std::map<image_t, int> id_to_idx;
            int img_idx = 0;
            image_t images_num = imageids_.size();
            std::vector<Image> images_temp(images_num);
            for (auto& iter : imageids_)
            {
                image_idx_to_id[img_idx] = iter;
                id_to_idx[iter] = img_idx;
                images_temp[img_idx] = (data_.GetImage(iter));
                img_idx++;

            }


            int cut_idx;
            long long lower_pixel_count;
            long long upper_pixel_count;

            int lower_ray_count, lower_point_count;
            int upper_ray_count, upper_point_count;

            std::vector<rect_s> lower_views_rect, upper_views_rect;
            std::vector<long long > lower_pixel_count_cut_at(interval_count, 0);
            std::vector<long long > upper_pixel_count_cut_at(interval_count, 0);

            std::vector<int> lower_ray_count_cut_at(interval_count);
            std::vector<int> upper_ray_count_cut_at(interval_count);

            std::vector<int> lower_point_count_cut_at(interval_count);
            std::vector<int> upper_point_count_cut_at(interval_count);

            viewrectbuckets_s lower_view_buckets(int(images_num), interval_count);
            viewrectbuckets_s upper_view_buckets(int(images_num), interval_count);

            std::vector<int> lower_point_buckets(interval_count, 0);
            std::vector<int> upper_point_buckets(interval_count, 0);

            std::vector<int> lower_ray_buckets(interval_count, 0);
            std::vector<int> upper_ray_buckets(interval_count, 0);

            
           
           
            clock_t t1, t2, time_consume;
            t1 = clock();
            std::vector< omplock_s> view_locks(images_num);
#pragma omp parallel for

            for (int pidx_id = 0; pidx_id < tile.ptx_idx_.size(); pidx_id++)
            {
                const point3D_t pid = tile.ptx_idx_[pidx_id];
                if (!data_.GetPointsViews().count(pid))
                {
                    continue;
                }
                const int tracks_count = int(data_.GetPointsViews().at(pid).size());
                const Eigen::Vector3d& p = data_.GetPoint3D(pid).GetXYZ();
                const float value = p[int(axis)];

                const int lower_bucket_id = std::max(0, int(std::ceil((value - min_value) / bucket_width)));
                const int upper_bucket_id = std::min(interval_count - 1, int(std::floor((value - min_value) / bucket_width)));

                bool lower_ok = lower_bucket_id < interval_count;
                bool upper_ok = upper_bucket_id >= 0;
              
                if (lower_ok)
                {
                    auto& point_buckets = lower_point_buckets[lower_bucket_id];
                    auto& ray_buckets = lower_ray_buckets[lower_bucket_id];

#pragma omp atomic
                    point_buckets++;
#pragma omp atomic
                    ray_buckets += tracks_count;


                }
                if (upper_ok)
                {
                    auto& point_buckets = upper_point_buckets[upper_bucket_id];
                    auto& ray_buckets = upper_ray_buckets[upper_bucket_id];

#pragma omp atomic
                    point_buckets++;
#pragma omp atomic
                    ray_buckets += tracks_count;

                }

                for (const auto& c : data_.GetPointsViews().at(pid))
                {


                    const auto view_id = c.first;
                    const auto view_idx = id_to_idx[view_id];
                    auto pmatrix = data_.GetImage(view_id).GetProjectionMatrix();

                    Eigen::Vector3d pt3d = pmatrix * (p.cast<double>()).homogeneous();
                    auto cam_matrix = data_.GetCamera(data_.GetImage(view_id).GetCameraId()).GetCalibrationMatrix();
                    const Eigen::Vector2i projected = ((cam_matrix * pt3d).hnormalized()).array().round().cast<int>();
                    view_locks[view_idx].lock();
                    if (lower_ok)
                    {
                        lower_view_buckets.rect(view_idx, lower_bucket_id).extend(projected.x(), projected.y());
                    }

                    if (upper_ok)
                    {
                        upper_view_buckets.rect(view_idx, upper_bucket_id).extend(projected.x(), projected.y());
                    }
                    view_locks[view_idx].unlock();
                }
            }
            t2 = clock();
             time_consume = (t2 - t1) ;
       
            
            t1 = clock();
            
            
            for(int vidx = 0;vidx<id_to_idx.size();vidx++)
            {
                auto v = vidx;
                for (int i = 1; i < interval_count; ++i)
                {
                    lower_view_buckets.rect(v, i).extend(lower_view_buckets.rect(v, i - 1));
                    
                }
               
                for (int i = interval_count - 2; i >= 0; --i)
                {
                    upper_view_buckets.rect(v, i).extend(upper_view_buckets.rect(v, i + 1));
                  
                }

                for (auto& bucket : { &lower_view_buckets,&upper_view_buckets })
                {
                    for (int i = 0; i < interval_count; ++i)
                    {
                        rect_s& rect = bucket->rect(v, i);
                      
                        rect_s image_rect(0, 0, images_temp[v].GetWidth(), images_temp[v].GetHeight());
                        
                        rect.clamp(image_rect);
                       
                        if (rect.isempty() || rect.width_ < params_.pixel_width_threshold_for_ignoring_view_ || rect.height_ <= params_.pixel_width_threshold_for_ignoring_view_)
                        {
                            rect = rect_s();
                        }
                        else
                        {
                            rect.extend(params_.rect_extension_width_);
                            rect.clamp(image_rect);
                        }
                        
                    }
                }
            }
            t2 = clock();
            time_consume = (t2 - t1) ;
           
            t1 = clock();
#pragma omp parallel for
            for (int i = 0; i < interval_count; ++i)
            {
                lower_pixel_count_cut_at[i] = 0;
                upper_pixel_count_cut_at[i] = 0;

                
                for (int vidx = 0; vidx < id_to_idx.size(); vidx++)
                {
                   
                    auto v = vidx;
                    
                    if (!lower_view_buckets.rect(v, i).isempty())
                    {
                        lower_pixel_count_cut_at[i] += lower_view_buckets.rect(v, i).area();
                       


                    }
                    if (!upper_view_buckets.rect(v, i).isempty())
                    {
                        upper_pixel_count_cut_at[i] += upper_view_buckets.rect(v, i).area();
                      
                    }
                }

            }

            lower_point_count_cut_at[0] = lower_point_buckets[0];
            lower_ray_count_cut_at[0] = lower_ray_buckets[0];
            for (int i = 1; i < interval_count; ++i)
            {
                lower_point_count_cut_at[i] = lower_point_count_cut_at[i - 1] + lower_point_buckets[i];
                lower_ray_count_cut_at[i] += lower_ray_count_cut_at[i - 1] + lower_ray_buckets[i];
            }
            upper_point_count_cut_at[interval_count - 1] = upper_point_buckets[interval_count - 1];
            upper_ray_count_cut_at[interval_count - 1] = upper_ray_buckets[interval_count - 1];

            for (int i = interval_count - 2; i >= 0; --i)
            {
                upper_point_count_cut_at[i] = upper_point_count_cut_at[i + 1] + upper_point_buckets[i];
                upper_ray_count_cut_at[i] += upper_ray_count_cut_at[i + 1] + upper_ray_buckets[i];
            }

           
            t2 = clock();
            time_consume = (t2 - t1) ;
           
            t1 = clock();
            std::vector<long long> obj_func;
            obj_func.reserve(interval_count);

            std::transform(lower_pixel_count_cut_at.begin(),
                lower_pixel_count_cut_at.end(),
                upper_pixel_count_cut_at.begin(),
                std::back_inserter(obj_func), std::minus<long long>());
            auto lower = std::lower_bound(obj_func.begin(), obj_func.end(), 0);
            auto upper = std::upper_bound(obj_func.begin(), obj_func.end(), 0);
           
            if (lower == obj_func.end())
            {
                cut_idx = int(obj_func.size() - 1);
            }
            else
            {
                if (upper == obj_func.begin())
                {
                    upper = obj_func.end() - 1;
                }
                const int lower_idx = int(lower - obj_func.begin());
                const int upper_idx = int(upper - obj_func.begin());
               
                cut_idx = (lower_idx + upper_idx) / 2;
            }
            t2 = clock();
            time_consume = (t2 - t1) ;
           
            best_cut = var[cut_idx];
           

            t1 = clock();
            lower_pixel_count = lower_pixel_count_cut_at[cut_idx];
            upper_pixel_count = upper_pixel_count_cut_at[cut_idx];

            lower_ray_count = lower_ray_count_cut_at[cut_idx];
            upper_ray_count = upper_ray_count_cut_at[cut_idx];

            lower_point_count = lower_point_count_cut_at[cut_idx];
            upper_point_count = upper_point_count_cut_at[cut_idx];

            lower_views_rect.resize(images_num);
            upper_views_rect.resize(images_num);

            
           
            for (int vidx = 0; vidx < id_to_idx.size(); vidx++)
            
            { 
                int imgcount = vidx;
                lower_views_rect[imgcount] = lower_view_buckets.rect(imgcount, cut_idx);
                upper_views_rect[imgcount] = upper_view_buckets.rect(imgcount, cut_idx);
                
            }

            ABBox3f lower_box, upper_box;
            constexpr float inf = std::numeric_limits<float>::infinity();
            const Eigen::Vector3f low_inf(-inf, -inf, -inf);
            const Eigen::Vector3f high_inf(inf, inf, inf);
            Eigen::Vector3f low_half = high_inf; low_half[int(axis)] = best_cut;
            Eigen::Vector3f high_half = low_inf; high_half[int(axis)] = best_cut;

            ABBox3f low_half_space(low_inf, low_half);
            ABBox3f high_half_space(high_half,high_inf );

            lower_box = low_half_space.intersection(tile.box_);
            upper_box = high_half_space.intersection(tile.box_);

            divisable_tile_info_s* block_ptrs[2] = { &child_tile.first,&child_tile.second };
            const ABBox3f bboxex_cut[2] = {lower_box,upper_box};
            const long long pixel_counts[2] = { lower_pixel_count,upper_pixel_count };

            const int ray_counts[2] = { lower_ray_count,upper_ray_count };

            const int point_counts[2] = { lower_point_count,upper_point_count };
            std::vector<rect_s>* view_bucket[2] = {&lower_views_rect,&upper_views_rect };
            t2 = clock();
            time_consume = (t2 - t1) ;
         
            t1 = clock();
            for (int i = 0; i < 2; ++i)
            {
                ABBox3f tight_box_cut;
                std::vector<point3D_t> pts_selection;
                pts_selection.reserve(tile.ptx_idx_.size());
                for (auto id_idx : tile.ptx_idx_)
                {
                    const auto& p = data_.GetPoint3D(id_idx).GetXYZ().cast<float>();

                    if (bboxex_cut[i].contains(p))
                    {
                        pts_selection.push_back(id_idx);
                        tight_box_cut.extend(p);
                    }
                }
                tight_box_cut = ExtendBoxToInfiniteInNonDivisibleDimensions(tight_box_cut);

                block_ptrs[i]->ptx_idx_ = pts_selection;
                block_ptrs[i]->tight_box_ = tight_box_cut;
                block_ptrs[i]->box_ = bboxex_cut[i];
                block_ptrs[i]->point_count_ = point_counts[i];
                block_ptrs[i]->pixel_count_ = pixel_counts[i];;
                block_ptrs[i]->ray_count_ = ray_counts[i];;
               
                
                for (int vidx = 0; vidx < id_to_idx.size(); vidx++)
                
                {
                    auto v = vidx;
                    if (!view_bucket[i]->at(v).isempty())
                    {
                        
                        block_ptrs[i]->image_and_rect_[image_idx_to_id[v]] = view_bucket[i]->at(v);
                    }
               }
            }
            t2 = clock();
            time_consume = (t2 - t1) ;
           

        }
        bool TilingImpl::FindBestCut(const divisable_tile_info_s& tile, std::pair<divisable_tile_info_s,
            divisable_tile_info_s>& child_tile, divisibleaxis& cut_dim) const
        {
            const auto& tight_bbox = tile.tight_box_;
            float ranges[kDivisibleDimN];

            for (int i = 0; i < kDivisibleDimN; ++i)
            {
                divisibleaxis dim = kDivisibleDims[i];
                ranges[i] = tight_bbox.max()(int(dim)) - tight_bbox.min()(int(dim));
            }
            const float cut_extreme_ration = 0.1f;
            const int interval_count = 256;
            float best_cut[kDivisibleDimN];

            std::pair<divisable_tile_info_s, divisable_tile_info_s> box_info_pairs[kDivisibleDimN];

            unsigned long long max_pixel_count[kDivisibleDimN];

            for (int k = 0; k < kDivisibleDimN; ++k)
            {
                const divisibleaxis aixs = kDivisibleDims[k];
                FindBestCutAlongAxis(tile, aixs, interval_count, cut_extreme_ration, best_cut[k], box_info_pairs[k]);

                max_pixel_count[k] = std::max(box_info_pairs[k].first.pixel_count_,box_info_pairs[k].second.pixel_count_);

            }


            const float too_long_ratio = 0.2f;
            std::vector<std::pair<float, int>> sorted_range_dim(kDivisibleDimN);
            for (int k = 0; k < kDivisibleDimN; ++k)
            {
                sorted_range_dim[k] = std::make_pair(ranges[k],k);

            }

            std::sort(sorted_range_dim.begin(), sorted_range_dim.end(), [](const std::pair<float, int>& a, const std::pair<float, int>& b)
                {
                    return a.first > b.first;
                }
            );

            const float max_range = sorted_range_dim[0].first;
            const float second_max_range = sorted_range_dim[1].first;
            const float dim_id_max_range = sorted_range_dim[0].second;
            const float box_too_long = second_max_range/ max_range < too_long_ratio;

            const float cost_almost_same_ratio = 0.1f;
            std::vector<std::pair<unsigned long long, int>> sorted_cost_dim(kDivisibleDimN);
            for (int k = 0; k < kDivisibleDimN; ++k)
            {
                sorted_cost_dim[k] = std::make_pair(max_pixel_count[k],k);
            }
            std::sort(sorted_cost_dim.begin(), sorted_cost_dim.end(), [](const std::pair<float, int>& a, const std::pair<float, int>& b)
                {
                    return a.first < b.first;
                }
            );

            const unsigned long long min_cost = sorted_cost_dim.front().first;
            if (double(min_cost) / double(tile.pixel_count_) > params_.stop_cutting_view_reduce_ratio_)
            {
                return false;
            }

            int dim_id_min_cost = sorted_cost_dim.front().second;
            int dim_id_second_min_cost = sorted_cost_dim[1].second;
            const unsigned long long second_min_cost = sorted_cost_dim[1].first;
            const bool cost_almost_same = double(second_min_cost - min_cost)/double(second_min_cost) < cost_almost_same_ratio;
            int cut_dim_id;
            if (box_too_long)
            {
                cut_dim_id = dim_id_max_range;
            }
            else
            {
                if (!cost_almost_same)
                {
                    cut_dim_id = dim_id_min_cost;
                }
                else
                {
                    cut_dim_id = ranges[dim_id_min_cost] > ranges[dim_id_second_min_cost] ? dim_id_min_cost : dim_id_second_min_cost;
                }
            }
            cut_dim = kDivisibleDims[cut_dim_id];
            child_tile.first = std::move(box_info_pairs[cut_dim_id].first);
            child_tile.second = std::move(box_info_pairs[cut_dim_id].second);
            return true;
        }
        bool TilingImpl::CutSfmInBlocks(std::vector<divisable_tile_info_s>& ret, const ABBox3f& bbox) const
        {
            divisable_tile_info_s global_block_info;

            global_block_info.box_ = bbox;
            auto& idx = global_block_info.ptx_idx_;
            idx.resize(point3dids_.size(), 0);
            point3D_t ptcnt = 0;
            std::map<point3D_t,bool> selected_point;
            for (auto& iter : point3dids_)
            {
                idx[ptcnt] = iter;

                if (bbox.contains(data_.GetPoint3D(iter).GetXYZ().cast<float>()))
                {
                    selected_point[iter] = true;
                }
                ptcnt++;

            }

            
            std::map<image_t, bool> views;
            int validcnt = 0;
            for (const auto& iter : point3dids_)
            {
                
                if (!selected_point.count(iter))
                {
                    continue;
                }
                if (!data_.GetPointsViews().count(iter))
                {                 
                    continue;
                }
                const auto p = data_.GetPoint3D(iter).GetXYZ().cast<float>();
                global_block_info.tight_box_.extend(p);
                global_block_info.point_count_++;
               
                global_block_info.ray_count_ += data_.GetPointsViews().at(iter).size();

                for (const auto& ele :data_.GetPointsViews().at(iter) )
                {
                    views[ele.first] = true;
                    validcnt++;
                }
            }
           
           

            for (const auto& image_id : imageids_)
            {
                if (views[image_id])
                {
                    auto image = data_.GetImage(image_id);
                    global_block_info.pixel_count_ += image.GetWidth() * image.GetHeight();
                    
                    global_block_info.image_and_rect_[image_id] = 
                    {
                        0,0,int(image.GetWidth()),image.GetHeight()
                    };



                }
            }
           
            CutRecursive(ret,global_block_info);


            unsigned long long total_pixel_used_count = 0;
            for (const auto& b : ret)
            {
                total_pixel_used_count += b.pixel_count_;
            }
            const double redundancy_rate = double(total_pixel_used_count) / double(global_block_info.pixel_count_);
            return true;
        }

    }
}

