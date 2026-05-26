#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <mutex>
#include <Eigen/Core>
#include <fstream>
#include <Constants.h>
#include "Core/Camera.h"
#include "Core/ProjectObject.h"
#include "Core/ATData.h"
#include "Core/Image.h"
#include "Core/Point2d.h"
#include "Core/Point3d.h"
#include "Core/Track.h"
#include "Core/alignment.h"
#include "Core/Types.h"
#include "Core/ReturnCode.h"
#include "Core/BlockObject.h"
#include "Core/Rapidjson.h"
#include "Core/ReturnCode.h"
#include "Core/Timer.h"
#include "Core/CoordinateSystem.h"
#include "Util/TaskProcess.h"
#include <filesystem>
#include <glog/logging.h>
#include "Core/SimilarityTransform.h"
#include "Core/TaskDef.h"
#include "Core/File.h"

namespace AI3D
{
	namespace CORE
	{
		ProjectObject::ProjectObject() {
			if (BlockObject::supportTempLogs())
			{
				std::ostringstream ss;
				ss << "project object constructor 1";
				LOGI(ss.str());
				
				for (auto& block_ptr : blocks_)
				{
					ss << "inside pb" << block_ptr.first << std::hex << std::showbase << block_ptr.second  << std::dec;
					LOGI(ss.str());
					std::cout << ss.str() << std::endl;

				}

				for (auto& block_id : blockids_)
				{
					ss << "inside pb" << block_id;
					LOGI(ss.str());
					std::cout << ss.str() << std::endl;
				}

			}

			for (auto& block_ptr : blocks_)
			{
				block_ptr.second = nullptr;
			}

			if (BlockObject::supportTempLogs())
			{
				std::ostringstream ss;
				ss << "project object constructor 2";
				LOGI(ss.str());
				
				for (auto& block_ptr : blocks_)
				{
					ss << "inside pb" << block_ptr.first << std::hex << std::showbase << block_ptr.second << std::dec;
					LOGI(ss.str());
					std::cout << ss.str() << std::endl;

				}
			}

		};
		
		
		

		
		
		
		

		
		int ProjectObject::NewProject(std::string name, std::string path)
		{
			
			try
			{
				if (!std::filesystem::exists(File::BoostPathFromUtf8(path)))
					return FILE_NOTEXISTS_ERROR;
				std::string postFix = "";
				if (PROJECT_USE_BIN) {
					postFix = BINDOTPROJECTPOSTFIX;
				}
				else {
					postFix = DOTPROJECTPOSTFIX;
				}
				std::string fullfile = path + "/" + name + "/" + name + postFix;
				
				if (std::filesystem::exists(File::BoostPathFromUtf8(fullfile)))
				{
					return FILE_DUPLICATED_ERROR;
				}
			}
			catch (std::filesystem::filesystem_error& fse)
			{
				std::ostringstream oss;
				oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
				LOGI(oss.str());
				return FILE_NOTEXISTS_ERROR;
			}
			catch (std::exception& ex)
			{
				std::ostringstream oss;
				oss << "exception:" << ex.what();
				LOGI(oss.str());
				return FILE_NOTEXISTS_ERROR;
			}

			name_ = name;
			path_ = path + PATH_SEPARATOR_STR + name;
			path_ = File::EnsureUnifySlash(path_);
			return true;
		}
		
		bool ProjectObject::Load(const std::string& name)
		{
			try
			{
				if (!std::filesystem::is_regular_file(File::BoostPathFromUtf8(name)))
				{
					LOG(ERROR) << "Project file is not exist!";
					std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
				}
			}
			catch (std::filesystem::filesystem_error& fse)
			{
				std::ostringstream oss;
				oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
				LOGI(oss.str());
				std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
				return false;
			}
			catch (std::exception& ex)
			{
				std::ostringstream oss;
				oss << "exception:" << ex.what();
				LOGI(oss.str());
				std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
				return false;
			}

			path_ = File::GetParentDir(name);
			
			if (PROJECT_USE_BIN) {
				if (!LoadBin(name))
				{
					LOG(ERROR) << "Load Project bin file error!";
					std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
					return false;
				}
			}
			else {
				if (!LoadJson(name))
				{
					LOG(ERROR) << "Load Project json file error!";
					std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
					return false;
				}
			}
			
			
			
			
			
			
			return true;
		}

		
		bool ProjectObject::Check()
		{
			for (auto it = blocks_.begin(); it != blocks_.end(); it++)
			{
				std::string blockpath = "";
				std::string sourcedatapath = "";
				if (BLK_USE_BIN) {
					
					blockpath = it->second->GetPath() + PATH_SEPARATOR_STR + it->second->GetName() + BLOCKBINFILE;
				}
				else {
					
					blockpath = it->second->GetPath() + PATH_SEPARATOR_STR + it->second->GetName() + BLOCKFILE;
				}
				if (SOURCEDATA_USE_BIN) {
					sourcedatapath = it->second->GetPath() + PATH_SEPARATOR_STR + ORIDATABIN;
				}
				else {
					
					sourcedatapath = it->second->GetPath() + PATH_SEPARATOR_STR + ORIDATAJSON;
				}

				try
				{
					if (!std::filesystem::is_regular_file(File::BoostPathFromUtf8(blockpath)) || !std::filesystem::is_regular_file(File::BoostPathFromUtf8(sourcedatapath)))
					{
						LOG(ERROR) << "Unvalid Block_" << it->first;
						return false;
					}
				}
				catch (std::filesystem::filesystem_error& fse)
				{
					std::ostringstream oss;
					oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
					LOGI(oss.str());
					return false;
				}
				catch (std::exception& ex)
				{
					std::ostringstream oss;
					oss << "exception:" << ex.what();
					LOGI(oss.str());
					return false;
				}
			}
			return true;
		}
		
