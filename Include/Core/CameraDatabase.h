
#ifndef _AI3D_CORE_CAMERA_DATABASE_H_
#define _AI3D_CORE_CAMERA_DATABASE_H_

#include <string>
#include <Constants.h>

#include "glog/logging.h"
#include "Core/CameraSpecs.h"
#include "Core/String.h"
namespace AI3D
{
	namespace CORE
	{

		class AI3D_API CameraDatabase
		{
		public:
			CameraDatabase();


			size_t GetNumEntries() const;
			
			static bool QuerySensorWidth(const std::string& make, const std::string& model, double* sensor_width);

			static bool QuerySensorWidthFromCameraBin(const std::string& make, const std::string& model, double* sensor_width);

			static std::vector<std::pair<std::string, double>>  SimplifyCameraDB();
			
			
		private:
			
			static const std::vector<camera_datasheet_s> camera_datasheet_;
			static const std::vector<std::pair<std::string, double>> camera_maker_sensor_;
		};
	}
} 

#endif  
