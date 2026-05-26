

#include <vector>

#include <Eigen/Core>

#include "Gui/BlockManager.h"


namespace AI3D
{
    namespace GUI
    {

				BlockManager::BlockManager(AI3D::CORE::BlockObject* block):
					block_(nullptr)
				{
					///block_ = new AI3D::CORE::BlockObject;
					block_ = block;
				}

				Block_Status_s& BlockManager::GetBlockStatusMutual()
				{
					if (block_->HasReconstructions())
					{
						block_wgt_status_.can_AT = false;
						block_wgt_status_.can_resubAT = false;
						block_wgt_status_.can_cancle = false;
						block_wgt_status_.can_add_pos = false;
						block_wgt_status_.can_del_pos = false;
						block_wgt_status_.can_add_photo = false;
						block_wgt_status_.can_del_photo = false;
						block_wgt_status_.can_add_gcp = false;
						block_wgt_status_.can_del_gcp = false;
						
					
					}
					else
					{
						block_wgt_status_.can_AT = CanSubmitAT();
						block_wgt_status_.can_resubAT = CanReSubmitAT();
						block_wgt_status_.can_cancle = CanCancleAT();
						block_wgt_status_.can_add_pos = CanAddPoses();
						block_wgt_status_.can_del_pos = CanDeletePoses();
						block_wgt_status_.can_add_photo = CanAddPhotos();
						block_wgt_status_.can_del_photo = CanDeletePhotos();
						block_wgt_status_.can_add_gcp = CanAddGcps();
						block_wgt_status_.can_del_gcp = CanDeleteGcps();
					}
					//重建的
				

					block_wgt_status_.can_submit_rec = CanSubmitRecon();
					//std::cout << "inside " << __FILE__ << " " << __LINE__ << " can_submit_rec:" << block_wgt_status_.can_submit_rec << std::endl;

					
					return block_wgt_status_;
				}

				AI3D::CORE::BlockObject* BlockManager::GetBlockMutual()
				{
					return block_;
				}
				//chy 0816修改
				bool BlockManager::CanSubmitAT()
				{
					auto status = block_->GetStatus();
					

					if (block_->GetCurrentATMutual()!=nullptr)
					{
						int imagenum = block_->GetCurrentATMutual()->GetNumImages();
						if (status == job_status_e::STATUS_NEW || (status == job_status_e::STATUS_COMPLETE)
							/*|| status == job_status_e::STATUS_FAILURE*/)
						{
							if (imagenum >= MINIMAGENUMFORAT)
							{
								return true;
							}
						}
					}
					
					return false;				
				}

				bool BlockManager::CanSubmitRecon()
				{
					auto status = block_->GetStatus();
					//std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << status << std::endl;
					if (status != job_status_e::STATUS_COMPLETE)
					{
					//	std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " not complete:" << status << std::endl;
						return false;
					}

					//std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " complete:" << status << std::endl;
					return block_->CanSubmitRecon();
					
				}
				

