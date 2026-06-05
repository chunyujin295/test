// Copyright Airlook, Inc. All Rights Reserved.
#pragma once

#include "OSGEditor/Base.h"
#include "OSGEditor/CustomNode.h"
#include "OSGEditor/OsgEngine.h"

class OsgEngine;

class OSGEDITOR_INTERNAL_CLASS PointNode : public CustomNode
{
public:
    PointNode(OsgEngine* pOsgEngine = nullptr) :CustomNode() { m_pOsgEngine = pOsgEngine;  }
    PointNode(const ST_TIEPOINT& tiepoint,OsgEngine* pOsgEngine = nullptr);



    virtual ~PointNode() {};

    virtual void Init();
    //点云颜色重置
    virtual void Reset();
    //选中相片时联动接口
  //  virtual void Picked();
    //通过影像拾取改变对应点颜色
    void Picked(const uint32_t& photoID);
    void Picked(const std::vector<int>& photos);
   
    //删除对应影像的点
    void Delete(const int& photoID = -1);
    void Delete(const std::vector<int> &photoIDs);
    //点击拾取
    bool Picked(osg::ref_ptr<osgViewer::Viewer> pViewer, const float& x, const float& y);
    //鼠标hover
    bool Hover(osg::ref_ptr<osgViewer::Viewer> pViewer, const float& x, const float& y, bool status = false);

    //矩形框选
    bool BoxSelect(osg::ref_ptr<osgViewer::Viewer> pViewer, const osg::Vec3& minXY, const osg::Vec3& maxXY);
    //点云缩放接口
    void ScaleChild(float value);
    //多边形框选
    bool PolygonSelect(osg::ref_ptr<osgViewer::Viewer> pViewer, const std::vector<osg::Vec3>& vecPoints);
    std::vector<int> m_vecPhotoID;

    void GetSelectedPointID(std::vector<int> &pointIDs);
    void UpdateGeometry();
private:


private:
    OsgEngine* m_pOsgEngine;

    ST_TIEPOINT m_stTiePoint;

    osg::ref_ptr<osg::Geometry> m_pGeometry;

    osg::ref_ptr<osg::Vec3Array> m_pPoints;

    osg::ref_ptr<osg::PositionAttitudeTransform> m_pHoveNode;    //点hover节点
    osg::BoundingBox m_stBox;                                  //点云包围盒
    std::vector<osg::Vec3> m_vecPoint;                        //拾取到的点为止
    std::vector<osg::ref_ptr<osg::Node>> m_vecTiePointLineNode; //点到相片的连接线    
    std::vector<int> m_vecPointIndex;                           //存放选中点的索引
    std::map<int, bool> m_mapTmpPointID;
   
};