		void ProjectObject::AddBlock(BlockObject* block)
		{
			
			
			if (!ExistsBlock(block->GetId()))
			{
				block_t block_id = GenerateValidBlockId();
				block->GetIdMutual() = block_id;
				block->Init();
				block->GetTaskInfoMutual().Block_XML = path_ + PATH_SEPARATOR_STR + block->GetName() + PATH_SEPARATOR_STR + SCBLOCKBIN;
				block->GetTaskInfoMutual().Tiepoints = path_ + PATH_SEPARATOR_STR + block->GetName() + PATH_SEPARATOR_STR + TIEPOINTS;
			}
			blocks_[block->GetId()] = block;

			
			{
				std::ostringstream oss;
				oss << "inside " << __FILE__ << " " << __LINE__ << " " << std::hex << std::showbase << block << " " << std::dec << block->GetId();
				LOGI(oss.str());
			}

			
			std::vector<int> statistic;
			statistic.emplace_back(block->GetCurrentAT()->GetImages().size()); 
			statistic.emplace_back(block->GetCurrentAT()->GetNumRegImages());
			statistic.emplace_back(block->GetCurrentAT()->GetControlPoints().size());
			statistic.emplace_back(block->GetCurrentAT()->GetPoints3D().size());
			statistic.emplace_back(0);
			blocks_statistics_[block->GetId()] = statistic;
			
			block->GetTaskInfoMutual().statisticinfo_.tiepointnum =  statistic[3];	
			if (block->CanSubmitRecon())
			{

				block->SetStatus(jobsta_e::STATUS_COMPLETE);
				block->SetAT0(block->GetATData());
				block->GetTaskInfoMutual().isFinished = true;
			}
			else
			{
				
				block->SetStatus(jobsta_e::STATUS_NEW);
			}
		};


		
		bool ProjectObject::ImportBlock(const std::string& name)
		{
			
			 
			Timer time;
			time.Start();
		   
			
			std::string blockpath = path_;
			BlockObject* block = new BlockObject(blockpath);
			if (BlockObject::supportTempLogs())
			{
				std::ostringstream oss;
				oss << "create bo:" << std::hex << std::showbase << block << std::dec;
			
			}
			
			block->setImportFilename(const_cast<std::string &>(name));

			bool ret = false;
			ret = block->LoadExternalFile(name);
			LOGD("Load " + name + " spends "+std::to_string(time.ElapsedSeconds()));
			

			if (ret)
			{
				AddBlock(block);
				
				std::string blockstring = block->GetTaskInfoMutual().blockString;
				blockstring.append("(");
				blockstring.append(File::GetPathBaseName(name).substr(0, File::GetPathBaseName(name).find_last_of(".")));
				blockstring.append(")");
				block->GetTaskInfoMutual().blockString = blockstring;
				std::string postFix = "";
				if (PROJECT_USE_BIN) {
					postFix = BINDOTPROJECTPOSTFIX;
				}
				else {
					postFix = DOTPROJECTPOSTFIX;
				}
				block->GetTaskInfoMutual().projectfile_ = path_ + "/" + name_  + postFix;
				time.Restart();

				
				if (!block->GetCurrentAT()->GetControlPoints().empty() && 1)
				{
					
					for (auto& cam : block->GetCurrentATMutual()->GetCamerasMutual())
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

					std::set<image_t> images_ids;
					
					for (const auto& gcp : block->GetCurrentATMutual()->GetControlPointsMutual())
					{
						block->GetCurrentAT()->PredictGCPMeasurement(gcp.first, images_ids, true);
					}
					for (const auto& gcp : block->GetCurrentATMutual()->GetUserPoints3DMutual())
					{
						block->GetCurrentAT()->PredictUserPtMeasurement(gcp.first, images_ids, true);
					}
				}

				LOGI("******************import block finished!***************************");
				return true;
			}

			LOGI("******************import block failed!***************************");
			return false;

		};


		block_t ProjectObject::GetNumBlocks()
		{
			return blocks_.size();
		}
		
		class BlockObject* ProjectObject::GetBlock(block_t id)
		{
			for (auto it: blocks_)
			{
				if (it.first == id)
				{
					return it.second;
				}

			}
			return nullptr;
		}

		class BlockObject* ProjectObject::GetBlockMutual(block_t id)
		{
			return blocks_.at(id);
		}

		class BlockObject* ProjectObject::GetBlockByImportFilename(const std::string &importFilename)
		{
			for (auto it : blocks_)
			{
				if (it.second->getImportFilename() == importFilename)
					return it.second;
			}
			return nullptr;
		}

