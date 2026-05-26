
#ifndef _AI3D_CORE_IMAGE_H_
#define _AI3D_CORE_IMAGE_H_

#include <string>
#include <vector>
#include <array>
#include <Eigen/Core>
#include <Constants.h>
#include "Core/Camera.h"
#include "Core/Point2d.h"

#include "Core/Alignment.h"
#include "Core/Logging.h"
#include "Core/Math.h"
#include "Core/Types.h"

#include "Core/PhotoGroup.h"

#include "Core/ExifIO.h"
#include "Core/AlgorithmBase.h"

namespace AI3D
{
	namespace CORE
	{
		
		
		
		class AI3D_API Image 
		{
		public:
			EIGEN_MAKE_ALIGNED_OPERATOR_NEW

			Image();				
			
			void SetUp(const Camera& camera);
			void TearDown();

			
			inline image_t GetImageId() const { return image_id_;};
			inline void SetImageId(const image_t image_id);

			inline const std::string& GetPath() const;
			inline std::string& GetPathMutual();
			inline void SetPath(const std::string& path);

			
			inline const std::string& GetName() const;
			inline std::string& GetNameMutual();
			inline void SetName(const std::string& name);

			
			
			inline camera_t GetCameraId() const;
			inline camera_t GetCameraIdMutual();
			inline void SetCameraId(const camera_t camera_id);
			
			inline bool HasCamera() const;
			bool IsPoseAndIntrinsicDefined(const Camera& cam) const;
			std::string GetPriviewFileFullName()const ;
			void SetPriviewFileFullName(const std::string& preview_name);
			
			void SetFixStatus(fix_e fix_status);
			
			const fix_e GetFixStatus() const;
			fix_e GetFixStatusMutual();
			
			
			bool GenPreviewImage(std::string outpath, int pre_width = PREIMG_W, int pre_height = PREIMG_H);
			static bool GenPreviewImageV2(std::string outpath,std::string imagepth,std::string &preview_name);
			static bool ExistsPreviewImage(std::string outpath, std::string imagepth, std::string& preview_name);
			bool ExistsPreviewImage(std::string outpath);

			void SetDepth(const Eigen::Vector3d& depth);
			Eigen::Vector3d GetDepth()const;
			Eigen::Vector3d& GetDepthMutual();

			
			float GetDepth(const Eigen::Vector3d& point);
			
			inline bool IsRegistered() const;
			inline void SetRegistered(const bool registered);

			
			inline point2D_t GetNumPoints2D() const;

			
			
			inline point2D_t GetNumPoints3D() const;

			inline void SetNumPoints3D(point3D_t numpoints3d);
			
			
			inline point2D_t GetNumObservations() const;
			inline void SetNumObservations(const point2D_t num_observations);

			
			inline point2D_t GetNumCorrespondences() const;
			inline void SetNumCorrespondences(const point2D_t num_observations);

			
			
			
			inline point2D_t GetNumVisiblePoints3D() const;

			

			inline point2D_t SetPoint2D(const Point2D& point2d);

	
			
			

		
	

			void SetRotationMatrix(std::array<double, 9> rotation);
			

			const Eigen::Vector3d& GetPosition() const;
			Eigen::Vector3d& GetPositionMutual() ;
			Eigen::Vector3d& GetPosition();
			double GetPosition(const size_t idx) const;
			double& GetPosition(const size_t idx);
			bool HasPosition() const;
			void SetPosition(const Eigen::Vector3d& position);

			const Eigen::Vector3d& GetPositionPrior() const;
			Eigen::Vector3d& GetPositionPrior();
			double GetPositionPrior(const size_t idx) const;
			double& GetPositionPrior(const size_t idx);
			bool HasPositionPrior() const;
			void SetPositionPrior(const Eigen::Vector3d& position);
			Eigen::Vector3d& GetPositionPriorMutual();
			const Eigen::Matrix3d& GetRotationMatrix() const;
			Eigen::Matrix3d& GetRotationMatrixMutual();
			
			void SetRotationMatrix(const Eigen::Matrix3d& position);

