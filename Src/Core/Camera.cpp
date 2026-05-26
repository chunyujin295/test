
#include "Core/Camera.h"

#include <iomanip>
#include <math.h>

#include "Core/CameraModels.h"
#include "Core/Logging.h"
#include "Core/File.h"
#include "Core/Math.h"


namespace AI3D
{
    namespace CORE
    {
        Camera::Camera()
            : camera_id_(kInvalidCameraId),
            model_id_(kInvalidCameraModelId),
            width_(0),
            height_(0),
            prior_focal_length_(false)
        {
            fixed_params_.clear();
        }

        void Camera::SetCameraName(std::string name)
        {
            CameraName_ = name;
        }

        std::string Camera::GetCameraNameMutual()
        {
            return CameraName_;
        }
        std::string Camera::GetCameraName()const
        {
            return CameraName_;
        }

        std::string Camera::GetModelName() const 
        { 
            return CameraModelIdToName(model_id_); 
        }

        void Camera::SetModelId(const int model_id) 
        {
            if (!CHECK_OPTION(ExistsCameraModelWithId(model_id)))
                return;
            model_id_ = model_id;
            params_.resize(GetCameraModelNumParams(model_id_), 0);
        }

        void Camera::SetModelIdFromName(const std::string& model_name) 
        {
            if (!CHECK_OPTION(ExistsCameraModelWithName(model_name)))
                return;
            model_id_ = CameraModelNameToId(model_name);
            params_.resize(GetCameraModelNumParams(model_id_), 0);
        }

        CameraModelType_e Camera::GetCameraModelType() const
        {
            return cameramodeltype_;
        }

        void Camera::SetCameraModelType(const CameraModelType_e& cmt)
        {
            cameramodeltype_ = cmt;
        }

        
        std::string Camera::GetMake()const
        {
            return cam_maker_;
        }
        std::string Camera::GetMakeMutual()
        {
            return cam_maker_;
        }
        void Camera::SetMake(const std::string& make)
        {
            cam_maker_ = make;
        }

        
        std::string Camera::GetMakeModel()const
        {
            return cam_makermodel_;
        }
        std::string Camera::GetMakeModelMutual()
        {
            return  cam_makermodel_;
        }
        void Camera::SetMakeModel(const std::string& model)
        {
			cam_makermodel_ = model;
        }

        const double Camera::GetSensorSize()const
        {
            return sensor_size_;
        }

        double& Camera::GetSensorSizeMutual()
        {
            return sensor_size_;
        }

        void Camera::SetSensorSize(const double& sensorsize)
        {
            sensor_size_ = sensorsize;
        }

        const std::vector<size_t>& Camera::GetFocalLengthIdxs() const 
        {
            return GetCameraModelFocalLengthIdxs(model_id_);
        }

        const std::vector<size_t>& Camera::GetPrincipalPointIdxs() const 
        {
            return GetCameraModelPrincipalPointIdxs(model_id_);
        }

        const std::vector<size_t>& Camera::GetExtraParamsIdxs() const 
        {
            return GetCameraModelExtraParamsIdxs(model_id_);
        }

        Eigen::Matrix3d Camera::GetCalibrationMatrix() const 
        {
            Eigen::Matrix3d K = Eigen::Matrix3d::Identity();

            const std::vector<size_t>& idxs = GetFocalLengthIdxs();
            if (idxs.size() == 1) {
                K(0, 0) = params_[idxs[0]];
                K(1, 1) = params_[idxs[0]];
            }
            else if (idxs.size() == 2) {
                K(0, 0) = params_[idxs[0]];
                K(1, 1) = params_[idxs[1]];
            }
            else {
                LOG(FATAL)
                    << "Camera model must either have 1 or 2 focal length parameters.";
            }

            K(0, 2) = GetPrincipalPointX();
            K(1, 2) = GetPrincipalPointY();

            return K;
        }
        bool Camera::HasMaker()
        {
            return ((cam_maker_ != "") && cam_makermodel_ != "");
        }

        bool Camera::IsMakerSame(const Camera& camera)
        {
            if (HasMaker())
            {
                return  (cam_maker_ == camera.GetMake()) && (cam_makermodel_ == camera.GetMakeModel());
            }
            return false;

        }

