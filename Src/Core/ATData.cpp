#include "Core/ATData.h"
#include "Core/Image.h"
#include <Eigen/Eigenvalues> 
#include <fstream>
#include <set>
#include "Core/CoordinateSystem.h"
#include <random>
#include "Core/File.h"
#include "Core/Timer.h"

#include <algorithm>
#include "Core/Warp.h"
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>
#include <cmath>
#include "Core/Camera.h"
#include "Core/AlgorithmBase.h"
#include "Core/CameraModels.h"
#include <Eigen/Core>
#include "Core/Application.h"
#include <exception> 
using namespace AI3D::CORE;


using std::ifstream;
using std::ofstream;
using std::istringstream;


class  WithComma : public std::numpunct<char> 
{
protected:
	char do_decimal_point() const { return '.'; } 
};

const double pi = 3.14159265358979323846264338327950288;

Eigen::Vector3d Quaterniond2Euler(const double x, const double y, const double z, const double w)
{
	Eigen::Quaterniond q;
	q.x() = x;
	q.y() = y;
	q.z() = z;
	q.w() = w;
	Eigen::Vector3d euler = q.toRotationMatrix().eulerAngles(2, 1, 0);
	
	return euler;
}
namespace AI3D
{
	namespace CORE
	{
		ATData::ATData()
			: 
			num_added_points3D_(0)
		{}

		ATData::ATData(const ATData& Atdata)
		{
			user_points3D_ = Atdata.user_points3D_;
			cameras_ = Atdata.cameras_;
			images_ = Atdata.images_;
			controlpoints_ = Atdata.controlpoints_;
			points3D_ = Atdata.points3D_;
			image_path_to_id_ = Atdata.image_path_to_id_;
			image_pairs_ = Atdata.image_pairs_;
			reg_image_ids_ = Atdata.reg_image_ids_;
			origin_srs_definition_ = Atdata.origin_srs_definition_;
			local_gcp_srs_definition_ = Atdata.local_gcp_srs_definition_;
			local_srs_definition_ = Atdata.local_srs_definition_;
			num_added_points3D_ = Atdata.num_added_points3D_;
			position_offset_ = Atdata.position_offset_;
			box_ = Atdata.box_;
			tile_aabb_box_ = Atdata.tile_aabb_box_;
			tight_box_ = Atdata.tight_box_;
			imageids_tiling_ = Atdata.imageids_tiling_;
			point3dids_tiling_ = Atdata.point3dids_tiling_;
			point_views_ = Atdata.point_views_;
			view_points_ = Atdata.view_points_;
			constraintList_ = Atdata.constraintList_;
		}
		ATData& ATData::operator=(const ATData& Atdata)
		{
			user_points3D_ = Atdata.user_points3D_;
			cameras_ = Atdata.cameras_;
			images_ = Atdata.images_;
			controlpoints_ = Atdata.controlpoints_;
			points3D_ = Atdata.points3D_;
			image_path_to_id_ = Atdata.image_path_to_id_;
			image_pairs_ = Atdata.image_pairs_;
			reg_image_ids_ = Atdata.reg_image_ids_;
			origin_srs_definition_ = Atdata.origin_srs_definition_;
			local_gcp_srs_definition_ = Atdata.local_gcp_srs_definition_;
			local_srs_definition_ = Atdata.local_srs_definition_;
			num_added_points3D_ = Atdata.num_added_points3D_;
			position_offset_ = Atdata.position_offset_;
			box_ = Atdata.box_;
			tile_aabb_box_ = Atdata.tile_aabb_box_;
			tight_box_ = Atdata.tight_box_;
			imageids_tiling_ = Atdata.imageids_tiling_;
			point3dids_tiling_ = Atdata.point3dids_tiling_;
			point_views_ = Atdata.point_views_;
			view_points_ = Atdata.view_points_;
			constraintList_ = Atdata.constraintList_;
			return *this;
		}
		ATData::~ATData() 
		{
			

			user_points3D_.clear();
			cameras_.clear();
			images_.clear();
			controlpoints_.clear();
			points3D_.clear();
			image_path_to_id_.clear();
			image_pairs_.clear();
			reg_image_ids_.clear();



			
			imageids_tiling_.clear();
			point3dids_tiling_.clear();
			point_views_.clear();
			view_points_.clear();
			constraintList_.clear();

		}

		inline  point3D_t ATData::GetNumUserPoints() const
		{
			return user_points3D_.size();
		}

		inline  point3D_t ATData::GetNumConstraint() const
		{
			return constraintList_.size();
		}

		std::unordered_set<point3D_t> ATData::GetPoint3DIds() const 
		{
			std::unordered_set<point3D_t> point3D_ids;
			point3D_ids.reserve(points3D_.size());

			for (const auto& point3D : points3D_) 
			{
				
					point3D_ids.insert(point3D.first);
			}

			return point3D_ids;
		}
		const bool ATData::SaveCBBin(const std::string& file) const
		{
			



			std::set<image_t> idsets;
			for (const auto& id : reg_image_ids_)
			{
				if (!images_.count(id))
				{
					LOGE("not count id");
					continue;
				}
				if (images_.at(id).HasColorParams())
				{
					idsets.insert(id);
				}
			}
			


			std::ofstream out = File::OpenOfstreamUtf8(file, std::ios::binary);
			if (!out.is_open()) {
				if (BlockObject::isChineseVersion())
				{
					LOGI(file + "输出失败 ");
				}
				else
				{
					LOGI("failed to write file " + file);

				}
				return false;
			}
			CBBinFile cbBinFile;
			int count = idsets.size();
			cbBinFile.num = count;
			cbBinFile.colorsParams.clear();
			for (auto iter : idsets)
			{
				CBItemData cbItemData;
				cbItemData.imageid = iter;
				std::array<double, 3> colors;
				colors[0] = images_.at(iter).GetColorParam().x();
				colors[1] = images_.at(iter).GetColorParam().y();
				colors[2] = images_.at(iter).GetColorParam().z();
				cbItemData.colors = colors;
				cbBinFile.colorsParams.push_back(cbItemData);
			}
			cbBinFile.Serialize(out);

			out.close();
			return true;
		}
		const bool ATData::ShouldCB() const
		{
			bool shouldcb = false;
			for (const auto& id : reg_image_ids_)
			{
				if (!images_.count(id))
				{
					LOGE("not count id");
					continue;
				}
				if (!images_.at(id).HasColorParams())
				{
					shouldcb = true;
				}
			}
			return shouldcb;
		}
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		

		
		
		
		
		
		
		
		

		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		

		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		

		
		
		
		
		

		
		
		
		
		

		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		

		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		

		
		
		
		
		
		

		
		
		
		

		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		

		
		
		
		
		
		
		
		
		

		
		
		
		
		
		
		
		
		
		
		
		
		
		std::vector<image_t> ATData::GetHasPostionImagesIds() const
		{
			
			std::vector<image_t> img_idx;

			for (auto it = images_.begin(); it != images_.end();++it)
			{
				if (it->second.HasPosition())			
				{
					img_idx.push_back(it->second.GetImageId());
				}
			}
			return img_idx;
			
		}
		void ATData::ComputeFrustum()
		{
			std::map<image_t, image_t> ids_to_idx, idx_to_ids;
			image_t imgcnt = 0;
			for (auto iter : images_)
			{
				ids_to_idx[iter.first] = imgcnt;
				idx_to_ids[imgcnt] = iter.first;
				imgcnt++;

			}
			
			
			
			
			
			
			
			
			
			
			

			
			
			
			
			
			

			
			
			
			for (image_t i = 0; i < idx_to_ids.size(); i++)
			{
				auto imgid = idx_to_ids[i];
				if (!images_.count(imgid))
				{
					return;
				}
				
			
				AI3D::CORE::Image& image = images_.at(imgid);
				const auto& camera = cameras_.at(image.GetCameraId());
				const Eigen::Matrix3d Rt = image.GetProjectionMatrix().block(0, 0, 3, 3).transpose();
				const Eigen::Vector3d t = image.GetProjectionMatrix().block(0, 3, 3, 1);
				const Eigen::Matrix3d Kinv = camera.GetCalibrationMatrix().inverse();

				Eigen::Vector2d range(image.GetDepth().x(), image.GetDepth().z());
				if (range[0] == std::numeric_limits<double>::max() || range[1] == std::numeric_limits<double>::lowest())
				{
					continue;
				}
				const double w = image.GetWidth();
				const double h = image.GetHeight();
				image.GetFrustumMutual().clear();
				
				{
					double d = image.GetDepth().z();
					std::vector<Eigen::Vector3d> pixel_depths =
					{ {0,0,d},{0,h * d,d},{w * d,0,d},{w * d,h * d,d} };
					for (int n = 0; n < 4; n++)
					{
						Eigen::Vector3d pix_d = pixel_depths[n];
						Eigen::Vector3d pt_world = Rt * (Kinv * pix_d - t);
						image.GetFrustumMutual().push_back(pt_world);
						
					}
				}
			}
		}

		void ATData::ComputeDepths()
		{
			std::map<image_t, std::vector<double> > images_to_point_depths;
			
			for (auto iter : GetRegImageIds())
			{
				if (!images_.count(iter))
				{
					return;
				}
				auto& image = images_.at(iter);
				// LOGI(image.GetImageId());

				if (image.HasDepth())
				{
					
					if(!image.HasFrustum())
					{
						const double w = image.GetWidth();
						const double h = image.GetHeight();
						image.GetFrustumMutual().clear();
						double d = image.GetDepth().z();
						auto camera = cameras_.at(image.GetCameraId());
						const Eigen::Matrix3d Rt = image.GetProjectionMatrix().block(0, 0, 3, 3).transpose();
						const Eigen::Vector3d t = image.GetProjectionMatrix().block(0, 3, 3, 1);
						const Eigen::Matrix3d Kinv = camera.GetCalibrationMatrix().inverse();
						std::vector<Eigen::Vector3d> pixel_depths =
						{ {0,0,d},{0,h * d,d},{w * d,0,d},{w * d,h * d,d} };
						for (int n = 0; n < 4; n++)
						{
							Eigen::Vector3d pix_d = pixel_depths[n];
							Eigen::Vector3d pt_world = Rt * (Kinv * pix_d - t);
							image.GetFrustumMutual().push_back(pt_world);

						}
					}

					continue;
				}
				if (!cameras_.count(image.GetCameraId()))
				{
					LOGE("has no camera");
					continue;
				}
				auto camera = cameras_.at(image.GetCameraId());
				for (auto iterobs : image.GetPoints2D())
				{
					auto pt3did = iterobs.GetPoint3DId();
					if (points3D_.count(pt3did))
					{
						auto point = points3D_.at(pt3did);
						const double proj_z = image.GetProjectionMatrix().row(2).dot(point.GetXYZ().homogeneous());
						const Eigen::Vector3d world_point = image.GetProjectionMatrix() * point.GetXYZ().homogeneous();
						images_to_point_depths[image.GetImageId()].push_back(world_point.z());

					}
				}
			}

			for (auto iter : images_to_point_depths)
			{
				const int depth_count = iter.second.size();
				if (depth_count < 3)
				{
					continue;
				}
				if (!images_.count(iter.first))
					continue;
				std::sort(iter.second.begin(), iter.second.end());
				
				auto& image = images_.at(iter.first);
				image.SetDepth(Eigen::Vector3d(iter.second[0], iter.second[depth_count / 2], iter.second[depth_count - 1]));
				{
					const double w = image.GetWidth();
					const double h = image.GetHeight();
					image.GetFrustumMutual().clear();
					double d = image.GetDepth().z();
					auto camera = cameras_.at(image.GetCameraId());
					const Eigen::Matrix3d Rt = image.GetProjectionMatrix().block(0, 0, 3, 3).transpose();
					const Eigen::Vector3d t = image.GetProjectionMatrix().block(0, 3, 3, 1);
					const Eigen::Matrix3d Kinv = camera.GetCalibrationMatrix().inverse();
					std::vector<Eigen::Vector3d> pixel_depths =
					{ {0,0,d},{0,h * d,d},{w * d,0,d},{w * d,h * d,d} };
					for (int n = 0; n < 4; n++)
					{
						Eigen::Vector3d pix_d = pixel_depths[n];
						Eigen::Vector3d pt_world = Rt * (Kinv * pix_d - t);
						image.GetFrustumMutual().push_back(pt_world);

					}
				}
			}
		}

		void ATData::ComputeTiepointError(std::vector<double>& errors)
		{
			
		}

		bool ATData::GenerateATReportForTiepoint(ConnectionPointQualityBriefing& connectionQB)
		{

			

			return true;
		}
		bool ATData::GenerateATReportForImageAndTiepoint(ConnectionPointQualityBriefing& connectionQB,Image_QualityBriefing& imageQB)
		{
			

			if (points3D_.empty())
				return false;
			
			std::map<image_t, int> n_errs_perphotomap;
			std::map<point3D_t, std::set<int>> obrs_petie;
			std::vector<double> project_errs;
			std::map<image_t,double> project_errs_perphoto;
			for (auto& cam : GetCamerasMutual())
			{
				
				double left_min_x, left_max_x, right_min_x, right_max_x, top_min_y, top_max_y, bottom_min_y, bottom_max_y;
				cam.second.GetValidUndistortBorder(left_min_x, left_max_x, right_min_x, right_max_x, top_min_y, top_max_y, bottom_min_y, bottom_max_y);
				double undistortedborder[8] = { left_min_x, left_max_x, right_min_x, right_max_x, top_min_y, top_max_y, bottom_min_y, bottom_max_y };
				cam.second.SetUndistortBorder(undistortedborder);
				
				Eigen::Vector2f diagonal(cam.second.GetWidth(), cam.second.GetHeight());
				double fov = diagonal.norm() / (cam.second.GetMeanFocalLength() * 2);
				fov = atan(fov) * 180 / M_PI;
				cam.second.SetFov(fov);
			}
			double rms = 0.;
			size_t obscount = 0;
			for (auto point : points3D_)
			{
				for (auto obs : point.second.GetTrackMutual().GetElementsMutual())
				{
					if (!images_.count(obs.image_id))
					{
						return false;
					}
					auto& image = images_.at(obs.image_id);
					auto& camera = cameras_.at(image.GetCameraId());

					n_errs_perphotomap[obs.image_id]++;
					Eigen::Vector2d estimated_xy = AlgorithmBase::ProjectPointToImage(point.second.GetXYZMutual(),
						image.GetProjectionMatrix(), camera, false);
					Eigen::Vector2d err = estimated_xy - obs.xy;
					project_errs.emplace_back(err.norm());
					rms += project_errs[obscount] * project_errs[obscount];
					obrs_petie[point.first].insert(obs.image_id);
					project_errs_perphoto[obs.image_id] += project_errs[obscount] * project_errs[obscount];
					obscount++;
				}
			}

			std::sort(project_errs.begin(), project_errs.end());
			double medianerr = project_errs[project_errs.size() / 2];
			rms = std::sqrt(rms / project_errs.size());
			connectionQB.rms_px = rms;
			connectionQB.error_repro_px = medianerr;
			
			std::vector<int> n_errs_perphoto(GetNumRegImages());
			obscount = 0;
			int imgcnt = 0;
			for (auto iter : GetRegImageIds())
			{
				n_errs_perphoto[imgcnt]=(images_.at(iter).GetPoints2D().size());
				imgcnt++;
				obscount += images_.at(iter).GetPoints2D().size();
			}

			std::sort(n_errs_perphoto.begin(), n_errs_perphoto.end());
			connectionQB.Meadium_ConnectionPoint_Perimage = n_errs_perphoto[n_errs_perphoto.size() / 2];
			connectionQB.num_observations = obscount;
			
			connectionQB.num_connections = GetNumPoints3D();

		
			
			std::map<image_t, std::map<image_t, bool>> connectedimg_perimg;
			
			for (auto point : points3D_)
			{
				for (auto connect : obrs_petie.at(point.first))
				{
					image_t imgid = connect;
					for (auto connectnew : obrs_petie.at(point.first))
					{
						if (connect == connectnew)
						{
							continue;
						}
						connectedimg_perimg[connect][connectnew] = true;
					}
				}
			}
			obrs_petie.clear();
			int npairs = 0;
			std::vector<int> nconnected_perphoto; nconnected_perphoto.resize(connectedimg_perimg.size());
			size_t count = 0;
			for (auto iter : connectedimg_perimg)
			{
				npairs += iter.second.size();
				nconnected_perphoto[count] = iter.second.size();
				SingleQB singleQB;

				auto image = images_.at(iter.first);
				singleQB.rms_px = std::sqrt(project_errs_perphoto.at(iter.first));

				singleQB.num_images_connection = iter.second.size();

				singleQB.imagefullpath = image.GetName();

				singleQB.photogroup_id = image.GetPhotoGroupID();

				singleQB.num_observations = image.GetNumPoints2D();

				imageQB.singleQB.push_back(singleQB);
				count++;

			}
			npairs /= 2;
			std::sort(nconnected_perphoto.begin(), nconnected_perphoto.end());;
			imageQB.Meadium_ConnectionPoint_Perimage = npairs;
			imageQB.Medium_Images_Perimage = nconnected_perphoto[count / 2];


			return true;
		}
		
		double ATData::GetGSD() const
		{
			auto imgcnt = GetRegImageIds().size();
			std::vector<std::vector<double>> image_points(imgcnt);
			std::vector<image_t> idx_to_id(imgcnt);
			image_t count = 0;
			for (auto iter : GetRegImageIds())
			{
				idx_to_id[count] = iter;
				count++;
			}
			std::vector<double> gsd(imgcnt);

			for (image_t i=0;i< idx_to_id.size();i++)
			{
				auto imageid = idx_to_id[i];
				auto image = images_.at(imageid);
				if (!image.HasPoints())
				{
					continue;
				}
				auto points2d = image.GetPoints2D();
				for (auto& pt : points2d)
				{
					if(pt.HasPoint3D())
						image_points[i].push_back(points3D_.at(pt.GetPoint3DId()).GetZ());
				}

				
				
			}
			for (image_t i = 0; i < idx_to_id.size(); i++)
			{
				if (image_points[i].empty())
				{
					continue;
				}
				auto imageid = idx_to_id[i];
				auto image = images_.at(imageid);
				std::nth_element(image_points[i].begin(), image_points[i].begin() + image_points[i].size() / 2, image_points[i].end());
				const double height = image_points[i][image_points[i].size() / 2];
				const double focal_lenth = cameras_.at(image.GetCameraId()).GetMeanFocalLength();
				gsd[i] = (image.GetPosition().z() - height) / focal_lenth;
			}
			std::nth_element(gsd.begin(), gsd.begin() + gsd.size() / 2, gsd.end());
			
			if (gsd.size() <= 0)
				return 1.0;
			return gsd[gsd.size() / 2];

		}
		float ATData::ComputeAvgResolution()
		{
			
			ComputeDepths();
			double res = 0.;
			int validimagcnt = 0;
			
			
			for (auto iter : GetRegImageIds())
			{
				auto image = images_.at(iter);
				if (image.HasDepth())
				{
					Camera camera = cameras_[image.GetCameraId()];

					if (!image.IsPoseAndIntrinsicDefined(camera))
						continue;
					double focalpix = cameras_.at(image.GetCameraId()).GetFocalLengthX();
					res += image.GetDepthMutual()[1] / focalpix;
					validimagcnt++;
				}
			}
			if (validimagcnt > 0)
			{
				res /= (double)validimagcnt;
			}
			return res;
			
		}
		
		bool ATData::GenerateATReportForCam(std::vector<CameraCalibration>& cameracalibrationvec)
		{
			
			for (auto iter : cameras_)
			{
				CameraCalibration cameracalibration;
				Camera camera = iter.second;
				cameracalibration.id = camera.GetCameraId();
				cameracalibration.parameters.assign(camera.GetParamsMutual().begin(), camera.GetParamsMutual().end());
				if (camera.GetFocalLengthMM() != UNDEFINEDVAL)
				{
					cameracalibration.focalinmm = camera.GetFocalLengthMM();
				}
				cameracalibration.focalin35mm = camera.GetFocalLengthX() * 36 / std::max(camera.GetWidth(), camera.GetHeight());
				cameracalibrationvec.push_back(cameracalibration);
			}
			return true;
		}

		

		bool ATData::GenerateBaseInfo(ProjectDescrip& proj_desc)
		{
			image_t totalimagenum = GetNumImages();
			image_t registeredimgnum = GetNumRegImages();
			image_t failedphoto_num = totalimagenum - registeredimgnum;
			proj_desc.num_photos = totalimagenum;
			proj_desc.Calibration_failed_photos = failedphoto_num;
			proj_desc.Calibration_success_photos = registeredimgnum;
			proj_desc.Ratio_Calibration_success = (float)registeredimgnum / float(totalimagenum);
			proj_desc.num_cameras = cameras_.size();
			proj_desc.average_resolution = ComputeAvgResolution();
			return true;
		}

		bool ATData::GenerateATReport( ATReport& at_report)
		{
			
			
			ProjectDescrip proj_desc;
			GenerateBaseInfo(proj_desc);
			AcquisitionReport gcp_accuracy;
			gcp_accuracy.gpt_type = gpt_e::GCP_CONTROL_HV;

			AcquisitionReport checkpoint_accuracy;
			checkpoint_accuracy.gpt_type = gpt_e::GCP_CHECK_HV;
			GenerateATReportForGCP(gcp_accuracy, checkpoint_accuracy);

			std::vector<CameraCalibration> CameraCalibrationVec;
			
			GenerateATReportForCam( CameraCalibrationVec);
			
			ConnectionPointQualityBriefing connectionQB;
			Image_QualityBriefing imageQB;
			GenerateATReportForImageAndTiepoint(connectionQB, imageQB);
		
			
			at_report.proj_desc = proj_desc;
			at_report.gcp_accuracy = gcp_accuracy;
			at_report.checkpoint_accuracy = checkpoint_accuracy;
			at_report.CameraCalibratedParam = CameraCalibrationVec;

			at_report.connectionQB = connectionQB;
			at_report.imageQB = imageQB;
			return true;
		}