		bool ProjectObject::DeleteBlock(const block_t id)
		{
			CHECK_OPTION(ExistsBlock(id));
			BlockObject block = *blocks_.at(id);
			std::string block_dir = block.GetPath();

			
			{
				std::ostringstream oss;
				oss << "inside " << __FILE__ << " " << __LINE__ << " delete block:" << std::hex << std::showbase
					<< " " << &block << " " << blocks_.at(id) << " " << std::dec << id;
				LOGI(oss.str());
			}

			
			try
			{
				if (std::filesystem::exists(File::BoostPathFromUtf8(block_dir)))
				{
					
					CHECK_OPTION(File::Remove(block_dir));
				}
			}
			catch (std::filesystem::filesystem_error& fse)
			{
				std::ostringstream oss;
				oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
				LOGI(oss.str());
			}
			catch (std::exception& ex)
			{
				std::ostringstream oss;
				oss << "exception:" << ex.what();
				LOGI(oss.str());
			}

			blocks_.erase(id);
			
			blocks_statistics_.erase(id);

			
			
			if (PROJECT_USE_BIN) {
				SaveBin(path_, savetype_e::PROJECT_SAVED);
			}
			else {
				SaveJson(path_, savetype_e::PROJECT_SAVED);
			}
			
			return true;
		}
		bool ProjectObject::MergeAndAdjustBlocks(const std::set<block_t>& blockids)
		{
			
			if (blockids.size() != 2)
				return false;
			
			auto& block1 = blocks_.at(*blockids.begin());
			auto& block2 = blocks_.at(*blockids.rbegin());
			block1->LoadTiepoints();
			block2->LoadTiepoints();
			
			ATData& reconstruction1 = *block1->GetCurrentAT();
			ATData& reconstruction2 = *block2->GetCurrentAT();

			
			std::string srs_src = reconstruction1.GetLocalSrs(), srs_dst = reconstruction2.GetLocalSrs();
			if (!CoordinateTransformer::IsSame(srs_src, srs_dst))
			{
				reconstruction1.TransFormImages(srs_src, srs_dst);
				reconstruction1.TransFormTiepoints(srs_src, srs_dst);
				
				
				reconstruction1.SetLocalSrs(srs_dst);
				
				block1->SetBlockSRS(srs_dst);
				
			}
			
			std::set<image_t> common_image_ids1, common_image_ids2;
			reconstruction1.FindCommonRegImages(reconstruction2, common_image_ids1, common_image_ids2);
			
			
				
				std::vector<image_t> comids1_vec, comids2_vec;
				comids1_vec.assign(common_image_ids1.begin(), common_image_ids1.end());
				comids2_vec.assign(common_image_ids2.begin(), common_image_ids2.end());
				

				std::set<point3D_t> ptidx1, ptidx2;
				std::set<std::pair<point3D_t, point3D_t>> commonpoint3didx;
				for (size_t i = 0; i < comids1_vec.size(); ++i)
				{
					const image_t image_id1 = comids1_vec[i];
					const image_t image_id2 = comids2_vec[i];
					const AI3D::CORE::Image& image1 = reconstruction1.GetImage(image_id1);
					const AI3D::CORE::Image& image2 = reconstruction2.GetImage(image_id2);

					int count = 0;
					std::set<point2D_t> pointidx1, pointidx2;
					std::set<std::pair<point2D_t, point2D_t>> pointidx;

					
					for (point2D_t point2D_idx1 = 0; point2D_idx1 < image1.GetNumPoints2D();
						++point2D_idx1)
					{
						const auto& point2D1 = image1.GetPoint2D(point2D_idx1);
						if (!point2D1.HasPoint3D()) {
							continue;
						}
						for (point2D_t point2D_idx2 = 0; point2D_idx2 < image2.GetNumPoints2D();
							++point2D_idx2)
						{
							const auto& point2D2 = image2.GetPoint2D(point2D_idx2);
							if (!point2D2.HasPoint3D()) {
								continue;
							}
							double dist = (point2D1.GetXY() - point2D2.GetXY()).norm();
							if (dist < 0.3333)
							{
								count++;
								
								const auto& ret1 = pointidx1.insert(point2D_idx1);
								const auto& ret2 = pointidx2.insert(point2D_idx2);
								if (ret1.second && ret2.second)
								{
									pointidx.insert(std::make_pair(point2D_idx1, point2D_idx2));
									const auto& retpt1 = ptidx1.insert(point2D1.GetPoint3DId());
									const auto& retpt2 = ptidx2.insert(point2D2.GetPoint3DId());
									if (retpt1.second && retpt2.second)
									{
										commonpoint3didx.insert(std::make_pair(point2D1.GetPoint3DId(), point2D2.GetPoint3DId()));
										ptidx1.insert(point2D1.GetPoint3DId());
										ptidx2.insert(point2D2.GetPoint3DId());
									}
								}
								break;
							}
						}
					}
				}
				std::cout << ptidx1.size() << " " << ptidx2.size() << " " << commonpoint3didx.size() << std::endl;
				
				std::map<point3D_t, int> pt1_meas, pt2_meas;
				std::vector<int> rr;
				rr.resize(11);
				std::set<image_t> commonpt_image_ids1, commonpt_image_ids2;
				ATData at1 = reconstruction1;
				ATData at2 = reconstruction2;
				std::set<std::pair<point3D_t, point3D_t>>  newptids;
				for (auto& iter : commonpoint3didx)
				{
					auto& track1 = reconstruction1.GetPoint3D(iter.first).GetTrack();
					auto& track2 = reconstruction2.GetPoint3D(iter.second).GetTrack();
					std::set<std::string> names1, names2;
					for (auto& mea : track1.GetElements())
					{
						std::string name1 = reconstruction1.GetImage(mea.image_id).GetName();

						names1.insert(name1);
						std::cout << name1 << " " << std::endl;
					}
					for (auto& mea : track2.GetElements())
					{
						std::string name2 = reconstruction2.GetImage(mea.image_id).GetName();
						names2.insert(name2);
						std::cout << name2 << std::endl;
					}
					int count = 0;
					for (auto& img : names1)
					{
						if (names2.count(img))
						{
							count++;
						}
					}
					int mincnt = std::min(names1.size(), names2.size());

					float r = float(count) / float(mincnt);
					int ri = r * 10;
					rr[ri]++;
					if (ri == 10 && names1.size() == names2.size())
					{

						for (auto& nameiter : names1)
						{

							auto image1 = at1.FindImageWithName(nameiter, at1.GetRegImageIds());
							auto image2 = at2.FindImageWithName(nameiter, at2.GetRegImageIds());
							if (image1 && image2)
							{
								commonpt_image_ids1.insert(image1->GetImageId());
								commonpt_image_ids2.insert(image2->GetImageId());
								newptids.insert(std::make_pair(iter.first, iter.second));
							}
						}
					}
				}

				std::cout << commonpoint3didx.size() << " " << at1.GetNumPoints3D() << " " << at2.GetNumPoints3D() << " " << newptids.size() << std::endl;

				std::vector<image_t> comptids1_vec, comptids2_vec;
				comptids1_vec.assign(commonpt_image_ids1.begin(), commonpt_image_ids1.end());
				comptids2_vec.assign(commonpt_image_ids2.begin(), commonpt_image_ids2.end());
				std::cout << " " << at1.GetNumPoints3D() << " " << at2.GetNumPoints3D() << " "
					<< at1.GetNumImages() << " " << at2.GetNumImages() << " " << std::endl;
				std::set<point3D_t> ptforat1, ptforat2;
				for (auto& iter : newptids)
				{
					ptforat1.insert(iter.first);
					ptforat2.insert(iter.second);
				}
				for (auto& iter : at1.GetPoint3DIds())
				{
					if (!ptforat1.count(iter))
						at1.DeletePoint3D(iter);

				}

				for (auto& iter : at2.GetPoint3DIds())
				{
					if (!ptforat2.count(iter))
						at2.DeletePoint3D(iter);

				}

				std::set<image_t> imageids;
				for (auto& iter : at1.GetImages())
				{
					imageids.insert(iter.first);
				}
				for (auto& iter : imageids)
				{
					auto& image = at1.GetImagesMutual()[iter];
					if (!image.IsRegistered() || (image.GetNumPoints2D() < 3) || (!commonpt_image_ids1.count(iter)))
					{
						at1.DeleteImage(iter);
					}

				}

				for (auto& iter : at2.GetImages())
				{
					if (!iter.second.IsRegistered() || (iter.second.GetNumPoints2D() < 3) || (!commonpt_image_ids2.count(iter.first)))
					{
						at2.DeleteImage(iter.first);
					}
				}
				std::cout << " " << at1.GetNumPoints3D() << " " << at2.GetNumPoints3D() << " "
					<< at1.GetNumImages() << " " << at2.GetNumImages() << " " << std::endl;
				
				bool alignment_success = true;
				int min_common_images = 3;
				RANSACOptions ransac_options;
				SimilarityTransform3 tform1;
				std::vector<std::string> ref_image_names;
				std::vector<Eigen::Vector3d> ref_locations;
				ransac_options.max_error = 0.6;
				for (auto& iter : at2.GetImages())
				{
					ref_image_names.push_back(iter.second.GetName());
					ref_locations.push_back(iter.second.GetPosition());
				}
				
				alignment_success = reconstruction1.AlignRobust(
					ref_image_names, ref_locations, min_common_images, ransac_options,
					&tform1);

				BlockObject newblk1 = *block1, newblk2 = *block2;

				if (BlockObject::supportTempLogs())
				{
					std::ostringstream oss;
					oss << "create bo:" << std::hex << std::showbase << &newblk1 << " " << &newblk2 << std::dec;
				
				}

				std::shared_ptr<ATData> atptr1 = std::make_shared<ATData>(reconstruction1);
				std::shared_ptr<ATData> atptr2 = std::make_shared<ATData>(reconstruction2);
				newblk1.SetAT0(atptr1);
				newblk2.SetAT0(atptr2);
				BlockObject::BlockExportOptions opt;
				opt.export_tiepoint_ = true;
				newblk1.ExportATXML("D:/1-N.xml", opt);
				newblk2.ExportATXML("D:/2-N.xml", opt);

			double min_inlier_observations = 0.3;
			double max_reproj_error = 1.0;
			Eigen::Matrix3x4d alignment;
			if (!ComputeAlignmentBetweenReconstructions(at1, at2,
				min_inlier_observations,
				max_reproj_error, &alignment)) {
				std::cout << "=> Reconstruction alignment failed" << std::endl;
				return EXIT_FAILURE;
			}
			block1->ExportATXML("D:/1-1.xml");
			block2->ExportATXML("D:/2-1.xml");
			const SimilarityTransform3 tform(alignment);
			std::cout << "Computed alignment transform:" << std::endl
				<< tform.Matrix() << std::endl;

			const size_t num_images = commonpt_image_ids1.size();
			std::vector<double> rotation_errors(num_images, 0.0);
			std::vector<double> translation_errors(num_images, 0.0);
			std::vector<double> proj_center_errors(num_images, 0.0);
			
			for (size_t i = 0; i < num_images; ++i) 	
			{
				const image_t image_id1 = comptids1_vec[i];
				const image_t image_id2 = comptids2_vec[i];
				AI3D::CORE::Image& image1 = reconstruction1.GetImageMutual(image_id1);
				AI3D::CORE::Image& image2 = reconstruction2.GetImageMutual(image_id2);
				const auto& qvec2 = AlgorithmBase::RotationMatrixToQuaternion(image2.GetRotationMatrix());
				const auto& qvec1 = AlgorithmBase::RotationMatrixToQuaternion(image1.GetRotationMatrix());
				Eigen::Matrix3x4d proj = AlgorithmBase::ComposeProjectionMatrix(image2.GetRotationMatrix(), image2.GetPosition());
				Eigen::Vector3d tvec2 = proj.rightCols<1>();
				
				Eigen::Matrix3x4d proj2;
				tform1.TransformPose(proj, proj2);
				image2.GetPositionMutual() = AlgorithmBase::ProjectionCenterFromMatrix(proj2);
				
				proj_center_errors[i] =
					(image1.GetPosition() - image2.GetPosition()).norm();
			}

			MergeBlocks(blockids);
			return true;
		}


		
		
