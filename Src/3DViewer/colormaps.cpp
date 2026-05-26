#include "3DViewer/colormaps.h"
#include "Core/Bitmap.h"

namespace AI3D
{
    namespace GUI
    {

        PointColormapBase::PointColormapBase()
            : scale(1.0f),
            min(0.0f),
            max(0.0f),
            range(0.0f),
            min_q(0.0f),
            max_q(1.0f) {}

        void PointColormapBase::UpdateScale(std::vector<float>* values) {
            if (values->empty()) 
            {
                min = 0.0f;
                max = 0.0f;
                range = 0.0f;
            }
            else 
            {
                std::sort(values->begin(), values->end());
                min = (*values)[static_cast<size_t>(min_q * (values->size() - 1))];
                max = (*values)[static_cast<size_t>(max_q * (values->size() - 1))];
                range = max - min;
            }
        }

        float PointColormapBase::AdjustScale(const float gray) {
            if (range == 0.0f) {
                return 0.0f;
            }
            else {
                const float gray_clipped = std::min(std::max(gray, min), max);
                const float gray_scaled = (gray_clipped - min) / range;
                return std::pow(gray_scaled, scale);
            }
        }

        void PointColormapPhotometric::Prepare(EIGEN_STL_UMAP(camera_t, AI3D::CORE::Camera)&
            cameras,
            EIGEN_STL_UMAP(image_t, AI3D::CORE::Image)& images,
            EIGEN_STL_UMAP(point3D_t, AI3D::CORE::Point3D)&
            points3D,
            std::vector<image_t>& reg_image_ids) {}

        Eigen::Vector3f PointColormapPhotometric::ComputeColor(
            const point3D_t point3D_id, const AI3D::CORE::Point3D& point3D) {
            return Eigen::Vector3f(point3D.GetColor(0) / 255.0f, point3D.GetColor(1) / 255.0f,
                point3D.GetColor(2) / 255.0f);
        }

        void PointColormapError::Prepare(EIGEN_STL_UMAP(camera_t, AI3D::CORE::Camera)& cameras,
            EIGEN_STL_UMAP(image_t, AI3D::CORE::Image)& images,
            EIGEN_STL_UMAP(point3D_t, AI3D::CORE::Point3D)& points3D,
            std::vector<image_t>& reg_image_ids) {
            std::vector<float> errors;
            errors.reserve(points3D.size());

            for (const auto& point3D : points3D) {
                errors.push_back(static_cast<float>(point3D.second.GetPixelRMS()));
            }

            UpdateScale(&errors);
        }

        Eigen::Vector3f PointColormapError::ComputeColor(const point3D_t point3D_id,
            const AI3D::CORE::Point3D& point3D) {
            const float gray = AdjustScale(static_cast<float>(point3D.GetPixelRMS()));
            return Eigen::Vector3f(AI3D::CORE::JetColormap::GetRed(gray), AI3D::CORE::JetColormap::GetGreen(gray),
                AI3D::CORE::JetColormap::GetBlue(gray));
        }

      

        void PointColormapTrackLen::Prepare(EIGEN_STL_UMAP(camera_t, AI3D::CORE::Camera)& cameras,
            EIGEN_STL_UMAP(image_t, AI3D::CORE::Image)& images,
            EIGEN_STL_UMAP(point3D_t, AI3D::CORE::Point3D)&
            points3D,
            std::vector<image_t>& reg_image_ids) {
            std::vector<float> track_lengths;
            track_lengths.reserve(points3D.size());

            for (const auto& point3D : points3D) {
                track_lengths.push_back(point3D.second.GetTrack().Length());
            }

            UpdateScale(&track_lengths);
        }

        Eigen::Vector3f PointColormapTrackLen::ComputeColor(const point3D_t point3D_id,
            const AI3D::CORE::Point3D& point3D) {
            const float gray = AdjustScale(point3D.GetTrack().Length());
            return Eigen::Vector3f(AI3D::CORE::JetColormap::GetRed(gray), AI3D::CORE::JetColormap::GetGreen(gray),
                AI3D::CORE::JetColormap::GetBlue(gray));
        }

        void PointColormapGroundResolution::Prepare(
            EIGEN_STL_UMAP(camera_t, AI3D::CORE::Camera)& cameras,
            EIGEN_STL_UMAP(image_t, AI3D::CORE::Image)& images,
            EIGEN_STL_UMAP(point3D_t, AI3D::CORE::Point3D)& points3D,
            std::vector<image_t>& reg_image_ids) 
        {
            std::vector<float> resolutions;
            resolutions.reserve(points3D.size());

            std::unordered_map<camera_t, float> focal_lengths;
            EIGEN_STL_UMAP(camera_t, Eigen::Vector2f) principal_points;
            for (const auto& camera : cameras) 
            {
                focal_lengths[camera.first] =
                    static_cast<float>(camera.second.GetMeanFocalLength());
                principal_points[camera.first] =
                    Eigen::Vector2f(static_cast<float>(camera.second.GetPrincipalPointX()),
                        static_cast<float>(camera.second.GetPrincipalPointY()));
            }

            EIGEN_STL_UMAP(image_t, Eigen::Vector3f) proj_centers;
            for (const auto& image : images) 
            {
                proj_centers[image.first] = image.second.GetProjectionCenter().cast<float>();
            }

            for (const auto& point3D : points3D) 
            {
                float min_resolution = std::numeric_limits<float>::max();

                const Eigen::Vector3f xyz = point3D.second.GetXYZ().cast<float>();

                for (const auto track_el : point3D.second.GetTrack().GetElements()) 
                {
                    const auto& image = images[track_el.image_id];
                    const float focal_length = focal_lengths[image.GetCameraId()];
                    const float focal_length2 = focal_length * focal_length;
                    const Eigen::Vector2f& pp = principal_points[image.GetCameraId()];

                    const Eigen::Vector2f xy =
                        image.GetPoint2D(track_el.point2D_idx).GetXY().cast<float>() - pp;

                    
                    const float pixel_radius1 = xy.norm();

                    const float x1 = xy(0) + (xy(0) < 0 ? -1.0f : 1.0f);
                    const float y1 = xy(1) + (xy(1) < 0 ? -1.0f : 1.0f);
                    const float pixel_radius2 = std::sqrt(x1 * x1 + y1 * y1);

                    
                    const float pixel_dist1 =
                        std::sqrt(pixel_radius1 * pixel_radius1 + focal_length2);
                    const float pixel_dist2 =
                        std::sqrt(pixel_radius2 * pixel_radius2 + focal_length2);

                    
                    const float dist = (xyz - proj_centers[track_el.image_id]).norm();

                    
                    const float r1 = pixel_radius1 * dist / pixel_dist1;
                    const float r2 = pixel_radius2 * dist / pixel_dist2;
                    const float dr = r2 - r1;

                    
                    
                    const float resolution = -dr * dr;

                    if (std::isfinite(resolution)) {
                        min_resolution = std::min(resolution, min_resolution);
                    }
                }

                resolutions.push_back(min_resolution);
                resolutions_[point3D.first] = min_resolution;
            }

            UpdateScale(&resolutions);
        }

        Eigen::Vector3f PointColormapGroundResolution::ComputeColor(
            const point3D_t point3D_id, const AI3D::CORE::Point3D& point3D) {
            const float gray = AdjustScale(resolutions_[point3D_id]);
            return Eigen::Vector3f(AI3D::CORE::JetColormap::GetRed(gray), AI3D::CORE::JetColormap::GetGreen(gray),
                AI3D::CORE::JetColormap::GetBlue(gray));
        }

    } 
}
