#include "Core/SimilarityTransform.h"
#include "Core/Image.h"
#include "Core/Ransac.h"
#include "Core/Loransac.h"
#include "Core/ATData.h"
#include "Core/File.h"

#include <fstream>

namespace AI3D {
    namespace CORE
    {

        struct ReconstructionAlignmentEstimator
        {
            static const int kMinNumSamples = 3;

            typedef const Image* X_t;
            typedef const Image* Y_t;
            typedef Eigen::Matrix3x4d M_t;

            void SetMaxReprojError(const double max_reproj_error)
            {
                max_squared_reproj_error_ = max_reproj_error * max_reproj_error;
            }

            void SetReconstructions(const ATData* reconstruction1,
                const ATData* reconstruction2)
            {
                
                reconstruction1_ = reconstruction1;
                reconstruction2_ = reconstruction2;
            }

            
            std::vector<M_t> Estimate(const std::vector<X_t>& images1,
                const std::vector<Y_t>& images2) const
            {
                

                std::vector<Eigen::Vector3d> proj_centers1(images1.size());
                std::vector<Eigen::Vector3d> proj_centers2(images2.size());
                for (size_t i = 0; i < images1.size(); ++i) {
                    
                    proj_centers1[i] = images1[i]->GetPosition();
                    proj_centers2[i] = images2[i]->GetPosition();
                }

                SimilarityTransform3 tform12;
                tform12.Estimate(proj_centers1, proj_centers2);

                return { tform12.Matrix().topRows<3>() };
            }

            
            
            
            
            
            void Residuals(const std::vector<X_t>& images1,
                const std::vector<Y_t>& images2, const M_t& alignment12,
                std::vector<double>* residuals) const
            {
                

                const Eigen::Matrix3x4d alignment21 =
                    SimilarityTransform3(alignment12).Inverse().Matrix().topRows<3>();

                residuals->resize(images1.size());

                for (size_t i = 0; i < images1.size(); ++i)
                {
                    const auto& image1 = *images1[i];
                    const auto& image2 = *images2[i];

                    

                    const auto& camera1 = reconstruction1_->GetCamera(image1.GetCameraId());
                    const auto& camera2 = reconstruction2_->GetCamera(image2.GetCameraId());

                    const Eigen::Matrix3x4d proj_matrix1 = image1.GetProjectionMatrix();
                    const Eigen::Matrix3x4d proj_matrix2 = image2.GetProjectionMatrix();

                    

                    size_t num_inliers = 0;
                    size_t num_common_points = 0;

                    
                    int count = 0;
                    std::set<point2D_t> pointidx1, pointidx2;
                    std::set<std::pair<point2D_t, point2D_t>> pointidx;
                    
                    for (point2D_t point2D_idx1 = 0; point2D_idx1 < image1.GetNumPoints2D();
                        ++point2D_idx1)
                    {
                        const auto& point2D1 = image1.GetPoint2D(point2D_idx1);
                        if (!point2D1.HasPoint3D()) {
                            continue;
                        }
                        for (point2D_t point2D_idx2 = 0; point2D_idx2 < image2.GetNumPoints2D();
                            ++point2D_idx2)
                        {
                            const auto& point2D2 = image2.GetPoint2D(point2D_idx2);
                            if (!point2D2.HasPoint3D()) {
                                continue;
                            }
                            double dist = (point2D1.GetXY() - point2D2.GetXY()).norm();
                            if (dist < 0.3333)
                            {
                                count++;
                                
                                const auto& ret1 = pointidx1.insert(point2D_idx1);
                                const auto& ret2 = pointidx2.insert(point2D_idx2);
                                if (ret1.second && ret2.second)
                                {
                                    pointidx.insert(std::make_pair(point2D_idx1, point2D_idx2));
                                }
                                break;
                            }
                        }
                    }
                    

                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    std::vector<std::pair<point2D_t, point2D_t>> pointidxvec;
                    pointidxvec.assign(pointidx.begin(), pointidx.end());

                    for (auto iter : pointidxvec)
                    {

                        num_common_points += 1;
                        const auto& point2D1 = image1.GetPoint2D(iter.first);
                        const auto& point2D2 = image2.GetPoint2D(iter.second);
                        
                       
                        const Eigen::Vector3d xyz12 =
                            alignment12 *
                            reconstruction1_->GetPoint3D(point2D1.GetPoint3DId()).GetXYZ().homogeneous();
                        double error12 = AlgorithmBase::CalculateSquaredReprojectionError(point2D2.GetXY(), xyz12,
                            proj_matrix2, camera2);
                        
                        if (error12 >
                            max_squared_reproj_error_) 
                        {
                            continue;
                        }

                        
                        const Eigen::Vector3d xyz21 =
                            alignment21 *
                            reconstruction2_->GetPoint3D(point2D2.GetPoint3DId()).GetXYZ().homogeneous();
                        double error21 = AlgorithmBase::CalculateSquaredReprojectionError(point2D1.GetXY(), xyz21,
                            proj_matrix1, camera1);
                        
                        if (error21 >   max_squared_reproj_error_) {
                            continue;
                        }

                        num_inliers += 1;
                    }

                    if (num_common_points == 0)
                    {
                        (*residuals)[i] = 1.0;
                    }
                    else
                    {
                        const double negative_inlier_ratio =
                            1.0 - static_cast<double>(num_inliers) /
                            static_cast<double>(num_common_points);
                        (*residuals)[i] = negative_inlier_ratio * negative_inlier_ratio;
                    }


                    
                    
                    
                    

                    
                    
                    
                    

                    
                    
                    
                    

                    

                    
                    
                    
                    
                    
                    
                    
                    
                    

                    
                    
                    
                    
                    
                    
                    
                    
                    

                    
                    

                    
                    
                    
                    
                    
                    
                    
                    
                    

                }
            }
        private:
            double max_squared_reproj_error_ = 0.0;
            const ATData* reconstruction1_ = nullptr;
            const ATData* reconstruction2_ = nullptr;
        };

    