        bool  Camera::IsSame(const Camera& camera)
        {
            
#if 0
            return (model_id_ == camera.GetModelId()) && (width_ == camera.GetWidth()) && (height_ == camera.GetHeight()) \
                && (focal_length_ == camera.GetFocalLengthMM())&& (focal_lengthIn35mm_ == camera.GetFocalLengthIn35mm())\
                && (cam_maker_ == camera.GetMake()) && (cam_makermodel_ == camera.GetMakeModel())&& (sensor_size_ == camera.GetSensorSize());
#else
            return (model_id_ == camera.GetModelId()) && (width_ == camera.GetWidth()) && (height_ == camera.GetHeight()) \
                && fabs(focal_length_ - camera.GetFocalLengthMM()) <= 0.0001 && fabs(focal_lengthIn35mm_ - camera.GetFocalLengthIn35mm()) <= 0.0001 \
                && (cam_maker_ == camera.GetMake()) && (cam_makermodel_ == camera.GetMakeModel()) && fabs(sensor_size_ - camera.GetSensorSize()) <= 0.0001;
#endif

        }

        std::string Camera::GetParamsInfo() const 
        {
            return GetCameraModelParamsInfo(model_id_);
        }

        double Camera::GetMeanFocalLength() const 
        {
            const auto& focal_length_idxs = GetFocalLengthIdxs();
            double focal_length = 0;
            for (const auto idx : focal_length_idxs) {
                focal_length += params_[idx];
            }
            return focal_length / focal_length_idxs.size();
        }

        double Camera::GetFocalLength() const 
        {
            const std::vector<size_t>& idxs = GetFocalLengthIdxs();
            CHECK_OPTION_GE(idxs.size(), 1);
            return params_[idxs[0]];
        }

        double Camera::GetFocalLengthX() const 
        {
            const std::vector<size_t>& idxs = GetFocalLengthIdxs();
            CHECK_OPTION_EQ(idxs.size(), 2);
            return params_[idxs[0]];
        }

        double Camera::GetFocalLengthY() const 
        {
            const std::vector<size_t>& idxs = GetFocalLengthIdxs();
            CHECK_OPTION_EQ(idxs.size(), 2);
            return params_[idxs[1]];
        }

        void Camera::SetFocalLength(const double focal_length) 
        {
            const std::vector<size_t>& idxs = GetFocalLengthIdxs();
            for (const auto idx : idxs) 
            {
                params_[idx] = focal_length;
            }
        }

        double Camera::GetFocalLengthMM() const
        {
            return focal_length_;
        }
        double& Camera::GetFocalLengthMMMultal()
        {
            return focal_length_;
        }
     
        void Camera::SetFocalLengthMM(const double focal_length_mm)
        {
            focal_length_ = focal_length_mm;
        }

        void Camera::SetFocalLengthX(const double focal_length_x) 
        {
            const std::vector<size_t>& idxs = GetFocalLengthIdxs();
            CHECK_OPTION_EQ(idxs.size(), 2);
            params_[idxs[0]] = focal_length_x;
        }

        void Camera::SetFocalLengthY(const double focal_length_y) 
        {
            const std::vector<size_t>& idxs = GetFocalLengthIdxs();
            CHECK_OPTION_EQ(idxs.size(), 2);
            params_[idxs[1]] = focal_length_y;
        }

        double Camera::GetPrincipalPointX() const 
        {
            const std::vector<size_t>& idxs = GetPrincipalPointIdxs();
            CHECK_OPTION_EQ(idxs.size(), 2);
            return params_[idxs[0]];
        }

        double Camera::GetPrincipalPointY() const 
        {
            const std::vector<size_t>& idxs = GetPrincipalPointIdxs();
            CHECK_OPTION_EQ(idxs.size(), 2);
            return params_[idxs[1]];
        }

        void Camera::SetPrincipalPointX(const double ppx) 
        {
            const std::vector<size_t>& idxs = GetPrincipalPointIdxs();
            CHECK_OPTION_EQ(idxs.size(), 2);
            params_[idxs[0]] = ppx;
        }

        void Camera::SetPrincipalPointY(const double ppy) 
        {
            const std::vector<size_t>& idxs = GetPrincipalPointIdxs();
            CHECK_OPTION_EQ(idxs.size(), 2);
            params_[idxs[1]] = ppy;
        }

        std::string Camera::ParamsToString() const 
        {
            return File::VectorToCSV(params_);
        }

