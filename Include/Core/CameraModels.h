
#ifndef _AI3D_CORE_CAMERA_MODELS_H_
#define _AI3D_CORE_CAMERA_MODELS_H_

#include <cfloat>
#include <string>
#include <vector>
#include <unordered_map>

#include <boost/algorithm/string.hpp>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <math.h>

#include<Constants.h>
namespace AI3D
{

	namespace CORE
	{
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		

		static const int kInvalidCameraModelId = -1;

#ifndef CAMERA_MODEL_DEFINITIONS
#define CAMERA_MODEL_DEFINITIONS(model_id_value, model_name_value,             \
                                 num_params_value)                             \
  static const int kModelId = model_id_value;                                  \
  static const size_t kNumParams = num_params_value;                           \
  static const int model_id;                                                   \
  static const std::string model_name;                                         \
  static const size_t num_params;                                              \
  static const std::string params_info;                                        \
  static const std::vector<size_t> focal_length_idxs;                          \
  static const std::vector<size_t> principal_point_idxs;                       \
  static const std::vector<size_t> extra_params_idxs;                          \
                                                                               \
  static inline int InitializeModelId() { return model_id_value; };            \
  static inline std::string InitializeModelName() {                            \
    return model_name_value;                                                   \
  };                                                                           \
  static inline size_t InitializeNumParams() { return num_params_value; };     \
  static inline std::string InitializeParamsInfo();                            \
  static inline std::vector<size_t> InitializeFocalLengthIdxs();               \
  static inline std::vector<size_t> InitializePrincipalPointIdxs();            \
  static inline std::vector<size_t> InitializeExtraParamsIdxs();               \
  static inline std::vector<double> InitializeParams(                          \
      const double focal_length, const size_t width, const size_t height);     \
                                                                               \
  template <typename T>                                                        \
  static void WorldToImage(const T* params, const T u, const T v, T* x, T* y); \
  template <typename T>                                                        \
  static void ImageToWorld(const T* params, const T x, const T y, T* u, T* v); \
  template <typename T>                                                        \
  static void Distortion(const T* extra_params, const T u, const T v, T* du,   \
                         T* dv);
#endif

#ifndef CAMERA_MODEL_CASES
#define CAMERA_MODEL_CASES                          \
  CAMERA_MODEL_CASE(SimplePinholeCameraModel)       \
  CAMERA_MODEL_CASE(PinholeCameraModel)             \
  CAMERA_MODEL_CASE(SimpleRadialCameraModel)        \
  CAMERA_MODEL_CASE(SimpleRadialFisheyeCameraModel) \
  CAMERA_MODEL_CASE(RadialCameraModel)              \
  CAMERA_MODEL_CASE(RadialFisheyeCameraModel)       \
  CAMERA_MODEL_CASE(OpenCVCameraModel)              \
  CAMERA_MODEL_CASE(OpenCVFisheyeCameraModel)       \
  CAMERA_MODEL_CASE(FullOpenCVCameraModel)          \
  CAMERA_MODEL_CASE(FOVCameraModel)                 \
  CAMERA_MODEL_CASE(ThinPrismFisheyeCameraModel)
#endif

#ifndef CAMERA_MODEL_SWITCH_CASES
#define CAMERA_MODEL_SWITCH_CASES         \
  CAMERA_MODEL_CASES                      \
  default:                                \
    CAMERA_MODEL_DOES_NOT_EXIST_EXCEPTION \
    break;
#endif

#define CAMERA_MODEL_DOES_NOT_EXIST_EXCEPTION \
  throw std::domain_error("Camera model does not exist");

		
		
		
		template <typename CameraModel>
		struct AI3D_API BaseCameraModel
		{
			template <typename T>
			static inline bool HasBogusParams(const std::vector<T>& params,
				const size_t width, const size_t height,
				const T min_focal_length_ratio,
				const T max_focal_length_ratio,
				const T max_extra_param);

			template <typename T>
			static inline bool HasBogusFocalLength(const std::vector<T>& params,
				const size_t width,
				const size_t height,
				const T min_focal_length_ratio,
				const T max_focal_length_ratio);

			template <typename T>
			static inline bool HasBogusPrincipalPoint(const std::vector<T>& params,
				const size_t width,
				const size_t height);

			template <typename T>
			static inline bool HasBogusExtraParams(const std::vector<T>& params,
				const T max_extra_param);

			template <typename T>
			static inline T ImageToWorldThreshold(const T* params, const T threshold);

