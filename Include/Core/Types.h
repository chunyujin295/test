#ifndef _AI3D_CORE_TYPES_H_
#define _AI3D_CORE_TYPES_H_

#include "Core/alignment.h"
#include "Core/String.h"
#include "Core/Rapidjson.h"

#include <ogr_spatialref.h>
#include <iostream>
#include <map>
#include <set>
#ifdef _MSC_VER
#if _MSC_VER >= 1600
#include <cstdint>
#else
typedef __int8 int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#endif
#elif __GNUC__ >= 3
#include <cstdint>
#endif


#define NON_COPYABLE(class_name)          \
  class_name(class_name const&) = delete; \
  void operator=(class_name const& obj) = delete;
#define NON_MOVABLE(class_name) class_name(class_name&&) = delete;

const bool BLK_USE_BIN = true;
const bool SOURCEDATA_USE_BIN = true;
const bool JOB_FEEDBACK_USE_BIN = true;
const bool JOB_INFO_USE_BIN = true;
const bool TASK_USE_BIN = true;
const bool PROJECT_USE_BIN = true;
const bool ENGINE_USE_BIN = true;
const bool SRS_USE_BIN = true;
const bool POS_USE_BIN = true;
const bool STAT_USE_BIN = true;

#include <Eigen/Core>
#define ORIDATABIN "OD.bin"
#define ORIDATAJSON "OD.json"
#define BLKFILENAME "BI"
#define PROJECTPOSTFIX "mai"
#define DOTPROJECTPOSTFIX ".mai"
#define PROJECTFILEBAK ".mai.bak"
#define BINPROJECTPOSTFIX "mai"
#define BINDOTPROJECTPOSTFIX ".mai"
#define BINPROJECTFILEBAK ".mai.bak"
#define BLOCKFILE ".blk"
#define BLOCKBAKFILE ".blk.bak"
#define BLOCKBINFILE ".bbin"
#define BLOCKBINBAKFILE ".bbin.bak"
#define BLOCK_PRE  "Block_"
#define XML_EXTENSION ".xml";
#define BINARY_EXTENSION ".bin"

#define TIEPOINTS "CP.bin"
#define TIEPOINTSNAME "CP"
#define SCBLOCKBIN "SCB.bin"
#define BAK ".bak"
#define JOBNAMELENGTH_AT 21
#define FEEDBACKFILELENGTH_AT 35
#define TIMEFILELENGTH_AT 31
#define NOMINMAX
#define FEEDBACK_PREFIX "feedback_"
#define TIME_PREFIX "time_"
#define LOCKFILE_POSTFIX ".lock"
#define JSONFILE_POSTFIX ".json"
#define FEEDBACK_BIN_PREFIX "JF_"
#define TIME_BIN_PREFIX "JT_"
#define BINFILE_POSTFIX ".bin"
#define JOB_BIN_PREFIX "JI_"
#define STAT_MASTER_POSTFIX "_M"
#define STAT_ENGINE_POSTFIX "_E"

#define CAMERA_DATABASE "CD.txt"
#define SRS_DATABASE "srs.db"
#define USER_DATABASE "usersrs.db"
#define PROJ_DATABASE "proj.db"




#define JOBPENDINGSTR "Pending"
#define JOBRUNNINGSTR "Running"
#define JOBCOMPLETEDSTR "Completed"
#define JOBCANCELLEDSTR "Cancelled"
#define JOBFAILEDSTR "Failed"
#define JOBENGINESSTR "Engines"


#define PJOBPENDINGSTR "PPending"
#define PJOBRUNNINGSTR "PRunning"
#define PJOBCOMPLETEDSTR "PCompleted"
#define PJOBCANCELLEDSTR "PCancelled"
#define PJOBFAILEDSTR "PFailed"

#define TASK_DEF_ZERO_FILE "task_def_0.json"
#define TASK_DEF_PREFIX "task_def_"
#define TASK_DEF_ZERO_BIN_FILE "TI_0.bin"
#define TASK_DEF_ZERO_JSON_FILE "TI_0.json"
#define TASK_DEF_BIN_PREFIX "TI_"

#define SOURCE_DATA_JSON "source_data.json"
#define MANUAL_TIES_PATH "manual_ties_path"


#define BATCH_STRING "BATCH"
#define TILE_STRING "TILE"