		bool ProjectObject::MergeBlocks(const std::set<block_t>& blockids)
		{
			BlockObject* block = new BlockObject();
			block_t id = GenerateValidBlockId();
			std::string blockString;
			std::string blockName;
			std::string mergedFrom;
			std::string blockSRSDefinition;
			bool isAllENUSRS = true;
			bool blocksHasTiepoints = false;

			
			{
				std::ostringstream oss;
				oss << "inside " << __FILE__ << " " << __LINE__ << " " << std::hex << std::showbase
					<< block << " " << std::dec << id;
				LOGI(oss.str());
			}

			for (const auto& block_id : blockids)
			{

				{
					if (!blocks_.at(block_id)->GetTaskInfo().isLoaded)
					{
						LoadBlockData(block_id);
						blocks_.at(block_id)->GetTaskInfoMutual().isLoaded = true;
					}
				}


				mergedFrom += blocks_.at(block_id)->GetTaskInfo().blockString + ",";
				if (!blocks_.at(block_id)->GetCurrentAT()->GetLocalSrs().empty())
				{
					blockSRSDefinition = BASESRS;
				}

				if (blocks_.at(block_id)->GetCurrentAT()->HasPriorPositionImages())
				{
					srs_s wgs84;
					wgs84.ID = 0;
					wgs84.definition = GEO84SRS;
					wgs84.name = CoordinateDescriptor::GetSRSFromDefinition(wgs84.definition).name;
					wgs84.type = coord_system_type_e::GEOGRAPHIC;
					block->GetSRSsMutual().insert(std::make_pair(0, wgs84));
				}
			}


			mergedFrom = mergedFrom.substr(0, mergedFrom.size() - 1);
			mergedFrom = "merged from: " + mergedFrom;
			blockString = "Block_" + std::to_string(id) + "(" + std::to_string(blockids.size()) + " blocks merged)";
			blockName = "Block_" + std::to_string(id);
			std::string blockPath = path_ + PATH_SEPARATOR_STR + blockName;
			block->GetTaskInfoMutual().blockString = blockString;

			block->GetTaskInfoMutual().mergedFrom = mergedFrom;
			block->SetName(blockName);
			block->SetPath(blockPath);
			block->SetId(id);



			block->GetTaskInfoMutual().Block_XML = path_ + PATH_SEPARATOR_STR + block->GetName() + PATH_SEPARATOR_STR + SCBLOCKBIN;
			block->GetTaskInfoMutual().Tiepoints = path_ + PATH_SEPARATOR_STR + block->GetName() + PATH_SEPARATOR_STR + TIEPOINTS;
			block->GetTaskInfoMutual().blockId = id;

			EIGEN_STL_UMAP(group_t, PhotoGroup) pgs;
			ATData AtdataMerge;

			bool isFirstblock = true;
			image_t imageidend = kInvalidImageId;
			camera_t cameraid = kInvalidCameraId;
			point3D_t gcpidend = kInvalidPoint3DId;
			point3D_t tiepointend = kInvalidPoint3DId;
			point3D_t photogroupidend = kInvalidGroupId;
			
			bool bincompleteblock = false;
			for (const auto& blockid : blockids)
			{
				blocks_.at(blockid)->LoadTiepoints();
				if (blocks_.at(blockid)->GetCurrentAT()->GetATCompleteStatus() == AT_complete_status_e::INCOMPLETE_PHOTOS)
				{
					bincompleteblock = true;
				}
				AI3D::CORE::ATData* AtdataNew = new AI3D::CORE::ATData();
				*AtdataNew = *blocks_.at(blockid)->GetCurrentAT().get();
				
				auto photogroups = blocks_.at(blockid)->GetPhotoGroups();

				if (AtdataNew->HasControlPoints())
				{
					for (auto& gcp : AtdataNew->GetControlPointsMutual())
					{
						srs_s srs = gcp.second.GetSrs();
						block->UpdateSRSMap(srs);
						gcp.second.SetSrs(std::find_if(block->GetSRSs().begin(), block->GetSRSs().end(), [srs](std::pair<srsid_t, srs_s> srstmp) {return srs.definition == srstmp.second.definition; })->second);
					}
				}

				if (AtdataNew->HasTiepoints())
				{
					blocksHasTiepoints = true;
				}

				
				if (AtdataNew->HasPriorPositionImages())
				{
					for (auto& img : AtdataNew->GetImagesMutual())
					{
						img.second.SetPriorSrs(block->GetSRSs().at(0));
					}
				}

				
				if (AtdataNew->GetLocalSrs() != blockSRSDefinition && !AtdataNew->GetLocalSrs().empty())
				{
					AtdataNew->TransFormImages(AtdataNew->GetLocalSrs(), blockSRSDefinition);
					AtdataNew->TransFormTiepoints(AtdataNew->GetLocalSrs(), blockSRSDefinition);
					AtdataNew->TransFormGCPs(AtdataNew->GetLocalSrs(), blockSRSDefinition);
				}

				if (isFirstblock)
				{

					imageidend = *AtdataNew->GetImagesIdSet().rbegin() + 1;
					cameraid = AtdataNew->GenerateValidCameraId();
					gcpidend = AtdataNew->GenerateValidGCPId();
					tiepointend = AtdataNew->GenerateValidPoint3DId();
					photogroupidend = blocks_.at(blockid)->GenerateValidPhotoGroupId();
					isFirstblock = false;

					AtdataMerge = *AtdataNew;
					for (const auto& pg : photogroups)
					{
						pgs.insert(pg);
					}
				}
				else
				{
					
					for (auto& photogroup : photogroups)
					{
						PhotoGroup pg = photogroup.second;
						pg.SetId(photogroupidend);
						pg.GetCameraMutual().SetCameraId(photogroupidend);
						Camera camera = AtdataNew->GetCamera(photogroup.first);
						camera.SetCameraId(photogroupidend);
						AtdataMerge.AddCamera(camera);
						
						std::set<image_t> imgids;
						for (const auto& imageid : pg.GetGroupImageIds())
						{
							AI3D::CORE::Image img = AtdataNew->GetImage(imageid);
							img.SetPhotoGroupID(photogroupidend);
							img.SetCameraId(photogroupidend);
							img.SetImageId(imageidend);
							imgids.insert(imageidend);
							
							for (int point2d_idx = 0; point2d_idx < img.GetNumPoints2D(); point2d_idx++)
							{
								Point3D& point3d = AtdataNew->GetPoint3DMutual(img.GetPoint2D(point2d_idx).GetPoint3DId());
								for (auto& ele : point3d.GetTrackMutual().GetElementsMutual())
								{
									if (ele.image_id == imageid)
									{
										ele.image_id = imageidend;
										break;
									}
								}
							}
							
							if (img.HasGCPs())
							{
								for (auto& point2dgcp : img.GetGcpsPoint2DMutual())
								{
									ControlPoint& cp = AtdataNew->GetControlPointsMutual()[point2dgcp.first];
									for (auto& ele : cp.GetObjectPointMutual().GetTrackMutual().GetElementsMutual())
									{
										if (ele.image_id == imageid)
										{
											ele.image_id = imageidend;
											break;
										}
									}
								}
							}
							if (img.IsRegistered())
							{
								AtdataMerge.GetRegImageIdsMutual().emplace_back(imageidend);
							}
							AtdataMerge.AddImage(img);
							imageidend++;
						}
						photogroupidend++;
						pg.SetGroupImage(imgids);
						pgs.insert(std::make_pair(pg.GetId(), pg));
					}


					
					for (const auto& tp : AtdataNew->GetPoints3D())
					{
						Point3D point3d = tp.second;
						
						
						point3d.SetId(tiepointend);
						AtdataMerge.GetPoints3DMutual().insert(std::make_pair(tiepointend, point3d));
						tiepointend++;
					}
					
					for (auto& cp : AtdataNew->GetControlPointsMutual())
					{
						ControlPoint controlpoint = cp.second;
						for (auto& ele : controlpoint.GetObjectPointMutual().GetTrackMutual().GetElementsMutual())
						{
							AI3D::CORE::Image& img = AtdataMerge.GetImageMutual(ele.image_id);
							img.GetGcpsPoint2DMutual().erase(cp.first);
							img.GetGcpsPoint2DMutual().insert(std::make_pair(gcpidend, ele.xy));
						}
						controlpoint.SetId(gcpidend);
						AtdataMerge.GetControlPointsMutual().insert(std::make_pair(gcpidend, controlpoint));
						gcpidend++;
					}
				}
			}

			AtdataMerge.SetLocalSrs(blockSRSDefinition);
			AtdataMerge.SetOriginSrs(srs_s().definition);
			auto Atdata_tmp = std::make_shared<ATData>(AtdataMerge);
			block->SetATData(Atdata_tmp);
			if (!bincompleteblock)
			{
				block->SetStatus(job_status_e::STATUS_COMPLETE);
				block->SetAT0(Atdata_tmp);
			}
			else
			{
				block->SetStatus(job_status_e::STATUS_NEW);

			}
			
			block->SetBlockSRS(blockSRSDefinition);




			
			block->GetCurrentAT()->SetPoint3DsStatus(blocksHasTiepoints);
			block->GetPhotoGroupsMutual() = pgs;

			blockids_.insert(id);
			blocks_.insert(std::make_pair(id, block));
			
			std::vector<int>statics;
			statics.emplace_back(AtdataMerge.GetImages().size());
			statics.emplace_back(AtdataMerge.GetRegImageIds().size());
			statics.emplace_back(AtdataMerge.GetControlPoints().size());
			statics.emplace_back(AtdataMerge.GetPoints3D().size());
			blocks_statistics_.insert(std::make_pair(id, statics));
			block->GetTaskInfoMutual().projectfile_ = blocks_.at(*blockids.cbegin())->GetTaskInfoMutual().projectfile_;
			block->GetTaskInfoMutual().statisticinfo_.imagenum = statics[0];
			block->GetTaskInfoMutual().statisticinfo_.regisimagenum = statics[1];
			block->GetTaskInfoMutual().statisticinfo_.gcpnum = statics[2];
			block->GetTaskInfoMutual().statisticinfo_.tiepointnum = statics[3];
			return true;
		}
		class BlockObject* ProjectObject::GetCurrentBlock()
		{
			auto endid = blockids_.end();
			BlockObject* block = GetBlock(*(--endid));
			return block;

		}
		
