#pragma once
#include "OSGEditor/Base.h"
#include "OSGEditor/OsgEngine.h"

class OsgEngine;

//class DLL_API CoordinateAxis : public osg::Camera
class CoordinateAxis : public osg::Camera
{
public:
    CoordinateAxis(OsgEngine* pOsgEngine = nullptr);
    ~CoordinateAxis();

    virtual void traverse(osg::NodeVisitor& nv);
    void setWidthHeight(int x, int y, int width, int height) { m_xx = x; m_yy = y; m_width = width; m_height = height; };
    void setMainCamera(osg::Camera* camera) { _mainCamera = camera; }

    void Init();

    void addAxis(Geode* coord, Vec3 pt, std::string text, Vec4 color);
private:
    OsgEngine* m_pOsgEngine;
    osg::ref_ptr<osg::Geode> axisGeode;

    int m_width, m_height;
    int m_x, m_y, m_xx, m_yy;
    osg::observer_ptr<osg::Camera> _mainCamera;
    osg::Matrix m_stMatrix;
};



