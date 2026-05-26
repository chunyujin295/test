
#include "Core/ATCommandSet.h"
#include "Core/TaskCommandSet.h"
#include "Core/CoordinateSystem.h"
#include "Core/TaskInfo.h"
#include "Util/TaskProcess.h"
#include "Core/Types.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/filewritestream.h>
#include "Core/DataStruct.h"
#include "Core/File.h"
namespace AI3D
{
    namespace CORE
    {

		int ATCommandSet::AddUserTiepoint(ATData& data, image_t image_id, std::string name)
		{
			Point3D point;
			point3D_t id = data.GenerateValidUserPtId();
			point.SetId(id);
			point.SetName(name);
			point.image_for_userptguide_ = image_id;

			data.GetUserPoints3DMutual()[id] = point;
			

			return AI3D_SUCCESS;
		}
       


		int ATCommandSet::WriteUserTiepointsJson(const ATData& atdata, const std::string& file)
		{
			
			if (!atdata.HasUserTiepoints())
			{
				LOGI("no usertiepoints to save");
				return AI3D_FAILURE;
			}
			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
			rapidjson::Document documnet;
			documnet.SetObject();
			rapidjson::Document::AllocatorType& allocator = documnet.GetAllocator();


			rapidjson::Value points(rapidjson::kArrayType);

			for (const auto& cp : atdata.GetUserPoints3D())
			{

				rapidjson::Value point(rapidjson::kObjectType);
				point3D_t point3d_t = cp.first;
				point.AddMember("manual_tie_id", rapidjson::Value(point3d_t), allocator);


				Track track = cp.second.GetTrack();
				rapidjson::Value vec_observation(rapidjson::kArrayType);
				for (const auto& ele : track.GetElements())
				{
					rapidjson::Value observation(rapidjson::kObjectType);
					observation.AddMember("id", rapidjson::Value(ele.image_id), allocator);

					Image img;
					if (!atdata.GetImages().count(ele.image_id))
					{
						continue;
					}
					img = atdata.GetImage(ele.image_id);


					Eigen::Vector2d uv_pix = ele.xy;
					if (uv_pix.x() == -DBL_MAX)
					{
						continue;
					}
					rapidjson::Value uv(rapidjson::kArrayType);
					for (int j = 0; j < 2; j++)
					{
						uv.PushBack(uv_pix[j], allocator);
					}
					observation.AddMember("uv", uv, allocator);
					vec_observation.PushBack(observation, allocator);
				}

				point.AddMember("observations", vec_observation, allocator);


				points.PushBack(point, allocator);
			}
			documnet.AddMember("manual_tie_points", points, allocator);
			documnet.Accept(writer);

			if (!String::SaveFileFromString(file, buffer.GetString()))
			{
				LOGE("Save manual tie points Json Failed!");
				return AI3D_FAILURE;
			}
			return AI3D_SUCCESS;
		}

		
		int ATCommandSet::WriteGCPMeasurementsJson(const ATData& atdata, const std::string& file)
		{
			
			if (!atdata.HasControlPoints())
			{
				LOGI("no controlpoint to save");
				return false;
			}
			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
			rapidjson::Document documnet;
			documnet.SetObject();
			rapidjson::Document::AllocatorType& allocator = documnet.GetAllocator();

			rapidjson::Value coordinate(rapidjson::kObjectType);
			

			ControlPoints gcps;

			for (auto& it : atdata.GetControlPoints())
			{
				gcps.ADDPoint(it.second);

			}

			
			point3D_t basecoorgcpid = -1;
			for (auto& cp : gcps.GetPointsMutual())
			{
				if (AI3D::CORE::CoordinateDescriptor::IsGeode(cp.second.GetSrs().type))
				{
					basecoorgcpid = cp.second.GetId();
				}
			}


			srs_s srs = atdata.GetControlPoints().at(basecoorgcpid).GetSrs();


			std::string definition = srs.definition;

			
			gcps.TransformPointsToBaseCoordinate(definition);

			srs.CreateJson(coordinate, documnet);
	

			documnet.AddMember("coordinate_system", coordinate, allocator);


			rapidjson::Value points(rapidjson::kArrayType);

			for (auto& cp : gcps.GetPointsMutual())
			{
				ControlPoint Control_Point = cp.second;
				if (!Control_Point.GetObjectPointMutual().IsValid())
				{
					continue;
				}
				rapidjson::Value point(rapidjson::kObjectType);
				point3D_t point3d_t = cp.first;
				point.AddMember("id", rapidjson::Value(point3d_t), allocator);
				point.AddMember("name", rapidjson::Value(cp.second.GetName().c_str(), allocator), allocator);

				Eigen::Vector3d point3d = Control_Point.GetGivenXYZ();
				
				if (point3d.x() == -DBL_MAX || point3d.x() == DBL_MAX ||
					point3d.y() == -DBL_MAX || point3d.y() == DBL_MAX ||
					point3d.z() == -DBL_MAX || point3d.z() == DBL_MAX)
				{
					continue;
				}


				rapidjson::Value vec_point3d(rapidjson::kArrayType);

			
				if (srs.type == GEOGRAPHIC)
				{
					vec_point3d.PushBack(point3d[1], allocator);
					vec_point3d.PushBack(point3d[0], allocator);
					vec_point3d.PushBack(point3d[2], allocator);
				}
				else
				{
					vec_point3d.PushBack(point3d[0], allocator);
					vec_point3d.PushBack(point3d[1], allocator);
					vec_point3d.PushBack(point3d[2], allocator);
				}
				point.AddMember("coordinate", vec_point3d, allocator);

				Track track = Control_Point.GetObjectPoint().GetTrack();
				rapidjson::Value vec_observation(rapidjson::kArrayType);
				for (const auto& ele : track.GetElements())
				{
					rapidjson::Value observation(rapidjson::kObjectType);
					observation.AddMember("id", rapidjson::Value(ele.image_id), allocator);

					AI3D::CORE::Image img;
					img = atdata.GetImage(ele.image_id);

					Eigen::Vector2d uv_pix = img.GetPoints2DGCP(point3d_t);
					if (uv_pix.x() == -DBL_MAX)
					{
						continue;
					}
					rapidjson::Value uv(rapidjson::kArrayType);
					for (int j = 0; j < 2; j++)
					{
						uv.PushBack(uv_pix[j], allocator);
					}
					observation.AddMember("uv", uv, allocator);
					vec_observation.PushBack(observation, allocator);
				}

				point.AddMember("observations", vec_observation, allocator);
				int unsage = (Control_Point.GetType() == gpt_e::GCP_CONTROL_HV ? 0 : 1);
				if (point3d.z() == 0.0)
				{
					unsage = 2;
				}

				{
					point.AddMember("usage", unsage, allocator);
				}
				points.PushBack(point, allocator);
			}
			documnet.AddMember("points", points, allocator);
			documnet.Accept(writer);

			if (!String::SaveFileFromString(file, buffer.GetString()))
			{
				LOGE("Save GCP Json Failed!");
				return false;
			}
			return true;
			return AI3D_SUCCESS;
		}
		int ATCommandSet::WritePOSJson(const std::string& file, Eigen::Vector3d pos_sigma,
			const ATData& data, std::set<image_t> fixedids)
		{
			ATData ATdata = data;
			if (pos_sigma.x() < 0. && fixedids.empty())
			{
				return AI3D_FAILURE;
			}
			if (data.GetNumImages() == 0)
			{
				return AI3D_FAILURE;
			}
			std::cout << ATdata.HasAbsPriorPositionImages() << " " << fixedids.size() << std::endl;
			if ((pos_sigma.x() >= 0. && !ATdata.HasAbsPriorPositionImages()) && fixedids.empty())
			{
				return AI3D_FAILURE;
			}
			
			
			
			rapidjson::Document document;
			
			document.SetObject();
			rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
			
			if (pos_sigma.x() >= 0. && ATdata.HasAbsPriorPositionImages())
			{				
				
				rapidjson::Value coordinate(rapidjson::kObjectType);
				srs_s priorsrsjson = srs_s();
				auto& images = ATdata.GetImagesMutual();
				ControlPoints gcps;
				EIGEN_STL_UMAP(point3D_t, ControlPoint) controlpoints;
				for (const auto& iter : images)
				{
					if (iter.second.HasPositionPrior())
					{
						controlpoints[iter.first].SetId(iter.second.GetImageId());
						controlpoints[iter.first].GetGivenXYZMutual() = (iter.second.GetPositionPrior());
						controlpoints[iter.first].SetSrs(iter.second.GetPriorSrs());
					}
				}
				if (!controlpoints.empty())
				{
					gcps.SetPoints(controlpoints);
					if (gcps.TransformPointsToTheSrsOfOnepoint())
					{
						for (auto& iter : images)
						{
							if (gcps.GetPoints().count(iter.first))
							{
								(iter.second.GetPositionPriorMutual()) = gcps.GetPoints().at(iter.first).GetGivenXYZ();
							}
						}


						priorsrsjson = CoordinateDescriptor::GetSRSFromDefinition(gcps.GetSRS());
					}



					priorsrsjson.CreateJson(coordinate, document);




					rapidjson::Value items(rapidjson::kArrayType);
					for (const auto& img : ATdata.GetImages())
					{
						rapidjson::Value item(rapidjson::kObjectType);
						rapidjson::Value accuracy(rapidjson::kArrayType);
						rapidjson::Value pos(rapidjson::kArrayType);

						accuracy.PushBack(pos_sigma[0], allocator);
						accuracy.PushBack(pos_sigma[1], allocator);
						accuracy.PushBack(pos_sigma[2], allocator);

						pos.PushBack(img.second.GetPositionPrior()[0], allocator);
						pos.PushBack(img.second.GetPositionPrior()[1], allocator);
						pos.PushBack(img.second.GetPositionPrior()[2], allocator);

						item.AddMember("accuracy", accuracy, allocator);
						item.AddMember("id", rapidjson::Value(img.second.GetImageId()), allocator);
						item.AddMember("position", pos, allocator);

						items.PushBack(item, allocator);
					}

					document.AddMember("coordinate_system", coordinate, allocator);
					document.AddMember("items", items, allocator);
				}
			}
			if (!fixedids.empty())
			{
				rapidjson::Value itemsjson(rapidjson::kArrayType);
				for (auto& iter : fixedids)
				{
					itemsjson.PushBack(iter,allocator);
				}
				document.AddMember("fixed_image_id_set", itemsjson, allocator);
			}

			

			if (RapidJsonCore::SaveFile((file), document) != AI3D_SUCCESS)
			{
				LOG(ERROR) << "Save imageposlist.josn failed!";
				return AI3D_FAILURE;
			}
			
			return AI3D_SUCCESS;
		}
		int ATCommandSet::WritePOSBin(const std::string& file, Eigen::Vector3d pos_sigma,
			const ATData& data, std::set<image_t> fixedids)
		{
			std::ofstream out = File::OpenOfstreamUtf8(file, std::ios::binary);
			if (!out.is_open()) {
				LOGE("Writing POS file failed!");
				return AI3D_FAILURE;
			}
			POSFile posFile;

			ATData ATdata = data;
			if (pos_sigma.x() < 0. && fixedids.empty())
			{
				return AI3D_FAILURE;
			}
			if (data.GetNumImages() == 0)
			{
				return AI3D_FAILURE;
			}
			std::cout << ATdata.HasAbsPriorPositionImages() << " " << fixedids.size() << std::endl;
			if ((pos_sigma.x() >= 0. && !ATdata.HasAbsPriorPositionImages()) && fixedids.empty())
			{
				return AI3D_FAILURE;
			}

			
			
			if (pos_sigma.x() >= 0. && ATdata.HasAbsPriorPositionImages())
			{
				
				srs_s priorsrsjson = srs_s();
				auto& images = ATdata.GetImagesMutual();
				ControlPoints gcps;
				EIGEN_STL_UMAP(point3D_t, ControlPoint) controlpoints;
				for (const auto& iter : images)
				{
					if (iter.second.HasPositionPrior())
					{
						controlpoints[iter.first].SetId(iter.second.GetImageId());
						controlpoints[iter.first].GetGivenXYZMutual() = (iter.second.GetPositionPrior());
						controlpoints[iter.first].SetSrs(iter.second.GetPriorSrs());
					}
				}
				if (!controlpoints.empty())
				{
					gcps.SetPoints(controlpoints);
					if (gcps.TransformPointsToTheSrsOfOnepoint())
					{
						for (auto& iter : images)
						{
							if (gcps.GetPoints().count(iter.first))
							{
								(iter.second.GetPositionPriorMutual()) = gcps.GetPoints().at(iter.first).GetGivenXYZ();
							}
						}
						
						priorsrsjson = CoordinateDescriptor::GetSRSFromDefinition(gcps.GetSRS());
					}

					coord_system_type_e type = priorsrsjson.type;
					posFile.type = (int)type;

					if (type == coord_system_type_e::LOCAL_ENU)
					{
						std::string latlon_tmp = AI3D::CORE::String::StringSplit(priorsrsjson.definition, ":")[1];
						std::string lat = AI3D::CORE::String::StringSplit(latlon_tmp, ",")[0];
						std::string lon = AI3D::CORE::String::StringSplit(latlon_tmp, ",")[1];
						posFile.ori[0] = std::stod(lat);
						posFile.ori[1] = std::stod(lon);
						posFile.ori[2] = 0.0;

					}
					else if (type == coord_system_type_e::LOCAL)
					{

					}
					else
					{
						auto definitionstrs = AI3D::CORE::String::StringSplit(priorsrsjson.definition, ":");
						std::string codeflag = definitionstrs[0];
						std::string codestr = definitionstrs[1];
						AI3D::CORE::String::StringToLower(&codeflag);
						if (codeflag == "epsg")
						{
							posFile.espgCode = codestr;
						}
						else
						{
							OGRSpatialReference sr;
							if (OGRERR_NONE == sr.importFromWkt(priorsrsjson.definition.c_str()))
							{
								std::string codestr(sr.GetAuthorityCode(NULL));
								posFile.espgCode = codestr;
							}
						}
					}

					int posNum = ATdata.GetImages().size();
					
					posFile.posNum = posNum;
					posFile.posList.clear();
					for (const auto& img : ATdata.GetImages())
					{
						POSItem posItem;
						posItem.accuracy[0] = pos_sigma[0];
						posItem.accuracy[1] = pos_sigma[1];
						posItem.accuracy[2] = pos_sigma[2];

						posItem.position[0] = img.second.GetPositionPrior()[0];
						posItem.position[1] = img.second.GetPositionPrior()[1];
						posItem.position[2] = img.second.GetPositionPrior()[2];

						posItem.id = img.second.GetImageId();
						posFile.posList.push_back(posItem);
					}
				}
			}
			int fixSize = 0;
			posFile.fixNum = fixSize;
			posFile.fixIdList.clear();
			if (!fixedids.empty())
			{
				posFile.fixNum = fixedids.size();
				for (auto& iter : fixedids)
				{
					int id = (int)iter;
					posFile.fixIdList.push_back(id);
				}
			}

			posFile.Serialize(out);
			out.close();
			return AI3D_SUCCESS;
		}
		int ATCommandSet::WriteGCPMeasurementsXML(const ATData& data, const std::string& file)
		{
			return AI3D_SUCCESS;
		}


