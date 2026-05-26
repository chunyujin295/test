
#ifndef _AI3D_CORE_CAMERA_SPECS_H_
#define _AI3D_CORE_CAMERA_SPECS_H_

#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <glog/logging.h>
#include "Core/String.h"
#include <filesystem>
#include "Core/Types.h"

namespace AI3D
{
	namespace CORE
	{
		
		struct camera_datasheet_s
		{
			camera_datasheet_s();

			camera_datasheet_s(const std::string maker, const std::string model, const double sensor_size);

			bool operator==(const camera_datasheet_s& rhs) const;


			std::string make_ = STR(undefined);
			std::string model_ = STR(undefined);

			double sensor_size_ = 0;
		};

		
		bool IsFileExistent(const std::filesystem::path& path);
		bool ParseDatabase(std::vector<camera_datasheet_s>& vec_database);
		
		typedef std::vector<std::pair<std::string, float>> camera_make_specs_t;
		typedef std::unordered_map<std::string, camera_make_specs_t> camera_specs_t;

		 camera_specs_t InitializeCameraSpecs();
		  std::vector<camera_datasheet_s> InitializeCameraDB();
	}
} 

#endif  
