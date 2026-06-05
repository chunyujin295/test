
#include "OSGEditor/ROINode.h"
#include "OSGEditor/EventManager.h"


#include "ROINode.h"
#include "EventManager.h"


ROINode::ROINode(const ST_BOUNDINGBOX& box,OsgEngine* pOsgEngine) : CustomNode(box.ID, box.name, Element_Type::ELEMENT_ROI)
{
    setCullingActive(false);
    m_pOsgEngine = pOsgEngine;
    m_bIsPolygonBox = false;
    m_stBoundingBox.set(box.minXYZ, box.maxXYZ);
    m_bROIStatus = pOsgEngine->GetROIStatus();
    setDataVariance(osg::Object::DYNAMIC);
}

ROINode::ROINode(const ST_POLYGON_BOX& box,OsgEngine* pOsgEngine) :m_stPolygonBoxInfo(box),
CustomNode(box.ID, box.name, Element_Type::ELEMENT_ROI)
{
    m_pOsgEngine = pOsgEngine;
    m_bIsPolygonBox = true;

}


ROINode::~ROINode()
{
    Clear();
}


void ROINode::Clear()
{
    if (m_pROIBoxGroup)
    {
        m_pROIBoxGroup->releaseGLObjects();

        m_pROIBoxGroup->removeChildren(0, m_pROIBoxGroup->getNumChildren());
        removeChild(m_pROIBoxGroup.get());
        m_pROIBoxGroup = nullptr;
        
    }

    if (m_pROIBorderGeometry)
    {
        m_pROIBorderGeometry->releaseGLObjects();

        removeChild(m_pROIBorderGeometry);
        m_pROIBorderGeometry = nullptr;
    }
}

void ROINode::Init()
{
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ <<
    //    std::hex << std::showbase << " pointer:" << this << std::dec << std::endl;
    if (!m_bIsPolygonBox)
    {
        Init(m_stBoundingBox);
    }
    else
    {
        InitPolygonBox(m_stPolygonBoxInfo);

    }
    setCullingActive(true);

}

void ROINode::Init(const osg::BoundingBox& bbox)
{

    m_pROIBoxGroup = Unitl::CreateBox(&bbox, Unitl::FromHex(EditerEngine::ROIDefaultColor[1]));
    m_pROIBorderGeometry = Unitl::CreateBoxBorder(&bbox, Unitl::FromHex(EditerEngine::ROIDefaultColor[0]));
    
    //m_pROIBorderGeometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
    //m_pROIBorderGeometry->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    //m_pROIBorderGeometry->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    //m_pROIBorderGeometry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    //m_pROIBorderGeometry->getOrCreateStateSet()->setRenderBinDetails(100, "RenderBin");
    m_pROIBorderGeometry->asGeode()->getDrawable(0)->asGeometry()->getOrCreateStateSet()->setAttributeAndModes(new osg::LineWidth(2.0), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);


    addChild(m_pROIBoxGroup.get());
    addChild(m_pROIBorderGeometry.get());

    m_vecBoxXYZ = (bbox._max - bbox._min) * 0.5;
}

void ROINode::InitPolygonBox(const ST_POLYGON_BOX& box)
{
    m_pROIBoxGroup = new osg::MatrixTransform;

    m_pROIBorderGeometry = Unitl::CreatePolygonBorder(box, Unitl::FromHex(EditerEngine::ROIDefaultColor[0]));

    addChild(m_pROIBorderGeometry);

    osg::ref_ptr<osg::Vec3Array> pPointTop = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec3Array> pPointDown = new osg::Vec3Array;
    m_dPolygonCenter.set(0, 0, 0);

    for (int i = 0; i < box.points.size(); i++)
    {
        pPointTop->push_back(osg::Vec3(box.points[i].x(), box.points[i].y(), box.maxHeight));
        pPointDown->push_back(osg::Vec3(box.points[i].x(), box.points[i].y(), box.minHeight));
        m_dPolygonCenter += box.points[i];
    }
    m_dPolygonCenter /= box.points.size();

    //上
    osg::ref_ptr<osg::Geode> pGeodeTop = Unitl::CreatePolygon(pPointTop, Unitl::FromHex(EditerEngine::ROIDefaultColor[1]), "Z");
    //下
    osg::ref_ptr<osg::Geode> pGeodeDown = Unitl::CreatePolygon(pPointDown, Unitl::FromHex(EditerEngine::ROIDefaultColor[1]), "-Z");
    m_pROIBoxGroup->addChild(pGeodeTop);
    m_pROIBoxGroup->addChild(pGeodeDown);
    addChild(m_pROIBoxGroup);

    m_vecVerticesDistance.clear();
    for (unsigned int i = 0; i < box.points.size(); i++)
    {
        Vec3 xyz = (box.points[i] - m_dPolygonCenter);

        double length = xyz.length();
        xyz.normalize();
        m_vecVerticesDistance.push_back(std::pair<double, osg::Vec3>(length, xyz));


    }
}


