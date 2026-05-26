

#ifndef _AI3D_BASE_SIMILARITY_TRANSFORM_H_
#define _AI3D_BASE_SIMILARITY_TRANSFORM_H_

#include <vector>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "alignment.h"
#include "types.h"

namespace AI3D
{

    namespace CORE
    {
        struct RANSACOptions;
        class ATData;



        
        
        
        
        
        
        
        
        
        
        
        template <int kDim, bool kEstimateScale = true>
        class SimilarityTransformEstimator {
        public:
            typedef Eigen::Matrix<double, kDim, 1> X_t;
            typedef Eigen::Matrix<double, kDim, 1> Y_t;
            typedef Eigen::Matrix<double, kDim, kDim + 1> M_t;

            
            
            
            static const int kMinNumSamples = kDim;

            
            
            
            
            
            
            static std::vector<M_t> Estimate(const std::vector<X_t>& src,
                const std::vector<Y_t>& dst);

            
            
            
            
            
            
            
            
            
            
            
            static void Residuals(const std::vector<X_t>& src,
                const std::vector<Y_t>& dst, const M_t& matrix,
                std::vector<double>* residuals);
        };

        
        
        

        template <int kDim, bool kEstimateScale>
        std::vector<typename SimilarityTransformEstimator<kDim, kEstimateScale>::M_t>
            SimilarityTransformEstimator<kDim, kEstimateScale>::Estimate(
                const std::vector<X_t>& src, const std::vector<Y_t>& dst) {
            

            Eigen::Matrix<double, kDim, Eigen::Dynamic> src_mat(kDim, src.size());
            Eigen::Matrix<double, kDim, Eigen::Dynamic> dst_mat(kDim, dst.size());
            for (size_t i = 0; i < src.size(); ++i) {
                src_mat.col(i) = src[i];
                dst_mat.col(i) = dst[i];
            }

            const M_t model = Eigen::umeyama(src_mat, dst_mat, kEstimateScale)
                .topLeftCorner(kDim, kDim + 1);

            if (model.array().isNaN().any()) {
                return std::vector<M_t>{};
            }

            return { model };
        }

        template <int kDim, bool kEstimateScale>
        void SimilarityTransformEstimator<kDim, kEstimateScale>::Residuals(
            const std::vector<X_t>& src, const std::vector<Y_t>& dst, const M_t& matrix,
            std::vector<double>* residuals) {
            CHECK_EQ(src.size(), dst.size());

            residuals->resize(src.size());

            for (size_t i = 0; i < src.size(); ++i) {
                const Y_t dst_transformed = matrix * src[i].homogeneous();
                (*residuals)[i] = (dst[i] - dst_transformed).squaredNorm();
            }
        }

        




            
        class SimilarityTransform3
        {
        public:
            EIGEN_MAKE_ALIGNED_OPERATOR_NEW

                SimilarityTransform3();

            explicit SimilarityTransform3(const Eigen::Matrix3x4d& matrix);

            explicit SimilarityTransform3(
                const Eigen::Transform<double, 3, Eigen::Affine>& transform);

            SimilarityTransform3(const double scale, const Eigen::Vector4d& qvec,
                const Eigen::Vector3d& tvec);

            void Write(const std::string& path);

            template <bool kEstimateScale = true>
            bool Estimate(const std::vector<Eigen::Vector3d>& src,
                const std::vector<Eigen::Vector3d>& dst);

            SimilarityTransform3 Inverse() const;

            void TransformPoint(Eigen::Vector3d* xyz) const;
            void TransformPose(Eigen::Vector4d* qvec, Eigen::Vector3d* tvec) const;
            void TransformPose(Eigen::Matrix3x4d src_matrix,Eigen::Matrix3x4d& dst_matrix) const;
            Eigen::Matrix4d Matrix() const;
            double Scale() const;
            Eigen::Vector4d Rotation() const;
            Eigen::Vector3d Translation() const;

            

        private:
            Eigen::Transform<double, 3, Eigen::Affine> transform_;
        };

        
        
        
        
        
        
        bool ComputeAlignmentBetweenReconstructions(
            const AI3D::CORE::ATData& src_reconstruction,
            const AI3D::CORE::ATData& ref_reconstruction,
            const double min_inlier_observations, const double max_reproj_error,
            Eigen::Matrix3x4d* alignment);

        
        
        

        template <bool kEstimateScale>
        bool SimilarityTransform3::Estimate(const std::vector<Eigen::Vector3d>& src,
            const std::vector<Eigen::Vector3d>& dst)
        {
            const auto results =
                SimilarityTransformEstimator<3, kEstimateScale>().Estimate(src, dst);
            if (results.empty())
            {
                return false;
            }

            
            transform_.matrix().topLeftCorner<3, 4>() = results[0];

            return true;
        }
    }
}

EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION_CUSTOM(AI3D::CORE::SimilarityTransform3)

#endif  
