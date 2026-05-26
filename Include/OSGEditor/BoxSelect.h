// Copyright Airlook, Inc. All Rights Reserved.
#pragma once

#include "Unitl.h"
#include "OsgEngine.h"
class OsgEngine;
class BoxSelect : public osg::Geode
{
public:
    BoxSelect(OsgEngine* pOsgEngine);
    BoxSelect() { m_pOsgEngine = nullptr; };
    ~BoxSelect() {};
    void Init();
    //框选更新位置，每次鼠标拖拽调用
    void Update(const Vec2& start, const Vec2& end);
    //框选结束接口，计算框选目标
    void End(osg::Camera* pCamera, const Element_Type& type);


private:
    OsgEngine* m_pOsgEngine;
    osg::ref_ptr<osg::Vec3Array> m_pVertexArray;
    osg::ref_ptr<osg::Geometry> m_pGeometry;
    osg::ref_ptr<osg::Geometry> m_pLineGeometry;
    std::vector<osg::ref_ptr<CustomNode>>* m_pSelectedNode;
    osg::Vec2 m_vecFristPoint;
    osg::Vec2 m_vecLastPoint;
};