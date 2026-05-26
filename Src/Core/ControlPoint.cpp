#include"Core/ControlPoint.h"
#include "Core/CoordinateSystem.h"
#include "Core/ReturnCode.h"
#include "Core/BlockObject.h"
#include "Core/File.h"
#include <pugixml.hpp>
namespace AI3D
{
	namespace CORE
	{

		ControlPoint::ControlPoint():name_(""),
			id_(kInvalidPoint3DId),
			type_(GCP_CONTROL_HV),
			weight_(Eigen::Vector2d(0.01,0.01)),
			error_3d_(kInvalidError),
			error_3d_xy_(kInvalidError),
		    error_3d_z_(kInvalidError)
			
		{

		}

		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		void ControlPoint::SetName(std::string name) 
		{ 
			this->name_ = name; 
		};

		std::string ControlPoint::GetName()const
		{ 
			return name_; 
		};

		std::string ControlPoint::GetNameMutual()
		{
			return name_;
		};
		
		void ControlPoint::SetId(point3D_t id) 
		{ 
			id_ = id; 
		};
		point3D_t ControlPoint::GetId() const
		{ 
			return id_; 
		};

		point3D_t ControlPoint::GetIdMutual() 
		{
			return id_;
		};

		void ControlPoint::SetSrs(srs_s srs)
		{
			origin_srs_ = srs;
		}
		const srs_s ControlPoint::GetSrs() const
		{
			return origin_srs_;
		}
		srs_s ControlPoint::GetSrsMutual()
		{
			return origin_srs_;
		}

		
		void ControlPoint::SetGivenXYZ(Eigen::Vector3d xyz) 
		{ 
			xyz_ = xyz; 
		};



		Eigen::Vector3d& ControlPoint::GetGivenXYZMutual()
		{ 
			return xyz_; 
		};
		const Eigen::Vector3d  ControlPoint::GetGivenXYZ()const
		{
			return xyz_;
		}
		bool ControlPoint::Has3DError()
		{
			return error_3d_ != -1.0;
		}
	
		bool ControlPoint::HasGivenXYZ() const
		{
			if (xyz_.x() != -DBL_MAX && xyz_.y() != -DBL_MAX && xyz_.z() != -DBL_MAX)
			{
				return true;
			}
			return false;
		}


		

		bool ControlPoint::HasEstimatedXYZ()
		{
			if (estimated_xyz_.x() != -DBL_MAX && estimated_xyz_.y() != -DBL_MAX && estimated_xyz_.z() != -DBL_MAX)
			{
				return true;
			}
			return false;
		}

		void ControlPoint::SetEstimatedXYZ(Eigen::Vector3d& xyz) 
		{ 
			estimated_xyz_ = xyz; 
		};
		Eigen::Vector3d& ControlPoint::GetEstimatedXYZMutual()
		{
			return estimated_xyz_;
		};
		const Eigen::Vector3d  ControlPoint::GetEstimatedXYZ()const
		{
			return estimated_xyz_;
		};
		
		void ControlPoint::SetType(gpt_e type) 
		{ 
			type_ = type; 
		};
		gpt_e ControlPoint::GetType() const
		{ 
			return type_; 
		};
		gpt_e ControlPoint::GetTypeMutual()
		{
			return type_;
		};

		
		void ControlPoint::SetWeight(Eigen::Vector2d weight) 
		{ 
			weight_ = weight; 
		};
		Eigen::Vector2d ControlPoint::GetWeightMutual()
		{
			return weight_;
		};

		Eigen::Vector2d ControlPoint::GetWeight()const
		{
			return weight_;
		}
		
		void ControlPoint::SetObjectPoint(const Point3D& objectPoint)
		{
			objectpoint_ = objectPoint;
		};
		Point3D& ControlPoint::GetObjectPointMutual()
		{
			return objectpoint_;
		};
		const Point3D ControlPoint::GetObjectPoint() const
		{
			return objectpoint_;
		};

		void ControlPoint::Set3DError(double error_3d)
		{
			error_3d_ = error_3d;
		};
		const double ControlPoint::Get3DError() const
		{
			return error_3d_;
		};