void ROINode::Reset()
{
    if (m_eMouseType == MOUSE_TYPE::MOUSE_NONE)
    {
        return;
    }

    UpdateBorder(Unitl::FromHex(EditerEngine::ROIDefaultColor[0]));
    //UpdatePlane(Unitl::FromHex(EditerEngine::ROIDefaultColor[1]));
    for (unsigned int i = 0; i < m_pROIBoxGroup->getNumChildren(); i++)
    {
        osg::Geometry* pGeometry = m_pROIBoxGroup->getChild(i)->asGeode()->getDrawable(0)->asGeometry();
        if (pGeometry)
        {
            osg::Vec4Array* pColor = dynamic_cast<osg::Vec4Array*>(pGeometry->getColorArray());
            if (!pColor)
            {
                return;
            }
            for (unsigned int ii = 0; ii < pColor->size(); ii++)
            {
                (*pColor)[ii] = Unitl::FromHex(EditerEngine::ROIDefaultColor[1]);
            }

            pGeometry->dirtyGLObjects();
        }
    }

    m_eMouseType = MOUSE_TYPE::MOUSE_NONE;

    if (m_pSelectGeometry)
    {
        osg::Vec4Array* pColor = dynamic_cast<osg::Vec4Array*>(m_pSelectGeometry->getColorArray());
        if (!pColor)
        {
            return;
        }
        for (unsigned int ii = 0; ii < pColor->size(); ii++)
        {
            (*pColor)[ii] = Unitl::FromHex(EditerEngine::ROIDefaultColor[1]);
        }

        m_pSelectGeometry->dirtyGLObjects();
    }
    //m_pSelectGeometry = nullptr;
}

void ROINode::SetROIStatus(bool status) 
{ 
    m_bROIStatus = status; 
}

void ROINode::Hover()
{

    if (m_eMouseType != MOUSE_TYPE::MOUSE_NONE || m_bROIStatus)
    {
        return;
    }
   

    UpdateBorder(Unitl::FromHex(EditerEngine::ROIHoverColor[0]));
    UpdatePlane(Unitl::FromHex(EditerEngine::ROIHoverColor[1]));
   
    m_eMouseType = MOUSE_TYPE::MOUSE_HOVER;


}

void ROINode::Picked(const SELECT_TYPE& type)
{
    if (m_eMouseType == MOUSE_TYPE::MOUSE_PICKED)
    {
        return;
    }
    
    UpdateBorder(Unitl::FromHex(EditerEngine::ROIHoverColor[0]));
    UpdatePlane(Unitl::FromHex(EditerEngine::ROIHoverColor[1]));

    m_eMouseType = MOUSE_TYPE::MOUSE_PICKED;
}


void ROINode::UpdateBorder(const osg::Vec4& color)
{
    if (m_pROIBorderGeometry == nullptr || m_pROIBorderGeometry->asGroup()->getNumChildren() == 0)
    {
        return;
    }
    osg::Vec4Array* pColor = dynamic_cast<osg::Vec4Array*>(m_pROIBorderGeometry->asGeode()->getDrawable(0)->asGeometry()->getColorArray());
    if (!pColor)
    {
        return;
    }
    for (int i = 0; i < pColor->size(); i++)
    {
        (*pColor)[i] = color;
    }

    m_pROIBorderGeometry->asGeode()->getDrawable(0)->asGeometry()->dirtyGLObjects();
}

void ROINode::UpdatePlane(const osg::Vec4& color, const std::string& name)
{
    if (m_pROIBoxGroup == nullptr)
    {
        return;
    }
    //更新选中面的颜色
    for (unsigned int i = 0; i < m_pROIBoxGroup->getNumChildren(); i++)
    {
        osg::Geometry* pGeometry = m_pROIBoxGroup->getChild(i)->asGeode()->getDrawable(0)->asGeometry();
        if (name.empty())
        {
            if (pGeometry && pGeometry == this->m_pSelectGeometry)  //点击选中面
            {
                osg::Vec4Array* pColor = dynamic_cast<osg::Vec4Array*>(pGeometry->getColorArray());
                if (!pColor)
                {
                    return;
                }
                for (unsigned int ii = 0; ii < pColor->size(); ii++)
                {
                    (*pColor)[ii] = color;
                }
                pGeometry->dirtyGLObjects();
                break;
            }
            else if (pGeometry && this->m_pSelectGeometry == nullptr) //全选
            {
                osg::Vec4Array* pColor = dynamic_cast<osg::Vec4Array*>(pGeometry->getColorArray());
                if (!pColor)
                {
                    return;
                }
                for (unsigned int ii = 0; ii < pColor->size(); ii++)
                {
                    (*pColor)[ii] = color;
                }
                pGeometry->dirtyGLObjects();
            }
        }
        else
        {
            if (pGeometry && pGeometry->getName() == name)
            {
                osg::Vec4Array* pColor = dynamic_cast<osg::Vec4Array*>(pGeometry->getColorArray());
                if (!pColor)
                {
                    return;
                }
                for (unsigned int ii = 0; ii < pColor->size(); ii++)
                {
                    (*pColor)[ii] = color;
                }
                pGeometry->dirtyGLObjects();
                break;
            }
        }
    }

}



