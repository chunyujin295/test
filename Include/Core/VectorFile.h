#ifndef _AI3D_CORE_VECTORFILE_H_
#define _AI3D_CORE_VECTORFILE_H_
#include <Eigen/Core>
#include <ogrsf_frmts.h>
#include <ogr_geometry.h>
#include <Constants.h>


std::vector<std::vector<Eigen::Vector3d>>  AI3D_API ReadPolygonsFromVecFile(const std::string& vec_files);
bool AI3D_API SavePolygonsToVecFile(std::vector<std::vector<Eigen::Vector3d>>& polygons  ,const std::string& vec_files);

#endif