				//只有失败和cancle状态下才可resubmit
				bool BlockManager::CanReSubmitAT()
				{
					auto status = block_->GetStatus();
					if (status == job_status_e::STATUS_CANCLE || status == job_status_e::STATUS_FAILURE)
					{
						return true;
					}
					return false;
				}
				//本身是cancle状态不可cancle，只有runnging状态和pending状态才有cancle
				bool BlockManager::CanCancleAT()
				{
					auto status = block_->GetStatus();
					if (status == job_status_e::STATUS_RUNNING || status == job_status_e::STATUS_PENDDING)
						return true;				
					return false;
				}
				//running\cancle状态下是绝不可加的
				bool BlockManager::CanAddPoses()
				{
					auto status = block_->GetStatus();
#ifdef USE_POS_DEBUG
					return true;
#else
					if (status == job_status_e::STATUS_NEW || status == job_status_e::STATUS_COMPLETE)
					{

						if (block_->GetCurrentATMutual() != nullptr)
						{
							if (block_->GetCurrentATMutual()->HasImages())
							{
								return true;
							}
						}
					}
					return false;
#endif
					
				}
				bool BlockManager::CanDeletePoses()
				{
					auto status = block_->GetStatus();
					if (status == job_status_e::STATUS_NEW || status == job_status_e::STATUS_COMPLETE)
					{
						if (block_->GetCurrentATMutual() != nullptr)
						{
							if (block_->GetCurrentATMutual()->HasPositionImages())
							{
								return true;
							}
						}
					}
						return false;									
				}
				bool BlockManager::CanAddPhotos()
				{
					auto status = block_->GetStatus();
					if (status == job_status_e::STATUS_NEW || status == job_status_e::STATUS_COMPLETE)
						return true;
					return false;
				}
				bool BlockManager::CanDeletePhotos()
				{				
					auto status = block_->GetStatus();
					if (status == job_status_e::STATUS_NEW || status == job_status_e::STATUS_COMPLETE)
					{
						if (block_->GetCurrentATMutual() != nullptr)
						{
							if (block_->GetCurrentATMutual()->HasImages())
							{
								return true;
							}
						}
					}
					return false;
				}
				
				bool BlockManager::CanAddGcps()
				{
#ifdef USE_POS_DEBUG
					return true;
#else
					auto status = block_->GetStatus();
					if (status == job_status_e::STATUS_NEW || status == job_status_e::STATUS_COMPLETE)
					{
						if (block_->GetCurrentATMutual() != nullptr)
						{
							if (block_->GetCurrentATMutual()->GetNumImages() > 2)
							{
								return true;
							}
						}
					}
					return false;
#endif								
				}
				bool BlockManager::CanDeleteGcps()
				{
					auto status = block_->GetStatus();
					if (status == job_status_e::STATUS_NEW || status == job_status_e::STATUS_COMPLETE)
					{
						if (block_->GetCurrentATMutual() != nullptr)
						{
							if (block_->GetCurrentATMutual()->HasControlPoints())
							{
								return true;
							}
						}
					}
					return false;
				}
				bool BlockManager::CanSubmitAT(AI3D::CORE::ATOptions option)
				{
					//首先需要确定options是肯定了
					if (block_->GetCurrentATMutual() != nullptr)
					{
						if (option.align_mode == ALIGN_WITHGCP)
						{
							size_t valid_gcp_count = block_->GetCurrentATMutual()->GetNumValidControlPoints();

							if (valid_gcp_count >= VALIDPREDICTGCPNUM)
							{
								return true;
							}

						}
					}
					return false;
				}
				
				//页卡控制   idx需要显示的页卡
				void BlockManager::ChangeTab(bool bAT, bool HasImages, bool HasGCPs, std::vector<int> &idx)
				{
					tabstatus[0] = std::make_pair(false, false);
					tabstatus[1] = std::make_pair(false, false);
					tabstatus[2] = std::make_pair(false, false);
					tabstatus[3] = std::make_pair(false, true);
					
					int currentid = 3;

					//  tabstatus[3] = std::make_pair(false, false);
					if (HasGCPs)
					{
						bool bgcp_insert = tabstatus[2].second;
						tabstatus[2] = std::make_pair(false, true);
						currentid = 2;
					}

					if (HasImages)
					{
						tabstatus[1] = std::make_pair(false, true);
						currentid = 1;
					}

					if (bAT)
					{
						tabstatus[1] = std::make_pair(false, true);
						tabstatus[0] = std::make_pair(false, true);
						currentid = 0;
					}
				
					idx.push_back(currentid);
				
					for (auto& it : tabstatus)
					{
						if (it.second.second || it.second.first)
						{
							idx.push_back(it.first);						
							it.second.first = true;
						}

					}				
				}
				//job目录下的文件，只需要写jobinfo;

				bool BlockManager::WriteJob()
				{

					return true;
				}
				bool BlockManager::WriteTaskDef()
				{
					return true;
				}
        
    }//GUI
}  //AI3D