		void ControlPoint::Calc3DError()
		{
			
			Eigen::Vector3d givenXYZ = objectpoint_.GetXYZ();
			Eigen::Vector3d emitXYZ = estimated_xyz_;
			if ((givenXYZ.x() != -DBL_MAX) && (emitXYZ.x() != -DBL_MAX))
			{
				double x1 = givenXYZ.x(); double y1 = givenXYZ.y();
				double x2 = emitXYZ.x(); double y2 = emitXYZ.y();
				double z1 = givenXYZ.z(); double z2 = emitXYZ.z();
				double dx = (x1 - x2);
				double dy = (y1 - y2);
				double dxy = sqrt(dx * dx + dy * dy);
				double dz = (z1 - z2);
				double dxyz = sqrt(dx * dx + dy * dy + dz *dz);
				error_3d_ = dxyz;
				error_3d_xy_ = dxy;
				error_3d_z_ = dz;
			}
		}


		

		void ControlPoint::SetXY3DError(double error)
		{
			error_3d_xy_ = error;
		};
		const double ControlPoint::GetXY3DError() const
		{
			return error_3d_xy_;
		};

		void ControlPoint::SetZ3DError(double error)
		{
			error_3d_z_ = error;
		};
		const double ControlPoint::GetZ3DError() const
		{
			return error_3d_z_;
		};

		const double  ControlPoint::GetX3DError() const
		{
			return error_3d_x_;
		}
		void  ControlPoint::SetX3DError(double error)
		{
			error_3d_x_ = error;
		}
		const double  ControlPoint::GetY3DError() const
		{
			return error_3d_y_;
		}
		void  ControlPoint::SetY3DError(double error)
		{
			error_3d_y_ = error;
		}
		point3D_t ControlPoints::GetGCPCount()
		{
			return controlpoints_.size();
		}
		point3D_t ControlPoints::GetCheckPointCount()
		{
			point3D_t count = 0;
			for (auto& it : controlpoints_)
			{
				if (it.second.GetType() == _gcp_type_e::GCP_CHECK_HV)
				{
					count++;
				}
			}
			return count;
		}
		
		point3D_t ControlPoints::GetValidGCPPointCount()
		{
			point3D_t count = 0;
			for (auto& it : controlpoints_)
			{
				if (it.second.GetObjectPoint().GetTrack().Length() >= 2)
				{
					count++;
				}
			}
			return count;
		}

		point3D_t ControlPoints::GetFullControlPointCount()
		{
			point3D_t count = 0;
			for (auto& it : controlpoints_)
			{
				if (it.second.GetType() == _gcp_type_e::GCP_CONTROL_HV)
				{
					count++;
				}
			}
			return count;
		}

		

		point3D_t ControlPoints::GetXYControlPointCount()
		{
			point3D_t count = 0;
			for (auto& it : controlpoints_)
			{
				if (it.second.GetType() == _gcp_type_e::GCP_CONTROL_H)
				{
					count++;
				}
			}
			return count;
		}
		point3D_t ControlPoints::GetZControlPointCount()
		{
			point3D_t count = 0;
			for (auto& it : controlpoints_)
			{
				if (it.second.GetType() == _gcp_type_e::GCP_CONTROL_V)
				{
					count++;
				}
			}
			return count;
		}

		void  ControlPoints::SetPoints(const EIGEN_STL_UMAP(point3D_t, ControlPoint)& gcps)
		{
			controlpoints_ = gcps;
		}

		bool ControlPoints::ExistsPoint(const point3D_t point_id) const
		{
			return controlpoints_.find(point_id) != controlpoints_.end();
		}
		void ControlPoints::ADDPoint(ControlPoint point)
		{
			if(!ExistsPoint(point.GetId()))
				controlpoints_[point.GetId()] = point;
		}
		void ControlPoints::DeletePoint(point3D_t id)
		{

		}
		ControlPoint& ControlPoints::GetPoint(point3D_t id)
		{
			return controlpoints_.at(id);
			
		}
		const EIGEN_STL_UMAP(point3D_t, ControlPoint)& ControlPoints::GetPoints() const
		{
			return controlpoints_;
		}
		EIGEN_STL_UMAP(point3D_t, ControlPoint)& ControlPoints::GetPointsMutual()
		{
			return controlpoints_;
		}