		int ATCommandSet::ReadUserTiepointsJson(ATData& data, const std::string& file)
		{
			return AI3D_SUCCESS;
		}
		int ATCommandSet::ReadGCPMeasurementsJson(ATData& data, const std::string& file)
		{
			return AI3D_SUCCESS;
		}
		int ATCommandSet::ReadPOSJson(ATData& data, const std::string& file)
		{
			return AI3D_SUCCESS;
		}
		int ATCommandSet::ReadPOSBin(ATData& data, const std::string& file)
		{
			return AI3D_SUCCESS;
		}
		int ATCommandSet::ReadGCPMeasurementsXML(ATData& data, const std::string& file)
		{
			return AI3D_SUCCESS;
		}



		bool ATCommandSet::LoadATBinary(const std::string& AT_filepath, std::shared_ptr<ATData>& ATdata)
		{
			std::ifstream in = File::OpenIfstreamUtf8(AT_filepath, std::ios::binary);
			
			if (!in.is_open())
			{
				LOGE(String::StringPrintf("Reading %s failed!", AT_filepath.c_str()));
				return false;
			}

			ATBinFile atBinFile;
			atBinFile.Deserialize(in);

			try
			{
				
				int version = atBinFile.version;
				std::string definition = atBinFile.definition;

				ATdata->SetLocalSrs(definition);
				std::string msg = "-----load=----- " + definition + " file " + AT_filepath + __FILE__ + " " + __FUNCTION__;
				msg += __LINE__;
				LOGI(msg);


				
				
				std::vector<PhotoGroup>pgs;
				int num_photogroups = atBinFile.num_photogroups;
				if (num_photogroups > std::numeric_limits<uint8_t>::max())
				{
					LOGW(String::StringPrintf("Invalid SCSFR.bin(%s),too many photogroups!", AT_filepath));
					return false;
				}
				std::set<image_t> image_ids;
				for (int pg_idx = 0; pg_idx < num_photogroups; pg_idx++)
				{
					ATCameraData atCameraData = atBinFile.photoGroups[pg_idx];
                    PhotoGroup pg;
                    Camera camera;
                    pg.SetName(atCameraData.cameraData.camera_name);
                    camera.SetCameraId(atCameraData.cameraData.id);
                    pg.SetId(atCameraData.cameraData.id);
                    camera.SetModelId(atCameraData.cameraData.cameraModelid);
                    camera.SetWidth(atCameraData.cameraData.width);
                    camera.SetHeight(atCameraData.cameraData.height);
                    CameraModelType_e cameramodeltype = (cameramodeltype_e)(atCameraData.cameraData.projection_model);
                    camera.SetCameraModelType(cameramodeltype);

                    std::vector<double>params;
                    
                    
                    
                    
                    
                    for (int i_param = 0; i_param < 12; i_param++)
                    {
                        params.push_back(atCameraData.cameraData.params[i_param]);
                    }
                    camera.SetParams(params);
					ATdata->AddCamera(camera);
					pg.SetCamera(camera);

					
					int num_images = atCameraData.num_images;
					for (int num_idx = 0; num_idx < num_images; num_idx++)
					{
						ImageData imageData = atCameraData.images[num_idx];
						AI3D::CORE::Image image;
						image_t image_id = imageData.image_id;
						image.SetCameraId(atCameraData.cameraData.id);
						image.SetPhotoGroupID(atCameraData.cameraData.id);
						image.SetImageId(image_id);
						image_ids.insert(image_id);
						bool isRegistered = imageData.isregis;
						image.SetRegistered(imageData.isregis);

						if (isRegistered)
						{
							ATdata->GetRegImageIdsMutual().emplace_back(image_id);
						}
						if (version <= -1010)
						{
							image.SetFixStatus(fix_e(imageData.status));
						}

						
						
						
						
						
						
						
						
						
						
						std::string path = imageData.path;
						std::string name = imageData.name;
#ifdef WIN32
						// path = UTF82GBK(path);
						// name = UTF82GBK(name);
#endif 
						image.SetPath(path);
						image.SetName(name);

						Eigen::Matrix3d R;
						for (int i = 0; i < 3; ++i) {
							for (int j = 0; j < 3; ++j) {
								R(i, j) = imageData.rotation[i][j];
							}
						}
						image.SetRotationMatrix(R);

						Eigen::Vector3d center;
						for (int i = 0; i < 3; ++i) {
							center[i] = imageData.center[i];
						}
						image.SetPosition(center);

						Eigen::Vector3d colorparam;
						for (int i = 0; i < 3; ++i) {
							colorparam[i] = imageData.color_param[i];
						}
						image.SetColorParam(colorparam);

						image.SetUp(camera);
						ATdata->AddImage(image);
						pg.AddImageId(image_id);
					}
					
					
					pgs.push_back(pg);
				}

				
				point3D_t num_tiepoints = atBinFile.num_tiepoints;;
				for (point3D_t i_pt = 0; i_pt < num_tiepoints; i_pt++)
				{
					PointItemData pointItemData = atBinFile.pointVec[i_pt];
					Point3D point3d;
					point3D_t index_point3d = pointItemData.index_point3d;

					
					Track track;
					Eigen::Vector3d xyz;
					for (int i = 0; i < 3; ++i) {
						xyz[i] = pointItemData.xyz[i];
					}
					Eigen::Vector3i rgb;
					for (int i = 0; i < 3; ++i) {
						rgb[i] = pointItemData.rgb[i];
					}
					
					int num_elements = pointItemData.num_elements;
					std::vector<TrackElement> vec_trackele;
					for (int i_ele = 0; i_ele < num_elements; i_ele++)
					{
						TrackItemData trackItemData = pointItemData.vec_trackele[i_ele];
						TrackElement trackelement;
						image_t image_id = trackItemData.image_id;
						Eigen::Vector2d uv;
						for (int i = 0; i < 2; ++i) {
							uv[i] = trackItemData.uv[i];
						}
						if (!image_ids.count(image_id))
						{
							LOGE("Invalid image id or image ids!");
							continue;
						}
						AI3D::CORE::Image& img = ATdata->GetImageMutual(image_id);
						trackelement.image_id = image_id;
						trackelement.point2D_idx = img.AddPoints2D(uv);
						img.SetPoint3DForPoint2D(trackelement.point2D_idx, index_point3d);
						trackelement.xy = uv;
						vec_trackele.push_back(trackelement);
					}
					track.AddElements(vec_trackele);
					point3d.SetId(index_point3d);
					point3d.SetTrack(track);
					point3d.SetColor(rgb);
					point3d.SetXYZ(xyz);

					ATdata->GetPoints3DMutual().insert(std::make_pair(index_point3d, point3d));

				}
				
				{
					uint64_t num_controlpoints = atBinFile.num_controlpoints;
					if (num_controlpoints > 0)
					{
						std::string gcpdefstr = atBinFile.gcpDefine;
						
						srs_s gcpsrs;
						
						gcpsrs = CoordinateDescriptor::GetSRSFromDefinition(gcpdefstr);

						
						
						EIGEN_STL_UMAP(point3D_t, class ControlPoint) cps;

						for (point3D_t gcp_idx = 0; gcp_idx < num_controlpoints; gcp_idx++)
						{
							GCPData gcpData = atBinFile.gcpVec[gcp_idx];
							ControlPoint cp;
							Track track;
							point3D_t point3d_id = gcpData.pointid;
							cp.SetId(point3d_id);
							cp.SetSrs(gcpsrs);
							Eigen::Vector3d cp_pos;
							for (int i = 0; i < 3; ++i) {
								cp_pos[i] = gcpData.cp_pos[i];
							}
							cp.SetGivenXYZ(cp_pos);
							Eigen::Vector2d weight;
							for (int i = 0; i < 2; ++i) {
								weight[i] = gcpData.weight[i];
							}
							cp.SetWeight(weight);
							cp.SetName(gcpData.name);
							gpt_e category = (gpt_e)(gcpData.category);
							cp.SetType(category);
							
							
							int num_eles = gcpData.num_eles;
							std::vector<TrackElement> vec_trackele;
							for (int ele_idx = 0; ele_idx < num_eles; ele_idx++)
							{
								GCPItem gcpItem = gcpData.elesVec[ele_idx];
								TrackElement trackelement;
								image_t image_id = kInvalidImageId;
								image_id = gcpItem.imageid;
								Eigen::Vector2d xy;
								for (int i = 0; i < 2; ++i) {
									xy[i] = gcpItem.xy[i];
								}
								if (!ATdata->GetImagesMutual().count(image_id))
								{
									LOGI("image id " + image_id);
									continue;
								}
								AI3D::CORE::Image& image = ATdata->GetImageMutual(image_id);
								image.SetPoints2DGCP(point3d_id, xy);
								trackelement.xy = xy;
								trackelement.image_id = image_id;
								trackelement.point2D_idx = point3d_id;
								vec_trackele.push_back(trackelement);
							}
							if (!vec_trackele.empty())
							{
								track.AddElements(vec_trackele);
								cp.GetObjectPointMutual().SetTrack(track);
							}
							cps.insert(std::make_pair(point3d_id, cp));
						}
						ATdata->SetControlPoints(cps);
					}
					

					EIGEN_STL_UMAP(point3D_t, class Point3D) userpts;
					point3D_t num_points = atBinFile.num_userpoints;
					for (point3D_t pt_idx = 0; pt_idx < num_points; pt_idx++)
					{
						Point3D cp;
						Track track;
						cp.SetId(point3D_t(pt_idx));
						
						int num_eles = atBinFile.usedPointVec[pt_idx].size();
						std::vector<TrackElement> vec_trackele;
						for (int ele_idx = 0; ele_idx < num_eles; ele_idx++)
						{
							UsedPointData usedPointData = atBinFile.usedPointVec[pt_idx][ele_idx];
							TrackElement trackelement;
							image_t image_id = usedPointData.imageid;
							Eigen::Vector2d xy;
							for (int i = 0; i < 2; ++i) {
								trackelement.xy[i] = usedPointData.xy[i];
							}
							trackelement.image_id = image_id;
							vec_trackele.push_back(trackelement);
						}
						if (!vec_trackele.empty())
						{
							track.AddElements(vec_trackele);
							cp.SetTrack(track);
						}
						userpts.insert(std::make_pair(pt_idx, cp));
					}
					ATdata->SetUserPoint3D(userpts);
				}

				in.close();
				
			}
			catch (const std::exception& err)
			{
				LOGE(err.what());
				in.close();
				return false;
			}
			return true;
		}
		bool ATCommandSet::ExportATBinary(const std::string& AT_filepath, ATData& data)
		{
			
			std::ofstream out = File::OpenOfstreamUtf8(AT_filepath, std::ios::binary);
			if (!out.is_open()) {
				LOGE(String::StringPrintf("Writing %s failed!", AT_filepath.c_str()));
				return false;
			}
			ATBinFile atBinFile;
			try
			{
				ATData* ATdata_tmp = new AI3D::CORE::ATData();
				*ATdata_tmp = data;

				std::string local_srs_def = ATdata_tmp->GetLocalSrs();

				if (ATdata_tmp->GetLocalSrs().empty())
				{
					local_srs_def = LOCALSRS;
				}

				
				int version = 0;
				atBinFile.version = version;

				
				atBinFile.definition = local_srs_def;
				std::string msg = "-----save=----- " + local_srs_def;
				LOGI(msg);

				
				std::map<camera_t, std::set<image_t>> groupimgids;
				for (auto& iter : ATdata_tmp->GetImages())
				{
					groupimgids[iter.second.GetCameraId()].insert(iter.second.GetImageId());

				}

				
				
				int num_photogroups = ATdata_tmp->GetCameras().size();
				atBinFile.num_photogroups = num_photogroups;
				atBinFile.photoGroups.clear();
				for (const auto& pg : ATdata_tmp->GetCameras())
				{
					ATCameraData atCameraData;
					CameraData cameraData;
					std::string image_name = pg.second.GetCameraName();
					int photogroupname_length = image_name.size();
					cameraData.camera_name = image_name;

					if (!ATdata_tmp->GetCameras().count(pg.second.GetCameraId()))
					{
						return false;
					}
					Camera camera = ATdata_tmp->GetCamera(pg.second.GetCameraId());
					cameraData.id = camera.GetCameraId();
					cameraData.cameraModelid = camera.GetModelId();
					cameraData.width = camera.GetWidth();
					cameraData.height = camera.GetHeight();

					CameraModelType_e cameramodeltype = camera.GetCameraModelType();
					cameraData.projection_model = (int)cameramodeltype;

					
					
					
					std::vector<double>params_0 = camera.GetParams();
					
					for (int p_idx = 0; p_idx < 12; p_idx++)
					{
						cameraData.params[p_idx] = params_0[p_idx];
					}
					atCameraData.cameraData = cameraData;
					
					int num_images = groupimgids.at(pg.second.GetCameraId()).size();
					if (!groupimgids.count(pg.second.GetCameraId()))
					{
						return false;
					}
					atCameraData.num_images = num_images;
					atCameraData.images.clear();
					for (const auto& img_id : groupimgids.at(pg.second.GetCameraId()))
					{
						ImageData imageData;
						if (!ATdata_tmp->GetImages().count(img_id))
						{
							return false;
						}
						AI3D::CORE::Image image = ATdata_tmp->GetImage(img_id);
						image_t image_id = image.GetImageId();
						imageData.image_id = image_id;
						imageData.isregis = image.IsRegistered();
						std::string path = image.GetPath();
						std::string name = image.GetName();
#ifdef WIN32
						// path = GBK2UTF8(path);
						// name = GBK2UTF8(name);
#endif 
						imageData.path = path;
						imageData.name = name;

						Eigen::Matrix3d R = image.GetRotationMatrix();
						imageData.hasRotaiton = true;
						for (int i = 0; i < 3; ++i) {
							for (int j = 0; j < 3; ++j) {
								imageData.rotation[i][j] = R(i, j);
							}
						}

						Eigen::Vector3d center = image.GetPosition();
						imageData.hasCenter = true;
						for (int j = 0; j < 3; ++j) {
							imageData.center[j] = center[j];
						}

						Eigen::Vector3d colorparam = image.GetColorParam();
						imageData.hasColorParam = true;
						for (int j = 0; j < 3; ++j) {
							imageData.color_param[j] = colorparam[j];
						}

						std::string tmep = image.GetName();
						String::StringToLower(&tmep);
						atCameraData.images.push_back(imageData);
					}
					atBinFile.photoGroups.push_back(atCameraData);
				}

				
				
				point3D_t num_tiepoints = ATdata_tmp->GetPoints3D().size();
				atBinFile.num_tiepoints = num_tiepoints;
				atBinFile.pointVec.clear();
				for (const auto& tp : ATdata_tmp->GetPoints3D())
				{
					PointItemData pointItemData;
					
					pointItemData.index_point3d = tp.first;
					Eigen::Vector3d xyz = tp.second.GetXYZ();
					for (int i = 0; i < 3; i++) {
						pointItemData.xyz[i] = xyz[i];
					}

					Eigen::Vector3i rgb = tp.second.GetColor();
					for (int i = 0; i < 3; i++) {
						pointItemData.rgb[i] = rgb[i];
					}

					
					int num_elements = tp.second.GetTrack().GetElements().size();
					pointItemData.num_elements = num_elements;
					pointItemData.vec_trackele.clear();
					for (const auto& ele : tp.second.GetTrack().GetElements())
					{
						TrackItemData trackItemData;
						image_t photo_id = ele.image_id;
						trackItemData.image_id = photo_id;
						Eigen::Vector2d xy = ele.xy;
						for (int i = 0; i < 2; i++) {
							trackItemData.uv[i] = xy[i];
						}
						pointItemData.vec_trackele.push_back(trackItemData);
					}
					atBinFile.pointVec.push_back(pointItemData);
				}
				
				{
					uint64_t num_controlpoints = ATdata_tmp->GetNumControlPoints();
					atBinFile.num_controlpoints = num_controlpoints;
					if (num_controlpoints > 0)
					{
						
						ControlPoints gcps;
						for (auto& it : ATdata_tmp->GetControlPoints())
						{
							gcps.ADDPoint(it.second);
						}
						
						point3D_t basecoorgcpid = -1;
						for (auto& cp : gcps.GetPointsMutual())
						{
							if (CoordinateDescriptor::IsGeode(cp.second.GetSrs().type))
							{
								basecoorgcpid = cp.second.GetId();
							}
						}
						srs_s srs = ATdata_tmp->GetControlPoints().at(basecoorgcpid).GetSrs();
						std::string definition = srs.definition;
						{
							gcps.TransformPointsToBaseCoordinate(definition);
						}
						atBinFile.gcpDefine = definition;

						atBinFile.gcpVec.clear();
						for (auto& cp_pair : gcps.GetPointsMutual())
						{
							GCPData gcpData;
							ControlPoint cp = cp_pair.second;
							
							gcpData.pointid = cp_pair.first;
							Eigen::Vector3d cp_pos = cp.GetGivenXYZ();
							for (int i = 0; i < 3; i++)
							{
								gcpData.cp_pos[i] = cp_pos[i];
							}
							Eigen::Vector2d weight = cp.GetWeight();
							for (int i = 0; i < 2; i++)
							{
								gcpData.weight[i] = weight[i];
							}
							
							
							gcpData.name = cp.GetName();							
							gpt_e category = cp.GetType();
							int icategory = int(category);
							gcpData.category = icategory;
							
							std::vector<TrackElement> elements = cp.GetObjectPoint().GetTrack().GetElements();
							int num_ele = elements.size();
							gcpData.num_eles = num_ele;
							gcpData.elesVec.clear();
							for (const auto& ele : elements)
							{
								GCPItem gcpItem;
								gcpItem.imageid = ele.image_id;
								Eigen::Vector2d xy = ele.xy;
								for (int i = 0; i < 2; i++)
								{
									gcpItem.xy[i] = xy[i];
								}
								gcpData.elesVec.push_back(gcpItem);
							}
							atBinFile.gcpVec.push_back(gcpData);
						}
					}
					

					{
						
						{
							point3D_t points_num = ATdata_tmp->GetNumUserPoints();
							atBinFile.num_userpoints = points_num;
							atBinFile.usedPointVec.clear();
							if (points_num > 0)
							{
								for (const auto& cp : ATdata_tmp->GetUserPoints3D())
								{
									std::vector<UsedPointData> level2;
									
									std::vector<TrackElement> elements = cp.second.GetTrack().GetElements();
									for (const auto& ele : elements)
									{
										UsedPointData usedPointData;
										usedPointData.imageid = ele.image_id;
										Eigen::Vector2d xy = ele.xy;
										for (int i = 0; i < 2; i++)
										{
											usedPointData.xy[i] = xy[i];
										}
										level2.push_back(usedPointData);
									}
									atBinFile.usedPointVec.push_back(level2);
								}
							}
						}
					}

				}

				atBinFile.Serialize(out);
				out.close();
			}
			catch (const std::exception& err)
			{
				LOGE(String::StringPrintf("Saving: %s failed! Msg: %s", AT_filepath.c_str(), err.what()));
				out.close();
				return false;
			}
			return true;
		}
		bool ATCommandSet::SaveSourceDataJson1(ATData& Atdata, const std::string file_path, const ATOptions& atoptions, Eigen::Vector3d possigma)
		{
			

			
			rapidjson::Document document;
			document.SetObject();
			rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

			rapidjson::Value coordinatejson(rapidjson::kObjectType);
			rapidjson::Value camerasjson(rapidjson::kArrayType);
			rapidjson::Value imagesjson(rapidjson::kArrayType);
			auto atdatasrs = CoordinateDescriptor::GetSRSFromDefinition(Atdata.GetLocalSrs());
			
			atdatasrs.CreateJson(coordinatejson, document);

			bool withpos = possigma.x() >= 0.;

			{
				auto cameras = Atdata.GetCameras();
				for (auto& camiter : cameras)
				{

					Camera cam = camiter.second;
					
					if (cam.GetFocalLengthX() <= 0)
					{
						
						double f_35eq = Application::Getinstance().ParseConfig().focal_length;
						LOGW(String::StringPrintf("no focal we set f_35eq = %f", f_35eq));
						double focal_pix = std::max(cam.GetWidth(), cam.GetHeight()) * f_35eq / 36;
						cam.SetFocalLengthX(focal_pix);
						cam.SetFocalLengthY(focal_pix);
						cam.SetPrincipalPointX(cam.GetWidth() / 2);
						cam.SetPrincipalPointY(cam.GetHeight() / 2);

					}
					rapidjson::Value dst(rapidjson::kObjectType);

					
					dst.AddMember("id", rapidjson::Value(cam.GetCameraId()), allocator);

					rapidjson::Value item(rapidjson::kObjectType);
					item.AddMember("camera_name", rapidjson::Value(camiter.second.GetCameraName().c_str(), allocator), allocator);
					double width, height;
					width = height = -DBL_MAX;
					width = cam.GetWidth();
					height = cam.GetHeight();
					if (width == -DBL_MAX || height == -DBL_MAX)
					{
						LOGE("camera has no width or height!");
						return false;
					}
					item.AddMember("width", rapidjson::Value(int(width)), allocator);
					item.AddMember("height", rapidjson::Value(int(height)), allocator);

					
					
					item.AddMember("projection_model", rapidjson::Value(0), allocator);
					rapidjson::Value parameters(rapidjson::kArrayType);

					
					for (int i = 0; i < 4; i++)
					{
						parameters.PushBack(cam.GetParams()[i], allocator);
					}
					parameters.PushBack(cam.GetParams()[4], allocator);
					parameters.PushBack(cam.GetParams()[5], allocator);
					parameters.PushBack(cam.GetParams()[8], allocator);
					parameters.PushBack(cam.GetParams()[7], allocator);
					parameters.PushBack(cam.GetParams()[6], allocator);
					parameters.PushBack(0, allocator);
					item.AddMember("parameters", parameters, allocator);
					
					dst.AddMember("meta_data", item, allocator);

					camerasjson.PushBack(dst, allocator);
				}
			}

			
			{
				auto cameras = Atdata.GetCameras();
				for (auto& img : Atdata.GetImages())
				{
					
					std::string imagefullpath = File::EnsureUnifySlash(img.second.GetPath() + PATH_SEPARATOR_STR + img.second.GetName());

					

					
					
					
					

					rapidjson::Value dst(rapidjson::kObjectType);

					
					dst.AddMember("id", rapidjson::Value(img.first), allocator);

					
					rapidjson::Value item(rapidjson::kObjectType);
					long long timestamp;
					std::string TimeOrigin = img.second.GetExifinfo().dateTime;
					std::vector<std::string> time_vec = String::StringSplit(TimeOrigin, "- T :");
					if (!TimeOrigin.empty())
					{
						TimeOrigin.clear();
						uint8_t i_time = 0;
						while (i_time < 6)
						{
							TimeOrigin += time_vec[i_time++];
						}
						timestamp = std::atoll(TimeOrigin.c_str());
					}
					else
					{
						timestamp = 0;
					}

					item.AddMember("camera_id", rapidjson::Value(img.second.GetCameraId()), allocator);
					item.AddMember("capture_time", rapidjson::Value(timestamp), allocator);
					item.AddMember("dewrap_flag", rapidjson::Value(img.second.GetDewrapFlag()), allocator);
					item.AddMember("width", rapidjson::Value(int(img.second.GetWidth())), allocator);
					item.AddMember("height", rapidjson::Value(int(img.second.GetHeight())), allocator);
					float f_35eq = 35;
					Camera camera;
					if (cameras.find(img.second.GetCameraId()) != cameras.end())
					{
						camera = cameras[img.second.GetCameraId()];
						if (camera.GetHeight() != img.second.GetHeight() || camera.GetWidth() != img.second.GetWidth())

						{
							LOGE("The width or height of image is not equal to camera!");
							return false;
						}
					}
					else
					{
						LOGE("Image has invalid camera_id in writing SourceData.json!");
						return false;
					}
					auto config = Application::Getinstance().ParseConfig();
					
					
					if (withpos)
					{
						

						
						if (img.second.HasRotationMatrix())
						{
							rapidjson::Value rotation(rapidjson::kArrayType);
							Eigen::Matrix3d R = img.second.GetRotationMatrix();
							for (int i = 0; i < 3; i++)
							{
								for (int j = 0; j < 3; j++)
								{
									rotation.PushBack(R(i, j), allocator);
								}
							}
							item.AddMember("orientation", rotation, allocator);
						}

						

						


						if (img.second.HasPosition())
						{
							rapidjson::Value pos(rapidjson::kArrayType);

							pos.PushBack(img.second.GetPosition()[0], allocator);
							pos.PushBack(img.second.GetPosition()[1], allocator);
							pos.PushBack(img.second.GetPosition()[2], allocator);

							{
								item.AddMember("pos", pos, allocator);

								if (withpos)
								{
									rapidjson::Value pos_sigma(rapidjson::kArrayType);
									for (int i = 0; i < 3; i++)
									{
										pos_sigma.PushBack(possigma[i], allocator);
									}
									item.AddMember("pos_sigma", pos_sigma, allocator);
								}
							}
						}
					}
					dst.AddMember("meta_data", item, allocator);


					// std::string imagefullpath2 = GBK2UTF8(imagefullpath);
					std::string imagefullpath2 = imagefullpath;
					dst.AddMember("path", rapidjson::Value(imagefullpath2.c_str(), allocator), allocator);

					imagesjson.PushBack(dst, allocator);
				}
			}

			document.AddMember("coordinate_system", coordinatejson, allocator);
			document.AddMember("camera_meta_data", camerasjson, allocator);
			document.AddMember("image_meta_data", imagesjson, allocator);

			

			if (RapidJsonCore::SaveFile(file_path, document) != AI3D_SUCCESS)
			{
				LOGE("Save sourcedata.json failed!");
				return false;
			}
			return true;
		}

