#ifndef _AI3D_SRC_UI_COLORMAPS_H_
#define _AI3D_SRC_UI_COLORMAPS_H_

#include <Eigen/Core>

#include "Core/ATData.h"
#include "Core/Alignment.h"
#include "Core/Types.h"

namespace AI3D 
{
    namespace GUI 
    {
        
        class PointColormapBase
        {
        public:
            PointColormapBase();

            virtual void Prepare(EIGEN_STL_UMAP(camera_t, AI3D::CORE::Camera)& cameras,
                EIGEN_STL_UMAP(image_t, AI3D::CORE::Image)& images,
                EIGEN_STL_UMAP(point3D_t, AI3D::CORE::Point3D)& points3D,
                std::vector<image_t>& image_ids) = 0;

            virtual Eigen::Vector3f ComputeColor(const point3D_t point3D_id,
                const AI3D::CORE::Point3D& point3D) = 0;

            void UpdateScale(std::vector<float>* values);
            float AdjustScale(const float gray);

            float scale;
            float min;
            float max;
            float range;
            float min_q;
            float max_q;
        };

        
        class PointColormapPhotometric : public PointColormapBase {
        public:
            void Prepare(EIGEN_STL_UMAP(camera_t, AI3D::CORE::Camera)& cameras,
                EIGEN_STL_UMAP(image_t, AI3D::CORE::Image)& images,
                EIGEN_STL_UMAP(point3D_t, AI3D::CORE::Point3D)& points3D,
                std::vector<image_t>& image_ids);

            Eigen::Vector3f ComputeColor(const point3D_t point3D_id,
                const AI3D::CORE::Point3D& point3D);
        };

        
        class PointColormapError : public PointColormapBase {
        public:
            void Prepare(EIGEN_STL_UMAP(camera_t, AI3D::CORE::Camera)& cameras,
                EIGEN_STL_UMAP(image_t, AI3D::CORE::Image)& images,
                EIGEN_STL_UMAP(point3D_t, AI3D::CORE::Point3D)& points3D,
                std::vector<image_t>& image_ids);

            Eigen::Vector3f ComputeColor(const point3D_t point3D_id,
                const AI3D::CORE::Point3D& point3D);
        };

        
        class PointColormapTrackLen : public PointColormapBase {
        public:
            void Prepare(EIGEN_STL_UMAP(camera_t, AI3D::CORE::Camera)& cameras,
                EIGEN_STL_UMAP(image_t, AI3D::CORE::Image)& images,
                EIGEN_STL_UMAP(point3D_t, AI3D::CORE::Point3D)& points3D,
                std::vector<image_t>& image_ids);

            Eigen::Vector3f ComputeColor(const point3D_t point3D_id,
                const AI3D::CORE::Point3D& point3D);
        };

        
        class PointColormapGroundResolution : public PointColormapBase {
        public:
            void Prepare(EIGEN_STL_UMAP(camera_t, AI3D::CORE::Camera)& cameras,
                EIGEN_STL_UMAP(image_t, AI3D::CORE::Image)& images,
                EIGEN_STL_UMAP(point3D_t, AI3D::CORE::Point3D)& points3D,
                std::vector<image_t>& image_ids);

            Eigen::Vector3f ComputeColor(const point3D_t point3D_id,
                const AI3D::CORE::Point3D& point3D);

        private:
            std::unordered_map<point3D_t, float> resolutions_;
        };
    }
} 

#endif  
