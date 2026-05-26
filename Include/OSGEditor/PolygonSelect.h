// Copyright Airlook, Inc. All Rights Reserved.
#pragma once

#include "OSGEditor/Unitl.h"
#include "OSGEditor/OsgEngine.h"
class OsgEngine;
class PolygonSelect : public osg::Geode
{
public:
    PolygonSelect(OsgEngine* pOsgEngine);
    PolygonSelect() { m_pOsgEngine = nullptr;  };//@add by  
    ~PolygonSelect() {};

    void Init();
    void UpdateSelect(const osg::Vec3& pixel);

    void Clear();
    void UpdateMove(const osg::Vec3& pixel);
    void Cancel(); //取消
    void SetStatus(const bool& status);

    const  std::vector<osg::Vec3>& GetPolygonPoints() { return m_vecPoints; };
private:
    void CreatePoint();
    void CreateLine();
    void CreatePolygon();
private:
    OsgEngine* m_pOsgEngine;
    osg::ref_ptr<Geometry> m_pGeometry;
    osg::ref_ptr<Geometry> m_pLineGeometry;
    osg::ref_ptr<Geometry> m_pPointsGeometry;
    osg::ref_ptr<Geometry> m_pMovePointsGeometry;
    std::vector<osg::Vec3> m_vecPoints;
    int m_iNum = 0;
    //bool m_bStatus = false;
};