		bool ATCommandSet::SaveSourceDataBinary(ATData& Atdata, const std::string file_path, Eigen::Vector3d possigma) {
			
			std::ofstream out = File::OpenOfstreamUtf8(file_path, std::ios::binary);
			
			if (!out.is_open()) {
				LOGE("Save inputdata bin failed!");
				return false;
			}
			OriBinFile oriBinFile;

			
			auto atdatasrs = CoordinateDescriptor::GetSRSFromDefinition(Atdata.GetLocalSrs());
			if (1)
			{
				auto srs = Atdata.GetDefaultEnuSRS();

				std::string src_srs_definition = Atdata.GetLocalSrs();
				Atdata.TransFormImages(src_srs_definition, srs.definition);
				atdatasrs = srs;
			}
			CoordinateData coordinateData;
			coord_system_type_e type = (coord_system_type_e)atdatasrs.type;
			
			coordinateData.type = atdatasrs.type;
			if (type == coord_system_type_e::LOCAL_ENU)
			{
				std::string latlon_tmp = AI3D::CORE::String::StringSplit(atdatasrs.definition, ":")[1];
				std::string lat = AI3D::CORE::String::StringSplit(latlon_tmp, ",")[0];
				std::string lon = AI3D::CORE::String::StringSplit(latlon_tmp, ",")[1];
				coordinateData.ori[0] = std::atof(lat.c_str());
				coordinateData.ori[1] = std::atof(lon.c_str());
				coordinateData.ori[2] = 0.0;
			}
			else if (type == coord_system_type_e::LOCAL)
			{

			}
			else
			{
				auto definitionstrs = AI3D::CORE::String::StringSplit(atdatasrs.definition, ":");
				std::string codeflag = definitionstrs[0];
				std::string codestr = definitionstrs[1];
				AI3D::CORE::String::StringToLower(&codeflag);
				if (codeflag == "epsg")
				{
					coordinateData.espgStr = codestr;
				}
				else
				{
					OGRSpatialReference sr;
					if (OGRERR_NONE == sr.importFromWkt(atdatasrs.definition.c_str()))
					{
						std::string codestr(sr.GetAuthorityCode(NULL));
						coordinateData.espgStr = codestr;
					}

				}

			}
			oriBinFile.coordinateData = coordinateData;

			bool withpos = possigma.x() >= 0.;
			
			auto cameras = Atdata.GetCameras();
			unsigned int camera_count = cameras.size();
			oriBinFile.cameras_size = camera_count;

			for (auto& camiter : cameras) {
				Camera cam = camiter.second;

				CameraData cameraData;
				
				if (cam.GetFocalLengthX() <= 0)
				{
					
					double f_35eq = Application::Getinstance().ParseConfig().focal_length;
					LOGW(String::StringPrintf("no focal we set f_35eq = %f", f_35eq));
					double focal_pix = std::max(cam.GetWidth(), cam.GetHeight()) * f_35eq / 36;
					cam.SetFocalLengthX(focal_pix);
					cam.SetFocalLengthY(focal_pix);
					cam.SetPrincipalPointX(cam.GetWidth() / 2);
					cam.SetPrincipalPointY(cam.GetHeight() / 2);

				}

				
				cameraData.id = cam.GetCameraId();
				cameraData.camera_name = cam.GetCameraName();
				double width, height;
				width = height = -DBL_MAX;
				width = cam.GetWidth();
				height = cam.GetHeight();
				if (width == -DBL_MAX || height == -DBL_MAX)
				{
					LOGE("camera has no width or height!");
					return false;
				}
				cameraData.width = int(width);
				cameraData.height = int(height);

				
				
				cameraData.projection_model = 0;

				
				for (int i = 0; i < 4; i++)
				{
					cameraData.params[i] = cam.GetParams()[i];
				}
				cameraData.params[4] = cam.GetParams()[4];
				cameraData.params[5] = cam.GetParams()[5];
				cameraData.params[8] = cam.GetParams()[8];
				cameraData.params[7] = cam.GetParams()[7];
				cameraData.params[6] = cam.GetParams()[6];
				cameraData.params[9] = 0.0;

				oriBinFile.cameraDataVec.push_back(cameraData);
			}

			
			auto images = Atdata.GetImages();
			unsigned int image_count = images.size();
			oriBinFile.images_size = image_count;

			for (const auto& img : images) {
				const AI3D::CORE::Image& image = img.second;
				ImageData imageData;
				
				std::string imagefullpath = File::EnsureUnifySlash(img.second.GetPath() + PATH_SEPARATOR_STR + img.second.GetName());

				

				
				
				
				
				imageData.image_id = img.first;

				
				long long timestamp;
				std::string TimeOrigin = img.second.GetExifinfo().dateTime;
				std::vector<std::string> time_vec = String::StringSplit(TimeOrigin, "- T :");
				if (!TimeOrigin.empty())
				{
					TimeOrigin.clear();
					uint8_t i_time = 0;
					while (i_time < 6)
					{
						TimeOrigin += time_vec[i_time++];
					}
					timestamp = std::atoll(TimeOrigin.c_str());
				}
				else
				{
					timestamp = 0;
				}

				imageData.camera_id = img.second.GetCameraId();
				imageData.time = timestamp;
				imageData.dewarp_flag = img.second.GetDewrapFlag();
				imageData.width = int(img.second.GetWidth());
				imageData.height = int(img.second.GetHeight());

				float f_35eq = 0;
				Camera camera;
				if (cameras.find(img.second.GetCameraId()) != cameras.end())
				{
					camera = cameras[img.second.GetCameraId()];
					if (camera.GetHeight() != img.second.GetHeight() || camera.GetWidth() != img.second.GetWidth())

					{
						LOGE("The width or height of image is not equal to camera!");
						return false;
					}
				}
				else
				{
					LOGE("Image has invalid camera_id in writing SourceData.json!");
					return false;
				}
				auto config = Application::Getinstance().ParseConfig();
				if (camera.GetFocalLengthIn35mm() == 0)
				{
					f_35eq = config.focal_length;
				}
				else
				{
					f_35eq = camera.GetFocalLengthIn35mm();
				}
				
				imageData.hasRotaiton = img.second.HasRotationMatrix();
				imageData.hasPosition = img.second.HasPosition();
				imageData.hasPosSigma = img.second.HasPosition();
				if (withpos)
				{
					

					
					
					if (img.second.HasRotationMatrix())
					{
						Eigen::Matrix3d R = img.second.GetRotationMatrix();
						for (int i = 0; i < 3; i++)
						{
							for (int j = 0; j < 3; j++)
							{
								imageData.rotation[i][j] = R(i, j);
							}
						}
					}

					

					


					if (img.second.HasPosition())
					{
						imageData.position[0] = img.second.GetPosition()[0];
						imageData.position[1] = img.second.GetPosition()[1];
						imageData.position[2] = img.second.GetPosition()[2];



						if (withpos)
						{
							for (int i = 0; i < 3; i++)
							{
								imageData.pos_sigma[i] = possigma[i];
							}
						}
					}
				}


				
				std::string tmpPath = img.second.GetPath();
				std::string tmpName = img.second.GetName();
#ifdef WIN32
				// tmpPath = GBK2UTF8(tmpPath);
				// tmpName = GBK2UTF8(tmpName);
#endif 
				imageData.path = tmpPath;
				imageData.name = tmpName;

				oriBinFile.imageDataVec.push_back(imageData);
			}
			oriBinFile.Serialize(out);

			out.close();
			
			
			return true;
		}

		
		