        SimilarityTransform3::SimilarityTransform3()
            : SimilarityTransform3(1, AlgorithmBase::ComposeIdentityQuaternion(),
                Eigen::Vector3d(0, 0, 0)) {}

        SimilarityTransform3::SimilarityTransform3(const Eigen::Matrix3x4d& matrix)
        {
            transform_.matrix().topLeftCorner<3, 4>() = matrix;
        }

        SimilarityTransform3::SimilarityTransform3(
            const Eigen::Transform<double, 3, Eigen::Affine>& transform)
            : transform_(transform) {}

        SimilarityTransform3::SimilarityTransform3(const double scale,
            const Eigen::Vector4d& qvec,
            const Eigen::Vector3d& tvec) 
        {
            Eigen::Matrix4d matrix = Eigen::MatrixXd::Identity(4, 4);
            matrix.topLeftCorner<3, 4>() = AlgorithmBase::ComposeProjectionMatrix(qvec, tvec);
            matrix.block<3, 3>(0, 0) *= scale;
            transform_.matrix() = matrix;
        }

        void SimilarityTransform3::Write(const std::string& path) {
            std::ofstream file = File::OpenOfstreamUtf8(path, std::ios::trunc);
            
            
            file.precision(17);
            file << transform_.matrix() << std::endl;
        }

        SimilarityTransform3 SimilarityTransform3::Inverse() const {
            return SimilarityTransform3(transform_.inverse());
        }

        void SimilarityTransform3::TransformPoint(Eigen::Vector3d* xyz) const {
            *xyz = transform_ * *xyz;
        }

