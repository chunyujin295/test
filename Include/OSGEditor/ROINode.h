
#pragma once
#include "Unitl.h"
#include "OsgEngine.h"

class OsgEngine;

class ROINode : public CustomNode
{
public:
    ROINode(const ST_BOUNDINGBOX& box, OsgEngine* pOsgEngine = nullptr);
    ROINode(const ST_POLYGON_BOX& box, OsgEngine* pOsgEngine = nullptr);
    ROINode(OsgEngine* pOsgEngine = nullptr) :CustomNode(false) { m_pOsgEngine = pOsgEngine; }
    ~ROINode() ;

    virtual void Init();
    //恢复默认颜色
    virtual void Reset();
    //修改hover颜色
    virtual void Hover();
    //修改选中颜色
    virtual void Picked(const SELECT_TYPE& type = SELECT_TYPE::SELECT_ONE);
    //拖拽结束
    void DragEnd();
    void Drag(const Vec3& dragPoint);
    void UpdatePlane(const osg::Vec4& color, const std::string& name = "");

    bool IsPolygonBox() { return m_bIsPolygonBox; };

    void ScaleChild(float value);
    void SetROIStatus(bool status);
private:
    void Clear();

    void Init(const osg::BoundingBox& box);

    void UpdateBorder(const osg::Vec4& color);




    void InitPolygonBox(const ST_POLYGON_BOX& box);

private:
    OsgEngine* m_pOsgEngine;
    //边框
    osg::ref_ptr<osg::Node> m_pROIBorderGeometry;
    //面
    osg::ref_ptr<osg::Group> m_pROIBoxGroup;

    ST_POLYGON_BOX m_stPolygonBoxInfo;
    osg::BoundingBox m_stBoundingBox;
    bool m_bIsPolygonBox;
    osg::Vec3 m_vecBoxXYZ;

    std::vector<std::pair<double, osg::Vec3>> m_vecVerticesDistance;
    osg::Vec3 m_dPolygonCenter;
    //兴趣框开关
    bool m_bROIStatus = false;
    Unitl g_roiUnitl;

};
