
#include <Eigen/Core>
#include <GeographicLib/LocalCartesian.hpp>
#include "Core/String.h"
#include "Core/CoordinateSystem.h"
#include <proj.h>
#include "Core/AlgorithmBase.h"

#include <iostream>
#ifdef USE_AI3D_PROJ
#include "Core/Proj/CoordinateReferenceSystem.h"
#include "Core/Proj/ProjUtils.h"
#endif
namespace AI3D
{
	namespace CORE
	{
		bool  CoordinateTransformer::IsSame(std::string src_def, std::string dst_def)
		{
			String::StringToLower(&src_def);
			String::StringToLower(&dst_def);
			if (src_def.find("enu") != std::string::npos && dst_def.find("enu") != std::string::npos)
			{
				Eigen::Vector2d src_latlon = CoordinateDescriptor::GetLatLonFromENUDefinition(src_def);
				Eigen::Vector2d def_latlon = CoordinateDescriptor::GetLatLonFromENUDefinition(dst_def);
				return src_latlon == def_latlon;
			}
			else
			{
				
				return src_def == dst_def;
			}
		}

		Eigen::Matrix3x4d CoordinateTransformer::TransformProjectMatrix(const Eigen::Matrix3x4d& projectMatrix, std::string src_def, std::string dst_def)
		{
			Eigen::Matrix3d R_old, R_old_t,R_new;
			Eigen::Vector3d C_old,C_new;
			R_old = projectMatrix.block(0, 0, 3, 3); R_old_t = R_old.transpose();
			C_old = -R_old_t * projectMatrix.block(0, 3, 3, 1);
			double x = C_old[0]; double y = C_old[1]; double z = C_old[2];
			Transform(x,y,z,C_new[0],C_new[1],C_new[2],src_def,dst_def);
			Transform(x+R_old_t(0,0), y + R_old_t(1, 0), z + R_old_t(2, 0), R_new(0,0), R_new(1,0), R_new(2,0), src_def, dst_def);
			Transform(x + R_old_t(0, 1), y + R_old_t(1, 1), z + R_old_t(2, 1), R_new(0, 1), R_new(1, 1), R_new(1, 1), src_def, dst_def);
			Transform(x + R_old_t(0, 2), y + R_old_t(1, 2), z + R_old_t(2, 2), R_new(0, 2), R_new(1, 2), R_new(2, 2), src_def, dst_def);
			R_new.colwise() -= C_new;
			R_new.colwise().normalize();
			R_new.transposeInPlace();
			Eigen::Matrix3x4d pnew;
			pnew.block(0, 0, 3, 3) = R_new;
			pnew.block(0, 3, 3, 1) = -R_new*C_new;
			return pnew;
		}
	
