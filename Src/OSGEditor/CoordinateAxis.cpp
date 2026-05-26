#include "OSGEditor/CoordinateAxis.h"
#include "OSGEditor/EventManager.h"
CoordinateAxis::CoordinateAxis(OsgEngine* pOsgEngine)
{
    m_pOsgEngine = pOsgEngine;
    Init();
}

CoordinateAxis::~CoordinateAxis()
{

}

void CoordinateAxis::Init()
{
    Geode* coord = new Geode;
    coord->getOrCreateStateSet()->setMode(GL_LIGHTING, StateAttribute::OFF);
    coord->getOrCreateStateSet()->setAttribute(new LineWidth(2), StateAttribute::ON); 

    float len = 1.;

    addAxis(coord, Vec3(len, 0., 0.), "X", Vec4(1., 0., 0., 1.)); // x-axis
    addAxis(coord, Vec3(0., len, 0.), "Y", Vec4(0., 1., 0., 1.)); // y-axis
    addAxis(coord, Vec3(0., 0., len), "Z", Vec4(0., 0., 1., 1.)); // z-axis

    addChild(coord);

    setProjectionMatrix(osg::Matrixd::ortho(-1., 1., -1., 1., -1.0, 1.0));
    setRenderOrder(osg::Camera::POST_RENDER);
    setClearMask(GL_DEPTH_BUFFER_BIT);
    setAllowEventFocus(false);
    setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
}



// 创建单个坐标分量
void CoordinateAxis::addAxis(Geode* coord, Vec3 pt, std::string text, Vec4 color)
{


    Vec3Array* v = new Vec3Array();
    v->push_back(Vec3(0,0,0));
    v->push_back(pt);

    Vec4Array* c = new Vec4Array();
    c->push_back(color);

    Geometry* axis = new Geometry;
    axis->setVertexArray(v);
    axis->setColorArray(c);
    axis->setColorBinding(Geometry::BIND_OVERALL);
    axis->addPrimitiveSet(new DrawArrays(PrimitiveSet::LINES, 0, 2));

    osgText::Text* tx = new osgText::Text;
    tx->setText(text);
    tx->setFont("Fonts/simhei.ttf");
    tx->setAxisAlignment(osgText::Text::SCREEN);
    tx->setCharacterSize(0.2);
    tx->setPosition(pt);

    osg::ref_ptr<osg::Material> material = new osg::Material();
    material->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE);
    material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    tx->getOrCreateStateSet()->setAttributeAndModes(material.get());

    coord->addDrawable(tx);
    coord->addDrawable(axis);

}

void CoordinateAxis::traverse(osg::NodeVisitor& nv)
{
    osg::Camera::traverse(nv);
    if (_mainCamera.valid() && nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
    {
        osg::Matrix matrix = _mainCamera->getViewMatrix();
        matrix.setTrans(osg::Vec3(-0.5, -0.5, 0));

        try
        {
            if (_mainCamera->getViewport())
            {
                if (m_x != _mainCamera->getViewport()->width() || _mainCamera->getViewport()->height() != m_y)
                {
                    m_x = _mainCamera->getViewport()->width();
                    m_y = _mainCamera->getViewport()->height();
                    this->setViewport(m_xx, m_yy, m_width, m_height);
                }
            }
        }
        catch (std::exception& ex)
        {
            std::cout << " ex:" << ex.what() << std::endl;

        }

        this->setViewMatrix(matrix);
        if (m_stMatrix != matrix)
        {
            m_stMatrix = matrix;

            //回调联动信息
            EventManager::GetInstance()->notifyEvent({ CALL_BACK_CAMERA, &m_stMatrix },m_pOsgEngine);
        }
    }

    //osg::Camera::traverse(nv);
}