			const Eigen::Matrix3d& GetRotationMatrixPrior() const;
			Eigen::Matrix3d& GetRotationMatrixPriorMutual();
			
			bool HasRotationMatrix() const;					  
			bool HasRotationMatrixPrior() const;
			void SetRotationMatrixPrior(const Eigen::Matrix3d& position);
			bool HasDepth() const;
			bool HasFrustum() const;
			const std::vector<Eigen::Vector3d> GetFrustumMutual() const { return frustum_; }
			std::vector<Eigen::Vector3d>& GetFrustumMutual() { return frustum_; }
			std::string GetImageFullPath();
			
			inline const Point2D& GetPoint2D(const point2D_t point2D_idx) const;
			inline Point2D& GetPoint2DMutual(const point2D_t point2D_idx);
			inline const std::vector<Point2D>& GetPoints2D() const;
			inline std::vector<Point2D>& GetPoints2DMutual() ;

			void SetPoints2D(const std::vector<Eigen::Vector2d>& points);
			void SetPoints2D(const std::vector<Point2D>& points);
			void SetPoints2D(point2D_t point2D_idx, Eigen::Vector2d& points);

			
			
			
			point2D_t AddPoints2D(Eigen::Vector2d& points);
		
			
			bool ClearPose();
			void DeleteGCPMeasurement(point3D_t id);
			
			void DeleteGCPs();
			bool HasGCPs() const;
			bool InsideImage(const Eigen::Vector2d& point)const;
			void SetPoints2DGCPMap(const std::map<point3D_t, Eigen::Vector2d >&points_2D_gcp);
			std::map<point3D_t, Eigen::Vector2d > GetPoints2DGCPMap()const;
			void SetPoints2DGCP(point3D_t id,Eigen::Vector2d& points);
			Eigen::Vector2d GetPoints2DGCP(point3D_t id)const;
			
			bool ExistPoints2DGCP(point3D_t id);
			
			void SetPoint3DForPoint2D(const point2D_t point2D_idx,
				const point3D_t point3D_id);
			

			
			void DeleteUserPts();
			bool HasUserPts() const;
			
			void SetPoints2DUserPtMap(const std::map<point3D_t, Eigen::Vector2d >& points_2D_userpt);
			std::map<point3D_t, Eigen::Vector2d > GetPoints2DUserPtMap()const;
			void SetPoints2DUserPt(point3D_t id, Eigen::Vector2d& points);
			Eigen::Vector2d GetPoints2DUserPt(point3D_t id)const;
			void DeleteUserPtMeasurement(point3D_t id);
			bool ExistPoints2DUserPt(point3D_t id);



			
			void ResetPoint3DForPoint2D(const point2D_t point2D_idx);

			
			
			inline bool IsPoint3DVisible(const point2D_t point2D_idx) const;
			bool HasPoints() const;
			
			bool HasPoint3D(const point3D_t point3D_id) const;
			
			
			
			
			void IncrementCorrespondenceHasPoint3D(const point2D_t point2D_idx);

			
			
			
			
			
			void DecrementCorrespondenceHasPoint3D(const point2D_t point2D_idx);

			Eigen::Matrix3x4d InverseProjectionMatrix() const;

			

			
			Eigen::Vector3d GetProjectionCenter() const;
			
			const Eigen::Matrix3x4d GetProjectionMatrix() const;


			
			
			Eigen::Vector3d ViewingDirection() const;
			Eigen::Matrix3d ArrayToRotationMatrix(const std::array<double, 9>& rotation_matrix) const;
			std::array<double, 9> RotationMatrixToArray(const Eigen::Matrix3d& rotation_matrix) const;
			
			static const int kNumPoint3DVisibilityPyramidLevels;
			void SetRotationMatrixPrior(std::array<double, 9> rotation);
			
			
			
			void SetPhotoGroupID(const group_t& group_id);
			const group_t GetPhotoGroupID() const;
			group_t GetPhotoGroupIDMutual() ;
			

			
			ExifInfo GetExifinfo()const;
			ExifInfo& GetExifinfoMutual();
			
