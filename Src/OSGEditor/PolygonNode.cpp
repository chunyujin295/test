
#include "OSGEditor/PolygonNode.h"
#include "osg/LineStipple"
#include <osgUtil/Tessellator>
PolygonNode::PolygonNode(const int& id, const std::string& name, const osg::ref_ptr<osg::Vec3Array> pPoints):CustomNode(id,name,ELEMENT_LAYER_TYPE::ELEMENT_POLYGON)
{
    m_pPoints = move(pPoints);
    //Init();
    ///Init();
}

void PolygonNode::Init()
{
    CreateLine();
    CreatePolygon();
}

void PolygonNode::CreateLine()
{

    m_pLineGeometry = new osg::Geometry;

    osg::ref_ptr<osg::DrawElementsUInt> drawIndexs = new osg::DrawElementsUInt(osg::PrimitiveSet::LINE_LOOP, 0);
    osg::ref_ptr<osg::Vec3Array> pPoint = new osg::Vec3Array;
    for (int i = 0; i < m_pPoints->size(); i++)
    {
        drawIndexs->push_back(i);
    }

    m_pLineGeometry->setVertexArray(m_pPoints);
    osg::Vec4 color = Unitl::FromHex(EditerEngine::PolygonWaterDefaultColor[0]);
    osg::ref_ptr<osg::Vec4Array> vc = new osg::Vec4Array;
    vc->push_back(color);
    m_pLineGeometry->setColorArray(vc);
    m_pLineGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    m_pLineGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::Mode::LINE_LOOP, 0, m_pPoints->size()));
    m_pLineGeometry->getOrCreateStateSet()->setAttributeAndModes(new osg::LineWidth, osg::StateAttribute::ON);
    m_pLineGeometry->getOrCreateStateSet()->setAttributeAndModes(new osg::LineStipple(1, 0x00FF));
    m_pLineGeometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
    pGeode->addDrawable(m_pLineGeometry);

    addChild(pGeode);
}

void PolygonNode::CreatePolygon()
{
    m_pGeometry = new osg::Geometry;

    osg::ref_ptr<osg::DrawElementsUInt> drawIndexs = new osg::DrawElementsUInt(osg::PrimitiveSet::POLYGON, 0);
    osg::ref_ptr<osg::Vec3Array> pPoint = new osg::Vec3Array;
    for (int i = 0; i < m_pPoints->size(); i++)
    {
        drawIndexs->push_back(i);
    }

    m_pGeometry->setVertexArray(m_pPoints);

    osg::Vec4 PolygonColor = Unitl::FromHex(EditerEngine::PolygonWaterDefaultColor[1]);
    osg::ref_ptr<osg::Vec4Array> vc = new osg::Vec4Array;
    vc->push_back(PolygonColor);
    m_pGeometry->setColorArray(vc);
    m_pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    m_pGeometry->addPrimitiveSet(drawIndexs);
    m_pGeometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    m_pGeometry->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    m_pGeometry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    //解决凹凸多边形问题
    osg::ref_ptr<osgUtil::Tessellator> tscx = new osgUtil::Tessellator;
    tscx->setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
    tscx->setBoundaryOnly(false);
    tscx->setWindingType(osgUtil::Tessellator::TESS_WINDING_ODD);
    tscx->retessellatePolygons(*(m_pGeometry.get()));
    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
    pGeode->addDrawable(m_pGeometry);

    addChild(pGeode);        
}

void PolygonNode::Reset()
{
    if (m_eMouseType == MOUSE_TYPE::MOUSE_NONE)
    {
        return;
    }
    m_eMouseType = MOUSE_TYPE::MOUSE_NONE;
}

void PolygonNode::Hover()
{
    if (m_eMouseType != MOUSE_TYPE::MOUSE_NONE)
    {
        return;
    }

    m_eMouseType = MOUSE_TYPE::MOUSE_HOVER;
}