		bool CoordinateTransformer::TransformRotation(int numPoints,
			std::vector<Eigen::Vector3d>& poses, std::vector<Eigen::Matrix3d>& rotations,
			srs_s src_crs, srs_s dst_crs, int mode)
		{

			if (src_crs.definition == "")
			{
				LOGI("src coor is empty.");
				return false;
			}
			if (dst_crs.definition == "")
			{
				LOGI("dst coor is empty.");
				return false;
			}
			if (src_crs.type == LOCAL)
			{
				LOGI("src coor type is local.");
				return false;
			}
			if (dst_crs.type == LOCAL)
			{
				LOGI("dst coor type is local.");
				return false;
			}


			if (IsSame(src_crs.definition, dst_crs.definition))
			{
				return true;
			}
			if (numPoints == 0)
			{
				LOGI("points num is 0.");
				return false;
			}
			if (poses.empty())
			{
				LOGI("poses num is 0.");
				return false;
			}
			if (numPoints > poses.size())
			{
				LOGI("points num is " + std::to_string(numPoints) + "poses num is " + std::to_string(poses.size()));
				return false;
			}

			if (mode == 0)
			{
				if (rotations.size() > 0)
				{
					if (numPoints > rotations.size())
					{
						LOGI("points num is " + std::to_string(numPoints) + "rots num is " + std::to_string(rotations.size()));
						return false;
					}
					if (poses.size() != rotations.size())
					{
						LOGI("poses num is " + std::to_string(poses.size()) + "rots num is " + std::to_string(rotations.size()));
						return false;
					}
					
					double m_dEpsilonX = 1;
					double m_dEpsilonY = 1;
					double m_dEpsilonZ = 1;
					std::vector<double> x, y, z;

					x.reserve(numPoints * 6);
					y.reserve(numPoints * 6);
					z.reserve(numPoints * 6);
					for (point3D_t i = 0; i < numPoints; i++)
					{
						
						Eigen::Vector3d pose = poses[i];
						Eigen::Vector3d mx(Eigen::Vector3d(pose.x() - m_dEpsilonX / 2., pose.y(), pose.z()));
						x.push_back(mx.x()); y.push_back(mx.y()); z.push_back(mx.z());
						Eigen::Vector3d px(Eigen::Vector3d(pose.x() + m_dEpsilonX / 2., pose.y(), pose.z()));
						x.push_back(px.x()); y.push_back(px.y()); z.push_back(px.z());
						Eigen::Vector3d my(Eigen::Vector3d(pose.x(), pose.y() - m_dEpsilonY / 2., pose.z()));
						x.push_back(my.x()); y.push_back(my.y()); z.push_back(my.z());
						Eigen::Vector3d py(Eigen::Vector3d(pose.x(), pose.y() + m_dEpsilonY / 2., pose.z()));
						x.push_back(py.x()); y.push_back(py.y()); z.push_back(py.z());
						Eigen::Vector3d mz(Eigen::Vector3d(pose.x(), pose.y(), pose.z() - m_dEpsilonZ / 2.));
						x.push_back(mz.x()); y.push_back(mz.y()); z.push_back(mz.z());
						Eigen::Vector3d pz(Eigen::Vector3d(pose.x(), pose.y(), pose.z() + m_dEpsilonZ / 2.));
						x.push_back(pz.x()); y.push_back(pz.y()); z.push_back(pz.z());
					}
					bool ret = false;
					ret = CoordinateTransformer::Transform(poses.size() * 6, &x[0], &y[0], &z[0], src_crs.definition, dst_crs.definition);
					if (!ret)
					{
						LOGI("transform pose failed in rotation transform.");
						return ret;
					}
					for (point3D_t i = 0; i < numPoints; i++)
					{
						
						Eigen::Vector3d mx(Eigen::Vector3d{ x[i * 6 + 0],y[i * 6 + 0] ,z[i * 6 + 0] });
						Eigen::Vector3d px(Eigen::Vector3d{ x[i * 6 + 1],y[i * 6 + 1] ,z[i * 6 + 1] });
						Eigen::Vector3d my(Eigen::Vector3d{ x[i * 6 + 2],y[i * 6 + 2] ,z[i * 6 + 2] });
						Eigen::Vector3d py(Eigen::Vector3d{ x[i * 6 + 3],y[i * 6 + 3] ,z[i * 6 + 3] });
						Eigen::Vector3d mz(Eigen::Vector3d{ x[i * 6 + 4],y[i * 6 + 4] ,z[i * 6 + 4] });
						Eigen::Vector3d pz(Eigen::Vector3d{ x[i * 6 + 5],y[i * 6 + 5] ,z[i * 6 + 5] });
						Eigen::Vector3d u = px - mx, v = py - my, w = pz - mz;
						u.normalize(); v.normalize(); w.normalize();
						Eigen::Matrix<double, 3, 3> R;
						R.col(0) = u; R.col(1) = v; R.col(2) = w;
						Eigen::Matrix<double, 3, 3> U, Vt;
						Eigen::Vector3d W;
						Eigen::JacobiSVD<Eigen::Matrix3d> SVD(R, Eigen::ComputeFullV | Eigen::ComputeFullU);
						U = SVD.matrixU();
						Vt = SVD.matrixV().transpose();
						rotations[i] = rotations[i] * (U * Vt).transpose();
					}
				}
				std::vector<double> x_pose, y_pose, z_pose;
				x_pose.reserve(numPoints);
				y_pose.reserve(numPoints);
				z_pose.reserve(numPoints);

				for (point3D_t i = 0; i < numPoints; i++)
				{
					x_pose.emplace_back(poses[i].x());
					y_pose.emplace_back(poses[i].y());
					z_pose.emplace_back(poses[i].z());

				}
				bool ret = false;
				ret = CoordinateTransformer::Transform(numPoints, &x_pose[0], &y_pose[0], &z_pose[0], src_crs.definition, dst_crs.definition);
				if (!ret)
				{
					LOGI("transform pose failed.");
					return ret;
				}
				for (point3D_t i = 0; i < numPoints; i++)
				{
					poses[i].x() = x_pose[i];
					poses[i].y() = y_pose[i];
					poses[i].z() = z_pose[i];

				}
			}
			else
			{
				for (point3D_t i = 0; i < numPoints; i++)
				{
					Eigen::Matrix3x4d pold;
					pold.block(0, 0, 3, 3) = rotations[i];
					pold.block(0, 3, 3, 1) = -rotations[i] * poses[i];
					Eigen::Matrix3x4d pnew = CoordinateTransformer::TransformProjectMatrix(pold, src_crs.definition, dst_crs.definition);
				}
			}
			return true;

		}