void ROINode::Drag(const Vec3& dragPoint)
{
    std::cout << "start drag =============" << std::endl;
    if (m_pSelectGeometry == nullptr || m_pROIBorderGeometry == nullptr)
    {
        return;
    }

    Clear();

    std::string axis = m_pSelectGeometry->getName();

    if (m_bIsPolygonBox)
    {
        if (axis == "Z")
        {
            m_stPolygonBoxInfo.maxHeight = dragPoint.z() > m_stPolygonBoxInfo.minHeight ? dragPoint.z() : m_stPolygonBoxInfo.minHeight;
        }
        else if (axis == "-Z")
        {
            m_stPolygonBoxInfo.minHeight = dragPoint.z() < m_stPolygonBoxInfo.maxHeight ? dragPoint.z() : m_stPolygonBoxInfo.maxHeight;
        }

        InitPolygonBox(m_stPolygonBoxInfo);

        std::vector<PolygonBox> vecCallback;
        vecCallback.push_back(m_stPolygonBoxInfo);
        //回调联动信息 EventManager::GetInstance()->notifyEvent({ CALL_BACK_ROI_POLYGON_DRAG, &vecCallback },m_pOsgEngine);
    }
    else
    {

        if (axis == "X")
        {
            m_stBoundingBox.xMax() = dragPoint.x() > m_stBoundingBox.xMin() ? dragPoint.x() : m_stBoundingBox.xMax();
        }
        else if (axis == "-X")
        {
            m_stBoundingBox.xMin() = dragPoint.x() < m_stBoundingBox.xMax() ? dragPoint.x() : m_stBoundingBox.xMin();
        }
        else if (axis == "Y")
        {
            m_stBoundingBox.yMax() = dragPoint.y() > m_stBoundingBox.yMin() ? dragPoint.y() : m_stBoundingBox.yMax();
        }
        else if (axis == "-Y")
        {
            m_stBoundingBox.yMin() = dragPoint.y() < m_stBoundingBox.yMax() ? dragPoint.y() : m_stBoundingBox.yMin();
        }
        else if (axis == "Z")
        {
            m_stBoundingBox.zMax() = dragPoint.z() > m_stBoundingBox.zMin() ? dragPoint.z() : m_stBoundingBox.zMax();
        }
        else if (axis == "-Z")
        {
            m_stBoundingBox.zMin() = dragPoint.z() < m_stBoundingBox.zMax() ? dragPoint.z() : m_stBoundingBox.zMin();
        }

        Init(m_stBoundingBox);
    }

    UpdatePlane(Unitl::FromHex(EditerEngine::ROIHoverColor[1]), axis);

}

void ROINode::DragEnd()
{
    //边框信息
    std::cout << " dara gend =================" << std::endl;
    ST_CALLBACK_ELEMENT_INFO CallbackInfo;
    CallbackInfo.ID = m_iID;
    CallbackInfo.name = m_strName;
    CallbackInfo.bbox = m_stBoundingBox;

    std::vector<ST_CALLBACK_ELEMENT_INFO> vecCallback;
    vecCallback.push_back(CallbackInfo);

    //回调联动信息 EventManager::GetInstance()->notifyEvent({ CALL_BACK_ROI_BOX_DRAG, &vecCallback },m_pOsgEngine);
    std::cout << " drag  end notify=============" << std::endl;

    //Reset();
}

void ROINode::ScaleChild(float value)
{
    Clear();
    if (!m_bIsPolygonBox)
    {
        m_vecBoxXYZ *= value;
        m_stBoundingBox._max = m_stBoundingBox.center() + m_vecBoxXYZ;
        m_stBoundingBox._min = m_stBoundingBox.center() - m_vecBoxXYZ;
        Init(m_stBoundingBox);

    }
    else
    {
        m_stPolygonBoxInfo.points.clear();
        for (unsigned int i = 0; i < m_vecVerticesDistance.size(); i++)
        {
            double distance = m_vecVerticesDistance.at(i).first;
            osg::Vec3f dir = m_vecVerticesDistance.at(i).second;

            osg::Vec3f xyz = dir * (distance * value);
            xyz += m_dPolygonCenter;

            m_stPolygonBoxInfo.points.push_back(xyz);
        }
        float diffValue = (m_stPolygonBoxInfo.maxHeight - m_stPolygonBoxInfo.minHeight);
        float offset = diffValue * value - diffValue;

        m_stPolygonBoxInfo.maxHeight += offset;
        m_stPolygonBoxInfo.minHeight -= offset;

        InitPolygonBox(m_stPolygonBoxInfo);

    }

}