			template <typename T>
			static inline void IterativeUndistortion(const T* params, T* u, T* v);
		};

		
		
		
		
		
		
		
		
		
		struct AI3D_API SimplePinholeCameraModel
			: public BaseCameraModel<SimplePinholeCameraModel>
		{
			CAMERA_MODEL_DEFINITIONS(0, "SIMPLE_PINHOLE", 3)
		};

		
		
		
		
		
		
		
		
		
		struct AI3D_API PinholeCameraModel : public BaseCameraModel<PinholeCameraModel>
		{
			CAMERA_MODEL_DEFINITIONS(1, "PINHOLE", 4)
		};

		
		
		
		
		
		
		
		
		
		
		
		struct AI3D_API SimpleRadialCameraModel
			: public BaseCameraModel<SimpleRadialCameraModel>
		{
			CAMERA_MODEL_DEFINITIONS(2, "SIMPLE_RADIAL", 4)
		};

		
		
		
		
		
		
		
		
		
		
		struct AI3D_API RadialCameraModel : public BaseCameraModel<RadialCameraModel>
		{
			CAMERA_MODEL_DEFINITIONS(3, "RADIAL", 5)
		};

		
		
		
		
		
		
		
		
		
		
		
		
		struct AI3D_API OpenCVCameraModel : public BaseCameraModel<OpenCVCameraModel>
		{
			CAMERA_MODEL_DEFINITIONS(4, "OPENCV", 8)
		};

		
		
		
		
		
		
		
		
		
		
		
		
		struct AI3D_API OpenCVFisheyeCameraModel
			: public BaseCameraModel<OpenCVFisheyeCameraModel>
		{
			CAMERA_MODEL_DEFINITIONS(5, "OPENCV_FISHEYE", 8)
		};

		
		
		
		
		
		
		
		
		
		
		
		struct AI3D_API FullOpenCVCameraModel : public BaseCameraModel<FullOpenCVCameraModel>
		{
			CAMERA_MODEL_DEFINITIONS(6, "FULL_OPENCV", 12)
		};

		
		
		
		
		
		
		
		
		
		
		
		
		
		
		struct AI3D_API FOVCameraModel : public BaseCameraModel<FOVCameraModel>
		{
			CAMERA_MODEL_DEFINITIONS(7, "FOV", 5)

				template <typename T>
			static void Undistortion(const T* extra_params, const T u, const T v, T* du,
				T* dv);
		};

		
		
		
		
		
		
		
		
		
		
		struct AI3D_API SimpleRadialFisheyeCameraModel
			: public BaseCameraModel<SimpleRadialFisheyeCameraModel>
		{
			CAMERA_MODEL_DEFINITIONS(8, "SIMPLE_RADIAL_FISHEYE", 4)
		};

		
		
		
		
		
		
		
		
		
		
		struct AI3D_API RadialFisheyeCameraModel
			: public BaseCameraModel<RadialFisheyeCameraModel>
		{
			CAMERA_MODEL_DEFINITIONS(9, "RADIAL_FISHEYE", 5)
		};

		
		
		
		
		
		
		
		
		
		
		
		
		struct AI3D_API ThinPrismFisheyeCameraModel
			: public BaseCameraModel<ThinPrismFisheyeCameraModel>
		{
			CAMERA_MODEL_DEFINITIONS(10, "THIN_PRISM_FISHEYE", 12)
		};

		
		bool ExistsCameraModelWithName(const std::string& model_name);
		bool ExistsCameraModelWithId(const int model_id);

		
		
		
		
		
		int CameraModelNameToId(const std::string& model_name);

		
		
		
		
		
		std::string CameraModelIdToName(const int model_id);

		
		
		
		
		
		
		
		
		
		std::vector<double> CameraModelInitializeParams(const int model_id,
			const double focal_length,
			const size_t width,
			const size_t height);

		
		
		
		std::string GetCameraModelParamsInfo(const int model_id);

		
		
		
		const std::vector<size_t>& GetCameraModelFocalLengthIdxs(const int model_id);
		const std::vector<size_t>& GetCameraModelPrincipalPointIdxs(const int model_id);
		const std::vector<size_t>& GetCameraModelExtraParamsIdxs(const int model_id);

		
		size_t GetCameraModelNumParams(const int model_id);

		
		
		
		
		
		bool CameraModelVerifyParams(const int model_id,
			const std::vector<double>& params);

		
		
		
		
		
		
		
		
		
		
		
		bool CameraModelHasBogusParams(const int model_id,
			const std::vector<double>& params,
			const size_t width, const size_t height,
			const double min_focal_length_ratio,
			const double max_focal_length_ratio,
			const double max_extra_param);

		
		
		
		
		
		
		
		
		
		inline void CameraModelWorldToImage(const int model_id,
			const std::vector<double>& params,
			const double u, const double v, double* x,
			double* y);

		
		
		
		
		
		
		
		
		inline void CameraModelImageToWorld(const int model_id,
			const std::vector<double>& params,
			const double x, const double y, double* u,
			double* v);

		
		
		
		
		
		
		
		