		bool CoordinateTransformer::TransformRotation(Eigen::Vector3d pose, Eigen::Matrix3d& rotation, 
			srs_s src_crs, srs_s dst_crs)
		{
			if (pose.x() == -DBL_MAX || pose.x() == DBL_MAX)
			{
				return false;
			}
			if ((src_crs.definition == "") || dst_crs.definition == "")
				return false;
			if (src_crs.type == LOCAL || dst_crs.type == LOCAL)
			{
				return false;
			}

			if (IsSame(src_crs.definition,dst_crs.definition))
			{
				return true;
			}
			
			Eigen::Matrix3d& rotation_matrix = rotation;
			double m_dEpsilonX = 1;
			double m_dEpsilonY = 1;
			double m_dEpsilonZ = 1;
			std::vector<double> x, y, z;

			
			Eigen::Vector3d mx(Eigen::Vector3d(pose.x() - m_dEpsilonX / 2., pose.y(), pose.z()));
			x.push_back(mx.x()); y.push_back(mx.y()); z.push_back(mx.z());
			Eigen::Vector3d px(Eigen::Vector3d(pose.x() + m_dEpsilonX / 2., pose.y(), pose.z()));
			x.push_back(px.x()); y.push_back(px.y()); z.push_back(px.z());
			Eigen::Vector3d my(Eigen::Vector3d(pose.x(), pose.y() - m_dEpsilonY / 2., pose.z()));
			x.push_back(my.x()); y.push_back(my.y()); z.push_back(my.z());
			Eigen::Vector3d py(Eigen::Vector3d(pose.x(), pose.y() + m_dEpsilonY / 2., pose.z()));
			x.push_back(py.x()); y.push_back(py.y()); z.push_back(py.z());
			Eigen::Vector3d mz(Eigen::Vector3d(pose.x(), pose.y(), pose.z() - m_dEpsilonZ / 2.));
			x.push_back(mz.x()); y.push_back(mz.y()); z.push_back(mz.z());
			Eigen::Vector3d pz(Eigen::Vector3d(pose.x(), pose.y(), pose.z() + m_dEpsilonZ / 2.));
			x.push_back(pz.x()); y.push_back(pz.y()); z.push_back(pz.z());
			CoordinateTransformer::Transform(pose.size() * 6, &x[0], &y[0], &z[0], src_crs.definition, dst_crs.definition);
			int i = 0;
			Eigen::Vector3d mx1(Eigen::Vector3d{ x[i * 6 + 0],y[i * 6 + 0] ,z[i * 6 + 0] });
			Eigen::Vector3d px1(Eigen::Vector3d{ x[i * 6 + 1],y[i * 6 + 1] ,z[i * 6 + 1] });
			Eigen::Vector3d my1(Eigen::Vector3d{ x[i * 6 + 2],y[i * 6 + 2] ,z[i * 6 + 2] });
			Eigen::Vector3d py1(Eigen::Vector3d{ x[i * 6 + 3],y[i * 6 + 3] ,z[i * 6 + 3] });
			Eigen::Vector3d mz1(Eigen::Vector3d{ x[i * 6 + 4],y[i * 6 + 4] ,z[i * 6 + 4] });
			Eigen::Vector3d pz1(Eigen::Vector3d{ x[i * 6 + 5],y[i * 6 + 5] ,z[i * 6 + 5] });
			Eigen::Vector3d u = px1 - mx1, v = py1 - my1, w = pz1 - mz1;
			u.normalize();
			v.normalize();
			w.normalize();
			Eigen::Matrix<double, 3, 3> R;
			R.col(0) = u;
			R.col(1) = v;
			R.col(2) = w;
			Eigen::Matrix<double, 3, 3> U, Vt;
			Eigen::Vector3d W;
			Eigen::JacobiSVD<Eigen::Matrix3d> SVD(R, Eigen::ComputeFullV | Eigen::ComputeFullU);
			U = SVD.matrixU();
			Vt = SVD.matrixV().transpose();
			rotation_matrix = rotation * (U * Vt).transpose();

			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			

			
			
			
			
			
			
			
			
			
			
			
			

			
			
			
			
			
			
			
			
			
			
			
			

			
			
			

			
			
			
			
			
			

			
			
			
			
			
			
			
			
			

			
			
			
			

			
			return true;
		}


		bool CoordinateTransformer::Transform(std::vector<posemetadata_s>& poses,
			srs_s src_crs, srs_s dst_crs)
		{
			

			if ((src_crs.definition == "") || dst_crs.definition == "")
				return false;
			if (IsSame(src_crs.definition, dst_crs.definition))
			{
				return true;
			}
			if (src_crs.type == LOCAL || dst_crs.type == LOCAL)
			{
				return false;
			}


#ifdef USE_OPENMP
#pragma omp parallel  for
#endif
			for (int i = 0; i < poses.size(); i++)
			{
				
				TransformRotation(poses[i].center, poses[i].rotation, src_crs, dst_crs);

			}


			std::vector<double> x, y, z;
			for (image_t i = 0; i < poses.size(); i++)
			{
				Eigen::Vector3d xyz = poses[i].center;
				x.push_back(xyz.x());
				y.push_back(xyz.y());
				z.push_back(xyz.z());
			}
			CoordinateTransformer::Transform(x.size(), &x[0], &y[0], &z[0], src_crs.definition, dst_crs.definition);
			for (image_t i = 0; i < poses.size(); i++)
			{
				poses[i].center.x() = x[i];
				poses[i].center.y() = y[i];
				poses[i].center.z() = z[i];
			}


			return true;
		}

		
		std::map<std::string, std::vector<srs_s>>  CoordinateTransformer::CSG_coordinateSystem_Global()
		{
			std::map<std::string, std::vector<srs_s>> CSG_define_srs_map;
			static srsid_t def_srs_id = 0;
			std::vector<srs_s> Default_srs;
			std::vector<srs_s> Recent_srs;
			std::vector<srs_s> More_srs;

			auto def_srs = srs_s();
			def_srs.ID = def_srs_id++;
			def_srs.name = "WGS 84 (EPSG:4326)";
			def_srs.definition = "EPSG:4326";
			def_srs.type = coord_system_type_e::GEOGRAPHIC;
			Default_srs.push_back(def_srs);
			
			for (int lon_begin = 102, epsg_begin = 4543; lon_begin <= 126; lon_begin = lon_begin + 3, epsg_begin++)
			{
				def_srs.ID = (def_srs_id++);
				def_srs.name = "CGCS2000 / 3-degree Gauss-Kruger CM " + std::to_string(lon_begin) + "E (EPSG:" + std::to_string(epsg_begin) + ")";
				def_srs.definition = "EPSG:" + std::to_string(epsg_begin);
				def_srs.type = coord_system_type_e::PROJECTION;
				Recent_srs.push_back(def_srs);
			}
			def_srs.ID = (def_srs_id++);
			def_srs.name = "ECEF - Earth - Centered, Earth - Fixed(EPSG:4978)";
			def_srs.definition = "EPSG:4978";
			def_srs.type = coord_system_type_e::GEOCENTRIC;
			More_srs.push_back(def_srs);

			def_srs.ID = (def_srs_id++);
			def_srs.name = "WGS 84 - World Geodetic System 1984 (EPSG:4326) + EGM96 geoid height (EPSG:5773)";
			def_srs.definition = "EPSG:4326+5773";
			
			def_srs.type = coord_system_type_e::GEOGRAPHIC;
			More_srs.push_back(def_srs);


			for (int lon_begin = 75, epsg_begin = 4534; lon_begin <= 135; lon_begin = lon_begin + 3, epsg_begin++) {
				def_srs.ID = (def_srs_id++);
				def_srs.name = "CGCS2000 / 3-degree Gauss-Kruger CM " + std::to_string(lon_begin) + "E (EPSG:" + std::to_string(epsg_begin) + ")";
				def_srs.definition = "EPSG:" + std::to_string(epsg_begin);
				def_srs.type = coord_system_type_e::PROJECTION;
				More_srs.push_back(def_srs);
			}
			for (int utm_begin = 38, epsg_begin = 32638; utm_begin <= 53; utm_begin++, epsg_begin++) {
				def_srs.ID = (def_srs_id++);
				def_srs.name = "WGS 84 / UTM zone " + std::to_string(utm_begin) + "N (EPSG:" + std::to_string(epsg_begin) + ")";
				def_srs.definition = "EPSG:" + std::to_string(epsg_begin);
				def_srs.type = coord_system_type_e::PROJECTION;
				More_srs.push_back(def_srs);
			}

			CSG_define_srs_map.insert(std::pair<std::string, std::vector<srs_s>>("Default", Default_srs));
			
			CSG_define_srs_map.insert(std::pair<std::string, std::vector<srs_s>>("Common", Recent_srs));
			CSG_define_srs_map.insert(std::pair<std::string, std::vector<srs_s>>("More", More_srs));

			return CSG_define_srs_map;
		}


		