		std::string ControlPoints::GetSRS()
		{
			return srs_;
		}
		void ControlPoints::SetSRS(std::string srs)
		{
			srs_ = srs;
		}
		const Eigen::Vector3d& ControlPoints::GetPositionOffset() const
		{
			return position_offset_;
		}
		Eigen::Vector3d& ControlPoints::GetPositionOffsetMutual()
		{
			return position_offset_;
		}

		std::vector<point3D_t> ControlPoints::GetControlPointIds() const
		{
			std::vector<point3D_t> img_idx;

			for (auto it = controlpoints_.begin(); it != controlpoints_.end(); it++)
			{					
					img_idx.push_back(it->second.GetId());			
			}
			return img_idx;

		}
		bool ControlPoints::TransformEstimatedXYZToGivenXYZSrs(std::string src_srs)
		{
			std::vector<point3D_t> gcpIds = ControlPoints::GetControlPointIds();


			std::map<std::string, std::vector<point3D_t>> srsmap_temp;
			for (point3D_t i = 0; i < gcpIds.size(); i++)
			{

				srs_s src_crs = controlpoints_[gcpIds[i]].GetSrs();
				if (src_crs.definition == "" || src_crs.type == coord_system_type_e::LOCAL)
				{
					continue;
				}
				srsmap_temp[src_crs.definition].push_back(gcpIds[i]);
			}

			for (auto& it : srsmap_temp)
			{

				point3D_t ptcount = it.second.size();

				
				std::vector<double> x_estimate(ptcount), y_estimate(ptcount), z_estimate(ptcount);


				for (point3D_t i_pt = 0; i_pt < it.second.size(); i_pt++)
				{
					

					if (controlpoints_[it.second[i_pt]].HasEstimatedXYZ())
					{
						Eigen::Vector3d xyz_estimate = controlpoints_[it.second[i_pt]].GetEstimatedXYZ();
						x_estimate[i_pt] = xyz_estimate.x();
						y_estimate[i_pt] = xyz_estimate.y();
						z_estimate[i_pt] = xyz_estimate.z();
					}

				}
				if ( x_estimate.empty())
				{
					return false;
				}
				
				CoordinateTransformer::Transform(x_estimate.size(), &x_estimate[0], &y_estimate[0], &z_estimate[0], src_srs,it.first);

				for (point3D_t i_pt = 0; i_pt < it.second.size(); i_pt++)
				{
					
					if (controlpoints_[it.second[i_pt]].HasEstimatedXYZ())
					{
						Eigen::Vector3d xyz{ x_estimate[i_pt],y_estimate[i_pt],z_estimate[i_pt] };
						controlpoints_[it.second[i_pt]].SetEstimatedXYZ(xyz);
					}
					
				}

			}
			return true;
		}
		