		bool ATData::GenerateATReportForGCP(AcquisitionReport& gcp_accuracy, AcquisitionReport& checkpoint_accuracy)
		{
			if (!HasControlPoints())
			{
				return false;
			}
			gcp_accuracy.gpt_type = gpt_e::GCP_CONTROL_HV;
			checkpoint_accuracy.gpt_type = gpt_e::GCP_CHECK_HV;
			std::map<point3D_t, std::map < image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >> measurement_error_map;
			UpdataGCPGlobalErrorInfo(measurement_error_map);
			std::vector<double> gcplist_pix, chklist_pix;
			double gcprms_pix = 0, chksrms_pix = 0;
			Eigen::Vector3d gcp_3derror, chkgcp_3derror;
			int gcpcount = 0, chkcount = 0;
			for (auto it : GetControlPoints())
			{
				Acquisition gcp_acq;
				gcp_acq.id = it.first;
				gcp_acq.rms_px = it.second.GetObjectPoint().GetPixelRMS();
				if ((gcp_acq.rms_px == kInvalidError || gcp_acq.rms_px == -DBL_MAX))
					continue;
				double squarerms = gcp_acq.rms_px * gcp_acq.rms_px;
				double error3d_xx = it.second.GetX3DError() * it.second.GetX3DError();
				double error3d_yy = it.second.GetY3DError() * it.second.GetY3DError();
				double error3d_zz = it.second.GetZ3DError() * it.second.GetZ3DError();
				bool bvalidxyz = (it.second.GetGivenXYZ().x() != -DBL_MAX);
				bool bvalidetimatedxyz = (it.second.GetEstimatedXYZ().x() != -DBL_MAX);
				if (!(bvalidxyz && bvalidetimatedxyz))
				{
					continue;
				}
				gcp_acq.error_z = it.second.GetZ3DError();
				gcp_acq.error_3d = it.second.Get3DError();
				gcp_acq.error_xy = it.second.GetXY3DError();
				gcp_acq.name = it.second.GetName();
				gcp_acq.num_observations = it.second.GetObjectPoint().GetTrack().Length();

				if (it.second.GetType() == gpt_e::GCP_CONTROL_HV)
				{
					gcprms_pix += squarerms;
					gcp_3derror.x() += error3d_xx;
					gcp_3derror.y() += error3d_yy;
					gcp_3derror.z() += error3d_zz;
					gcpcount++;

					gcp_accuracy.accuracy_vec.push_back(gcp_acq);
					gcplist_pix.push_back(gcp_acq.rms_px);
				}
				else if (it.second.GetType() == gpt_e::GCP_CHECK_HV)
				{
					chksrms_pix += squarerms;
					chklist_pix.push_back(gcp_acq.rms_px);
					chkcount++;
					chkgcp_3derror.x() += error3d_xx;
					chkgcp_3derror.y() += error3d_yy;
					chkgcp_3derror.z() += error3d_zz;

					checkpoint_accuracy.accuracy_vec.push_back(gcp_acq);
				}

			}
			std::sort(gcplist_pix.begin(), gcplist_pix.end());
			std::sort(chklist_pix.begin(), chklist_pix.end());

			if (gcpcount > 0)
			{
				double gcp_rms_px_all = std::sqrt(gcprms_pix / gcpcount);
				gcp_accuracy.rms_px_all = gcp_rms_px_all;
				gcp_3derror.x() = std::sqrt(gcp_3derror.x() / gcpcount);
				gcp_3derror.y() = std::sqrt(gcp_3derror.y() / gcpcount);
				gcp_3derror.z() = std::sqrt(gcp_3derror.z() / gcpcount);
				gcp_accuracy.error_3d_all = gcp_3derror.norm();
				gcp_accuracy.error_xy_all = std::sqrt(gcp_3derror.x() * gcp_3derror.x() + gcp_3derror.y() * gcp_3derror.y());
				gcp_accuracy.error_z_all = gcp_3derror.z();
				gcp_accuracy.error_reproj = gcplist_pix[gcplist_pix.size() / 2];
				gcp_accuracy.num_gcps = gcpcount;

			}
			if (chkcount > 0)
			{
				double gcpchk_rms_px_all = std::sqrt(chksrms_pix / chkcount);
				checkpoint_accuracy.rms_px_all = gcpchk_rms_px_all;
				chkgcp_3derror.x() = std::sqrt(chkgcp_3derror.x() / chkcount);
				chkgcp_3derror.y() = std::sqrt(chkgcp_3derror.y() / chkcount);
				chkgcp_3derror.z() = std::sqrt(chkgcp_3derror.z() / chkcount);

				checkpoint_accuracy.error_3d_all = chkgcp_3derror.norm();
				checkpoint_accuracy.error_xy_all = std::sqrt(chkgcp_3derror.x() * chkgcp_3derror.x() + chkgcp_3derror.y() * chkgcp_3derror.y());
				checkpoint_accuracy.error_z_all = chkgcp_3derror.z();
				checkpoint_accuracy.error_reproj = chklist_pix[chklist_pix.size() / 2];;
				checkpoint_accuracy.num_gcps = chkcount;
			}
			return true;
		}

		void ATData::GeneratePointViews(std::set<image_t>& imageids, std::set<point3D_t>& point3dids)
		{
			// Full per-image visibility pass is too slow on large blocks; subsample tiepoints instead.
			if (points3D_.size() <= 200000)
			{
				std::map<int, image_t> image_idx_to_id;
				std::map<image_t, int> id_to_idx;
				std::vector<AI3D::CORE::Image> imagevec(reg_image_ids_.size());
				for (image_t imgidx = 0; imgidx < reg_image_ids_.size(); imgidx++)
				{
					image_t image_id = reg_image_ids_[imgidx];
					imagevec[imgidx] = images_[image_id];
					AI3D::CORE::Image image = images_[image_id];
					image_idx_to_id[imgidx] = image_id;
					id_to_idx[image_id] = imgidx;
					for (int ptidx = 0; ptidx < images_[image_id].GetPoints2D().size(); ptidx++)
					{
						Point2D point2d = images_[image_id].GetPoints2D()[ptidx];
						point3D_t point_id = point2d.GetPoint3DId();
						if (!points3D_.count(point_id))
						{
							continue;
						}
						Point3D point3d = points3D_.at(point_id);
						if (image.IsVisible(point3d.GetXYZ(), cameras_.at(image.GetCameraId()).GetCalibrationMatrix()))
						{
							point_views_[point_id].emplace_back(std::make_pair(image_id, 1.0));
							view_points_[image_id].emplace_back(point_id);
							imageids.insert(image_id);
							point3dids.insert(point_id);
						}
					}
				}
			}
			else
			{
				point3D_t pointcnt = points3D_.size();
				
				std::vector<point3D_t> pointids(pointcnt);
				{
					EIGEN_STL_UMAP(point3D_t, Point3D) tps;
					point3D_t idx = 0;
					
					std::random_device rd;
					std::mt19937 g(rd());
					std::vector<int> reserved_point_id;
					for (auto& pt : points3D_)
					{
						reserved_point_id.push_back(pt.first);
					}
					point3D_t remaincnt = pointcnt > 200000 ? 200000 : pointcnt;

					std::shuffle(reserved_point_id.begin(), reserved_point_id.end(), g);
					reserved_point_id.erase(reserved_point_id.begin() + remaincnt, reserved_point_id.end());
					for (int i = 0; i < reserved_point_id.size(); i++)
					{
						tps.insert(std::make_pair(reserved_point_id[i], points3D_[reserved_point_id[i]]));
					}

					for (auto& iterpoint : tps)
					{
						pointids[idx] = iterpoint.first;
						idx++;
					}
				}
				for (int i =0 ;i< pointids.size();i++)
				{

					point3D_t id = pointids[i];
					auto iterpoint = points3D_[id];
					for (auto& iterele : iterpoint.GetTrack().GetElements())
					{
						image_t  image_id = iterele.image_id;
						point3D_t point_id = iterpoint.GetId();
						auto image = images_.at(image_id);
						auto camera = cameras_.at(image.GetCameraId());
						if (image.IsVisible(iterpoint.GetXYZ(), camera.GetCalibrationMatrix()))
						{
							point_views_[point_id].emplace_back(std::make_pair(image_id, 1.0));
							view_points_[image_id].emplace_back(point_id);
							imageids.insert(image_id);
							point3dids.insert(point_id);
						}
					}
				}
			}
			
		}

		void  ATData::GeneratePointViews()
		{
			if (!imageids_tiling_.empty() && !point3dids_tiling_.empty())
			{
				return;
			}
			GeneratePointViews(imageids_tiling_, point3dids_tiling_);
		}
		const std::map<point3D_t, std::vector<viewweight>>& ATData::GetPointsViews() const
		{
			return point_views_;
		}
		const std::map<image_t, std::vector<point3D_t>>& ATData::GetViewPoints() const
		{
			return view_points_;
		}

		bool  ATData::Simplify(const SimplifyOptions& opt)
		{
			int min_overlap = opt.min_overlap_;
			int max_overlap = opt.max_overlap_;
			int max_tie_point = opt.max_tiepoint_count_;
			float max_proj_error = opt.max_proj_error_;

			
			std::vector<point3D_t> reserved_point_id;
			for (point3D_t i = 0; i < points3D_.size(); ++i)
			{
				if (min_overlap > 0 && points3D_[i].GetTrack().Length() < min_overlap)
				{
					continue;
				}
				reserved_point_id.push_back(i);
			}

			std::random_device rd;
			std::mt19937 g(rd());
			std::shuffle(reserved_point_id.begin(), reserved_point_id.end(), g);
			if (0)
			{
				int gcpid = 0;

				
				
				for (point3D_t i = 0; i < 5000; i++)
				{
					point3D_t idx = reserved_point_id[reserved_point_id.size() - 5000 + i];
					
					{
						ControlPoint gcp;
						gcp.SetId(gcpid);
						gcp.SetName(std::to_string(gcpid));
						gcp.SetType(GCP_CONTROL_HV);

						Point3D pt = points3D_.at(idx);
						gcp.SetObjectPoint(pt);
						gcp.SetGivenXYZ(points3D_[idx].GetXYZMutual());
						gcpid++;
						controlpoints_[gcpid] = gcp;
					}
				}
			}
			if (0 < max_tie_point && max_tie_point < reserved_point_id.size())
			{
				reserved_point_id.erase(reserved_point_id.begin() + max_tie_point, reserved_point_id.end());
			}
			
			std::set<point3D_t> reserved_point_id_set;
			for (point3D_t i = 0; i < reserved_point_id.size(); ++i)
			{
				reserved_point_id_set.insert(reserved_point_id[i]);
			}
			EIGEN_STL_UMAP(point3D_t, Point3D) points3D_new;
			for (point3D_t i = 0; i < points3D_.size(); ++i)
			{
				if (reserved_point_id_set.count(i) <= 0)
				{
					continue;
				}

				Point3D point = points3D_[i];;
				if (max_proj_error >= 0.0)
				{
					auto remove_pos = std::remove_if(points3D_[i].GetTrackMutual().GetElementsMutual().begin(),
						points3D_[i].GetTrackMutual().GetElementsMutual().end(), [&](TrackElement ele)
						{
							image_t img_id = ele.image_id;
							auto& image = images_[img_id];
							auto& camera = cameras_[image.GetCameraId()];
							Eigen::Vector2d estimated_xy = AlgorithmBase::ProjectPointToImage(point.GetXYZ(),
								image.GetProjectionMatrix(),
								camera, false);
							bool status1 = (camera.GetWidth()) > estimated_xy.x() && estimated_xy.x() > 0;
							bool status2 = (camera.GetHeight()) > estimated_xy.y() && estimated_xy.y() > 0;
							if (!(status1 && status2))
							{
								return false;
							}
							double error = (estimated_xy - ele.xy).norm();
							if (error > max_proj_error)
							{
								return true;
							}
							return false;
						});
					points3D_[i].GetTrackMutual().GetElementsMutual().erase(remove_pos, points3D_[i].GetTrackMutual().GetElementsMutual().end());
					if (0 <min_overlap && points3D_[i].GetTrackMutual().Length() < min_overlap)
					{
						continue;
					}

				}

				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				

				
				points3D_new[points3D_[i].GetId()] = points3D_[i];
			}
			points3D_ = points3D_new;
			points3D_new.clear();
			return true;
		}


		std::vector<image_t> ATData::GetHasPriorPostionImagesIds() const
		{
			std::vector<image_t> img_idx;

			for (auto it = images_.begin(); it != images_.end();++it)
			{
				if (it->second.HasPositionPrior())
				{
					img_idx.push_back(it->second.GetImageId());
				}
			}
			return img_idx;

		}

		std::set<image_t> ATData::GetImagesIdSet() const
		{
			std::set<image_t> images_ids;
			
			for (const auto& image : images_)
			{
				images_ids.insert(image.first);
			}

			return images_ids;
		}

		std::vector<image_t> ATData::GetImagesIds() const
		{
			std::vector<image_t> images_ids;
			images_ids.reserve(images_.size());

			for (const auto& image : images_)
			{
				images_ids.push_back(image.first);
			}

			return images_ids;
		}

		void ATData::SetOriginSrs(std::string srs)
		{
			origin_srs_definition_ = srs;

		}
		const std::string  ATData::GetOriginSrs()  const
		{
			return origin_srs_definition_;
		}
		void ATData::SetLocalSrs(std::string srs)
		{
			local_srs_definition_ = srs;
		}
		
		void  ATData::EraseDuplicateImages(ATData& datatemp)
		{

			
			std::map<std::string, image_t > imagesset;
			
			std::set < image_t > imageids;
			for (const auto& iter : images_)
			{
				
				
				std::string name = iter.second.GetName();
				String::StringToLower(&name);
				if (!imagesset.count(name))
				{
					imagesset[name] = iter.first;
					
					imageids.insert(iter.first);
				}

			}

			ExtractATDataByImages(imageids, datatemp);

		}
		const std::string  ATData::GetLocalSrs()  const
		{
			return local_srs_definition_;
		}

		bool ATData::GenPreviewImages(const std::string& path)
		{
			std::vector<image_t> img_idx = GetImagesIds();

			
			

			std::string tempfile = path;
			String::StringTrim(String::StringReplace(tempfile, "\\", "/"), "/");
			std::string  prjpath = File::GetParentDir(tempfile + ".txt");



			std::string previewpath = File::JoinPaths(prjpath, PREIMG_PATH);
			File::CreateDirIfNotExists(previewpath);
			{
				
				int num_cpu_core = omp_get_num_procs();
				num_cpu_core = num_cpu_core >= 6 ? 3 : num_cpu_core;
				int numOFThreads = num_cpu_core * 2;
#ifdef USE_OPENMP
#pragma omp parallel for schedule(dynamic) num_threads(numOFThreads)
#endif
				for (int i = 0; i < img_idx.size(); i++)
				{
					images_[img_idx[i]].GenPreviewImage(previewpath);					
				}
				
			}
			return true;
		}


		bool ATData::GenPreviewImages(const std::string& path, std::set<image_t> vecimage)
		{
			std::vector<image_t> img_idx(vecimage.begin(),vecimage.end());


			

			std::string tempfile = path;
			String::StringTrim(String::StringReplace(tempfile, "\\", "/"), "/");
			std::string  prjpath = File::GetParentDir(tempfile + ".txt");




			std::string previewpath = File::JoinPaths(prjpath, PREIMG_PATH);
			File::CreateDirIfNotExists(previewpath);
			{


#ifdef USE_OPENMP
#pragma omp parallel for schedule(dynamic)
#endif
				
				for (int i = 0; i < img_idx.size(); i++)
				{
					images_[img_idx[i]].GenPreviewImage(previewpath);
				}

			}
			return true;
		}

		bool ATData::GenPreviewImages(const std::string& path, std::vector<image_t> vecimage)
		{
			std::vector<image_t> img_idx = vecimage;


			

			std::string tempfile = path;
			String::StringTrim(String::StringReplace(tempfile, "\\", "/"), "/");
			std::string  prjpath = File::GetParentDir(tempfile + ".txt");




			std::string previewpath = File::JoinPaths(prjpath, PREIMG_PATH);
			File::CreateDirIfNotExists(previewpath);
			{


#ifdef USE_OPENMP
#pragma omp parallel for schedule(dynamic)
#endif
				
				for (int i = 0; i < img_idx.size(); i++)
				{
					images_[img_idx[i]].GenPreviewImage(previewpath);
				}

			}
			return true;
		}

		 void ATData::RenderPoses(Eigen::Vector3d& offset, srs_s& srs)
		{
			

			 
			 std::string src_definition = local_srs_definition_;
			 if (local_srs_definition_ == BASESRS || CoordinateDescriptor::GetSRSFromDefinition(local_srs_definition_).type == PROJECTION)
			 {
				 ComputePositionOffsetByAvgCenter(offset, srs.definition);

				
				 TransFormTiepoints(src_definition, srs.definition);
				 TransFormImages(src_definition, srs.definition);
				 
				 local_srs_definition_ = srs.definition;
			 }
			 srs.definition =  local_srs_definition_ ;
			 TransformControlPoints(srs.definition);
		}


		 bool ATData::CanPredict()
		 {
			 if (GetNumControlPoints() < VALIDPREDICTGCPNUM)
			 {
				 return false;
			 }
			 for (auto& it : controlpoints_)
			 {
				 if(it.second.GetObjectPoint().GetTrack().Length()<= VALIDTRIANGLENUM)
				 {
					 return false;
				 }
			 }
			 return true;
		 }
		 
		 void ATData::Predict()
		 {
			 if (CanPredict())
			 {
				using Elements = std::vector<TrackElement>;
				 
				 std::map<uint32_t, Eigen::Vector3d> map_control_points, map_triangulated;
				 std::map<uint32_t, double> map_triangulation_errors;
			
			 for (const auto& control_point_it : controlpoints_)
				 {
					 const Track& landmark = control_point_it.second.GetObjectPoint().GetTrack();
					 
					 const Elements& obs = landmark.GetElements();
					 std::vector<Eigen::Vector2f> points;
					 std::vector<Eigen::Matrix<float, 3, 4>> poses;
					 points.reserve(obs.size());
					 poses.reserve(obs.size());

					 for (const auto& obs_it : obs)
					 {
						 
						 AI3D::CORE::Image image = images_.at(obs_it.image_id);
						 Camera camera = cameras_[image.GetCameraId()];

						 if (!image.IsPoseAndIntrinsicDefined(camera))
							 continue;
						
						  Eigen::Vector2d pt = obs_it.xy;
						 
						 points.emplace_back(camera.UndistortPixel(pt).cast<float>());
						 poses.emplace_back((camera.GetCalibrationMatrix() * image.GetProjectionMatrix()).cast<float>());
					 }
					 Eigen::Vector3f xyzf = AlgorithmBase::TriangulatePoint(poses, points);
					 Eigen::Vector3d xyz(xyzf.x(), xyzf.y(), xyzf.z());
					
					
					
					 
					 
					
					
					 
					 bool bCheirality = true;
					 int i(0);
					 double reprojection_error_sum(0.0);
					 for (const auto& obs_it : obs)
					 {
						  AI3D::CORE::Image image = images_[obs_it.image_id];
						  Camera camera = cameras_[image.GetCameraId()];

						  if (!image.IsPoseAndIntrinsicDefined(camera))
							  continue;
						  const Eigen::Matrix3x4d proj_matrix = image.GetProjectionMatrix();
						  const double proj_z = proj_matrix.row(2).dot(xyz.homogeneous());
						
						  
						
						 bCheirality &= (proj_z > 0.0);
						
						 const  Eigen::Vector2d residual = AlgorithmBase::ProjectPointToImage(xyz, proj_matrix, camera,false);
						 reprojection_error_sum += residual.norm();
						 ++i;
					 }
					 if (bCheirality) 
					 {
						 map_triangulated[control_point_it.first] = xyz;
						 map_control_points[control_point_it.first] = control_point_it.second.GetObjectPoint().GetXYZ();
						 map_triangulation_errors[control_point_it.first] = reprojection_error_sum / (double)points.size();
					 }
					 else
					 {
						 LOGI("Control Point cannot be triangulated (not in front of the cameras)");
						 return;
					 }
				 }

				 if (map_control_points.size() < 3)
				 {
					 LOGI("the size of control_points s not enough");
					 return;
				 }

				 
				 {
					 
					 Eigen::MatrixXd x1(3, map_control_points.size()),
						 x2(3, map_control_points.size());

					 uint32_t id_col = 0;
					 for (const auto& cp : map_control_points)
					 {
						 x1.col(id_col) = map_triangulated[cp.first];
						 x2.col(id_col) = cp.second;
						 ++id_col;
					 }

					 std::cout
						 << "Control points observation triangulations:\n"
						 << x1 << std::endl << std::endl
						 << "Control points coords:\n"
						 << x2 << std::endl << std::endl;

					 Eigen::Vector3d t;
					 Eigen::Matrix<double, 3, 3> R;
					 double S;
					 if (AlgorithmBase::FindRTS(x1, x2, &S, &t, &R))
					 {
						 AlgorithmBase::Refine_RTS(x1, x2, &S, &t, &R);
						 std::cout << "Found transform:\n"
							 << " scale: " << S << "\n"
							 << " rotation:\n" << R << "\n"
							 << " translation: " << t.transpose() << std::endl;


						 
						 
						 

						Similarity3 sim(posemetadata_s(R, -R.transpose() * t / S), S);
	

						
						 for (auto& img_it : images_)
						 {
							 Eigen::Matrix<double, 3, 3> rotation = img_it.second.GetRotationMatrix();
							 Eigen::Vector3d center = img_it.second.GetPosition();
							 posemetadata_s pose,result;
							 pose.center = center;
							 pose.rotation = rotation;
							 result = sim(pose);
							 img_it.second.SetPosition(result.center);
							 img_it.second.SetRotationMatrix(result.rotation);
						 }

				



						 
						 std::stringstream os;
						 for (auto& gcp :controlpoints_)
						 {
							 int  CPIndex = gcp.first;
							 
							 if (map_triangulation_errors.find(CPIndex) == map_triangulation_errors.end())
								 continue;

							 os
								 << "CP index: " << CPIndex << "\n"
								 << "CP triangulation error: " << map_triangulation_errors[CPIndex] << " pixel(s)\n"
								 << "CP registration error: "
								 << (sim(map_triangulated[CPIndex]) - map_control_points[CPIndex]).norm() << " user unit(s)" << "\n\n";
						 }
						 std::cout << os.str();

						 
					 }
					 else
					 {
						 LOGI("Registration failed. Please check your Control Points coordinates.");
						
					 }
				 }


			 }
		 }

