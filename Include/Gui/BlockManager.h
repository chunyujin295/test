/**
  * @file      Application.h
  * @brief    block的AT、Photo、GCP、3DVIEW界面类
  * @details
  * @author    
  * @attention
  */
#ifndef _AI3D_CORE_BLOCKMANAGER_H_
#define _AI3D_CORE_BLOCKMANAGER_H_

#include <vector>

#include <Eigen/Core>

#include "Constants.h"

#include "Core/BlockObject.h"
#include "Gui/GlobalStruct.h"





namespace AI3D
{
    namespace GUI
    {

        class GUI_API BlockManager
        {
			
			 public:
				 BlockManager(AI3D::CORE::BlockObject* block);
			   //用于提交空三前，空三按钮的状态设置
				Block_Status_s& GetBlockStatusMutual();
				

				//用于提交空三点击后 生成相关文件前的判断
				bool CanSubmitAT(AI3D::CORE::ATOptions option);		
				bool  CanSubmitRecon();
				block_t GetBlockId() { return block_->GetId(); };
				AI3D::CORE::BlockObject* GetBlockMutual();
				//页卡控制
				void ChangeTab(bool bAT, bool HasImages, bool HasGCPs, std::vector<int>& idx);
				bool WriteJob();
				bool WriteTaskDef();
				void SetJobType(jobtype_e type)
				{
					job_type_ = type;
				};
           private:
			   bool CanSubmitAT();
			   bool CanReSubmitAT();
			   bool CanCancleAT();
			   bool CanAddPoses();
			   bool CanDeletePoses();
			   bool CanAddPhotos();
			   bool CanDeletePhotos();
			   bool CanAddGcps();
			   bool CanDeleteGcps();

			   
			 

			   AI3D::CORE::BlockObject* block_;
			   std::shared_ptr<AI3D::CORE::ATData> atdata_;
			   Block_Status_s block_wgt_status_;
			   std::map<int, std::pair<bool, bool> > tabstatus;
			   jobtype_e job_type_ = jobtype_e::JOB_AT;
        };
    }//GUI
}  //AI3D

#endif  // _AI3D_CORE_APPLICATION_H_