			XmpData GetXmpData()const;
			XmpData& GetXmpDataMutual();
			void SetXmpData(const XmpData& xmpdata);
			void SetExifinfo(const ExifInfo& exifinfo);
			
			short ParseOrientation(const std::string& imagepath);

			void SetPriorSrs(const srs_s srs) ;
			const srs_s GetPriorSrs() const;
			srs_s GetPriorSrsMutual();

			
			inline const Eigen::Vector3d& GetColorParam() const;
			inline Eigen::Vector3d& GetColorParamMutual();
			inline double GetColorParam(const size_t idx) const;
			inline double& GetColorParamMutual(const size_t idx);
			inline void SetColorParam(const Eigen::Vector3d& colorparam);
		
			void SetDewrapFlag( bool dewrap_flag);
			bool GetDewrapFlag()const;
			std::map<point3D_t, Eigen::Vector2d>& GetGcpsPoint2DMutual();	
			std::map<point3D_t, Eigen::Vector2d>& GetUserPtsPoint2DMutual();
			bool HasPreCalibParams()const;

			int GetWidth()const;
			void SetWidth(int width);
			int& GetWidthMutual();

			int GetHeight()const;
			void SetHeight(int height);
			int& GetHeightMutual();

			

			int ParseExif(const std::string& outPath = "");
			bool IsVisible(const Eigen::Vector3d& pt, const Eigen::Matrix3d& cam_mat) const
			{
				
				auto pmat = GetProjectionMatrix();
				
				Eigen::Vector3d obr = AlgorithmBase::TransformPointW2Iz(pmat, cam_mat,pt);
				return !(obr(0) < 0.1 || obr(1) < 0.1 ||
					obr(0) > width_ -0.1|| obr(1) > height_-0.1 || obr(2) < 0.1);

				
			}
			
			
			bool IsPoseCompleted() const;
			const bool HasColorParams() const;

		private:
			
			
			image_t image_id_;

			group_t group_id_ ;
			
			std::string name_ ;
			std::string path_;
			std::string preview_name_;				 
			
			
			camera_t camera_id_ ;

			
			bool registered_ ;

			
			
			point2D_t num_points3D_ ;

			
			
			point2D_t num_points3DGCP_;
			point2D_t num_points3DUserPt_;
			
			
			point2D_t num_observations_ ;

			
			point2D_t num_correspondences_ ;

			
			
			
			point2D_t num_visible_points3D_ ;

			
			
			

			Eigen::Vector3d center_{ -DBL_MAX,-DBL_MAX,-DBL_MAX };
			Eigen::Matrix3d rotation_matrix_ = Eigen::Matrix3d::Zero();

			srs_s prior_srs_def_ ;
			Eigen::Vector3d center_prior_{ -DBL_MAX,-DBL_MAX,-DBL_MAX };

			Eigen::Matrix3d rotation_matrix_prior_ = Eigen::Matrix3d::Zero();

			Eigen::Vector3d depth_{ -DBL_MAX,-DBL_MAX,-DBL_MAX };
		
			
			
			std::vector<Point2D> points2D_;

			
			std::map<point3D_t, Eigen::Vector2d > points2D_gcp_;
			std::map<point3D_t, Eigen::Vector2d > points2D_userpt_;
			
			std::vector<image_t> num_correspondences_have_point3D_;
			
			std::vector<image_t> num_correspondences_have_point3DGCP_;
			std::vector<image_t> num_correspondences_have_point3DUserPt_;
			
			
			
		  
			int width_;
			int height_;

			 
			 ExifInfo exifinfo_;
			 
			 XmpData xmpdata_;
			 
			 Eigen::Vector3d colorparam_ = Eigen::Vector3d{1.,1.,1.};
			 std::vector<Eigen::Vector3d> frustum_;
			 
			 bool dewrap_flag_ = false;
			 fix_e fix_status_ = fix_e::EOE_FREE;
		};

		



		
	}
} 

EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION_CUSTOM(AI3D::CORE::Image)

#endif  