		inline double CameraModelImageToWorldThreshold(
			const int model_id, const std::vector<double>& params,
			const double threshold);

		
		
		

		
		

		template <typename CameraModel>
		template <typename T>
		bool BaseCameraModel<CameraModel>::HasBogusParams(
			const std::vector<T>& params, const size_t width, const size_t height,
			const T min_focal_length_ratio, const T max_focal_length_ratio,
			const T max_extra_param)
		{
			if (HasBogusPrincipalPoint(params, width, height))
			{
				return true;
			}

			if (HasBogusFocalLength(params, width, height, min_focal_length_ratio,
				max_focal_length_ratio))
			{
				return true;
			}

			if (HasBogusExtraParams(params, max_extra_param))
			{
				return true;
			}

			return false;
		}

		template <typename CameraModel>
		template <typename T>
		bool BaseCameraModel<CameraModel>::HasBogusFocalLength(
			const std::vector<T>& params, const size_t width, const size_t height,
			const T min_focal_length_ratio, const T max_focal_length_ratio)
		{
			const size_t max_size = std::max(width, height);

			for (const auto& idx : CameraModel::focal_length_idxs)
			{
				const T focal_length_ratio = params[idx] / max_size;
				if (focal_length_ratio < min_focal_length_ratio ||
					focal_length_ratio > max_focal_length_ratio)
				{
					return true;
				}
			}

			return false;
		}

		template <typename CameraModel>
		template <typename T>
		bool BaseCameraModel<CameraModel>::HasBogusPrincipalPoint(
			const std::vector<T>& params, const size_t width, const size_t height)
		{
			const T cx = params[CameraModel::principal_point_idxs[0]];
			const T cy = params[CameraModel::principal_point_idxs[1]];
			return cx < 0 || cx > width || cy < 0 || cy > height;
		}

		template <typename CameraModel>
		template <typename T>
		bool BaseCameraModel<CameraModel>::HasBogusExtraParams(
			const std::vector<T>& params, const T max_extra_param)
		{
			for (const auto& idx : CameraModel::extra_params_idxs)
			{
				if (std::abs(params[idx]) > max_extra_param)
				{
					return true;
				}
			}

			return false;
		}

		template <typename CameraModel>
		template <typename T>
		T BaseCameraModel<CameraModel>::ImageToWorldThreshold(const T* params,
			const T threshold)
		{
			T mean_focal_length = 0;
			for (const auto& idx : CameraModel::focal_length_idxs)
			{
				mean_focal_length += params[idx];
			}
			mean_focal_length /= CameraModel::focal_length_idxs.size();
			return threshold / mean_focal_length;
		}

		template <typename CameraModel>
		template <typename T>
		void BaseCameraModel<CameraModel>::IterativeUndistortion(const T* params, T* u,
			T* v)
		{
			
			
			
			const size_t kNumIterations = 100;
			const double kMaxStepNorm = 1e-10;
			const double kRelStepSize = 1e-6;

			Eigen::Matrix2d J;
			const Eigen::Vector2d x0(*u, *v);
			Eigen::Vector2d x(*u, *v);
			Eigen::Vector2d dx;
			Eigen::Vector2d dx_0b;
			Eigen::Vector2d dx_0f;
			Eigen::Vector2d dx_1b;
			Eigen::Vector2d dx_1f;

			for (size_t i = 0; i < kNumIterations; ++i)
			{
				const double step0 = std::max(std::numeric_limits<double>::epsilon(),
					std::abs(kRelStepSize * x(0)));
				const double step1 = std::max(std::numeric_limits<double>::epsilon(),
					std::abs(kRelStepSize * x(1)));
				CameraModel::Distortion(params, x(0), x(1), &dx(0), &dx(1));
				CameraModel::Distortion(params, x(0) - step0, x(1), &dx_0b(0), &dx_0b(1));
				CameraModel::Distortion(params, x(0) + step0, x(1), &dx_0f(0), &dx_0f(1));
				CameraModel::Distortion(params, x(0), x(1) - step1, &dx_1b(0), &dx_1b(1));
				CameraModel::Distortion(params, x(0), x(1) + step1, &dx_1f(0), &dx_1f(1));
				J(0, 0) = 1 + (dx_0f(0) - dx_0b(0)) / (2 * step0);
				J(0, 1) = (dx_1f(0) - dx_1b(0)) / (2 * step1);
				J(1, 0) = (dx_0f(1) - dx_0b(1)) / (2 * step0);
				J(1, 1) = 1 + (dx_1f(1) - dx_1b(1)) / (2 * step1);
				const Eigen::Vector2d step_x = J.inverse() * (x + dx - x0);
				x -= step_x;
				if (step_x.squaredNorm() < kMaxStepNorm)
				{
					break;
				}
			}

			*u = x(0);
			*v = x(1);
		}

		
		

