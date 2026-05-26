// Copyright Airlook, Inc. All Rights Reserved.
#pragma once

#include "base.h"
//#include "PhotosNode.h"
#include "OsgEngine.h"
#include "BoxSelect.h"
#include "PolygonSelect.h"


class OsgEngine;
class PolygonSelect;
class BoxSelect;
class ROINode;
class PickEventHandler : public osgGA::GUIEventHandler
{
public:
    PickEventHandler(OsgEngine* pOsgEngine);
    PickEventHandler() { m_pOsgEngine = nullptr; }
    ~PickEventHandler() {}
    
    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor*);
    void SetCurrentSelectType(const SELECT_TYPE& type);                                                

    void Clear();

private:
    void Clear(const osgGA::GUIEventAdapter& ea);
    void Hover(const osgGA::GUIEventAdapter& ea);
    osg::ref_ptr <CustomNode> Pick(osgViewer::View* pview, float x, float y);
    osg::ref_ptr <CustomNode> Hover(osgViewer::View* pview, float x, float y);

    void Init();

    //元素图层
    bool Photos(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa); //相片、相机
    bool Tile(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);   //切片
    bool TiePoints(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);  //点云
    bool SurveyPoints(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);  //测控点
    bool ROI(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);         //兴趣框


private:
    osgViewer::View* m_pView;
    OsgEngine* m_pOsgEngine;
    float m_fPickX;
    float m_fPickY;
    float m_fHoverX;
    float m_fHoverY;
    float m_fLastX;
    float m_fLastY;
    double m_tLastFrameTime;
    bool m_bMouseLeft = false;
    bool m_bMouseRight = false;
    bool m_bMouseMid = false;
    bool m_bHover = false;
    bool m_bDrag = false;
    bool m_bMove = false;
    bool m_bSelectPicked = false;   //框选、多边形选择状态
    osg::Timer* m_pTimer;
    bool m_bROIDragStatus = false;  //兴趣框拖拽状态
                     
    osg::ref_ptr<CustomNode> m_pHoverNode;
    osg::ref_ptr<osg::Camera> m_pBoxCamera;
    osg::ref_ptr<BoxSelect> m_pBoxSelect;
    osg::ref_ptr<PolygonSelect> m_pPolygonSelect;
    osg::ref_ptr<osg::Node> m_pHoverTextNode;
    osgGA::GUIEventAdapter::EventType m_eLastEventType;
};

