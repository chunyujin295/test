
#include "OSGEditor/ModelNode.h"
#include "OSGEditor/FileLibrary.h"
#include "OSGEditor/EventManager.h"
#include "Core/File.h"
#include <osgDB/DatabasePager>

namespace {

/** OSG 3.6.5: no DatabasePager::GetActivePagedLOD (3.7.x custom); use pending queue depth. */
int DatabasePagerPendingWork(osgDB::DatabasePager* pager)
{
	if (!pager) {
		return 0;
	}
	return static_cast<int>(
		pager->getFileRequestListSize()
		+ pager->getDataToCompileListSize()
		+ pager->getDataToMergeListSize()
		+ (pager->getRequestsInProgress() ? 1 : 0));
}

void WaitForDatabasePagerSettled(osgDB::DatabasePager* pager)
{
	int nums = -1;
	while (DatabasePagerPendingWork(pager) != nums)
	{
		nums = DatabasePagerPendingWork(pager);
		Sleep(1000);
	}
}

} // namespace

ModelNode::ModelNode(const int& id, const std::string& name, std::string& filePath, OsgEngine* pOsgEngine) :CustomNode(id, name, Element_Type::ELEMENT_MODEL)
{
    m_strModelPath = filePath;    
    m_pOsgEngine = pOsgEngine;
}

void ModelNode::Init()
{
    
    std::list<string> fileList;
    osg::ref_ptr<osg::Group> pGroup = new osg::Group;
    if (AI3D::CORE::File::ExistsPath(m_strModelPath))
    {
        //加载合并节点数据
        osg::Node *pNode = osgDB::readNodeFile(m_strModelPath);
        addChild(pNode);
    }
    else
    {
        //加载非合并
        FileLibrary::getInstance()->getAllSubFiles(m_strModelPath, fileList, true, false, false, ""); //获取目录
        for (auto it : fileList)
        {
            std::list<string> fileList2;
                
            FileLibrary::getInstance()->getAllSubFiles(it, fileList2, false, true, false, "osgb"); //遍历每个目录下的根节点
            if (fileList2.size() == 0)
            {
                continue;
            }
            std::string filepath = *fileList2.begin();
            osg::Node* pNode = osgDB::readNodeFile(filepath);
            pGroup->addChild(pNode);
        }

        addChild(pGroup);
    }

    WaitForDatabasePagerSettled(m_pOsgEngine->GetViewer()->getDatabasePager());
    //根节点加载完成回调通知
    ST_CALLBACK_ELEMENT_INFO CallbackInfo;
    CallbackInfo.ID = m_iID;
    std::vector<ST_CALLBACK_ELEMENT_INFO> vecCallback;
    vecCallback.push_back(CallbackInfo); EventManager::GetInstance()->notifyEvent({ CALL_BACK_OSGB_LOADED, &vecCallback });

      
}

void ModelNode::Reset()
{

}

void ModelNode::Picked(const SELECT_TYPE& type)
{

}
