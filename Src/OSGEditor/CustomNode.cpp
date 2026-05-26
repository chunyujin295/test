#include "OSGEditor/CustomNode.h"
#include "OSGEditor/Unitl.h"
#include "OSGEditor/ROINode.h"
#include "OSGEditor/PointNode.h"
//#include "OSGEditor/PhotosNode.h"

CustomNode::CustomNode(const bool& bThread) :m_iID(-1), m_strName(""), m_iElementType(Element_Type::ELEMENT_NONE)
{
    m_bThread = bThread;

    m_eMouseType = MOUSE_TYPE::MOUSE_NONE;
    m_pSelectGeometry = nullptr;
    setDataVariance(Object::DYNAMIC);

    m_pRootSwitch = new osg::Switch;
    m_pRootSwitch->setDataVariance(Object::DYNAMIC);
    //m_pRootSwitch->setRangeMode(LOD::RangeMode::DISTANCE_FROM_EYE_POINT);
    addChild(m_pRootSwitch);

    if (m_bThread)
    {
        m_pThreadPool = new ThreadManager(10);
    }
};

CustomNode::CustomNode(const int& id, const std::string& name, const Element_Type& type) :
    m_iID(id), m_strName(name), m_iElementType(type)
{
    m_eMouseType = MOUSE_TYPE::MOUSE_NONE;
    m_pSelectGeometry = nullptr;
    setDataVariance(Object::DYNAMIC);
}

CustomNode::~CustomNode()
{

}

void CustomNode::AddChild(const int& id, ref_ptr<CustomNode> node)
{

    if (m_pTotalNode.find(id) == m_pTotalNode.end())
    {
        m_pTotalNode.insert(make_pair(id, node.get()));
        m_pRootSwitch->addChild(node.get());
    }
    else
    {
        RemoveChild(id);

        m_pTotalNode.insert(make_pair(id, node.get()));
        m_pRootSwitch->addChild(node.get());
    }

    if (m_bThread)
    {

        m_pThreadPool->Add(node.get());
    }
    else
    {
        node->Init();
    }
};

ref_ptr<CustomNode> CustomNode::GetChild(const int& id)
{
    if (m_pTotalNode.find(id) != m_pTotalNode.end())
    {
        return m_pTotalNode.find(id)->second;
    }

    return nullptr;
};

void CustomNode::RemoveChild(const int& id)
{
    if (/*id != 0 &&*/ GetChild(id).get())
    {
        m_pTotalNode.erase(id);
        m_pRootSwitch->removeChild(GetChild(id));
    }
}

void CustomNode::RemoveChild(ref_ptr<CustomNode> node)
{
    if (node.get() && GetChild(node->m_iID).get())
    {
        m_pTotalNode.erase(node->m_iID);
        m_pRootSwitch->removeChild(node);
    }
}

void CustomNode::RemoveAll()
{
    for (auto it : m_pTotalNode)
    {
        m_pRootSwitch->removeChild(it.second);
    }
    
    m_pTotalNode.clear();
}

void CustomNode::Visible(bool value)
{
    if (value)
    {
        m_pRootSwitch->setAllChildrenOn();
        setCullingActive(true);
        //m_pRootSwitch->setNodeMask(1);
    }
    else
    {
        m_pRootSwitch->setAllChildrenOff();
        //m_pRootSwitch->setNodeMask(0);
        setCullingActive(false);
    }
}

void CustomNode::Scale(float value)
{
    for (auto it : m_pTotalNode)
    {
        osg::ref_ptr<CustomNode> pCustomNode = it.second;

        //Unitl::Log(pCustomNode->getBound().center());
        if (pCustomNode->GetElementType() == ELEMENT_LAYER_TYPE::ELEMENT_ROI)
        {
            osg::ref_ptr<ROINode> pNode = dynamic_cast<ROINode*>(it.second);
            pNode->ScaleChild(value);
        }
        else if (pCustomNode->GetElementType() == ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS)
        {
            osg::ref_ptr<PointNode> pNode = dynamic_cast<PointNode*>(it.second);
            pNode->ScaleChild(value);
        }
        else if (pCustomNode->GetElementType() == ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS)
        {
          /*  osg::ref_ptr<PhotosNode> pNode = dynamic_cast<PhotosNode*>(it.second);
            pNode->ScaleChild(value);*/
        }
        else
        {
            osg::Matrix mt = pCustomNode->getMatrix();
            pCustomNode->setMatrix(mt * osg::Matrix::translate(-pCustomNode->getBound().center()) * osg::Matrix::scale(value, value, value) * osg::Matrix::translate(pCustomNode->getBound().center()));

        }
    }
}