        bool Camera::SetParamsFromString(const std::string& string) 
        {
            params_ = File::CSVToVector<double>(string);
            return VerifyParams();
        }
        bool Camera::HasValidParams() const
        {
            if (VerifyParams())
            {
               
                const std::vector<size_t>& idxs = GetFocalLengthIdxs();
                if (idxs.size() == 2)
                {
                    return (params_[2] > 0.0 && params_[3]>0.0);
                }
                else if (idxs.size() == 1)
                {
                    return (params_[1] > 0.0 && params_[2] > 0.0);
                }
            }
            return false;
        }

        void  Camera::SetUndistortBorder(double* undistortedborder)
        {
            for (int i = 0; i < 8; i++)
            {
				undistortedborder_[i] = *undistortedborder;
                undistortedborder++;
            }
        }
        double* Camera::GetUndistortBorder()
        {
            return undistortedborder_;
        }


        void Camera::UndistortCamera(const UndistortCameraOptions_s& options,
             Camera& undistorted_camera)
        {
           
            undistorted_camera.SetModelId(PinholeCameraModel::model_id);
           

            
            const std::vector<size_t>& focal_length_idxs = GetFocalLengthIdxs();
            
            if (focal_length_idxs.size() == 1) {
                undistorted_camera.SetFocalLengthX(GetFocalLength());
                undistorted_camera.SetFocalLengthY(GetFocalLength());
            }
            else if (focal_length_idxs.size() == 2) {
                undistorted_camera.SetFocalLengthX(GetFocalLengthX());
                undistorted_camera.SetFocalLengthY(GetFocalLengthY());
            }

            
            undistorted_camera.SetPrincipalPointX(GetPrincipalPointX());
            undistorted_camera.SetPrincipalPointY(GetPrincipalPointY());
            undistorted_camera.SetWidth(int(2* GetPrincipalPointX()+0.5));
            undistorted_camera.SetHeight(int(2* GetPrincipalPointY()+0.5));
            if (0)
            {
                
                size_t roi_min_x = 0;
                size_t roi_min_y = 0;
                size_t roi_max_x = GetWidth();
                size_t roi_max_y = GetHeight();

                const bool roi_enabled = options.roi_min_x > 0.0 || options.roi_min_y > 0.0 ||
                    options.roi_max_x < 1.0 || options.roi_max_y < 1.0;

                if (roi_enabled) {
                    roi_min_x = static_cast<size_t>(
                        std::round(options.roi_min_x * static_cast<double>(GetWidth())));
                    roi_min_y = static_cast<size_t>(
                        std::round(options.roi_min_y * static_cast<double>(GetHeight())));
                    roi_max_x = static_cast<size_t>(
                        std::round(options.roi_max_x * static_cast<double>(GetWidth())));
                    roi_max_y = static_cast<size_t>(
                        std::round(options.roi_max_y * static_cast<double>(GetHeight())));

                    
                    roi_min_x = std::min(roi_min_x, GetWidth() - 1);
                    roi_min_y = std::min(roi_min_y, GetHeight() - 1);
                    roi_max_x = std::max(roi_max_x, roi_min_x + 1);
                    roi_max_y = std::max(roi_max_y, roi_min_y + 1);

                    undistorted_camera.SetWidth(roi_max_x - roi_min_x);
                    undistorted_camera.SetHeight(roi_max_y - roi_min_y);

                    undistorted_camera.SetPrincipalPointX(GetPrincipalPointX() -
                        static_cast<double>(roi_min_x));
                    undistorted_camera.SetPrincipalPointY(GetPrincipalPointY() -
                        static_cast<double>(roi_min_y));
                }

                
                if (roi_enabled || (GetModelId() != SimplePinholeCameraModel::model_id &&
                    GetModelId() != PinholeCameraModel::model_id)) {
                    
                    const double cx = undistorted_camera.GetPrincipalPointX();
                    const double cy = undistorted_camera.GetPrincipalPointY();
                    double left_min_x = 0.;
                    double left_max_x = 0.;
                    double right_min_x = 2. * cx;
                    double right_max_x = 2.0 * cx; std::numeric_limits<double>::lowest();

                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    

                    

                    double top_min_y = 0.;
                    double top_max_y = 0.;
                        double bottom_min_y = 2.*cy; 
                        double bottom_max_y = 2.0*cy;

                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    

                   

                    
                     double scale_x =
                        std::min(cx / (cx - left_min_x),
                            (undistorted_camera.GetWidth() - 0.5 - cx) / (right_max_x - cx));
                     double scale_y = std::min(
                        cy / (cy - top_min_y),
                        (undistorted_camera.GetHeight() - 0.5 - cy) / (bottom_max_y - cy));
                    scale_x = 1.0 / scale_x;
                    scale_y = 1.0 / scale_y;
                    
                   

                    
                   

                    
                   

                    
                  

                    
                   
                }

                if (options.max_image_size > 0) {
                    const double max_image_scale_x =
                        options.max_image_size /
                        static_cast<double>(undistorted_camera.GetWidth());
                    const double max_image_scale_y =
                        options.max_image_size /
                        static_cast<double>(undistorted_camera.GetHeight());
                    const double max_image_scale =
                        std::min(max_image_scale_x, max_image_scale_y);
                    if (max_image_scale < 1.0) {
                        undistorted_camera.Rescale(max_image_scale);
                    }
                }
            }
            
        }


