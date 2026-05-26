
#include "OSGEditor/TileNode.h"

TileNode::TileNode(const ST_BOUNDINGBOX& box) :m_stBoxInfo(box),
CustomNode(box.ID, box.name, Element_Type::ELEMENT_TILE)
{
    setCullingActive(false);
    setDataVariance(osg::Object::DYNAMIC);
}

TileNode::~TileNode()
{
   if (m_pBoxGroup)
   {
       m_pBoxGroup->removeChildren(0, m_pBoxGroup->getNumChildren());

       removeChild(m_pBoxGroup);
       m_pBoxGroup = nullptr;
   }
   if (m_pTileBorderGeometry)
   {
       removeChild(m_pTileBorderGeometry);
       m_pTileBorderGeometry = nullptr;
   }
}

void TileNode::Reset()
{
    if (m_eMouseType == MOUSE_TYPE::MOUSE_NONE)
    {
        return;
    }

    UpdateBorder(Unitl::FromHex(EditerEngine::TileDefaultColor[0]));
    UpdatePlane(Unitl::FromHex(EditerEngine::TileDefaultColor[1]));
    m_eMouseType = MOUSE_TYPE::MOUSE_NONE;
}


void TileNode::Hover()
{
    if (m_eMouseType != MOUSE_TYPE::MOUSE_NONE)
    {
        return;
    }

    UpdateBorder(Unitl::FromHex(EditerEngine::TileHoverColor[0]));
    m_eMouseType = MOUSE_TYPE::MOUSE_HOVER;

}
void TileNode::Picked(const SELECT_TYPE& type)
{
    if (m_eMouseType == MOUSE_TYPE::MOUSE_PICKED)
    {
        return;
    }

    UpdateBorder(Unitl::FromHex(EditerEngine::TileSelectColor[0]));
    UpdatePlane(Unitl::FromHex(EditerEngine::TileSelectColor[1]));


    m_eMouseType = MOUSE_TYPE::MOUSE_PICKED;
}

void TileNode::Init()
{
    osg::BoundingBox bbx(m_stBoxInfo.minXYZ, m_stBoxInfo.maxXYZ);
    if (!bbx.valid())
        return;

    m_pBoxGroup = Unitl::CreateBox(&bbx, Unitl::FromHex(EditerEngine::TileDefaultColor[1]));

    if (m_pBoxGroup == nullptr)
    {
        return;
    }
    m_pTileBorderGeometry = Unitl::CreateBoxBorderTile(&bbx, Unitl::FromHex(EditerEngine::TileDefaultColor[0]));

    getBound();       

    addChild(m_pTileBorderGeometry.get());
    addChild(m_pBoxGroup.get());
    //
    setCullingActive(true);
}


void TileNode::UpdateBorder(const osg::Vec4& color)
{
    if (m_pTileBorderGeometry == nullptr)
    {
        return;
    }
    osg::Vec4Array* pColor = dynamic_cast<osg::Vec4Array*>(m_pTileBorderGeometry->asGeode()->getDrawable(0)->asGeometry()->getColorArray());
    if (!pColor)
    {
        return;
    }
    for (int i = 0; i < pColor->size(); i++)
    {
        (*pColor)[i] = color;
    }

    m_pTileBorderGeometry->asGeode()->getDrawable(0)->asGeometry()->dirtyGLObjects();
}

void TileNode::UpdatePlane(const osg::Vec4& color)
{
    if (m_pBoxGroup == nullptr)
    {
        return;
    }
    //更新选中面的颜色
    for (unsigned i = 0; i < m_pBoxGroup->getNumChildren(); i++)
    {
        osg::Geometry* pGeometry = m_pBoxGroup->getChild(i)->asGeode()->getDrawable(0)->asGeometry();
        if (pGeometry/* && pGeometry == this->m_pSelectGeometry*/)
        {
            osg::Vec4Array* pColor = dynamic_cast<osg::Vec4Array*>(pGeometry->getColorArray());
            if (!pColor)
            {
                return;
            }
            for (int i = 0; i < pColor->size(); i++)
            {
                (*pColor)[i] = color;
            }

            pGeometry->dirtyGLObjects();
        }
    }

}