		std::string SimplePinholeCameraModel::InitializeParamsInfo()
		{
			return "f, cx, cy";
		}

		std::vector<size_t> SimplePinholeCameraModel::InitializeFocalLengthIdxs()
		{
			return { 0 };
		}

		std::vector<size_t> SimplePinholeCameraModel::InitializePrincipalPointIdxs()
		{
			return { 1, 2 };
		}

		std::vector<size_t> SimplePinholeCameraModel::InitializeExtraParamsIdxs()
		{
			return {};
		}

		std::vector<double> SimplePinholeCameraModel::InitializeParams(
			const double focal_length, const size_t width, const size_t height)
		{
			return { focal_length, width / 2.0, height / 2.0 };
		}

		template <typename T>
		void SimplePinholeCameraModel::WorldToImage(const T* params, const T u,
			const T v, T* x, T* y)
		{
			const T f = params[0];
			const T c1 = params[1];
			const T c2 = params[2];

			

			
			*x = f * u + c1;
			*y = f * v + c2;
		}

		template <typename T>
		void SimplePinholeCameraModel::ImageToWorld(const T* params, const T x,
			const T y, T* u, T* v)
		{
			const T f = params[0];
			const T c1 = params[1];
			const T c2 = params[2];

			*u = (x - c1) / f;
			*v = (y - c2) / f;
		}

		
		

		std::string PinholeCameraModel::InitializeParamsInfo()
		{
			return "fx, fy, cx, cy";
		}

		std::vector<size_t> PinholeCameraModel::InitializeFocalLengthIdxs()
		{
			return { 0, 1 };
		}

		std::vector<size_t> PinholeCameraModel::InitializePrincipalPointIdxs()
		{
			return { 2, 3 };
		}

		std::vector<size_t> PinholeCameraModel::InitializeExtraParamsIdxs()
		{
			return {};
		}

		std::vector<double> PinholeCameraModel::InitializeParams(
			const double focal_length, const size_t width, const size_t height)
		{
			return { focal_length, focal_length, width / 2.0, height / 2.0 };
		}

		template <typename T>
		void PinholeCameraModel::WorldToImage(const T* params, const T u, const T v,
			T* x, T* y)
		{
			const T f1 = params[0];
			const T f2 = params[1];
			const T c1 = params[2];
			const T c2 = params[3];

			

			
			*x = f1 * u + c1;
			*y = f2 * v + c2;
		}

		template <typename T>
		void PinholeCameraModel::ImageToWorld(const T* params, const T x, const T y,
			T* u, T* v)
		{
			const T f1 = params[0];
			const T f2 = params[1];
			const T c1 = params[2];
			const T c2 = params[3];

			*u = (x - c1) / f1;
			*v = (y - c2) / f2;
		}

		
		

		std::string SimpleRadialCameraModel::InitializeParamsInfo()
		{
			return "f, cx, cy, k";
		}

		std::vector<size_t> SimpleRadialCameraModel::InitializeFocalLengthIdxs()
		{
			return { 0 };
		}

		std::vector<size_t> SimpleRadialCameraModel::InitializePrincipalPointIdxs()
		{
			return { 1, 2 };
		}

		std::vector<size_t> SimpleRadialCameraModel::InitializeExtraParamsIdxs()
		{
			return { 3 };
		}

		std::vector<double> SimpleRadialCameraModel::InitializeParams(
			const double focal_length, const size_t width, const size_t height)
		{
			return { focal_length, width / 2.0, height / 2.0, 0 };
		}

		template <typename T>
		void SimpleRadialCameraModel::WorldToImage(const T* params, const T u,
			const T v, T* x, T* y)
		{
			const T f = params[0];
			const T c1 = params[1];
			const T c2 = params[2];

			
			T du, dv;
			Distortion(&params[3], u, v, &du, &dv);
			*x = u + du;
			*y = v + dv;

			
			*x = f * *x + c1;
			*y = f * *y + c2;
		}

		template <typename T>
		void SimpleRadialCameraModel::ImageToWorld(const T* params, const T x,
			const T y, T* u, T* v)
		{
			const T f = params[0];
			const T c1 = params[1];
			const T c2 = params[2];

			
			*u = (x - c1) / f;
			*v = (y - c2) / f;

			IterativeUndistortion(&params[3], u, v);
		}

		template <typename T>
		void SimpleRadialCameraModel::Distortion(const T* extra_params, const T u,
			const T v, T* du, T* dv)
		{
			const T k = extra_params[0];

			const T u2 = u * u;
			const T v2 = v * v;
			const T r2 = u2 + v2;
			const T radial = k * r2;
			*du = u * radial;
			*dv = v * radial;
		}

		
		