		bool ControlPoints::TransformPointsToTheSrsOfOnepoint()
		{
			std::vector<point3D_t> gcpIds = ControlPoints::GetControlPointIds();


			std::map<std::string, std::vector<point3D_t>> srsmap_temp;
			for (point3D_t i = 0; i < gcpIds.size(); i++)
			{

				srs_s src_crs = controlpoints_[gcpIds[i]].GetSrs();
				if (src_crs.definition == "" || src_crs.type == coord_system_type_e::LOCAL)
				{
					continue;
				}
				srsmap_temp[src_crs.definition].push_back(gcpIds[i]);
			}
			if (srsmap_temp.empty())
			{
				return false;
			}
			std::string dst_crs = srsmap_temp.begin()->first;
			for (auto& it : srsmap_temp)
			{

				point3D_t ptcount = it.second.size();

				std::vector<double> x(ptcount), y(ptcount), z(ptcount);
				std::vector<double> x_estimate(ptcount), y_estimate(ptcount), z_estimate(ptcount);


				for (point3D_t i_pt = 0; i_pt < it.second.size(); i_pt++)
				{
					if (controlpoints_[it.second[i_pt]].HasGivenXYZ())
					{
						Eigen::Vector3d xyz = controlpoints_[it.second[i_pt]].GetGivenXYZ();
						x[i_pt] = xyz.x();
						y[i_pt] = xyz.y();
						z[i_pt] = xyz.z();
					}

					if (controlpoints_[it.second[i_pt]].HasEstimatedXYZ())
					{
						Eigen::Vector3d xyz_estimate = controlpoints_[it.second[i_pt]].GetEstimatedXYZ();
						x_estimate[i_pt] = xyz_estimate.x();
						y_estimate[i_pt] = xyz_estimate.y();
						z_estimate[i_pt] = xyz_estimate.z();
					}

				}
				if (x.empty() || x_estimate.empty())
				{
					return false;
				}
				CoordinateTransformer::Transform(x.size(), &x[0], &y[0], &z[0], it.first, dst_crs);
				CoordinateTransformer::Transform(x_estimate.size(), &x_estimate[0], &y_estimate[0], &z_estimate[0], it.first, dst_crs);

				for (point3D_t i_pt = 0; i_pt < it.second.size(); i_pt++)
				{
					if (controlpoints_[it.second[i_pt]].HasGivenXYZ())
					{
						Eigen::Vector3d xyz{ x[i_pt],y[i_pt],z[i_pt] };
						controlpoints_[it.second[i_pt]].SetGivenXYZ(xyz);
					}
					if (controlpoints_[it.second[i_pt]].HasEstimatedXYZ())
					{
						Eigen::Vector3d xyz{ x_estimate[i_pt],y_estimate[i_pt],z_estimate[i_pt] };
						controlpoints_[it.second[i_pt]].SetEstimatedXYZ(xyz);
					}
					auto srsnew = CoordinateDescriptor::GetSRSFromDefinition(dst_crs);
					srsnew.ID = controlpoints_[it.second[i_pt]].GetSrsMutual().ID;
					controlpoints_[it.second[i_pt]].SetSrs(srsnew);
				}

			}
			srs_ = dst_crs;
			return true;
		}

		
		bool ControlPoints::TransformPoints(const std::string dst_crs)
		{
			
			
			std::vector<point3D_t> gcpIds = ControlPoints::GetControlPointIds();
			

			std::map<std::string, std::vector<point3D_t>> srsmap_temp;
			for (point3D_t i = 0; i < gcpIds.size(); i++)
			{
				
				srs_s src_crs = controlpoints_[gcpIds[i]].GetSrs();
				if (src_crs.definition == "" || src_crs.type == coord_system_type_e::LOCAL)
				{
					continue;
				}
				srsmap_temp[src_crs.definition].push_back(gcpIds[i]);
			}

			for (auto& it : srsmap_temp)
			{
				
				point3D_t ptcount = it.second.size();

				std::vector<double> x(ptcount), y(ptcount), z(ptcount);
				std::vector<double> x_estimate(ptcount), y_estimate(ptcount), z_estimate(ptcount);

				
				for (point3D_t i_pt = 0; i_pt < it.second.size(); i_pt++)
				{
					if (controlpoints_[it.second[i_pt]].HasGivenXYZ())
					{
						Eigen::Vector3d xyz = controlpoints_[it.second[i_pt]].GetGivenXYZ();
						x[i_pt] = xyz.x();
						y[i_pt] = xyz.y();
						z[i_pt] = xyz.z();
					}

					if (controlpoints_[it.second[i_pt]].HasEstimatedXYZ())
					{
						Eigen::Vector3d xyz_estimate = controlpoints_[it.second[i_pt]].GetEstimatedXYZ();
						x_estimate[i_pt] = xyz_estimate.x();
						y_estimate[i_pt] = xyz_estimate.y();
						z_estimate[i_pt] = xyz_estimate.z();
					}
					
				}
				if (x.empty() || x_estimate.empty())
				{
					return false;
				}
				CoordinateTransformer::Transform(x.size(), &x[0], &y[0], &z[0], it.first, dst_crs);
				CoordinateTransformer::Transform(x_estimate.size(), &x_estimate[0], &y_estimate[0], &z_estimate[0], it.first, dst_crs);
			
				for (point3D_t i_pt = 0; i_pt < it.second.size(); i_pt++)
				{
					if (controlpoints_[it.second[i_pt]].HasGivenXYZ())
					{
						Eigen::Vector3d xyz{ x[i_pt],y[i_pt],z[i_pt] };
						controlpoints_[it.second[i_pt]].SetGivenXYZ(xyz);
					}
					if (controlpoints_[it.second[i_pt]].HasEstimatedXYZ())
					{
						Eigen::Vector3d xyz{ x_estimate[i_pt],y_estimate[i_pt],z_estimate[i_pt] };
						controlpoints_[it.second[i_pt]].SetEstimatedXYZ(xyz);
					}
					auto srsnew = CoordinateDescriptor::GetSRSFromDefinition(dst_crs);
					srsnew.ID = controlpoints_[it.second[i_pt]].GetSrsMutual().ID;
					controlpoints_[it.second[i_pt]].SetSrs(srsnew);
				}

			}
			return true;
		}


		
		bool ControlPoints::TransformPointsToBaseCoordinate(const std::string dst_crs)
		{		
				
				
			std::vector<point3D_t> gcpIds =  ControlPoints::GetControlPointIds();


			std::map<std::string, std::vector<point3D_t>> srsmap_temp;
			for (point3D_t i = 0; i < gcpIds.size(); i++)
			{
				srs_s src_crs = controlpoints_[gcpIds[i]].GetSrs();
				srsmap_temp[src_crs.definition].push_back(gcpIds[i]);
			}

			for (auto& it : srsmap_temp)
			{
				point3D_t ptcount = it.second.size();
				
				std::vector<double> x(ptcount), y(ptcount), z(ptcount);

				
				for (point3D_t i_pt = 0; i_pt < it.second.size(); i_pt++)
				{
					Eigen::Vector3d xyz = controlpoints_[it.second[i_pt]].GetGivenXYZ();
					x[i_pt] = xyz.x();
					y[i_pt] = xyz.y();
					z[i_pt] = xyz.z();
					
				}
				CoordinateTransformer::Transform(x.size(), &x[0], &y[0],&z[0], it.first, dst_crs);
				for (point3D_t i_pt = 0; i_pt < it.second.size(); i_pt++)
				{
					Eigen::Vector3d xyz{ x[i_pt],y[i_pt],z[i_pt] };
					controlpoints_[it.second[i_pt]].GetObjectPointMutual().SetXYZ(xyz);
					
				}
			}
			srs_ = dst_crs;
		
			
			
			return true;
		}

