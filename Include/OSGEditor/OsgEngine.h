/*! @file
********************************************************************************
<PRE>
模块名       : <OsgEngine>
文件名       : <OsgEngine.h>
相关文件     : <OsgEngine.cpp>
文件实现功能 : <API接口文件>
作者         : <zhaobf>
版本         : <1.0>
日期         : 2023.7
</PRE>
*******************************************************************************/
#pragma once

#include <windows.h>
#include "OSGEditor/Base.h"
#include "OSGEditor/OsgEngineManipulator.h"
#include "OSGEditor/PickEventHandler.h"
#include "OSGEditor/SurveyPointsNode.h"
//#include "OSGEditor/PhotosNode.h"
#include "OSGEditor/PointNode.h"
#include "OSGEditor/ModelNode.h"
#include "OSGEditor/ROINode.h"
#include "OSGEditor/TileNode.h"
#include "OSGEditor/PolygonNode.h"
#include "Core/ReconstructionObject.h"
#include "Core/ATData.h"
#include "OSGEditor/PhotosNodeManager.h"
using namespace AI3D::CORE;
class PickEventHandler;
class PointNode;
class ROINode;
class PhotosNodeManager;
class  ModelNode;
class DLL_API OsgEngine
{
public:
    OsgEngine();
    ~OsgEngine() { std::cout << "osgengine destroyed:" << std::hex << std::showbase << this << std::dec << std::endl; };

    const float kInitNearPlane = 0.5f;// 1.0f;
    const float kMinNearPlane = 1e-3f;
    const float kMaxNearPlane = 1e5f;
    const float kNearPlaneScaleSpeed = 0.02f;
    const float kFarPlane = 1e5f; //100000000;
    const float kInitFocusDistance = 100.0f; //10.f
    const float kMinFocusDistance = 1e-5f;;//1e-10f;
    const float kMaxFocusDistance = 1e8f;
    const float kFieldOfView = 25.0f;
    const float kFocusSpeed = 1.0f;
    const float kTranslateSpeed = 2.0f;
    const float kInitPointSize = 4.0f;//5.0
    const float kMinPointSize = 0.1f;
    const float kMaxPointSize = 100.0f;
    const float kPointScaleSpeed = 0.1f;
    const float kInitImageSize = 0.04f; //colmap : 0.2f  my 5.0f;
    const float kMinImageSize = 1e-6f;
    const float kMaxImageSize = 1e3f;
    const float kImageScaleSpeed = 0.1f;
    const int kDoubleClickInterval = 250;
    
    void RemovePickedPhotosFromATSide();                                                          
    void RemovePickedTiePointsFromATSide(); 
    float focus_distance_;
                                                                                
    Eigen::Vector4f scene_center_dot_;

    float point_size_;

    float image_size_;
    
    

    
    static OsgEngine* getInstance2(unsigned long callerId,bool *bNewEngine = nullptr);
    static void deleteInstance(unsigned long callerId);
    void Run2();
    
//to delete @

    void initViewer();
     //删除节点
    void Remove(const Element_Type &type, osg::ref_ptr<osg::Node> pNode);
        
    
   
    osg::Geode* createFrustumGeode(osg::Camera* camera);

    //相机聚焦模型
  //  void LookAtModel(osg::ref_ptr<osg::Node> pNode, ModelViewType type = ModelViewType::MODEL_FRONT);
    //====以下为各个视图渲染的具体内容的接口==========//
    //1:空三数据
  
    void RenderBBox(ST_BOUNDINGBOX box);

  

    //5:tile块
    void RenderTiles(const std::vector<ST_BOUNDINGBOX>& box);
    //6:模型
  
    //加载osg模型
    osg::ref_ptr<osg::Node> LoadOsgModel(std::string& filePath/*osg文件路径*/);
    
    void  ScaleTiePointsElement(const float& scale);
    void ScalePhotosElement(const float& scale);
    
    
    void BuildAxis( const ABBox3d box);
    static OsgEngine* getInstance();
    void initViewer(osg::ref_ptr< osgViewer::Viewer>pViewer, int x, int y, int width, int height);
    void Run();
    

    //获取场景viewer
    osg::ref_ptr<osgViewer::Viewer>  GetViewer();
    
    //获取场景根节点
    osg::ref_ptr<osg::Group> GetRootNode();
    //添加照片、相机视椎体及缩略图
    void AddPhotos(const std::vector<ST_CAMERA_INFO>& stCamera);
   
    
//private:
    //添加照片、相机视椎体及缩略图
   // osg::ref_ptr<osg::Node> AddPhotos(const ST_CAMERA_INFO& stCamera);
   

                                           

   
                                                


    //加载osg模型
    osg::ref_ptr<osg::Node> LoadOsgModel(const int &id, const std::string &name, std::string &filePath/*osg文件路径*/);
                              
                                                             

     osg::ref_ptr<osg::Node> AddTiePoint(const ST_TIEPOINT& tiepoint);                                           
    