		bool ATCommandSet::SaveSourceDataJson(ATData& Atdata, const std::string file_path,  Eigen::Vector3d possigma)
		{
			

			rapidjson::StringBuffer buffer;
		
			rapidjson::Document document;
			document.SetObject();
			rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

			rapidjson::Value coordinatejson(rapidjson::kObjectType);
			rapidjson::Value camerasjson(rapidjson::kArrayType);
			rapidjson::Value imagesjson(rapidjson::kArrayType);
			auto atdatasrs = CoordinateDescriptor::GetSRSFromDefinition(Atdata.GetLocalSrs());
			if(1)
			{
				auto srs = Atdata.GetDefaultEnuSRS();

				std::string src_srs_definition = Atdata.GetLocalSrs();
				Atdata.TransFormImages(src_srs_definition, srs.definition);
				atdatasrs = srs;
			}
			atdatasrs.CreateJson(coordinatejson, document);
			
			bool withpos = possigma.x() >= 0.;
									
			{
				auto cameras = Atdata.GetCameras();
				for (auto& camiter : cameras)
				{

					Camera cam = camiter.second;
					
					if (cam.GetFocalLengthX() <= 0)
					{
						
						double f_35eq = Application::Getinstance().ParseConfig().focal_length;
						LOGW(String::StringPrintf("no focal we set f_35eq = %f", f_35eq));
						double focal_pix = std::max(cam.GetWidth(), cam.GetHeight()) * f_35eq / 36;
						cam.SetFocalLengthX(focal_pix);
						cam.SetFocalLengthY(focal_pix);
						cam.SetPrincipalPointX(cam.GetWidth() / 2);
						cam.SetPrincipalPointY(cam.GetHeight() / 2);

					}
					rapidjson::Value dst(rapidjson::kObjectType);

					
					dst.AddMember("id", rapidjson::Value(cam.GetCameraId()), allocator);

					rapidjson::Value item(rapidjson::kObjectType);
					item.AddMember("camera_name", rapidjson::Value(camiter.second.GetCameraName().c_str(), allocator), allocator);
					double width, height;
					width = height = -DBL_MAX;
					width = cam.GetWidth();
					height = cam.GetHeight();
					if (width == -DBL_MAX || height == -DBL_MAX)
					{
						LOGE("camera has no width or height!");
						return false;
					}
					item.AddMember("width", rapidjson::Value(int(width)), allocator);
					item.AddMember("height", rapidjson::Value(int(height)), allocator);

					
					
					item.AddMember("projection_model", rapidjson::Value(0), allocator);
					rapidjson::Value parameters(rapidjson::kArrayType);

					
					for (int i = 0; i < 4; i++)
					{
						parameters.PushBack(cam.GetParams()[i], allocator);
					}
					parameters.PushBack(cam.GetParams()[4], allocator);
					parameters.PushBack(cam.GetParams()[5], allocator);
					parameters.PushBack(cam.GetParams()[8], allocator);
					parameters.PushBack(cam.GetParams()[7], allocator);
					parameters.PushBack(cam.GetParams()[6], allocator);
					parameters.PushBack(0, allocator);
					item.AddMember("parameters", parameters, allocator);
					
					dst.AddMember("meta_data", item, allocator);

					camerasjson.PushBack(dst, allocator);
				}
			}

			
			{
				auto cameras = Atdata.GetCameras();
				for (auto& img : Atdata.GetImages()) 
				{
					
					std::string imagefullpath = File::EnsureUnifySlash(img.second.GetPath() + PATH_SEPARATOR_STR + img.second.GetName());

					

					
					
					
					

					rapidjson::Value dst(rapidjson::kObjectType);

					
					dst.AddMember("id", rapidjson::Value(img.first), allocator);

					
					rapidjson::Value item(rapidjson::kObjectType);
					long long timestamp;
					std::string TimeOrigin = img.second.GetExifinfo().dateTime;
					std::vector<std::string> time_vec = String::StringSplit(TimeOrigin, "- T :");
					if (!TimeOrigin.empty())
					{
						TimeOrigin.clear();
						uint8_t i_time = 0;
						while (i_time < 6)
						{
							TimeOrigin += time_vec[i_time++];
						}
						timestamp = std::atoll(TimeOrigin.c_str());
					}
					else
					{
						timestamp = 0;
					}

					item.AddMember("camera_id", rapidjson::Value(img.second.GetCameraId()), allocator);
					item.AddMember("capture_time", rapidjson::Value(timestamp), allocator);
					item.AddMember("dewrap_flag", rapidjson::Value(img.second.GetDewrapFlag()), allocator);
					item.AddMember("width", rapidjson::Value(int(img.second.GetWidth())), allocator);
					item.AddMember("height", rapidjson::Value(int(img.second.GetHeight())), allocator);
					float f_35eq = 0;
					Camera camera;
					if (cameras.find(img.second.GetCameraId()) != cameras.end())
					{
						camera = cameras[img.second.GetCameraId()];
						if (camera.GetHeight() != img.second.GetHeight() || camera.GetWidth() != img.second.GetWidth())

						{
							LOGE("The width or height of image is not equal to camera!");
							return false;
						}
					}
					else
					{
						LOGE("Image has invalid camera_id in writing SourceData.json!");
						return false;
					}
					auto config = Application::Getinstance().ParseConfig();
					if (camera.GetFocalLengthIn35mm() == 0)
					{
						f_35eq = config.focal_length;
					}
					else
					{
						f_35eq = camera.GetFocalLengthIn35mm();
					}
				
					if (withpos)
					{
						

						
						if (img.second.HasRotationMatrix())
						{
							rapidjson::Value rotation(rapidjson::kArrayType);
							Eigen::Matrix3d R = img.second.GetRotationMatrix();
							for (int i = 0; i < 3; i++)
							{
								for (int j = 0; j < 3; j++)
								{
									rotation.PushBack(R(i, j), allocator);
								}
							}
							item.AddMember("orientation", rotation, allocator);
						}

						

						


						if (img.second.HasPosition())
						{
							rapidjson::Value pos(rapidjson::kArrayType);

							pos.PushBack(img.second.GetPosition()[0], allocator);
							pos.PushBack(img.second.GetPosition()[1], allocator);
							pos.PushBack(img.second.GetPosition()[2], allocator);

							{
								item.AddMember("pos", pos, allocator);

								if(withpos)
								{
									rapidjson::Value pos_sigma(rapidjson::kArrayType);
									for (int i = 0; i < 3; i++)
									{
										pos_sigma.PushBack(possigma[i], allocator);
									}
									item.AddMember("pos_sigma", pos_sigma, allocator);
								}
							}
						}
					}
					dst.AddMember("meta_data", item, allocator);


					// std::string imagefullpath2 = GBK2UTF8(imagefullpath);
					std::string imagefullpath2 = imagefullpath;
					dst.AddMember("path", rapidjson::Value(imagefullpath2.c_str(), allocator), allocator);

					imagesjson.PushBack(dst, allocator);
				}
			}

			document.AddMember("coordinate_system", coordinatejson, allocator);
			document.AddMember("camera_meta_data", camerasjson, allocator);
			document.AddMember("image_meta_data", imagesjson, allocator);

			

			if (RapidJsonCore::SaveFile(file_path, document) != AI3D_SUCCESS)
			{
				LOGE("Save sourcedata.json failed!");
				return false;
			}
			return true;
		}