		std::map<srsid_t, srs_s> CoordinateTransformer::Get_CSG_in_srs_map()
		{
			std::map<std::string, std::vector<srs_s>> CSG_define_srs_map = CSG_coordinateSystem_Global();
			std::map<srsid_t, srs_s> CSG_in_srs_map_temp;
			for (auto value_it = CSG_define_srs_map["Default"].begin(); value_it != CSG_define_srs_map["Default"].end(); value_it++) {
				CSG_in_srs_map_temp.insert(std::pair<srsid_t, srs_s>(value_it->ID, *value_it));
			}
			for (auto value_it = CSG_define_srs_map["Recent"].begin(); value_it != CSG_define_srs_map["Recent"].end(); value_it++) {
				CSG_in_srs_map_temp.insert(std::pair<srsid_t, srs_s>(value_it->ID, *value_it));
			}
			for (auto value_it = CSG_define_srs_map["More"].begin(); value_it != CSG_define_srs_map["More"].end(); value_it++) {
				CSG_in_srs_map_temp.insert(std::pair<srsid_t, srs_s>(value_it->ID, *value_it));
			}
			 return CSG_in_srs_map_temp;
		}
		
		std::map<srsid_t, srs_s> CoordinateTransformer::CSG_in_srs_map = Get_CSG_in_srs_map();
		std::map<srsid_t, srs_s> CoordinateTransformer::GetSRSDB()
		{
			return CSG_in_srs_map;
		}
		void NullTransform(double src_x, double src_y, double src_z,
			double& dst_x, double& dst_y, double& dst_z)
		{
			dst_x = src_x;
			dst_y = src_y;
			dst_z = src_z;
		}

		Eigen::Vector2d CoordinateDescriptor::GetLatLonFromENUDefinition(const std::string& enu_def)
		{
			std::string definition_temp = enu_def;
			String::StringToUpper(&definition_temp);
			size_t index_colon = definition_temp.find_first_of(":");
			size_t index_comma = definition_temp.find_first_of(",");
			std::string lat = definition_temp.substr(index_colon + 1, index_comma - index_colon - 1);
			std::string lon = definition_temp.substr(index_comma + 1);

			return Eigen::Vector2d(std::atof(lat.c_str()), std::atof(lon.c_str()));
		}
#ifdef USE_AI3D_PROJ
		
		srs_s CoordinateDescriptor::GetSRSFromDefinition(const std::string& definition)
		{
			AI3D::PROJ::CoordinateReferenceSystem crs(definition);
			srs_s srs_temp;
			srs_temp.definition = definition;
			srs_temp.name = crs.GetDescription();
			srs_temp.type = crs.GetType();
			
			return srs_temp;
		}
		srs_s CoordinateDescriptor::GetSRSFromName(const std::string& name)
		{
			
			std::string name_temp = name;
			String::StringToUpper(&name_temp);
			std::string localstr = LOCALSTR;
			String::StringToUpper(&localstr);
			std::string definition_srs ;
			srs_s srs_temp;

			if (String::StringContains(name_temp, localstr))
			{
				srs_temp.definition = LOCALSRS;
				srs_temp.name = LOCALSTR;
				srs_temp.type = coord_system_type_e::LOCAL;
			}
			else
			{
			
				if (name_temp.find("ENU") != std::string::npos)
				{
					size_t index_colon = name_temp.find_first_of(":");
					std::string istream = name_temp.substr(index_colon + 1);

					std::string lat, lon;
					auto strvec = String::StringSplit(istream, " ");
					for (std::vector<std::string>::iterator it = strvec.begin();
						it != strvec.end();)
					{
						if (*it == "")
						{
							strvec.erase(it);
						}
						else
						{
							it++;
						}
					}
				
					if (strvec.size() != 2)
					{
						LOGE(String::StringPrintf("Parse CoordinateSystem failed: %s", name));
						

												
												
												
												
												
												

						srs_temp.setInvalid();
						

						return srs_temp;
					}

					lat = strvec[0];
					lon = strvec[1];
					lat = lat.substr(0, lat.find_first_of("N"));
					lon = lon.substr(0, lon.find_first_of("E"));
					

					srs_temp.name = "Local East-North-Up (ENU); origin: " + lat + "N " + lon + "E";
					srs_temp.definition = "ENU:" + lat + "," + lon;
					srs_temp.type = coord_system_type_e::LOCAL_ENU;
				}
				else if (name_temp.find("EPSG:") != std::string::npos)
				{
					
					size_t pos = -1;
					std::string definition = "EPSG:";
					bool isfirst = true;
					while ((pos = name_temp.find("EPSG:", pos + 1)) != std::string::npos)
					{

						std::string epsg = name_temp.substr(pos + 5, name_temp.find_first_of(")", pos) - pos - 5);
						if (isfirst)
						{
							definition = definition + epsg;
							isfirst = false;
						}
						else
						{
							definition = definition + "+" + epsg;
						}
					}
					srs_temp = GetSRSFromDefinition(definition);
				}
				
			}
			return srs_temp;


			
		}

#else

