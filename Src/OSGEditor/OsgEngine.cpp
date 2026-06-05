#include "OSGEditor/OsgEditorDllBuild.h"
#include "OSGEditor/OsgEngine.h"
#include "OSGEditor/PhotosNodeManager.h"
#include "OSGEditor/LodTreeProcessor.h"
#include "OSGEditor/CoordinateAxis.h"
#include "OSGEditor/Unitl.h"
#include "OSGEditor/EventManager.h"
#include <future>
using namespace AI3D::CORE;

namespace {

inline PhotosNodeManager* AsPhotosNodeManager(const osg::ref_ptr<CustomNode>& node)
{
    return static_cast<PhotosNodeManager*>(node.get());
}

} // namespace

static std::map<unsigned long, OsgEngine*> m_mapEngineCaller;//@add by 

OsgEngine::OsgEngine()
{
    m_pOsgViewer = new osgViewer::Viewer();
    m_pRootGroup = new osg::Group();
    m_pRootGroup->setDataVariance(osg::Object::DataVariance::DYNAMIC);
    //m_pPhotosRootGroup = new PhotosNode();

    m_pTiesPointsRootGroup = new PointNode(this);
    m_pSurveyPointsRootGroup = new SurveyPointsNode();
    
    m_pTilesRootGroup = new TileNode();
    m_pROIRootGroup = new ROINode(this);
    m_pOsgModelRootGroup = new ModelNode();
    m_pPolygonRootGroup = new PolygonNode();                                 

   // m_pRootGroup->addChild(m_pPhotosRootGroup);
    m_pRootGroup->addChild(m_pTiesPointsRootGroup);
    m_pRootGroup->addChild(m_pSurveyPointsRootGroup);
    m_pRootGroup->addChild(m_pTilesRootGroup);
    m_pRootGroup->addChild(m_pROIRootGroup);
    m_pRootGroup->addChild(m_pOsgModelRootGroup);                                         
    m_pRootGroup->addChild(m_pPolygonRootGroup);
    m_pPhotosNodeManager = new PhotosNodeManager(this, m_pOsgViewer);
    m_pRootGroup->addChild(m_pPhotosNodeManager);

    m_strResourceDir = Unitl::GetCurrentDir();

    image_size_ = static_cast<float>(2 * image_size_);
    point_size_ = static_cast<float>(2 * point_size_);

    std::cout << "osgengine constructor:" << std::hex << std::showbase << this << std::dec << std::endl;
}

OsgEngine::~OsgEngine()
{
    std::cout << "osgengine destroyed:" << std::hex << std::showbase << this << std::dec << std::endl;
}

OsgEngine* OsgEngine::getInstance()
{

    static OsgEngine* s_registry = new OsgEngine();
    return s_registry; // will return NULL on erase
    //if (m_pOsgEngine == NULL) {
    //    m_pOsgEngine = new OsgEngine();
    //}

    //return m_pOsgEngine;
}

// callerId:different callerId represents different OsgEngine instance.
OsgEngine* OsgEngine::getInstance2(unsigned long callerId,bool *bNewEngine)
{
    OsgEngine* pCurrEngine = nullptr;
    
    if(bNewEngine)
        *bNewEngine = false;

    if (callerId == 0)
        return pCurrEngine;

    if (m_mapEngineCaller.find(callerId) != m_mapEngineCaller.end())
    {
        return m_mapEngineCaller.find(callerId)->second;
    }

    pCurrEngine = new OsgEngine();
    m_mapEngineCaller.insert(make_pair(callerId, pCurrEngine));

    if(bNewEngine)
        *bNewEngine = true;

    return pCurrEngine;
}

void OsgEngine::deleteInstance(unsigned long callerId)
{
    if (callerId == 0)
        return;

    if (m_mapEngineCaller.find(callerId) != m_mapEngineCaller.end())
    {
        m_mapEngineCaller.erase(callerId);
    }
}

//void OsgEngine::SetViewer(osg::ref_ptr<osgViewer::Viewer> pviewer)
//{
//    m_pOsgViewer = pviewer;
//}