        void Camera::GenUndistortCamera(Camera& undistorted_camera)
        {
           
            undistorted_camera.SetCameraId(camera_id_);
            undistorted_camera.SetModelId(PinholeCameraModel::model_id);
            undistorted_camera.SetCameraModelType(CameraModelType_e::Perspective);
            
            undistorted_camera.SetWidth(GetWidth());
            undistorted_camera.SetHeight(GetHeight());

            
            const std::vector<size_t>& focal_length_idxs = GetFocalLengthIdxs();
            if (!CHECK_OPTION_LE(focal_length_idxs.size(), 2))
            {
                LOGE("Not more than two focal length parameters supported.");
                return;
            }
            if (focal_length_idxs.size() == 1) {
                undistorted_camera.SetFocalLengthX(GetFocalLength());
                undistorted_camera.SetFocalLengthY(GetFocalLength());
            }
            else if (focal_length_idxs.size() == 2)
            {
                undistorted_camera.SetFocalLengthX(GetFocalLengthX());
                undistorted_camera.SetFocalLengthY(GetFocalLengthY());
            }

            
            undistorted_camera.SetPrincipalPointX(GetPrincipalPointX());
            undistorted_camera.SetPrincipalPointY(GetPrincipalPointY());
            for (int i = 4; i < undistorted_camera.GetParamsMutual().size(); i++)
            {
                undistorted_camera.GetParamsMutual()[i] = 0.0;
            }
          
        }

        bool Camera::GetValidUndistortBorder(double& left_min_x, double& left_max_x, double& right_min_x, double& right_max_x,
            double& top_min_y, double& top_max_y, double& bottom_min_y, double& bottom_max_y)
        {
            size_t roi_min_x = 0;
            size_t roi_min_y = 0;
            size_t roi_max_x = GetWidth();
            size_t roi_max_y = GetHeight();
            Camera undistorted_camera;
             GenUndistortCamera(undistorted_camera);
            if ((GetModelId() != SimplePinholeCameraModel::model_id &&
                GetModelId() != PinholeCameraModel::model_id))
            {
                

                double _left_min_x = std::numeric_limits<double>::max();
                double _left_max_x = std::numeric_limits<double>::lowest();
                double _right_min_x = std::numeric_limits<double>::max();
                double _right_max_x = std::numeric_limits<double>::lowest();

                for (size_t y = roi_min_y; y < roi_max_y; ++y)
                {
                    
                    const Eigen::Vector2d undistorted_point1 = UndistortPixel(Eigen::Vector2d(0.5, y + 0.5));
                    _left_min_x = std::min(_left_min_x, undistorted_point1(0));
                    _left_max_x = std::max(_left_max_x, undistorted_point1(0));
                    

                    const Eigen::Vector2d undistorted_point2 =
                        UndistortPixel(Eigen::Vector2d(GetWidth() - 0.5, y + 0.5));
                    _right_min_x = std::min(_right_min_x, undistorted_point2(0));
                    _right_max_x = std::max(_right_max_x, undistorted_point2(0));
                }

                

                double _top_min_y = std::numeric_limits<double>::max();
                double _top_max_y = std::numeric_limits<double>::lowest();
                double _bottom_min_y = std::numeric_limits<double>::max();
                double _bottom_max_y = std::numeric_limits<double>::lowest();

                for (size_t x = roi_min_x; x < roi_max_x; ++x)
                {
                    

                    const Eigen::Vector2d undistorted_point1 =
                        UndistortPixel(Eigen::Vector2d(x + 0.5, 0.5));
                    _top_min_y = std::min(_top_min_y, undistorted_point1(1));
                    _top_max_y = std::max(_top_max_y, undistorted_point1(1));
                    
                    const Eigen::Vector2d undistorted_point2 =
                        UndistortPixel(Eigen::Vector2d(x + 0.5, GetHeight() - 0.5));
                    _bottom_min_y = std::min(_bottom_min_y, undistorted_point2(1));
                    _bottom_max_y = std::max(_bottom_max_y, undistorted_point2(1));
                }

                if (   _left_min_x  != std::numeric_limits<double>::max()
                    && _left_max_x  != std::numeric_limits<double>::lowest()
                    && _right_min_x != std::numeric_limits<double>::max()
                    && _right_max_x != std::numeric_limits<double>::lowest()
                    && _top_min_y   != std::numeric_limits<double>::max()
                    && _top_max_y   != std::numeric_limits<double>::lowest()
                    && _bottom_min_y != std::numeric_limits<double>::max()
                    && _bottom_max_y != std::numeric_limits<double>::lowest())
                {
                        left_min_x  =  _left_min_x     ;
                        left_max_x  =  _left_max_x     ;
                        right_min_x =   _right_min_x   ;
                        right_max_x =   _right_max_x   ;
                        top_min_y   = _top_min_y       ;
                        top_max_y   = _top_max_y       ;
                        bottom_min_y=    _bottom_min_y ;
                        bottom_max_y=    _bottom_max_y ;
                        return true;
                }
            }
            return false;
        }