		bool ATCommandSet::LoadSourceDataJson1(ATData& Atdata, const std::string file_path)
		{
			
			
			std::string blkcontent;
			std::ifstream in = File::OpenIfstreamUtf8(file_path, std::ios::in);
			if (!in.is_open())
				return false;
			std::string line;
			std::string content;
			while (std::getline(in, line))
			{

				if (line[line.size() - 1] != '\n')
					line.append("\n");

				content.append(line);
			}
			in.close();
			rapidjson::Document doc;
			if (doc.Parse(content.data()).HasParseError())
			{
				return false;
			}
			if (!doc.IsObject())
			{
				return false;
			}
			srs_s srs;
			if (doc.HasMember("coordinate_system"))
			{
				srs.ParseJson(doc["coordinate_system"]);
			}
			auto& images = Atdata.GetImagesMutual();
			images.clear();
			auto& cameras = Atdata.GetCamerasMutual();
			cameras.clear();
			Atdata.SetLocalSrs(srs.definition);




			if (doc.HasMember("camera_meta_data"))
			{
				rapidjson::Value& cams_jstr = doc["camera_meta_data"];
				for (unsigned i = 0; i < cams_jstr.Size(); i++)
				{
					camera_t camera_id;
					if (cams_jstr[i].HasMember("id"))
					{
						camera_id = cams_jstr[i]["id"].GetInt();
					}
					{
						std::string camera_name = "";
						std::string CameraOrientation = "XRightYDown";
						int  cameramodeltype = 0;

						int image_width = kInvalideNum;
						int image_height = kInvalideNum;

						double focal_length_pixelx = kInvalideNum;
						double focal_length_pixely = kInvalideNum;
						double sensor_size = kInvalideNum;
						double focal_lengthin35mm = kInvalideNum;
						double cx, cy;
						double k1, k2, k3, k4, k5, k6, p1, p2, p3;
						cx = cy = k1 = k2 = k3 = k4 = k5 = k6 = p1 = p2 = p3 = 0.0;

						Camera  camera;

						if (cams_jstr[i].HasMember("meta_data"))
						{
							rapidjson::Value& cam_jstr = cams_jstr[i]["meta_data"];
							camera_name = cam_jstr["camera_name"].GetString();

							image_width = cam_jstr["width"].GetInt();
							image_height = cam_jstr["height"].GetInt();
							cameramodeltype = cam_jstr["projection_model"].GetInt();
							if (cam_jstr.HasMember("parameters"))
							{
								rapidjson::Value& param_jstr = cam_jstr["parameters"];
								std::vector<double> params(10);
								for (int i = 0; i < 10; ++i)
								{
									params[i] = param_jstr[i].GetDouble();
								}
								focal_length_pixelx = (params[0]);
								focal_length_pixely = (params[1]);
								cx = (params[2]);
								cy = (params[3]);
								k1 = params[4];
								k2 = params[5];
								p2 = params[6];
								p1 = params[7];
								k3 = params[8];
								p3 = params[9];
							}

							camera.SetModelIdFromName("FULL_OPENCV");

							{
								
								camera.SetParams({ focal_length_pixelx,focal_length_pixely,cx,cy,k1,k2,p1,p2,k3, k4, k5, k6 });
							}


							camera.SetWidth(image_width);
							camera.SetHeight(image_height);
							camera.SetCameraId(camera_id);
							camera.SetCameraModelType(CameraModelType_e(cameramodeltype));

							camera.SetCameraOrientation(CameraOrientation);


						}
						cameras[camera_id] = camera;
					}
				}
			}

			if (doc.HasMember("image_meta_data"))
			{
				rapidjson::Value& img_jstr = doc["image_meta_data"];
				for (unsigned i = 0; i < img_jstr.Size(); i++)
				{

					AI3D::CORE::Image image;
					
					int photo_id = -1;
					camera_t cam_id;
					int component = -1;
					bool dewrap_flag;
					fix_e fixedstatus = fix_e::EOE_FREE;
					std::string image_path = "";
					std::string image_name = "";
					int image_width, image_height;

					if (img_jstr[i].HasMember("id"))
					{
						photo_id = img_jstr[i]["id"].GetInt();
					}
					else
						return false;
					if (img_jstr[i].HasMember("meta_data"))
					{
						rapidjson::Value& it = img_jstr[i]["meta_data"];
						if (it.HasMember("camera_id"))
						{
							cam_id = it["camera_id"].GetInt();
						}
						if (it.HasMember("dewrap_flag"))
						{
							dewrap_flag = it["dewrap_flag"].GetBool();
						}
						if (it.HasMember("width"))
						{
							image_width = it["width"].GetInt();
						}
						if (it.HasMember("height"))
						{
							image_height = it["height"].GetInt();
						}
						if (it.HasMember("pos"))
						{
							auto itarr = it["pos"].GetArray();

							Eigen::Vector3d xyz(itarr[0].GetDouble(), itarr[1].GetDouble(), itarr[2].GetDouble());
							image.SetPosition(xyz);
							
						}
						if (it.HasMember("orientation"))
						{
							auto itarr = it["orientation"].GetArray();
							Eigen::Matrix3d rot;
							for (int i = 0; i < 3; ++i)
								for (int j = 0; j < 3; ++j)
									 rot(i, j)= it[i * 3 + j].GetDouble();
							image.SetRotationMatrix(rot);
						}
						
					}

					if (img_jstr[i].HasMember("path"))
					{
						image_path = img_jstr[i]["path"].GetString();
					}
					image_name = File::BoostPathToUtf8String(File::BoostPathFromUtf8(image_path).filename());
					image_path = File::BoostPathToUtf8String(File::BoostPathFromUtf8(image_path).parent_path());


					image_path = File::EnsureUnifySlash(image_path);
#ifdef _MSC_VER
					image_path = String::StringReplace(image_path, PATH_SEPARATOR_STR, REVERSE_PATH_SEPARATOR_STR);
#endif 


					image.SetHeight(image_height);
					image.SetWidth(image_width);
					image.SetPhotoGroupID(cam_id);
					image.SetCameraId(cam_id);


					image.SetImageId(photo_id);
					
					image.SetPath(image_path);
					image.SetName(image_name);

					images[photo_id] = image;
				}

			}
			return true;
		}