		 bool ATData::IsEmpty()
		 {
			 return images_.empty();
				 
		 }

		 
		 bool  ATData::UnditortData(const std::string& path)
		 {
			 std::map<camera_t, Camera> undistrotcams;
			
			
			 auto& reg_image_ids = GetRegImageIds();
			 auto& images = GetImagesMutual();
			 auto& cameras = GetCamerasMutual();
			 for (auto& camera: cameras)
			 {
				
				 
				 
				 Camera cam = camera.second;
				 auto id = cam.GetCameraId();
				 if (undistrotcams.count(id))
				 {
					 continue;
				 }
				 Camera undiscam;			
				 
				 cam.GenUndistortCamera(undiscam);
				 undistrotcams[id] = undiscam;
				 
			 }
			 

			 auto imageids = GetImagesIds();
			 std::set<camera_t> camids;
			 std::set<image_t> imgids;
			 for (auto& id : imageids)
			 {
				 if (!images.count(id))
				 {
					 continue;
				 }
				 auto& image = images.at(id);
				 
				 {
					 image.SetPath(path);
					 image.SetName(std::to_string(image.GetImageId()) + "_" + image.GetName());

					 
					 for (point2D_t idx = 0; idx < image.GetNumPoints2D(); idx++)
					 {
						 auto& point2d = image.GetPoint2DMutual(idx);
						 point2d.SetXY(undistrotcams.at(image.GetCameraId()).WorldToImage(cameras.at(image.GetCameraId()).ImageToWorld(point2d.GetXY())));
					 }
					 for (auto& iter : image.GetPoints2DGCPMap())
					 {
						 auto pointnew = undistrotcams.at(image.GetCameraId()).WorldToImage(cameras.at(image.GetCameraId()).ImageToWorld(iter.second));
						 iter.second = pointnew;
					 }
					 for (auto& iter : image.GetUserPtsPoint2DMutual())
					 {
						 auto pointnew = undistrotcams.at(image.GetCameraId()).WorldToImage(cameras.at(image.GetCameraId()).ImageToWorld(iter.second));
						 iter.second = pointnew;
					 }

					 camids.insert(image.GetCameraId());
				 }

			 }
			 
			 cameras.clear();
			 for (auto& id : camids)
			 {
				 
				
				 cameras[id] = undistrotcams.at(id);
			 }
			 return true;
		 }
		 bool ATData::UndistortImages(const std::string& path)
		 {
			 std::map<camera_t, Camera> undistrotcams;
			
			 for (auto& id : reg_image_ids_)
			 {
				 AI3D::CORE::Image img = images_.at(id);
				 
				 Camera cam = cameras_.at(img.GetCameraId());
				 if (undistrotcams.count(cam.GetCameraId()))
				 {
					 continue;
				 }
				 Camera undiscam;
				 undiscam.SetModelIdFromName("PINHOLE");
				 undiscam.SetCameraModelType(CameraModelType_e::Perspective);
				 cam.GenUndistortCamera(undiscam);
				 undistrotcams[cam.GetCameraId()] = undiscam;
			 }
			 std::set<image_t> ids;
#ifdef USE_OPENMP
#pragma omp parallel  for
#endif
			
			 for(int i=0;i< reg_image_ids_.size();i++)
			 {
				 int id = reg_image_ids_[i];
				 AI3D::CORE::Image& image = images_.at(id);
				 Bitmap distorted_bitmap, undistorted_bitmap;
				 std::string imagepath = image.GetPath();
				 imagepath = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(imagepath));

				 const std::string input_image_path = File::JoinPaths(imagepath, image.GetName());
				 if (!distorted_bitmap.Read(input_image_path))
				 {
					 LOGE("Cannot read image at path " + input_image_path);
					 continue;
					
				 }
				 Camera undistorted_camera = undistrotcams.at(image.GetCameraId());
				 undistorted_bitmap.Allocate(static_cast<int>(undistorted_camera.GetWidth()),
					 static_cast<int>(undistorted_camera.GetHeight()),
					 distorted_bitmap.IsRGB());
				 distorted_bitmap.CloneMetadata(&undistorted_bitmap);
				
				 WarpImageBetweenCameras(cameras_.at(image.GetCameraId()), undistorted_camera, distorted_bitmap, &undistorted_bitmap);
				 std::string output_image_path = path + "/" + std::to_string(image.GetImageId()) + "_" + image.GetName();

				bool bsec= undistorted_bitmap.Write(output_image_path);
				if (bsec)
				{
					image.GetNameMutual() = output_image_path;
				}
#ifdef USE_OPENMP
#pragma omp critical
#endif
				{
					if (bsec)
					{
						ids.insert(id);
					}
				}
				 
			 }
			 for (auto& image : images_)
			 {
				 if (!ids.count(image.first))
				 {
					 images_.erase(image.first);
				 }
				 else
				 {
					 cameras_.at(image.second.GetCameraId()) = undistrotcams.at(image.second.GetCameraId());
				 }
				 
			 }
			 
		 }
		 void ATData::GetEpipolarLines(point2D_t img_id, point3D_t pt_id, std::map<int, std::pair<Eigen::Vector2d, Eigen::Vector2d>>& epipolarlines)
		 {
			
			 AI3D::CORE::Image image_base = images_[img_id];
			 Camera camera_base = cameras_[image_base.GetCameraId()];
			 auto& eles = controlpoints_[pt_id].GetObjectPointMutual().GetTrackMutual().GetElementsMutual();
			 for (auto& ele : eles)
			 {
				 if (ele.image_id != img_id)
				 {
					 AI3D::CORE::Image image = images_[ele.image_id];
					 Camera camera = cameras_[image.GetCameraId()];
					 Eigen::Vector2d xy = image.GetPoints2DGCP(pt_id);
					 if (xy.x() != -DBL_MAX && xy.y() != -DBL_MAX)
					 {
						 std::pair< Eigen::Vector2d, Eigen::Vector2d > line;
						 AlgorithmBase::GetEpipolarLine(xy, image.GetProjectionMatrix(),
							 image_base.GetProjectionMatrix(),
							 camera, camera_base,  line);
						 if (line.first.x() != -DBL_MAX && line.second.x() != -DBL_MAX)
						 {
							 epipolarlines[ele.image_id] = line;
						 }
					 }
				 }
			 }
			
		 }

		 void ATData::PredictUserPtMeasurement(point3D_t gcp_id, image_t img_id,
			 Eigen::Vector2d& estimated_xy, bool checkborder, bool istopredict)
		 {
			 if (!user_points3D_.count(gcp_id))
			 {
				 return;
			 }
			 istopredict = false;
			 Point3D& gcp = user_points3D_[gcp_id];

			 Eigen::Vector3d xyz;
			 if(gcp.HasEstimatedXYZ())
				xyz = gcp.GetEstimatedXYZ();
			 else
			 {
				 if (gcp.GetTrack().Length() >= VALIDMEASUREMENTNUM)
				 {
					 if (!ComputeUserPtEstimatedXYZ(gcp.GetId()))
						 return;
					 xyz = gcp.GetEstimatedXYZ();
				 }
			 }
			
			 auto& image = images_[img_id];
			 auto& camera = cameras_[image.GetCameraId()];
			 if (!image.IsPoseAndIntrinsicDefined(camera))
			 {
				 return;
			 }

			 
			 estimated_xy = AlgorithmBase::ProjectPointToImage(xyz,
				 image.GetProjectionMatrix(),
				 camera, checkborder);
		 }

		 void ATData::PredictGCPMeasurement(const point3D_t& gcp_id, image_t img_id,
			 Eigen::Vector2d& estimated_xy,bool checkborder,bool istopredict )
		 {
			 
			 ControlPoint& gcp = controlpoints_[gcp_id];
			 Eigen::Vector3d xyz = gcp.GetObjectPoint().GetXYZ();
			 if (istopredict)
			 {
				 if (gcp.GetObjectPoint().GetTrack().Length() >= VALIDMEASUREMENTNUM)
				 {
					 if (!gcp.HasEstimatedXYZ())
					 {
						 if (!ComputeGCPEstimatedXYZ(gcp.GetId()))
							 return;
					 }
					 xyz = gcp.GetObjectPoint().GetEstimatedXYZ();

				 }
			 }
			 auto& image = images_[img_id];
			 auto& camera = cameras_[image.GetCameraId()];
			 if (!image.IsPoseAndIntrinsicDefined(camera))
			 {
				 return;
			 }
			
			 
			 estimated_xy = AlgorithmBase::ProjectPointToImage(xyz,
				 image.GetProjectionMatrix(),
				 camera, checkborder);
		 }

		 void  ATData::ComputeAvgHeight()
		 {
			 image_t imgcount = images_.size();
			 if (imgcount == 0)
			 {
				 return;
			 }
			 int count = imgcount < 300 ? imgcount :300;
			 int step = imgcount / count;
			 double height_sum = 0.0;
			
			 int count_result = 0;			 
			 double z0 = images_.begin()->second.GetPosition().z();
			 std::vector<image_t> imgs = GetImagesIds();
			 for (int i = 0; i < count; i+=step)
			 {
				
				 double z = images_[imgs[i]].GetPosition().z() - z0;
				 height_sum += (z);
				 count_result++;
			 }
			
			 avg_height_ = (height_sum) / count_result + z0;

		 }

		 void ATData::PredictUserPtMeasurement(const point3D_t& gcp_id, std::map<image_t, Eigen::Vector2d >& estimated_xys)
		 {
			 if (!user_points3D_.count(gcp_id))
				 return;
			 
			 

			 ComputeAvgHeight();
			 if (avg_height_ == -DBL_MAX)
			 {
				 return;
			 }
			 auto& gcp = user_points3D_.at(gcp_id);
			 auto gcp_pos = gcp.GetXYZ();


			 for (auto& it : images_)
			 {
				 auto& image = it.second;
				 double dist_range = 3 * avg_height_;
				 
				 if (std::abs(gcp_pos[0] - image.GetPosition().x()) > dist_range ||
					 std::abs(gcp_pos[1] - image.GetPosition().y()) > dist_range ||
					 std::abs(gcp_pos[2] - image.GetPosition().z()) > dist_range)
				 {
					 continue;
				 }

				 auto& camera = cameras_[image.GetCameraId()];

				 Eigen::Vector2d emist_xy{ -DBL_MAX , -DBL_MAX };
				 PredictUserPtMeasurement(gcp_id, it.first, emist_xy);
				 
				 if (emist_xy.x() != -DBL_MAX)
				 {
					 estimated_xys[image.GetImageId()] = emist_xy;
				 }
			 }

		 }

		 void ATData::PredictUserPtMeasurement(const point3D_t& gcp_id, std::set<image_t>& imgids, bool btopredict_)
		 {
			 if (!user_points3D_.count(gcp_id))
				 return;
			 if (imgids.empty())
				 return;
			 auto& gcp = user_points3D_.at(gcp_id);

			 auto gcp_pos = gcp.GetXYZ();
			 if (gcp.GetTrack().Length() >= VALIDMEASUREMENTNUM)
			 {
				 if (gcp.HasEstimatedXYZ())
				 {
					 if (btopredict_)
					 {
						 gcp_pos = gcp.GetEstimatedXYZ();
					 }
				 }
			 }

			 auto image_ids = GetImagesIds();

			 for (int i = 0; i < image_ids.size(); i++)
			 {
				 const auto& image = images_[image_ids[i]];
				 
				 if (!image.HasRotationMatrix())
				 {
					 continue;
				 }

				 Eigen::Vector3d dir = (gcp_pos - image.GetPosition());
				 double dist = dir.norm();
				 double sita = dir.dot(image.ViewingDirection());
				 double a = sita / (dist * image.ViewingDirection().norm());
				 
				 a = std::abs(a);
				 double angle = acos(a) * 180 / M_PI;
				 auto& camera = cameras_[image.GetCameraId()];

				 
				 
				 
				 double fov = camera.GetFov();
				 if (angle >= fov)
				 {
					 continue;
				 }

				 Eigen::Vector2d emist_xy{ -DBL_MAX , -DBL_MAX };
				 PredictUserPtMeasurement(gcp_id, image.GetImageId(), emist_xy, true, btopredict_);
				 if (emist_xy.x() != -DBL_MAX )
				 {
					 imgids.insert(image.GetImageId());
				 }
			 }
		 }



		 
		 void ATData::PredictGCPMeasurement(const point3D_t& gcp_id, std::map<image_t, Eigen::Vector2d >& estimated_xys)
		 {
			 
			 if (!controlpoints_.count(gcp_id))
				 return;

			 

			 ComputeAvgHeight();
			 if (avg_height_ == -DBL_MAX)
			 {
				 return;
			 }
			
			 auto& gcp = controlpoints_.at(gcp_id);
			 auto gcp_pos = gcp.GetObjectPoint().GetXYZ();
			
			 
			 for (auto& it : images_)
			 {
				 auto& image = it.second;
				 double dist_range = 3 * avg_height_;
				 
				 if (std::abs(gcp_pos[0] - image.GetPosition().x()) > dist_range || 
					 std::abs(gcp_pos[1] - image.GetPosition().y()) > dist_range || 
					 std::abs(gcp_pos[2] - image.GetPosition().z()) > dist_range)
				 {
					 continue;
				 }
						
				 auto& camera = cameras_[image.GetCameraId()];
				 
				 Eigen::Vector2d emist_xy{ -DBL_MAX , - DBL_MAX };
				 PredictGCPMeasurement(gcp_id, it.first, emist_xy);
				 
				 if (emist_xy.x() != -DBL_MAX)
				 {
					 estimated_xys[image.GetImageId()] = emist_xy;
				 }
			 }
			
		 }
		 
		 void ATData::PredictGCPMeasurement(const point3D_t& gcp_id, std::set<image_t>& imgids, bool btopredict_)
		 {
			 auto& gcp = controlpoints_[gcp_id];

			 auto gcp_pos = gcp.GetObjectPoint().GetXYZ();
			 if (gcp.GetObjectPoint().GetTrack().Length() >= VALIDMEASUREMENTNUM)
			 {
				 if (gcp.HasEstimatedXYZ())
				 {
					 if (btopredict_)
					 {
						 gcp_pos = gcp.GetObjectPoint().GetEstimatedXYZ();
					 }
				 }
			 }

			 auto image_ids = GetImagesIds();
	
			 for (int i = 0; i < image_ids.size(); i++)
			 {
				 const auto& image = images_[image_ids[i]];
				 
				 if (!image.HasRotationMatrix())
				 {
					 continue;
				 }

				 Eigen::Vector3d dir = (gcp_pos - image.GetPosition());
				 double dist = dir.norm();
				 double sita = dir.dot(image.ViewingDirection());
				 double a = sita / (dist * image.ViewingDirection().norm());
				 
				 a = std::abs(a);
				 double angle = acos(a) * 180 / M_PI;
				 auto& camera = cameras_[image.GetCameraId()];

				 
				 
				 
				 double fov = camera.GetFov();
				 if (angle >= fov)
				 {
					 continue;
				 }

				 Eigen::Vector2d emist_xy{ -DBL_MAX , -DBL_MAX };
				 PredictGCPMeasurement(gcp_id, image.GetImageId(), emist_xy, true, btopredict_);
				 if (emist_xy.x() != -DBL_MAX )
				 {
					 imgids.insert(image.GetImageId());
				 }
			 }
		 }
		 
		 srs_s ATData::GetDefaultEnuSRS()
		{
			 srs_s srs;
			if (CoordinateDescriptor::GetSRSFromDefinition(local_srs_definition_).type == LOCAL)
			{
				return srs;
			}

			if (CoordinateDescriptor::GetSRSFromDefinition(local_srs_definition_).type != LOCAL_ENU  )
			{
				Eigen::Vector3d offset;
				if (ComputePositionOffsetByAvgCenter(offset, srs.definition))
				{
					srs = CoordinateDescriptor::GetSRSFromDefinition(srs.definition);
				}
			}
			else
				srs = CoordinateDescriptor::GetSRSFromDefinition(local_srs_definition_);;
			return srs;
		}

		 bool ATData::ComputePositionOffsetByAvgCenter(
			 Eigen::Vector3d& position_offset)
		 {
			 if (!HasPositionImages())
			 {
				 return false;
			 }
			 std::vector<image_t> img_idx = GetHasPostionImagesIds();



			 Eigen::Vector3d sum = Eigen::Vector3d::Zero();
			 Eigen::Vector3d point_first = images_[*img_idx.begin()].GetPosition();

			 for (int i = 0; i < img_idx.size(); i++)
			 {
				 sum += (images_[img_idx[i]].GetPosition() - point_first);
			 }
			 position_offset = sum / img_idx.size() + point_first;

			 return true;
		 }
		 
		bool ATData::ComputePositionOffsetByAvgCenter(
			Eigen::Vector3d& position_offset, std::string& local_srs_definition)
		{
			
			ComputePositionOffsetByAvgCenter(position_offset);
			
			CoordinateTransformer::Transform(1, &position_offset[0],
				&position_offset[1], &position_offset[2],
				local_srs_definition_, GEO84SRS);
			position_offset[2] = 0.0;

			char buf[1024];
			sprintf(buf, "%.5f,%.5f", position_offset[1], position_offset[0]);
			std::string strlb(buf);
			local_srs_definition = "ENU:" + strlb;
			return true;
		}

		
		
		bool ATData::HasPriorPositionImages() const
		{
			return GetHasPriorPostionImagesIds().size() > 0;
		}
		void ATData::DeleteGCP(const point3D_t gcp_id)
		{
			if (!ExistsGCP(gcp_id))
			{
				return;
			}
			for (auto& it : controlpoints_[gcp_id].GetObjectPointMutual().GetTrackMutual().GetElements())
			{
				image_t image_id = it.image_id;
				images_[image_id].DeleteGCPMeasurement(gcp_id);
			}
			controlpoints_.erase(gcp_id);
		}
		void ATData::DeleteUserPt(const point3D_t& point_id)
		{
			if (!ExistsUserPt(point_id))
			{
				return;
			}
			for (auto& it : user_points3D_[point_id].GetTrackMutual().GetElements())
			{
				image_t image_id = it.image_id;
				images_[image_id].DeleteUserPtMeasurement(point_id);
			}
			user_points3D_.erase(point_id);
		}

		void ATData::DeleteUserPtMeasurement(point3D_t point_id, image_t img_id, std::map < image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >& measurement_error_map_)
		{
			if (img_id == kInvalidImageId)
				return;
			if (point_id == kInvalidPoint3DId)
				return;
			if (!images_.count(img_id))
			{
				return;
			}
			if(!user_points3D_.count(point_id))
			{
				return;
			}
			AI3D::CORE::Image& image = images_[img_id];

			image.DeleteUserPtMeasurement(point_id);
			auto& objectpoint = user_points3D_.at(point_id);
			objectpoint.GetTrackMutual().DeleteElementByImageId(img_id);

			objectpoint.SetPixelRMS(kInvalidError);
			objectpoint.SetDistRMS(kInvalidError);
			Eigen::Vector3d estim_xyz = { -DBL_MAX ,-DBL_MAX ,-DBL_MAX };
			objectpoint.SetEstimatedXYZ(estim_xyz);
		
			
			ComputeGCPEstimatedXYZ(point_id);
			UpdataUserPtErrorInfo(point_id, measurement_error_map_);

		}

		
		void ATData::DeleteGCPMeasurement(point3D_t gcp_id, image_t img_id, std::map < image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >& measurement_error_map_)
		{
			if (img_id == kInvalidImageId)
				return;
			if (gcp_id == kInvalidPoint3DId)
				return;
			
			if (!images_.count(img_id))
			{
				return;
			}
			if (!controlpoints_.count(gcp_id))
			{
				return;
			}

			AI3D::CORE::Image& image = images_[img_id];

			image.DeleteGCPMeasurement(gcp_id);
			auto& objectpoint = controlpoints_[gcp_id].GetObjectPointMutual();
			objectpoint.GetTrackMutual().DeleteElementByImageId(img_id);
			
			objectpoint.SetPixelRMS(kInvalidError);
			objectpoint.SetDistRMS(kInvalidError);
			Eigen::Vector3d estim_xyz = { -DBL_MAX ,-DBL_MAX ,-DBL_MAX };
			objectpoint.SetEstimatedXYZ(estim_xyz);

			controlpoints_[gcp_id].SetEstimatedXYZ(estim_xyz);
			controlpoints_[gcp_id].Set3DError(kInvalidError);
			controlpoints_[gcp_id].SetXY3DError(kInvalidError);
			controlpoints_[gcp_id].SetZ3DError(kInvalidError);
			
			ComputeGCPEstimatedXYZ(gcp_id);
			UpdataGCPErrorInfo(gcp_id, measurement_error_map_);
			
		}


		void ATData::DeleteGCPs() {
		
			
		
			for (auto it = controlpoints_.begin(); it != controlpoints_.end();it++)
			{
				 auto elements = it->second.GetObjectPointMutual().GetTrackMutual().GetElements();
				 for (const auto& perele: elements)
				 {
					 image_t image_id = perele.image_id;
					 images_[image_id].DeleteGCPMeasurement(it->second.GetId());
				 }

				
			}

			controlpoints_.clear();
		}





		void ATData::DeletePoint3D(const point3D_t point3D_id) 
		{
			
			
			if (!points3D_.count(point3D_id))
				return;
			const Track& track = GetPoint3D(point3D_id).GetTrack();

			const bool kIsDeletedPoint3D = true;

			

			for (const auto& track_el : track.GetElements()) {
				AI3D::CORE::Image& image = GetImageMutual(track_el.image_id);
				image.ResetPoint3DForPoint2D(track_el.point2D_idx);
			}

			points3D_.erase(point3D_id);
		}

		void ATData::DeleteConstraint(const constraint_t constraint_id) {
			if (!constraintList_.count(constraint_id))
				return;
			MeasureConstraint measureConstraint = GetConstraint(constraint_id);
			measureConstraint.GetPointList().clear();
			measureConstraint.GetConstraintItemList().clear();

			constraintList_.erase(constraint_id);
		}
		void ATData::ResetTriObservations(const image_t image_id,
			const point2D_t point2D_idx,
			const bool is_deleted_point3D) 
		{
			
			
			
			

			
			
			
			

			
			

			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
		}

		
		void ATData::ClearPoints2D(image_t imageid)
		{
			auto& image = images_.at(imageid);
			int deletecnt = 0;
			for (point2D_t point2D_idx = 0; point2D_idx < image.GetNumPoints2D();
				++point2D_idx)
			{
				if (image.GetPoint2D(point2D_idx).HasPoint3D())
				{
					DeleteObservation(imageid, point2D_idx);
					deletecnt++;
				}
			}
			if (deletecnt > 0)
			{
				btiepoints_changed_ = true;
			}
		}

		
		void ATData::ClearPose(const std::set<image_t>& imageids)
		{
			for (auto id : imageids)
			{
				if (images_.count(id))
				{
					auto& image = images_.at(id);
					image.ClearPose();
					
					
					ClearPoints2D(id);
					reg_image_ids_.erase(
						std::remove(reg_image_ids_.begin(), reg_image_ids_.end(), id),
						reg_image_ids_.end());
				}
				
			}
			if (!HasPositionImages())
			{
	
				{
					SetLocalSrs(LOCALSRS);
				}
			}
			
			
			
			
			
			
			

		}
		void ATData::DeleteObservation(const image_t image_id,
			const point2D_t point2D_idx) 
		{
			AI3D::CORE::Image& image = GetImageMutual(image_id);
			
			
			const point3D_t point3D_id = image.GetPoint2D(point2D_idx).GetPoint3DId();

			Point3D& point3D = GetPoint3DMutual(point3D_id);
			point3D.SetStatus(true);
			
			point3D.GetTrackMutual().DeleteElement(image_id, point2D_idx);
			
			
			 image.ResetPoint3DForPoint2D(point2D_idx);
			
			if (point3D.GetTrack().Length() <= 2) 
			{
				DeletePoint3D(point3D_id);
				return;
			}

			

			const bool kIsDeletedPoint3D = false;
			
			

			
		}
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		

		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		

		
		
		
		
		
		

		
		
		
		
		
		
		
		
		
		
		
		
		

		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		void ATData::ExtractATDataByImages(std::set<image_t> ids, ATData& data)
		{


			
			std::set<image_t> mids;
			EIGEN_STL_UMAP(point3D_t, Point3D) tps;
			for (auto& iter : ids)
			{
				image_t imgid = (iter);
				if (!images_.count(imgid))
					continue;
				auto image = images_.at(imgid);
				if (image.IsRegistered())
				{
					mids.insert(imgid);
				}
				data.AddImage(image);
				auto cam = cameras_.at(image.GetCameraId());
				data.AddCamera(cam);
				for (auto& pt : image.GetPoints2D())
				{
					auto ptid = pt.GetPoint3DId();
					if (!points3D_.count(ptid))
						continue;
					Point3D pointtemp = points3D_.at(ptid);

					TrackElement* tel = pointtemp.GetTrackMutual().FindElementByImageIdMutual(imgid);
					if (!tel)
					{
						continue;
					}
					TrackElement ele = *tel;
					if (tps.count(ptid))
					{

						auto& eles = tps.at(ptid).GetTrackMutual().GetElementsMutual();
						eles.push_back(ele);

					}
					else
					{



						auto& eles = pointtemp.GetTrackMutual().GetElementsMutual();
						eles.clear();
						eles.push_back(ele);
						tps[ptid] = pointtemp;
					}
				}
			}

			std::vector<image_t> vids;
			vids.assign(mids.begin(), mids.end());
			if (!mids.empty())
			{
				data.SetRegImageIds(vids);
			}
			data.SetPoint3D(tps);
			data.SetLocalSrs(local_gcp_srs_definition_);
			data.SetOriginSrs(origin_srs_definition_);

		}
		void ATData::ExtractATDataByTiepoints(std::set<point3D_t> ids, ATData& data)
		{
			
				EIGEN_STL_UMAP(point3D_t, Point3D) tps;
				std::set<image_t> mids;
				for (auto& iter : ids)
				{
					if (!points3D_.count(iter))
						continue;
					
					tps[iter] = points3D_.at(iter);
					
					
				
					for (auto& ele : points3D_.at(iter).GetTrack().GetElements())
					{
						auto imgid = ele.image_id;
						if (!images_.count(imgid))
							continue;
						auto image = images_.at(imgid);
						if (image.IsRegistered())
						{
							mids.insert(imgid);
						}
						data.AddImage(image);
						auto cam = cameras_.at(image.GetCameraId());
						data.AddCamera(cam);
					}
				}
				data.SetPoint3D(tps);
				std::vector<image_t> vids;
				vids.assign(mids.begin(), mids.end());
				if (!mids.empty())
				{
					data.SetRegImageIds(vids);
				}
				data.SetLocalSrs(local_gcp_srs_definition_);
				data.SetOriginSrs(origin_srs_definition_);
		}