		std::string RadialCameraModel::InitializeParamsInfo()
		{
			return "f, cx, cy, k1, k2";
		}

		std::vector<size_t> RadialCameraModel::InitializeFocalLengthIdxs()
		{
			return { 0 };
		}

		std::vector<size_t> RadialCameraModel::InitializePrincipalPointIdxs()
		{
			return { 1, 2 };
		}

		std::vector<size_t> RadialCameraModel::InitializeExtraParamsIdxs()
		{
			return { 3, 4 };
		}

		std::vector<double> RadialCameraModel::InitializeParams(
			const double focal_length, const size_t width, const size_t height)
		{
			return { focal_length, width / 2.0, height / 2.0, 0, 0 };
		}

		template <typename T>
		void RadialCameraModel::WorldToImage(const T* params, const T u, const T v,
			T* x, T* y)
		{
			const T f = params[0];
			const T c1 = params[1];
			const T c2 = params[2];

			
			T du, dv;
			Distortion(&params[3], u, v, &du, &dv);
			*x = u + du;
			*y = v + dv;

			
			*x = f * *x + c1;
			*y = f * *y + c2;
		}

		template <typename T>
		void RadialCameraModel::ImageToWorld(const T* params, const T x, const T y,
			T* u, T* v)
		{
			const T f = params[0];
			const T c1 = params[1];
			const T c2 = params[2];

			
			*u = (x - c1) / f;
			*v = (y - c2) / f;

			IterativeUndistortion(&params[3], u, v);
		}

		template <typename T>
		void RadialCameraModel::Distortion(const T* extra_params, const T u, const T v,
			T* du, T* dv)
		{
			const T k1 = extra_params[0];
			const T k2 = extra_params[1];

			const T u2 = u * u;
			const T v2 = v * v;
			const T r2 = u2 + v2;
			const T radial = k1 * r2 + k2 * r2 * r2;
			*du = u * radial;
			*dv = v * radial;
		}

		
		

		std::string OpenCVCameraModel::InitializeParamsInfo()
		{
			return "fx, fy, cx, cy, k1, k2, p1, p2";
		}

		std::vector<size_t> OpenCVCameraModel::InitializeFocalLengthIdxs()
		{
			return { 0, 1 };
		}

		std::vector<size_t> OpenCVCameraModel::InitializePrincipalPointIdxs()
		{
			return { 2, 3 };
		}

		std::vector<size_t> OpenCVCameraModel::InitializeExtraParamsIdxs()
		{
			return { 4, 5, 6, 7 };
		}

		std::vector<double> OpenCVCameraModel::InitializeParams(
			const double focal_length, const size_t width, const size_t height)
		{
			return { focal_length, focal_length, width / 2.0, height / 2.0, 0, 0, 0, 0 };
		}

		template <typename T>
		void OpenCVCameraModel::WorldToImage(const T* params, const T u, const T v,
			T* x, T* y)
		{
			const T f1 = params[0];
			const T f2 = params[1];
			const T c1 = params[2];
			const T c2 = params[3];

			
			T du, dv;
			Distortion(&params[4], u, v, &du, &dv);
			*x = u + du;
			*y = v + dv;

			
			*x = f1 * *x + c1;
			*y = f2 * *y + c2;
		}

		template <typename T>
		void OpenCVCameraModel::ImageToWorld(const T* params, const T x, const T y,
			T* u, T* v)
		{
			const T f1 = params[0];
			const T f2 = params[1];
			const T c1 = params[2];
			const T c2 = params[3];

			
			*u = (x - c1) / f1;
			*v = (y - c2) / f2;

			IterativeUndistortion(&params[4], u, v);
		}

		template <typename T>
		void OpenCVCameraModel::Distortion(const T* extra_params, const T u, const T v,
			T* du, T* dv)
		{
			const T k1 = extra_params[0];
			const T k2 = extra_params[1];
			const T p1 = extra_params[2];
			const T p2 = extra_params[3];

			const T u2 = u * u;
			const T uv = u * v;
			const T v2 = v * v;
			const T r2 = u2 + v2;
			const T radial = k1 * r2 + k2 * r2 * r2;
			*du = u * radial + T(2) * p1 * uv + p2 * (r2 + T(2) * u2);
			*dv = v * radial + T(2) * p2 * uv + p1 * (r2 + T(2) * v2);
		}

		
		

		std::string OpenCVFisheyeCameraModel::InitializeParamsInfo()
		{
			return "fx, fy, cx, cy, k1, k2, k3, k4";
		}

		std::vector<size_t> OpenCVFisheyeCameraModel::InitializeFocalLengthIdxs()
		{
			return { 0, 1 };
		}