		bool ATCommandSet::LoadSourceDataBinary(ATData& Atdata, const std::string file_path) {
			

			
			std::ifstream in = File::OpenIfstreamUtf8(file_path, std::ios::binary);
			
			if (!in.is_open())
				return false;

			OriBinFile oriBinFile;
			oriBinFile.Deserialize(in);

			
			srs_s srs;

			coord_system_type_e type = (coord_system_type_e)oriBinFile.coordinateData.type;
			srs.type = (coord_system_type_e)oriBinFile.coordinateData.type;

			if (type == coord_system_type_e::LOCAL_ENU)
			{
				double lat = oriBinFile.coordinateData.ori[0];
				double lon = oriBinFile.coordinateData.ori[1];
				double alt = oriBinFile.coordinateData.ori[2];
				srs.definition = "ENU:" + std::to_string(lat) + "," + std::to_string(lon);

			}
			else if (type == coord_system_type_e::GEOGRAPHIC || type == coord_system_type_e::PROJECTION || type == coord_system_type_e::GEOCENTRIC)
			{
				srs.definition = oriBinFile.coordinateData.espgStr;

			}

			Atdata.SetLocalSrs(srs.definition);

			
			auto& cameras = Atdata.GetCamerasMutual();
			cameras.clear();
			
			for (int i = 0; i < oriBinFile.cameras_size; ++i)
			{
				

				std::string camera_name = "";
				std::string CameraOrientation = "XRightYDown";
				int  cameramodeltype = 0;

				int image_width = kInvalideNum;
				int image_height = kInvalideNum;

				double focal_length_pixelx = kInvalideNum;
				double focal_length_pixely = kInvalideNum;
				double sensor_size = kInvalideNum;
				double focal_lengthin35mm = kInvalideNum;
				double cx, cy;
				double k1, k2, k3, k4, k5, k6, p1, p2, p3;
				cx = cy = k1 = k2 = k3 = k4 = k5 = k6 = p1 = p2 = p3 = 0.0;
				Camera camera;

				camera_t camera_id = oriBinFile.cameraDataVec[i].id;

				
				camera_name = oriBinFile.cameraDataVec[i].camera_name;

				cameramodeltype = oriBinFile.cameraDataVec[i].projection_model;

				
				image_width = oriBinFile.cameraDataVec[i].width;
				image_height = oriBinFile.cameraDataVec[i].height;

				
				focal_length_pixelx = oriBinFile.cameraDataVec[i].params[0];
				focal_length_pixely = oriBinFile.cameraDataVec[i].params[1];
				cx = oriBinFile.cameraDataVec[i].params[2];
				cy = oriBinFile.cameraDataVec[i].params[3];
				k1 = oriBinFile.cameraDataVec[i].params[4];
				k2 = oriBinFile.cameraDataVec[i].params[5];
				p2 = oriBinFile.cameraDataVec[i].params[6];
				p1 = oriBinFile.cameraDataVec[i].params[7];
				k3 = oriBinFile.cameraDataVec[i].params[8];
				p3 = oriBinFile.cameraDataVec[i].params[9];

				
				
				camera.SetModelIdFromName("FULL_OPENCV");
				camera.SetParams({ focal_length_pixelx,focal_length_pixely,cx,cy,k1,k2,p1,p2,k3, k4, k5, k6 });  
				camera.SetWidth(image_width);
				camera.SetHeight(image_height);
				camera.SetCameraId(camera_id);
				camera.SetCameraModelType(CameraModelType_e(cameramodeltype));
				camera.SetCameraOrientation(CameraOrientation);

				
				cameras[camera_id] = camera;
			}

			
			auto& images = Atdata.GetImagesMutual();
			images.clear();
			for (int i = 0; i < oriBinFile.images_size; ++i)
			{

				AI3D::CORE::Image image;
				int photo_id = -1;
				camera_t cam_id;
				int component = -1;
				bool dewrap_flag;
				fix_e fixedstatus = fix_e::EOE_FREE;
				std::string image_path = "";
				std::string image_name = "";
				int image_width, image_height;

				photo_id = oriBinFile.imageDataVec[i].image_id;
				cam_id = oriBinFile.imageDataVec[i].camera_id;
				dewrap_flag = oriBinFile.imageDataVec[i].dewarp_flag;
				image_width = oriBinFile.imageDataVec[i].width;
				image_height = oriBinFile.imageDataVec[i].height;

				
				std::string tmpPath = oriBinFile.imageDataVec[i].path;
				std::string tmpName = oriBinFile.imageDataVec[i].name;
#ifdef WIN32
				// tmpPath = UTF82GBK(tmpPath);
				// tmpName = UTF82GBK(tmpName);
#endif 
				image_path = tmpPath;
				image_name = tmpName;

				image_path = File::EnsureUnifySlash(image_path);
#ifdef _MSC_VER
				image_path = String::StringReplace(image_path, PATH_SEPARATOR_STR, REVERSE_PATH_SEPARATOR_STR);
#endif 

				
				image.SetHeight(image_height);
				image.SetWidth(image_width);
				image.SetImageId(photo_id);
				image.SetCameraId(cam_id);

				if (oriBinFile.imageDataVec[i].hasPosition) {
					Eigen::Vector3d xyz(oriBinFile.imageDataVec[i].position[0], oriBinFile.imageDataVec[i].position[1], oriBinFile.imageDataVec[i].position[2]);
					image.SetPosition(xyz);
				}
				if (oriBinFile.imageDataVec[i].hasPosSigma) {
					
				}
				if (oriBinFile.imageDataVec[i].hasRotaiton) {
					Eigen::Matrix3d rot;
					for (int i = 0; i < 3; ++i)
						for (int j = 0; j < 3; ++j)
							rot(i, j) = oriBinFile.imageDataVec[i].rotation[i][j];
					image.SetRotationMatrix(rot);
				}

				image.SetPath(image_path);
				image.SetName(image_name);

				images[photo_id] = image;
			}

			in.close();
			return true;
		}

