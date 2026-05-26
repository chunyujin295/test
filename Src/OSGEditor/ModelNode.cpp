
#include "OSGEditor/ModelNode.h"
#include "OSGEditor/FileLibrary.h"
#include "OSGEditor/EventManager.h"



ModelNode::ModelNode(const int& id, const std::string& name, std::string& filePath, OsgEngine* pOsgEngine) :CustomNode(id, name, Element_Type::ELEMENT_MODEL)
{
    m_strModelPath = filePath;    
    m_pOsgEngine = pOsgEngine;
}

void ModelNode::Init()
{
    
    std::list<string> fileList;
    int nums = -1;
    osg::ref_ptr<osg::Group> pGroup = new osg::Group;
    ifstream fin(m_strModelPath);
    if (fin)
    {
        //加载合并节点数据
        fin.close();
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

    while (m_pOsgEngine->GetViewer()->getDatabasePager()->GetActivePagedLOD() != nums)
    {
        nums = m_pOsgEngine->GetViewer()->getDatabasePager()->GetActivePagedLOD();
        Sleep(1000);
    }
    //根节点加载完成回调通知
    ST_CALLBACK_ELEMENT_INFO CallbackInfo;
    CallbackInfo.ID = m_iID;
    std::vector<ST_CALLBACK_ELEMENT_INFO> vecCallback;
    vecCallback.push_back(CallbackInfo);
    EventManager::GetInstance()->notifyEvent({ CALL_BACK_OSGB_LOADED, &vecCallback });

      
}

void ModelNode::Reset()
{

}

void ModelNode::Picked(const SELECT_TYPE& type)
{

}