		block_t ProjectObject::GenerateValidBlockId()
		{
			block_t block_id = kInvalidBlockId;


			for (auto& it : blockids_)
			{
				block_id = it;
			}
			block_id++;


			blockids_.insert(block_id);
			return block_id;
		}
		
		std::string ProjectObject::GenerateValidBlockName(std::string rawname)
		{
			std::set<std::string> blockname_set;
			for (auto iter : blocks_)
			{
				std::string blockname = iter.second->GetTaskInfo().blockString;
				blockname_set.insert(blockname);
			}
			
			if (blockname_set.count(rawname))
			{
				
				int num = 2;
				std::string newname = rawname + "(" + std::to_string(num) + ")";
				while (blockname_set.count(newname))
				{
					num++;
					newname = rawname + "(" + std::to_string(num) + ")";
				}
				return newname;
			}
			else
			{
				return rawname;
			}
		}


		
		bool ProjectObject::CloneBlock(block_t& id, bool AT_USED, std::string prefix)
		{
			if (!ExistsBlock(id))
			{
				LOGE(String::StringPrintf("Block is not Exist!"));
				return false;
			}
			blocks_.at(id)->LoadTiepoints();
			BlockObject* block = new BlockObject("");
			*block = *blocks_.at(id);
			
			if (BlockObject::supportTempLogs())
			{
				std::ostringstream oss;
				oss << "create bo:" << std::hex << std::showbase << block << std::dec;
			
			}

			BlockObject::Task_Info& block_info = block->GetTaskInfoMutual();
			
			block_info.atjson_ = "";
			block_info.gcpjson_ = "";
			BlockObject::Task_Info block_info_origin = block->GetTaskInfo();
			
			if (AT_USED)
			{
				block_info.AT_Num = 0;
				
				block->SetStatus(jobsta_e::STATUS_PENDDING);
				
				
				if (!block->GetATGroupMutual().empty())
				{
					block->SetATData(block->GetATGroupMutual().at(0).GetATDataMutual());
					block->GetATGroupMutual().erase(0);
					
				}
			
			}

			block_t block_id = GenerateValidBlockId();
			id = block_id;
			block->SetId(block_id);
			block->GetTaskInfoMutual().blockId = block_id;
		
			
			{
				std::ostringstream oss;
				oss << "inside " << __FILE__ << " " << __LINE__ << " " << std::hex << std::showbase
					<< block << " " << std::dec << block_id;
				LOGI(oss.str());
			}

			std::string newblockname = block->GetTaskInfo().blockString + prefix;
			
			newblockname = GenerateValidBlockName(newblockname);

			block->GetTaskInfoMutual().blockString = newblockname;
			block->GetNameMutual() = BLOCK_PRE + std::to_string(block_id);

			
			std::string block_path_origin = block->GetPath();
			block_path_origin = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(block_path_origin)));
			std::string currentdir, lastdir;
			File::GetLastSecondDir(block_path_origin, currentdir, lastdir);
			block->GetPathMutual() = AI3D::CORE::File::EnsureUnifySlash( currentdir + PATH_SEPARATOR_STR + block->GetName());
			

			
			std::string Block_XML = block->GetPath() + PATH_SEPARATOR_STR + SCBLOCKBIN;
			std::string Tiepoints = block->GetPath() + PATH_SEPARATOR_STR + TIEPOINTS;
			block_info.Block_XML = Block_XML;
			block_info.Tiepoints = Tiepoints;
			block_info.reconstructions_info_.clear();
			block_info.reconstructionjobs_.clear();
			
			
			
			block->GetTaskInfoMutual().isSaved = false;
			block->GetTaskInfoMutual().isFinished= false;
			block->GetTaskInfoMutual().isLoaded = false;
			blocks_[block->GetId()] = block;
			

