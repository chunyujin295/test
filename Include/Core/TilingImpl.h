#ifndef _AI3D_CORE_TILINGIMPL_H_
#define _AI3D_CORE_TILINGIMPL_H_
#include "Core/ReconstructionOptions.h"
#include "Core/Types.h"
#include "Constants.h"
#include "Core/ATData.h"
#include "Core/CoordinateSystem.h"
namespace AI3D
{
    namespace CORE
    {

        enum ex_dividemode_e
        {
            REGULAR_PLANAR_GRID = 0,
            REGULAR_VOLUMETRIC_GRID = 1,
            ADAPTIVE_TILING
        };


        struct ex_tilegrid_s
        {
            Eigen::Vector3i start = Eigen::Vector3i::Zero();
            Eigen::Vector3i dims = Eigen::Vector3i::Zero();

        };



        struct ex_tileinfo_s
        {
            std::string name_;
            double max_memory_gpu_ = 0;
            double max_memory_cpu_ = 0;
            bool bempty_ = true;

            sceneROI_s roi_;

            ex_tileinfo_s(const std::string& name,double max_memory_cpu, double max_memory_gpu,bool empty,const sceneROI_s& roi):name_(name),
                max_memory_cpu_(max_memory_cpu), max_memory_gpu_(max_memory_gpu),bempty_(empty),roi_(roi)
            {

            }

            void ExpandROI(float ratio)
            {
                if (roi_.boundary_.size() != 4) return;
                auto& bds = roi_.boundary_;
                Eigen::Vector2d dl = bds[2] - bds[0], d2 = bds[3] - bds[1];
                bds[0] -= dl * ratio;
                bds[2] += dl * ratio;
                bds[1] -= d2 * ratio;
                bds[3] += d2 * ratio;
                roi_.min_z_ -= (roi_.max_z_ - roi_.min_z_) * ratio;
                roi_.max_z_ += (roi_.max_z_ - roi_.min_z_) * ratio;
            }

        };
        
        struct divide_params_s
        {
            ex_dividemode_e divide_mode_ = REGULAR_PLANAR_GRID;
            double tile_size_ = -1.;
            double usable_memory_cpu_ = 0;
            bool cut_image_rect_ = false;
            ex_tilegrid_s tile_grid_;
          

            Eigen::Vector3d tile_origin_ = Eigen::Vector3d::Zero();

            divide_params_s()=default;



        };
       static  uint64_t EstimateMemoryUsage(const std::vector<std::pair<int, int>>& image_size, int resolution_level = 1)
        {
            uint64_t pixels_all = 0ull;
            if (resolution_level < 0 || resolution_level>3)
            {
                throw std::runtime_error("error resolution level");
            }
            for (const auto& img : image_size)
            {
                pixels_all += img.first * img.second;
            }

            const uint64_t pixels_avg = pixels_all / image_size.size();

            const int patch_match_level = std::max(0, resolution_level - 1);
            const int refine_image_level = std::max(0, resolution_level - 1);
            const int texture_image_level = std::max(0, resolution_level - 1);
            const int batch_size = std::min(int(image_size.size()), 100);
            std::vector<uint64_t> memory_occupy(5);

            memory_occupy[0] = uint64_t(pixels_all * std::pow(0.5, patch_match_level * 2) * sizeof(uchar) +
                double(pixels_avg) * batch_size * std::pow(0.5, resolution_level * 2) * (
                    +sizeof(float)
                    + sizeof(uchar)
                    + sizeof(uint16_t)
                    + sizeof(float) * 3
                    + sizeof(float)));
            memory_occupy[0] = uint64_t(memory_occupy[0] * 0.75);
            memory_occupy[1] = uint64_t(pixels_all * std::pow(0.5, patch_match_level * 2) * sizeof(uchar) * 2.2f);
            memory_occupy[2] = uint64_t(pixels_all * std::pow(0.5, refine_image_level * 2) * sizeof(uchar) * 4.0f);
            memory_occupy[3] = uint64_t(pixels_all * std::pow(0.5, texture_image_level * 2) * sizeof(uchar) * 3.6f);
            memory_occupy[4] = uint64_t(pixels_all * std::pow(0.5, texture_image_level * 2) * sizeof(uchar) * 4.0f);
            return  *std::max_element(memory_occupy.begin(), memory_occupy.end()) ;
        }
       static int CutBlockForTile(std::map<int, std::string> indexes,divide_params_s& param, const ATData& data, EIGEN_STL_UMAP(std::string, tile_info_s)& tiles, float& maxram_tile);
       static int CutBlockForTileFrustum(std::map<int, std::string> indexes, divide_params_s& param, const ATData& data, EIGEN_STL_UMAP(std::string, tile_info_s)& tiles, float& maxram_tile);
        static float GetExpectedMaxRamUsageForAJob(std::vector<std::pair<int, int>> image_size, int resolution_level=1)
        {

            if (image_size.empty())
                return 0.f;
            auto raminbyte = EstimateMemoryUsage(image_size, resolution_level);
            float raminGB = raminbyte / 1024.0 / 1024.0 / 1024.0;
            return  raminGB;
        }
        