        Eigen::Vector2d Camera::UndistortPixel(Eigen::Vector2d xy)
        {
            Eigen::Vector2d world_point = ImageToWorld(xy);
            Camera undistorted_camera;
            GenUndistortCamera(undistorted_camera);
            
            
            
            return  (undistorted_camera.GetCalibrationMatrix() * world_point.homogeneous()).hnormalized();        
        }



        bool Camera::HasDistortion()
        {
            if (VerifyParams())
            {

                const std::vector<size_t>& idxs = GetFocalLengthIdxs();
                if (idxs.size() == 2&& params_.size()>4)
                {
                    return (params_[4] != 0.0);
                }
                else if (idxs.size() == 1 && params_.size() > 3)
                {
                    return (params_[3] != 0.0);
                }
            }
            return false;
        }

        bool Camera::VerifyParams() const 
        {
            return CameraModelVerifyParams(model_id_, params_);
        }

        bool Camera::HasBogusParams(const double min_focal_length_ratio,
            const double max_focal_length_ratio,
            const double max_extra_param) const 
        {
            return CameraModelHasBogusParams(model_id_, params_, width_, height_,
                min_focal_length_ratio,
                max_focal_length_ratio, max_extra_param);
        }

        void Camera::InitializeWithId(const int model_id, const double focal_length,
            const size_t width, const size_t height) 
        {
            if (!CHECK_OPTION(ExistsCameraModelWithId(model_id)))
                return;
            model_id_ = model_id;
            width_ = width;
            height_ = height;
            params_ = CameraModelInitializeParams(model_id, focal_length, width, height);
        }

        void Camera::InitializeWithName(const std::string& model_name,
            const double focal_length, const size_t width,
            const size_t height) 
        {
            InitializeWithId(CameraModelNameToId(model_name), focal_length, width,
                height);
        }

        Eigen::Vector2d Camera::ImageToWorld(const Eigen::Vector2d& image_point) const
        {
            Eigen::Vector2d world_point;
            CameraModelImageToWorld(model_id_, params_, image_point(0), image_point(1),
                &world_point(0), &world_point(1));
            return world_point;
        }

        

        double Camera::ImageToWorldThreshold(const double threshold) const 
        {
            return CameraModelImageToWorldThreshold(model_id_, params_, threshold);
        }

        Eigen::Vector2d Camera::WorldToImage(const Eigen::Vector2d& world_point) const 
        {
            Eigen::Vector2d image_point;
            CameraModelWorldToImage(model_id_, params_, world_point(0), world_point(1),
                &image_point(0), &image_point(1));
            return image_point;
        }