		srs_s CoordinateDescriptor::GetSRSFromName(const std::string& name)
		{
			srs_s srs_temp;

			if (name == LOCALSRS)
			{
				srs_temp.definition = name;
				srs_temp.name = name;
				srs_temp.type = coord_system_type_e::LOCAL;
			}
			else
			{
				std::string name_temp = name;
				String::StringToUpper(&name_temp);
				if (name_temp.find("ENU") != std::string::npos)
				{
					size_t index_colon = name_temp.find_first_of(":");
					std::string istream = name_temp.substr(index_colon + 1);
					
					std::string lat, lon;
					auto strvec = String::StringSplit(istream, ",");
					if (strvec.size() != 2)
					{
						LOGE(String::StringPrintf("Parse CoordinateSystem failed: %s", name));

 
						
						
						
						
						
						

						srs_temp.setInvalid();
						

						return srs_temp;
					}
					
					lat = strvec[0];
					lon = strvec[1];
					lat = lat.substr(0, lat.find_first_of("N"));
					lon = lon.substr(0, lon.find_first_of("E"));
					
					
					srs_temp.name = "Local East-North-Up (ENU); origin: " + lat + "N " + lon + "E";
					srs_temp.definition = "ENU:" + lat + "," + lon;
					srs_temp.type = coord_system_type_e::LOCAL_ENU;
				}
				else if (name_temp.find("EPSG:") != std::string::npos)
				{
					
					size_t pos = -1;
					std::string definition = "EPSG:";
					bool isfirst = true;
					while ((pos = name_temp.find("EPSG:", pos + 1)) != std::string::npos)
					{

						std::string epsg = name_temp.substr(pos + 5, name_temp.find_first_of(")", pos) - pos - 5);
						if (isfirst)
						{
							definition = definition + epsg;
							isfirst = false;
						}
						else
						{
							definition = definition + "+" + epsg;
						}
					}
					srs_temp = GetSRSFromDefinition(definition);
				}
				else
				{
					
					String::StringToLower(&name_temp);
					String::StringRemoveALL(name_temp, " ");
					for (const auto& srs : CoordinateTransformer::GetSRSDB())
					{
						std::string srsname = srs.second.name;
						String::StringToLower(&srsname);
						String::StringRemoveALL(srsname, " ");
						auto pos = srsname.find(name_temp);
						if (pos != std::string::npos)
						{
							srs_temp = srs.second;
						}
					}
				}
			}
			return srs_temp;
		}
		srs_s CoordinateDescriptor::GetSRSFromDefinition(const std::string& definition)
		{
			
			srs_s srs_temp;
			if (definition == LOCALSRS)
			{
				srs_temp.definition = definition;
				srs_temp.name = definition;
				srs_temp.type = coord_system_type_e::LOCAL;
			}
			
			else
			{

				std::string definition_temp = definition;
				String::StringToUpper(&definition_temp);
				size_t index_colon = definition_temp.find_first_of(":");
				std::string coord_name = definition_temp.substr(0, index_colon);
				if (coord_name == "ENU")
				{
					Eigen::Vector2d LatLon = GetLatLonFromENUDefinition(definition);
					srs_temp.definition = definition;
					srs_temp.name = "Local East-North-Up (ENU); origin: " + std::to_string(LatLon(0)) + "N " + std::to_string(LatLon(1)) + "E";
					srs_temp.type = coord_system_type_e::LOCAL_ENU;
				}
				else
				{
					for (const auto& srs : CoordinateTransformer::GetSRSDB())
					{
						if (srs.second.definition == definition_temp)
						{
							srs_temp = srs.second;
							break;
						}
					}
				}
			}
			return srs_temp;
		}

#endif
		bool CoordinateTransformer::TransformBBox(ABBox3d& box,std::string src_crs, std::string dst_crs)
		{
			std::vector<double> x, y, z;
			
			Eigen::Vector3d  pt[8];
			pt[0] =  box.corner(ABBox3d::BottomLeftFloor) ;
			pt[1] = box.corner(ABBox3d::BottomRightFloor);
			pt[2] = box.corner(ABBox3d::TopRightFloor);
			pt[3] = box.corner(ABBox3d::TopLeftFloor);
			pt[4] = box.corner(ABBox3d::BottomLeftCeil);
			pt[5] = box.corner(ABBox3d::BottomRightCeil);
			pt[6] = box.corner(ABBox3d::TopRightCeil);
			pt[7] = box.corner(ABBox3d::TopLeftCeil);
			

			for (int i = 0; i < 8; i++)
			{
				x.push_back(pt[i].x());
				y.push_back(pt[i].y());
				z.push_back(pt[i].z());
			}

			Transform(8,&x[0],&y[0],&z[0], src_crs, dst_crs);

			box.corner(ABBox3d::BottomLeftFloor) = Eigen::Vector3d{ x[0],y[0],z[0] };

			
			 box.corner(ABBox3d::BottomRightFloor) = Eigen::Vector3d{ x[1],y[1],z[1] }; ;
			 box.corner(ABBox3d::TopRightFloor) = Eigen::Vector3d{ x[2],y[2],z[2] };;
			 box.corner(ABBox3d::TopLeftFloor) = Eigen::Vector3d{ x[3],y[3],z[3] };;
			 box.corner(ABBox3d::BottomLeftCeil) = Eigen::Vector3d{ x[4],y[4],z[4] };;
			 box.corner(ABBox3d::BottomRightCeil) = Eigen::Vector3d{ x[5],y[5],z[5] };;
			 box.corner(ABBox3d::TopRightCeil) = Eigen::Vector3d{ x[6],y[6],z[6] };;
			 box.corner(ABBox3d::TopLeftCeil)= Eigen::Vector3d{ x[7],y[7],z[7] };;
			 return true;
		}


bool CoordinateTransformer::TransformByEnu(int numPoints, 
	double* src_x, double* src_y, double* src_z, std::string src_crs, std::string dst_crs)
{
	if (numPoints == 0)
		return false;
	if (IsSame(src_crs, dst_crs))
	{		
		return true;
	}
	if (CoordinateDescriptor::GetSRSFromDefinition(src_crs).type == LOCAL
		|| CoordinateDescriptor::GetSRSFromDefinition(dst_crs).type == LOCAL)
	{
		return false;
	}

	
	std::vector<std::string> importVec;
	std::vector<std::string> exportVec;

	double latitude, longitude;
	std::string sepstr = ":,";
	importVec = String::StringSplit(src_crs, sepstr);
	exportVec = String::StringSplit(dst_crs, sepstr);
	std::string srsstr = importVec[0];
	String::StringToLower(&srsstr);
	std::string dststr = exportVec[0];
	String::StringToLower(&dststr);

	


	if ((srsstr == "epsg"))
	{
		if (dststr == "enu")
		{
			latitude = std::stold(exportVec[1]);
			longitude = std::stold(exportVec[2]);
			TransformByEpsgCode(numPoints, src_x,src_y,  src_z,  src_crs, "epsg:4326");
			for (int i=0;i<numPoints;i++)
			{
				const GeographicLib::Geocentric& earth = GeographicLib::Geocentric::WGS84();

				GeographicLib::LocalCartesian proj(latitude, longitude, 0, earth);
				double dst_x, dst_y, dst_z;
				proj.Forward(src_y[i], src_x[i], src_z[i], dst_x, dst_y, dst_z);
				src_x[i] = dst_x;
				src_y[i] = dst_y;
				src_z[i] = dst_z;

			}

		}
	}


	if (srsstr == "enu") 
	{

	

		latitude = std::stold(importVec[1]);
		longitude = std::stold(importVec[2]);
		const GeographicLib::Geocentric& earth = GeographicLib::Geocentric::WGS84();

		GeographicLib::LocalCartesian proj_src(latitude, longitude, 0, earth);
		if ((dststr == "enu") )
		{
			double latitude_dst = std::stold(exportVec[1]);
			double longitude_dst = std::stold(exportVec[2]);
			GeographicLib::LocalCartesian proj_dst(latitude_dst, longitude_dst, 0, earth);
			for (int i = 0; i < numPoints; i++)
			{
				proj_src.Reverse(src_x[i], src_y[i], src_z[i], src_x[i], src_y[i], src_z[i]);	
				double dst_x, dst_y, dst_z;
				proj_dst.Forward(src_x[i], src_y[i], src_z[i], dst_x, dst_y, dst_z);
				src_x[i] = dst_x;
				src_y[i] = dst_y;
				src_z[i] = dst_z;
			}
		}
		else if (dststr == "epsg")
		{
		
			for (int i = 0; i < numPoints; i++)
			{
				proj_src.Reverse(src_x[i], src_y[i], src_z[i], src_x[i], src_y[i], src_z[i]);
			}
			TransformByEpsgCode(numPoints, src_y, src_x, src_z, "epsg:4326", dst_crs);
			
			for (int i = 0; i < numPoints; i++)
			{
				double tmp = src_y[i];
				src_y[i] = src_x[i];
				src_x[i] = tmp;
			}

		}
		else
		{
			return false;
		}
	}



	return true;
}

bool CoordinateTransformer::TransformByEnu(double src_x, double src_y, double src_z,
	double& dst_x, double& dst_y, double& dst_z,
	std::string src_crs, std::string dst_crs)
{
	if ((src_x == -DBL_MAX) || (src_x == DBL_MAX))
	{
		return false;
	}
	if (IsSame(src_crs, dst_crs))
	{
		dst_x = src_x;
		dst_y = src_y;
		dst_z = src_z;
		return true;
	}
	if (CoordinateDescriptor::GetSRSFromDefinition(src_crs).type == LOCAL
		|| CoordinateDescriptor::GetSRSFromDefinition(dst_crs).type == LOCAL)
	{
		return false;
	}
	std::vector<std::string> importVec;
	std::vector<std::string> exportVec;

	double latitude, longitude;
	std::string sepstr = ":,";
	importVec = String::StringSplit(src_crs, sepstr);
	exportVec = String::StringSplit(dst_crs, sepstr);
	std::string srsstr = importVec[0];
	String::StringToLower(&srsstr);
	std::string dststr = exportVec[0];
	String::StringToLower(&dststr);

	if ((srsstr == "epsg"))
	{
		if (dststr == "enu")
		{

			latitude = std::stold(exportVec[1]);
			longitude = std::stold(exportVec[2]);
			
			TransformByEpsgCode(1, &src_x, &src_y, &src_z, src_crs, "epsg:4326");
			
			const GeographicLib::Geocentric& earth = GeographicLib::Geocentric::WGS84();

			GeographicLib::LocalCartesian proj(latitude, longitude, 0, earth);

			proj.Forward(src_y, src_x, src_z, dst_x, dst_y, dst_z);

		}
		

	}


	if ((srsstr == "enu") )
	{

		

		latitude = std::stold(importVec[1]);
		longitude = std::stold(importVec[2]);
		const GeographicLib::Geocentric& earth = GeographicLib::Geocentric::WGS84();

		GeographicLib::LocalCartesian proj(latitude, longitude, 0, earth);

		proj.Reverse(src_x, src_y, src_z, src_x, src_y, src_z);

		if ((dststr == "enu") )
		{
			latitude = std::stold(exportVec[1]);
			longitude = std::stold(exportVec[2]);



			const GeographicLib::Geocentric& earth = GeographicLib::Geocentric::WGS84();

			GeographicLib::LocalCartesian proj(latitude, longitude, 0, earth);

			proj.Forward(src_x, src_y, src_z, dst_x, dst_y, dst_z);
		}		
		else if (dststr == "epsg")
		{
			
			TransformByEpsgCode(1, &src_y, &src_x, &src_z, "epsg:4326", dst_crs);
			dst_x = src_y;
			dst_y = src_x;
			dst_z = src_z;


		}
		else
		{
			return false;
		}
	}
	return true;
}

