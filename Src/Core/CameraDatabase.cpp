















#include "Core/CameraDatabase.h"

namespace AI3D
{
    namespace CORE
    {
        
        const std::vector<camera_datasheet_s> CameraDatabase::camera_datasheet_ = InitializeCameraDB();

        std::vector<std::pair<std::string, double>> CameraDatabase::SimplifyCameraDB()
        {
            std::vector<std::pair<std::string, double>> camera_maker_sensor;
			for (const auto& cameradb : camera_datasheet_)
            {
                std::string cleaned_make = cameradb.make_;
                std::string cleaned_model = cameradb.model_;

                cleaned_make = String::StringReplace(cleaned_make, " ", "");
                cleaned_model = String::StringReplace(cleaned_model, " ", "");
                cleaned_make = String::StringReplace(cleaned_make, "-", "");
                cleaned_model = String::StringReplace(cleaned_model, "-", "");
                String::StringToLower(&cleaned_make);
                String::StringToLower(&cleaned_model);

                std::string make_model = cleaned_make + cleaned_model;
                camera_maker_sensor.emplace_back(std::make_pair(make_model, cameradb.sensor_size_));
            }
            return camera_maker_sensor;
        }

        const std::vector<std::pair<std::string, double>> CameraDatabase::camera_maker_sensor_ = SimplifyCameraDB();

       
        CameraDatabase::CameraDatabase() 
        {

        }


        
        
        
        
        


        bool CameraDatabase::QuerySensorWidthFromCameraBin(const std::string& make, const std::string& model,
            double* sensor_width)
        {
            
            std::string cleaned_make = make;
            std::string cleaned_model = model;
            cleaned_make = String::StringReplace(cleaned_make, " ", "");
            cleaned_model = String::StringReplace(cleaned_model, " ", "");
            cleaned_make = String::StringReplace(cleaned_make, "-", "");
            cleaned_model = String::StringReplace(cleaned_model, "-", "");
            String::StringToLower(&cleaned_make);
            String::StringToLower(&cleaned_model);


            std::string make_model = cleaned_make + cleaned_model;
            auto pos = std::find_if(camera_maker_sensor_.begin(), camera_maker_sensor_.end(), [make_model](const std::pair<std::string, double>& maker_sensor) {return maker_sensor.first == make_model; });
			if (pos != camera_maker_sensor_.end())
			{
                *sensor_width = pos->second;
                return true;
            }
           
            return false;
		}

    }
} 
