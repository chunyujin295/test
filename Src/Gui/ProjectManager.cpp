#include "Gui/ProjectManager.h"
namespace AI3D
{
	namespace GUI
	{


		ProjectManager* ProjectManager::instance_ = nullptr;
		std::once_flag ProjectManager::oc_;
		ProjectManager* ProjectManager::GetInstance()
		{
			std::call_once(oc_, []() {instance_ = new ProjectManager(); });
			return instance_;
		}

		ProjectManager::ProjectManager(/*std::shared_ptr<AI3D::CORE::BlockObject> block*/)
		{
			project_ = std::make_shared <AI3D::CORE::ProjectObject> ();
			iscansave_ = false;
		}
		ProjectManager::~ProjectManager(/*std::shared_ptr<AI3D::CORE::BlockObject> block*/)
		{

		}
		
		void ProjectManager::SetProejctModified(bool issave)
		{
			
				iscansave_ = issave;
		}
		void ProjectManager::SaveProject()
		{
			//±£´ætriÎÄ¼þ
			if (iscansave_)
			{
				if (PROJECT_USE_BIN) {
					project_->SaveBin(project_->GetFullName(), savetype_e::XML_SAVED);
				}
				else {
					project_->SaveJson(project_->GetFullName(), savetype_e::XML_SAVED);
				}
			}
				

		}

		void ProjectManager::InitBlockManager()
		{

			blockmanagers_.clear();
			
			

		}

		void ProjectManager::AddBlockManager(AI3D::CORE::BlockObject* block)
		{
			auto it = blockmanagers_.find(block->GetId());
			if (it != blockmanagers_.end())
			{
				BlockManager* manager = new BlockManager(block);
				blockmanagers_[block->GetId()] = manager;
			}
			else {
				BlockManager* manager = new BlockManager(block);
				blockmanagers_.insert(std::pair<int, BlockManager*>(block->GetId(), manager));
			}
			
		}

		
		BlockManager* ProjectManager::GetBlockManaget(block_t id)
		{
			
			return blockmanagers_[id];
			
			
		}
		
		void ProjectManager::DeleteBlockManager(block_t id)
		{
			auto it = blockmanagers_.find(id);
			if (it != blockmanagers_.end()) 
			{
				blockmanagers_.erase(it);
			}
		}
		
	}
}