void OsgEngine::initViewer(osg::ref_ptr< osgViewer::Viewer> pViewer, int x, int y, int width, int height)
{
    osg::ref_ptr<osgGA::OrbitManipulator> manipulator = new osgGA::OrbitManipulator;
    if (pViewer)
    {
        m_pOsgViewer = pViewer;
        AsPhotosNodeManager(m_pPhotosNodeManager)->SetViewer(m_pOsgViewer);
    }
    manipulator->setAllowThrow(false);
            manipulator->setMinimumDistance(1.0);                            
    m_pOsgViewer->setCameraManipulator(manipulator/*new osgGA::TrackballManipulator()*/);
    m_pOsgViewer->getCameraManipulator()->setAutoComputeHomePosition(false);

    m_pOsgViewer->addEventHandler(new osgGA::StateSetManipulator(m_pOsgViewer->getCamera()->getOrCreateStateSet()));

    m_pOsgViewer->addEventHandler(new osgViewer::ThreadingHandler);

    m_pOsgViewer->addEventHandler(new osgViewer::WindowSizeHandler);

    m_pOsgViewer->addEventHandler(new osgViewer::StatsHandler);

    m_pOsgViewer->addEventHandler(new osgViewer::HelpHandler());

    m_pOsgViewer->addEventHandler(new osgViewer::RecordCameraPathHandler);

    m_pOsgViewer->addEventHandler(new osgViewer::LODScaleHandler);

    m_pOsgViewer->addEventHandler(new osgViewer::ScreenCaptureHandler);
    m_pPickEventHandler = new PickEventHandler(this);
    m_pOsgViewer->addEventHandler(m_pPickEventHandler);

    m_pOsgViewer->setThreadingModel(ViewerBase::AutomaticSelection);
    m_pOsgViewer->getDatabasePager()->setUpThreads(8, 0);
    m_pOsgViewer->getDatabasePager()->setDoPreCompile(true);
    m_pOsgViewer->getDatabasePager()->setTargetMaximumNumberOfPageLOD(3000);
    m_pOsgViewer->setKeyEventSetsDone(0); //屏蔽ESC
    m_pOsgCamera = m_pOsgViewer->getCamera();

  //  m_pOsgCamera->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES);
   // m_pOsgCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    m_pOsgCamera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    m_pOsgCamera->setClearColor(Unitl::FromHex("151515"));
    //m_pOsgCamera->setClearColor(osg::Vec4(15/255.f, 15/255.f, 15/255.f, 1.f)); //设置场景背景色
    osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
    ds->setNumMultiSamples(4);//抗锯齿
    m_pOsgViewer->setDisplaySettings(ds);

    //m_pOsgCamera ->getOrCreateStateSet()->setRenderBinMode(StateSet::USE_RENDERBIN_DETAILS);
    //m_pOsgCamera->setRenderOrder(osg::Camera::PRE_RENDER);
    m_pOsgCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER);

    //初始化坐标轴
    //InitCoordinateAxis();

    //osg::GraphicsContext *gc =  m_pOsgViewer->getCamera()->getGraphicsContext();
    //int num = gc->getWindowingSystemInterface()->getNumScreens();
    if (m_pOsgViewer->getCamera()->getGraphicsContext() == nullptr)
    {
        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
        traits->x = x;
        traits->y = y;
        traits->width = width;
        traits->height = height;
        traits->windowDecoration = true;
        traits->doubleBuffer = true;
        traits->sharedContext = 0;
        traits->samples = 4;
        traits->useCursor = true;

        osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
        m_pOsgViewer->getCamera()->setGraphicsContext(gc);
        m_pOsgViewer->getCamera()->setViewport(new osg::Viewport(x, y, width, height));
    }

    osgDB::Registry::instance()->setOptions(new osgDB::Options()); 
    osgDB::Registry::instance()->getOptions()->setObjectCacheHint(osgDB::Options::CACHE_ALL);  //设置读取的模型是否缓存

    //设置预编译
    osgUtil::IncrementalCompileOperation* pIn = new osgUtil::IncrementalCompileOperation;
    pIn->assignForceTextureDownloadGeometry();
    pIn->setMinimumTimeAvailableForGLCompileAndDeletePerFrame(1);
    pIn->setConservativeTimeRatio(1);
    pIn->setMaximumNumOfObjectsToCompilePerFrame(20);
    m_pOsgViewer->setIncrementalCompileOperation(pIn);
                                                                                                                                
    m_pRootGroup->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

    m_pRootGroup->setDataVariance(Object::DataVariance::DYNAMIC);
    //m_pOsgViewer->setUpViewOnSingleScreen(1);
   // m_pOsgViewer->setSceneData(m_pRootGroup);
}


void OsgEngine::Run()
{
    osgUtil::Optimizer optimizer;
    

    while (!m_pOsgViewer->done())
    {
        m_pOsgViewer->frame();
    }
}


void OsgEngine::AddCoordinateAxis(const osg::Vec3& center, const osg::Vec3& range, const float& textSize)
{

    Geode* coord = new Geode;
   
    coord->getOrCreateStateSet()->setAttribute(new LineWidth(2), StateAttribute::ON);
    //coord->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    coord->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
    coord->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    coord->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE); // 关闭深度测试，并设置覆盖标志
   
    
Unitl::AddAxis(coord, Vec3(range.x(), 0., 0.), "", Vec4(1., 0., 0., 1.)); // x-axis
Unitl::AddAxis(coord, Vec3(0., range.y(), 0.), "", Vec4(0., 1., 0., 1.)); // y-axis
Unitl::AddAxis(coord, Vec3(0., 0., range.z()), "", Vec4(0., 0., 1., 1.)); // z-axis
    m_pCoordinateAxisNode = new osg::PositionAttitudeTransform;
    m_pCoordinateAxisNode->addChild(coord);
    m_pCoordinateAxisNode->setPosition(center);

    m_pRootGroup->addChild(m_pCoordinateAxisNode);

   
    osg::ref_ptr<osg::AutoTransform> pXNode = Unitl::AddAxisText(Vec3(range.x(), 0., 0.), "X", textSize);
    osg::ref_ptr<osg::AutoTransform> pYNode = Unitl::AddAxisText(Vec3(0., range.y(), 0.), "Y", textSize);
    osg::ref_ptr<osg::AutoTransform> pZNode = Unitl::AddAxisText(Vec3(0., 0., range.z()), "Z", textSize);
    m_pCoordinateAxisNode->addChild(pXNode);
    m_pCoordinateAxisNode->addChild(pYNode);
    m_pCoordinateAxisNode->addChild(pZNode);                                                                                                     

}
void OsgEngine::RemoveCoordinateAxis()
{
    if (m_pCoordinateAxisNode.valid())
    {
        m_pRootGroup->removeChild(m_pCoordinateAxisNode);
    }
}
void OsgEngine::Run2()
{
//    osgUtil::Optimizer optimizer;
//    optimizer.optimize(m_pRootGroup);
//    m_pOsgViewer->setSceneData(m_pRootGroup);

    //m_pOsgViewer->realize();
    //m_pOsgViewer->run();

    ///while (!m_pOsgViewer->done())
    if(!m_pOsgViewer->done())
    {
        m_pOsgViewer->frame();
    }
}