		bool ATCommandSet::LoadSourceDataJson(ATData& Atdata, const std::string file_path, Eigen::Vector3d& possigma, std::string skfpath)
		{
			possigma = {10.,10.,10.};
			
			std::string blkcontent;
			std::ifstream in = File::OpenIfstreamUtf8(file_path, std::ios::in);
			if (!in.is_open())
				return false;
			std::string line;
			std::string content;
			while (std::getline(in, line))
			{

				if (line[line.size() - 1] != '\n')
					line.append("\n");

				content.append(line);
			}
			in.close();
			rapidjson::Document doc;
			if (doc.Parse(content.data()).HasParseError())
			{
				return false;
			}
			if (!doc.IsObject())
			{
				return false;
			}
			srs_s srs;
			if (doc.HasMember("coordinate_system"))
			{
				srs.ParseJson(doc["coordinate_system"]);
			}
			auto& images = Atdata.GetImagesMutual();
			images.clear();
			auto& cameras = Atdata.GetCamerasMutual();
			cameras.clear();
			Atdata.SetLocalSrs(srs.definition);
			
		


			if (doc.HasMember("camera_meta_data"))
			{
				rapidjson::Value& cams_jstr = doc["camera_meta_data"];
				for (unsigned i = 0; i < cams_jstr.Size(); i++)
				{
					 camera_t camera_id;
					 if (cams_jstr[i].HasMember("id"))
					 {
						 camera_id = cams_jstr[i]["id"].GetInt();
					 }
					 {
						std::string camera_name = "";
						std::string CameraOrientation = "XRightYDown";
						int  cameramodeltype = 0;
						
						int image_width = kInvalideNum;
						int image_height = kInvalideNum;
						
						double focal_length_pixelx = kInvalideNum;
						double focal_length_pixely = kInvalideNum;
						double sensor_size = kInvalideNum;
						double focal_lengthin35mm = kInvalideNum;
						double cx, cy;
						double k1, k2, k3, k4, k5, k6, p1, p2, p3;
						cx = cy = k1 = k2 = k3 = k4 = k5 = k6 = p1 = p2 = p3 = 0.0;
						
						Camera  camera;
						
						if (cams_jstr[i].HasMember("meta_data"))
						{
							rapidjson::Value& cam_jstr = cams_jstr[i]["meta_data"];
							camera_name = cam_jstr["camera_name"].GetString();
							
							image_width = cam_jstr["width"].GetInt();
							image_height = cam_jstr["height"].GetInt();
							cameramodeltype = cam_jstr["projection_model"].GetInt();
							if (cam_jstr.HasMember("parameters"))
							{
								rapidjson::Value& param_jstr = cam_jstr["parameters"];
								std::vector<double> params(10);
								for (int i = 0; i < 10; ++i)
								{
									params[i] = param_jstr[i].GetDouble();
								}
								focal_length_pixelx = (params[0]);
								focal_length_pixely = (params[1]);
								cx =(params[2]);
								cy = (params[3]);
								k1 = params[4];
								k2 = params[5];
								p2 = params[6];
								p1 = params[7];
								k3 = params[8];
								p3 = params[9];
							}
							
							camera.SetModelIdFromName("FULL_OPENCV");
						
							{
							
								camera.SetParams({ focal_length_pixelx,focal_length_pixely,cx,cy,k1,k2,p1,p2,k3, k4, k5, k6 });
							}

							
							camera.SetWidth(image_width);
							camera.SetHeight(image_height);
							camera.SetCameraId(camera_id);
							camera.SetCameraModelType(CameraModelType_e(cameramodeltype));
							
							camera.SetCameraOrientation(CameraOrientation);
							

						}
						cameras[camera_id] = camera;
					}
				}
			}

			if (doc.HasMember("image_meta_data"))
			{
				rapidjson::Value& img_jstr = doc["image_meta_data"];
				for (unsigned i = 0; i < img_jstr.Size(); i++)
				{
				
					Image image;
					
					int photo_id = -1;
					camera_t cam_id;
					int component = -1;
					bool dewrap_flag;
					fix_e fixedstatus = fix_e::EOE_FREE;
					std::string image_path = "";
					std::string image_name = "";
					int image_width, image_height;

					if (img_jstr[i].HasMember("id"))
					{
						photo_id = img_jstr[i]["id"].GetInt();
					}

					if (img_jstr[i].HasMember("meta_data"))
					{
						rapidjson::Value& it = img_jstr[i]["meta_data"];
						if (it.HasMember("camera_id"))
						{
							cam_id = it["camera_id"].GetInt();
						}
						if (it.HasMember("dewrap_flag"))
						{
							dewrap_flag = it["dewrap_flag"].GetBool();
						}
						if (it.HasMember("width"))
						{
							image_width = it["width"].GetInt();
						}
						if (it.HasMember("height"))
						{
							image_height = it["height"].GetInt();
						}
						
					}

					if (img_jstr[i].HasMember("path"))
					{
						image_path = img_jstr[i]["path"].GetString();
					}
					image_name = File::BoostPathToUtf8String(File::BoostPathFromUtf8(image_path).filename());
					image_path = File::BoostPathToUtf8String(File::BoostPathFromUtf8(image_path).parent_path());


					image_path = File::EnsureUnifySlash(image_path);
#ifdef _MSC_VER
					image_path = String::StringReplace(image_path, PATH_SEPARATOR_STR, REVERSE_PATH_SEPARATOR_STR);
#endif 


					image.SetHeight(image_height);
					image.SetWidth(image_width);
					image.SetPhotoGroupID(cam_id);
					image.SetCameraId(cam_id);


					image.SetImageId(photo_id);
					std::string sfkfile = skfpath + "/" + std::to_string(photo_id) + ".skf";
					sfkfile = File::EnsureUnifySlash(sfkfile);
					if (File::ExistsFile(sfkfile))
						continue;
					image.SetPath(image_path);
					image.SetName(image_name);

					images[photo_id] = image;
				}

			}
			return true;
		}