		std::vector<size_t> OpenCVFisheyeCameraModel::InitializePrincipalPointIdxs()
		{
			return { 2, 3 };
		}

		std::vector<size_t> OpenCVFisheyeCameraModel::InitializeExtraParamsIdxs()
		{
			return { 4, 5, 6, 7 };
		}

		std::vector<double> OpenCVFisheyeCameraModel::InitializeParams(
			const double focal_length, const size_t width, const size_t height)
		{
			return { focal_length, focal_length, width / 2.0, height / 2.0, 0, 0, 0, 0 };
		}

		template <typename T>
		void OpenCVFisheyeCameraModel::WorldToImage(const T* params, const T u,
			const T v, T* x, T* y)
		{
			const T f1 = params[0];
			const T f2 = params[1];
			const T c1 = params[2];
			const T c2 = params[3];

			
			T du, dv;
			Distortion(&params[4], u, v, &du, &dv);
			*x = u + du;
			*y = v + dv;

			
			*x = f1 * *x + c1;
			*y = f2 * *y + c2;
		}

		template <typename T>
		void OpenCVFisheyeCameraModel::ImageToWorld(const T* params, const T x,
			const T y, T* u, T* v)
		{
			const T f1 = params[0];
			const T f2 = params[1];
			const T c1 = params[2];
			const T c2 = params[3];

			
			*u = (x - c1) / f1;
			*v = (y - c2) / f2;

			IterativeUndistortion(&params[4], u, v);
		}

		template <typename T>
		void OpenCVFisheyeCameraModel::Distortion(const T* extra_params, const T u,
			const T v, T* du, T* dv)
		{
			const T k1 = extra_params[0];
			const T k2 = extra_params[1];
			const T k3 = extra_params[2];
			const T k4 = extra_params[3];

			const T r = std::sqrt(u * u + v * v);

			if (r > T(std::numeric_limits<double>::epsilon()))
			{
				const T theta = std::atan(r);
				const T theta2 = theta * theta;
				const T theta4 = theta2 * theta2;
				const T theta6 = theta4 * theta2;
				const T theta8 = theta4 * theta4;
				const T thetad =
					theta * (T(1) + k1 * theta2 + k2 * theta4 + k3 * theta6 + k4 * theta8);
				*du = u * thetad / r - u;
				*dv = v * thetad / r - v;
			}
			else {
				*du = T(0);
				*dv = T(0);
			}
		}


		template <typename T>
		void FullOpenCVCameraModel::WorldToImage(const T* params, const T u, const T v,
			T* x, T* y)
		{
			const T f1 = params[0];
			const T f2 = params[1];
			const T c1 = params[2];
			const T c2 = params[3];

			
			T du, dv;
			Distortion(&params[4], u, v, &du, &dv);
			*x = u + du;
			*y = v + dv;

			
			*x = f1 * *x + c1;
			*y = f2 * *y + c2;
		}

		template <typename T>
		void FullOpenCVCameraModel::ImageToWorld(const T* params, const T x, const T y,
			T* u, T* v)
		{
			const T f1 = params[0];
			const T f2 = params[1];
			const T c1 = params[2];
			const T c2 = params[3];

			
			*u = (x - c1) / f1;
			*v = (y - c2) / f2;

			IterativeUndistortion(&params[4], u, v);
		}

		template <typename T>
		void FullOpenCVCameraModel::Distortion(const T* extra_params, const T u,
			const T v, T* du, T* dv)
		{
			const T k1 = extra_params[0];
			const T k2 = extra_params[1];
			const T p1 = extra_params[3];
			const T p2 = extra_params[2];
			const T k3 = extra_params[4];
			const T k4 = extra_params[5];
			const T k5 = extra_params[6];
			const T k6 = extra_params[7];

			const T u2 = u * u;
			const T uv = u * v;
			const T v2 = v * v;
			const T r2 = u2 + v2;
			const T r4 = r2 * r2;
			const T r6 = r4 * r2;
			const T radial = (T(1) + k1 * r2 + k2 * r4 + k3 * r6) /
				(T(1) + k4 * r2 + k5 * r4 + k6 * r6);
			*du = u * radial + T(2) * p1 * uv + p2 * (r2 + T(2) * u2) - u;
			*dv = v * radial + T(2) * p2 * uv + p1 * (r2 + T(2) * v2) - v;
		}

		template <typename T>
		void FOVCameraModel::WorldToImage(const T* params, const T u, const T v, T* x,
			T* y)
		{
			const T f1 = params[0];
			const T f2 = params[1];
			const T c1 = params[2];
			const T c2 = params[3];

			
			Distortion(&params[4], u, v, x, y);

			
			*x = f1 * *x + c1;
			*y = f2 * *y + c2;
		}