		int ControlPoints::SaveXML(const std::string& xml_file_path,const std::map<image_t, std::string>& images) const
		{
			
			EIGEN_STL_UMAP(srsid_t, srs_s) output_srs;
			for (auto& iter : controlpoints_)
			{
				srs_s src_crs = iter.second.GetSrs();
				output_srs[src_crs.ID]=src_crs;
			}

			pugi::xml_document doc;
			pugi::xml_node declaration_node = doc.append_child(pugi::node_declaration);

			declaration_node.append_attribute("version") = "1.0";
			declaration_node.append_attribute("encoding") = "utf-8";

			pugi::xml_node blocksexchange = doc.append_child("SurveysData");
			
			
			

			
			if (!output_srs.empty())
			{
				pugi::xml_node SpatialReferenceSystems = blocksexchange.append_child("SpatialReferenceSystems");


				for (const auto& srs_map : output_srs)
				{
					pugi::xml_node srs = SpatialReferenceSystems.append_child("SRS");
					pugi::xml_node id = srs.append_child("Id");
					id.append_child(pugi::node_pcdata).set_value(std::to_string(srs_map.second.ID).c_str());
					pugi::xml_node name = srs.append_child("Name");
					name.append_child(pugi::node_pcdata).set_value(srs_map.second.name.c_str());
					
					pugi::xml_node definition = srs.append_child("Definition");
					definition.append_child(pugi::node_pcdata).set_value(srs_map.second.definition.c_str());
				}

			}


			pugi::xml_node node_cp = blocksexchange.append_child("ControlPoints");
		
			for (auto gcpiter : controlpoints_)
			{
				auto cp = gcpiter.second;
				pugi::xml_node SRSId = node_cp.append_child("SRSId");
				srsid_t id = cp.GetSrs().ID;
				SRSId.append_child(pugi::node_pcdata).set_value(std::to_string(id).c_str());

				pugi::xml_node Name = node_cp.append_child("Name");
				Name.append_child(pugi::node_pcdata).set_value(cp.GetName().c_str());
				std::string category = "";
				pugi::xml_node Position = node_cp.append_child("Position");
				pugi::xml_node x = Position.append_child("x");
				pugi::xml_node y = Position.append_child("y");
				pugi::xml_node z = Position.append_child("z");
				Eigen::Vector3d xyz = cp.GetGivenXYZ();

				x.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(xyz(0)).c_str());
				y.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(xyz(1)).c_str());
				z.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(xyz(2)).c_str());