        void Camera::SetFixed(std::vector<int> fixed_params)
        {
            fixed_params_.clear();
            fixed_params_.assign(fixed_params.begin(), fixed_params.end());
        }
        std::vector<int> Camera::GetFixed()
        {
            return fixed_params_;
        }


        void Camera::Rescale(const double scale)
        {
            if (!CHECK_OPTION_GT(scale, 0.0))
                return;
            const double scale_x =
                std::round(scale * width_) / static_cast<double>(width_);
            const double scale_y =
                std::round(scale * height_) / static_cast<double>(height_);
            width_ = static_cast<size_t>(std::round(scale * width_));
            height_ = static_cast<size_t>(std::round(scale * height_));
            SetPrincipalPointX(scale_x * GetPrincipalPointX());
            SetPrincipalPointY(scale_y * GetPrincipalPointY());
            if (GetFocalLengthIdxs().size() == 1) 
            {
                SetFocalLength((scale_x + scale_y) / 2.0 * GetFocalLength());
            }
            else if (GetFocalLengthIdxs().size() == 2) 
            {
                SetFocalLengthX(scale_x * GetFocalLengthX());
                SetFocalLengthY(scale_y * GetFocalLengthY());
            }
            else 
            {
                LOG(FATAL)
                    << "Camera model must either have 1 or 2 focal length parameters.";
            }
        }

        void Camera::Rescale(const size_t width, const size_t height) 
        {
            const double scale_x =
                static_cast<double>(width) / static_cast<double>(width_);
            const double scale_y =
                static_cast<double>(height) / static_cast<double>(height_);
            width_ = width;
            height_ = height;
            SetPrincipalPointX(scale_x * GetPrincipalPointX());
            SetPrincipalPointY(scale_y * GetPrincipalPointY());
            if (GetFocalLengthIdxs().size() == 1) {
                SetFocalLength((scale_x + scale_y) / 2.0 * GetFocalLength());
            }
            else if (GetFocalLengthIdxs().size() == 2) {
                SetFocalLengthX(scale_x * GetFocalLengthX());
                SetFocalLengthY(scale_y * GetFocalLengthY());
            }
            else {
                LOG(FATAL)
                    << "Camera model must either have 1 or 2 focal length parameters.";
            }
        }

        
       

       
       
       
       
       
       
       
       
       
        camera_t Camera::GetCameraId() const { return camera_id_; }


        camera_t Camera::GetCameraIdMutual() { return camera_id_; }
        void Camera::SetCameraId(const camera_t camera_id) { camera_id_ = camera_id; }

        int Camera::GetModelId() const { return model_id_; }

        size_t Camera::GetWidth() const { return width_; }

        size_t Camera::GetHeight() const { return height_; }

        void Camera::SetWidth(const size_t width) { width_ = width; }

        void Camera::SetHeight(const size_t height) { height_ = height; }

        double Camera::GetFov()
        {
            return fov_;
        }
        void Camera::SetFov(const double& fov)
        {
            fov_ = fov;
        }

        double Camera::GetFocalLengthIn35mm() const
        {
            return focal_lengthIn35mm_;
        }

        void Camera::SetFocalLengthIn35mm(const double focal_length_mm)
        {
            focal_lengthIn35mm_ = focal_length_mm;
        }

        bool Camera::HasPriorFocalLength() const { return prior_focal_length_; }

        void Camera::SetPriorFocalLength(const bool prior) {
            prior_focal_length_ = prior;
        }


        size_t Camera::GetNumParams() const { return params_.size(); }

        const std::vector<double>& Camera::GetParams() const { return params_; }

        std::vector<double>& Camera::GetParamsMutual() { return params_; }

        double Camera::GetParams(const size_t idx) const { return params_[idx]; }

        double& Camera::GetParamsMutual(const size_t idx) { return params_[idx]; }

        const double* Camera::ParamsData() const { return params_.data(); }

        double* Camera::ParamsDataMutual() { return params_.data(); }

        void Camera::SetParams(const std::vector<double>& params) 
        { 
            params_ = params; 
        }

       
        void Camera::SetCameraOrientation(const std::string& CameraOrientation)
        {
            CameraOrientation_ = CameraOrientation;
        }
        std::string Camera::GetCameraOrientation()const
        {
            return CameraOrientation_;
        }

          
          
          
          
          
          
          
          
          
          
        
          
          
          
          
          
          
          
          
          
          
          
          
          
    }
} 
