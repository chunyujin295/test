/**
  * @file      Application.h
  * @brief     project状态管理类
  * @details
  * @author    
  * @attention
  */
#ifndef _AI3D_CORE_PROJECTMANAGER_H_
#define _AI3D_CORE_PROJECTMANAGER_H_

#include <vector>
#include <list>
#include <Eigen/Core>

#include "Constants.h"
#include "Core/BlockObject.h"
#include "Gui/BlockManager.h"
#include "Core/ProjectObject.h"
#include "Core/Types.h"

namespace AI3D
{
    namespace GUI
    {
        class GUI_API ProjectManager
        {
            
            public:
                static ProjectManager* GetInstance();
                std::shared_ptr<AI3D::CORE::ProjectObject> GetProject() { return project_; };
                //是否能够保存
                void SetProejctModified(bool issave);
                bool GetProjectModified() { return iscansave_; };
                void SaveProject();
                void InitBlockManager();
                void AddBlockManager(AI3D::CORE::BlockObject *block);
                BlockManager* GetBlockManaget(block_t id);
                void DeleteBlockManager(block_t id);
                
                std::map<int, BlockManager*>& GetBlockManagers() 
                { return blockmanagers_; };
            private:
                ProjectManager();
                ~ProjectManager();
                ProjectManager(const ProjectManager& instance);
                const ProjectManager& operator = (const ProjectManager* instance);
                bool iscansave_;
                std::shared_ptr<AI3D::CORE::ProjectObject> project_;
                std::map<int, BlockManager*> blockmanagers_;//管理按钮的状态
                static ProjectManager* instance_;
                static std::once_flag oc_;

               
        };
    }//GUI
}  //AI3D

#endif  // _AI3D_CORE_APPLICATION_H_