		void ATData::UpdateImageObrs(std::set<image_t> imageids)
		{
			for (auto image_id : imageids)
			{
				AI3D::CORE::Image& image = GetImageMutual(image_id);
				ClearPoints2D(image_id);			

				image.SetRegistered(false);

				reg_image_ids_.erase(
					std::remove(reg_image_ids_.begin(), reg_image_ids_.end(), image_id),
					reg_image_ids_.end());

			}
			
			
		}

		
		void ATData::DeleteImage(const image_t image_id) 
		{
			AI3D::CORE::Image& image = GetImageMutual(image_id);
			ClearPoints2D(image_id);
			

			image.SetRegistered(false);

			reg_image_ids_.erase(
				std::remove(reg_image_ids_.begin(), reg_image_ids_.end(), image_id),
				reg_image_ids_.end());

			
			
			auto& gcps_currentimage = image.GetGcpsPoint2DMutual();
			for (auto& it : gcps_currentimage)
			{
				controlpoints_[it.first].GetObjectPointMutual().GetTrackMutual().DeleteElementByImageId(image_id);
			}
			
			images_.erase(image_id);
		}

		bool ATData::HasImages() const
		{
			return GetNumImages() > 0;
		}


		bool ATData::GetSceneUnit() const
		{
			bool hasposimages = HasAbsPositionImages();
			return hasposimages;
			
		}
		bool ATData::HasAbsPriorPositionImages() const
		{
			
			
			{
				return HasPriorPositionImages();
			}
			return false;
		}
		bool ATData::Empty() const
		{
			return images_.empty();
		}

		bool ATData::HasAbsPositionImages() const
		{
			if (CoordinateDescriptor::GetSRSFromDefinition(local_srs_definition_).type != LOCAL)
			{
				return HasPositionImages() ;
			}
			return false;
		}
		
		bool ATData::HasPositionImages() const
		{
			return GetHasPostionImagesIds().size() > 0;
		}
		bool ATData::HasRegImages() const
		{
			return GetNumRegImages() > 0;
		}
		
		void ATData::SetTileAABBBox(const ABBox3f& box)
		{
			tile_aabb_box_ = box;
		}
		const ABBox3f& ATData::GetTileAABBBox() const
		{
			return tile_aabb_box_;
		}
		ABBox3f& ATData::GetTileAABBBoxMutual()
		{
			return tile_aabb_box_;
		}
		void ATData::SetTightBox(const ABBox3f& tight_box)
		{
			tight_box_ = tight_box;
		}
		const ABBox3f& ATData::GetTightBox() const
		{
			return tight_box_;
		}
		ABBox3f& ATData::GetTightBoxMutual()
		{
			return tight_box_;
		}

		

		bool ATData::HasTiepoints() const
		{
			return !GetPoint3DIds().empty();
		}

		void ATData::TransFormATData(const std::string& dst_srs)
		{
			std::string tgtsrs = dst_srs;
			srs_s atlocal_srs = CoordinateDescriptor::GetSRSFromDefinition(local_srs_definition_);
			if (atlocal_srs.type == LOCAL || atlocal_srs.type == Unsupported)
			{
				return ;
			}
			if (CoordinateTransformer::IsSame(tgtsrs, atlocal_srs.definition))
			{
				return;
			}
			
			
			 TransFormTiepoints(local_srs_definition_, tgtsrs);
		
			TransFormImages(local_srs_definition_, tgtsrs);
			TransFormGCPs(local_srs_definition_, tgtsrs);
			SetLocalSrs(tgtsrs);
			
			return ;
		}

		bool ATData::TransFormTiepoints(std::string src,std::string dst)
		{

			if (!HasTiepoints())
				return false;
			
			if (CoordinateTransformer::IsSame(src, dst))
			{
				return true;
			}

			if (CoordinateDescriptor::GetSRSFromDefinition(src).type == LOCAL
				|| CoordinateDescriptor::GetSRSFromDefinition(dst).type == LOCAL)
			{
				return false;
			}
			point3D_t ptcount = GetPoint3DIds().size();
						
			std::vector<double> x(ptcount), y(ptcount), z(ptcount);
			
			point3D_t i_pt = 0;
			for (auto& it : GetPoints3D())
			{
				x[i_pt] = it.second.GetX();
				y[i_pt] = it.second.GetY();
				z[i_pt] = it.second.GetZ();
				i_pt++;
			}
			CoordinateTransformer::Transform(x.size(),&x[0],&y[0],
				&z[0], src, dst);
			i_pt = 0;
			
			for (auto& it : GetPoints3DMutual())
			{
				it.second.SetXYZ(Eigen::Vector3d(x[i_pt],y[i_pt],z[i_pt]));
				i_pt++;
			}
		
			
			return true;
		}

		
		bool ATData::TransFormGCPs(std::string src, std::string dst)
		{
			if (CoordinateTransformer::IsSame(src, dst))
			{
				return true;
			}
			if (!HasControlPoints())
				return false;
			if (CoordinateDescriptor::GetSRSFromDefinition(src).type == LOCAL
				|| CoordinateDescriptor::GetSRSFromDefinition(dst).type == LOCAL)
			{
				return false;
			}
			std::vector<double> x, y, z;
			x.reserve(controlpoints_.size());
			y.reserve(controlpoints_.size());
			z.reserve(controlpoints_.size());
			std::vector<point3D_t> ids;
			for (auto& it : controlpoints_)
			{
				x.emplace_back(it.second.GetObjectPoint().GetXYZ().x());
				y.emplace_back(it.second.GetObjectPoint().GetXYZ().y());
				z.emplace_back(it.second.GetObjectPoint().GetXYZ().z());
				ids.emplace_back(it.first);
				
			}
			CoordinateTransformer::Transform(x.size(), &x[0], &y[0], &z[0], src, dst);
			for (point3D_t i= 0;i<ids.size();i++)
			{
				controlpoints_[ids[i]].GetObjectPointMutual().GetXYZMutual().x() = x[i];
				controlpoints_[ids[i]].GetObjectPointMutual().GetXYZMutual().y() = y[i];
				controlpoints_[ids[i]].GetObjectPointMutual().GetXYZMutual().z() = z[i];
				
			}
			


			return true;
		}



		
		
		bool ATData::TransFormImages(std::string src, std::string dst)
		{

			if (!HasPositionImages())
			{
				return false;
			}
			if (CoordinateDescriptor::GetSRSFromDefinition(src).type == LOCAL
				|| CoordinateDescriptor::GetSRSFromDefinition(dst).type == LOCAL)
			{
				return false;
			}
			if (CoordinateTransformer::IsSame(src, dst))
			{
				return true;
			}

			std::vector<image_t> img_idx = GetHasPostionImagesIds();

			srs_s dst_crs = CoordinateDescriptor::GetSRSFromDefinition(dst);
			srs_s src_crs = CoordinateDescriptor::GetSRSFromDefinition(src);

			

			
			std::vector<Eigen::Vector3d> poses;
			std::vector<Eigen::Matrix3d> rotations;
			for (int i = 0; i < img_idx.size(); i++)
			{
				poses.push_back(images_[img_idx[i]].GetPositionMutual());
			
				
				{
					rotations.push_back(images_[img_idx[i]].GetRotationMatrixMutual());
				}
				

			}

			CoordinateTransformer::TransformRotation(poses.size(),poses, rotations,src_crs, dst_crs);
			for (int64_t i = 0; i < img_idx.size(); i++)
			{
				images_[img_idx[i]].GetPositionMutual() = poses[i];
				
			}


			
			{
				for (int64_t i = 0; i < img_idx.size(); i++)
				{
					images_[img_idx[i]].GetRotationMatrixMutual() = rotations[i];
					
				}
			}
			


			return true;


		}


		bool ATData::AddPoses(srs_s srs, std::vector<pose_s>  poses, std::vector<pose_s>& image_remain)
		{
			LOGI(String::StringPrintf("Add %d Poses...", poses.size()));
			std::string srs_def = local_srs_definition_;
			if (local_srs_definition_ == LOCALSRS)
			{
				srs_def = BASESRS;
				local_srs_definition_ = BASESRS;
			}
			
			srs_s dst_crs = CoordinateDescriptor::GetSRSFromDefinition(srs_def);
			
			std::vector<pose_s>  poses_srs(poses);
			std::vector<Eigen::Vector3d> poses_temp(poses_srs.size());
			std::vector<Eigen::Matrix3d> rotations_temp(poses_srs.size());
			for (image_t i = 0; i < poses_srs.size(); i++)
			{
				Eigen::Vector3d xyz = poses_srs[i].metadata_.center;
				Eigen::Matrix3d R = poses_srs[i].metadata_.rotation;
				poses_temp[i]=(xyz);
				
				{
					rotations_temp[i]=(R);
				}
			}
			bool ret = false;
			ret = CoordinateTransformer::TransformRotation(poses_temp.size(), poses_temp, rotations_temp,
				srs, dst_crs);
			if (!ret)
			{
				LOGI("transform pose(s) and rotation(s) failed.");
				return ret;
			}
			else
			{
				LOGI("transform pose(s) and rotation(s) success.");
			}
			

			for (image_t i = 0; i < poses.size(); i++)
			{
				LOGI("----input pos :" + std::to_string(i));
				AI3D::CORE::Image* image = nullptr;
				
				std::string name = poses[i].name;
				name = File::EnsureUnifySlash(name);
				bool bFullname = false;
				if (name.find_last_of("/") != std::string::npos)
				{
					bFullname = true;
				}
				if (bFullname)
				{
					image = FindImageWithFullName(name);
				}
				else
				{
					image = FindImageWithName(name,GetImageIds());
				}
				if (image != nullptr)
				{
					if (poses_srs[i].metadata_.center.x() != -DBL_MAX)
					{
						
						
						image->SetPriorSrs(srs);

						image->GetPositionPriorMutual() = poses_srs[i].metadata_.center;
						if (poses_srs[i].metadata_.rotation != Eigen::Matrix3d::Zero())
						{
							image->GetRotationMatrixPriorMutual() = poses_srs[i].metadata_.rotation;
						}
					}
					if (poses_temp[i].x() != -DBL_MAX)
					{
						image->GetPositionMutual() = poses_temp[i];
						if (rotations_temp[i]!= Eigen::Matrix3d::Zero())
						{
							image->GetRotationMatrixMutual() = rotations_temp[i];
						}
					}
					ClearPoints2D(image->GetImageId());
				}
				else
				{
					if (poses[i].metadata_.center.x() != -DBL_MAX)
					{
						image_remain.push_back(poses[i]);
					}
					
				}
			}
			

			return true;
		}
		void ATData::DeleteTiePoints(const std::vector<point3D_t>& ids, std::vector<image_t>& delteImageID)
		{
			std::set<point3D_t> ptid_todelete;
			std::set<image_t> imgid_posslibletodelete;

			for (auto& point3D_id : ids)
			{
				
				if (points3D_.count(point3D_id))
				{
					ptid_todelete.insert(point3D_id);
				}

				const Track& track = GetPoint3D(point3D_id).GetTrack();


				for (const auto& track_el : track.GetElements())
				{
					imgid_posslibletodelete.insert(track_el.image_id);

				}
			}

			if (ptid_todelete.empty())
			{
				return;
			}


			for (auto& point3D_id : ids)
			{
				DeletePoint3D(point3D_id);
			}

			int count = 0;

			for (auto& iter : imgid_posslibletodelete)
			{


				if (!GetImagesMutual().count(iter))
				{
					continue;
				}
				auto& image = GetImagesMutual()[iter];;
				if (image.GetNumPoints3D() < 3)
				{
					
					delteImageID.push_back(iter);
				}
				else
				{
					count++;

				}

			}
			return;
		}
		
		bool ATData::SetMetadataToCenter()
		{
			if (HasPositionImages())
			{
				return false;
			}
			if (!HasPriorPositionImages())
			{
				return false;
			}

			std::vector<image_t> img_idx = GetHasPriorPostionImagesIds();

			srs_s dst_crs;
			if (local_srs_definition_ == LOCALSRS)
			{
				local_srs_definition_ = BASESRS;
			}
			dst_crs.definition = local_srs_definition_;
			dst_crs.type = GEOCENTRIC;



			std::map<srsid_t, std::vector<image_t>> imgs_temp;
			for (int i = 0; i < img_idx.size(); i++)
			{
				srs_s src_crs = images_[img_idx[i]].GetPriorSrs();
				imgs_temp[src_crs.ID].push_back(img_idx[i]);
			}
			for (auto& it : imgs_temp)
			{
				std::vector<Eigen::Vector3d> poses_temp(it.second.size());
				std::vector<Eigen::Matrix3d> rotations_temp(it.second.size());;
				for (image_t i = 0; i < it.second.size(); i++)
				{
					Eigen::Vector3d xyz = images_[it.second[i]].GetPositionPrior();
					Eigen::Matrix3d R = images_[it.second[i]].GetRotationMatrixPrior();
					poses_temp[i]=(xyz);
					
					{
						rotations_temp[i]=(R);
					}

				}

				CoordinateTransformer::TransformRotation(poses_temp.size(), poses_temp, rotations_temp,
					images_[it.second[0]].GetPriorSrs(), dst_crs);

				for (image_t i = 0; i < it.second.size(); i++)
				{
					if (!poses_temp.empty())
					{
						images_[it.second[i]].GetPositionMutual() = poses_temp[i];
						
						
						{
							images_[it.second[i]].GetRotationMatrixMutual() = rotations_temp[i];
							
						}
					}

					
				}
			}


			
			
			
			
			
			

			
			return true;
		}

		camera_t ATData::GenerateValidCameraId()
		{
			camera_t cam_id = kInvalidCameraId;
			std::set<camera_t> cam_ids;
			for (auto& it : cameras_)
			{
				cam_ids.insert(it.first);
			}
			if (!cam_ids.empty())
			{
				cam_id = *cam_ids.rbegin();
			}
			cam_id++;
			
			return cam_id;
		}

		point3D_t ATData::GenerateValidPoint3DId()
		{
			point3D_t point_id = kInvalidPoint3DId;
			std::set<point3D_t> point_ids;
			for (auto& it : points3D_)
			{
				point_ids.insert(it.first);
			}
			if (!point_ids.empty())
			{
				point_id = *point_ids.rbegin();
			}
			point_id++;

			return point_id;
		}
		bool ATData::HasValidPriorPositionImages() const
		{
			return GetHasPriorPostionImagesIds().size() > 2;
		}

		inline const std::set<image_t> ATData::GetImageIdsSet() const
		{
			std::set<image_t> ids;
			for (auto& image : images_)
			{
				ids.insert(image.second.GetImageId());
			}
			return ids;
		}

		inline const std::vector<image_t> ATData::GetImageIds() const
		{
			std::vector<image_t> ids;
			for (auto& image : images_)
			{
				ids.push_back(image.second.GetImageId());
			}
			return ids;
		}
		
		AT_complete_status_e  ATData::GetATCompleteStatus()
		{
			if (GetNumRegImages() < 2)
				return AT_complete_status_e::INCOMPLETE_PHOTOS;
			else if (GetNumRegImages() >1 && GetNumRegImages() < GetNumImages())
				return AT_complete_status_e::PARTIALLY_COMPLETE_PHOTOS;
			else if ( GetNumRegImages() == GetNumImages())
				return AT_complete_status_e::COMPLETE_PHOSTOS;

		}
		bool ATData::AreAllImagesPoseComplete() const
		{
			
			auto ids = GetImageIds();
			return std::all_of(ids.begin(), ids.end(), [&](image_t id) {return images_.at(id).IsPoseCompleted(); });
		}


		bool ATData::AreAllImagesRegistered() const
		{
			
			auto ids = GetImageIds();
			return std::all_of(ids.begin(), ids.end(), [&](image_t id) {return images_.at(id).IsRegistered(); });
		}
		point3D_t ATData::GenerateValidGCPId()
		{
			point3D_t point_id = kInvalidPoint3DId;
			std::set<point3D_t> point_ids;
			for (auto& it : controlpoints_)
			{
				point_ids.insert(it.first);
			}
			if (!point_ids.empty())
			{
				point_id = *point_ids.rbegin();
			}
			point_id++;

			return point_id;
		}


		point3D_t ATData::GenerateValidUserPtId()
		{
			point3D_t point_id = kInvalidPoint3DId;
			std::set<point3D_t> point_ids;
			for (auto& it : user_points3D_)
			{
				point_ids.insert(it.first);
			}
			if (!point_ids.empty())
			{
				point_id = *point_ids.rbegin();
			}
			point_id++;

			return point_id;
		}

		constraint_t ATData::GenerateValidConstraintId() {
			constraint_t constraint_id = kInvalidMerasureId;
			std::set<constraint_t> constraint_ids;
			for (auto& it : constraintList_)
			{
				constraint_ids.insert(it.first);
			}
			if (!constraint_ids.empty())
			{
				constraint_id = *constraint_ids.rbegin();
			}
			constraint_id++;

			return constraint_id;
		}

		
		camera_t ATData::ExistsCamera(const Camera& camera)
		{
			for (auto& cam : cameras_)
			{
				if(cam.second.IsSame(camera))
				{
					return cam.second.GetCameraId();
				}
			}
			return kInvalidCameraId;
		}
		
		

		
		bool ATData::TransformControlPoints(std::string& crs_definition)
		{
			
			if (!HasControlPoints())
				return false;

			ControlPoints gcps;
			for (auto& it : GetControlPointsMutual())
			{
				gcps.ADDPoint(it.second);
			}

			std::string definition = crs_definition;
			gcps.TransformPointsToBaseCoordinate(definition);
		
			for (auto& it : GetControlPointsMutual())
			{
				GetControlPointsMutual()[it.second.GetId()].GetObjectPointMutual().GetXYZMutual() 
					= gcps.GetPoint(it.second.GetId()).GetObjectPointMutual().GetXYZMutual();
			
			}
		
			return true;
		}
		void ATData::SetLocalGcpSrs(std::string local_gcp_srs_definition)
		{
			local_gcp_srs_definition_ = local_gcp_srs_definition;
		}
		const std::string  ATData::GetLocalGcpSrs()  const
		{
			return local_gcp_srs_definition_;
		}



		void ATData::TearDown() 
		{
			

			
			std::unordered_set<camera_t> keep_camera_ids;
			for (auto it = images_.begin(); it != images_.end();) 
			{
				if (it->second.IsRegistered())
				{
					keep_camera_ids.insert(it->second.GetCameraId());
					it->second.TearDown();
					++it;
				}
				else 
				{
					it = images_.erase(it);
				}
			}

			
			for (auto it = cameras_.begin(); it != cameras_.end();) 
			{
				if (keep_camera_ids.count(it->first) == 0)
				{
					it = cameras_.erase(it);
				}
				else 
				{
					++it;
				}
			}

			
			
			for (auto& point3D : points3D_) 
			{
				point3D.second.GetTrackMutual().Compress();
			}
			
		}
		bool ATData::HasUnRegisteredImages()
		{
			return (reg_image_ids_.size() < GetNumImages());
		}
		void ATData::AddCamera(const Camera& camera) 
		{
			
			if (ExistsCamera(camera.GetCameraId()))
				return;
			CHECK_OPTION(camera.VerifyParams());
			cameras_.emplace(camera.GetCameraId(), camera);
		}

		inline bool ATData::ExistsUserPt(const point3D_t point3D_id) const
		{
			return user_points3D_.find(point3D_id) != user_points3D_.end();
		}

		inline bool ATData::ExistsConstraint(const constraint_t constraint_id) const {
			return constraintList_.find(constraint_id) != constraintList_.end();
		}
		inline bool ATData::ExistsGCP(const point3D_t point3D_id) const
		{
			return controlpoints_.find(point3D_id) != controlpoints_.end();
		}

		void ATData::AddImage(const AI3D::CORE::Image& image)
		{
			
			if (ExistsImage(image.GetImageId()))
				return;
			images_[image.GetImageId()] = image;
		}
		

		

		image_t ATData::GetImageId(const std::string& image_path)
		{
			if (image_path_to_id_.empty())
			{
				for (auto it = images_.cbegin(); it != images_.end(); ++it) 
				{
					image_path_to_id_[it->second.GetName()] = it->first;
				}
			}

			if (image_path_to_id_.find(image_path) != image_path_to_id_.end()) 
			{
				return image_path_to_id_.at(image_path);
			}

			return kInvalidImageId;
		}