				pugi::xml_node HorizontalAccuracy = node_cp.append_child("HorizontalAccuracy");
				pugi::xml_node VerticalAccuracy = node_cp.append_child("VerticalAccuracy");
				HorizontalAccuracy.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(cp.GetWeight()(0)).c_str());
				VerticalAccuracy.append_child(pugi::node_pcdata).set_value(File::ToStringWithHighPrecision(cp.GetWeight()(1)).c_str());

				gpt_e gpte;

				bool checkpoint = false;
				switch (cp.GetType())
				{
				case 1:
					category = "Horizontal";
					break;
				case 2:
					category = "Vertical";
					break;
				case 3:
					category = "Full";
					break;
				case 4:
					checkpoint = true;
					break;
				case 5:
					checkpoint = true;
					break;
				case 6:
					checkpoint = true;
					break;
				}
				
				
				pugi::xml_node Category = node_cp.append_child("Category");
				Category.append_child(pugi::node_pcdata).set_value(category.c_str());
				pugi::xml_node CheckPoint = node_cp.append_child("CheckPoint");
				CheckPoint.append_child(pugi::node_pcdata).set_value(checkpoint ? "true" : "false");
				std::vector<TrackElement> vector = cp.GetObjectPoint().GetTrack().GetElements();
				for (const auto& it : vector)
				{
					pugi::xml_node Measurement = node_cp.append_child("Measurement");
					pugi::xml_node photo_id = Measurement.append_child("PhotoId");
					
					photo_id.append_child(pugi::node_pcdata).set_value(std::to_string(it.image_id).c_str());
					pugi::xml_node Imagepath = Measurement.append_child("ImagePath");					
					Imagepath.append_child(pugi::node_pcdata).set_value(images.at(it.image_id).c_str());
					
					pugi::xml_node x = Measurement.append_child("x");
					x.append_child(pugi::node_pcdata).set_value(std::to_string(it.xy.x()).c_str());
					pugi::xml_node y = Measurement.append_child("y");
					y.append_child(pugi::node_pcdata).set_value(std::to_string(it.xy.y()).c_str());
				}
			}
			bool saveSucceed = doc.save_file(xml_file_path.c_str());
			if (!saveSucceed)
			{
				LOG(ERROR) << "saving xml failed！";
				return AI3D_FAILURE;
			}
			return AI3D_SUCCESS;
			
		}
		int ControlPoints::LoadXML(const std::string& xml_file_path)
		{
			return AI3D_SUCCESS;
		}
		bool ControlPoints::LoadText(const std::string& gcpfilepath)
		{
			point3D_t index_controlpoints = 0;
			std::string strs;
			if (RapidJsonCore::ReadFile(gcpfilepath, strs) != AI3D_SUCCESS)
			{
				LOGE(String::StringPrintf("Load gcp file %s error!", gcpfilepath.c_str()));
				return false;
			}
			std::istringstream in(strs);
			std::string name,key;
			double x, y, z;
			x = y = z = -1;
			while(in >> key)
			{
				if (key != "\n")
				{
					name = key;
					in >> x >> y >> z;
				}
				else
				{
					continue;
				}

				ControlPoint cp;
				cp.SetName(name);
				Eigen::Vector3d xyz(x, y, z);
				cp.SetGivenXYZ(xyz);
				cp.SetId(index_controlpoints);
				
				controlpoints_.insert(std::make_pair(index_controlpoints, cp));
				index_controlpoints++;
			}

			return true;
		}

		bool ControlPoints::SaveTextFor3DView(const std::string& gcpfilepath) const
		{
			std::ofstream file = File::OpenOfstreamUtf8(gcpfilepath, std::ios::trunc);
			if (!file.is_open())
			{
				return false;
			}
			for (auto& gcp : controlpoints_)
			{
				int colorcode = 0;
				if (gcp.second.GetType() == gpt_e::GCP_CONTROL_HV)
				{
					if(gcp.second.GetObjectPoint().GetTrack().Length()<2)
						colorcode = 2;
					else
						colorcode = 1;
				}
				else if (gcp.second.GetType() == gpt_e::GCP_CHECK_HV)
				{
					if (gcp.second.GetObjectPoint().GetTrack().Length() < 2)
						colorcode = 3;
					else
						colorcode = 4;
				}
				
				file << gcp.second.GetName() << " " << gcp.second.GetGivenXYZ().x() << " " << gcp.second.GetGivenXYZ().y() << " " << gcp.second.GetGivenXYZ().z()<< " "<< colorcode << std::endl;
			}
			file.close();
			return true;
		}


		bool ControlPoints::SaveText(const std::string& filepath) const
		{

			std::ofstream file = File::OpenOfstreamUtf8(filepath, std::ios::trunc);
			if (!file.is_open())
			{
				return false;
			}
			for (auto& gcp : controlpoints_)
			{
				file << gcp.second.GetName() << " " << gcp.second.GetGivenXYZ().x() << " " << gcp.second.GetGivenXYZ().y() << " " << gcp.second.GetGivenXYZ().z() << std::endl;
			}
			file.close();
			return true;

		}
		int ControlPoints::LoadJson(const std::string& path)
		{
			std::string context;
			if (RapidJsonCore::ReadFile(path, context)!= AI3D_SUCCESS)
			{
				LOG(ERROR) << "ReadFile " << path << " Error";
				return -1;
			}
			if (context.empty())
			{
				LOG(ERROR) << "GCP file is empty!";
				return -1;
			}

			
			rapidjson::Document doc;
			if (doc.Parse(context.data()).HasParseError())
			{
				LOG(ERROR) << "Parse GCP file ERROR!";
				return -1;
			}

			if (!doc.IsObject())
			{
				LOG(ERROR) << "Parse GCP file ERROR!";
				return -1;
			}

			if (!doc.HasMember("coordinate_system") || !doc.HasMember("points"))
			{
				LOG(ERROR) << "GCP Memeber was lost!";
				return -1;
			}

			rapidjson::Value& coord_sys = doc["coordinate_system"];
			rapidjson::Value& points = doc["points"];

			_srs_s srs;
			if (coord_sys.HasMember("type"))
			{
				srs.type = coord_system_type_e(coord_sys["type"].GetDouble());
				
			}
			


			for (int i = 0; i < points.Size(); i++)
			{
				point3D_t index_p3d = 0;
				ControlPoint cp;
				Track track;
				if (points[i].HasMember("coordinate"))
				{
					
					
					Eigen::Vector3d xyz(points[i]["coordinate"][0].GetDouble(), points[i]["coordinate"][1].GetDouble(), points[i]["coordinate"][2].GetDouble());
					cp.SetGivenXYZ(xyz);

					Point3D point3d;
					cp.SetObjectPoint(point3d);
				}

				if (points[i].HasMember("id"))
				{
					index_p3d = points[i]["id"].GetInt();
					cp.SetId(index_p3d);
				}
				else
				{
					LOG(ERROR) << "ControlPoints have no id!";
					return -1;
				}

				if (points[i].HasMember("observations"))
				{
					std::vector<TrackElement> vec_track_ele;
					rapidjson::Value& observations = points[i]["observations"];
					for (int j = 0; j < observations.Size(); j++)
					{
						TrackElement ele;
						ele.point2D_idx = index_p3d;
						Eigen::Vector2d x(0, 0);
						if (observations[j].HasMember("id"))
						{
							ele.image_id = observations[j]["id"].GetInt();
						}
						else
						{
							LOG(ERROR) << "observations have no id!";
							return false;
						}

						if (observations[j].HasMember("uv"))
						{
							x.x() = 1.0 * round(observations[j]["uv"][0].GetDouble() * 100) / 100;
							x.y() = 1.0 * round(observations[j]["uv"][1].GetDouble() * 100) / 100;
							ele.xy = x;
						}
						else
						{
							LOG(ERROR) << "observations have no uv!";
							return false;
						}
						vec_track_ele.push_back(ele);
					}
					track.AddElements(vec_track_ele);
				}
				cp.GetObjectPointMutual().SetTrack(track);

				if (points[i].HasMember("usage"))
				{
					cp.SetType(gpt_e(points[i]["usage"].GetInt()));
				}
				else
				{
					LOG(ERROR) << "ControlPoints have no usage(type)!";
					return -1;
				}
				controlpoints_.insert(std::make_pair(index_p3d, cp));
			}
			return AI3D_SUCCESS;
		}


		

		void ControlPoints::SaveJson(const std::string& outpath, const srs_s& srs) const
		{
			
			rapidjson::Document documnet;
			documnet.SetObject();
			rapidjson::Document::AllocatorType& allocator = documnet.GetAllocator();

			rapidjson::Value coordinate(rapidjson::kObjectType);
			
			coordinate.AddMember("type", rapidjson::Value(), allocator);
			documnet.AddMember("coordinate_system", coordinate, allocator);


			rapidjson::Value points(rapidjson::kArrayType);
			
			for (const auto& cp : controlpoints_)
			{
				rapidjson::Value point(rapidjson::kObjectType);
				point3D_t point3d_t = cp.first;
				point.AddMember("id", rapidjson::Value(point3d_t), allocator);

				ControlPoint Control_Point = cp.second;
				Eigen::Vector3d point3d = Control_Point.GetGivenXYZ();

				rapidjson::Value vec_point3d(rapidjson::kArrayType);
				for (int i = 0; i < point3d.size(); i++)
				{
					vec_point3d.PushBack(point3d[i], allocator);
				}
				point.AddMember("coordinate", vec_point3d, allocator);

				Track track = Control_Point.GetObjectPoint().GetTrack();
				rapidjson::Value vec_observation(rapidjson::kArrayType);
				for (const auto& ele : track.GetElements())
				{
					rapidjson::Value observation(rapidjson::kObjectType);
					observation.AddMember("id", rapidjson::Value(ele.image_id), allocator);


					Eigen::Vector2d uv_pix = ele.xy;
					rapidjson::Value uv(rapidjson::kArrayType);
					for (int j = 0; j < 2; j++)
					{
						uv.PushBack(uv_pix[j], allocator);
					}
					observation.AddMember("uv", uv, allocator);
					vec_observation.PushBack(observation, allocator);
				}
				point.AddMember("observations", vec_observation, allocator);
				point.AddMember("usage", Control_Point.GetType(), allocator);
				points.PushBack(point,allocator);
			}
			documnet.AddMember("points", points, allocator);
			

			if (RapidJsonCore::SaveFile(outpath, documnet) != AI3D_SUCCESS)
			{
				LOG(ERROR) << "Save GCP file failed!";
			}
		}

	}
}