        void SimilarityTransform3::TransformPose(Eigen::Vector4d* qvec,
            Eigen::Vector3d* tvec) const {
            
            
            
            
            
            
            
            
            
            
            
            
            

            Eigen::Matrix4d src_matrix = Eigen::MatrixXd::Identity(4, 4);
            src_matrix.topLeftCorner<3, 4>() = AlgorithmBase::ComposeProjectionMatrix(*qvec, *tvec);
            Eigen::Matrix4d dst_matrix =
                src_matrix.matrix() * transform_.inverse().matrix();
            dst_matrix *= Scale();

            *qvec = AlgorithmBase::RotationMatrixToQuaternion(dst_matrix.block<3, 3>(0, 0));
            *tvec = dst_matrix.block<3, 1>(0, 3);
        }


        void SimilarityTransform3::TransformPose(Eigen::Matrix3x4d src_matrix1, Eigen::Matrix3x4d& dst_matrix) const {
            
            
            
            
            
            
            
            
            
            
            
            
            

            Eigen::Matrix4d src_matrix = Eigen::MatrixXd::Identity(4, 4);
            src_matrix.topLeftCorner<3, 4>() = src_matrix1;
            Eigen::Matrix4d dst_matrix1 =
                src_matrix.matrix() * transform_.inverse().matrix();
            dst_matrix1 *= Scale();
            dst_matrix = dst_matrix1.topLeftCorner<3, 4>();
           
        }

        Eigen::Matrix4d SimilarityTransform3::Matrix() const
        {
            return transform_.matrix();
        }

        double SimilarityTransform3::Scale() const 
        {
            return Matrix().block<1, 3>(0, 0).norm();
        }

        Eigen::Vector4d SimilarityTransform3::Rotation() const 
        {
            return AlgorithmBase::RotationMatrixToQuaternion(Matrix().block<3, 3>(0, 0) / Scale());
        }

        Eigen::Vector3d SimilarityTransform3::Translation() const 
        {
            return Matrix().block<3, 1>(0, 3);
        }

    

        bool ComputeAlignmentBetweenReconstructions(
            const ATData& src_reconstruction,
            const ATData& ref_reconstruction,
            const double min_inlier_observations, const double max_reproj_error,
            Eigen::Matrix3x4d* alignment) {
            

            RANSACOptions ransac_options;
            ransac_options.max_error = 1.0 - min_inlier_observations;
            ransac_options.min_inlier_ratio = 0.2;

            LORANSAC<ReconstructionAlignmentEstimator, ReconstructionAlignmentEstimator>
                ransac(ransac_options);
            ransac.estimator.SetMaxReprojError(max_reproj_error);
            ransac.estimator.SetReconstructions(&src_reconstruction, &ref_reconstruction);
            ransac.local_estimator.SetMaxReprojError(max_reproj_error);
            ransac.local_estimator.SetReconstructions(&src_reconstruction,
                &ref_reconstruction);
            std::set<image_t> common_image_ids1, common_image_ids2;
            src_reconstruction.FindCommonRegImages(ref_reconstruction, common_image_ids1, common_image_ids2);
            
              

            if (common_image_ids1.size() < 3) {
                return false;
            }

            std::vector<const Image*> src_images(common_image_ids1.size());
            std::vector<const Image*> ref_images(common_image_ids2.size());
            std::vector<image_t> comids1_vec, comids2_vec;
            comids1_vec.assign(common_image_ids1.begin(), common_image_ids1.end());
            comids2_vec.assign(common_image_ids2.begin(), common_image_ids2.end());


           
            for (size_t i = 0; i < comids1_vec.size(); ++i)
            {
                if (src_reconstruction.GetImages().count(comids1_vec[i]) && ref_reconstruction.GetImages().count(comids2_vec[i]))
                {
                    
                    
                    src_images[i]=( &src_reconstruction.GetImage(comids1_vec[i]));
                    ref_images[i]=(&ref_reconstruction.GetImage(comids2_vec[i]));
                }
            }

            const auto report = ransac.Estimate(src_images, ref_images);

            if (report.success) {
                *alignment = report.model;
            }

            return report.success;
            
        }
    }
}  