		AI3D::CORE::Image* ATData::FindImageWithFullName(
			const std::string& name) 
		{
			std::string comname = File::EnsureUnifySlash(name);
			String::StringToLower(&comname);
			for (auto& elem : images_)
			{
				std::string fullname = elem.second.GetPath() + "/" + elem.second.GetName();
				fullname = File::EnsureUnifySlash(fullname);
				String::StringToLower(&fullname);
				if (fullname == name)
				{
					return &elem.second;
				}
			}
			return nullptr;
		}

		
		 const AI3D::CORE::Image* ATData::FindImageWithFullName(
			const std::string& name) const
		{
			 std::string comname = File::EnsureUnifySlash(name);
			 String::StringToLower(&comname);
			 for (const auto& elem : images_)
			 {
				 std::string fullname = elem.second.GetPath() + "/" + elem.second.GetName();
				 fullname = File::EnsureUnifySlash(fullname);
				 String::StringToLower(&fullname);
				 if (fullname == name)
				 {
					 return &elem.second;
				 }
			 }
			return nullptr;
		}
		 AI3D::CORE::Image* ATData::FindImageWithFullName(
			 const std::string& name, std::vector<image_t> imgs_ids)
		 {
			 
			 for (auto& it : imgs_ids)
			 {
				 if (!images_.count(it))
				 {
					 continue;
				 }
				 AI3D::CORE::Image& elem = images_[it];
				 std::string imgname = elem.GetPath() + "/"+elem.GetName();
				 File::EnsureUnifySlash(imgname);
				 std::string root, ext;
				 String::StringToLower(&imgname);
				 if (imgname.find_last_of(".") != std::string::npos)
					 imgname.erase(imgname.find_last_of("."));
				 
				 std::string comp_name = name;
				 File::EnsureUnifySlash(comp_name);
				 String::StringToLower(&comp_name);
				 if (comp_name.find_last_of(".") != std::string::npos)
					 comp_name.erase(comp_name.find_last_of("."));
				 std::string root1, ext1;
				 
				 if (imgname == comp_name)
				 {
					 return &elem;
				 }
			 }
			 return nullptr;
		 }

		 bool ATData::IsImageRegistered(const image_t image_id) const
		 {
			 return GetImage(image_id).IsRegistered();
		 }
		 AI3D::CORE::Image * ATData::FindImageWithName(
			const std::string& name,  std::vector<image_t> imgs_ids)
		{
			
			 for(auto& it : imgs_ids)
			{
				 if (!images_.count(it))
				 {
					 continue;
				 }
				AI3D::CORE::Image& elem = images_[it];
				std::string imgname = elem.GetName();
				std::string root,ext;
				String::StringToLower(&imgname);
				if(imgname.find_last_of(".") != std::string::npos)
					imgname.erase(imgname.find_last_of("."));
				
				std::string comp_name = name;
				String::StringToLower(&comp_name);
				if(comp_name.find_last_of(".") != std::string::npos)
					comp_name.erase(comp_name.find_last_of("."));
				std::string root1,ext1;
				
				if (imgname == comp_name)
				{
					return &elem;
				}
			}
			return nullptr;
		}

		 ABBox2d ATData::GetImageCenterABB()
		 {
			 ABBox2d ret;
			 for (auto& iter : images_)
			 {
				 if (iter.second.HasPosition())
				 {
					 auto& pos = iter.second.GetPosition();
					 ret.extend(pos.block(0,0,2,1));
				 }
			 }
			 return ret;
		 }

		std::vector<image_t> ATData::FindCommonRegImageIds(
			const ATData& reconstruction) const 
		{
			std::vector<image_t> common_reg_image_ids;
			for (const auto image_id : reg_image_ids_) 
			{
				if (reconstruction.ExistsImage(image_id) &&
					reconstruction.IsImageRegistered(image_id)) 
				{
					CHECK_OPTION_EQ(GetImage(image_id).GetName(), reconstruction.GetImage(image_id).GetName());
					common_reg_image_ids.push_back(image_id);
				}
			}
			return common_reg_image_ids;
		}

		void ATData::FindCommonRegImages(
			const ATData& reconstruction, std::set<image_t>& ids1, std::set<image_t>& ids2) const
		{
			std::set<std::string>  names;
			for (auto image_id : GetRegImageIds())
			{
				auto image = GetImage(image_id);

				std::string fullname = image.GetPath() + "/" + image.GetName();

				names.insert(fullname);
			}
			for (auto image_id : reconstruction.GetRegImageIds())
			{
				auto image = reconstruction.GetImage(image_id);

				std::string fullname = image.GetPath() + "/" + image.GetName();
				if (names.count(fullname) > 0)
				{
					ids2.insert(image_id);
					
					ids1.insert(FindImageWithFullName(fullname)->GetImageId());
				}

			}
			
		}

		void ATData::FindCommonImages(
			const ATData& reconstruction, std::set<image_t>& ids1, std::set<image_t>& ids2) const
		{
			std::set<std::string>  names;
			for (auto image_id : GetImagesIds())
			{
				
				auto image = GetImage(image_id);
				
				std::string fullname = image.GetPath() + "/" + image.GetName();
				
				names.insert(fullname);
			}
			for (auto image_id : reconstruction.GetImagesIds())
			{
				auto image = reconstruction.GetImage(image_id);

				std::string fullname = image.GetPath() + "/" + image.GetName();
				if (names.count(fullname) > 0)
				{
					ids2.insert(image_id);
					ids1.insert(FindImageWithFullName(fullname)->GetImageId());
				}
				
			}

			
		}

		size_t ATData::ComputeNumObservations() const 
		{
			size_t num_obs = 0;
			for (const image_t image_id : GetImagesIds() )
			{

				num_obs += GetImage(image_id).GetNumPoints3D();
			}
			return num_obs;
		}

		double ATData::ComputeMeanTrackLength() const 
		{
			if (points3D_.empty()) 
			{
				return 0.0;
			}
			else {
				return ComputeNumObservations() / static_cast<double>(points3D_.size());
			}
		}

		double ATData::ComputeMeanObservationsPerRegImage() const 
		{
			if (reg_image_ids_.empty())
			{
				return 0.0;
			}
			else 
			{
				return ComputeNumObservations() /
					static_cast<double>(reg_image_ids_.size());
			}
		}

		double ATData::ComputeMeanReprojectionError() const 
		{
			double error_sum = 0.0;
			size_t num_valid_errors = 0;
			for (const auto& point3D : points3D_) 
			{
				if (point3D.second.HasPixelRMS()) 
				{
					error_sum += point3D.second.GetPixelRMS();
					num_valid_errors += 1;
				}
			}

			if (num_valid_errors == 0) 
			{
				return 0.0;
			}
			else 
			{
				return error_sum / num_valid_errors;
			}
		}

		void  ATData::ComputeSquaredReprojectionErrorForGCP(point3D_t id)
		{
			ControlPoint& gcp = controlpoints_[id];
			if (!gcp.GetObjectPointMutual().HasElement())
			{
				return;
			}
			
			double reproj_error_sum = 0.0;
			bool haserror = false;
#ifdef USE_WIN_DEBUG
			LOGD(String::StringPrintf("The size of TrackElements is %d.", gcp.GetObjectPointMutual().GetTrackMutual().GetElements().size()));
#endif
			for (auto& track_el : gcp.GetObjectPointMutual().GetTrackMutual().GetElements())
			{
				AI3D::CORE::Image& image = GetImageMutual(track_el.image_id);				
				Camera& camera = GetCameraMutual(image.GetCameraId());
				if (!image.IsPoseAndIntrinsicDefined(camera))
					continue;
				const Eigen::Vector2d& point2D = track_el.xy;			
				const double squared_reproj_error = AlgorithmBase::CalculateSquaredReprojectionError(
					point2D, gcp.GetObjectPoint().GetXYZ(), image.GetProjectionMatrix(),
					 camera);				
				{
					
					reproj_error_sum += squared_reproj_error;
					haserror = true;
				}
			}
			if (!haserror)
			{
				return;
			}
			reproj_error_sum = std::sqrt(reproj_error_sum / gcp.GetObjectPoint().GetTrack().Length());
			
			gcp.GetObjectPointMutual().SetPixelRMS(reproj_error_sum);
			
		}
		
		
		void ATData::ComputeDistErrorForGCP(point3D_t GCP_id)
		{
			ControlPoint& gcp = controlpoints_[GCP_id];
			std::map < image_t, std::pair<Eigen::Vector2d,std::pair<double, double> > > errors;
			UpdateGCPMeasurementError(GCP_id, errors);
			size_t error_size = errors.size();

			if (error_size == 0)
			{
				return;
			}
			double error_dist = 0.0;
			double error_pix = 0.0;
			bool haserror = false;
			for (auto& it : errors)
			{
				double dist_error = it.second.second.second;
				
				error_dist += dist_error * dist_error;
				haserror = true;
			}
			if (!haserror)
			{
				return;
			}
			gcp.GetObjectPointMutual().SetDistRMS(error_dist / error_size);
			
		}

		void ATData::Compute3DErrorForGCP(point3D_t id)
		{
			
			ControlPoint& gcp = controlpoints_[id];
			if (!gcp.HasEstimatedXYZ())
			{
				return;
			}
		
			if (!gcp.HasGivenXYZ())
			{
				return;
			}
			Eigen::Vector3d xyz = gcp.GetGivenXYZ();
			
			Eigen::Vector3d xyz_estimated = gcp.GetEstimatedXYZMutual();

			if (gcp.GetSrsMutual().type == GEOGRAPHIC)
			{
				CoordinateTransformer::Transform(1, &xyz.x(), &xyz.y(), &xyz.z(), gcp.GetSrsMutual().definition, BASESRS);
				CoordinateTransformer::Transform(1, &xyz_estimated.x(), &xyz_estimated.y(), &xyz_estimated.z(), gcp.GetSrsMutual().definition, BASESRS);
			}
			double dx = xyz_estimated.x() - xyz.x();
			double dy = xyz_estimated.y() - xyz.y();
			double dz =xyz_estimated.z() - xyz.z();
		
			
			double  dxy = std::sqrt(dx * dx + dy * dy);
			double  dxyz = std::sqrt(dx * dx + dy * dy + dz * dz);
			 
			
			gcp.Set3DError(dxyz);
			gcp.SetZ3DError(dz);
			gcp.SetXY3DError(dxy);

			
		
		}
		bool ATData::LoadViewsBin(const std::string& filename, std::set<image_t>& ids)
		{
			std::fstream ifile(filename, std::ios::in | std::ios::binary);
			if (!ifile)
			{
				return false;
			}
			int num;
			ifile.read((char*)&num, sizeof(int));
			for (int i_pg = 0; i_pg < num; i_pg++)
			{
				image_t imgid;
				ifile.read((char*)&imgid, sizeof(image_t));
				ids.insert(imgid);
			}
			return true;
		}

		 void ATData::GetBoundingBox(bool& imagechanged, bool& tiepointchanged, bool& gcpchanged)
		{
			 
			 box_.xmin_ = DBL_MAX;
			 box_.xmax_ = -DBL_MAX;
			 box_.ymin_ = DBL_MAX;
			 box_.ymax_ = -DBL_MAX;
			 box_.zmin_ = DBL_MAX;
			 box_.zmax_ = -DBL_MAX;

			 std::vector<Eigen::Vector3d> points;
			 std::map<image_t, Eigen::Vector3d>points_unreg;
			 int invalidpt = 0;
			 if (controlpoints_.size() > 0)
			 {
				 for (auto& it : controlpoints_)
				 {
					 Eigen::Vector3d point = it.second.GetObjectPoint().GetXYZ();
					 if (it.second.GetObjectPoint().HasXYZ())
					 {
						 points.push_back(point);
					 }
					 else
					 {
						 invalidpt++;
					 }
					 
				 }
				 gcpchanged = true;
			 }
			
			 invalidpt = 0;
			 if (CoordinateDescriptor::GetSRSFromDefinition(local_srs_definition_).type != LOCAL)
			 {
				 for (auto& image : images_)
				 {
					 if (image.second.HasPosition())
					 {
						 points.push_back(image.second.GetPosition());
						 
							 imagechanged = true;
					 }
					 else if (image.second.HasPositionPrior())
					 {
						 Eigen::Vector3d posecenter = image.second.GetPositionPrior();
						 points_unreg.insert(std::make_pair(image.first, posecenter));
						 imagechanged = true;
					 }
				 }

				 {
					 
					 std::vector<double> x;
					 std::vector<double> y;
					 std::vector<double> z;
					 for (const auto& point : points_unreg)
					 {
						 x.emplace_back(point.second(0));
						 y.emplace_back(point.second(1));
						 z.emplace_back(point.second(2));
					 }

					 CoordinateTransformer::Transform(points_unreg.size(), &x[0], &y[0], &z[0], images_[0].GetPriorSrs().definition, local_srs_definition_);
					 int i = 0;
					 for (const auto& point : points_unreg)
					 {
						 Eigen::Vector3d pos = { x[i],y[i],z[i] };
						 points.push_back(pos);
						 images_[point.first].SetPosition(pos);
						 i++;
					 }
				 }
				 
			 }
			 else
			 {
				 if (controlpoints_.size() == 0)
				 {
					 for (const auto& reg_id : reg_image_ids_)
					 {
						 if (images_[reg_id].HasPosition())
						 {
							 points.push_back(images_[reg_id].GetPosition());
						 }
						 else
						 {
							 invalidpt++;
						 }
					 }
					 if (!reg_image_ids_.empty())
						 imagechanged = true;
				 }
			 }

			 for (auto& it : points3D_)
			 {
				 if (it.second.HasXYZ())
				 {
					 points.push_back(it.second.GetXYZ());
				 }
				 else
				 {
					 invalidpt++;
				 }
				
			 }
			 if (!points3D_.empty())
				 tiepointchanged = true;
			

			 for (auto& it : points)
			 {

				 Eigen::Vector3d point = it;
				 if (point(0) == DBL_MAX || point(0) == -DBL_MAX
					 || point(1) == DBL_MAX || point(1) == -DBL_MAX
					 || point(2) == DBL_MAX || point(2) == -DBL_MAX
					 )
				 {
					
					 continue;
				}
				 if (point(0) > box_.xmax_)
				 {
					 box_.xmax_ = point(0);
				 }
				 if (point(0) < box_.xmin_)
				 {
					 box_.xmin_ = point(0);
				 }
				 if (point(1) > box_.ymax_)
				 {
					 box_.ymax_ = point(1);
				 }
				 if (point(1) < box_.ymin_)
				 {
					 box_.ymin_ = point(1);
				 }
				 if (point(2) > box_.zmax_)
				 {
					 box_.zmax_ = point(2);
				 }
				 if (point(2) < box_.zmin_)
				 {
					 box_.zmin_ = point(2);
				 }
			 }

		}
		const ABBox3d ATData::GetBox() { return box_.toABBox3d(); };
		double ATData::GetMin3dPoint_x() const { return box_.xmin_; }
		double ATData::GetMin3dPoint_y() const { return box_.ymin_; }
		double ATData::GetMin3dPoint_z() const { return box_.zmin_; }
		double ATData::GetMax3dPoint_x() const { return  box_.xmax_; }
		double ATData::GetMax3dPoint_y() const { return box_.ymax_; }
		double ATData::GetMax3dPoint_z() const { return box_.zmax_; }
	
	
	

		size_t ATData::GetNumCameras() const 
		{ return cameras_.size(); }
		
		size_t ATData::GetNumImages() const 
		{
			if (images_.size() > 0)
				return images_.size();
			else
				return 0;
		}

		

		size_t ATData::GetNumValidUserPoints()
		{
			size_t valid_gcp_count = 0;
			for (auto& it : user_points3D_)
			{
				
				{
					int obs_count = it.second.GetTrack().Length();
					if (obs_count >= VALIDTRIANGLENUM)
					{
						valid_gcp_count++;
					}
				}
			}
			return valid_gcp_count;
		}

		size_t ATData::GetNumControlPoints() const { return controlpoints_.size(); }

		size_t ATData::GetNumValidControlPoints() 
		{
			size_t valid_gcp_count = 0;
			for (auto& it : controlpoints_)
			{
				if (it.second.GetType() == gpt_e::GCP_CONTROL_HV || it.second.GetType() == gpt_e::GCP_CONTROL_H
					|| it.second.GetType() == gpt_e::GCP_CONTROL_V)
				{
					int obs_count = it.second.GetObjectPoint().GetTrack().Length();
					if (obs_count >= VALIDTRIANGLENUM)
					{
						valid_gcp_count++;
					}
				}
			}
			return valid_gcp_count;
		}
		size_t ATData::GetNumGCPElements() const
		{
			size_t num_gcp_elements = 0;
			for (auto& it : controlpoints_)
			{
				if (it.second.GetType() == gpt_e::GCP_CONTROL_HV || it.second.GetType() == gpt_e::GCP_CONTROL_H
					|| it.second.GetType() == gpt_e::GCP_CONTROL_V)
				{
					if (it.second.GetObjectPoint().GetTrack().Length() > 0)
					{
						num_gcp_elements += it.second.GetObjectPoint().GetTrack().Length();
					}
				}
			}
			return num_gcp_elements;
		}



		size_t ATData::GetNumCheckPoints()
		{
			int valid_gcp_count = 0;
			for (auto& it : controlpoints_)
			{
				if (it.second.GetType() == GCP_CHECK_HV)
				{
					valid_gcp_count++;
				}
			}
			return valid_gcp_count;
		}


		std::vector<Eigen::Vector3d> ATData::ComputePoints3dCoordinateAxis() 
		{

			

			std::vector<Eigen::Vector3d> coordinate_axis;
			
			auto bound = ComputeBoundsAndCentroid();
			

	

			if (std::get<0>(bound).x() == DBL_MAX || std::get<1>(bound).x() == -DBL_MAX
				|| std::get<0>(bound).y() == DBL_MAX || std::get<1>(bound).y() == -DBL_MAX
				|| std::get<0>(bound).z() == DBL_MAX || std::get<1>(bound).z() == -DBL_MAX
				)
			{
				std::vector<Eigen::Vector3d> coordinate_axis = {};
				return coordinate_axis;
			}

			
			coordinate_axis.push_back(std::get<2>(bound));
			
			double x = std::fabs(GetMax3dPoint_x() - GetMin3dPoint_x());
			double y = std::fabs(GetMax3dPoint_y() - GetMin3dPoint_y());
			double z = std::fabs(GetMax3dPoint_z() - GetMin3dPoint_z());
			double scalex = 1.1;
			double scalez = 1.1;
			if (z < 0.5 * std::min(x, y))
			{
				scalez = 2.0;
			}
			coordinate_axis.push_back(Eigen::Vector3d{ scalex * x, 0, 0 });
			coordinate_axis.push_back(Eigen::Vector3d{ 0,scalex * y,  0 });
			coordinate_axis.push_back(Eigen::Vector3d{ 0,0,scalez * z});

			

			return coordinate_axis;
		}


		size_t ATData::GetNumRegImages() const { return reg_image_ids_.size(); }

		size_t ATData::GetNumPoints3D() const { return points3D_.size(); }

		size_t ATData::GetNumImagePairs() const { return image_pairs_.size(); }
		
		const Camera& ATData::GetCamera(const camera_t camera_id) const
		{
			return cameras_.at(camera_id);
		}

		const AI3D::CORE::Image& ATData::GetImage(const image_t image_id) const
		{
			try
			{
				return images_.at(image_id);
			}
			catch (std::out_of_range oor)
			{

				return AI3D::CORE::Image();
			}
		}


		int ATData::WriteImageText(const std::string& path) const
		{
			std::ofstream file = File::OpenOfstreamUtf8(path, std::ios::trunc);
			CHECK(file.is_open()) << path;

			
			file.precision(17);

		
			file << "#   CAMERA_ID,WIDTH, HEIGHT,FocalPixel" << std::endl;
		

			for (const auto& camera : cameras_) 
			{
				std::ostringstream line;
				line.precision(17);

				line << camera.first << " ";
				
				line << camera.second.GetWidth() << " ";
				line << camera.second.GetHeight() << " ";

				line << camera.second.GetMeanFocalLength() << " ";

				std::string line_string = line.str();
				line_string = line_string.substr(0, line_string.size() - 1);

				file << line_string << std::endl;
			}

			file << "#   IMAGE_NAME，CAM_ID,X,Y,Z,R,Depth" << std::endl;

			for (const auto& image : images_)
			{
				std::ostringstream line;
				line.precision(17);
				std::string path = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(image.second.GetPath())));

				line << path + image.second.GetName() << " ";
				line << image.second.GetCameraId() << " ";
				line << image.second.GetPosition().x() << " ";
				line << image.second.GetPosition().y() << " ";
				line << image.second.GetPosition().z() << " ";

				line << image.second.GetRotationMatrix() << " ";
				line << image.second.GetDepth()[0] << " ";
				line << image.second.GetDepth()[1] << " ";
				line << image.second.GetDepth()[2] << " ";
				

				std::string line_string = line.str();
				line_string = line_string.substr(0, line_string.size() - 1);

