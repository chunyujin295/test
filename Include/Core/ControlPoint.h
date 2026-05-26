
#ifndef _AI3D_CONTROLPOINT_H_
#define _AI3D_CONTROLPOINT_H_

#include <fstream>
#include <Constants.h>
#include <unordered_map>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#include "Core/Types.h"
#include "Core/Point3d.h"
#include "Core/Image.h"
#include "Core/Rapidjson.h"

namespace AI3D
{
	namespace CORE
	{
		
		

			


		

		class ATData;
		class AI3D_API ControlPoint
		{

		public:

			EIGEN_MAKE_ALIGNED_OPERATOR_NEW

			ControlPoint() ;
			
			

		public:
			
			void SetName(std::string name);
			std::string GetName()const;
			std::string GetNameMutual();

			
			void SetId(point3D_t id);
			point3D_t GetId()const ;
			point3D_t GetIdMutual() ;

			
			void SetGivenXYZ(Eigen::Vector3d xyz);
			Eigen::Vector3d& GetGivenXYZMutual();
			const Eigen::Vector3d GetGivenXYZ()const;

			
			void SetEstimatedXYZ(Eigen::Vector3d& xyz);
			Eigen::Vector3d& GetEstimatedXYZMutual();
			const Eigen::Vector3d  GetEstimatedXYZ() const;

			
			void SetType(gpt_e type);
			gpt_e GetType() const;
			gpt_e GetTypeMutual() ;

			
			void SetWeight(Eigen::Vector2d weight);
			Eigen::Vector2d GetWeightMutual();
			Eigen::Vector2d GetWeight()const;
			void SetSrs(srs_s srs);
			const srs_s GetSrs() const;
			srs_s GetSrsMutual() ;
			
			void SetObjectPoint(const Point3D& objectPoint);
			Point3D& GetObjectPointMutual();
			const Point3D GetObjectPoint() const;
			
			void Set3DError(double error_3d);
			const double Get3DError() const;
			
			void SetXY3DError(double error);
			const double GetXY3DError() const;
			void Calc3DError();
			
			
			void SetZ3DError(double error);
			const double GetZ3DError() const;

			const double GetX3DError() const;
			void SetX3DError(double error);
			const double GetY3DError() const;
			void SetY3DError(double error);

			bool Has3DError();
			bool HasGivenXYZ() const;
			bool HasEstimatedXYZ();
		protected:
			std::string name_;							
			point3D_t id_;						
			
			gpt_e type_;								
			Eigen::Vector3d xyz_{-DBL_MAX,-DBL_MAX,-DBL_MAX };						
			Eigen::Vector3d estimated_xyz_{ -DBL_MAX,-DBL_MAX,-DBL_MAX };						
			Eigen::Vector2d weight_;							
			srs_s origin_srs_;
			
			
			Point3D objectpoint_;
			double error_3d_;    
			double error_3d_xy_;
			double error_3d_z_;
			double error_3d_x_, error_3d_y_;;
			

		};


		class AI3D_API ControlPoints
		{
		public:
			point3D_t GetGCPCount();
			point3D_t GetCheckPointCount();
			point3D_t GetValidGCPPointCount();

			point3D_t GetFullControlPointCount();
			point3D_t GetXYControlPointCount();
			point3D_t GetZControlPointCount();
			
			bool TransformEstimatedXYZToGivenXYZSrs(std::string src_srs);
			
			bool TransformPointsToTheSrsOfOnepoint();

			bool TransformPoints(const std::string dst_crs);
			std::vector<point3D_t> GetControlPointIds() const;
			void ADDPoint(ControlPoint point);
			void SetPoints(const EIGEN_STL_UMAP(point3D_t, ControlPoint)& gcps);
			void DeletePoint(point3D_t id);
			ControlPoint& GetPoint(point3D_t id);
			const EIGEN_STL_UMAP(point3D_t, ControlPoint)& GetPoints() const;
			EIGEN_STL_UMAP(point3D_t, ControlPoint)& GetPointsMutual();

			std::string GetSRS();
			void SetSRS(std::string srs);
			const Eigen::Vector3d& GetPositionOffset() const;
			Eigen::Vector3d& GetPositionOffsetMutual();
			bool ExistsPoint(const point3D_t point_id) const;
			

			
			bool TransformPointsToBaseCoordinate(const std::string dst_crs);

		

			int LoadXML(const std::string& gcpfilepath);
			int SaveXML(const std::string& gcpfilepath,const std::map<image_t, std::string>& images) const;
			bool LoadText(const std::string &gcpfilepath);
			bool SaveText(const std::string& gcpfilepath) const;
			
			
			
			bool SaveTextFor3DView(const std::string& gcpfilepath) const;
			int LoadJson(const std::string& path);
			void SaveJson(const std::string& outpath, const srs_s& srs) const;
			
			

		private:
			std::string srs_;
			EIGEN_STL_UMAP(point3D_t, ControlPoint) controlpoints_;
			Eigen::Vector3d position_offset_ = { 0.0,0.0,0.0 };
		};

	}
}
#endif 