			if (!AT_USED)
			{
				
				try
				{
					
					File::CopyDirectory(block_path_origin, block->GetPath(), false);
				}
				catch (const std::exception& err)
				{
					LOG(ERROR) << err.what();
					return false;
				}
			}
			return true;

		}

		bool ProjectObject::CloneBlock(BlockObject* block, std::string prefix)
		{
		

			BlockObject::Task_Info& block_info = block->GetTaskInfoMutual();

			block_info.atjson_ = "";
			block_info.gcpjson_ = "";
			BlockObject::Task_Info block_info_origin = block->GetTaskInfo();
			
			

			block_t block_id = GenerateValidBlockId();
			
			block->SetId(block_id);
			block->GetTaskInfoMutual().blockId = block_id;

			
			{
				std::ostringstream oss;
				oss << "inside " << __FILE__ << " " << __LINE__ << " " << std::hex << std::showbase
					<< block << " " << std::dec << block_id;
			}

			std::string newblockname = block->GetTaskInfo().blockString + prefix;

			newblockname = GenerateValidBlockName(newblockname);

			block->GetTaskInfoMutual().blockString = newblockname;
			block->GetNameMutual() = BLOCK_PRE + std::to_string(block_id);

			
			std::string block_path_origin = block->GetPath();
			block->GetPathMutual() = File::GetParentDir(block_path_origin) + PATH_SEPARATOR_STR + block->GetName();


			
			std::string Block_XML = block->GetPath() + PATH_SEPARATOR_STR + SCBLOCKBIN;
			std::string Tiepoints = block->GetPath() + PATH_SEPARATOR_STR + TIEPOINTS;
			block_info.Block_XML = Block_XML;
			block_info.Tiepoints = Tiepoints;
			block_info.reconstructions_info_.clear();
			block_info.reconstructionjobs_.clear();

			
			
			block->GetTaskInfoMutual().isSaved = false;
			block->GetTaskInfoMutual().isFinished = false;
			block->GetTaskInfoMutual().isLoaded = false;
			blocks_[block->GetId()] = block;
			
			

			
			{
				
				try
				{
					
					File::CopyDirectory(block_path_origin, block->GetPath(), false);
				}
				catch (const std::exception& err)
				{
					LOG(ERROR) << err.what();
					return false;
				}
			}
			return true;

		}



		void ProjectObject::Clear()
		{
			name_ = "";
			path_ = "";
			blockids_.clear();
			blocks_.clear();
			srs_ids_.clear();
			blocks_statistics_.clear();
		}

		
		const std::string ProjectObject::GetName()const
		{
			return name_;
		}
		const std::string ProjectObject::GetPath()const
		{
			return path_;
		}
		const std::string ProjectObject::GetFullName() const
		{
			std::string postFix = "";
			if (PROJECT_USE_BIN) {
				postFix = BINDOTPROJECTPOSTFIX;
			}
			else {
				postFix = DOTPROJECTPOSTFIX;
			}
			std::string fullfile = path_ + "/" + name_ + postFix;
			return fullfile;
		}
		std::string& ProjectObject::GetNameMutual()
		{
			return name_;
		}
		void ProjectObject::ReName(std::string name)
		{
			
			GetNameMutual() = name;
		}

		bool ProjectObject::ExistsBlock(block_t block_id)
		{
			return blocks_.find(block_id) != blocks_.end();
		};

		bool ProjectObject::LoadBin(const std::string& path)
		{
			std::ifstream in = AI3D::CORE::File::OpenIfstreamUtf8(path, std::ios::binary);
			
			if (!in.is_open()) {
				std::cout << "Load project bin failed!" << std::endl;
				return false;
			}

			ProjectFile projectFile;
			projectFile.Deserialize(in);
			std::string projectName = projectFile.projectName;
#ifdef WIN32
			// projectName = UTF82GBK(projectName);
#endif
			name_ = projectName;
			int blockNum = projectFile.blockNum;
			bool mokhasblockstatus = false;
			for (int i = 0; i < blockNum; i++)
			{
				block_t block_id = kInvalidBlockId;
				BlockObject* block = new BlockObject;
				std::string blockpath;
				std::string blockname;

				blockname = projectFile.blocks[i].blockName;
				block_id = std::stoi(blockname.substr(6));
				blockpath = projectFile.blocks[i].blockPath;
#ifdef WIN32
				// blockname = UTF82GBK(blockname);
				// blockpath = UTF82GBK(blockpath);
#endif 

				block->SetName(blockname);				
				block->SetId(block_id);
				if (BlockObject::supportTempLogs())
				{
					std::ostringstream oss;
					oss << "create bo:" << std::hex << std::showbase << block << std::dec;
					
				}				

				
				blockpath = File::GetParentDir(path) + blockpath.substr(1);
				if (!File::IsFileExistent(blockpath))
				{
					LOGE(std::string(blockpath) + " is not exist.");
					std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
					return false;
				}
				blockpath = File::GetParentDir(blockpath);
				block->SetPath(blockpath);

				
				
				
				
				
				
				
				
				
				
				std::string block_filepath = "";
				if (BLK_USE_BIN) {
					block_filepath = blockpath + PATH_SEPARATOR + blockname + BLOCKBINFILE;
					
				}
				else {
					block_filepath = blockpath + PATH_SEPARATOR + blockname + BLOCKFILE;
					
				}
				
				
				if (!block->Load(block_filepath))
				{
					std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
					return false;
				}

				block->GetTaskInfoMutual().projectfile_ = path_ + PATH_SEPARATOR_STR + name_ + BINDOTPROJECTPOSTFIX;
				if (mokhasblockstatus && !block->GetTaskInfoMutual().hasstatisinfo)
				{
					block->GetTaskInfoMutual().statisticinfo_.imagenum = blocks_statistics_[block_id][0];
					block->GetTaskInfoMutual().statisticinfo_.regisimagenum = blocks_statistics_[block_id][1];
					block->GetTaskInfoMutual().statisticinfo_.gcpnum = blocks_statistics_[block_id][2];
					block->GetTaskInfoMutual().statisticinfo_.tiepointnum = blocks_statistics_[block_id][3];
					try
					{
						if (BLK_USE_BIN) {
							std::filesystem::copy_file(File::BoostPathFromUtf8(block_filepath), File::BoostPathFromUtf8(block_filepath + BLOCKBINFILE), std::filesystem::copy_options::overwrite_existing);
						}
						else {
							std::filesystem::copy_file(File::BoostPathFromUtf8(block_filepath), File::BoostPathFromUtf8(block_filepath + BLOCKFILE), std::filesystem::copy_options::overwrite_existing);
						}

					}
					catch (std::filesystem::filesystem_error& fse)
					{
						std::ostringstream oss;
						oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
						LOGI(oss.str());
						std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
					}
					catch (std::exception& ex)
					{
						std::ostringstream oss;
						oss << "exception:" << ex.what();
						LOGI(oss.str());
						std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
					}

					if (BLK_USE_BIN) {
						block->GetTaskInfoMutual().WriteBlockInfoToBin(block_filepath, block->GetTaskInfoMutual().hasatsetting);
					}
					else {
						block->GetTaskInfoMutual().WriteBlockInfoToJson(block_filepath, block->GetTaskInfoMutual().hasatsetting);
					}

					blocks_statistics_.erase(block_id);
				}

				blockids_.insert(block_id);
				blocks_[block_id] = block;
			}
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			

			
			

			in.close();
			std::cout << __FILE__ << " OK " << __FUNCTION__ << " " << __LINE__ << std::endl;
			return true;
		};

		
		bool ProjectObject::LoadJson(const std::string& path)
		{
			std::string context;
			bool ret = RapidJsonCore::ReadFile(path, context);
			if (!ret)
			{
				LOGE(String::StringPrintf("File: %s was Read Error", path));
				std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
				return false;
			}
			if (context.empty())
			{
				LOGE("Project file is empty!");
				std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
				return false;
			}

			
			rapidjson::Document doc;
			if (doc.Parse(context.data()).HasParseError())
			{
				LOGE("Parse Project file ERROR!");
				std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
				return false;
			}

			if (!doc.IsObject())
			{
				LOGE("Parse Project file ERROR!");
				std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
				return false;
			}

			if (!doc.HasMember("Project"))
			{
				LOGE("Project member was lost!");
				std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
				return false;
			}
			rapidjson::Value& project = doc["Project"];

			if (project.HasMember("ProjectName"))
			{
				
				std::string name2_ = project["ProjectName"].GetString();
		// name_ = UTF82GBK(name2_);
		 name_ = name2_;
			}
			bool mokhasblockstatus = false;
			if (project.HasMember("blocks"))
			{
				rapidjson::Value& blocks = project["blocks"];
				for (int i = 0; i < blocks.Size(); i++)
				{
					block_t block_id = kInvalidBlockId;
					BlockObject* block = new BlockObject;
					std::string blockpath;
					std::string blockname;
					if (blocks[i].HasMember("blockName"))
					{
						blockname = blocks[i]["blockName"].GetString();
						block->SetName(blockname);
						block_id = std::stoi(blockname.substr(6));
						block->SetId(block_id);
					}
			
					if (BlockObject::supportTempLogs())
					{
						std::ostringstream oss;
						oss << "create bo:" << std::hex << std::showbase << block << std::dec;
					
					}

					if (blocks[i].HasMember("blockPath"))
					{

						std::string blockpath2 = blocks[i]["blockPath"].GetString();
				// blockpath = UTF82GBK(blockpath2);
				 blockpath = blockpath2;

						
						blockpath = File::GetParentDir(path) + blockpath.substr(1);
						if (!File::IsFileExistent(blockpath))
						{
							LOGE(std::string(blockpath)+" is not exist.");
							std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
							return false;
						}
						blockpath = File::GetParentDir(blockpath);
						block->SetPath(blockpath);
					}
					
					mokhasblockstatus = blocks[i].HasMember("blockStatistics");
					if (mokhasblockstatus)
					{
						for (int j = 0; j < blocks[i]["blockStatistics"].Size(); j++)
						{
							blocks_statistics_[block_id].push_back(blocks[i]["blockStatistics"][j].GetInt());
						}
					}
					
					std::string block_filepath = "";
					if (BLK_USE_BIN) {
						block_filepath = blockpath + PATH_SEPARATOR + blockname + BLOCKBINFILE;
						
					}
					else {
						block_filepath = blockpath + PATH_SEPARATOR + blockname + BLOCKFILE;
						
					}
					
					
					if (!block->Load(block_filepath))
					{
						std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
						return false;
					}
					block->GetTaskInfoMutual().projectfile_ = path_ + PATH_SEPARATOR_STR + name_ + DOTPROJECTPOSTFIX;
					if (mokhasblockstatus && !block->GetTaskInfoMutual().hasstatisinfo )
					{
						block->GetTaskInfoMutual().statisticinfo_.imagenum = blocks_statistics_[block_id][0];
						block->GetTaskInfoMutual().statisticinfo_.regisimagenum = blocks_statistics_[block_id][1];
						block->GetTaskInfoMutual().statisticinfo_.gcpnum = blocks_statistics_[block_id][2];
						block->GetTaskInfoMutual().statisticinfo_.tiepointnum = blocks_statistics_[block_id][3];
						try
						{
							if (BLK_USE_BIN) {
								std::filesystem::copy_file(File::BoostPathFromUtf8(block_filepath), File::BoostPathFromUtf8(block_filepath + BLOCKBINFILE), std::filesystem::copy_options::overwrite_existing);
							}
							else {
								std::filesystem::copy_file(File::BoostPathFromUtf8(block_filepath), File::BoostPathFromUtf8(block_filepath + BLOCKFILE), std::filesystem::copy_options::overwrite_existing);
							}
							
						}
						catch (std::filesystem::filesystem_error& fse)
						{
							std::ostringstream oss;
							oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
							LOGI(oss.str());
							std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
						}
						catch (std::exception& ex)
						{
							std::ostringstream oss;
							oss << "exception:" << ex.what();
							LOGI(oss.str());
							std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
						}

						if (BLK_USE_BIN) {
							block->GetTaskInfoMutual().WriteBlockInfoToBin(block_filepath, block->GetTaskInfoMutual().hasatsetting);
						}
						else {
							block->GetTaskInfoMutual().WriteBlockInfoToJson(block_filepath, block->GetTaskInfoMutual().hasatsetting);
						}
						
						blocks_statistics_.erase(block_id);
					}

					blockids_.insert(block_id);
					blocks_[block_id] = block;
				}
			}
			if (mokhasblockstatus)
			{
				try
				{
					std::filesystem::copy_file(File::BoostPathFromUtf8(path), File::BoostPathFromUtf8(path + DOTPROJECTPOSTFIX), std::filesystem::copy_options::overwrite_existing);
				}
				catch (std::filesystem::filesystem_error& fse)
				{
					std::ostringstream oss;
					oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
					LOGI(oss.str());
					std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
				}
				catch (std::exception& ex)
				{
					std::ostringstream oss;
					oss << "exception:" << ex.what();
					LOGI(oss.str());
					std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
				}

				SaveJson(path_, PROJECT_SAVED);
			}

			std::cout << __FILE__ << " OK " << __FUNCTION__ << " " << __LINE__ << std::endl;
			return true;
		};

		int ProjectObject::SaveBin(const std::string& path, savetype_e savetype)
		{

			std::string propath = path;

			try
			{
				
				if (!std::filesystem::exists(File::BoostPathFromUtf8(propath)))
				{
					std::filesystem::create_directory(File::BoostPathFromUtf8(propath));
					
				}

				std::string fullfile = propath + PATH_SEPARATOR_STR + name_ + BINDOTPROJECTPOSTFIX;

				
				std::string bak_file = propath + PATH_SEPARATOR_STR + name_ + BINPROJECTFILEBAK;
				if (File::IsFileExistent(fullfile))
				{
					std::filesystem::copy_file(File::BoostPathFromUtf8(fullfile), File::BoostPathFromUtf8(bak_file), std::filesystem::copy_options::overwrite_existing);
					std::filesystem::remove(File::BoostPathFromUtf8(fullfile));
				}

				std::ofstream out = AI3D::CORE::File::OpenOfstreamUtf8(bak_file, std::ios::binary);
				
				if (!out.is_open()) {
					std::cout << "Save taskdef bin failed!" << std::endl;
					return false;
				}
				ProjectFile projectFile;
				std::string projectName = name_;
#ifdef WIN32
				// projectName = GBK2UTF8(projectName);
#endif 
				projectFile.projectName = projectName;
				projectFile.blockNum = blocks_.size();
				
				projectFile.blocks.clear();
				for (EIGEN_STL_UMAP(block_t, BlockObject*)::const_iterator it = blocks_.begin(); it != blocks_.end(); it++)
				{
					BlockData blockData;
					BlockObject* block = new BlockObject();
					block = it->second;

					if (BlockObject::supportTempLogs())
					{
						std::ostringstream oss;
						oss << "create bo:" << std::hex << std::showbase << block << std::dec;
						
					}

					BlockObject::Task_Info taskinfo = block->GetTaskInfo();
					std::string blockName = block->GetName();
#ifdef WIN32
					// blockName = GBK2UTF8(blockName);
#endif 
					blockData.blockName = blockName;
					std::string blockPath2_ = "";
					if (BLK_USE_BIN) {
						blockPath2_ = ("./" + block->GetName() + PATH_SEPARATOR_STR + block->GetName() + BLOCKBINFILE);
					}
					else {
						blockPath2_ = ("./" + block->GetName() + PATH_SEPARATOR_STR + block->GetName() + BLOCKFILE);
					}
#ifdef WIN32
					// blockPath2_ = GBK2UTF8(blockPath2_);
#endif 
					blockData.blockPath = blockPath2_;
					
					
					projectFile.blocks.push_back(blockData);

					
					if (savetype != SaveType_e::PROJECT_SAVED)
					{


						if (!block->Save())
						{
							return AI3D_FAILURE;
						}
					}
				}
				projectFile.Serialize(out);
				out.close();

				std::filesystem::copy_file(File::BoostPathFromUtf8(bak_file), File::BoostPathFromUtf8(fullfile), std::filesystem::copy_options::overwrite_existing);
				std::filesystem::remove(File::BoostPathFromUtf8(bak_file));
			}
			catch (std::filesystem::filesystem_error& fse)
			{
				std::ostringstream oss;
				oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
				LOGI(oss.str());
				return AI3D_FAILURE;
			}
			catch (std::exception& ex)
			{
				std::ostringstream oss;
				oss << "exception:" << ex.what();
				LOGI(oss.str());
				return AI3D_FAILURE;
			}

			return AI3D_SUCCESS;
		}
	
	
		int ProjectObject::SaveJson(const std::string& path, savetype_e savetype)
		{

			std::string propath = path;

			try
			{
				
				if (!std::filesystem::exists(File::BoostPathFromUtf8(propath)))
				{
					std::filesystem::create_directory(File::BoostPathFromUtf8(propath));
					
				}

				std::string fullfile = propath + PATH_SEPARATOR_STR + name_ + DOTPROJECTPOSTFIX;

				
				std::string bak_file = propath + PATH_SEPARATOR_STR + name_ + PROJECTFILEBAK;
				if (File::IsFileExistent(fullfile))
				{
					std::filesystem::copy_file(File::BoostPathFromUtf8(fullfile), File::BoostPathFromUtf8(bak_file), std::filesystem::copy_options::overwrite_existing);
					std::filesystem::remove(File::BoostPathFromUtf8(fullfile));
				}

				
				rapidjson::Document document;
				document.SetObject();
				rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

				rapidjson::Value root(rapidjson::kObjectType);


				// std::string name2_ = GBK2UTF8(name_);
				std::string name2_ = name_;
				root.AddMember("ProjectName", rapidjson::Value(name2_.c_str(), allocator), allocator);


				rapidjson::Value blockArray(rapidjson::kArrayType);
				for (EIGEN_STL_UMAP(block_t, BlockObject*)::const_iterator it = blocks_.begin(); it != blocks_.end(); it++)
				{
					BlockObject* block = new BlockObject();
					block = it->second;

					if (BlockObject::supportTempLogs())
					{
						std::ostringstream oss;
						oss << "create bo:" << std::hex << std::showbase << block << std::dec;
					
					}

					BlockObject::Task_Info taskinfo = block->GetTaskInfo();
					rapidjson::Value objectTemp(rapidjson::kObjectType);
					objectTemp.AddMember("blockName", rapidjson::Value(block->GetName().c_str(), allocator), allocator);

					std::string blockPath2_ = "";
					if (BLK_USE_BIN) {
						blockPath2_ = ("./" + block->GetName() + PATH_SEPARATOR_STR + block->GetName() + BLOCKBINFILE);
					}
					else {
						blockPath2_ = ("./" + block->GetName() + PATH_SEPARATOR_STR + block->GetName() + BLOCKFILE);
					}
					// std::string blockPath_ = GBK2UTF8(blockPath2_);
					std::string blockPath_ = blockPath2_;
					objectTemp.AddMember("blockPath", rapidjson::Value(blockPath_.c_str(), allocator), allocator);

					



					blockArray.PushBack(objectTemp, allocator);

					
					if (savetype != SaveType_e::PROJECT_SAVED)
					{
						
						
						if (!block->Save())
						{
							return AI3D_FAILURE;
						}
					}
				}

				root.AddMember("blocks", blockArray, allocator);

				document.AddMember("Project", root, allocator);
				
				if (RapidJsonCore::SaveFile(bak_file, document) != AI3D_SUCCESS)
				{
					std::ostringstream oss;
					oss << "Save " << bak_file << " failed.";
					LOGI(oss.str());
					return AI3D_FAILURE;
				}

				std::filesystem::copy_file(File::BoostPathFromUtf8(bak_file), File::BoostPathFromUtf8(fullfile), std::filesystem::copy_options::overwrite_existing);
				std::filesystem::remove(File::BoostPathFromUtf8(bak_file));
			}
			catch (std::filesystem::filesystem_error& fse)
			{
				std::ostringstream oss;
				oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
				LOGI(oss.str());
				return AI3D_FAILURE;
			}
			catch (std::exception& ex)
			{
				std::ostringstream oss;
				oss << "exception:" << ex.what();
				LOGI(oss.str());
				return AI3D_FAILURE;
			}

			return AI3D_SUCCESS;
		}

		int  ProjectObject::Save(savetype_e savetype)
		{
			
			int ret = 0;
			if (PROJECT_USE_BIN) {
				ret = SaveBin(path_, savetype);
			}
			else {
				ret = SaveJson(path_, savetype);
			}

			if (ret != AI3D_SUCCESS)
			{

				std::cout << "======================Save project falied and end =================" << std::endl;
			}
			else
			{
				std::cout << "======================Save project  end =================" << std::endl;
			}
			return ret;
		}

		EIGEN_STL_UMAP(block_t, std::vector<int>) ProjectObject::GetBlocksStatisics()
		{
			return blocks_statistics_;
		}
		void ProjectObject::SetBlocksStatisics(const std::pair<block_t, std::vector<int>>& blocks_statistics)
		{
			blocks_statistics_[blocks_statistics.first] = blocks_statistics.second;
		}

		bool ProjectObject::LoadBlockData(block_t block_id)
		{
			Timer time;
			time.Start();
			BlockObject* block = blocks_.at(block_id);
			auto Atdata = std::make_shared<ATData>();
			if (!block->LoadBlockATData(Atdata))
			{
				LOGE(String::StringPrintf("Load block %s failed!", block->GetTaskInfo().blockString));
				return false;
			}
			
			
			std::string jobname = block->GetTaskInfo().job_;
			std::string jobpath = block->GetPath() +"/"+ jobname+"/";

			if (jobname != "")
			{
				
				
				
				
				std::string feedbackfile = "";
				std::string timefile = "";
				if (JOB_FEEDBACK_USE_BIN) {
					feedbackfile = MAKE_FEEDBAK_BIN_FILE(block->GetPath(), jobname);
					timefile = MAKE_TIMESUM_BIN_FILE(block->GetPath(), jobname);
				}
				else {
					feedbackfile = MAKE_FEEDBAK_JSON_FILE(block->GetPath(), jobname);
					timefile = MAKE_TIMESUM_BIN_FILE(block->GetPath(), jobname);
					
				}
				JobFeedBack_s feadback;
				std::string context;
				bool ret = RapidJsonCore::ReadFile(feedbackfile, context);
				if (!ret)
				{
					LOGE(String::StringPrintf("File: %s was Read Error", feedbackfile));
					return false;
				}
				if (context.empty())
				{
					LOGE("Project file is empty!");
					return false;
				}

				
				rapidjson::Document doc;
				if (doc.Parse(context.data()).HasParseError())
				{
					LOGE("Parse Project file ERROR!");
					return false;
				}

				if (!doc.IsObject())
				{
					LOGE("Parse Project file ERROR!");
					return false;
				}

				if (!doc.HasMember("Status"))
				{
					LOGE("Project member was lost!");
					return false;
				}
				rapidjson::Value& status = doc["Status"];
				feadback.Status = (jobsta_e)status.GetInt();
				
				block->SetStatus(feadback.Status);
				

			}
			else
			{
				block->SetStatus(jobsta_e::STATUS_NEW);
				
			}

			
			std::string postFix = "";
			if (PROJECT_USE_BIN) {
				postFix = BINDOTPROJECTPOSTFIX;
			}
			else {
				postFix = DOTPROJECTPOSTFIX;
			}
			block->GetTaskInfoMutual().projectfile_ = path_ + PATH_SEPARATOR_STR + name_ + postFix;
			
			LOGD(String::StringPrintf("Loading Block_%d spends %0.2f", block_id, time.ElapsedSeconds()));
			
			return true;

		}
		bool ProjectObject::SaveBLK(block_t block_id, savetype_e savetype)
		{
			BlockObject* block = blocks_.at(block_id);
			std::string blkpath;
			if (BLK_USE_BIN) {
				blkpath = block->GetPath() + PATH_SEPARATOR + block->GetName() + BLOCKBINFILE;
			}
			else {
				blkpath = block->GetPath() + PATH_SEPARATOR + block->GetName() + BLOCKFILE;
			}
			block->Save();
			block->GetTaskInfoMutual().isSaved = true;
			return true;
		}
	}
}