				file << line_string << std::endl;
			}
			file.close();
			return AI3D_SUCCESS;
		}

		int ATData::WritePoints3DText(const std::string& path) const 
		{
			std::ofstream file = File::OpenOfstreamUtf8(path, std::ios::trunc);
			CHECK(file.is_open()) << path;

			
			file.precision(17);
			file << "#  X, Y, Z, R, G, B" << std::endl;

			for (const auto& point3D : points3D_) 
			{
				
				file << point3D.second.GetXYZ()(0) << " ";
				file << point3D.second.GetXYZ()(1) << " ";
				file << point3D.second.GetXYZ()(2) << " ";
				file << static_cast<int>(point3D.second.GetColor(0)) << " ";
				file << static_cast<int>(point3D.second.GetColor(1)) << " ";
				file << static_cast<int>(point3D.second.GetColor(2)) << " ";
				
				std::ostringstream line;
				line.precision(17);
				std::string line_string = line.str();
				line_string = line_string.substr(0, line_string.size() - 1);

				file << line_string << std::endl;
			}
			file.close();
			return AI3D_SUCCESS;
		}

		const Point3D& ATData::GetPoint3D(const point3D_t point3D_id) const 
		{
			if(points3D_.count(point3D_id))
				return points3D_.at(point3D_id);
			return Point3D();

		}

		const class MeasureConstraint& ATData::GetConstraint(const constraint_t constraint_id) const {
			if (constraintList_.count(constraint_id))
				return constraintList_.at(constraint_id);
			return MeasureConstraint();
		}

		inline void ATData::SetPoint3D(const EIGEN_STL_UMAP(point3D_t, Point3D)& point3D)
		{
			points3D_ = point3D;
		}

		inline void ATData::SetUserPoint3D(const EIGEN_STL_UMAP(point3D_t, Point3D)& point3D)
		{
			user_points3D_ = point3D;
		}

		inline void ATData::SetConstraint(EIGEN_STL_UMAP(constraint_t, MeasureConstraint)& constraints) {
			constraintList_ = constraints;
		}

		const std::pair<size_t, size_t>& ATData::GetImagePair(
			const image_pair_t pair_id) const 
		{
			return image_pairs_.at(pair_id);
		}

		

		
		void ATData::UpdateGivenGCP(int GCP_id, int idx,double value, std::map<image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >& error_map)
		{
			
			
			
			Eigen::Vector3d xyz = controlpoints_[GCP_id].GetGivenXYZMutual();
			xyz(idx) = value;
			controlpoints_[GCP_id].GetGivenXYZMutual() = xyz;
			CoordinateTransformer::Transform(1,&xyz[0],&xyz[1],&xyz[2], controlpoints_[GCP_id].GetSrs().definition, local_srs_definition_);
			
			controlpoints_[GCP_id].GetObjectPointMutual().GetXYZMutual() = xyz;
			UpdateGCPError(GCP_id);
			
			UpdateGCPMeasurementError(GCP_id, error_map);
			

		}
		
	
		
		void ATData::UpdateGCPError(int GCP_id)
		{
			
			
		
			Compute3DErrorForGCP(GCP_id);
			ComputeSquaredReprojectionErrorForGCP(GCP_id);
		}
		void ATData::UpdateUserPtMeasurementError(int gcp_id, std::map<image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >& estimate_xy, bool istopredict)
		{
			
			if (!user_points3D_.count(gcp_id))
				return;
			Point3D point = user_points3D_.at(gcp_id);
			std::map < image_t, std::pair<double, double> > errors;
			for (auto& it : point.GetTrackMutual().GetElements())
			{

				AI3D::CORE::Image& image = images_[it.image_id];
				Camera& camera = cameras_[images_[it.image_id].GetCameraId()];
				
				if (!image.IsPoseAndIntrinsicDefined(camera))
				{
					continue;
				}


				Eigen::Vector2d emist_xy{ -DBL_MAX , -DBL_MAX };
				PredictUserPtMeasurement(gcp_id, it.image_id, emist_xy, false, istopredict);
				Eigen::Vector2d src_xy = it.xy;
				Eigen::Vector2d dxy = src_xy - emist_xy;
				
				
				double projerror = std::sqrt(dxy.x() * dxy.x() + dxy.y() * dxy.y());

				
				estimate_xy[it.image_id].first = src_xy;
				estimate_xy[it.image_id].second.first = projerror;
				Eigen::Matrix3d scale_K_inverse = camera.GetCalibrationMatrix().inverse();
				Eigen::Vector3d _CAM_obr = scale_K_inverse * src_xy.homogeneous();
				Eigen::Vector3d _CAM_gcp = scale_K_inverse * emist_xy.homogeneous();

				
				Eigen::Vector4d vec1(_CAM_gcp[0], _CAM_gcp[1], _CAM_gcp[2], 0);
				Eigen::Vector4d vec2(_CAM_obr[0], _CAM_obr[1], _CAM_obr[2], 0);

				double disterror = vec1.cross3(vec2).norm();
				estimate_xy[it.image_id].second.second = disterror;

			}
			return;
		}

		void ATData::UpdateGCPMeasurementError(int gcp_id, std::map<image_t, std::pair<Eigen::Vector2d,std::pair<double,double> > >& estimate_xy, bool istopredict)
		{
		
			if (!controlpoints_.count(gcp_id))
				return;
			ControlPoint gcp = controlpoints_.at(gcp_id);
			std::map < image_t, std::pair<double, double> > errors;
			for (auto& it : gcp.GetObjectPointMutual().GetTrackMutual().GetElements())
			{
				
				AI3D::CORE::Image& image = images_[it.image_id];
				Camera& camera = cameras_[images_[it.image_id].GetCameraId()];
				
				if (!image.IsPoseAndIntrinsicDefined(camera))
				{
					continue;
				}
			
				
				Eigen::Vector2d emist_xy{ -DBL_MAX , -DBL_MAX };
				PredictGCPMeasurement(gcp_id, it.image_id, emist_xy,false, istopredict);
				Eigen::Vector2d src_xy = it.xy;
				Eigen::Vector2d dxy = src_xy - emist_xy;
				
				
				double projerror = std::sqrt(dxy.x() * dxy.x() + dxy.y() * dxy.y());

				estimate_xy[it.image_id].first = emist_xy;
				estimate_xy[it.image_id].second.first = projerror;
				Eigen::Matrix3d scale_K_inverse = camera.GetCalibrationMatrix().inverse();
				Eigen::Vector3d _CAM_obr = scale_K_inverse * src_xy.homogeneous();
				Eigen::Vector3d _CAM_gcp = scale_K_inverse * emist_xy.homogeneous();

				
				Eigen::Vector4d vec1(_CAM_gcp[0], _CAM_gcp[1], _CAM_gcp[2], 0);
				Eigen::Vector4d vec2(_CAM_obr[0], _CAM_obr[1], _CAM_obr[2], 0);

				double disterror = vec1.cross3(vec2).norm();
				estimate_xy[it.image_id].second.second = disterror;	
				
			}
			return;
		}


		void ATData::ComputeTileBoundingBox(atpoint_elements_e elements, bool bremoveoutliers)
		{

			ABBox3d box;
			bool bviewconsidered = false;
			bool btieptconsidered = false;
			bool bgcpconsidered = false;

			std::vector<Eigen::Vector3d> points;

			if (elements & int(atpoint_elements_e::PT_ELE_VIEWS))
			{
				bviewconsidered = true;
			}
			if (elements & int(atpoint_elements_e::PT_ELE_TIEPOINTS))
			{
				btieptconsidered = true;
			}
			if (elements & int(atpoint_elements_e::PT_ELE_VIEWS_CONTROPOINTS))
			{
				bgcpconsidered = true;
			}
			if (bviewconsidered)
			{
				for (auto& iter : images_)
				{
					AI3D::CORE::Image image = iter.second;
					camera_t camid = image.GetCameraId();
					Camera camera = cameras_.at(camid);

					bool  bvalidimage = image.IsPoseAndIntrinsicDefined(camera);
					if (bvalidimage)
						points.push_back(iter.second.GetPosition());

				}
			}

			if (btieptconsidered)
			{
				for (auto& iter : points3D_)
				{

					points.push_back(iter.second.GetXYZ());

				}
			}
			if (bgcpconsidered)
			{
				for (auto& iter : controlpoints_)
				{

					points.push_back(iter.second.GetObjectPoint().GetXYZ());

				}
			}

			if (bremoveoutliers)
			{
				
			}

			for (auto& point : points)
			{
				box.extend(point);
			}

			tile_aabb_box_ = box.cast<float>();

		}
		
		void ATData::ComputeTileBoundingBox(bb_scope_e elements, bool bremoveoutliers)
		{
			
			bremoveoutliers = false;
			
			ABBox3d box_pos_pts, box_points, box_frustum;
			

			ABBox3d box ;
			bool bviewconsidered = false;
			bool btieptconsidered = false;
			bool bfrustumconsidered = false;

			std::vector<Eigen::Vector3d> points;

			if (elements ==(bb_scope_e::BB_SCOPE_VIEWS))
			{
				bviewconsidered = true;
				btieptconsidered = true;
			}
			else if (elements == (bb_scope_e::BB_SCOPE_VIEWS_TIEPOINTS))
			{
				bviewconsidered = true;
				btieptconsidered = true;
			}
			else if (elements == (bb_scope_e::BB_SCOPE_VIEWFRUSTUM))
			{
				bviewconsidered = false;
				btieptconsidered = true;
				bfrustumconsidered = true;
			}
			else if (elements == bb_scope_e::BB_SCOPE_TIEPOINTS)
			{
				btieptconsidered = true;
			}
			
			
			
			{
				for (auto& iter : images_)
				{
					AI3D::CORE::Image image = iter.second;
					camera_t camid = image.GetCameraId();
					AI3D::CORE::Camera camera = cameras_.at(camid);

					bool  bvalidimage = image.IsPoseAndIntrinsicDefined(camera);
					if (bvalidimage)
					{

						box_pos_pts.extend(iter.second.GetPosition());
					}

				}
			}
			bool valid = IsBoundingBoxValid(box_pos_pts);
			std::cout << box_pos_pts.min() << " " << box_pos_pts.max() << " "<< box_pos_pts.isEmpty()<<" "<< valid << std::endl;
			
			std::vector<bool> pts_valid(points3D_.size(), true);
			if (btieptconsidered)
			{

				
				if (bremoveoutliers)
				{
					std::array<Eigen::Vector3d, 2> pts_stat;
					pts_stat[0] = Eigen::Vector3d::Zero();
					pts_stat[1] = Eigen::Vector3d::Zero();

					for (const auto& point : points3D_)
					{
						pts_stat[0] += point.second.GetXYZ();
					}
					pts_stat[0] /= points3D_.size();

					for (const auto& point : points3D_)
					{
						pts_stat[1] += (point.second.GetXYZ() - pts_stat[0]).cwiseAbs2();
					}
					pts_stat[1] /= points3D_.size();
					pts_stat[1] = pts_stat[1].cwiseSqrt();
					constexpr int K = 4;
					const  Eigen::Vector3d max_dist = K * pts_stat[1];
					point3D_t count = 0;
					for (auto& iter : points3D_)
					{
						const auto& point = iter.second;
						Eigen::Vector3d d = (point.GetXYZ() - pts_stat[0]).cwiseAbs();

						if (d[0] > max_dist[0] || d[1] > max_dist[1] || d[2] > max_dist[2])
						{
							pts_valid[count] = false;

						}
						count++;
					}

				}

				
				
				point3D_t count = 0;
				for (auto& iter : points3D_)
				{
					if (pts_valid[count])
					{
						box_points.extend(iter.second.GetXYZ());
					}
					count++;
				}

				
			}
			
			
			if (bfrustumconsidered)
			{
				std::vector<Eigen::Vector2d> depth_ranges(images_.size());
				std::map<image_t, image_t> ids_to_idx, idx_to_ids;
				image_t imgcnt = 0;
				for (auto iter : images_)
				{
					ids_to_idx[iter.first] = imgcnt;
					idx_to_ids[imgcnt] = iter.first;
					imgcnt++;

				}
				for (auto& range : depth_ranges)
				{
					range[0] = std::numeric_limits<double>::max();
					range[1] = std::numeric_limits<double>::lowest();
				}
				point3D_t count = 0;
				for (const auto& iter : points3D_)
				{
					if (pts_valid[count])
					{
						const auto& point = iter.second;
						for (const auto& obs : point.GetTrack().GetElements())
						{
							image_t imgid = obs.image_id;
							AI3D::CORE::Image image = images_.at(imgid);
							const Eigen::Vector3d pt_c = image.GetProjectionMatrix() * point.GetXYZ().homogeneous();
							
							auto& range = depth_ranges[ids_to_idx.at(imgid)];
						
							range[0] = std::min(range[0], pt_c[2]);
							range[1] = std::max(range[1], pt_c[2]);
							
						}

					}
					count++;
				}



				for (image_t i = 0; i < idx_to_ids.size(); i++)
				{
					
					auto imgid = idx_to_ids[i];
					AI3D::CORE::Image image = images_.at(imgid);
					const auto& camera = cameras_.at(image.GetCameraId());
					const Eigen::Matrix3d Rt = image.GetProjectionMatrix().block(0, 0, 3, 3).transpose();
					const Eigen::Vector3d t = image.GetProjectionMatrix().block(0, 3, 3, 1);
					const Eigen::Matrix3d Kinv = camera.GetCalibrationMatrix().inverse();

					auto& range = depth_ranges[i];
					if (range[0] == std::numeric_limits<double>::max() || range[1] == std::numeric_limits<double>::lowest())
					{
						continue;
					}
					const double w = image.GetWidth();
					const double h = image.GetHeight();
					for (int k = 0; k < 2; k++)
					{
						double d = range[k];
						std::vector<Eigen::Vector3d> pixel_depths =
						{ {0,0,d},{0,h * d,d},{w * d,0,d},{w*d,h*d,d} };
						for (int n = 0; n < 4; n++)
						{
							Eigen::Vector3d pix_d = pixel_depths[n];
							Eigen::Vector3d pt_world = Rt * (Kinv * pix_d - t);
							
							box_frustum.extend(pt_world);
						}
					}
				}
			}
			
			if (elements == (bb_scope_e::BB_SCOPE_VIEWS))
			{
				box_pos_pts.min().z() = box_points.min().z();
				box_pos_pts.max().z() = box_points.max().z();
				tile_aabb_box_ = box_pos_pts.cast<float>();
			}
			else if ((elements ==(bb_scope_e::BB_SCOPE_VIEWS_TIEPOINTS) )||(elements == (bb_scope_e::BB_SCOPE_TIEPOINTS)))
			{
				tile_aabb_box_ = box_points.cast<float>();
			}
			else if (elements == (bb_scope_e::BB_SCOPE_VIEWFRUSTUM))
			{
				tile_aabb_box_ = box_frustum.cast<float>();
			}
			
			MakeBoundingBoxValid(tile_aabb_box_);
			
			
			int precision = 100;
			auto srs_temp = CoordinateDescriptor::GetSRSFromDefinition(GetLocalSrs());
			if (srs_temp.type == GEOGRAPHIC)
			{
				precision = 1e6;
			}
			
			BoundingBoxToPresicion(tile_aabb_box_, precision);
			if (box_pos_pts.isEmpty())
			{
				tile_aabb_box_.min() =Eigen::Vector3f::Zero()  ;
				tile_aabb_box_.max() = Eigen::Vector3f::Zero();
			}
			
		}


		std::tuple<Eigen::Vector3d, Eigen::Vector3d, Eigen::Vector3d>
			ATData::ComputeBoundsAndCentroid()  
		{
			bool imagechanged, tiepointchanged, gcpchanged;
			GetBoundingBox(imagechanged, tiepointchanged, gcpchanged);
			if (box_.xmin_ == DBL_MAX || box_.xmax_ == -DBL_MAX
				|| box_.ymin_ == DBL_MAX || box_.ymax_ == -DBL_MAX
				|| box_.zmin_ == DBL_MAX || box_.zmax_ == -DBL_MAX
				)
			{
				LOGD("get bounding box failed. ");
				return std::make_tuple(Eigen::Vector3d(DBL_MAX, DBL_MAX, DBL_MAX), 
					Eigen::Vector3d(-DBL_MAX, -DBL_MAX, -DBL_MAX),
					Eigen::Vector3d(0, 0, 0));
			}

			
			const Eigen::Vector3d bbox_min(box_.xmin_, box_.ymin_, box_.zmin_);
			const Eigen::Vector3d bbox_max(box_.xmax_, box_.ymax_, box_.zmax_);
			
			Eigen::Vector3d mean_coord(0, 0, 0);

			mean_coord = (bbox_min + bbox_max) * 0.5;
			auto bound = std::make_tuple(bbox_min, bbox_max, mean_coord);			
			return bound;
		}


		double ATData::GetSceneScale(const double extent, const double p0,
			const double p1)
		{
			CHECK_OPTION_GT(extent, 0);
			if (!(HasPositionImages() || HasControlPoints() || HasTiepoints()))
			{
				LOGD("no 3dpoints. ");
				return -DBL_MAX;
			}
			bool imagechanged, tiepointchanged, gcpchanged;
			GetBoundingBox(imagechanged, tiepointchanged, gcpchanged);
			

			if (box_.xmin_ == DBL_MAX || box_.xmax_ == -DBL_MAX
				|| box_.ymin_ == DBL_MAX || box_.ymax_ == -DBL_MAX
				|| box_.zmin_ == DBL_MAX || box_.zmax_ == -DBL_MAX
				)
			{

				LOGD("invalid boundingbox. ");
				return -DBL_MAX;
			}
			const Eigen::Vector3d bbox_min(box_.xmin_, box_.ymin_, box_.zmin_);
			const Eigen::Vector3d bbox_max(box_.xmax_, box_.ymax_, box_.zmax_);
			const double old_extent = (bbox_max - bbox_min).norm();
			double scale;
			if (old_extent < std::numeric_limits<double>::epsilon())
			{
				scale = 1;
			}
			else
			{
				scale = extent / old_extent;
			}
			return scale;
		}

		void ATData::Normalize(const double extent, const double p0,
			const double p1, const bool use_images)
		{
			CHECK_OPTION_GT(extent, 0);
			if (!(HasPositionImages() || HasControlPoints() || HasTiepoints()))
			{
				LOGD("no 3dpoints. ");
				return;
			}
			bool imagechanged, tiepointchanged, gcpchanged;
			GetBoundingBox(imagechanged, tiepointchanged, gcpchanged);
			
		
			if (box_.xmin_ == DBL_MAX || box_.xmax_ == -DBL_MAX
				|| box_.ymin_ == DBL_MAX || box_.ymax_ == -DBL_MAX
				|| box_.zmin_ == DBL_MAX || box_.zmax_ == -DBL_MAX
				)
			{
				
				LOGD("invalid boundingbox. ");
				return;
			}
			const Eigen::Vector3d bbox_min(box_.xmin_, box_.ymin_, box_.zmin_);
			const Eigen::Vector3d bbox_max(box_.xmax_, box_.ymax_, box_.zmax_);
			const double old_extent = (bbox_max - bbox_min).norm();
			double scale;
			if (old_extent < std::numeric_limits<double>::epsilon())
			{
				scale = 1;
			}
			else
			{
				scale = extent / old_extent;
			}
			
			if (tiepointchanged)
			{
				for (auto& it : points3D_)
				{
					if (it.second.HasXYZ())
					{
						it.second.GetXYZMutual() = scale * it.second.GetXYZMutual() - scale * bbox_min;
					}
				}
			}
			if (gcpchanged)
			{
				for (auto& it : controlpoints_)
				{
					if (it.second.GetObjectPointMutual().HasXYZ())
					{
						it.second.GetObjectPointMutual().GetXYZMutual() = scale * it.second.GetObjectPointMutual().GetXYZMutual() - scale * bbox_min;
					}
				}
			}
			

			if (imagechanged)
			{
				std::map<image_t, Eigen::Vector3d> points_unreg;
				for (auto& it : images_)
				{
					if (it.second.HasPosition())
					{
						it.second.GetPositionMutual() = scale * it.second.GetPositionMutual() - scale * bbox_min;
						if (it.second.HasDepth())
							it.second.GetDepthMutual() *= scale;
					}
					else if (it.second.HasPositionPrior())
					{
						Eigen::Vector3d posecenter = it.second.GetPositionPrior();
						points_unreg.insert(std::make_pair(it.first, posecenter));
						
					}
				}
				if (!points_unreg.empty())
				{
					
					std::vector<double> x;
					std::vector<double> y;
					std::vector<double> z;
					for (const auto& point : points_unreg)
					{
						x.emplace_back(point.second(0));
						y.emplace_back(point.second(1));
						z.emplace_back(point.second(2));
					}

					CoordinateTransformer::Transform(points_unreg.size(), &x[0], &y[0], &z[0], images_[0].GetPriorSrs().definition, local_srs_definition_);
					int i = 0;
					for (const auto& point : points_unreg)
					{
						Eigen::Vector3d pos = { x[i],y[i],z[i] };
						pos = scale * pos - scale * bbox_min;
						images_[point.first].SetPosition(pos);				

						i++;
					}
				}
			}

			
			
			
		}

		
		void ATData::UpdateGCPMeasurementError(int gcp_id, std::map<image_t, std::pair<double, double>>& error_map)
		{
			
		
			ControlPoint gcp = controlpoints_[gcp_id];
			std::map < image_t, std::pair<double, double> > errors;
			for (auto& it : gcp.GetObjectPointMutual().GetTrackMutual().GetElements())
			{

				AI3D::CORE::Image& image = images_[it.image_id];				
				Camera& camera = cameras_[images_[it.image_id].GetCameraId()];			
				if (!image.IsPoseAndIntrinsicDefined(camera))
					continue;

				
				Eigen::Vector2d emist_xy{ -DBL_MAX , -DBL_MAX };
				PredictGCPMeasurement(gcp_id, it.image_id, emist_xy, false);
				bool status1 = (camera.GetWidth() ) > emist_xy.x() && emist_xy.x() > 0;
				bool status2 = (camera.GetHeight() ) > emist_xy.y() && emist_xy.y() > 0;
				if (!(status1 && status2))
				{
					return;
				}
				
				Eigen::Vector2d src_xy = it.xy;
				Eigen::Vector2d dxy = src_xy - emist_xy;
				double projerror = std::sqrt(dxy.x() * dxy.x() + dxy.y() * dxy.y());


				Eigen::Matrix3d scale_K_inverse = camera.GetCalibrationMatrix().inverse();
				Eigen::Vector3d _CAM_obr = scale_K_inverse * src_xy.homogeneous();
				Eigen::Vector3d _CAM_gcp = scale_K_inverse * emist_xy.homogeneous();

				
				Eigen::Vector4d vec1(_CAM_gcp[0], _CAM_gcp[1], _CAM_gcp[2], 0);
				Eigen::Vector4d vec2(_CAM_obr[0], _CAM_obr[1], _CAM_obr[2], 0);

				double disterror = vec1.cross3(vec2).norm();

				
				Eigen::Vector2d world_xy = camera.ImageToWorld(emist_xy);
				Eigen::Matrix3x4d proj_matrix = image.GetProjectionMatrix();
				
				const double proj_z = proj_matrix.row(2).dot(gcp.GetObjectPoint().GetXYZ().homogeneous());
				Eigen::Vector3d emist_xyz{ world_xy.x() * proj_z,world_xy.y()* proj_z, proj_z };
				
				double f_temp = sqrt(proj_z * proj_z - (emist_xyz.x() * emist_xyz.x() + emist_xyz.y() * emist_xyz.y()));
				
				Eigen::Vector2d world_src_xy = camera.ImageToWorld(src_xy);

				Eigen::Vector3d src_xyz{ world_src_xy.x()* f_temp,world_src_xy.y()* f_temp,f_temp };
				
				double dist_src = src_xyz.norm();
				
				
				
				double dist_emist = emist_xyz.norm();
				Eigen::Vector3d dxyz_im_cam = src_xyz - emist_xyz;
				double dist_in_mm = dxyz_im_cam.norm();

				double cosalpha2 =( (dist_emist * dist_emist) + (dist_in_mm * dist_in_mm) - (dist_src * dist_src)) / (2* dist_emist * dist_in_mm);
				
				disterror = dist_in_mm * std::sqrt(1 - cosalpha2);
				errors[it.image_id] = std::make_pair(projerror, disterror);
				
			}
			error_map = errors;
			return ;
			
		}

		void ATData::UpdataUserPtErrorInfo(point3D_t id, std::map<image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >& error_map, bool istopredict)
		{
			
			if (id != kInvalidPoint3DId)
			{
				if (!user_points3D_.count(id))
					return;
				auto& point = user_points3D_.at(id);
				if (point.IsValid())
				{
					ComputeUserPtEstimatedXYZ(id);

				
				}

				
				if (point.HasElement())
				{
					UpdateUserPtMeasurementError(id, error_map, istopredict);
					double reproj_error_sum = 0.0;
					double dist_error_sum = 0.0;
					bool haserror = false;
					int ele_size = error_map.size();
					for (auto& error : error_map)
					{
						double reproj_error = error.second.second.first;
						double dist_error = error.second.second.second;
						if (reproj_error == -DBL_MAX || dist_error == DBL_MAX)
						{
							continue;
						}
						const double squared_reproj_error = reproj_error * reproj_error;
						const double dist_reproj_error = dist_error * dist_error;
						{
							
							reproj_error_sum += squared_reproj_error;
							dist_error_sum += dist_reproj_error;
							haserror = true;
						}
					}
					if (!haserror)
					{
						return;
					}
					reproj_error_sum = std::sqrt(reproj_error_sum / ele_size);
					dist_error_sum = std::sqrt(dist_error_sum / ele_size);
					point.SetPixelRMS(reproj_error_sum);
				
				}
			}
			return;
		}



		void ATData::UpdataGCPErrorInfo(point3D_t id, std::map<image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >& error_map, bool istopredict)
		{
			
			if (id != kInvalidPoint3DId)
			{
				if (!controlpoints_.count(id))
					return;
				auto& controlpoint = controlpoints_.at(id);
				if (controlpoint.GetObjectPointMutual().IsValid())
				{
					ComputeGCPEstimatedXYZ(id);
		
					if (controlpoint.HasEstimatedXYZ())
					{
						Compute3DErrorForGCP(id);
					}
				}

				
				if (controlpoint.GetObjectPointMutual().HasElement())
				{
					UpdateGCPMeasurementError(id, error_map, istopredict);
					double reproj_error_sum = 0.0;
					double dist_error_sum = 0.0;
					bool haserror = false;
					int ele_size = error_map.size();
					for (auto& error : error_map)
					{			
						double reproj_error = error.second.second.first;
						double dist_error = error.second.second.second;
						if (reproj_error == -DBL_MAX || dist_error == DBL_MAX)
						{
							continue;
						}
						const double squared_reproj_error = reproj_error * reproj_error;
						const double dist_reproj_error = dist_error * dist_error;
						{
							
							reproj_error_sum += squared_reproj_error;
							dist_error_sum += dist_reproj_error;
							haserror = true;
						}
					}
					if (!haserror)
					{
						return;
					}
					reproj_error_sum = std::sqrt(reproj_error_sum / ele_size);
					dist_error_sum = std::sqrt(dist_error_sum / ele_size);
					controlpoint.GetObjectPointMutual().SetPixelRMS(reproj_error_sum);
					controlpoint.GetObjectPointMutual().SetDistRMS(dist_error_sum);
				}
			}			
			return;
		}
		
		void ATData::TriangulateTiePoints()
		{
			std::vector<int>ids_point;
			std::vector<double> estimate_x;
			std::vector<double> estimate_y;
			std::vector<double> estimate_z;
			for (auto& pt_tmp : points3D_)
			{
				auto point = pt_tmp.second;
				
				if (!point.HasElement())
				{
					Eigen::Vector3d xyz = Eigen::Vector3d{ -DBL_MAX, -DBL_MAX, -DBL_MAX };
					point.GetEstimatedXYZMutual() = xyz;
					continue;
				}

				std::vector<Eigen::Vector2f> points;
				std::vector<Eigen::Matrix<float, 3, 4>> poses;
				std::vector<Eigen::Matrix<double, 3, 4>> poseds;
				std::vector<Eigen::Vector3d> bearing;
				std::vector<image_t> imageids;
				for (auto& ele : point.GetTrackMutual().GetElements())
				{
					AI3D::CORE::Image image = images_[ele.image_id];

					Eigen::Vector2d xy = ele.xy;
					Camera& camera = cameras_[image.GetCameraId()];


					if (!image.IsPoseAndIntrinsicDefined(camera))
					{
						std::pair<Eigen::Vector2d, std::pair<double, double> > blankxy;
						blankxy.first = Eigen::Vector2d(-DBL_MAX, -DBL_MAX);
						blankxy.second.first = kInvalidError;
						blankxy.second.second = kInvalidError;
						
						continue;
					}
					Eigen::Vector2d  undis_xy = camera.UndistortPixel(xy);
					
					points.push_back(Eigen::Vector2f{ undis_xy.x(), undis_xy.y() });

					
					if (fabs(image.GetProjectionCenter().x()) > BIGSCENECOOR ||
						fabs(image.GetProjectionCenter().y()) > BIGSCENECOOR||
						fabs(image.GetProjectionCenter().z()) > BIGSCENECOOR						)
					{
						imageids.push_back(ele.image_id);
					}
					else 
					{
						poses.push_back((camera.GetCalibrationMatrix() * image.GetProjectionMatrix()).cast<float>());
						
					}
				}

				if (!imageids.empty() && poses.empty())
				{
					Eigen::Vector3d originpt = images_[imageids[0]].GetPosition();
					for (auto& it : imageids)
					{
						AI3D::CORE::Image image = images_[it];
						image.GetPositionMutual() -= originpt;
						poses.push_back((cameras_[image.GetCameraId()].GetCalibrationMatrix() * image.GetProjectionMatrix()).cast<float>());
						
					}
				}


				Eigen::Vector3d xyz;
				if (points.size() >= VALIDTRIANGLENUM)
				{
					
					
					
					
					

					Eigen::Vector3f xyzf = AlgorithmBase::TriangulatePoint(poses, points);
					xyz(0) = xyzf.x();
					xyz(1) = xyzf.y();
					xyz(2) = xyzf.z(); 
					if (!imageids.empty())
					{
						xyz += images_[imageids[0]].GetPosition();;

					}

				
					point.GetEstimatedXYZMutual() = xyz;
					
					

				}
				else
				{
					xyz = Eigen::Vector3d{ -DBL_MAX, -DBL_MAX, -DBL_MAX };
					point.GetEstimatedXYZMutual() = xyz;
					
				}
			}
			
		}
		
		void ATData::UpdataUserTiepointsGlobalErrorInfo(std::map<point3D_t, std::map < image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >>& gcp_error_map, bool istopredict)
		{
			(void)istopredict;
			
			auto data_tocalc = this;
			ATData datatemp = *data_tocalc;
			auto srs_data = datatemp.GetLocalSrs();
			datatemp.TransFormATData(BASESRS);
			
			auto& userpoints = datatemp.GetUserPoints3DMutual();

			auto& cameras = datatemp.GetCamerasMutual();
			auto& images = datatemp.GetImagesMutual();

			
			{
				
				std::vector<image_t> img_idx = datatemp.GetHasPostionImagesIds();
				if (img_idx.empty())
				{
					return;
				}
				Eigen::Vector3d sum = Eigen::Vector3d::Zero();
				Eigen::Vector3d point_first = images[*img_idx.begin()].GetPosition();
				for (int i = 0; i < img_idx.size(); i++)
				{
					sum += (images[img_idx[i]].GetPosition() - point_first);
				}
				Eigen::Vector3d position_offset = sum / img_idx.size() + point_first;
				
				
				std::vector<double> estimate_x, given_x, estimate_x_before;
				std::vector<double> estimate_y, given_y, estimate_y_before;
				std::vector<double> estimate_z, given_z, estimate_z_before;
				std::vector<int>  ids_gcp_forPixError;
				for (auto& gcp_tmp : userpoints)
				{

					std::map<image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > > error_map;
					auto& gcp = gcp_tmp.second;
					
					if (!gcp.HasElement())
					{
						gcp_error_map[gcp_tmp.first] = error_map;
						continue;
					}

					std::vector<Eigen::Vector2f> points;
					std::vector<Eigen::Matrix<float, 3, 4>> poses;
					
					std::vector<Eigen::Vector3d> bearing;
					std::vector<image_t> imageids;
					
					for (auto& ele : gcp.GetTrackMutual().GetElements())
					{
						if (!images.count(ele.image_id)) {
							
							continue;
						}
						Image& image = images.at(ele.image_id);

						Eigen::Vector2d xy = ele.xy;
						if (!cameras.count(image.GetCameraId())) {
							continue;
						}
						Camera& camera = cameras.at(image.GetCameraId());


						if (!image.IsPoseAndIntrinsicDefined(camera))
						{
							std::pair<Eigen::Vector2d, std::pair<double, double> > blankxy;
							blankxy.first = Eigen::Vector2d(-DBL_MAX, -DBL_MAX);
							blankxy.second.first = kInvalidError;
							blankxy.second.second = kInvalidError;
							error_map[ele.image_id] = blankxy;
							gcp_error_map[gcp_tmp.first] = error_map;
							continue;
						}
						Eigen::Vector2d  undis_xy = camera.UndistortPixel(xy);
						
						points.push_back(Eigen::Vector2f{ undis_xy.x(), undis_xy.y() });

						
						if (fabs(image.GetProjectionCenter().x()) > BIGSCENECOOR)
						{
							imageids.push_back(ele.image_id);
						}
						else
						{

							poses.push_back((camera.GetCalibrationMatrix() * image.GetProjectionMatrix()).cast<float>());
							
						}
					}

					if (!imageids.empty() && poses.empty())
					{

						std::vector<Eigen::Vector3d> imagepose;
						for (auto& it : imageids)
						{
							AI3D::CORE::Image image = images.at(it);
							image.GetPositionMutual() -= position_offset;
							imagepose.push_back(image.GetPositionMutual());
							poses.push_back((cameras.at(image.GetCameraId()).GetCalibrationMatrix() * image.GetProjectionMatrix()).cast<float>());
							
						}

					}



					Eigen::Vector3d xyz;
					if (points.size() >= VALIDTRIANGLENUM)
					{
					
						Eigen::Vector3f xyzf = AlgorithmBase::TriangulatePoint(poses, points);
						xyz(0) = xyzf.x();
						xyz(1) = xyzf.y();
						xyz(2) = xyzf.z(); 
						if (!imageids.empty())
						{
							xyz += position_offset;;

						}


						
						ids_gcp_forPixError.emplace_back(gcp_tmp.first);
						gcp.GetEstimatedXYZMutual() = xyz;
						
						estimate_x.push_back(xyz(0));
						estimate_y.push_back(xyz(1));
						estimate_z.push_back(xyz(2));
						

					}
					else
					{
						xyz = Eigen::Vector3d{ -DBL_MAX, -DBL_MAX, -DBL_MAX };
						gcp.GetEstimatedXYZMutual() = xyz;
						
						ids_gcp_forPixError.emplace_back(gcp_tmp.first);
						
						estimate_x.push_back(xyz(0));
						estimate_y.push_back(xyz(1));
						estimate_z.push_back(xyz(2));
						
					}
				}


				{
					ControlPoints gcpsestimated;
					auto& points_est = gcpsestimated.GetPointsMutual();
					points_est.clear();
					int index1 = 0;
					for (const auto& gcp_id : ids_gcp_forPixError)
					{
						ControlPoint point;
						point.SetId(gcp_id);
						point.SetObjectPoint(user_points3D_.at(gcp_id));
						
						point.GetEstimatedXYZMutual() =
							Eigen::Vector3d(estimate_x[index1], estimate_y[index1], estimate_z[index1]);
						point.SetSrs(CoordinateDescriptor::GetSRSFromDefinition(srs_data));
						points_est[gcp_id] = point;
						index1++;
					}
					gcpsestimated.TransformEstimatedXYZToGivenXYZSrs(BASESRS);

					

					index1 = 0;
					for (const auto& gcp_id : ids_gcp_forPixError)
					{

						user_points3D_.at(gcp_id).GetEstimatedXYZMutual() = gcpsestimated.GetPointsMutual().at(gcp_id).GetEstimatedXYZMutual();
						index1++;
					}
				}

				if (ids_gcp_forPixError.empty())
				{
					return;
				}

				
				for (const auto& gcp_id : ids_gcp_forPixError)
				{
					std::map<image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > > error_map;
					Point3D& up = user_points3D_.at(gcp_id);
					if (up.HasElement())
					{
						for (const auto& ele : up.GetTrackMutual().GetElements())
						{
							std::pair<Eigen::Vector2d, std::pair<double, double> > row;
							row.first = ele.xy;
							row.second.first = 0.0;
							row.second.second = 0.0;
							error_map[ele.image_id] = row;
							if (up.image_for_userptguide_ == kInvalidImageId)
							{
								up.image_for_userptguide_ = ele.image_id;
							}
						}
						up.SetPixelRMS(0);
					}
					gcp_error_map[gcp_id] = error_map;
				}

			}
			

			return;
		}



		void ATData::UpdataGCPGlobalErrorInfo(std::map<point3D_t, std::map < image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >>& gcp_error_map, bool istopredict)
		{
		
			
			auto data_tocalc = this;
			ATData datatemp = *data_tocalc;
			
			datatemp.TransFormATData(BASESRS);
			for (const auto& gcp : datatemp.GetControlPoints())
			{
				auto gcp_id = gcp.second.GetId();

				
			}
			AI3D::CORE::ControlPoints gcps;
			
			gcps.GetPointsMutual() = datatemp.GetControlPointsMutual();
			gcps.TransformPoints(BASESRS);
			
			datatemp.GetControlPointsMutual() = gcps.GetPoints();
			
			auto& controlpoints = datatemp.GetControlPointsMutual();
			
			auto& cameras = datatemp.GetCamerasMutual();
			auto& images = datatemp.GetImagesMutual();

			std::string srsdef_begin = controlpoints_.begin()->second.GetSrs().definition;
			std::string srsdef_begin1 = controlpoints.begin()->second.GetSrs().definition;
			
			
			
			
			
			
			
			
			
			
			
			{
				
				std::vector<image_t> img_idx = datatemp.GetHasPostionImagesIds();
				if (img_idx.empty())
				{
					return;
				}
				Eigen::Vector3d sum = Eigen::Vector3d::Zero();
				Eigen::Vector3d point_first = images[*img_idx.begin()].GetPosition();
				for (int i = 0; i < img_idx.size(); i++)
				{
					sum += (images[img_idx[i]].GetPosition() - point_first);
				}
				Eigen::Vector3d position_offset = sum / img_idx.size() + point_first;
				
				
				std::vector<double> estimate_x,given_x, estimate_x_before;
				std::vector<double> estimate_y, given_y, estimate_y_before;
				std::vector<double> estimate_z, given_z, estimate_z_before;
				std::vector<int> ids_gcp_for3DError, ids_gcp_forPixError;
				for (auto& gcp_tmp : controlpoints)
				{
				
					
					
					std::map<image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > > error_map;
					auto& gcp = gcp_tmp.second;
					if (!gcp.HasGivenXYZ())
					{
						gcp_error_map[gcp_tmp.first] = error_map;
						continue;
					}

					if (!gcp.GetObjectPointMutual().HasElement())
					{
						gcp_error_map[gcp_tmp.first] = error_map;
						continue;
					}

					std::vector<Eigen::Vector2f> points;
					std::vector<Eigen::Matrix<float, 3, 4>> poses;
					
					std::vector<Eigen::Vector3d> bearing;
					std::vector<image_t> imageids;
					
					for (auto& ele : gcp.GetObjectPointMutual().GetTrackMutual().GetElements())
					{
						AI3D::CORE::Image& image = images.at(ele.image_id);	
					
						Eigen::Vector2d xy = ele.xy;
						Camera& camera = cameras[image.GetCameraId()];


						if (!image.IsPoseAndIntrinsicDefined(camera))
						{
							std::pair<Eigen::Vector2d, std::pair<double, double> > blankxy;
							blankxy.first = Eigen::Vector2d(-DBL_MAX, -DBL_MAX);
							blankxy.second.first = kInvalidError;
							blankxy.second.second = kInvalidError;
							error_map[ele.image_id] = blankxy;
							gcp_error_map[gcp_tmp.first] = error_map;
							continue;
						}
						Eigen::Vector2d  undis_xy = camera.UndistortPixel(xy);
						
						points.push_back(Eigen::Vector2f{ undis_xy.x(), undis_xy.y() });

						
						if (fabs(image.GetProjectionCenter().x()) > BIGSCENECOOR)
						{
							imageids.push_back(ele.image_id);
						}
						else 
						{
							
							poses.push_back((camera.GetCalibrationMatrix() * image.GetProjectionMatrix()).cast<float>());
							
						}
					}
					
					if (!imageids.empty() && poses.empty())
					{
						
						std::vector<Eigen::Vector3d> imagepose;
						for (auto& it : imageids)
						{
							AI3D::CORE::Image image = images.at(it);
							image.GetPositionMutual() -= position_offset;
							imagepose.push_back(image.GetPositionMutual());
							poses.push_back((cameras.at(image.GetCameraId()).GetCalibrationMatrix() * image.GetProjectionMatrix()).cast<float>());
							
						}
						
					}

					

					Eigen::Vector3d xyz;
					if (points.size() >= VALIDTRIANGLENUM)
					{
						
						
						
						
						

						


						
						
						Eigen::Vector3f xyzf = AlgorithmBase::TriangulatePoint(poses, points);
						xyz(0) = xyzf.x();
						xyz(1) = xyzf.y();
						xyz(2) = xyzf.z(); 
						if (!imageids.empty())
						{
							xyz += position_offset;;

						}

						const bool triFinite = std::isfinite(xyz.x()) && std::isfinite(xyz.y()) && std::isfinite(xyz.z());
						if (triFinite)
						{
						ids_gcp_for3DError.emplace_back(gcp_tmp.first);
						ids_gcp_forPixError.emplace_back(gcp_tmp.first);
						gcp.GetObjectPointMutual().GetEstimatedXYZMutual() = xyz;
						
						
						
						estimate_x.push_back(xyz(0));
						estimate_y.push_back(xyz(1));
						estimate_z.push_back(xyz(2));
						given_x.push_back(gcp.GetGivenXYZMutual().x());
						given_y.push_back(gcp.GetGivenXYZMutual().y());
						given_z.push_back(gcp.GetGivenXYZMutual().z());
						}
						else
						{
						xyz = Eigen::Vector3d{ -DBL_MAX, -DBL_MAX, -DBL_MAX };
						gcp.GetObjectPointMutual().GetEstimatedXYZMutual() = xyz;
						gcp.GetEstimatedXYZMutual() = xyz;
						ids_gcp_forPixError.emplace_back(gcp_tmp.first);
						}

					}
					else
					{
						xyz = Eigen::Vector3d{ -DBL_MAX, -DBL_MAX, -DBL_MAX };
						gcp.GetObjectPointMutual().GetEstimatedXYZMutual() = xyz;
						gcp.GetEstimatedXYZMutual() = xyz;
						ids_gcp_forPixError.emplace_back(gcp_tmp.first);
						
					}
				}

				
				
				
				
				{
				ControlPoints gcpsestimated;
				gcpsestimated.GetPointsMutual() = controlpoints_;
				int index1 = 0;
				for (const auto& gcp_id : ids_gcp_for3DError)
				{
					gcpsestimated.GetPointsMutual().at(gcp_id).GetEstimatedXYZMutual() = 
						Eigen::Vector3d(estimate_x[index1], estimate_y[index1], estimate_z[index1]);
					index1++;
				}
				gcpsestimated.TransformEstimatedXYZToGivenXYZSrs(BASESRS);

					

					 index1 = 0;
					for (const auto& gcp_id : ids_gcp_for3DError)
					{

						controlpoints_.at(gcp_id).GetEstimatedXYZMutual() = gcpsestimated.GetPointsMutual().at(gcp_id).GetEstimatedXYZMutual();
						index1++;
					}
				}
				 
				 
				 int index = 0;
				 for (const auto& gcp_id : ids_gcp_for3DError)
				 {
					 std::map<image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > > error_map;

					

					 double dx = estimate_x[index] - given_x[index];
					 double dy = estimate_y[index] - given_y[index];
					 double dz = estimate_z[index] - given_z[index];
					 double  dxy = std::sqrt(dx * dx + dy * dy);
					 double  dxyz = std::sqrt(dx * dx + dy * dy + dz * dz);

					
					 
					 if (!CoordinateTransformer::IsSame(controlpoints_.at(gcp_id).GetSrs().definition, BASESRS)	)			
					 {
						 dz = controlpoints_.at(gcp_id).GetEstimatedXYZ().z() - controlpoints_.at(gcp_id).GetGivenXYZ().z();
						 dxy = std::sqrt(dxyz * dxyz - dz * dz);
						 
					 }

					
					
				
					
					 controlpoints_.at(gcp_id).Set3DError(dxyz);
					 controlpoints_.at(gcp_id).SetZ3DError(dz);
					 controlpoints_.at(gcp_id).SetX3DError(dx);
					 controlpoints_.at(gcp_id).SetY3DError(dy);
					 controlpoints_.at(gcp_id).SetXY3DError(dxy);
					 index++;
				 }
				 index = 0;
				 auto Atdata = this;
				 ATData ATdata_tmp = *Atdata;
				 ATdata_tmp.TransFormImages(ATdata_tmp.GetLocalSrs(), BASESRS);
				 ATdata_tmp.TransFormTiepoints(ATdata_tmp.GetLocalSrs(), BASESRS);
				 ATdata_tmp.TransFormGCPs(ATdata_tmp.GetLocalSrs(), BASESRS);

				 for (const auto& gcp_id : ids_gcp_forPixError)
				 {
					 std::map<image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > > error_map;
					
					 
					 {
						 if (controlpoints.at(gcp_id).GetObjectPointMutual().HasElement())
						 {
							 datatemp.UpdateGCPMeasurementError(gcp_id, error_map, istopredict);
							 double reproj_error_sum = 0.0;
							 double dist_error_sum = 0.0;
							 bool haserror = false;
							 int ele_size = error_map.size();
							 for (auto& error : error_map)
							 {
								 double reproj_error = error.second.second.first;
								 double dist_error = error.second.second.second;
								 if (reproj_error == -DBL_MAX || dist_error == DBL_MAX)
								 {
									 continue;
								 }
								 const double squared_reproj_error = reproj_error * reproj_error;
								 const double dist_reproj_error = dist_error * dist_error;
								 {
									 
									 reproj_error_sum += squared_reproj_error;
									 dist_error_sum += dist_reproj_error;
									 haserror = true;
								 }
							 }
							 if (!haserror)
							 {
								 continue;
							 }
							 reproj_error_sum = std::sqrt(reproj_error_sum / ele_size);
							 
							 dist_error_sum = std::sqrt(dist_error_sum / ele_size);
							 controlpoints_.at(gcp_id).GetObjectPointMutual().SetPixelRMS(reproj_error_sum);
							 controlpoints_.at(gcp_id).GetObjectPointMutual().SetDistRMS(dist_error_sum);
						 }
					 }
					gcp_error_map[gcp_id] = error_map;
					index++;
				}
				
			}
			

			return;
		}


		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		

		
		


		
		

		
		
		

		
		
		
		
		
		
		
		
		
		

		
		
		
		
		
		
		
		
		
		
		



		
		
		
		
		
		
		

		
		
		
		
		

		


		



		
		
		


		
		
		
		
		
		
		
		
		

		
		

		bool ATData::ComputeUserPtEstimatedXYZ(point3D_t id)
		{
			if (!HasRegImages())
			{
				return false;
			}
			if (!user_points3D_.count(id))
				return false;
			auto& gcp = user_points3D_.at(id);
			
			if (gcp.GetTrack().Length()<2)
			{
				return false;
			}
			std::vector<Eigen::Vector2f> points;
			std::vector<Eigen::Matrix<float, 3, 4>> poses;
			std::vector<Eigen::Matrix<double, 3, 4>> poseds;
			std::vector<Eigen::Vector3d> bearing;
			std::vector<image_t> imageids;
			for (auto& ele : gcp.GetTrackMutual().GetElements())
			{
				AI3D::CORE::Image image = images_[ele.image_id];

				Eigen::Vector2d xy = ele.xy;
				Camera& camera = cameras_[image.GetCameraId()];


				
				

				Eigen::Vector2d  undis_xy = camera.UndistortPixel(xy);
				
				points.push_back(Eigen::Vector2f{ undis_xy.x(), undis_xy.y() });

				
				if (fabs(image.GetProjectionCenter().x()) > BIGSCENECOOR)
				{
					imageids.push_back(ele.image_id);
				}
				else {
					poses.push_back((camera.GetCalibrationMatrix() * image.GetProjectionMatrix()).cast<float>());
					
				}
			}

			if (!imageids.empty() && poses.empty())
			{
				Eigen::Vector3d originpt = images_[imageids[0]].GetPosition();
				for (auto& it : imageids)
				{
					AI3D::CORE::Image image = images_[it];
					image.GetPositionMutual() -= originpt;
					poses.push_back((cameras_[image.GetCameraId()].GetCalibrationMatrix() * image.GetProjectionMatrix()).cast<float>());
					
				}
			}



			if (points.size() >= VALIDTRIANGLENUM)
			{
				
				
				
				
				

				Eigen::Vector3f xyzf = AlgorithmBase::TriangulatePoint(poses, points);
				Eigen::Vector3d xyz(xyzf.x(), xyzf.y(), xyzf.z()); 
				if (!imageids.empty())
				{
					xyz += images_[imageids[0]].GetPosition();;

				}

				gcp.GetEstimatedXYZMutual() = xyz;
				gcp.SetXYZ(xyz);
			
				return true;
			}
			else
			{
				Eigen::Vector3d xyz{ -DBL_MAX, -DBL_MAX, -DBL_MAX };
				
				gcp.GetEstimatedXYZMutual() = xyz;
				return false;
			}
			return false;
		}
		
		
		bool ATData::ComputeGCPEstimatedXYZ(point3D_t id)
		{
			if (!HasRegImages())
			{
				return false;
			}
			if (!controlpoints_.count(id))
				return false;
			auto& gcp = controlpoints_.at(id);
			if (!gcp.HasGivenXYZ())
			{
				return false;
			}
			if (!gcp.GetObjectPointMutual().HasElement())
			{
				return false;
			}
			std::vector<Eigen::Vector2f> points;
			std::vector<Eigen::Matrix<float, 3, 4>> poses;
			std::vector<Eigen::Matrix<double, 3, 4>> poseds;
			std::vector<Eigen::Vector3d> bearing;
			std::vector<image_t> imageids;
			for (auto& ele : controlpoints_[id].GetObjectPointMutual().GetTrackMutual().GetElements())
			{
				AI3D::CORE::Image image = images_[ele.image_id];
				
				Eigen::Vector2d xy = ele.xy;
				Camera& camera = cameras_[image.GetCameraId()];
				
				
				
				
				
				Eigen::Vector2d  undis_xy = camera.UndistortPixel(xy);
				
				points.push_back(Eigen::Vector2f{ undis_xy.x(), undis_xy.y() });
				
				
				if (fabs(image.GetProjectionCenter().x()) > BIGSCENECOOR)
				{
					imageids.push_back(ele.image_id);
				}
				else {
					poses.push_back((camera.GetCalibrationMatrix() * image.GetProjectionMatrix()).cast<float>());
					
				}
			}
			
			if (!imageids.empty() && poses.empty())
			{
				Eigen::Vector3d originpt = images_[imageids[0]].GetPosition();
				for (auto& it : imageids)
				{
					AI3D::CORE::Image image = images_[it];
					image.GetPositionMutual() -= originpt;
					poses.push_back(( cameras_[image.GetCameraId()].GetCalibrationMatrix() * image.GetProjectionMatrix()).cast<float>());
					
				}
			}
			
			

			if (points.size() >= VALIDTRIANGLENUM)
			{
				
				
				
				
				
				
     				Eigen::Vector3f xyzf = AlgorithmBase::TriangulatePoint(poses, points);
				Eigen::Vector3d xyz(xyzf.x(), xyzf.y(), xyzf.z()); 
				if (!imageids.empty())
				{
					xyz += images_[imageids[0]].GetPosition();;
					
				}
				
				
				controlpoints_[id].GetObjectPointMutual().GetEstimatedXYZMutual() = xyz;

				

				CoordinateTransformer::Transform(1,&xyz[0], &xyz[1], &xyz[2], 
					CoordinateDescriptor::GetSRSFromDefinition(GetLocalSrs()).definition, controlpoints_[id].GetSrs().definition);
				controlpoints_[id].GetEstimatedXYZMutual() = xyz;
				
				return true;
			}
			else
			{
				Eigen::Vector3d xyz{ -DBL_MAX, -DBL_MAX, -DBL_MAX };
				controlpoints_[id].GetObjectPointMutual().GetEstimatedXYZMutual() = xyz;
				controlpoints_[id].GetEstimatedXYZMutual() = xyz;
				return false;
			}		
			return false;
		}

		bool ATData::UpdateTiepoints()
		{
			if (btiepoints_changed_ && points3D_.empty())
			{
				return true;
			}

			std::vector<point3D_t> point3d_ids;
			for (const auto& tp : points3D_)
			{
				
				if (!tp.second.GetStatus())
				{
					continue;
				}
				point3d_ids.emplace_back(tp.first);
			}
			
			
			if (point3d_ids.size() == 0)
			{
				return false;
			}

			int num_cpu_core = omp_get_num_procs();
			int num_thread = num_cpu_core >= 3 ? 3 : num_cpu_core;




			for (int i = 0; i < point3d_ids.size(); i++)
			{
				auto& tiepoint = points3D_[point3d_ids[i]];
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				std::vector<Eigen::Vector2f> points;
				std::vector<Eigen::Matrix<float, 3, 4>> poses;

				std::vector<image_t> imageids;
				for (auto& ele : tiepoint.GetTrackMutual().GetElements())
				{
					AI3D::CORE::Image image = images_[ele.image_id];

					Eigen::Vector2d xy = ele.xy;
					Camera& camera = cameras_[image.GetCameraId()];

					
					
					
					
					
					
					

					Eigen::Vector2d  undis_xy = camera.UndistortPixel(xy);
					points.push_back(Eigen::Vector2f{ undis_xy.x(), undis_xy.y() });

					if (fabs(image.GetProjectionCenter().x()) > BIGSCENECOOR)
					{
						imageids.push_back(ele.image_id);
					}
					else {
						poses.push_back((camera.GetCalibrationMatrix() * image.GetProjectionMatrix()).cast<float>());
					}

				}
				
				if (!imageids.empty() && poses.empty())
				{
					Eigen::Vector3d originpt = images_[imageids[0]].GetPosition();
					for (auto& it : imageids)
					{
						AI3D::CORE::Image image = images_[it];
						image.GetPositionMutual() -= originpt;
						poses.push_back((cameras_[image.GetCameraId()].GetCalibrationMatrix() * image.GetProjectionMatrix()).cast<float>());
						
					}
				}


				if (points.size() >= VALIDTRIANGLENUM)
				{
					
					
					
					
					

					Eigen::Vector3f xyzf = AlgorithmBase::TriangulatePoint(poses, points);
					Eigen::Vector3d xyz(xyzf.x(), xyzf.y(), xyzf.z()); 
					if (!imageids.empty())
					{
						xyz += images_[imageids[0]].GetPosition();;

					}

					tiepoint.GetEstimatedXYZMutual() = xyz;
				}
				else
				{
					Eigen::Vector3d xyz{ -DBL_MAX, -DBL_MAX, -DBL_MAX };
					tiepoint.GetEstimatedXYZMutual() = xyz;
					
					
					
					
					
				}
				tiepoint.SetStatus(false);
			}
			return true;
		}

		
		
		
		
		

		Camera& ATData::GetCameraMutual(const camera_t camera_id) 
		{
			return cameras_.at(camera_id);
		}

		AI3D::CORE::Image& ATData::GetImageMutual(const image_t image_id) 
		{
			return images_.at(image_id);
		}

		Point3D& ATData::GetPoint3DMutual(const point3D_t point3D_id) 
		{
			return points3D_.at(point3D_id);
		}

		class MeasureConstraint& ATData::GetConstraintMutual(const constraint_t constraint_id) {
			return constraintList_.at(constraint_id);
		}

		std::pair<size_t, size_t>& ATData::GetImagePairMutual(
			const image_pair_t pair_id) 
		{
			return image_pairs_.at(pair_id);
		}

		
		
		
		
		

		const EIGEN_STL_UMAP(camera_t, Camera)& ATData::GetCameras() const
		{
			return cameras_;
		}

		inline  EIGEN_STL_UMAP(camera_t, Camera)& ATData::GetCamerasMutual()
		{
			return cameras_;
		}

		const EIGEN_STL_UMAP(image_t, AI3D::CORE::Image)& ATData::GetImages() const
		{
			return images_;
		}
	
		EIGEN_STL_UMAP(image_t, AI3D::CORE::Image)& ATData::GetImagesMutual()
		{
			return images_;
		}

		const EIGEN_STL_UMAP(point3D_t, ControlPoint)& ATData::GetControlPoints() const
		{
			return controlpoints_;
		}
		EIGEN_STL_UMAP(point3D_t, ControlPoint)& ATData::GetControlPointsMutual()
		{
			return controlpoints_;
		}

		const EIGEN_STL_UMAP(constraint_t, MeasureConstraint)& ATData::GetConstraints() const {
			return constraintList_;
		}
		EIGEN_STL_UMAP(constraint_t, MeasureConstraint)& ATData::GetConstraintsMutual() {
			return constraintList_;
		}
		void  ATData::ClearControlPoints()
		{
			controlpoints_.clear();
		}

		void ATData::SetControlPoints(const EIGEN_STL_UMAP(point3D_t, ControlPoint)& controlpoints)
		{
			controlpoints_ = controlpoints;
		}

		void ATData::ClearConstraints() {
			constraintList_.clear();
		}
		inline void ATData::SetConstraints(const EIGEN_STL_UMAP(constraint_t, MeasureConstraint)& constraints) {
			constraintList_ = constraints;
		}
		const std::vector<image_t> ATData::GetRegImageIds() const 
		{
			return reg_image_ids_;
		}

		std::vector<image_t>& ATData::GetRegImageIdsMutual()
		{
			return reg_image_ids_;
		}

		void ATData::SetRegImageIds(const std::vector<image_t>& reg_image_ids)
		{
			reg_image_ids_ = reg_image_ids;
		}
		const EIGEN_STL_UMAP(point3D_t, Point3D)& ATData::GetPoints3D() const 
		{
			return points3D_;
		}

		inline EIGEN_STL_UMAP(point3D_t, Point3D)& ATData::GetPoints3DMutual()
		{
			return points3D_;
		}

		bool  ATData::HasUserTiepoints() const
		{
			return !user_points3D_.empty();
		}

		inline EIGEN_STL_UMAP(point3D_t, Point3D)& ATData::GetUserPoints3DMutual()
		{
			return user_points3D_;
		}
		inline const EIGEN_STL_UMAP(point3D_t, Point3D)& ATData::GetUserPoints3D() const
		{
			return user_points3D_;
		}
		const std::unordered_map<image_pair_t, std::pair<size_t, size_t>>&
			ATData::GetImagePairs() const 
		{
			return image_pairs_;
		}
		bool ATData::HasSurveyPoints() const
		{
			return HasControlPoints() | HasUserTiepoints();
		}
		bool ATData::HasControlPoints() const
		{
			return controlpoints_.size() > 0;
		}

		bool ATData::HasConstraints() const {
			return constraintList_.size() > 0;
		}

		bool ATData::ExistsCamera(const camera_t camera_id) const 
		{
			return cameras_.find(camera_id) != cameras_.end();
		}

		bool ATData::ExistsCameraBeta(const Camera camera) const
		{
			for (const auto& camera_itr : cameras_)
			{
				int width = 0;
				int height = 0;
				float sensorsize = 0;
				float focal_length = 0;
			}
			return false;
		}

		bool ATData::ExistsImage(const image_t image_id) const 
		{
			return images_.find(image_id) != images_.end();
		}

		bool ATData::ExistsPoint3D(const point3D_t point3D_id) const
		{
			return points3D_.find(point3D_id) != points3D_.end();
		}

		bool ATData::ExistsImagePair(const image_pair_t pair_id) const 
		{
			return image_pairs_.find(pair_id) != image_pairs_.end();
		}

		Eigen::Vector3d ATData::ComputeAvgPosition()
		{
			
			double x_avg, y_avg, z_avg;
			x_avg = y_avg = z_avg = 0;
			int count = 0;
			for (const auto& image : images_)
			{
				if (image.second.HasPosition())
				{
					x_avg += image.second.GetPosition().x();
					y_avg += image.second.GetPosition().y();
					z_avg += image.second.GetPosition().z();
					count++;
				}

			}
			x_avg = x_avg / count;
			y_avg = y_avg / count;
			z_avg = z_avg / count;
			positon_avg_ = { x_avg,y_avg,z_avg };
			return positon_avg_;
		}

		void ATData::SetPoint3DStatus(image_t image_id)
		{
			AI3D::CORE::Image& image = images_[image_id];
			for (int point2D_id = 0; point2D_id < image.GetNumPoints2D(); point2D_id++)
			{
				point3D_t point3d_id =image.GetPoint2D(point2D_id).GetPoint3DId();
				points3D_[point3d_id].SetStatus(true);
			}
		}

		bool ATData::GetPoint3DsStatus()const
		{
			return btiepoints_changed_;
		}
		bool& ATData::GetPoint3DsStatusMutual()
		{
			return btiepoints_changed_;
		}
		void ATData::SetPoint3DsStatus(bool btiepoints_changed)
		{
			btiepoints_changed_ = btiepoints_changed;
		}
		void ATData::Transform(const SimilarityTransform3& tform)
		{
			const double sim_scale = tform.Scale();
			for (auto& image : images_) 
			{
				
				const auto& qvec = AlgorithmBase::RotationMatrixToQuaternion(image.second.GetRotationMatrix());
				Eigen::Matrix3x4d src_matrix = AlgorithmBase::ComposeProjectionMatrix(image.second.GetRotationMatrix(), image.second.GetPosition());
				Eigen::Matrix3x4d dst_matrix;
				tform.TransformPose(src_matrix,dst_matrix);
			
			
				image.second.GetPositionMutual() = AlgorithmBase::ProjectionCenterFromMatrix(dst_matrix);
				image.second.GetRotationMatrixMutual()= AlgorithmBase::RotationCenterFromMatrix(dst_matrix);
			}
			for (auto& point3D : points3D_) 
			{
				tform.TransformPoint(&point3D.second.GetXYZMutual());
			}
		}

		bool ATData::IsConstraintScaleSimilarityAlreadyApplied() const
		{
			EIGEN_STL_UMAP(constraint_t, MeasureConstraint) scaleConstraints;
			for (const auto& constraintItem : constraintList_) {
				if (constraintItem.second.GetType() == CONSTRAINT_TYPE::CONSTRAINT_SCALE) {
					const constraint_t id = constraintItem.second.GetId();
					scaleConstraints[id] = constraintItem.second;
				}
			}
			if (scaleConstraints.empty()) {
				return false;
			}
			MeasureGroup measureGroup;
			measureGroup.SetConstraintList(scaleConstraints);
			const double scale = measureGroup.calAveScale();
			if (scale <= 0.0) {
				return false;
			}
			const double apply_scale = 1.0 / scale;
			return std::fabs(apply_scale - 1.0) < 1e-12;
		}

		bool ATData::handleConstraint() {
			
			if (HasRegImages() && !user_points3D_.empty()) {
				std::map<point3D_t, std::map<image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > > >
					tmp_user_err;
				UpdataUserTiepointsGlobalErrorInfo(tmp_user_err, false);
			}
			for (auto& constraintItem : constraintList_) {
				for (auto& pt : constraintItem.second.GetPointList()) {
					const point3D_t pid = pt.first;
					if (user_points3D_.count(pid)) {
						auto& up = user_points3D_.at(pid);
						if (HasRegImages() && up.GetTrack().Length() >= VALIDTRIANGLENUM &&
						    !up.HasEstimatedXYZ()) {
							ComputeUserPtEstimatedXYZ(pid);
						}
						pt.second = user_points3D_.at(pid);
					}
					else if (points3D_.count(pid)) {
						pt.second = points3D_.at(pid);
					}
				}
			}
			
			EIGEN_STL_UMAP(constraint_t, MeasureConstraint) scaleConstraints;
			for (auto& constraintItem : constraintList_) {
				auto ctype = constraintItem.second.GetType();
				if (ctype == CONSTRAINT_TYPE::CONSTRAINT_SCALE) {
					constraint_t id = constraintItem.second.GetId();
					scaleConstraints[id] = constraintItem.second;
				}
			}
			if(scaleConstraints.size() > 0){
				MeasureGroup measureGroup;
				measureGroup.SetConstraintList(scaleConstraints);
				double scale = measureGroup.calAveScale();
				if (scale <= 0.0) {
					LOGI("scale <=0 ");
					return false;
				}
				
				const double apply_scale = 1.0 / scale;
				LOGI("apply_scale: " + std::to_string(apply_scale));
				
				if (std::fabs(apply_scale - 1.0) < 1e-12) {
					LOGI("apply_scale ~1, skip similarity");
					return false;
				}
				const SimilarityTransform3 tform(
					apply_scale,
					AlgorithmBase::ComposeIdentityQuaternion(),
					Eigen::Vector3d(0.0, 0.0, 0.0));
				Transform(tform);
				for (auto& image : images_)
				{
					if (image.second.HasDepth()) {
						image.second.GetDepthMutual() *= apply_scale;
						image.second.GetFrustumMutual().clear();
					}
					else if (!image.second.GetFrustumMutual().empty()) {
						for (auto& corner : image.second.GetFrustumMutual()) {
							tform.TransformPoint(&corner);
						}
					}
				}

				for (auto& point : user_points3D_) {
					if (point.second.HasXYZ()) {
						tform.TransformPoint(&point.second.GetXYZMutual());
					}
					if (point.second.HasEstimatedXYZ()) {
						tform.TransformPoint(&point.second.GetEstimatedXYZMutual());
					}
				}
				for (auto& gcp : controlpoints_) {
					Point3D& obj = gcp.second.GetObjectPointMutual();
					tform.TransformPoint(&obj.GetXYZMutual());
					if (obj.HasEstimatedXYZ()) {
						tform.TransformPoint(&obj.GetEstimatedXYZMutual());
					}
					if (gcp.second.HasEstimatedXYZ()) {
						tform.TransformPoint(&gcp.second.GetEstimatedXYZMutual());
					}
				}
				for (auto& constraintItem : constraintList_) {
					for (auto& pt : constraintItem.second.GetPointList()) {
						if (pt.second.HasXYZ()) {
							tform.TransformPoint(&pt.second.GetXYZMutual());
						}
						if (pt.second.HasEstimatedXYZ()) {
							tform.TransformPoint(&pt.second.GetEstimatedXYZMutual());
						}
					}
				}
				SetPoint3DsStatus(true);
				
				if (!points3D_.empty()) {
					ComputeTileBoundingBox(bb_scope_e::BB_SCOPE_TIEPOINTS, false);
				}
				return true;
			}
			return false;
		}

		bool ATData::LoadConstraint(const std::string& path) {
			std::ifstream in = File::OpenIfstreamUtf8(path, std::ios::binary);
			
			if (!in.is_open()) {
				LOGE("Load constraint bin failed!");
				return false;
			}

			ConstraintFile constraintFile;
			constraintFile.Deserialize(in);
			constraintList_.clear();
			bool kvValid = true;
			for (auto& constraintItem : constraintFile.constraintsVec) {
				constraint_t id = constraintItem.id;
				MeasureConstraint measureConstraint;
				measureConstraint.SetId(id);
				measureConstraint.SetName(constraintItem.name);
				CONSTRAINT_TYPE ctype = static_cast<CONSTRAINT_TYPE>(constraintItem.type);
				measureConstraint.SetType(ctype);
				EIGEN_STL_UMAP(point3D_t, Point3D) measurePointList;
				measurePointList.clear();
				for (auto& storedPid : constraintItem.pointIds) {
					const point3D_t pid = static_cast<point3D_t>(storedPid);
					Point3D point;
					if (user_points3D_.count(pid)) {
						point = user_points3D_.at(pid);
					}
					else if (points3D_.count(pid)) {
						point = points3D_.at(pid);
					}
					else {
						point.SetId(pid);
					}
					measurePointList[pid] = point;
				}
				measureConstraint.SetPointList(measurePointList);
				std::vector<double> kvs = constraintItem.values;
				std::vector<ConstraintKV> newkvs;
				newkvs.clear();

				if (ctype == CONSTRAINT_TYPE::CONSTRAINT_SCALE) {
					if (kvs.size() < 2) {
						kvValid = false;
						break;
					}
					ConstraintKV dist(CONSTRAINT_KEY_TYPE::KEY_DOUBLE, kvs[0]);
					newkvs.push_back(dist);
					ConstraintKV unit(CONSTRAINT_KEY_TYPE::KEY_INT, kvs[1]);
					newkvs.push_back(unit);
				}
				else if (ctype == CONSTRAINT_TYPE::CONSTRAINT_ORIGIN) {
					if (kvs.size() < 1) {
						kvValid = false;
						break;
					}
					ConstraintKV id(CONSTRAINT_KEY_TYPE::KEY_INDEX, kvs[0]);
					newkvs.push_back(id);
				}
				else if (ctype == CONSTRAINT_TYPE::CONSTRAINT_AXIS) {
					if (kvs.size() < 1) {
						kvValid = false;
						break;
					}
					ConstraintKV axisType(CONSTRAINT_KEY_TYPE::KEY_AXIS, kvs[0]);
					newkvs.push_back(axisType);
				}
				else if (ctype == CONSTRAINT_TYPE::CONSTRAINT_PLAIN) {
					if (kvs.size() < 2) {
						kvValid = false;
						break;
					}
					ConstraintKV axisType(CONSTRAINT_KEY_TYPE::KEY_AXIS, kvs[0]);
					newkvs.push_back(axisType);
					ConstraintKV directType(CONSTRAINT_KEY_TYPE::KEY_DIRECTION, kvs[1]);
					newkvs.push_back(directType);
				}
				else {

				}
				measureConstraint.SetConstraintItemList(newkvs);
				constraintList_[id] = measureConstraint;
			}

			in.close();
			if (!kvValid) {
				return kvValid;
			}
			return true;
		}

		bool ATData::SaveConstraint(const std::string& outpath) {
			std::ofstream out = File::OpenOfstreamUtf8(outpath, std::ios::binary);
			if (!out.is_open()) {
				LOGE("Save constraint bin failed!");
				return false;
			}
			ConstraintFile constraintFile;
			std::vector<ConstraintData> constraintsVec;
			for (auto& constraintItem : constraintList_) {
				ConstraintData constraintData;
				constraintData.id = constraintItem.second.GetId();
				constraintData.name = constraintItem.second.GetName();
				constraintData.type = static_cast<int>(constraintItem.second.GetType());
				constraintData.pointNum = constraintItem.second.GetPointList().size();
				std::vector<unsigned long long> pointIds;
				for (auto& measureItem : constraintItem.second.GetPointList()) {
					const unsigned long long pointId =
						static_cast<unsigned long long>(measureItem.first);
					pointIds.push_back(pointId);
				}
				constraintData.pointIds = pointIds;
				constraintData.valueNum = constraintItem.second.GetConstraintItemList().size();
				std::vector<double> values;
				for (auto& cvalue : constraintItem.second.GetConstraintItemList()) {
					double tmpValue = cvalue.getDoubleValue();
					values.push_back(tmpValue);
				}
				constraintData.values = values;
				constraintsVec.push_back(constraintData);
			}
			constraintFile.constraintNum = constraintsVec.size();
			constraintFile.constraintsVec = constraintsVec;
			constraintFile.Serialize(out);
			out.close();
			return true;
		}
		
	}
} 
