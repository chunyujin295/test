#pragma once
//#include "PhotosNode.h"
#include "OSGEditor/Base.h"
#include "PhotoGeometry.h"
#include "OsgEngine.h"

//照片节点管理类
class DLL_API PhotosNodeManager : public CustomNode
{
public:
    PhotosNodeManager(OsgEngine *pOsgEngine, osg::ref_ptr<osgViewer::Viewer> pViewer);
    void SetViewer(osg::ref_ptr<osgViewer::Viewer> pViewer) { m_pViewer = pViewer; };
    void Add(const std::vector<ST_CAMERA_INFO> &vecCamera);
    //默认根据选中的id恢复状态
    virtual void Reset();
    //根据外面传入的id恢复对应相片
    void Reset(const std::vector<int>& vecPhotoID);

    virtual void Hover();
    //单选
    virtual void Picked(const SELECT_TYPE& type = SELECT_TYPE::SELECT_ONE) ;
    //根据id多选
    void Picked(const std::vector<int>& vecPhotoID);

    void RemoveChild(const int &id = -1);

    void Scale(float value);

    void RemoveAll();

    void Visible(bool value);

    //矩形框选
    bool BoxSelect(osg::ref_ptr<osgViewer::Viewer> pViewer, const osg::Vec3& minXY, const osg::Vec3& maxXY);

    //多边形框选
    bool PolygonSelect(osg::ref_ptr<osgViewer::Viewer> pViewer, const std::vector<osg::Vec3>& vecPoints);

    void GetPickedPhotosID(std::vector<int> *vecPhotosID);

    void RemoveSelectedPhoto();
    bool IsEmpty();
private:
    //初始化虚拟相机参数
    void InitCamera(const ST_CAMERA_INFO &camera);
    void InitCentrumGeometry();
    void InitNoCentrumGeometry();

    void Clear();

    void EventCallBack();

    void ScaleCentrumGeometry();


    static void Worker(void* arg);   //工作线程
private:
    float m_fVirtualDepth = 2.0;

    std::vector<CameraCentrum> m_vecCameraCentrum;
    std::vector<CameraCentrum> m_vecCameraNoCentrum;   //   

    osg::ref_ptr<osgViewer::Viewer> m_pViewer;
    osg::ref_ptr<osg::Geode>  m_pPickedCentrumGeode; //控制点击后添加视椎体
    osg::ref_ptr<osg::Switch> m_pRootGeodeSwitch; //控制显隐

    std::map<int, ST_CAMERA_INFO> m_pTotalCameraInfoMap;
    std::vector<osg::ref_ptr<PhotoGeometry>> m_vecSelectedPhotoGeometry;
    std::vector<osg::ref_ptr<PhotoGeometry>> m_vecTotalPhotoGeometry;
    OsgEngine* m_pOsgEngine;
    float mScale = 1.0;//@add by chenhaiyan
};

                                                                                                