		bool ATCommandSet::CreateATFiles(ATData& atdata, std::string path, ATOptions& options)
		{
			
			LOGI("=================at 0:" + atdata.GetDefaultEnuSRS().type);
			BA_estimation_polices_s& bap1 = options.sfmsettings.bapolicies;
			
			bap1.use_user_tiepoints_ = atdata.HasUserTiepoints();
			if ((options.align_mode == (int)alignmode_e::ALIGN_WITHGCP))
			{
				options.align_mode = ALIGN_WITHGCP;
				if (atdata.HasAbsPositionImages())
				{
					options.align_mode = ALIGN_WITHGCP_POS;
				}
				else
					options.align_mode = ALIGN_WITHGCP_ARBITRARY;
			}
			


			bap1.use_gcp_ = (options.align_mode &(int) alignmode_e::ALIGN_WITHGCP);
			bap1.use_image_position_ = (options.align_mode & (int)ALIGN_WITHPOS);
			if (bap1.use_user_tiepoints_)
			{

				bap1.usertiepoints_path_ = path + "/manual_ties.json";

				bap1.usertiepoints_path_ = File::EnsureUnifySlash(bap1.usertiepoints_path_);
				ATCommandSet::WriteUserTiepointsJson(atdata, bap1.usertiepoints_path_);
			}
			
			
			
			
			if (bap1.use_gcp_)
			{
				
				bap1.gcp_path_ = path + "/gcp.json";
				bap1.gcp_path_ = File::EnsureUnifySlash(bap1.gcp_path_);
				ATCommandSet::WriteGCPMeasurementsJson(atdata, bap1.gcp_path_);
			}
			
			std::set<image_t> ids;
			if (bap1.pos_policy_ == POLICIES_KEEP)
			{
				
				ids = atdata.GetImageIdsSet();

			}
			if (!bap1.use_image_position_)
			{
				options.sfmsettings.pos_sigma = { -1.0, -1.0, -1.0 };
			}
			bool shouldExportSourcedata = true;
			if (bap1.tiepoints_policy_ == POLICIES_KEEP || bap1.tiepoints_policy_ == POLICIES_ADJUST)
			{
				
				std::string at_path = path + "/" + SCBINFILE;
				at_path = File::EnsureUnifySlash(at_path);
				bap1.at_path_ = at_path;
				shouldExportSourcedata = false;
				
				
				
				
				

				ATCommandSet::ExportATBinary(at_path, atdata);
			}
			if (bap1.use_image_position_ || !ids.empty())
			{
				std::string posFile = "";
				if (POS_USE_BIN) {
					posFile = POSBIN;
				}
				else {
					posFile = POSJSON;
				}
				
				bap1.pos_path_ = path + "/" + posFile;
				bap1.pos_path_ = File::EnsureUnifySlash(bap1.pos_path_);
				int ret = AI3D_FAILURE;
				if (POS_USE_BIN) {
					ret = ATCommandSet::WritePOSBin(bap1.pos_path_, options.sfmsettings.pos_sigma, atdata, ids);
				}
				else {
					ret = ATCommandSet::WritePOSJson(bap1.pos_path_, options.sfmsettings.pos_sigma, atdata, ids);
				}
				if (ret == AI3D_FAILURE)
				{
					bap1.use_image_position_ = false;
					bap1.pos_path_ = "";
				}
				
			}
			
			
			

			
			
		
			Eigen::Vector3d possigma{ -1.0,-1.0,-1.0 };
			
			{
				if (bap1.pos_policy_ == POLICIES_KEEP)
				{
					possigma = Eigen::Vector3d{ 0.01,0.01,0.01 };
				}
				else if (bap1.pos_policy_ == POLICIES_ADJUST)
				{
					possigma = Eigen::Vector3d{ 0.1,0.1,0.1 };
				}
				else if(bap1.pos_policy_ == POLICIES_COMPUTE)
				{
					
					
					if (options.align_mode != alignmode_e::ALIGN_ARBITRARY)
					{
						
						std::map<std::string, std::vector<image_t>> srsmap_temp;
						
						
						
						for ( auto& iter : atdata.GetImagesMutual())
						{
							if (iter.second.HasPositionPrior())
							{
								
								srs_s src_crs = iter.second.GetPriorSrs();
								if (src_crs.definition == "" || src_crs.type == coord_system_type_e::LOCAL)
								{
									continue;
								}
								srsmap_temp[src_crs.definition].push_back(iter.second.GetImageId());
							}
						}
						if (!srsmap_temp.empty())
						{
							auto dst_crs = srsmap_temp.begin()->first;
							for (auto& it : srsmap_temp)
							{

								image_t ptcount = it.second.size();
								if (ptcount == 0)
								{
									continue;
								}
								std::vector<Eigen::Vector3d> poses_temp(ptcount);
								
								std::vector<Eigen::Matrix3d> rotations_temp(ptcount);

								for (image_t i_pt = 0; i_pt < it.second.size(); i_pt++)
								{
									if (atdata.GetImagesMutual()[it.second[i_pt]].HasPositionPrior())
									{
										Eigen::Vector3d xyz = atdata.GetImagesMutual()[it.second[i_pt]].GetPositionPrior();
										poses_temp[i_pt] = xyz;
										
									}
									else
									{
										poses_temp[i_pt].setConstant(-DBL_MAX);
									}

									if (atdata.GetImagesMutual()[it.second[i_pt]].HasRotationMatrixPrior())
									{
										auto R = atdata.GetImagesMutual()[it.second[i_pt]].GetRotationMatrixPrior();
										rotations_temp[i_pt] = R;
										
									}
									else
									{
										rotations_temp[i_pt].setConstant(-DBL_MAX);
									}
									
								}
								if (poses_temp.empty() )
								{
									continue;
								}
								
								CoordinateTransformer::TransformRotation(poses_temp.size(), poses_temp, rotations_temp, 
									CoordinateDescriptor::GetSRSFromDefinition(it.first), CoordinateDescriptor::GetSRSFromDefinition(dst_crs));

								for (image_t i_pt = 0; i_pt < it.second.size(); i_pt++)
								{
									if (atdata.GetImagesMutual()[it.second[i_pt]].HasPositionPrior())
									{
										atdata.GetImagesMutual()[it.second[i_pt]].SetPosition(poses_temp[i_pt]) ;								
									}

									if (atdata.GetImagesMutual()[it.second[i_pt]].HasRotationMatrixPrior())
									{
										 atdata.GetImagesMutual()[it.second[i_pt]].SetRotationMatrix(rotations_temp[i_pt]);									
									}
								}
							}
							atdata.SetLocalSrs(dst_crs);							
						}
						
					
						possigma = Eigen::Vector3d{ 10.,10.0,10.0 };
					}
					
					
					
				}
				else
				{
					LOGE("not support.");
					return false;
				}
			}
			if (shouldExportSourcedata)
			{
				if (SOURCEDATA_USE_BIN) {
					LOGI("=================gen oriData");
					std::string sourcedatabin = path + "/" + ORIDATABIN;
					sourcedatabin = File::EnsureUnifySlash(sourcedatabin);
					ATCommandSet::SaveSourceDataBinary(atdata, sourcedatabin,  possigma);
				}
				else {
					LOGI("=================gen sourceData");
					std::string sourcedatajson = path + "/" + ORIDATAJSON;
					sourcedatajson = File::EnsureUnifySlash(sourcedatajson);
					ATCommandSet::SaveSourceDataJson(atdata, sourcedatajson,  possigma);
				}
				
				
			}
	
			return true;
		}
		int  ATCommandSet::LoadBlock(const std::string& file, AI3D::CORE::BlockObject& block)
		{
			std::string ext = File::GetFileExtension(file);
			String::StringToLower(&ext);
			if (ext == ".json")
			{
				ATData atdata1;
				bool ret = ATCommandSet::LoadSourceDataJson1(atdata1, file);
				if (!ret)
				{
					return AI3D_FAILURE;
				}
				auto atdata = std::make_shared<ATData>(atdata1);
				block.SetATData(atdata);
			}else if (ext == ".bin")
			{
				ATData atdata1;
				bool ret = ATCommandSet::LoadSourceDataBinary(atdata1, file);
				if (!ret)
				{
					return AI3D_FAILURE;
				}
				auto atdata = std::make_shared<ATData>(atdata1);
				block.SetATData(atdata);
			}
			else
			{

				auto atdata = std::make_shared<AI3D::CORE::ATData>();
				bool ret = block.LoadATXML(file, atdata, false);
				if (!ret)
				{
					return AI3D_FAILURE;
				}
				block.SetATData(atdata);
			}

			return AI3D_SUCCESS;
		}

		bool  ATCommandSet::CreateATTaskInfo(std::string hostname, std::string jobpath, 
			std::string blockpath, const AI3D::CORE::BlockObject::Task_Info& taskinfo, std::string& jobstr)
		{
			
			std::string datetime = GetCurrentTimeStr();
			jobstr = JOB_PREFIX + datetime + SC_POSTFIX;
			
			std::string job = jobstr;
			std::string itempath = BLOCK_PRE + std::to_string(taskinfo.blockId);
			std::string projectfile = taskinfo.projectfile_;
			LOGI("=================projectfile:" + projectfile);
			LOGI("=================blockpath:" + blockpath);
			LOGI("=================jobpath:" + jobpath);
			std::string taskname = "";
			std::string firstTask = "0";
			if (TASK_USE_BIN) {
				
				taskname = TASK_DEF_BIN_PREFIX + firstTask;
			}
			else {
				taskname = TASK_DEF_BIN_PREFIX + firstTask;
			}
			
			int tasktype = 1;
			int taskid = 0;
			
			

			
			
			{
				std::vector<std::string> dirs = File::GetDirList(blockpath);
				std::vector<std::string> dirs_shouldremove, files_shouldremove;
				for (auto& dir : dirs)
				{
					if (String::StringContains(dir, SC_POSTFIX) && String::StringContains(dir, JOB_PREFIX))
					{
						dirs_shouldremove.push_back(dir);
						File::Remove(dir);
					}
				}
				std::string postFix = "";
				if (TASK_USE_BIN) {
					postFix = BINFILE_POSTFIX;
				}
				else {
					postFix = JSONFILE_POSTFIX;
				}
				std::vector<std::string> files = File::GetFileList(blockpath, postFix);

				for (auto& file : files)
				{
					std::string jobPrifix = JOB_PREFIX;
					std::string feedbackPrfix = FEEDBACK_BIN_PREFIX + jobPrifix;
					std::string timePrfix = TIME_BIN_PREFIX + jobPrifix;
					if (String::StringContains(file, "feedback_job_") || String::StringContains(file, "time_job_") || String::StringContains(file, feedbackPrfix) || String::StringContains(file, timePrfix))
					{
						files_shouldremove.push_back(file);
						
					}
				}
				File::RemoveFiles(files_shouldremove);
			}
			std::string postFix = "";
			if (TASK_USE_BIN) {
				postFix = BINFILE_POSTFIX;
			}
			else {
				postFix = JSONFILE_POSTFIX;
			}
			std::string taskfullname = blockpath + PATH_SEPARATOR_STR + job + PATH_SEPARATOR_STR + taskname + postFix;
			taskfullname = File::EnsureUnifySlash(taskfullname);
			File::CreateDirIfNotExists(File::GetParentDir(taskfullname));
			
			task_base_info_s base(itempath, job, projectfile);
			auto step = mile_stone_e::MS_GENATTASK;
			auto msg = MileStoneStringToshow.at(step);
			auto functionname = milestone_function.at(step);
			
			task_metadata_s  meta(msg, functionname, taskname, tasktype, taskid);
			
			task_info_s task(base, meta);
			task.at_options_ = taskinfo.at_options;
			task.task_metadata_.keyMaxImgNum_ = taskinfo.keyMaxImgNum;
			task.task_metadata_.matchMaxImgNum_ = taskinfo.matchMaxImgNum;
			

			if (TASK_USE_BIN) {
				task.WriteToBin(taskfullname);
			}
			else {
				task.WriteToJson(taskfullname);
			}
			

			LOGI("Writing Taskdef0.json Finished!");
			

			TaskCommandSet::CreateJobAndFeedbackFiles(jobpath, projectfile, itempath, hostname, datetime, blockpath, job,true);

			

			return true;
		}
    }
}

       