#define AT_PREFIX "AT_"
#define AT_POSTFIX "_AT"
#define TILE_PREFIX "TILE_"
#define TILE_POSTFIX "_TILE"
#define SC_PREFIX "SC_"
#define SC_POSTFIX "_SC"
#define JOB_PREFIX "J_"
#define JOB_TYPE_SC "SC"
#define JOB_TYPE_RE "TILE"
#define RECONSTRUCT_PREFIX "Reconstruction_"
#define PRODUCTION_PREFIX "Production_"
#define PRODUCTION_DIR "Productions"
#define BATCH_PREFIX "BATCH_"
#define BATCH_POSTFIX "_BATCH"

#define PRODUCTIONVIEWIDSBIN "RB.bin"
#define TILEIMAGES "pick_list.txt"
#define TILEBBOX "polygon.txt"
#define TILEBBOXCUT "polygon_cut.txt"
#define SCBINFILE "SCSFR.bin"
#define UNDISTORTPATH "RU"
#define COLORBIN "RGBC.bin"
#define SCIMAG "SCI"
#define SCCOVERIMAG "SCC.jpg"
#define SCSCENEIMAG "SSC.jpg"
#define SRSBIN "SRS.bin"
#define SRSJSON "SRS.json"
#define POSJSON "PL.json"
#define POSBIN "PL.bin"
#define CONSTRAINTFILE "CON.bin"


#define ATSTARTTYPE 1
#define ATRUNNINGTYPE 2
#define ATCOMPLETETYPE 0
#define ATLASTTASKTYPE 3
#define BATCHSTARTTYPE 4

#define RECONSTRUCTIONSTARTTYPE 4

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif
namespace Eigen 
{

typedef Eigen::Matrix<double, 3, 4> Matrix3x4d;
typedef Eigen::Matrix<uint8_t, 3, 1> Vector3ub;
typedef Eigen::Matrix<uint8_t, 4, 1> Vector4ub;
typedef Eigen::Matrix<double, 6, 1> Vector6d;

}  
#define MAKE_FEEDBAK_FILE(p,j) (AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(p))) + FEEDBACK_PREFIX + std::string(j) + JSONFILE_POSTFIX)
#define MAKE_TIMESUM_FILE(p,j) (AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(p))) + TIME_PREFIX + std::string(j) + JSONFILE_POSTFIX)
#define MAKE_FEEDBAK_BIN_FILE(p,j) (AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(p))) + FEEDBACK_BIN_PREFIX + std::string(j) + BINFILE_POSTFIX)
#define MAKE_TIMESUM_BIN_FILE(p,j) (AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(p))) + TIME_BIN_PREFIX + std::string(j) + BINFILE_POSTFIX)
#define MAKE_FEEDBAK_JSON_FILE(p,j) (AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(p))) + FEEDBACK_BIN_PREFIX + std::string(j) + JSONFILE_POSTFIX)
#define MAKE_TIMESUM_JSON_FILE(p,j) (AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(p))) + TIME_BIN_PREFIX + std::string(j) + JSONFILE_POSTFIX)
#define MAKE_TASK_BIN_FILE(p,j) (AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(p))) + TASK_DEF_BIN_PREFIX + std::string(j) + BINFILE_POSTFIX)
#define MAKE_TASK_JSON_FILE(p,j) (AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(p))) + TASK_DEF_BIN_PREFIX + std::string(j) + JSONFILE_POSTFIX)
		
		
		
        #define PREIMG_W 160
#define BIGSCENECOOR 10000 
		#define PREIMG_H 106
		#define PREVIEWIMGRES 640
        #define PREIMG_PATH "previews"	
		#define BASESRS "EPSG:4978"
#define GEO84SRS "EPSG:4326"
#define LOCALSRS "Local:0" 
#define LOCALSTR "Local coordinate system" 
#define GROUPBASENAME "PhotoGroup"
#define VALIDMEASUREMENTNUM 3
#define VALIDTRIANGLENUM 2

#define STR(str) #str
#define UNDEFINEDSTR "undefined"
#define MINIMAGENUMFORAT 2
#define LONG_ERROR_PRECISION 8
#define OBJECT_ERROR_PRECISION 6
#define IMAGE_ERROR_PRECISION 6
#define MEASUREMENT_PRECISION 2
#define VALIDPREDICTGCPNUM 3
#define HIGHLEVEL "High"
#define LOWLEVEL "Low"
#define NORMALLEVEL "Normal"
#define RMSGOOD 1
#define RMSBAD 3
#define INVALIDY 10000000
#define COMPLETE_PROGRESS 100
#define RUNNINGSTYLE "QProgressBar::chunk{background-color: rgb(122, 237, 171)}"
#define FAILURESTYLE "QProgressBar::chunk{background-color: #aa0000}" 
#define CANCELSTYLE "QProgressBar::chunk{background-color: #F7BA0B}"

