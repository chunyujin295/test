
#ifndef _AI3D_CORE_CAMERA_H_
#define _AI3D_CORE_CAMERA_H_

#include <vector>
#include<Constants.h>
#include "Core/Types.h"

namespace AI3D
{

    namespace CORE
    {
        struct UndistortCameraOptions_s {
            
            double blank_pixels = 0.0;

            
            
            double min_scale = 0.2;
            double max_scale = 2.0;

            
            int max_image_size = -1;

            
            
            
            
            double roi_min_x = 0.0;
            double roi_min_y = 0.0;
            double roi_max_x = 1.0;
            double roi_max_y = 1.0;
        };
        
        
        
        
        class AI3D_API Camera 
        {
        public:
            Camera();

            
           
            
             
            inline camera_t GetCameraId() const;
            camera_t GetCameraIdMutual() ;
            inline void SetCameraId(const camera_t camera_id);
            
            void SetCameraName(std::string name);
            std::string GetCameraNameMutual();
            std::string GetCameraName()const;

            
            inline int GetModelId() const;
            std::string GetModelName() const;
            void SetModelId(const int model_id);
            void SetModelIdFromName(const std::string& model_name);

            
            CameraModelType_e GetCameraModelType() const;
            void SetCameraModelType(const CameraModelType_e& cmt);

            
            std::string GetMake()const;
            std::string GetMakeMutual();
            void SetMake(const std::string& make);


            
            std::string GetMakeModel()const;
            std::string GetMakeModelMutual();
            void SetMakeModel(const std::string& model);
            bool IsMakerSame(const Camera& camera);
            bool HasMaker();
            bool IsSame(const Camera& camera);
            
            
            const double GetSensorSize()const;
            void SetSensorSize(const double& sensorsize);
            double& GetSensorSizeMutual();
            
            inline size_t GetWidth() const;
            inline size_t GetHeight() const;
            inline void SetWidth(const size_t width);
            inline void SetHeight(const size_t height);

            
            double GetFov();
            void SetFov(const double& fov);
            
            double GetMeanFocalLength() const;
            double GetFocalLength() const;
            double GetFocalLengthX() const;
            double GetFocalLengthY() const;
            void SetFocalLength(const double focal_length);
            void SetFocalLengthX(const double focal_length_x);
            void SetFocalLengthY(const double focal_length_y);

            
            double GetFocalLengthMM() const;
            double& GetFocalLengthMMMultal();
            void SetFocalLengthMM(const double focal_length_mm);

            
            double GetFocalLengthIn35mm()const;
            void SetFocalLengthIn35mm(const double focal_length_mm);

            
            inline bool HasPriorFocalLength() const;
            inline void SetPriorFocalLength(const bool prior);

            
            
            double GetPrincipalPointX() const;
            double GetPrincipalPointY() const;
            void SetPrincipalPointX(const double ppx);
            void SetPrincipalPointY(const double ppy);

            
            const std::vector<size_t>& GetFocalLengthIdxs() const;
            const std::vector<size_t>& GetPrincipalPointIdxs() const;
            const std::vector<size_t>& GetExtraParamsIdxs() const;

            
            
            Eigen::Matrix3d GetCalibrationMatrix() const;

            
            std::string GetParamsInfo() const;

            
            inline size_t GetNumParams() const;
            inline const std::vector<double>& GetParams() const;
            inline std::vector<double>& GetParamsMutual();
            inline double GetParams(const size_t idx) const;
            inline double& GetParamsMutual(const size_t idx);
            inline const double* ParamsData() const;
            inline double* ParamsDataMutual();
            inline void SetParams(const std::vector<double>& params);

            
            std::string ParamsToString() const;

            
            bool SetParamsFromString(const std::string& string);
            bool HasValidParams() const;
            bool GetValidUndistortBorder(double& left_min_x, double& left_max_x, double& right_min_x, double& right_max_x,
                double& top_min_y, double& top_max_y, double& bottom_min_y, double& bottom_max_y);

           
           void UndistortCamera(const UndistortCameraOptions_s& options,
                Camera& newcamera);
             void GenUndistortCamera(Camera& cam);
             void SetUndistortBorder(double* undistortedborder);
             double* GetUndistortBorder();
            
            Eigen::Vector2d UndistortPixel(Eigen::Vector2d xy);
            bool HasDistortion();
            
            
            bool VerifyParams() const;

            
            bool HasBogusParams(const double min_focal_length_ratio,
                const double max_focal_length_ratio,
                const double max_extra_param) const;

            
            
            void InitializeWithId(const int model_id, const double focal_length,
                const size_t width, const size_t height);
            void InitializeWithName(const std::string& model_name,
                const double focal_length, const size_t width,
                const size_t height);

            
            Eigen::Vector2d ImageToWorld(const Eigen::Vector2d& image_point) const;

            
            double ImageToWorldThreshold(const double threshold) const;

            
            Eigen::Vector2d WorldToImage(const Eigen::Vector2d& world_point) const;

            
            
            void Rescale(const double scale);
            void Rescale(const size_t width, const size_t height);
            
            void SetCameraOrientation(const std::string& CameraOrientation);
            std::string GetCameraOrientation()const;
            void SetFixed(std::vector<int> fixed_params);
            std::vector<int> GetFixed();
            const double GetPixelSize() const {             return pixel_size_; };
            void SetPixelSize(double size) { pixel_size_ = size; };

            
        private:
            
            
            camera_t camera_id_;

            
            
            int model_id_;

            
            size_t width_;
            size_t height_;
            std::string CameraName_;
            
            
            std::vector<double> params_;

            
            
            bool prior_focal_length_;
            
            CameraModelType_e cameramodeltype_;
            
            double focal_length_ = UNDEFINEDVAL;
            
            double focal_lengthIn35mm_ = UNDEFINEDVAL;
            
            std::string cam_maker_ = "";
            
            std::string cam_makermodel_ = "";
            
            double sensor_size_ = UNDEFINEDVAL;
			double undistortedborder_[8] = { 0.0 };
            double fov_ = 0;
            std::string CameraOrientation_ = "XRightYDown";
            std::vector<int> fixed_params_;
            double pixel_size_ = UNDEFINEDVAL;
        };       
    }
} 

#endif  