osg::ref_ptr <osgViewer::Viewer> OsgEngine::GetViewer()
{ 
    return m_pOsgViewer; 
}

osg::ref_ptr<osg::Group> OsgEngine::GetRootNode()
{
    return m_pRootGroup;
}

osg::ref_ptr<osg::Node> OsgEngine::LoadOsgModel(std::string& strFilePath)
{
    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(strFilePath);    
    m_pRootGroup->addChild(loadedModel);

    return loadedModel;
}

osg::ref_ptr<osg::Node> OsgEngine::LoadOsgModel(const int& id, const std::string& name, std::string& strFilePath)
{
    if (!osgDB::fileExists(strFilePath))
    {
        return nullptr;
    }

    m_pOsgModelRootGroup->AddChild(id, new ModelNode(id, name, strFilePath, this));
    m_pOsgViewer->getDatabasePager()->registerPagedLODs(m_pOsgModelRootGroup);
    
    return m_pOsgModelRootGroup;
}
osg::ref_ptr<osg::Node> OsgEngine::AddTiePoint(const ST_TIEPOINT& tiepoint)
{
    if (tiepoint.ID == 0)
    {
        return nullptr;
    }
    m_pTiesPointsRootGroup->AddChild(tiepoint.ID, new PointNode(tiepoint,this));

    //test
    if(0)
    {
        std::string name("test");
        string path = "H:\\Production_2\\data\\Production_2.osgb";
        m_pOsgModelRootGroup->AddChild(tiepoint.ID, new ModelNode(tiepoint.ID, name, path, this));

        osg::ref_ptr<osg::CullFace> cullface = new osg::CullFace(osg::CullFace::BACK);
        m_pOsgModelRootGroup->getOrCreateStateSet()->setAttribute(cullface.get());
        m_pOsgModelRootGroup->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::ON);

        m_pOsgViewer->getDatabasePager()->registerPagedLODs(m_pOsgModelRootGroup);

    }
    return nullptr;
}

osg::ref_ptr<osg::Node> OsgEngine::DrawBoundBox(osg::Node* pNode)
{
    osg::ComputeBoundsVisitor cbVisitor;
    pNode->accept(cbVisitor);
    Unitl mUnitl;
    osg::ref_ptr<osg::Node> pBoxNode = mUnitl.CreateBoxBorder(&cbVisitor.getBoundingBox(), osg::Vec4(67. / 255.0f, 248. / 255.0f, 238. / 255.0f, 1.0f));
    m_pRootGroup->addChild(pBoxNode);
    return pBoxNode;
}

std::vector<osg::ref_ptr<CustomNode>>* OsgEngine::GetPickedNode() { 
    //std::cout << "inside " << __FILE__ << " " << __LINE__ << " " << m_vecPickedNode.size() << std::endl;
    return &m_vecPickedNode; 
};

size_t OsgEngine::GetPickedNodeCount() const
{
    return m_vecPickedNode.size();
}

void OsgEngine::GetPickedElementIds(std::vector<int>& elementIds) const
{
    elementIds.clear();
    for (const auto& node : m_vecPickedNode) {
        if (node.valid()) {
            elementIds.push_back(node->m_iID);
        }
    }
}

bool OsgEngine::GetPickedPhotoIds(std::vector<int>& photoIds)
{
    photoIds.clear();
    if (m_vecPickedNode.empty()) {
        return false;
    }
    PhotosNodeManager* photos = dynamic_cast<PhotosNodeManager*>(m_vecPickedNode.front().get());
    if (!photos) {
        return false;
    }
    photos->GetPickedPhotosID(&photoIds);
    return !photoIds.empty();
}

bool OsgEngine::GetPickedTileIds(std::vector<int>& tileIds)
{
    tileIds.clear();
    for (const auto& node : m_vecPickedNode) {
        if (!node.valid()) {
            continue;
        }
        if (node->GetElementType() == ELEMENT_LAYER_TYPE::ELEMENT_TILE && node->m_iID > 0) {
            tileIds.push_back(node->m_iID);
        }
    }
    return !tileIds.empty();
}

void OsgEngine::GetTileDirCoarseLevelTrees(
    const std::string& dir, std::vector<std::string>& trees, const std::string& extension)
{
    LodTreeProcessor::GetTileDirCoarseLevelTrees(dir, trees, extension);
}

osg::ref_ptr<osg::Node> OsgEngine::AddTileNodes(const std::vector<ST_BOUNDINGBOX>& box)
{

    for (auto it : box)
    {
        m_pTilesRootGroup->AddChild(it.ID,new TileNode(it));
    }

    return m_pTilesRootGroup;
}
void OsgEngine::AddROIBox(const std::vector<ST_BOUNDINGBOX>& box)
{
    
    for (auto it : box)
    {
        m_pROIRootGroup->AddChild(it.ID, new ROINode(it,this));
    }
    

}