#define UNDEFINEDVAL -1
#define POLYGONEXT ".poly"

#define RECONSTRUCIONPREFIX "Reconstruction_"
#define PRODUCTIONPREFIX "Production_"
template<typename Scalar,int Dim>
using AABB = Eigen::AlignedBox<Scalar, Dim>;

template<typename Scalar>
using ABBox2 = Eigen::AlignedBox<Scalar, 2>;
typedef ABBox2<double> ABBox2d;
typedef ABBox2<float> ABBox2f;
template<typename Scalar>
using ABBox3 = Eigen::AlignedBox<Scalar, 3>;
typedef ABBox3<double> ABBox3d;
typedef ABBox3<float> ABBox3f;

#define EMPTY_TILE_POINT_THRESHOLD 50

template<typename Scalar, int Dim>
using PointEigen = Eigen::Matrix<Scalar, Dim,1>;

template<typename Scalar, int Dim>
using VectorEigen = Eigen::Matrix<Scalar, Dim, 1>;

template <typename TYPE, int DIMS>
bool IsBoundingBoxValid(const AABB<TYPE, DIMS>& box)
{
	auto dig = box.diagonal();
	if (dig.x() < TYPE(0) || dig.y() < TYPE(0) || dig.z() < TYPE(0))
	{
		return false;
	}
	if (dig.x() <= TYPE(1e-6) &&dig.y() <= TYPE(1e-6) &&dig.z() <= TYPE(1e-6))
	{
		return false;
	}
	return  true ;
}

template <typename TYPE, int DIMS>
void MakeBoundingBoxValid(AABB<TYPE, DIMS>& box)
{
	using BBox = AABB<TYPE, DIMS>;
	
	
	if (!IsBoundingBoxValid(box))
	{
		BBox ret;
		ret.extend(box.min());
		ret.extend(box.max());	
		box = ret;
	}
	
}

template <typename TYPE, int DIMS>
void BoundingBoxToInt(AABB<TYPE, DIMS>& box)
{
	using BBoxInt = AABB<int, DIMS>;
	
	BBoxInt ret;
	
	
	for (int i = 0; i < DIMS; i++)
	{
		ret.min()[i] = box.min()[i] ;
		ret.max()[i] = box.max()[i] + (TYPE)1;
	}
	
	box = ret.cast<TYPE>();
	
}


template <typename TYPE, int DIMS>
void BoundingBoxToPresicion(AABB<TYPE, DIMS>& box, int precision)
{
	if (precision <= (0))
		return;
	using BBoxInt = AABB<int, DIMS>;
	
	AABB<TYPE, DIMS> ret;
	

	for (int i = 0; i < DIMS; i++)
	{
		double x = int(box.min()[i] * precision) / (double)precision;
		double y = int(box.max()[i] * precision+1) / (double)precision;
		ret.min()[i] = TYPE(x);
		ret.max()[i] = TYPE(y);
	}
	box = ret;
	

}

template <typename TYPE, int DIMS>
AABB<TYPE, DIMS> ExtendBoundingBox(const AABB<TYPE, DIMS>& in,
	const VectorEigen<TYPE, DIMS>& extension)
{
	using BBox = AABB<TYPE, DIMS>;
	using Pt = PointEigen<TYPE, DIMS>;
	using Vec = VectorEigen<TYPE, DIMS>;
	BBox ret = in;
	ret.extend(in.max() + extension);
	ret.extend(in.min() - extension);
	return ret;
}

template <typename TYPE, int DIMS>
AABB<TYPE, DIMS> PointsToBox(const  std::vector<VectorEigen<TYPE, DIMS> > & points)
{
	using BBox = AABB<TYPE, DIMS>;
	using Pt = PointEigen<TYPE, DIMS>;
	using Vec = VectorEigen<TYPE, DIMS>;
	BBox box;
	for (auto& iter1 : points)
	{
		
		{
			box.extend(iter1);
		}
	}
	return box;

}

template <typename TYPE, int DIMS>
AABB<TYPE, DIMS> ExtendBoundingBoxByRatio(const AABB<TYPE, DIMS>& in, TYPE ratio)
{
	using BBox = AABB<TYPE, DIMS>;
	using Pt = PointEigen<TYPE, DIMS>;
	using Vec = VectorEigen<TYPE, DIMS>;
	Vec delta;
		for (int i = 0; i < DIMS; i++)
		{
			delta[i] = (in.max()[i] - in.min()[i])*ratio;
		
		}
	
	
	return ExtendBoundingBox(in, delta);
}