    //添加模型包围盒
    osg::ref_ptr<osg::Node> DrawBoundBox(osg::Node *pNode/*场景模型节点*/);
  

    //添加兴趣框
    void AddROIBox(const std::vector<ST_BOUNDINGBOX> & box);    //立方体
    void AddROIBox(const std::vector <ST_POLYGON_BOX> & box);   //多面体

    
   //添加多边形范围、水域约束
    void AddPolygon(const int& id, const std::string& name, const osg::ref_ptr<osg::Vec3Array> pPoints);

    bool IsATEmpty();
    //兴趣框编辑状态开关
    void SetROIStatus(bool status);;
    const bool GetROIStatus();

    osg::ref_ptr<osg::Node> AddTileNodes(const std::vector<ST_BOUNDINGBOX>& box);

    //添加控制点
    void AddControlPoint(const int& id, const osg::Vec3& location, const  std::string& txt, const int& type);       

    //相机聚焦模型
    void LookAtModel(osg::ref_ptr<osg::Node> pNode, const ModelViewType& type = ModelViewType::MODEL_FRONT,float scale = 4);
   
    //相机聚焦图层
    void LookAt(const ELEMENT_LAYER_TYPE& layer, const ModelViewType& type = ModelViewType::MODEL_FRONT);
    //删除元素
    void Remove(const ELEMENT_LAYER_TYPE& type, const int &id);
    //删除osg模型    
    void Remove(osg::ref_ptr<osg::Node> pNode);
    //删除图层节点
    void RemoveAll(const ELEMENT_LAYER_TYPE& type);
    //清空场景所有模型
    void RemoveAll();
    void RemoveScene();
    //设置当前选择类型
    void SetSelectType(const SELECT_TYPE& type);
    const SELECT_TYPE& GetCurrentSelectType();

    //设置当前选择图层、要素
    void SetElementType(const ELEMENT_LAYER_TYPE&type);
    const ELEMENT_LAYER_TYPE& GetCurrentElementType();
    
    
   
    
    //元素图层选择联动
    void SetSelectElement(const ELEMENT_LAYER_TYPE& type, const std::vector<int> &vecID);
    //情况选中的元素
    void ClearSelectElement();
    void ClearSelectElement(const ELEMENT_LAYER_TYPE& type, const std::vector<int>& vecID);

    void ScaleElement(float scale);
    //图层显隐
    void SetElementVisible(const ELEMENT_LAYER_TYPE& type, bool status);

    std::vector<osg::ref_ptr<CustomNode>>* GetPickedNode() ;

    void RemovePickedNode();
    void DeselectPickedNodeWithoutDeleting();

    void SetSceneOperationType(const SECENE_OPERATION_TYPE& mode);

    SECENE_OPERATION_TYPE GetSceneOperationType();

    osg::ref_ptr<CustomNode> GetElementLayerRoot(const ELEMENT_LAYER_TYPE& type);                                                                                       
     //添加坐标轴模型
    void AddCoordinateAxis(const osg::Vec3& center, const osg::Vec3& range, const float& textSize = 20.0);
    //删除坐标轴
    void RemoveCoordinateAxis();

    void PauseEngine(bool bPause);

    //@add by  20231127 为了增加提交重建后空三不能编辑而临时加的
    bool bCanDelete = true;
    bool bPaused = false;
private:
    
    static OsgEngine* m_pOsgEngine;                                                       
    osg::ref_ptr <osgViewer::Viewer> m_pOsgViewer;
    osg::ref_ptr<osg::Camera> m_pOsgCamera;
    osg::ref_ptr<osg::Group> m_pRootGroup;
     osg::ref_ptr<PickEventHandler> m_pPickEventHandler;                                                   
 //要素根节点集合
    osg::ref_ptr<PhotosNodeManager> m_pPhotosNodeManager;  //照片根节点管理对象                                                                             
    
    osg::ref_ptr<PointNode> m_pTiesPointsRootGroup;         //点云
    osg::ref_ptr<SurveyPointsNode> m_pSurveyPointsRootGroup;  //控制点
    osg::ref_ptr<TileNode> m_pTilesRootGroup;
    osg::ref_ptr<ROINode> m_pROIRootGroup;
    osg::ref_ptr<ModelNode> m_pOsgModelRootGroup;
    osg::ref_ptr<PolygonNode> m_pPolygonRootGroup;  //多边形根节点
    osg::ref_ptr<osg::PositionAttitudeTransform> m_pCoordinateAxisNode;
    
    
    //std::vector<osg::ref_ptr<PhotosNode>> m_pPhotosNodeVector;

   
   

    
    
   

    std::vector<osg::ref_ptr<CustomNode>> m_vecPickedNode;

    SELECT_TYPE m_eCurrentSelectType = SELECT_TYPE::SELECT_ONE;
    ELEMENT_LAYER_TYPE m_eCurrentElementType = ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS;
                                                                
   
    std::string m_strResourceDir;
    bool m_bROIStatus = false;
    SECENE_OPERATION_TYPE m_enSceneOperationType;




};