		template <typename T>
		void FOVCameraModel::ImageToWorld(const T* params, const T x, const T y, T* u,
			T* v)
		{
			const T f1 = params[0];
			const T f2 = params[1];
			const T c1 = params[2];
			const T c2 = params[3];

			
			const T uu = (x - c1) / f1;
			const T vv = (y - c2) / f2;

			
			Undistortion(&params[4], uu, vv, u, v);
		}

		template <typename T>
		void FOVCameraModel::Distortion(const T* extra_params, const T u, const T v,
			T* du, T* dv)
		{
			const T omega = extra_params[0];

			
			const T kEpsilon = T(1e-4);

			const T radius2 = u * u + v * v;
			const T omega2 = omega * omega;

			T factor;
			if (omega2 < kEpsilon)
			{
				
				
				
				
				
				factor = (omega2 * radius2) / T(3) - omega2 / T(12) + T(1);
			}
			else if (radius2 < kEpsilon)
			{
				
				
				
				
				
				const T tan_half_omega = std::tan(omega / T(2));
				factor = (T(-2) * tan_half_omega *
					(T(4) * radius2 * tan_half_omega * tan_half_omega - T(3))) /
					(T(3) * omega);
			}
			else
			{
				const T radius = std::sqrt(radius2);
				const T numerator = std::atan(radius * T(2) * std::tan(omega / T(2)));
				factor = numerator / (radius * omega);
			}

			*du = u * factor;
			*dv = v * factor;
		}

		template <typename T>
		void FOVCameraModel::Undistortion(const T* extra_params, const T u, const T v,
			T* du, T* dv)
		{
			T omega = extra_params[0];

			
			const T kEpsilon = T(1e-4);

			const T radius2 = u * u + v * v;
			const T omega2 = omega * omega;

			T factor;
			if (omega2 < kEpsilon)
			{
				
				
				
				
				
				factor = (omega2 * radius2) / T(3) - omega2 / T(12) + T(1);
			}
			else if (radius2 < kEpsilon)
			{
				
				
				
				
				
				factor = (omega * (omega * omega * radius2 + T(3))) /
					(T(6) * std::tan(omega / T(2)));
			}
			else
			{
				const T radius = std::sqrt(radius2);
				const T numerator = std::tan(radius * omega);
				factor = numerator / (radius * T(2) * std::tan(omega / T(2)));
			}

			*du = u * factor;
			*dv = v * factor;
		}

		
		
		template <typename T>
		void SimpleRadialFisheyeCameraModel::WorldToImage(const T* params, const T u,
			const T v, T* x, T* y)
		{
			const T f = params[0];
			const T c1 = params[1];
			const T c2 = params[2];

			
			T du, dv;
			Distortion(&params[3], u, v, &du, &dv);
			*x = u + du;
			*y = v + dv;

			
			*x = f * *x + c1;
			*y = f * *y + c2;
		}

		template <typename T>
		void SimpleRadialFisheyeCameraModel::ImageToWorld(const T* params, const T x,
			const T y, T* u, T* v)
		{
			const T f = params[0];
			const T c1 = params[1];
			const T c2 = params[2];

			
			*u = (x - c1) / f;
			*v = (y - c2) / f;

			IterativeUndistortion(&params[3], u, v);
		}

		template <typename T>
		void SimpleRadialFisheyeCameraModel::Distortion(const T* extra_params,
			const T u, const T v, T* du,
			T* dv)
		{
			const T k = extra_params[0];

			const T r = std::sqrt(u * u + v * v);

			if (r > T(std::numeric_limits<double>::epsilon())) {
				const T theta = std::atan(r);
				const T theta2 = theta * theta;
				const T thetad = theta * (T(1) + k * theta2);
				*du = u * thetad / r - u;
				*dv = v * thetad / r - v;
			}
			else {
				*du = T(0);
				*dv = T(0);
			}
		}

		
		
		template <typename T>
		void RadialFisheyeCameraModel::WorldToImage(const T* params, const T u,
			const T v, T* x, T* y)
		{
			const T f = params[0];
			const T c1 = params[1];
			const T c2 = params[2];

			
			T du, dv;
			Distortion(&params[3], u, v, &du, &dv);
			*x = u + du;
			*y = v + dv;

			
			*x = f * *x + c1;
			*y = f * *y + c2;
		}

		template <typename T>
		void RadialFisheyeCameraModel::ImageToWorld(const T* params, const T x,
			const T y, T* u, T* v)
		{
			const T f = params[0];
			const T c1 = params[1];
			const T c2 = params[2];

			
			*u = (x - c1) / f;
			*v = (y - c2) / f;

			IterativeUndistortion(&params[3], u, v);
		}