template <typename TYPE, int DIMS>
AABB<TYPE, DIMS> ExtendBoundingBox(const AABB<TYPE, DIMS>& in, TYPE d)
{
	using BBox = AABB<TYPE, DIMS>;
	using Pt = PointEigen<TYPE,DIMS>;
	using Vec = VectorEigen<TYPE, DIMS>;
	Vec delta;
	delta.fill(d);
	return ExtendBoundingBox(in,delta);
}

enum pose_status_e
{
	POSE_ST_UNKNOWN,
	POSE_ST_COMPLETED,
};
		
		typedef uint32_t group_t;

		typedef uint32_t block_t;
		typedef uint32_t reconstruction_t;
		typedef uint32_t production_t;

		typedef uint32_t srsid_t;
		
		typedef uint32_t camera_t;

		
		typedef uint32_t image_t;

		
		typedef uint64_t image_pair_t;

		
		typedef uint32_t point2D_t;

		
		
		
		typedef uint64_t point3D_t;

		typedef uint64_t constraint_t;
		const constraint_t kInvalidMerasureId = 0;

		
		const camera_t kInvalidCameraId = std::numeric_limits<camera_t>::max();
		const image_t kInvalidImageId = std::numeric_limits<image_t>::max();
		const group_t kInvalidGroupId = std::numeric_limits<group_t>::max();
		const block_t kInvalidBlockId = 0;
		const block_t kInvalidReconstructionId = 0;
		const block_t kInvalidProductionId = 0;
		const srsid_t kInvalidSrsId = std::numeric_limits<srsid_t>::max();
		const uint16_t kInvalideNum = 0;

		const image_pair_t kInvalidImagePairId =
			std::numeric_limits<image_pair_t>::max();
		const point2D_t kInvalidPoint2DIdx = std::numeric_limits<point2D_t>::max();
		const point3D_t kInvalidPoint3DId = std::numeric_limits<point3D_t>::max();
		static const double kNaN = std::numeric_limits<double>::quiet_NaN();
		const double kInvalidError = -1.0;

		enum jobtype_e
		{
			JOB_AT = 0,
		    JOB_TILE = 1,
			JOB_BATCH =2,
			JOB_UNKNOWN
		};

		
		
		enum pair_selection_mode_e
		{
			PAIR_NORMAL = 0, 
			PAIR_SEQUENCE = 1,
			PAIR_LOOP = 2  
		};
		enum sfm_mode_e
		{
			SFM_GLOBAL = 0,
			SFM_INCREAMENTAL = 1
		};
		typedef enum _job_status_e
		{
			
			STATUS_PENDDING = 0,
			STATUS_RUNNING = 1,  
			STATUS_COMPLETE = 2,
			STATUS_CANCLE = 3,	
			STATUS_FAILURE = 4,
			STATUS_NEW = 5,
			STATUS_UNKNOWN = 6,
			

		}job_status_e, jobsta_e;

		
		enum blk_status_e
		{

			BLK_PENDDING = 0,
			BLK_RUNNING = 1,  
			BLK_COMPLETE = 2,
			BLK_CANCLE = 3,
			BLK_FAILURE = 4,
			BLK_NEW = 5,
			BLK_EMPTY = 6,
			BLK_UNKNOWN,


		};
		
			static std::map<job_status_e, std::string> blk_status_str =
			{
				{STATUS_PENDDING ,JOBPENDINGSTR},
			{STATUS_RUNNING, JOBRUNNINGSTR},
			{STATUS_COMPLETE, JOBCOMPLETEDSTR},
			{STATUS_CANCLE, JOBCANCELLEDSTR},
			{STATUS_FAILURE, JOBFAILEDSTR},
			{STATUS_NEW, "Block"},
			{STATUS_UNKNOWN, "Status Unknown"},
			};

			static std::map<job_status_e, std::string> blk_status_str_chinese =
			{
				{STATUS_PENDDING ,"等待"},
			{STATUS_RUNNING, "运行"},
			{STATUS_COMPLETE, "完成"},
			{STATUS_CANCLE, "取消"},
			{STATUS_FAILURE, "失败"},
			{STATUS_NEW, "新块"},
			{STATUS_UNKNOWN, "未知"},
			};
			
		
			typedef enum _sfm_align_mode_e
			{
				ALIGN_ARBITRARY = 1,
				ALIGN_WITHGCP = 2,
				ALIGN_WITHPOS = 4,
				ALIGN_WITHGCP_ARBITRARY= ALIGN_ARBITRARY | ALIGN_WITHGCP,
			ALIGN_WITHGCP_POS = ALIGN_WITHGCP | ALIGN_WITHPOS,
			}alignmode_e, sfm_align_mode_e;



		typedef enum _gcp_type_e
		{
			GCP_UNKNOWN = 0,						
			GCP_CONTROL_H = 1,					 
			GCP_CONTROL_V = 2,							
			GCP_CONTROL_HV = 3,							
			GCP_CHECK_H = 4,					
			GCP_CHECK_V =5,							
			GCP_CHECK_HV =6,							
		}gcp_type_e, gpt_e;


		typedef enum _point_type_e
		{
			PT_NONE = 0,							
			PT_CONTROL = 1,						
			PT_USER = 2

		}point_type_e, ptt_e;


		typedef enum  _coord_system_type_e
		{
			LOCAL_ENU = 0,
			LOCAL = 1,
			GEOGRAPHIC = 2,
			PROJECTION = 3,
			GEOCENTRIC = 4,
			Unsupported = 5
		}coord_system_type_e;

		
		typedef enum _rotation_format_e
		{
			ROTFORMAT_R =0,
			ROTFORMAT_OMK = 1,
			ROTFORMAT_YPR = 2,
		}rot_format_e;


		typedef struct _srs_s
		{
			_srs_s()
			{
				ID = kInvalidSrsId;
				name = LOCALSRS;
				definition = LOCALSRS;
				type = coord_system_type_e::LOCAL;
			}
			bool isValid() 
			{
				return type != coord_system_type_e::Unsupported;};
			srsid_t ID;			
			std::string name;		
			std::string definition;	
			
			coord_system_type_e type;

			void setInvalid()
			{
				type = coord_system_type_e::Unsupported;
			}

			void setLocalENU()
			{
				type = coord_system_type_e::LOCAL_ENU;
			}

			void ParseJson(rapidjson::Value& jstr)
			{

				if (jstr.HasMember("name"))
				{
					
					name = jstr["name"].GetString();
				}

				if (jstr.HasMember("type"))
				{
					type = coord_system_type_e(jstr["type"].GetInt());
					
				}

				if (type == coord_system_type_e::LOCAL_ENU)
				{
					auto originpoint = jstr["origin_point"].GetArray();
					double lat = originpoint[0].GetDouble();
					double lon = originpoint[1].GetDouble();
					double alt = originpoint[2].GetDouble();
					definition = "ENU:" + std::to_string(lat) + "," + std::to_string(lon);
				}
				else if (type == GEOGRAPHIC || type == PROJECTION || type == GEOCENTRIC)
				{
					if (jstr.HasMember("epsg_code"))
					{
						int code = jstr["epsg_code"].GetInt();
						OGRSpatialReference sr;
						if (OGRERR_NONE == sr.importFromEPSG(code))
						{
							definition = "EPSG:" + std::to_string(code);
						}
					}
					else if (jstr.HasMember("wkt"))
					{
						auto wkt = jstr["wkt"].GetString();
						OGRSpatialReference sr;
						if (OGRERR_NONE == sr.importFromWkt(wkt))
						{
							std::string codestr(sr.GetAuthorityCode(NULL));
							definition = "EPSG:" + codestr;
						}
					}
				}

			}

			void CreateJson(rapidjson::Value& coordinate, rapidjson::Document& doc) 
			{
				rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
				coordinate.AddMember("type", rapidjson::Value(type), allocator);
			
				if (type == coord_system_type_e::LOCAL_ENU)
				{
					rapidjson::Value origin_point(rapidjson::kArrayType);
					std::string latlon_tmp = AI3D::CORE::String::StringSplit(definition, ":")[1];
					std::string lat = AI3D::CORE::String::StringSplit(latlon_tmp, ",")[0];
					std::string lon = AI3D::CORE::String::StringSplit(latlon_tmp, ",")[1];
					origin_point.PushBack(std::atof(lat.c_str()), allocator);
					origin_point.PushBack(std::atof(lon.c_str()), allocator);
					origin_point.PushBack(0, allocator);
					coordinate.AddMember("origin_point", origin_point, allocator);

				}
				else if (type == coord_system_type_e::LOCAL)
				{

				}
				else
				{
					auto definitionstrs = AI3D::CORE::String::StringSplit(definition, ":");
					std::string codeflag = definitionstrs[0];
					std::string codestr = definitionstrs[1];
					AI3D::CORE::String::StringToLower(&codeflag);
					if (codeflag == "epsg")
					{
					
						coordinate.AddMember("epsg_code", std::atoi(codestr.c_str()), allocator);
					}
					else
					{
						OGRSpatialReference sr;
						if (OGRERR_NONE == sr.importFromWkt(definition.c_str()))
						{

							std::string codestr(sr.GetAuthorityCode(NULL));
							
							coordinate.AddMember("epsg_code", std::atoi(codestr.c_str()), allocator);

						}
						
					}

				}
			}


		}srs_s;

		

		typedef enum _tiling_mode_e
		{
			TILE_NONE = 0,
			TILE_PALNAR_GRID = 1,
			TILE_VOL_GRID = 2,
			TILE_ADAPTIVE = 3,

		}tiling_mode_e;

		typedef struct _boundingbox_pt_s
		{
			_boundingbox_pt_s(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax)
			{
				xmin_ = xmin;
				xmax_ = xmax;
				ymin_ = ymin;
				ymax_ = ymax;
				zmin_ = zmin;
				zmax_ = zmax;
			};

			_boundingbox_pt_s(const ABBox3d& box)
			{
				xmin_ = box.min().x();
				xmax_ = box.max().x();
				ymin_ = box.min().y();
				ymax_ = box.max().y();
				zmin_ = box.min().z();
				zmax_ = box.max().z();
			}
			_boundingbox_pt_s() {};
			double xmin_ = DBL_MAX;
			double xmax_ = -DBL_MAX;
			double ymin_ = DBL_MAX;
			double ymax_ = -DBL_MAX;
			double zmin_ = DBL_MAX;
			double zmax_ = -DBL_MAX;
			
			ABBox3d toABBox3d()
			{
				ABBox3d ab;
				ab.min() = Eigen::Vector3d{ xmin_,ymin_,zmin_ };
				ab.max() = Eigen::Vector3d{ xmax_,ymax_,zmax_ };
				return ab;
			}


			bool isValid() { return xmin_ != DBL_MAX; };

			bool isinside(double x, double y, double z) 
			{
				if (x >= xmin_ && x <= xmax_ && y >= ymin_ && y <= ymax_ && z >= zmin_ && z <= zmax_)
				{
					return true;
				}
				return false;
			};

			void CreateJson(rapidjson::Value& jstr, rapidjson::Document& doc) 
			{
				rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
				rapidjson::Value item(rapidjson::kArrayType);
				
				item.PushBack(xmin_, allocator);
				item.PushBack(ymin_, allocator);
				item.PushBack(zmin_, allocator);
				jstr.AddMember("min", item, allocator);
				
				rapidjson::Value item2(rapidjson::kArrayType);
				item2.PushBack(xmax_, allocator);
				item2.PushBack(ymax_, allocator);
				item2.PushBack(zmax_, allocator);
				jstr.AddMember("max", item2, allocator);
			}
			void ParseJson(rapidjson::Value& jstr) 
			{
				if (jstr.HasMember("min"))
				{
					xmin_ = (jstr["min"][0].GetDouble());
					ymin_ = (jstr["min"][1].GetDouble());
					zmin_ = (jstr["min"][2].GetDouble());
				}
				if (jstr.HasMember("max"))
				{
					xmax_ = (jstr["max"][0].GetDouble());
					ymax_ = (jstr["max"][1].GetDouble());
					zmax_ = (jstr["max"][2].GetDouble());
				}
			}

		} bbox_s;


		struct tiling_param_s
		{
			struct regular_tiling_param_s
			{
				Eigen::Vector3d automatic_origin_;
				Eigen::Vector3d custom_origin_;
				float tilesize_= -1.;
				void CreateJson(rapidjson::Value& jstr, rapidjson::Document& doc) 
				{
					rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
					rapidjson::Value ptjson(rapidjson::kArrayType);
					ptjson.PushBack(automatic_origin_.x(), allocator);
					ptjson.PushBack(automatic_origin_.y(), allocator);
					ptjson.PushBack(automatic_origin_.z(), allocator);
					jstr.AddMember("automatic_origin", ptjson, allocator);
					rapidjson::Value pt2json(rapidjson::kArrayType);
					
					pt2json.PushBack(custom_origin_.x(), allocator);
					pt2json.PushBack(custom_origin_.y(), allocator);
					pt2json.PushBack(custom_origin_.z(), allocator);
					jstr.AddMember("custom_origin", pt2json, allocator);
					jstr.AddMember("tilesize", rapidjson::Value(tilesize_), allocator);

				}
				void ParseJson(const rapidjson::Value& jstr)
				{
					if (jstr.HasMember("tilesize"))
					{
						tilesize_ = jstr["tilesize"].GetFloat();
					}
					if (jstr.HasMember("automatic_origin"))
					{
						automatic_origin_.x() = jstr["automatic_origin"].GetArray()[0].GetDouble();
						automatic_origin_.y() = jstr["automatic_origin"].GetArray()[1].GetDouble();
						automatic_origin_.z() = jstr["automatic_origin"].GetArray()[2].GetDouble();
					}
					if (jstr.HasMember("custom_origin"))
					{
						custom_origin_.x() = jstr["custom_origin"].GetArray()[0].GetDouble();
						custom_origin_.y() = jstr["custom_origin"].GetArray()[1].GetDouble();
						custom_origin_.z() = jstr["custom_origin"].GetArray()[2].GetDouble();
					}

				}
			};
			struct adaptive_tiling_param_s 
			{
				float target_ram_used_ = 16.;
				void CreateJson(rapidjson::Value& jstr, rapidjson::Document& doc) 
				{
					rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
					rapidjson::Value ptjson(rapidjson::kArrayType);
					
					jstr.AddMember("target_ram_used", rapidjson::Value(target_ram_used_), allocator);

				}
				void ParseJson(const rapidjson::Value& jstr) 
				{
					

					target_ram_used_ = jstr["target_ram_used"].GetFloat();

				}
			};

			tiling_mode_e mode_;
			regular_tiling_param_s regular_params_;
			adaptive_tiling_param_s adaptive_params_;
		
			float expected_max_ram_used_;
			
			tiling_param_s(tiling_mode_e mode)
			{
				mode_ = mode;
			}
			tiling_param_s() {};

			void CreateJson(rapidjson::Value& jstr, rapidjson::Document& doc) 
			{
				rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

				jstr.AddMember("tiling_mode", rapidjson::Value(int(mode_)), allocator);

				jstr.AddMember("expected_max_ram_used", rapidjson::Value((expected_max_ram_used_)), allocator);
				
				rapidjson::Value tilejson(rapidjson::kObjectType);
				if (mode_ == tiling_mode_e::TILE_NONE)
				{

				}
				else if(mode_ == tiling_mode_e::TILE_PALNAR_GRID )
				{
					regular_params_.CreateJson(tilejson,doc);
				}
				else if (mode_ == tiling_mode_e::TILE_VOL_GRID)
				{
					regular_params_.CreateJson(tilejson, doc);
				}
				else if (mode_ == tiling_mode_e::TILE_ADAPTIVE)
				{
					adaptive_params_.CreateJson(tilejson, doc);
				}
				if (!tilejson.Empty()) {
					jstr.AddMember("tiling_param", tilejson, allocator);
				}
				
			}
			void ParseJson(const rapidjson::Value& jstr)
			{
				if (jstr.HasMember("tiling_mode"))
				{
					mode_ = tiling_mode_e(jstr["tiling_mode"].GetInt());
					
				}
				if (jstr.HasMember("expected_max_ram_used"))
				{
					expected_max_ram_used_ = (jstr["expected_max_ram_used"].GetFloat());
				}
				if (jstr.HasMember("tiling_param"))
				{
					if (mode_ == tiling_mode_e::TILE_NONE)
					{

					}
					else if (mode_ == tiling_mode_e::TILE_PALNAR_GRID)
					{
						regular_params_.ParseJson(jstr["tiling_param"]);

					}
					else if (mode_ == tiling_mode_e::TILE_VOL_GRID)
					{
						regular_params_.ParseJson(jstr["tiling_param"]);
					}
					else if (mode_ == tiling_mode_e::TILE_ADAPTIVE)
					{
						adaptive_params_.ParseJson(jstr["tiling_param"]);
					}
					
					
				}
			}
		};
		
		
		
		
		

	

		
		enum class retouching_status_e
		{
			RETOUCHING_NONE,
			RETOUCHING_GEOMETRY,
			RETOUCHING_TEXTURE,

		};


		typedef enum _fixed_status_e
		{
			EOE_FREE = 0,						
			EOE_FIXED = 1,						
			
		}fix_status_e, fix_e;
		typedef struct _pose_metadata_s
		{
			_pose_metadata_s() {};
			_pose_metadata_s(
				const Eigen::Matrix3d r,
				const Eigen::Vector3d c)
				: rotation(r), center(c)
			{

			};

			Eigen::Vector3d center = { -DBL_MAX,-DBL_MAX,-DBL_MAX };;
			Eigen::Matrix3d   rotation = Eigen::Matrix3d::Zero();


			template<typename T>
			inline typename T::PlainObject operator() (const T& p) const
			{
				return rotation * (p.colwise() - center);
			}
			
			inline typename Eigen::Vector3d::PlainObject operator() (const Eigen::Vector3d& p) const
			{
				return rotation * (p - center);
			}

			_pose_metadata_s inverse() const
			{
				return { rotation.transpose(),  -(rotation * center) };
			}

		}pose_metadata_s, posemetadata_s;


		typedef enum _camera_param_e
		{
			SENSOR_SIZE = 0,
			FOCAL = 1,
			FOCAL_IN35MM = 2,
			PPX = 3,
			PPY = 4,
			K1 = 5,
			K2 = 6,
			K3 = 7,
			P1 = 8,
			P2 = 9,
			PIXEL_SIZE,

		}cam_para_e;
		typedef struct _pose_data_s
		{
			_pose_data_s() {};
			std::string name = "";
			posemetadata_s metadata_;
			
			
		}pose_data_s, pose_s;

	

		
		typedef enum CameraModelType_e
		{
			Perspective = 0,
			Fisheye = 1
		}cameramodeltype_e;

		typedef enum SaveType_e
		{
			PROJECT_SAVED = 0,
			XML_SAVED_WithOutTiepoints = 1,
			XML_SAVED = 2
		}savetype_e;


		struct sceneROI_s
		{
			std::vector<Eigen::Vector2d> boundary_;
			double min_z_ = 0;
			double max_z_ = 0;
			bool bempty_()
			{
				return boundary_.size() < 3;
			}
			void CreateJson(rapidjson::Value& json, rapidjson::Document& doc)
			{
				rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
				doc.AddMember("max_z", max_z_, allocator);
				doc.AddMember("min_z", min_z_, allocator);

				rapidjson::Value boundaryjson(rapidjson::kArrayType);
				for (auto& iter : boundary_)
				{
					boundaryjson.PushBack(iter.x(), allocator);
					boundaryjson.PushBack(iter.y(), allocator);
				}
				doc.AddMember("boundary", boundaryjson, allocator);
			};

			sceneROI_s() = default;
			sceneROI_s(const ABBox3f& bbox)
			{
				boundary_.resize(4);
				boundary_[0] = bbox.corner(ABBox3f::BottomLeftFloor).topRows(2).cast<double>();
				boundary_[1] = bbox.corner(ABBox3f::BottomLeftFloor).topRows(2).cast<double>();
				boundary_[2] = bbox.corner(ABBox3f::TopRightFloor).topRows(2).cast<double>();
				boundary_[3] = bbox.corner(ABBox3f::TopLeftFloor).topRows(2).cast<double>();

				min_z_ = bbox.corner(ABBox3f::BottomLeftFloor)[2];
				max_z_ = bbox.corner(ABBox3f::BottomLeftFloor)[2];

			}



		};




		struct rect_s
		{
			int x_ = std::numeric_limits<int>::max() / 2;
			int y_ = std::numeric_limits<int>::max() / 2;
			int width_ = std::numeric_limits<int>::min();
			int height_ = std::numeric_limits<int>::min();
			rect_s() = default;
			rect_s(int x, int y, int width, int height) :
				x_(x), y_(y), width_(width), height_(height) {};
			int right() const { return x_ + width_; };
			int bottom() const { return y_ + height_; };

			void extend(int length)
			{
				x_ -= length;
				y_ -= length;
				width_ += 2 * length;
				height_ += 2 * length;
			};
			
			void extend(int px, int py)
			{
				const int new_x = std::min(x_, px);
				const int new_y = std::min(y_, py);
				const int new_right = std::max(px + 1, right());
				const int new_bottom = std::max(py + 1, bottom());
				x_ = new_x;
				y_ = new_y;
				width_ = new_right - x_;
				height_ = new_bottom - y_;
			};

			void extend(const rect_s& other)
			{
				const int new_x = std::min(x_, other.x_);
				const int new_y = std::min(y_, other.y_);
				const int new_right = std::max(right(), other.right());
				const int new_bottom = std::max(bottom(), other.bottom());
				x_ = new_x;
				y_ = new_y;
				width_ = new_right - x_;
				height_ = new_bottom - y_;
			};
			void clamp(const rect_s& other)
			{
				const int new_x = std::max(x_, other.x_);
				const int new_y = std::max(y_, other.y_);
				const int new_right = std::min(right(), other.right());
				const int new_bottom = std::min(bottom(), other.bottom());
				x_ = new_x;
				y_ = new_y;
				width_ = new_right - x_;
				height_ = new_bottom - y_;
			};
			bool isempty() const{ return width_ <= 0 || height_ <= 0; };
			size_t area() const { return size_t(width_ * height_); };
		};

#endif  
