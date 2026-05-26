
#include "Core/CameraModels.h"

namespace AI3D
{
	namespace CORE
	{
		
		
#define CAMERA_MODEL_CASE(CameraModel)                          \
  const int CameraModel::model_id = InitializeModelId();        \
  const std::string CameraModel::model_name =                   \
      CameraModel::InitializeModelName();                       \
  const size_t CameraModel::num_params = InitializeNumParams(); \
  const std::string CameraModel::params_info =                  \
      CameraModel::InitializeParamsInfo();                      \
  const std::vector<size_t> CameraModel::focal_length_idxs =    \
      CameraModel::InitializeFocalLengthIdxs();                 \
  const std::vector<size_t> CameraModel::principal_point_idxs = \
      CameraModel::InitializePrincipalPointIdxs();              \
  const std::vector<size_t> CameraModel::extra_params_idxs =    \
      CameraModel::InitializeExtraParamsIdxs();

		CAMERA_MODEL_CASES

#undef CAMERA_MODEL_CASE

			std::unordered_map<std::string, int> InitialzeCameraModelNameToId() 
		{
			std::unordered_map<std::string, int> camera_model_name_to_id;

#define CAMERA_MODEL_CASE(CameraModel)                     \
  camera_model_name_to_id.emplace(CameraModel::model_name, \
                                  CameraModel::model_id);

			CAMERA_MODEL_CASES

#undef CAMERA_MODEL_CASE

				return camera_model_name_to_id;
		}

		std::unordered_map<int, std::string> InitialzeCameraModelIdToName() 
		{
			std::unordered_map<int, std::string> camera_model_id_to_name;

#define CAMERA_MODEL_CASE(CameraModel)                   \
  camera_model_id_to_name.emplace(CameraModel::model_id, \
                                  CameraModel::model_name);

			CAMERA_MODEL_CASES

#undef CAMERA_MODEL_CASE

				return camera_model_id_to_name;
		}

		static const std::unordered_map<std::string, int> CAMERA_MODEL_NAME_TO_ID =
			InitialzeCameraModelNameToId();

		static const std::unordered_map<int, std::string> CAMERA_MODEL_ID_TO_NAME =
			InitialzeCameraModelIdToName();

		bool ExistsCameraModelWithName(const std::string& model_name) 
		{
			return CAMERA_MODEL_NAME_TO_ID.count(model_name) > 0;
		}

		bool ExistsCameraModelWithId(const int model_id) 
		{
			return CAMERA_MODEL_ID_TO_NAME.count(model_id) > 0;
		}

		int CameraModelNameToId(const std::string& model_name) 
		{
			const auto it = CAMERA_MODEL_NAME_TO_ID.find(model_name);
			if (it == CAMERA_MODEL_NAME_TO_ID.end()) 
			{
				return kInvalidCameraModelId;
			}
			else 
			{
				return it->second;
			}
		}

		std::string CameraModelIdToName(const int model_id) 
		{
			const auto it = CAMERA_MODEL_ID_TO_NAME.find(model_id);
			if (it == CAMERA_MODEL_ID_TO_NAME.end()) 
			{
				return "";
			}
			else 
			{
				return it->second;
			}
		}

		std::vector<double> CameraModelInitializeParams(const int model_id,
			const double focal_length,
			const size_t width,
			const size_t height) 
		{
			
			
			
			switch (model_id) {
#define CAMERA_MODEL_CASE(CameraModel)                                 \
  case CameraModel::kModelId:                                          \
    return CameraModel::InitializeParams(focal_length, width, height); \
    break;

				CAMERA_MODEL_SWITCH_CASES

#undef CAMERA_MODEL_CASE
			}
		}

		std::string GetCameraModelParamsInfo(const int model_id) 
		{
			switch (model_id) {
#define CAMERA_MODEL_CASE(CameraModel) \
  case CameraModel::kModelId:          \
    return CameraModel::params_info;   \
    break;

				CAMERA_MODEL_SWITCH_CASES

#undef CAMERA_MODEL_CASE
			}

			return "Camera model does not exist";
		}

		static const std::vector<size_t> EMPTY_IDXS;

		const std::vector<size_t>& GetCameraModelFocalLengthIdxs(const int model_id) 
		{
			switch (model_id) 
			{
#define CAMERA_MODEL_CASE(CameraModel)     \
  case CameraModel::kModelId:              \
    return CameraModel::focal_length_idxs; \
    break;

				CAMERA_MODEL_SWITCH_CASES

#undef CAMERA_MODEL_CASE
			}

			return EMPTY_IDXS;
		}

		const std::vector<size_t>& GetCameraModelPrincipalPointIdxs(const int model_id) 
		{
			switch (model_id) 
			{
#define CAMERA_MODEL_CASE(CameraModel)        \
  case CameraModel::kModelId:                 \
    return CameraModel::principal_point_idxs; \
    break;

				CAMERA_MODEL_SWITCH_CASES

#undef CAMERA_MODEL_CASE
			}

			return EMPTY_IDXS;
		}

		const std::vector<size_t>& GetCameraModelExtraParamsIdxs(const int model_id) 
		{
			switch (model_id) 
			{
#define CAMERA_MODEL_CASE(CameraModel)     \
  case CameraModel::kModelId:              \
    return CameraModel::extra_params_idxs; \
    break;

				CAMERA_MODEL_SWITCH_CASES

#undef CAMERA_MODEL_CASE
			}

			return EMPTY_IDXS;
		}

		size_t GetCameraModelNumParams(const int model_id) 
		{
			switch (model_id) 
			{
#define CAMERA_MODEL_CASE(CameraModel) \
  case CameraModel::kModelId:          \
    return CameraModel::num_params;

				CAMERA_MODEL_SWITCH_CASES

#undef CAMERA_MODEL_CASE
			}

			return 0;
		}

