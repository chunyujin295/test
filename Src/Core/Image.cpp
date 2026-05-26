
#include "Core/Image.h"
#include "Core/CameraDatabase.h"
#include "Core/String.h"
#include "Core/File.h"
#include <filesystem>
#include <vector>
#include "Core/Bitmap.h"
#include "exiv2/preview.hpp"
#include "Core/ReturnCode.h"

namespace AI3D
{
    namespace CORE
    {
        namespace {

            static const double kNaN = std::numeric_limits<double>::quiet_NaN();

        }  

        const int Image::kNumPoint3DVisibilityPyramidLevels = 6;
       
        Image::Image()
            : image_id_(kInvalidImageId),
            group_id_(kInvalidGroupId),
            name_(""),
            path_(""),
            camera_id_(kInvalidCameraId),
            registered_(false),
            num_points3D_(0),
            num_observations_(0),
            num_correspondences_(0),
            num_visible_points3D_(0)
            
            
            
            
        {
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    rotation_matrix_prior_(i,j) = 0;
                    rotation_matrix_(i, j) = 0;
                }
            }
        }
        
        bool Image::IsPoseCompleted() const
        {
            return (HasPosition() && HasRotationMatrix()) || (HasPositionPrior() && HasRotationMatrixPrior());
        }

        void Image::SetFixStatus(fix_e fix_status)
        {
            fix_status_ = fix_status;
        }
        const fix_e Image::GetFixStatus() const
        {
            return fix_status_;
        };
        fix_e Image::GetFixStatusMutual()
        {
            return fix_status_;
        };
        void Image::SetUp(const Camera& camera) 
        {
            if (!CHECK_OPTION_EQ(camera_id_, camera.GetCameraId()))
                return;
            width_ = camera.GetWidth();
            height_ = camera.GetHeight();
            
              
        }

        void Image::TearDown() 
        {
            
        }

        

        void Image::SetPoints2D(const std::vector<Eigen::Vector2d>& points) 
        {
            CHECK(points2D_.empty());
            points2D_.resize(points.size());
            num_correspondences_have_point3D_.resize(points.size(), 0);
            for (point2D_t point2D_idx = 0; point2D_idx < points.size(); ++point2D_idx) 
            {
                points2D_[point2D_idx].SetXY(points[point2D_idx]);
            }
        }

        void Image::SetPoints2D(point2D_t point2D_idx, Eigen::Vector2d& points) 
        {
            points2D_.resize(std::max(size_t(point2D_idx + 1), points2D_.size()));
            num_correspondences_have_point3D_.resize(std::max(size_t(point2D_idx + 1), points2D_.size()), 0);
            points2D_[point2D_idx].SetXY(points);
        }

        point2D_t Image::AddPoints2D(Eigen::Vector2d& points)
        {
            AI3D::CORE::Point2D point;
            point.SetXY(points);
            points2D_.push_back(point);
            num_correspondences_have_point3D_.resize(points2D_.size(), 0);
            return point2D_t(points2D_.size() - 1);
        }
        bool Image::ClearPose()
        {
           
            Eigen::Vector3d center{ -DBL_MAX ,-DBL_MAX ,-DBL_MAX };
            center_= center;
            rotation_matrix_ = Eigen::Matrix3d::Zero();
            registered_ = false;
           
            return true;
        }
        bool Image::HasUserPts() const
        {
            return points2D_userpt_.size() > 0;
        }
        bool Image::HasPoints() const
        {
            return !points2D_.empty();
        }

        bool Image::HasGCPs() const
        {
            return points2D_gcp_.size() > 0;
        }

        void Image::DeleteGCPMeasurement(point3D_t id)
        {
            points2D_gcp_.erase(id);
           
        }
        void Image::DeleteGCPs()
        {
            points2D_gcp_.clear();
        }
        void Image::DeleteUserPtMeasurement(point3D_t id)
        {
            points2D_userpt_.erase(id);

        }
        void Image::DeleteUserPts()
        {
            points2D_userpt_.clear();
        }

        bool Image::InsideImage(const Eigen::Vector2d& point)const
        {
            return (point.x() >= 0.5 && point.x() <= (double)width_-0.5
                && point.y() <= (double)height_-0.5 && point.y() >= 0.5);
        }
        void Image::SetPoints2DGCPMap(const std::map<point3D_t, Eigen::Vector2d >& points_2D_gcp)
        {
            points2D_gcp_ = points_2D_gcp;
        }
        std::map<point3D_t, Eigen::Vector2d > Image::GetPoints2DGCPMap()const
        {
            return points2D_gcp_;
        }
        void Image::SetPoints2DGCP(point3D_t id, Eigen::Vector2d& point)
        {
            if (InsideImage(point))
            {
                points2D_gcp_[id] = point;
            }
        }


        Eigen::Vector2d Image::GetPoints2DGCP(point3D_t id)const
        {
            if (points2D_gcp_.count(id) )
            {
                if (InsideImage(points2D_gcp_.at(id)))
                {
                    return  points2D_gcp_.at(id);
                }
            }
            return   Eigen::Vector2d{-DBL_MAX,-DBL_MAX};
        }
        bool Image::ExistPoints2DGCP(point3D_t id)
        {
            return points2D_gcp_.find(id) != points2D_gcp_.end();
        }
      

        void Image::SetPoints2D(const std::vector<Point2D>& points)
        {
            CHECK(points2D_.empty());
            points2D_ = points;
            num_correspondences_have_point3D_.resize(points.size(), 0);
        }

        std::map<point3D_t, Eigen::Vector2d>& Image::GetGcpsPoint2DMutual()
        {
            return points2D_gcp_;
        }
        std::map<point3D_t, Eigen::Vector2d>& Image::GetUserPtsPoint2DMutual()
        {
            return points2D_userpt_;
        }
        void Image::SetPoints2DUserPtMap(const std::map<point3D_t, Eigen::Vector2d >& points_2D_userpt)
        {
            points2D_userpt_ = points_2D_userpt;
        }
        std::map<point3D_t, Eigen::Vector2d > Image::GetPoints2DUserPtMap()const
        {
            return points2D_userpt_;
        }
        void Image::SetPoints2DUserPt(point3D_t id, Eigen::Vector2d& point)
        {
            if (InsideImage(point))
            {
                points2D_userpt_[id] = point;
            }
        }


        Eigen::Vector2d Image::GetPoints2DUserPt(point3D_t id)const
        {
            if (points2D_userpt_.count(id))
            {
                if (InsideImage(points2D_userpt_.at(id)))
                {
                    return  points2D_userpt_.at(id);
                }
            }
            return   Eigen::Vector2d{ -DBL_MAX,-DBL_MAX };
        }
        bool Image::ExistPoints2DUserPt(point3D_t id)
        {
            return points2D_userpt_.find(id) != points2D_userpt_.end();
        }

        bool Image::HasPreCalibParams()const
        {
            if (xmpdata_.pre_calib_params.empty())
            {
                return false;
            }
            return std::all_of(std::begin(xmpdata_.pre_calib_params), std::end(xmpdata_.pre_calib_params), [](double val)
                {
                return !std::isnan(val); 
                });
        }

        void Image::SetPoint3DForPoint2D(const point2D_t point2D_idx,
            const point3D_t point3D_id) 
        {
            if (!CHECK_OPTION_NE(point3D_id, kInvalidPoint3DId))
                return;
            Point2D& point2D = points2D_.at(point2D_idx);
            if (!point2D.HasPoint3D()) 
            {
                num_points3D_ += 1;
            }
            point2D.SetPoint3DId(point3D_id);
        }
       

        void Image::ResetPoint3DForPoint2D(const point2D_t point2D_idx)
        {
            Point2D& point2D = points2D_.at(point2D_idx);
            if (point2D.HasPoint3D()) 
            {
                point2D.SetPoint3DId(kInvalidPoint3DId);
                num_points3D_ -= 1;
            }
        }

        bool Image::HasPoint3D(const point3D_t point3D_id) const 
        {
            return std::find_if(points2D_.begin(), points2D_.end(),
                [point3D_id](const Point2D& point2D)
                {
                    return point2D.GetPoint3DId() == point3D_id;
                }) != points2D_.end();
        }

        

        void Image::IncrementCorrespondenceHasPoint3D(const point2D_t point2D_idx) 
        {
            const Point2D& point2D = points2D_.at(point2D_idx);

            num_correspondences_have_point3D_[point2D_idx] += 1;
            if (num_correspondences_have_point3D_[point2D_idx] == 1) 
            {
                num_visible_points3D_ += 1;
            }

            

            assert(num_visible_points3D_ <= num_observations_);
        }

        void Image::DecrementCorrespondenceHasPoint3D(const point2D_t point2D_idx) 
        {
            const Point2D& point2D = points2D_.at(point2D_idx);

            num_correspondences_have_point3D_[point2D_idx] -= 1;
            if (num_correspondences_have_point3D_[point2D_idx] == 0)
            {
                num_visible_points3D_ -= 1;
            }

            

            assert(num_visible_points3D_ <= num_observations_);
        }

        Eigen::Matrix3x4d Image::InverseProjectionMatrix() const 
        {
            Eigen::Matrix3x4d mat = AlgorithmBase::ComposeProjectionMatrix(rotation_matrix_, center_).eval();
            Eigen::Matrix3x4d mat_inv = AlgorithmBase::InvertProjectionMatrix(mat);
            return mat_inv;
        }

        const Eigen::Matrix3x4d Image::GetProjectionMatrix() const
        {
            Eigen::Matrix3x4d mat = AlgorithmBase::ComposeProjectionMatrix(rotation_matrix_, center_);
            return mat;
        };

  

        void Image::SetPhotoGroupID(const group_t& group_id)
        {
            group_id_ = group_id;
        }
        const group_t Image::GetPhotoGroupID() const
        {
            return group_id_;
        }

         group_t Image::GetPhotoGroupIDMutual() 
        {
            return group_id_;
        }

        ExifInfo Image::GetExifinfo()const
        {
            return exifinfo_;
        }

        ExifInfo& Image::GetExifinfoMutual()
        {
            return exifinfo_;
        }

        
        XmpData Image::GetXmpData()const
        {
            return xmpdata_;
        }
        XmpData& Image::GetXmpDataMutual()
        {
            return xmpdata_;
        }
        void Image::SetXmpData(const XmpData& xmpdata)
        {
            xmpdata_ = xmpdata;
        }

        void Image::SetExifinfo(const ExifInfo &exifinfo)
        {
            exifinfo_ = exifinfo;
        }


         Eigen::Matrix3d Image::ArrayToRotationMatrix(const std::array<double, 9>& rotation_matrix) const
        {

             Eigen::Matrix3d rot;
             for (int i = 0; i < 3; i++)
             {
                 for (int j = 0; j < 3; j++)
                 {
                     rot(i, j) = rotation_matrix[i * 3 + j];
                 }
             }
             return rot;
        }


         std::array<double, 9> Image::RotationMatrixToArray(const Eigen::Matrix3d& rotation_matrix) const
         {

             std::array<double, 9> rot;
             for (int i = 0; i < 3; i++)
             {
                 for (int j = 0; j < 3; j++)
                 {
                      rot[i * 3 + j] = rotation_matrix(i, j);
                 }
             }
             return rot;
         }

  

     

        Eigen::Vector3d Image::GetProjectionCenter() const 
        {
            return center_;
        }


        void Image::SetDepth(const Eigen::Vector3d& depth)
        {
            depth_ = depth;
        }
        Eigen::Vector3d Image::GetDepth()const
        {
            return depth_;
        }
        Eigen::Vector3d& Image::GetDepthMutual()
        {
            return depth_;
        }
        
        Eigen::Vector3d Image::ViewingDirection() const 
        {
            return GetRotationMatrix().row(2);
        }

        
        
        
        

        void Image::SetImageId(const image_t image_id) 
        { 
            image_id_ = image_id;
        }

        const std::string& Image::GetName() const
        { 
            return name_; 
        }

        std::string& Image::GetNameMutual() 
        { 
            return name_; 
        }

        void Image::SetName(const std::string& name) 
        { 
            name_ = name;
        }

        bool Image::IsRegistered() const 
        { 
            return registered_; 
        }

        void Image::SetRegistered(const bool registered) 
        {
            registered_ = registered; 
        }

        point2D_t Image::GetNumPoints2D() const 
        {
            return static_cast<point2D_t>(points2D_.size());
        }

        point2D_t Image::GetNumPoints3D() const 
        { 
            return num_points3D_; 
        }

        void Image::SetNumPoints3D(point3D_t numpoints3d)
        {
            num_points3D_ = numpoints3d;
        }

        point2D_t Image::GetNumObservations() const 
        { 
            return num_observations_; 
        }

        void Image::SetNumObservations(const point2D_t num_observations) 
        {
            num_observations_ = num_observations;
        }

        point2D_t Image::GetNumCorrespondences() const 
        { 
            return num_correspondences_;
        }

        void Image::SetNumCorrespondences(const point2D_t num_correspondences) 
        {
            num_correspondences_ = num_correspondences;
        }

        point2D_t Image::GetNumVisiblePoints3D() const 
        { 
            return num_visible_points3D_;
        }      
        const bool Image::HasColorParams() const
        {
            double e1 = 1.0;
            double e0 = 0.;
            double e2 = 1e-3;
            bool allisone = (fabs(colorparam_.x() - e1) < e2 && fabs(colorparam_.y() - e1) < e2 && fabs(colorparam_.z() - e1) < e2);
            bool alliszero = (fabs(colorparam_.x() - e0) < e2 && fabs(colorparam_.y() - e0) < e2 && fabs(colorparam_.z() - e0) < e2);
            
            return !(allisone || alliszero);
        }

        const Point2D& Image::GetPoint2D(const point2D_t point2D_idx) const 
        {
            return points2D_.at(point2D_idx);
        }

        Point2D& Image::GetPoint2DMutual(const point2D_t point2D_idx) 
        {
            return points2D_.at(point2D_idx);
        }

        srs_s Image::GetPriorSrsMutual()
        {
            return prior_srs_def_;
        }

        const std::vector<Point2D>& Image::GetPoints2D() const
        {
            return points2D_;
        }
        std::vector<Point2D>& Image::GetPoints2DMutual()
        {
            return points2D_;
        }

        bool Image::IsPoint3DVisible(const point2D_t point2D_idx) const
        {
            return num_correspondences_have_point3D_.at(point2D_idx) > 0;
        }

        inline camera_t Image::GetCameraId() const { return camera_id_; }
        inline camera_t Image::GetCameraIdMutual()  { return camera_id_; }
        inline void Image::SetCameraId(const camera_t camera_id) 
        {
            if (!CHECK_OPTION_NE(camera_id, kInvalidCameraId))
                return;
            camera_id_ = camera_id;
        }

        inline bool Image::HasCamera() const { return camera_id_ != kInvalidCameraId; }


        bool Image::IsPoseAndIntrinsicDefined(const Camera& camera) const
        {
            return (HasPosition() && HasRotationMatrix()
                && HasCamera()&& camera.GetCameraId() == camera_id_
                && camera.HasValidParams());
           
        }

		bool Image::GenPreviewImage(std::string outpath, int _pre_width, int _pre_height)
		{
            outpath = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(outpath));
			Bitmap bitmap;
			
			int pre_width = 80;
			int pre_height = 60;

            std::string imagepath = GetPath();
            imagepath = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(imagepath));

			imagepath = File::JoinPaths(imagepath, GetName());

			if (!File::IsFileExistent(imagepath))
			{
				return false;
			}

			std::string hashfile = String::ToSHA256(imagepath);

			std::string hash_path = hashfile.substr(0, 2);

			std::string hash_filename = hashfile.substr(2) ;

          
           
			std::string hash_filepath = File::JoinPaths(outpath, hash_path, hash_filename);

			if (File::IsFileExistent(hash_filepath))
			{
				preview_name_ = hash_filepath;
				return true;
			}
			if (File::IsFileExistent(hash_filepath + ".jpg"))
			{
				preview_name_ = hash_filepath + ".jpg";
				return false;
			}

			
			
			
			
			
			
			
   
   
			
			
            
            
            

			std::vector<unsigned char> exivHostFileBytes;
			if (!File::ReadBinaryFileUtf8(imagepath, exivHostFileBytes) || exivHostFileBytes.empty())
			{
				return false;
			}

            Exiv2::Image::UniquePtr imgPtr;
            try
            {
                imgPtr = Exiv2::ImageFactory::open(
					reinterpret_cast<const Exiv2::byte*>(exivHostFileBytes.data()),
					exivHostFileBytes.size());
            }
            catch (Exiv2::Error& e)
            {
                LOGE(String::StringPrintf("Caught Exiv2 exception %s", e.what()));
                return false;
            }
			if (!imgPtr) {
				LOGE(String::StringPrintf("Failed to open image: %s", imagepath.c_str()));
				return false;
			}
            imgPtr->readMetadata();

			bool isThumbnailSavedSuccessfully = false;
            
			Exiv2::PreviewManager previewmanager(*imgPtr);
			Exiv2::PreviewPropertiesList previewList = previewmanager.getPreviewProperties();
			if (!previewList.empty())
			{ 
				Exiv2::PreviewImage preview = previewmanager.getPreviewImage(
					previewList[0]);
				File::CreateDirIfNotExists(File::GetParentDir(hash_filepath));
                
                preview.writeFile(hash_filepath);

				hash_filepath += preview.extension().c_str();
				preview_name_ = hash_filepath;
				isThumbnailSavedSuccessfully = true;
			}

			if (!isThumbnailSavedSuccessfully)
			{
				
				
				bool ret = bitmap.Read(imagepath);
				if (!ret)
				{
					return false;
				}
				
			  
				bitmap.Rescale(pre_width, pre_height);
				File::CreateDirIfNotExists(File::GetParentDir(hash_filepath));

				ret = bitmap.Write(hash_filepath, FREE_IMAGE_FORMAT::FIF_JPEG);
				if (!ret)
				{
					
					bitmap.Deallocate();
					return false;
				}
              
				
				bitmap.Deallocate();
				preview_name_ = hash_filepath;
			}
			return true;

        }
		 bool Image::ExistsPreviewImage(std::string _outpath)
         {
             
                          
                          
                          
            
             std::string outpathtemp, lastdir;
             AI3D::CORE::File::GetLastSecondDir(_outpath, outpathtemp, lastdir);
             std::string outpath = File::JoinPaths(outpathtemp, "previews");
             File::CreateDirIfNotExists(outpath);

            
             std::string imagepath = path_ + "/" + name_;
			 if (!File::IsFileExistent(imagepath))
             {
                 return false;
             }

             std::string hashfile = String::ToSHA256(imagepath);

             std::string hash_path = hashfile.substr(0, 2);

             std::string hash_filename = hashfile.substr(2) ;

             std::string hash_filepath = File::JoinPaths(outpath, hash_path, hash_filename);

             if (File::IsFileExistent(hash_filepath))
             {
                 preview_name_ = hash_filepath;
                 return true;
             }

             if (File::IsFileExistent(hash_filepath + ".jpg"))
             {
                 preview_name_ = hash_filepath + ".jpg";

                 

                 return true;
             }

             return false;
         }

         bool Image::ExistsPreviewImage(std::string _outpath, std::string imagepath, std::string& preview_name_)
         {

             
             
             

            
             std::string outpathtemp, lastdir;
             AI3D::CORE::File::GetLastSecondDir(_outpath, outpathtemp, lastdir);
             std::string outpath = File::JoinPaths(outpathtemp, "previews");
             File::CreateDirIfNotExists(outpath);


             if (!File::IsFileExistent(imagepath))
             {
                 return false;
             }

             std::string hashfile = String::ToSHA256(imagepath);

             std::string hash_path = hashfile.substr(0, 2);

             std::string hash_filename = hashfile.substr(2) ;

             std::string hash_filepath = File::JoinPaths(outpath, hash_path, hash_filename);

             if (File::IsFileExistent(hash_filepath))
             {
                 preview_name_ = hash_filepath;
                 return true;
             }

             if (File::IsFileExistent(hash_filepath + ".jpg"))
             {
                 preview_name_ = hash_filepath + ".jpg";

                 

                 return true;
             }

             return false;
         }

         bool Image::GenPreviewImageV2(std::string _outpath,std::string imagepath,std::string & preview_name_)
         {
             std::string outpathtemp, lastdir;
             AI3D::CORE::File::GetLastSecondDir(_outpath, outpathtemp, lastdir);
            
             Bitmap bitmap;
             
             int pre_width = 80;
             int pre_height = 60;

             std::string outpath = File::JoinPaths(outpathtemp, "previews");
             File::CreateDirIfNotExists(outpath);

            

             if (!File::IsFileExistent(imagepath))
             {
                 return false;
             }

             std::string hashfile = String::ToSHA256(imagepath);

             std::string hash_path = hashfile.substr(0, 2);

             std::string hash_filename = hashfile.substr(2) ;

             std::string hash_filepath = File::JoinPaths(outpath, hash_path, hash_filename);

             if (File::IsFileExistent(hash_filepath))
             {
                 preview_name_ = hash_filepath;
                 return true;
             }

             if (File::IsFileExistent(hash_filepath + ".jpg"))
             {
                 preview_name_ = hash_filepath + ".jpg";
                 return true;
             }

             
             

             
             

             std::vector<unsigned char> exivHostFileBytesV2;
             if (!File::ReadBinaryFileUtf8(imagepath, exivHostFileBytesV2) || exivHostFileBytesV2.empty())
             {
                 return false;
             }

             Exiv2::Image::UniquePtr img;
             try
             {
                 img = Exiv2::ImageFactory::open(
                     reinterpret_cast<const Exiv2::byte*>(exivHostFileBytesV2.data()),
                     exivHostFileBytesV2.size());
             }
             catch (Exiv2::Error& e)
             {
                 LOGE(String::StringPrintf("Caught Exiv2 exception %s", e.what()));
                 return false;
             }
             if (!img)
             {
                 LOGE(String::StringPrintf("Failed to open image: %s", imagepath.c_str()));
                 return false;
             }
             img->readMetadata();

             bool isThumbnailSavedSuccessfully = false;
             Exiv2::PreviewManager previewmanager(*img);
             Exiv2::PreviewPropertiesList previewList = previewmanager.getPreviewProperties();
             if (!previewList.empty())
             { 
                 Exiv2::PreviewImage preview = previewmanager.getPreviewImage(
                     previewList[0]);
                 File::CreateDirIfNotExists(File::GetParentDir(hash_filepath));
                 preview.writeFile(hash_filepath);

                 hash_filepath += preview.extension().c_str();
                 preview_name_ = hash_filepath;
                 isThumbnailSavedSuccessfully = true;
             }

             if (!isThumbnailSavedSuccessfully)
             {
                 
                 
                 bool ret = bitmap.Read(imagepath);
                 if (!ret)
                 {
                     return false;
                 }
                 
               
                 bitmap.Rescale(pre_width, pre_height);
                 File::CreateDirIfNotExists(File::GetParentDir(hash_filepath));
                 ret = bitmap.Write(hash_filepath, FREE_IMAGE_FORMAT::FIF_JPEG);
                 if (!ret)
                 {
                     
                     bitmap.Deallocate();
                     return false;
                 }
                 
                 bitmap.Deallocate();
                 preview_name_ = hash_filepath;
             }
             return true;
         }

        std::string Image::GetPriviewFileFullName()const
        {
            return preview_name_;
        }															   

        void  Image::SetPriviewFileFullName(const std::string& preview_name)
        {
            preview_name_ = preview_name;
        }

        inline point2D_t Image::SetPoint2D(const Point2D& point2d) {
            points2D_.push_back(point2d);
            return points2D_.size() - 1;
        }
        std::string Image::GetImageFullPath()
        {
            std::string fullname = AI3D::CORE::File::JoinPaths(path_, name_);
            fullname =  File::EnsureUnifySlash(fullname);
            return fullname;
        }
       
        inline const std::string& Image::GetPath() const
        {
            return path_;
        }
        inline std::string& Image::GetPathMutual()
        {
            return path_;
        }
        inline void Image::SetPath(const std::string& path)
        {
            path_ = path;
        }

       
     

        
         Eigen::Vector3d& Image::GetPositionMutual() 
        {
            return center_;
        };
        const Eigen::Vector3d& Image::GetPosition() const
        {
            return center_;
        };
        Eigen::Vector3d& Image::GetPosition()
        {
            return center_;
        };
        double Image::GetPosition(const size_t idx) const
        {
            return center_(idx);
        };
        double& Image::GetPosition(const size_t idx)
        {
            return center_(idx);
        };
        void Image::SetPosition(const Eigen::Vector3d& position)
        {
            center_ = position;
        }
        inline bool Image::HasPosition() const
        {
            
            return center_.x()!= -DBL_MAX;
        }

        const Eigen::Vector3d& Image::GetPositionPrior() const
        {
            return center_prior_;
        }

        inline bool Image::HasPositionPrior() const
        {
             return center_prior_.x()!=-DBL_MAX;
        }

        Eigen::Vector3d& Image::GetPositionPriorMutual()
        {
            return center_prior_;
        };

        bool Image::HasDepth() const
        {
            
             return  depth_.x() != -DBL_MAX ;
        }
        
        bool Image::HasFrustum() const
        {
            return !frustum_.empty();
        }

        Eigen::Vector3d& Image::GetPositionPrior()
        {
            return center_prior_;
        }
        double Image::GetPositionPrior(const size_t idx) const
        {
            return center_prior_(idx);
        }
        double& Image::GetPositionPrior(const size_t idx)
        {
            return center_prior_(idx);
        }
        

        void Image::SetPositionPrior(const Eigen::Vector3d& position)
        {
            center_prior_ = position;
        }

        const Eigen::Matrix3d& Image::GetRotationMatrix() const
        {
           
            return rotation_matrix_;
        }
        Eigen::Matrix3d& Image::GetRotationMatrixMutual()
        {
         
            return rotation_matrix_;
        }
       
        void Image::SetRotationMatrix(const Eigen::Matrix3d& rotation)
        {
            rotation_matrix_ = rotation;
        }

        void Image::SetRotationMatrix(std::array<double, 9> rotation)
        {
            rotation_matrix_ = ArrayToRotationMatrix(rotation);
        }
        const Eigen::Matrix3d& Image::GetRotationMatrixPrior() const
        {
           return rotation_matrix_prior_;
        }
        Eigen::Matrix3d& Image::GetRotationMatrixPriorMutual()
        {
            return rotation_matrix_prior_;
        }
        bool Image::HasRotationMatrix() const
        {
            return rotation_matrix_ != Eigen::Matrix3d::Zero();
         
        }

        short Image::ParseOrientation(const std::string& imagepath)
        {
			short orientation = -1;
            if (exifinfo_.Orientation != 0)
            {
                orientation = exifinfo_.Orientation;
            }
            else
            {
                
                auto exif_ = std::make_unique<ExifIO>();
				if (exif_->Open(imagepath) == REGULAR_IMAGE)
                {
                    orientation = exif_->GetOrientation();
                }
            }
            return orientation;
        }

        void Image::SetPriorSrs(srs_s srs)
        { 
            prior_srs_def_ = srs;
        };
        const srs_s Image::GetPriorSrs() const
        {
            return prior_srs_def_;
        };


        bool Image::HasRotationMatrixPrior() const
        {
													   
            return rotation_matrix_prior_ != Eigen::Matrix3d::Zero();
           
        }
        void Image::SetRotationMatrixPrior(const Eigen::Matrix3d& rotation)
        {
            rotation_matrix_prior_ = rotation;
        }

        void Image::SetRotationMatrixPrior(std::array<double, 9> rotation)
        {
            rotation_matrix_prior_ = ArrayToRotationMatrix(rotation);
        }

        const Eigen::Vector3d& Image::GetColorParam() const
        {
            return colorparam_;
        }

        Eigen::Vector3d& Image::GetColorParamMutual()
        {
            return colorparam_;
        }

        double Image::GetColorParam(const size_t idx) const
        {
            return colorparam_(idx);
        }

        double& Image::GetColorParamMutual(const size_t idx)
        {
            return colorparam_(idx);
        }

        void Image::SetColorParam(const Eigen::Vector3d& colorparam)
        {
            colorparam_ = colorparam;
        }

        void Image::SetDewrapFlag(bool dewrap_flag)
        {
            dewrap_flag_ = dewrap_flag;
        }
        bool Image::GetDewrapFlag()const
        {
            return dewrap_flag_;
        }


        int Image::GetWidth()const
        {
            return width_;
        }
        void Image::SetWidth(int width)
        {
            width_ = width;
        }
        int& Image::GetWidthMutual()
        {
            return width_;
        }

        int Image::GetHeight()const
        {
            return height_;
        }
        void Image::SetHeight(int height)
        {
            height_ = height;
        }
        int& Image::GetHeightMutual()
        {
            return height_;
        }
        int Image::ParseExif(const std::string& outPath)
        {
            
            std::string imagePath = path_ + name_;

            
            auto exif = std::make_unique<ExifIO>();
            Bitmap bitmap;
            ExifInfo exifinfo;
            XmpData xmpdata;

            int width = kInvalideNum;
            int height = kInvalideNum;

            int ret = exif->Open(imagePath);
            if (ret == NOEXIF_IMAGE)
            {
                LOGI(String::StringPrintf("Image: %s has no exif file", imagePath.c_str()));
            }
            else if (ret == ERROR_IMAGE)
            {
                return ERROR_IMAGE;
            }
           
            

            
            std::string image_extension;

            try
            {
                image_extension = File::BoostPathToUtf8String(File::BoostPathFromUtf8(imagePath).extension());
            }
            catch (const std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
                LOGI(oss.str());
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
            }

            String::StringToLower(&image_extension);
            if ( !image_extension.compare(".arw") || !image_extension.compare(".rw2") || !image_extension.compare(".raw"))
            {
                bitmap.Read(imagePath);
                width = bitmap.GetWidth();
                height = bitmap.GetHeight();
            }
            else
            {
                width = exif->GetWidth();
                height = exif->GetHeight();
                if (width <= 0 || height <= 0)
                {
                    
                    LOGI("Read the whole image!!!");
                    bitmap.Read(imagePath);
                    width = bitmap.GetWidth();
                    height = bitmap.GetHeight();
                    if (width <= 0 || height <= 0)
                    {
						LOGE(String::StringPrintf("Invalid image: %s, Image Error: Width=  Height= ", imagePath.c_str(), width, height));
                        return ERROR_IMAGE;
                    }
                }
            }
            
            double f_dist = -DBL_MAX;
            f_dist = exif->GetFocal();
            double f_35mmeq = -DBL_MAX;
            f_35mmeq = exif->GetFocalLengthIn35mm();
            std::string make = exif->GetBrand();
            std::string model = exif->GetModel();
            std::string datetime = exif->GetDateTime();


            double lon, lat, alt;
            lon = lat = alt = -DBL_MAX;
            lon = exif->GPSLongitude();
            lat = exif->GPSLatitude();
            alt = exif->GPSAltitude();
            double f_pix = kInvalideNum;
			double sensorsize = kInvalideNum;

            String::StringRemoveALL(model, make, false);
            if (!CameraDatabase::QuerySensorWidthFromCameraBin(make, model, &sensorsize))
            {

            }


           
            
            exifinfo.make = make;
            exifinfo.model = model;
            exifinfo.longitude = lon;
            exifinfo.latitude = lat;
            exifinfo.altitude = alt;
            exifinfo.dateTime = datetime;
            exifinfo.focalLengthIn35mm = f_35mmeq;
            exifinfo.focalLength = f_dist;
            exifinfo.width = width;
            exifinfo.height = height;
            exifinfo.Orientation = exif->GetOrientation();
            exifinfo.sensor_width = sensorsize;
            exifinfo.rotation = exif->GetRotation();
            width_ = width;
            height_ = height;
            
            if (exif->DoesHaveXMPData())
            {
                xmpdata_ = exif->AllXMPData();

                dewrap_flag_ = xmpdata_.DewarpFlag;
            }
            exifinfo_ = exifinfo;
            
            return REGULAR_IMAGE;
        }
       
    }
} 