void OsgEngine::AddROIBox(const std::vector <ST_POLYGON_BOX>& box)
{
   for (auto it : box)
   {
       m_pROIRootGroup->AddChild(it.ID, new ROINode(it,this));
   }
}


void OsgEngine::AddPolygon(const int& id, const std::string& name, const osg::ref_ptr<osg::Vec3Array> pPoints)
{
    m_pPolygonRootGroup->AddChild(id, new PolygonNode(id, name, pPoints));
}




void OsgEngine::AddControlPoint(const int& id, const osg::Vec3& location, const  std::string& txt, const int& type)
{
                                                                                                         

    m_pSurveyPointsRootGroup->AddChild(id, new SurveyPointsNode(id, txt, location, type));

                                    
}

void OsgEngine::AddPhotos(const std::vector<ST_CAMERA_INFO>& stCamera)
{
    osg::Timer tmpTimer;
    tmpTimer.setStartTick();

    AsPhotosNodeManager(m_pPhotosNodeManager)->Add(stCamera);
   // std::cout << "init camera : " << tmpTimer.time_s() << std::endl;
}


void OsgEngine::LookAt(const ELEMENT_LAYER_TYPE& layer, const ModelViewType& type)
{
    switch (layer)
    {
    case ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS:
    {
        LookAtModel(m_pPhotosNodeManager, type);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_SURVEY_POINTS:
    {
        LookAtModel(m_pSurveyPointsRootGroup, type);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS:
    {
        LookAtModel(m_pTiesPointsRootGroup, type);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_TILE:
    {
        LookAtModel(m_pTilesRootGroup, type);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_ROI:
    {
        LookAtModel(m_pROIRootGroup, type);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_MODEL:
    {
        LookAtModel(m_pOsgModelRootGroup, type);
    }break;
    default:
        break;
    }
}

void OsgEngine::LookAtModel(osg::ref_ptr<osg::Node> pNode, const ModelViewType& type, float scale)
{
    if (!pNode)
        return;

   // float scale = 2000;
    //默认初始视角
    //osg::Vec3d eye, center = pNode->getBound().center(), up = Vec3(0, 0, 1);
    osg::Vec3d eye, center = pNode->getBound().valid() ? pNode->getBound().center() : Vec3(0, 100, 0), up = Vec3(0, 0, 1);
    osg::Camera *pCamera = m_pOsgViewer->getCamera();
    eye = center + Vec3(0, -1, 0) * pNode->getBound().radius() * scale;
    //eye = pNode->getBound().center() + Vec3(0, -1, 0) * pNode->getBound().radius() * 4;
    m_pOsgViewer->getCameraManipulator()->setHomePosition(eye, center, up);
    m_pOsgViewer->home();

    //m_pOsgViewer->getCameraManipulator()->getHomePosition(eye, center, up);

    osg::Matrixd cmt = m_pOsgViewer->getCameraManipulator()->getMatrix();
    
    
    switch (type)
    {
    case MODEL_FRONT:
    {
       
    }
        break;
    case MODEL_BACK:
    {
       osg::Matrixd newMt = cmt * osg::Matrix::rotate(osg::DegreesToRadians(180.), osg::Vec3d(0, 0, 1));

        //设置漫游器后，该接口无效
        //pCamera->setViewMatrixAsLookAt(newMt.getTrans(), pNode->getBound().center(), Vec3(0, 0, 1));
        eye = center + Vec3(0, 1, 0) * pNode->getBound().radius() * scale;
        newMt.setTrans(eye);
        m_pOsgViewer->getCameraManipulator()->setByMatrix(newMt);
    }
        break;
    case MODEL_LEFT:
    {
        osg::Matrixd newMt = cmt * osg::Matrix::rotate(osg::DegreesToRadians(90.), osg::Vec3d(0, 0, 1));
        eye = center + Vec3(1, 0, 0) * pNode->getBound().radius() * scale;
        newMt.setTrans(eye);    
        m_pOsgViewer->getCameraManipulator()->setByMatrix(newMt);
    }
        break;
    case MODEL_RIGHT:
    {
        osg::Matrixd newMt = cmt * osg::Matrix::rotate(osg::DegreesToRadians(-90.), osg::Vec3d(0, 0, 1));
        eye = center + Vec3(-1, 0, 0) * pNode->getBound().radius() * scale;
        newMt.setTrans(eye);
        m_pOsgViewer->getCameraManipulator()->setByMatrix(newMt);
    }
        break;
    case MODEL_UP:
    {
      
        osg::Matrixd newMt = cmt * osg::Matrix::rotate(osg::DegreesToRadians(-90.), osg::Vec3d(1, 0, 0));

        eye = center + Vec3(0, 0, 1) * pNode->getBound().radius() * scale;
        newMt.setTrans(eye);
        m_pOsgViewer->getCameraManipulator()->setByMatrix(newMt);
    }
        break;
    case MODEL_DOWN:
    {
        osg::Matrixd newMt = cmt * osg::Matrix::rotate(osg::DegreesToRadians(90.), osg::Vec3d(1, 0, 0));

        eye = center + Vec3(0, 0, -1) * pNode->getBound().radius() * scale;

        newMt.setTrans(eye);
        m_pOsgViewer->getCameraManipulator()->setByMatrix(newMt);
              
        
    }
        break;
    default:
        break;
    }

}

//从场景根节点中删除
void OsgEngine::Remove(osg::ref_ptr<osg::Node> pNode)
{
    if (pNode)
    {
        m_pRootGroup->removeChild(pNode);
    }
}

//从要素图层中删除
void OsgEngine::Remove(const ELEMENT_LAYER_TYPE& type, const int& id)
{
   
    switch (type)
    {
    case ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS:                                                 
    {
        AsPhotosNodeManager(m_pPhotosNodeManager)->RemoveChild(id);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_SURVEY_POINTS:
    {
        m_pSurveyPointsRootGroup->RemoveChild(id);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS:
 
        
    {
        m_pTiesPointsRootGroup->RemoveChild(id);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_TILE:
    {
        m_pTilesRootGroup->RemoveChild(id);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_ROI:
    {
        m_pROIRootGroup->RemoveChild(id);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_MODEL:
    {
        m_pOsgModelRootGroup->RemoveChild(id);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_POLYGON:
    {
        m_pPolygonRootGroup->RemoveChild(id);
    }break;
    default:
        break;
    }
}
void  OsgEngine::RemoveAll(const ELEMENT_LAYER_TYPE& type)
{
    switch (type)
    {
    case ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS:
    {
        AsPhotosNodeManager(m_pPhotosNodeManager)->RemoveAll();
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_SURVEY_POINTS:
    {
        m_pSurveyPointsRootGroup->RemoveAll();
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS:
    {
        m_pTiesPointsRootGroup->RemoveAll();
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_TILE:
    {
        m_pTilesRootGroup->RemoveAll();
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_ROI:
    {
        m_pROIRootGroup->RemoveAll();
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_MODEL:
    {
        m_pOsgModelRootGroup->RemoveAll();
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_POLYGON:
    {
        m_pPolygonRootGroup->RemoveAll();
    }break;
    default:
        break;
    }
}

void OsgEngine::RemoveAll()
{
    AsPhotosNodeManager(m_pPhotosNodeManager)->RemoveAll();
    m_pSurveyPointsRootGroup->RemoveAll();
    m_pTiesPointsRootGroup->RemoveAll();
    m_pTilesRootGroup->RemoveAll();
    m_pROIRootGroup->RemoveAll();
    m_pOsgModelRootGroup->RemoveAll();
    m_pPolygonRootGroup->RemoveAll();
    RemoveCoordinateAxis();
}
bool OsgEngine::IsATEmpty()
{
    return AsPhotosNodeManager(m_pPhotosNodeManager)->IsEmpty();
}
void OsgEngine::RemoveScene()
{
   
    m_pTilesRootGroup->RemoveAll();
    m_pROIRootGroup->RemoveAll();
    m_pOsgModelRootGroup->RemoveAll();
    m_pPolygonRootGroup->RemoveAll();
    RemoveCoordinateAxis();
}



void OsgEngine::RemovePickedNode()
{
   // for (auto it : m_vecPickedNode)
    {
        switch (GetCurrentElementType())
        {
        case ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS:
        {
            std::vector<int> tmpPickedID;
            AsPhotosNodeManager(m_pPhotosNodeManager)->GetPickedPhotosID(&tmpPickedID);
            AsPhotosNodeManager(m_pPhotosNodeManager)->RemoveChild();

            ////删除影像对应的点
            if (m_pTiesPointsRootGroup->getNumChildren() == 1)
            {
                auto pointIt = m_pTiesPointsRootGroup->GetAllChild()->begin();
                PointNode* pNode = dynamic_cast<PointNode*>(pointIt->second);

                pNode->Delete(tmpPickedID);

            }
            else
            {
                for (auto it : tmpPickedID)
                {
                    for (auto pointIt = m_pTiesPointsRootGroup->GetAllChild()->begin(); pointIt != m_pTiesPointsRootGroup->GetAllChild()->end(); pointIt++)
                    {
                        PointNode* pNode = dynamic_cast<PointNode*>(pointIt->second);

                        pNode->Delete(it);
                    }
                }

            }
           /* for (auto it : tmpPickedID)
            {
                for (auto pointIt = m_pTiesPointsRootGroup->GetAllChild()->begin(); pointIt != m_pTiesPointsRootGroup->GetAllChild()->end(); pointIt++)
                {
                    PointNode* pNode = dynamic_cast<PointNode*>(pointIt->second);

                    pNode->Delete(it);
                }
            }*/

            
        }break;
        case ELEMENT_LAYER_TYPE::ELEMENT_SURVEY_POINTS:
        {
            for (auto it : m_vecPickedNode)
            {
                m_pSurveyPointsRootGroup->RemoveChild(it);
            }
        }break;
        case ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS:
        {
            for (auto pointIt = m_pTiesPointsRootGroup->GetAllChild()->begin(); pointIt != m_pTiesPointsRootGroup->GetAllChild()->end(); pointIt++)
            {
                PointNode* pNode = dynamic_cast<PointNode*>(pointIt->second);

                pNode->Delete();
            }
        }break;
        case ELEMENT_LAYER_TYPE::ELEMENT_TILE:
        {
            for (auto it : m_vecPickedNode)
            {
                m_pTilesRootGroup->RemoveChild(it);
            }
        }break;
        case ELEMENT_LAYER_TYPE::ELEMENT_ROI:
        {
            for (auto it : m_vecPickedNode)
            {
                m_pROIRootGroup->RemoveChild(it);
            }
        }break;
        case ELEMENT_LAYER_TYPE::ELEMENT_POLYGON:
        {
            for (auto it : m_vecPickedNode)
            {
                m_pPolygonRootGroup->RemoveChild(it);
            }
        }break;
        default:
            break;
        }
    }
    m_vecPickedNode.clear();
}

void OsgEngine::DeselectPickedNodeWithoutDeleting()
{
    m_pPickEventHandler->Clear();

    {
        switch (GetCurrentElementType())
        {
        case ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS:
        {
            AsPhotosNodeManager(m_pPhotosNodeManager)->RemoveSelectedPhoto();

        }break;
        case ELEMENT_LAYER_TYPE::ELEMENT_SURVEY_POINTS:
        {
        }break;
        case ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS:
        {
        }break;
        case ELEMENT_LAYER_TYPE::ELEMENT_TILE:
        {
        }break;
        case ELEMENT_LAYER_TYPE::ELEMENT_ROI:
        {

        }break;
        case ELEMENT_LAYER_TYPE::ELEMENT_POLYGON:
        {
        }break;
        default:
            break;
        }
    }

    m_vecPickedNode.clear();
}

void OsgEngine::SetSelectType(const SELECT_TYPE& type)
{
    m_eCurrentSelectType = type;
    //改变鼠标样式
    switch (type)
    {    
    case SELECT_TYPE::SELECT_ONE:
    {
        osgViewer::Viewer::Windows windows;
        m_pOsgViewer->getWindows(windows);
        for (osgViewer::Viewer::Windows::iterator itr = windows.begin(); itr != windows.end(); ++itr)
        {
            //(*itr)->setCursor(osgViewer::GraphicsWindow::CrosshairCursor);
            (*itr)->setCursor(osgViewer::GraphicsWindow::LeftArrowCursor); //鼠标指针样式
        }
    }break;
    case SELECT_TYPE::SELECT_BOX:
    case SELECT_TYPE::SELECT_POLYGON:
    {
        osgViewer::Viewer::Windows windows;
        m_pOsgViewer->getWindows(windows);
        for (osgViewer::Viewer::Windows::iterator itr = windows.begin(); itr != windows.end(); ++itr)
        {
            (*itr)->setCursor(osgViewer::GraphicsWindow::CrosshairCursor);  //鼠标十字样式
            //(*itr)->setCursor(osgViewer::GraphicsWindow::LeftArrowCursor);
        }
    }break;
    default:
        break;
    }
    m_pPickEventHandler->SetCurrentSelectType(type);

}

void OsgEngine::SetElementType(const ELEMENT_LAYER_TYPE& type) 
{
    m_eCurrentElementType = type;
}

const SELECT_TYPE& OsgEngine::GetCurrentSelectType()
{
    return m_eCurrentSelectType;
}
const ELEMENT_LAYER_TYPE& OsgEngine::GetCurrentElementType()
{
    return m_eCurrentElementType;
}
void  OsgEngine::ScalePhotosElement(const float& scale)
{
    AsPhotosNodeManager(m_pPhotosNodeManager)->Scale(scale);
}

void  OsgEngine::ScaleTiePointsElement(const float& scale)
{
    m_pTiesPointsRootGroup->Scale(scale);
}

void OsgEngine::ScaleElement(float scale)
{
    switch (m_eCurrentElementType)
    {
    case ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS:
    {
        AsPhotosNodeManager(m_pPhotosNodeManager)->Scale(scale);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_SURVEY_POINTS:
    {
        m_pSurveyPointsRootGroup->Scale(scale);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS:
    {
        m_pTiesPointsRootGroup->Scale(scale);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_TILE:
    {
        m_pTilesRootGroup->Scale(scale);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_ROI:
    {
        m_pROIRootGroup->Scale(scale);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_POLYGON:
    {
        m_pPolygonRootGroup->Scale(scale);
    }break;
    default:
        break;
    }
}



SECENE_OPERATION_TYPE OsgEngine::GetSceneOperationType() 
{ 
    return m_enSceneOperationType; 
};

void OsgEngine::SetSelectElement(const ELEMENT_LAYER_TYPE& type, const std::vector<int>& vecID)
{
    if (vecID.size() == 0)
    {
        return;
    }

    switch (type)
    {
    case ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS:
    {
        AsPhotosNodeManager(m_pPhotosNodeManager)->Picked(vecID);
        m_vecPickedNode.push_back(m_pPhotosNodeManager);
        for (auto photoID : vecID)
        {
           

            for (auto pointIt = m_pTiesPointsRootGroup->GetAllChild()->begin(); pointIt != m_pTiesPointsRootGroup->GetAllChild()->end(); pointIt++)
            {
                PointNode* pNode = dynamic_cast<PointNode*>(pointIt->second);
                pNode->Picked(photoID);
                auto itNode = std::find(m_vecPickedNode.begin(), m_vecPickedNode.end(), pNode);
                if (itNode == m_vecPickedNode.end())
                {
                    m_vecPickedNode.push_back(pNode);
                }

            }
        }
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_SURVEY_POINTS:
    {
        for (auto it : vecID)
        {
            m_pSurveyPointsRootGroup->GetChild(it)->Picked();
            m_vecPickedNode.push_back(m_pSurveyPointsRootGroup->GetChild(it));
        }
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS:
    {
#if 1
        if (m_pTiesPointsRootGroup->GetAllChild()->size() == 1)
        {
            auto pointNode = m_pTiesPointsRootGroup->GetAllChild()->begin();
            PointNode* pNode = dynamic_cast<PointNode*>(pointNode->second);

            auto itNode = std::find(m_vecPickedNode.begin(), m_vecPickedNode.end(), pNode);
            if (itNode == m_vecPickedNode.end())
            {
                m_vecPickedNode.push_back(pNode);
            }
            pNode->Picked(vecID);

        }
        else
        {
            for (auto it : vecID) //根据影像id改变对应点的颜色
            {
                for (auto pointIt = m_pTiesPointsRootGroup->GetAllChild()->begin(); pointIt != m_pTiesPointsRootGroup->GetAllChild()->end(); pointIt++)
                {
                    PointNode* pNode = dynamic_cast<PointNode*>(pointIt->second);
                    auto itNode = std::find(m_vecPickedNode.begin(), m_vecPickedNode.end(), pNode);
                    if (pNode)
                    {
                        pNode->Picked(it);
                        if (itNode == m_vecPickedNode.end())
                        {
                            m_vecPickedNode.push_back(pNode);
                        }
                    }
                }
            }

        }
#endif
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_TILE:
    {
        for (auto it : vecID)
        {
            if (m_pTilesRootGroup && m_pTilesRootGroup->GetChild(it))
            {
                m_pTilesRootGroup->GetChild(it)->Picked();
                m_vecPickedNode.push_back(m_pTilesRootGroup->GetChild(it));
            }
        }
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_ROI:
    {
        for (auto it : vecID)
        {
            m_pROIRootGroup->GetChild(it)->Picked();
            m_vecPickedNode.push_back(m_pROIRootGroup->GetChild(it));
        }
    }break;
    default:
        break;
    }
}

void OsgEngine::ClearSelectElement()
{
    for (auto it : m_vecPickedNode)
    {
        it->Reset();
    }

    m_vecPickedNode.clear();
}

void OsgEngine::ClearSelectElement(const ELEMENT_LAYER_TYPE& type, const std::vector<int>& vecID)
{

    switch (type)
    {
    case ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS:
    {
        
        AsPhotosNodeManager(m_pPhotosNodeManager)->Reset(vecID);
        for (auto pointIt = m_pTiesPointsRootGroup->GetAllChild()->begin(); pointIt != m_pTiesPointsRootGroup->GetAllChild()->end(); pointIt++)
        {
            auto itNode = std::find(m_vecPickedNode.begin(), m_vecPickedNode.end(), pointIt->second);
            if (itNode == m_vecPickedNode.end())
            {
                continue;
            }

            dynamic_cast<PointNode*>(pointIt->second)->Reset();
            m_vecPickedNode.erase(itNode);
        }
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_SURVEY_POINTS:
    {
        //  for (auto it : vecID)
        //  {
              //m_pSurveyPointsRootGroup;
        //  }
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS:
    {
        for (auto it : vecID)
        {
            m_pTiesPointsRootGroup->GetChild(it)->Reset();
        }
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_TILE:
    {
        for (auto it : vecID)
        {
            m_pTilesRootGroup->GetChild(it)->Reset();
        }

    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_ROI:
    {
        for (auto it : vecID)
        {
            m_pROIRootGroup->GetChild(it)->Reset();
        }
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_POLYGON:
    {
        for (auto it : vecID)
        {
            m_pPolygonRootGroup->GetChild(it)->Reset();
        }
    }break;
    default:
        break;
    }

    m_vecPickedNode.clear();
}

void OsgEngine::SetROIStatus(bool status)
{
    m_bROIStatus = status;
    osg::ref_ptr<osg::Switch> pSwitch = m_pROIRootGroup->getChild(0)->asSwitch();
    if (m_bROIStatus)
    {
        for (unsigned int i = 0; i < pSwitch->getNumChildren(); i++)
        {
            ROINode* pNode = dynamic_cast<ROINode*>(pSwitch->getChild(i));
            if (pNode)
            {
                pNode->m_eMouseType = MOUSE_PICKED;
                pNode->SetROIStatus(status);

                if (pNode->IsPolygonBox())
                {
                    pNode->UpdatePlane(Unitl::FromHex(EditerEngine::ROIHoverColor[1]), "Z");
                }
                else
                {
                    pNode->UpdatePlane(Unitl::FromHex(EditerEngine::ROIHoverColor[1]), "-Y");

                }
                GetPickedNode()->push_back(pNode);
            }

        }
    }
    else
    {
        for (unsigned int i = 0; i < pSwitch->getNumChildren(); i++)
        {
            ROINode* pNode = dynamic_cast<ROINode*>(pSwitch->getChild(i));
            if (pNode)
            {
                pNode->Reset();
                pNode->SetROIStatus(status);

            }
        }
    }
}


const bool OsgEngine::GetROIStatus()
{ 
    return m_bROIStatus; 
}


void OsgEngine::SetSceneOperationType(const SECENE_OPERATION_TYPE& mode)
{
    m_enSceneOperationType = mode;
}

osg::ref_ptr<CustomNode> OsgEngine::GetElementLayerRoot(const ELEMENT_LAYER_TYPE& type)
{
    switch (type)
    {
    case ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS:
    {
        return m_pPhotosNodeManager;
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_SURVEY_POINTS:
    {
        return m_pSurveyPointsRootGroup;
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS:
    {
        return m_pTiesPointsRootGroup;
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_TILE:
    {
        return m_pTilesRootGroup;
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_ROI:
    {
        return m_pROIRootGroup;
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_POLYGON:
    {
        return m_pPolygonRootGroup;
    }break;
    default:
        break;
    }

    return nullptr;
}

void OsgEngine::SetElementVisible(const ELEMENT_LAYER_TYPE& type, bool status)
{
    switch (type)
    {
    case ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS:
    {
        AsPhotosNodeManager(m_pPhotosNodeManager)->Visible(status);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_SURVEY_POINTS:
    {
        m_pSurveyPointsRootGroup->Visible(status);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS:
    {
        m_pTiesPointsRootGroup->Visible(status);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_TILE:
    {
        m_pTilesRootGroup->Visible(status);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_ROI:
    {
        m_pROIRootGroup->Visible(status);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_POLYGON:
    {
        m_pPolygonRootGroup->Visible(status);
    }break;
    default:
        break;
    }
}

void OsgEngine::BuildAxis(const ABBox3d box)
{
    Eigen::Vector3d distance;
    distance = box.max() - box.min();

    double xlen = distance.x();
    double ylen = distance.y();
    double zlen = distance.z();
    zlen = zlen < 10 ? 20 : zlen;
    xlen = xlen < 10 ? 20 : xlen;
    ylen = ylen < 10 ? 20 : ylen;

    double scalex = 1.2;
    double scaley = 1.2;
    double scalez = 1.5;
    RemoveCoordinateAxis();
    AddCoordinateAxis(osg::Vec3(box.min().x(), box.min().y(), box.min().z()), osg::Vec3(scalex * xlen, scaley * ylen, scalez * zlen));
}
void OsgEngine::RemovePickedPhotosFromATSide()
{
    std::vector<ST_CALLBACK_ELEMENT_INFO> vecCallback;

    if (m_vecPickedNode.size() <= 0)
        return;

    for (auto t : m_vecPickedNode)
    {
        if (t->GetElementType() == ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS)
        {
            std::vector<int> tmpPickedID;
            AsPhotosNodeManager(m_pPhotosNodeManager)->GetPickedPhotosID(&tmpPickedID);
            for (auto it : tmpPickedID)
            {
                ST_CALLBACK_ELEMENT_INFO callbackinfo;
                callbackinfo.ID = it;
                //callbackinfo.name = t->m_strName;
                vecCallback.push_back(callbackinfo);

            }
        }
    } EventManager::GetInstance()->notifyEvent({ CALL_BACK_REMOVE_PHOTO, &vecCallback }, this);
   /* OsgEngine* pOsgEngine = this;
    async(launch::async, [&vecCallback, pOsgEngine]() {

        });*/

    //RemovePickedNode();
}

void OsgEngine::RemovePickedTiePointsFromATSide()
{
    std::vector<ST_CALLBACK_ELEMENT_INFO> vecCallback;

    if (m_vecPickedNode.size() <= 0)
        return;

    for (auto t : m_vecPickedNode)
    {
        if (t->GetElementType() == ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS)
        {
            for (auto pointIt = m_pTiesPointsRootGroup->GetAllChild()->begin(); pointIt != m_pTiesPointsRootGroup->GetAllChild()->end(); pointIt++)
            {
                std::vector<int> tmpPickedID;
                PointNode* pNode = dynamic_cast<PointNode*>(pointIt->second);
                pNode->GetSelectedPointID(tmpPickedID);
                for (auto it : tmpPickedID)
                {
                    ST_CALLBACK_ELEMENT_INFO callbackinfo;
                    callbackinfo.ID = it;
                    //callbackinfo.name = t->m_strName;
                    vecCallback.push_back(callbackinfo);

                }                   
            }	 
        }
    } EventManager::GetInstance()->notifyEvent({ CALL_BACK_REMOVE_TIEPOINTS, &vecCallback },this);
}


void OsgEngine::initViewer()
{
    std::cout << "begin " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
    m_pOsgViewer->setCameraManipulator(new osgGA::TrackballManipulator());
    m_pOsgViewer->getCameraManipulator()->setAutoComputeHomePosition(false);

    m_pOsgViewer->addEventHandler(new osgGA::StateSetManipulator(m_pOsgViewer->getCamera()->getOrCreateStateSet()));

    m_pOsgViewer->addEventHandler(new osgViewer::ThreadingHandler);

    m_pOsgViewer->addEventHandler(new osgViewer::WindowSizeHandler);

    m_pOsgViewer->addEventHandler(new osgViewer::StatsHandler);

    m_pOsgViewer->addEventHandler(new osgViewer::HelpHandler());

    m_pOsgViewer->addEventHandler(new osgViewer::RecordCameraPathHandler);

    m_pOsgViewer->addEventHandler(new osgViewer::LODScaleHandler);

    m_pOsgViewer->addEventHandler(new osgViewer::ScreenCaptureHandler);
    //m_pOsgViewer->addEventHandler(new PickEventHandler(&m_pCameraNodeVector));


    m_pOsgCamera = m_pOsgViewer->getCamera();
    m_pOsgCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    m_pOsgCamera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    m_pOsgCamera->setClearColor(osg::Vec4(15 / 255.f, 15 / 255.f, 15 / 255.f, 1.f)); //设置场景背景色

osg:DisplaySettings* ds = osg::DisplaySettings::instance();
    ds->setNumMultiSamples(16);
    m_pOsgViewer->setDisplaySettings(ds);



   
}

void OsgEngine::RenderTiles(const std::vector<ST_BOUNDINGBOX>& box)
{
    AddTileNodes(box);

}

void OsgEngine::PauseEngine(bool bPaused)
{
    this->bPaused = bPaused;
}
