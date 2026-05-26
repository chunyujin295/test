#ifndef _AI3D_CORE_TILING_H_
#define _AI3D_CORE_TILING_H_
#include <Constants.h>
#include <glog/logging.h>
#include <pugixml.hpp>
#include "Core/ATData.h"
#include "Core/Types.h"
#include "Core/ReturnCode.h"
#include <map>
#include <omp.h>
#include "Core/Rapidjson.h"
#include "Core/ReconstructionOptions.h"
namespace AI3D
{
    namespace CORE
    {

        
        
        class AI3D_API Tiling
        {
        public:
            Tiling() {};
            Tiling( tiling_param_s params) ;
            
            virtual int RunDefault(const ATData& data, const ABBox3d& points) =  0;
            virtual int Run(const ATData& data, const ABBox3d& points) = 0;
            
            int GetNumTiles();
            
            const  EIGEN_STL_UMAP(std::string, tile_info_s)& GetTilesInfo() const ;
             
            const tiling_param_s& GetParams() const;
            tiling_param_s& GetParamsMutual();
        protected:
            EIGEN_STL_UMAP(std::string, tile_info_s) tiles_;
            tiling_param_s params_;
           
        };

       

        class AI3D_API NoneModeTiling :public Tiling
        {
        public:
            NoneModeTiling() {};
            NoneModeTiling( tiling_param_s params) ;
            int RunDefault(const ATData& data, const ABBox3d& points);
            int Run(const ATData& data, const ABBox3d& points);
            
        private:

           
        };

        class AI3D_API Regular2DModeTiling :public Tiling
        {
        public:
           
            Regular2DModeTiling(tiling_param_s params);
            int RunDefault(const ATData& data, const ABBox3d& points);
            int Run(const ATData& data, const ABBox3d& points);
            
        private:
           
        };

        class AI3D_API Regular3DModeTiling :public Tiling
        {
        public:
            
            Regular3DModeTiling(tiling_param_s params);
            int RunDefault(const ATData& data, const ABBox3d& points);
            int Run(const ATData& data, const ABBox3d& points);
            
        private:
          
        };

        class AI3D_API AdaptiveModeTiling :public Tiling
        {
        public:
            
            AdaptiveModeTiling(tiling_param_s params);
            int RunDefault(const ATData& data, const ABBox3d& points);
            int Run(const ATData& data, const ABBox3d& points);
            
        private:
        
        };


        extern "C" 
        {
            AI3D_API Tiling* TilingGenaratorFactory( tiling_param_s param);
            AI3D_API float GetExpectedMaxRamUsageForAJob(const ATData& data, const std::vector<Eigen::Vector2d>& points);
        }
        
    }
   
}
#endif