		bool CameraModelVerifyParams(const int model_id,
			const std::vector<double>& params) 
		{
			switch (model_id) 
			{
#define CAMERA_MODEL_CASE(CameraModel)              \
  case CameraModel::kModelId:                       \
    if (params.size() == CameraModel::num_params) { \
      return true;                                  \
    }                                               \
    break;

				CAMERA_MODEL_SWITCH_CASES

#undef CAMERA_MODEL_CASE
			}

			return false;
		}

		bool CameraModelHasBogusParams(const int model_id,
			const std::vector<double>& params,
			const size_t width, const size_t height,
			const double min_focal_length_ratio,
			const double max_focal_length_ratio,
			const double max_extra_param) 
		{
			switch (model_id) 
			{
#define CAMERA_MODEL_CASE(CameraModel)                                         \
  case CameraModel::kModelId:                                                  \
    return CameraModel::HasBogusParams(                                        \
        params, width, height, min_focal_length_ratio, max_focal_length_ratio, \
        max_extra_param);                                                      \
    break;

				CAMERA_MODEL_SWITCH_CASES

#undef CAMERA_MODEL_CASE
			}

			return false;
		}


		
		

		std::string FullOpenCVCameraModel::InitializeParamsInfo()
		{
			return "fx, fy, cx, cy, k1, k2, p1, p2, k3, k4, k5, k6";
		}

		std::vector<size_t> FullOpenCVCameraModel::InitializeFocalLengthIdxs()
		{
			return { 0, 1 };
		}

		std::vector<size_t> FullOpenCVCameraModel::InitializePrincipalPointIdxs()
		{
			return { 2, 3 };
		}

		std::vector<size_t> FullOpenCVCameraModel::InitializeExtraParamsIdxs()
		{
			return { 4, 5, 7, 6, 8, 9, 10, 11 };
		}

		std::vector<double> FullOpenCVCameraModel::InitializeParams(
			const double focal_length, const size_t width, const size_t height)
		{
			return { focal_length,
					focal_length,
					width / 2.0,
					height / 2.0,
					0,
					0,
					0,
					0,
					0,
					0,
					0,
					0 };
		}


		
		

		std::string FOVCameraModel::InitializeParamsInfo()
		{
			return "fx, fy, cx, cy, omega";
		}

		std::vector<size_t> FOVCameraModel::InitializeFocalLengthIdxs()
		{
			return { 0, 1 };
		}

		std::vector<size_t> FOVCameraModel::InitializePrincipalPointIdxs()
		{
			return { 2, 3 };
		}

		std::vector<size_t> FOVCameraModel::InitializeExtraParamsIdxs()
		{
			return { 4 };
		}

		std::vector<double> FOVCameraModel::InitializeParams(const double focal_length,
			const size_t width,
			const size_t height)
		{
			return { focal_length, focal_length, width / 2.0, height / 2.0, 1e-2 };
		}

		std::string SimpleRadialFisheyeCameraModel::InitializeParamsInfo()
		{
			return "f, cx, cy, k";
		}

		std::vector<size_t>
			SimpleRadialFisheyeCameraModel::InitializeFocalLengthIdxs()
		{
			return { 0 };
		}

		std::vector<size_t>
			SimpleRadialFisheyeCameraModel::InitializePrincipalPointIdxs()
		{
			return { 1, 2 };
		}

		std::vector<size_t>
			SimpleRadialFisheyeCameraModel::InitializeExtraParamsIdxs()
		{
			return { 3 };
		}

		std::vector<double> SimpleRadialFisheyeCameraModel::InitializeParams(
			const double focal_length, const size_t width, const size_t height)
		{
			return { focal_length, width / 2.0, height / 2.0, 0 };
		}

		std::string RadialFisheyeCameraModel::InitializeParamsInfo()
		{
			return "f, cx, cy, k1, k2";
		}

		std::vector<size_t> RadialFisheyeCameraModel::InitializeFocalLengthIdxs()
		{
			return { 0 };
		}

		std::vector<size_t> RadialFisheyeCameraModel::InitializePrincipalPointIdxs()
		{
			return { 1, 2 };
		}

		std::vector<size_t> RadialFisheyeCameraModel::InitializeExtraParamsIdxs()
		{
			return { 3, 4 };
		}

		std::vector<double> RadialFisheyeCameraModel::InitializeParams(
			const double focal_length, const size_t width, const size_t height)
		{
			return { focal_length, width / 2.0, height / 2.0, 0, 0 };
		}

		std::string ThinPrismFisheyeCameraModel::InitializeParamsInfo()
		{
			return "fx, fy, cx, cy, k1, k2, p1, p2, k3, k4, sx1, sy1";
		}

		std::vector<size_t> ThinPrismFisheyeCameraModel::InitializeFocalLengthIdxs()
		{
			return { 0, 1 };
		}

		std::vector<size_t>
			ThinPrismFisheyeCameraModel::InitializePrincipalPointIdxs()
		{
			return { 2, 3 };
		}

		std::vector<size_t> ThinPrismFisheyeCameraModel::InitializeExtraParamsIdxs()
		{
			return { 4, 5, 6, 7, 8, 9, 10, 11 };
		}

		std::vector<double> ThinPrismFisheyeCameraModel::InitializeParams(
			const double focal_length, const size_t width, const size_t height)
		{
			return { focal_length,
					focal_length,
					width / 2.0,
					height / 2.0,
					0,
					0,
					0,
					0,
					0,
					0,
					0,
					0 };
		}

	}
} 