		bool CoordinateTransformer::Transform(double src_x, double src_y, double src_z,
			double& dst_x, double& dst_y, double& dst_z,
			std::string src_crs, std::string dst_crs)
		{
			if ((src_x == -DBL_MAX) || (src_x == DBL_MAX))
			{
				return false;
			}
			if (IsSame(src_crs, dst_crs))
			{
				dst_x = src_x;
				dst_y = src_y;
				dst_z = src_z;
				return true;
			}
			if (CoordinateDescriptor::GetSRSFromDefinition(src_crs).type == LOCAL
				|| CoordinateDescriptor::GetSRSFromDefinition(dst_crs).type == LOCAL)
			{
				return false;
			}
			std::vector<std::string> importVec;
			std::vector<std::string> exportVec;

			double latitude, longitude;
			std::string sepstr = ":,";
			importVec = String::StringSplit(src_crs, sepstr);
			exportVec = String::StringSplit(dst_crs, sepstr);
			std::string srsstr = importVec[0];
			String::StringToLower(&srsstr);
			std::string dststr = exportVec[0];
			String::StringToLower(&dststr);

			if ((srsstr == "epsg"))
			{
				if ((dststr == "epsg"))
				{
					
					TransformByEpsgCode(1,&src_x, &src_y, &src_z,src_crs, dst_crs);
					dst_x = src_x;
					dst_y = src_y;
					dst_z = src_z;
				}
				else if (dststr == "enu")
				{

					latitude = std::stold(exportVec[1]);
					longitude = std::stold(exportVec[2]);
					
					TransformByEpsgCode(1, &src_x, &src_y, &src_z, src_crs, "epsg:4326");
					
					const GeographicLib::Geocentric& earth = GeographicLib::Geocentric::WGS84();

					GeographicLib::LocalCartesian proj(latitude, longitude, 0, earth);

					proj.Forward(src_y, src_x, src_z, dst_x, dst_y, dst_z);

				}
				else
				{
					return false;
				}

			}


			if ((srsstr == "enu") )
			{

			

				latitude = std::stold(importVec[1]);
				longitude = std::stold(importVec[2]);
				const GeographicLib::Geocentric& earth = GeographicLib::Geocentric::WGS84();

				GeographicLib::LocalCartesian proj(latitude, longitude, 0, earth);

				proj.Reverse(src_x, src_y, src_z, src_x, src_y, src_z);

				if ((dststr == "enu") )
				{
					latitude = std::stold(exportVec[1]);
					longitude = std::stold(exportVec[2]);



					const GeographicLib::Geocentric& earth = GeographicLib::Geocentric::WGS84();

					GeographicLib::LocalCartesian proj(latitude, longitude, 0, earth);

					proj.Forward(src_x, src_y, src_z, dst_x, dst_y, dst_z);
				}
				else if (dststr == "epsg")
				{
					
					TransformByEpsgCode(1, &src_y, &src_x, &src_z,  "epsg:4326", dst_crs);
					std::swap(src_x, src_y);
					dst_x = src_x;
					dst_y = src_y;
					dst_z = src_z;

				}
				else
				{
					return false;
				}

			}
			return true;
		}