		template <typename T>
		void RadialFisheyeCameraModel::Distortion(const T* extra_params, const T u,
			const T v, T* du, T* dv)
		{
			const T k1 = extra_params[0];
			const T k2 = extra_params[1];

			const T r = std::sqrt(u * u + v * v);

			if (r > T(std::numeric_limits<double>::epsilon()))
			{
				const T theta = std::atan(r);
				const T theta2 = theta * theta;
				const T theta4 = theta2 * theta2;
				const T thetad =
					theta * (T(1) + k1 * theta2 + k2 * theta4);
				*du = u * thetad / r - u;
				*dv = v * thetad / r - v;
			}
			else {
				*du = T(0);
				*dv = T(0);
			}
		}

		
		

		template <typename T>
		void ThinPrismFisheyeCameraModel::WorldToImage(const T* params, const T u,
			const T v, T* x, T* y)
		{
			const T f1 = params[0];
			const T f2 = params[1];
			const T c1 = params[2];
			const T c2 = params[3];

			const T r = std::sqrt(u * u + v * v);

			T uu, vv;
			if (r > T(std::numeric_limits<double>::epsilon()))
			{
				const T theta = std::atan(r);
				uu = theta * u / r;
				vv = theta * v / r;
			}
			else {
				uu = u;
				vv = v;
			}

			
			T du, dv;
			Distortion(&params[4], uu, vv, &du, &dv);
			*x = uu + du;
			*y = vv + dv;

			
			*x = f1 * *x + c1;
			*y = f2 * *y + c2;
		}

		template <typename T>
		void ThinPrismFisheyeCameraModel::ImageToWorld(const T* params, const T x,
			const T y, T* u, T* v)
		{
			const T f1 = params[0];
			const T f2 = params[1];
			const T c1 = params[2];
			const T c2 = params[3];

			
			*u = (x - c1) / f1;
			*v = (y - c2) / f2;

			IterativeUndistortion(&params[4], u, v);

			const T theta = std::sqrt(*u * *u + *v * *v);
			const T theta_cos_theta = theta * std::cos(theta);
			if (theta_cos_theta > T(std::numeric_limits<double>::epsilon()))
			{
				const T scale = std::sin(theta) / theta_cos_theta;
				*u *= scale;
				*v *= scale;
			}
		}

		template <typename T>
		void ThinPrismFisheyeCameraModel::Distortion(const T* extra_params, const T u,
			const T v, T* du, T* dv)
		{
			const T k1 = extra_params[0];
			const T k2 = extra_params[1];
			const T p1 = extra_params[2];
			const T p2 = extra_params[3];
			const T k3 = extra_params[4];
			const T k4 = extra_params[5];
			const T sx1 = extra_params[6];
			const T sy1 = extra_params[7];

			const T u2 = u * u;
			const T uv = u * v;
			const T v2 = v * v;
			const T r2 = u2 + v2;
			const T r4 = r2 * r2;
			const T r6 = r4 * r2;
			const T r8 = r6 * r2;
			const T radial = k1 * r2 + k2 * r4 + k3 * r6 + k4 * r8;
			*du = u * radial + T(2) * p1 * uv + p2 * (r2 + T(2) * u2) + sx1 * r2;
			*dv = v * radial + T(2) * p2 * uv + p1 * (r2 + T(2) * v2) + sy1 * r2;
		}

		

		void CameraModelWorldToImage(const int model_id,
			const std::vector<double>& params, const double u,
			const double v, double* x, double* y) {
			switch (model_id)
			{
#define CAMERA_MODEL_CASE(CameraModel)                    \
  case CameraModel::kModelId:                             \
    CameraModel::WorldToImage(params.data(), u, v, x, y); \
    break;

				CAMERA_MODEL_SWITCH_CASES

#undef CAMERA_MODEL_CASE
			}
		}

		void CameraModelImageToWorld(const int model_id,
			const std::vector<double>& params, const double x,
			const double y, double* u, double* v) {
			switch (model_id)
			{
#define CAMERA_MODEL_CASE(CameraModel)                    \
  case CameraModel::kModelId:                             \
    CameraModel::ImageToWorld(params.data(), x, y, u, v); \
    break;

				CAMERA_MODEL_SWITCH_CASES

#undef CAMERA_MODEL_CASE
			}
		}

		double CameraModelImageToWorldThreshold(const int model_id,
			const std::vector<double>& params,
			const double threshold)
		{
			switch (model_id)
			{
#define CAMERA_MODEL_CASE(CameraModel)                                   \
  case CameraModel::kModelId:                                            \
    return CameraModel::ImageToWorldThreshold(params.data(), threshold); \
    break;

				CAMERA_MODEL_SWITCH_CASES

#undef CAMERA_MODEL_CASE
			}

			return -1;
		}
	}
} 

#endif  
