#ifndef _AI3D_COORDINATESYSTEM_H_
#define _AI3D_COORDINATESYSTEM_H_

#include <Core/Types.h>
#include <ogr_spatialref.h>

#include <proj.h>

#include "Eigen/Core"
#include "GeographicLib/LocalCartesian.hpp"

#include <Constants.h>
#include <map>
namespace AI3D
{
    namespace CORE
    {


        class AI3D_API CoordinateDescriptor
        {
        public:

          

            static int GetCGCS2000Code(const Eigen::Vector3d& lat_lon_alt)
            {
                return int(std::floor((lat_lon_alt[1] - 73.5) / 3.0) + 4534);
            }

            void SaveCoordinateMetaData(const std::string& filename, const Eigen::Vector3d& offset);

            CoordinateDescriptor()
            {
                type_ = LOCAL;
            }

    
            void InitialOrigin(const Eigen::Vector3d& origin_point);

            const Eigen::Vector3d& GetOriginPoint() const
            {
                return origin_point_;
            }

            coord_system_type_e GetType() const { return type_; }

            int GetEPSGCode() const { return epsg_code_; }

            std::string GetWKT() const { return wkt_; }

            static bool IsGeode(const coord_system_type_e type)
            {
                if (type == LOCAL || type == Unsupported)
                    return false;
                return true;
            }
           
            
            static srs_s GetSRSFromDefinition(const std::string& definition);

            
            static srs_s GetSRSFromName(const std::string& name);

			static Eigen::Vector2d GetLatLonFromENUDefinition(const std::string& enu_definition);
        private:

            Eigen::Vector3d origin_point_ = { 0.0, 0.0, 0.0 };
            coord_system_type_e type_;
            int epsg_code_ = -1;
            std::string wkt_;;
        };

        class AI3D_API CoordinateTransformer
        {
        public:

            CoordinateTransformer(const std::shared_ptr<CoordinateDescriptor>& src_descriptor,
                const std::shared_ptr<CoordinateDescriptor>& dst_descriptor);


            static bool TransformBBox(ABBox3d& box, std::string src_crs, std::string dst_crs);

            static bool TransformByEnu(int numPoints, double* src_x, double* src_y, double* src_z, std::string src_crs, std::string dst_crs);

            static bool TransformByEnu(double src_x, double src_y, double src_z,
                double& dst_x, double& dst_y, double& dst_z,
                std::string src_crs, std::string dst_crs);
            static bool Transform(int numPoints, double* src_x, double* src_y, double* src_z, std::string src_crs, std::string dst_crs);

            static bool TransformPoints(std::vector < std::vector<Eigen::Vector3d>>& points, std::string src_crs, std::string dst_crs);
            static bool TransformByEpsgCode(double& src_x, double& src_y,
                double& src_z, double& dst_x, double& dst_y, double& dst_z, std::string src_crs, std::string dst_crs);
            static bool TransformByEpsgCode(int numPoints, double* src_x, double* src_y,double* src_z , std::string src_crs, std::string dst_crs);
            static bool Transform(std::vector<posemetadata_s>& poses,
                srs_s src_crs, srs_s dst_crs);
            static bool TransformRotation(int numPoints,std::vector<Eigen::Vector3d>& poses, std::vector<Eigen::Matrix3d>& rotations,
                srs_s src_crs, srs_s dst_crs, int mode = 0);
            static Eigen::Matrix3x4d TransformProjectMatrix(const Eigen::Matrix3x4d& projectMatrix,std::string src_def,std::string dst_def);
            static bool TransformRotation(Eigen::Vector3d pose, Eigen::Matrix3d& rotation,
                srs_s src_crs, srs_s dst_crs);
                static bool Transform(double src_x, double src_y, double src_z,
               double& dst_x, double& dst_y, double& dst_z,
               std::string src_src, std::string dst_src);
                
                
                static  bool IsSame(std::string src_def,std::string dst_def);

			static std::map<std::string, std::vector<srs_s>> CSG_coordinateSystem_Global();
            
           
            static std::map<srsid_t, srs_s> Get_CSG_in_srs_map();
            static std::map<srsid_t, srs_s> GetSRSDB();

        private:
            static std::map<srsid_t, srs_s> CSG_in_srs_map;
            std::shared_ptr<CoordinateDescriptor> src_descriptor_;
            std::shared_ptr<CoordinateDescriptor> dst_descriptor_;

            std::shared_ptr<GeographicLib::LocalCartesian> local_cartesian_;
            std::function<void(double, double, double, double&, double&, double&)> trans_func_;
            bool valid_transfor_;
        };

        

    }
}
#endif 