		bool CoordinateTransformer::TransformByEpsgCode(double& src_x, double& src_y,
			double& src_z, double& dst_x, double& dst_y, double& dst_z,
			std::string src_crs, std::string dst_crs)
		{
			
			if (IsSame(src_crs, dst_crs))
			{

				dst_x = src_x;
				dst_y = src_y;
				dst_z = src_z;
				return true;
				
			}
			if (CoordinateDescriptor::GetSRSFromDefinition(src_crs).type == LOCAL
				|| CoordinateDescriptor::GetSRSFromDefinition(dst_crs).type == LOCAL)
			{
				return false;
			}
			PJ_CONTEXT* C;
			PJ* P;
			PJ* P_for_GIS;
			PJ_COORD a, b;
			a.xyz.x = src_x;
			a.xyz.y = src_y;
			a.xyz.z = src_z;
			
			
			C = proj_context_create();

			
			

			  
			  
			 
			  


			P = proj_create_crs_to_crs(C,
				src_crs.c_str(),
				dst_crs.c_str(), 
				NULL);


			if (0 == P)
			{

				return false;
			}


			P_for_GIS = proj_normalize_for_visualization(C, P);
			if (0 == P_for_GIS)
			{

				return false;
			}
			proj_destroy(P);
			P = P_for_GIS;

			b = proj_trans(P, PJ_FWD, a);
			dst_x = b.xyz.x; dst_y = b.xyz.y; dst_z = b.xyz.z;



			
			proj_destroy(P);
			
			proj_context_destroy(C); 
			return true;
		}
		bool CoordinateTransformer::TransformPoints(std::vector < std::vector<Eigen::Vector3d>>& points,std::string src_crs, std::string dst_crs)
		{
			if (points.empty())
				return false;
			if (IsSame(src_crs, dst_crs))
			{
				return true;
			}
			if (CoordinateDescriptor::GetSRSFromDefinition(src_crs).type == LOCAL
				|| CoordinateDescriptor::GetSRSFromDefinition(dst_crs).type == LOCAL)
			{
				return false;
			}
			int ptcount = 0;
			for (auto& iter : points)
			{
				ptcount += iter.size();
			}
			
			int i_pt = 0;
			int step = 0;
			std::vector<double> x(ptcount), y(ptcount), z(ptcount);
			for (int i = 0; i < points.size(); i++)
			{
			
				for (int j = 0; j < points[i].size(); j++)
				{
					x[step + j] = points[i][j].x();
					y[step + j] = points[i][j].y();
					z[step + j] = points[i][j].z();
				}
				step += points[i].size();
			}
			
			CoordinateTransformer::Transform(x.size(), &x[0], &y[0],
				&z[0], src_crs, dst_crs);
			step = 0;
			for (int i = 0; i < points.size(); i++)
			{
				
				for (int j = 0; j < points[i].size(); j++)
				{
					points[i][j].x() = x[step + j]  ;
					points[i][j].y() = y[step + j];
					points[i][j].z() = z[step + j] ;
				}
				step += points[i].size();
			}
			
			return true;
		}