        enum class divisibleaxis : int {

            X=0,
            Y,
            Z,
            Size
        };

        class AI3D_API TilingImpl
        {
        public:
            struct divisable_tile_info_s
            {
                ABBox3f box_;
                ABBox3f tight_box_;
                size_t point_count_ = 0;
                size_t ray_count_ =  0;
                unsigned long long pixel_count_ = 0;
                std::map<image_t, rect_s> image_and_rect_;
                std::string name_;
                std::vector<point3D_t> ptx_idx_;
               
                size_t memory_need_cpu_ = 0;
            };

            struct params_s
            {
                const unsigned long long usable_memory_in_bytes_=256;
                const int pixel_width_threshold_for_ignoring_view_ = 256;
                const int rect_extension_width_ = 128;
                const int max_depth_=-1;
                const float stop_cutting_view_reduce_ratio_ = 0.9f;
                ex_dividemode_e mode_ = ex_dividemode_e::ADAPTIVE_TILING;
                const int resolution_level_=1;

                double tile_size_ =-1.;
                params_s(double tile_size, ex_dividemode_e mode)
                {
                    tile_size_ = tile_size;
                    mode_ = mode;
                };
                params_s(unsigned long long usable_memory_in_bytes,int max_depth,
                   int resolution_level,int reconstruct_mode ):
                    usable_memory_in_bytes_(usable_memory_in_bytes),
                    max_depth_(max_depth),
                    resolution_level_(resolution_level)
                
                {
                };
               
            };
           
            TilingImpl(const ATData& data, const ABBox3f& box, const params_s& param);
            ~TilingImpl();
            bool Run(float& maxram_for_tiles) ;

            const  EIGEN_STL_UMAP(std::string, tile_info_s) GetTilesInfo();
            
            void CreateTilesInRegular(divide_params_s& param, std::map<int, std::string>& localid_to_tile);
            params_s GetParams();
            params_s& GetParamsMutual();
        private:
            params_s params_;
            ATData data_;
            std::set<image_t> imageids_;
            std::set<point3D_t> point3dids_;
            ABBox3f global_bbox_;
            EIGEN_STL_UMAP(std::string, tile_info_s) tiles_;
            
            void FindBestCutAlongAxis(const divisable_tile_info_s& tile,const divisibleaxis axis,const int interval_count,
                const float cut_extreme_ration,float& best_cut,std::pair<divisable_tile_info_s, divisable_tile_info_s>& child_tile) const;
            bool FindBestCut(const divisable_tile_info_s& tile,  
                std::pair<divisable_tile_info_s, divisable_tile_info_s>& child_tile ,
                divisibleaxis& cut_dim) const;
            bool CutSfmInBlocks(std::vector<divisable_tile_info_s>& ret,const ABBox3f& bbox) const;
            void CutRecursive(std::vector<divisable_tile_info_s>& ret,  divisable_tile_info_s& block, int depth = 0) const;

        };
    }
}

#endif