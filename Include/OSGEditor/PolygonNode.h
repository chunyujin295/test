#pragma once
#include "Unitl.h"

class PolygonNode : public CustomNode
{
public:
    PolygonNode(const int &id, const std::string &name, const osg::ref_ptr<osg::Vec3Array> pPoints);
    PolygonNode():CustomNode(false){};
    ~PolygonNode() {};

    virtual void Init();
    //恢复默认颜色
    virtual void Reset() ;
    //修改hover颜色     
    virtual void Hover() ;

private:
  
    void CreateLine();
    void CreatePolygon();
private:
    osg::ref_ptr<Geometry> m_pGeometry;
    osg::ref_ptr<Geometry> m_pLineGeometry;
    osg::ref_ptr<Geometry> m_pPointsGeometry;
    osg::ref_ptr<osg::Vec3Array> m_pPoints;
};