		bool CoordinateTransformer::Transform(int numPoints, double* src_x, double* src_y, double* src_z, std::string src_crs, std::string dst_crs)
		{
			if (numPoints == 0)
				return false;
			
			if (IsSame(src_crs, dst_crs))
			{				
				return true;
			}
			if (CoordinateDescriptor::GetSRSFromDefinition(src_crs).type == LOCAL
				|| CoordinateDescriptor::GetSRSFromDefinition(dst_crs).type == LOCAL)
			{
				return false;
			}
			std::vector<std::string> importVec;
			std::vector<std::string> exportVec;

			
			std::string sepstr = ":,";
			importVec = String::StringSplit(src_crs, sepstr);
			exportVec = String::StringSplit(dst_crs, sepstr);
			std::string srsstr = importVec[0];
			String::StringToLower(&srsstr);
			std::string dststr = exportVec[0];
			String::StringToLower(&dststr);

			if ((srsstr == "epsg") && (dststr == "epsg"))
			{
				TransformByEpsgCode(numPoints, src_x, src_y, src_z, src_crs, dst_crs);
			}
			else
			{
				TransformByEnu(numPoints, src_x, src_y, src_z, src_crs, dst_crs);			
			}
				
			return true;
		}

		bool CoordinateTransformer::TransformByEpsgCode(int numPoints, double* src_x, double* src_y, double* src_z, std::string src_crs, std::string dst_crs)
		{
			if (IsSame(src_crs, dst_crs))
			{
				return true;
			}
			if (CoordinateDescriptor::GetSRSFromDefinition(src_crs).type == LOCAL
				|| CoordinateDescriptor::GetSRSFromDefinition(dst_crs).type == LOCAL)
			{
				return false;
			}





















































































			PJ_CONTEXT* C;
			PJ* P;
			PJ* P_for_GIS;
			
					
			C = proj_context_create();
			P = proj_create_crs_to_crs(C,src_crs.c_str(),dst_crs.c_str(), NULL);
			if (0 == P)
			{
				return false;
			}

			P_for_GIS = proj_normalize_for_visualization(C, P);
			if (0 == P_for_GIS)
			{
				return false;
			}


		

			proj_destroy(P);
			P = P_for_GIS;

		

			proj_trans_generic(P, PJ_FWD,
				src_x, sizeof(double), numPoints,
				src_y, sizeof(double), numPoints,
				src_z, sizeof(double), numPoints,
				nullptr, sizeof(double), 0);

			proj_destroy(P);
			
			proj_context_destroy(C); 

			return true;
		}


	}
}
