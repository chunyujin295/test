#include "OSGEditor/PointNode.h"
#include "osg/DrawPixels"
#include "OSGEditor/Unitl.h"
#include "OSGEditor/OsgEngine.h"
#include "OSGEditor/EventManager.h"
#include <algorithm>
#include <future>
#include <thread>
#include <unordered_set>
#include <vector>

namespace
{
    constexpr size_t kParallelPhotoThreshold = 256;
    constexpr size_t kPhotosPerWorker = 1000;

    std::vector<int> CollectPointIndicesForPhotos(
        const ST_TIEPOINT& tiepoint,
        const std::vector<int>& photos,
        size_t begin,
        size_t end)
    {
        std::vector<int> indices;
        std::unordered_set<int> seenPointIds;
        indices.reserve(256);

        for (size_t i = begin; i < end; ++i)
        {
            const int photoID = photos[i];
            const auto photoIt = tiepoint.PhotoRelevancyID.find(photoID);
            if (photoIt == tiepoint.PhotoRelevancyID.end())
            {
                continue;
            }

            for (const int pointID : photoIt->second)
            {
                if (!seenPointIds.insert(pointID).second)
                {
                    continue;
                }

                const auto indexIt = tiepoint.PointIDRelevancyIndex.find(pointID);
                if (indexIt == tiepoint.PointIDRelevancyIndex.end())
                {
                    continue;
                }

                indices.push_back(indexIt->second);
            }
        }

        return indices;
    }
}

osg::Vec3 WorldToScreen(osgViewer::Viewer* viewer, osg::Vec3& world)
{
    osg::Camera* camera = viewer->getCamera();
    osg::Matrix viewMat = camera->getViewMatrix(); //获取当前视图矩阵
    osg::Matrix projMat = camera->getProjectionMatrix();//获取投影矩阵
    osg::Matrix windMat = camera->getViewport()->computeWindowMatrix();//获取窗口矩阵
    osg::Matrix MVPW = viewMat * projMat * windMat;

    osg::Vec3 mouseWorld = world * MVPW;

    return mouseWorld;
}

PointNode::PointNode(const ST_TIEPOINT& tiepoint,OsgEngine* pOsgEngine) :m_stTiePoint(tiepoint), CustomNode(tiepoint.ID, tiepoint.name, ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS)
    ,m_pOsgEngine(pOsgEngine)
{
    setCullingActive(false);
}

void PointNode::Init()
{
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();
    geometry->setVertexArray(m_stTiePoint.points);
    /// note:the following line has ever crashed.(Oct 9,11:39 p.m.)
    geometry->setColorArray(osg::clone(m_stTiePoint.colors.get()));
    geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);


    if (m_stTiePoint.points.get())
    {
        geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, m_stTiePoint.points->size()));
    }

    osg::ref_ptr<osg::Point> pointSize = new osg::Point();
    pointSize->setSize(m_stTiePoint.size);
    geometry->getOrCreateStateSet()->setAttributeAndModes(pointSize, osg::StateAttribute::ON);
    geometry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    geometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

    osg::ref_ptr<osg::Geode> geode = new osg::Geode();
    geode->addDrawable(geometry.get());

    m_pGeometry = geometry;

    addChild(geode);

    //osg::ComputeBoundsVisitor cbVisitor;
    //m_pGeometry->accept(cbVisitor);
    //m_stBox = cbVisitor.getBoundingBox();

    setCullingActive(true);
}

void PointNode::Picked(const uint32_t& photoID)
{
    m_mapTmpPointID.clear();
    if (m_stTiePoint.PhotoRelevancyID.find(photoID) != m_stTiePoint.PhotoRelevancyID.end())
    {
        auto vecPointsID = m_stTiePoint.PhotoRelevancyID.find(photoID)->second;
        int pointIndex = 0;
        osg::Vec4 color = Unitl::FromHex(EditerEngine::TiePointColor[1]);
        osg::ref_ptr<osg::Vec4Array> pColor = dynamic_cast<osg::Vec4Array*>(m_pGeometry->getColorArray());
        if (pColor == nullptr)
        {
            return;
        }
        for (auto it : vecPointsID)
        {
            if (m_mapTmpPointID.find(it) != m_mapTmpPointID.end())
            {
                continue;;
            }

            pointIndex = m_stTiePoint.PointIDRelevancyIndex.find(it) != m_stTiePoint.PointIDRelevancyIndex.end() ? m_stTiePoint.PointIDRelevancyIndex.find(it)->second : -1;
            if (pointIndex != -1)
            {
                (*pColor)[pointIndex] = color;
                m_vecPointIndex.push_back(pointIndex);
                m_mapTmpPointID.insert(make_pair(it,true));
            }
        }

        m_pGeometry->dirtyDisplayList();
        m_eMouseType = MOUSE_TYPE::MOUSE_PICKED;

    }
    
}

void PointNode::Picked(const std::vector<int>& photos)
{
    if (photos.empty())
    {
        return;
    }
    m_vecPointIndex.clear();
    m_mapTmpPointID.clear();

    osg::ref_ptr<osg::Vec4Array> pColor = dynamic_cast<osg::Vec4Array*>(m_pGeometry->getColorArray());
    if (pColor == nullptr)
    {
        return;
    }

    std::vector<int> pointIndices;
    const ST_TIEPOINT& tiepoint = m_stTiePoint;

    if (photos.size() < kParallelPhotoThreshold)
    {
        pointIndices = CollectPointIndicesForPhotos(tiepoint, photos, 0, photos.size());
    }
    else
    {
        const size_t workerCount = std::max<size_t>(
            1,
            std::min(
                static_cast<size_t>(std::thread::hardware_concurrency()),
                (photos.size() + kPhotosPerWorker - 1) / kPhotosPerWorker));

        const size_t chunkSize = (photos.size() + workerCount - 1) / workerCount;
        std::vector<std::future<std::vector<int>>> futures;
        futures.reserve(workerCount);

        for (size_t worker = 0; worker < workerCount; ++worker)
        {
            const size_t begin = worker * chunkSize;
            if (begin >= photos.size())
            {
                break;
            }
            const size_t end = std::min(begin + chunkSize, photos.size());

            futures.push_back(std::async(
                std::launch::async,
                [&tiepoint, &photos, begin, end]() {
                    return CollectPointIndicesForPhotos(tiepoint, photos, begin, end);
                }));
        }

        std::unordered_set<int> seenPointIndices;
        seenPointIndices.reserve(photos.size());
        for (auto& future : futures)
        {
            for (const int pointIndex : future.get())
            {
                if (seenPointIndices.insert(pointIndex).second)
                {
                    pointIndices.push_back(pointIndex);
                }
            }
        }
    }

    const osg::Vec4 color = Unitl::FromHex(EditerEngine::TiePointColor[1]);
    for (const int pointIndex : pointIndices)
    {
        (*pColor)[pointIndex] = color;
    }

    m_vecPointIndex = std::move(pointIndices);
    m_pGeometry->dirtyDisplayList();
    m_eMouseType = MOUSE_TYPE::MOUSE_PICKED;

}

void PointNode::Delete(const int& photoID)
{
    if (m_stTiePoint.PhotoRelevancyID.find(photoID) != m_stTiePoint.PhotoRelevancyID.end())
    {
        int pointIndex = 0;
        auto vecPointsID = m_stTiePoint.PhotoRelevancyID.find(photoID)->second;
        osg::Vec4Array* pColor = dynamic_cast<osg::Vec4Array*>(m_pGeometry->getColorArray());
        if (pColor == nullptr)
        {
            return;
        }
        Vec3Array* pArray = dynamic_cast<Vec3Array*>(m_pGeometry->getVertexArray());
        std::vector<int> tmpVector;
       
        for (auto it : vecPointsID)
        {
            pointIndex = m_stTiePoint.PointIDRelevancyIndex.find(it) != m_stTiePoint.PointIDRelevancyIndex.end() ? m_stTiePoint.PointIDRelevancyIndex.find(it)->second : -1;
            if (pointIndex != -1)
            {
                (*pColor)[pointIndex].set(0, 0, 0, 0);
                pArray->at(pointIndex).set(0, 0, 0);

                m_stTiePoint.PointIDRelevancyIndex.erase(it);
            }
        }

      

        m_pGeometry->dirtyDisplayList();
        m_stTiePoint.PhotoRelevancyID.erase(photoID);
    }
    else if (photoID == -1 && m_vecPointIndex.size() > 0)  //删除选中点
    {
        osg::ref_ptr<osg::Vec4Array> pColor = dynamic_cast<osg::Vec4Array*>(m_pGeometry->getColorArray());
        if (pColor == nullptr)
        {
            return;
        }
        osg::ref_ptr<Vec3Array> pArray = dynamic_cast<Vec3Array*>(m_pGeometry->getVertexArray());

        for (unsigned i = 0; i < m_vecPointIndex.size(); i++)
        {
            int pointIndex = m_vecPointIndex.at(i);
            (*pColor)[pointIndex].set(0, 0, 0, 0);
            pArray->at(pointIndex).set(0, 0, 0);
        }
        m_pGeometry->dirtyDisplayList();

       // m_vecPointIndex.clear();

        for (auto it : m_vecTiePointLineNode)
        {
            removeChild(it);
        }

        m_vecTiePointLineNode.clear();
        m_eMouseType = MOUSE_TYPE::MOUSE_NONE;
    }

    m_mapTmpPointID.clear();
}


void PointNode::Delete(const std::vector<int>& photoIDs)
{
    osg::Vec4Array* pColor = dynamic_cast<osg::Vec4Array*>(m_pGeometry->getColorArray());
    Vec3Array* pArray = dynamic_cast<Vec3Array*>(m_pGeometry->getVertexArray());

    for (auto photoID : photoIDs)
    {
        auto pointsID = m_stTiePoint.PhotoRelevancyID.find(photoID);
        if (pointsID == m_stTiePoint.PhotoRelevancyID.end())
        {
            continue;
        }

        int pointIndex = 0;
        auto vecPointsID = pointsID->second;
        if (pColor == nullptr)
        {
            return;
        }

        std::vector<int> tmpVector;
        for (auto it : vecPointsID)
        {
            pointIndex = m_stTiePoint.PointIDRelevancyIndex.find(it) != m_stTiePoint.PointIDRelevancyIndex.end() ? m_stTiePoint.PointIDRelevancyIndex.find(it)->second : -1;
            if (pointIndex != -1)
            {
                (*pColor)[pointIndex].set(0, 0, 0, 0);
                pArray->at(pointIndex).set(0, 0, 0);

                m_stTiePoint.PointIDRelevancyIndex.erase(it);
            }
        }



        m_stTiePoint.PhotoRelevancyID.erase(photoID);
    }


    m_pGeometry->dirtyDisplayList();

}

void PointNode::UpdateGeometry()
{
    m_pGeometry->dirtyDisplayList();
}

void PointNode::Reset()
{
    if (m_eMouseType == MOUSE_TYPE::MOUSE_NONE)
    {
        return;
    }

    if (m_eMouseType == MOUSE_HOVER)
    {
        if (m_pHoveNode)
        {
            removeChild(m_pHoveNode);
        }
    }
    else if (m_eMouseType == MOUSE_PICKED)
    {
        osg::ref_ptr<osg::Vec4Array> pColor = dynamic_cast<osg::Vec4Array*>(m_pGeometry->getColorArray());
        if (pColor == nullptr)
        {
            return;
        }
        for (auto it : m_vecPointIndex)
        {
            (*pColor)[it] = (*m_stTiePoint.colors)[it];
        }
        m_pGeometry->dirtyDisplayList();
       // m_vecPointIndex.clear();

        for (auto it : m_vecTiePointLineNode)
        {
            removeChild(it);
        }

        m_vecTiePointLineNode.clear();
        m_mapTmpPointID.clear();
    }
m_vecPointIndex.clear();
    m_eMouseType = MOUSE_TYPE::MOUSE_NONE;
}

bool PointNode::Hover(osg::ref_ptr<osgViewer::Viewer> pViewer, const float& x, const float& y, bool status)
{

    if (m_pHoveNode || !status)
    {
        removeChild(m_pHoveNode);
        m_pHoveNode = nullptr;

        m_eMouseType = m_eMouseType == MOUSE_TYPE::MOUSE_HOVER ? MOUSE_TYPE::MOUSE_NONE : m_eMouseType;

        return true;
    }

    if (m_eMouseType != MOUSE_TYPE::MOUSE_NONE || (x == 0 && y == 0))
    {
        return false;
    }

    osgUtil::SceneView* pSceneView = dynamic_cast<osgViewer::Renderer*>(pViewer->getCamera()->getRenderer())->getSceneView(0);
    if (!pSceneView)
    {
        return false;
    }

    osg::Vec3 minXY, maxXY;

    //hover边框范围
    if (!m_pGeometry)
        return false;
    // has ever crashed.
    osg::ref_ptr<Vec3Array> pArray = dynamic_cast<Vec3Array*>(m_pGeometry->getVertexArray());
    minXY.set(x - 5, y - 5, 0);
    maxXY.set(x + 5, y + 5, 0);

    osg::Vec3 windows;
    std::map<float, std::pair<int, osg::Vec3>> tmpMap;
    osg::Vec3 cameraLocation = osg::Matrix::inverse(pViewer->getCamera()->getViewMatrix()).getTrans();
    //点是否在拾取范围内1
    if (pArray == nullptr)
        return false;
    // has every crashed at 12:06 p.m. On Oct 9,2023 (pArray is null)
    for (unsigned i = 0; i < pArray->size(); i++)
    {
        pSceneView->projectObjectIntoWindow(pArray->at(i), windows);
         if (Unitl::IsPointInsideBoundingBox(windows, minXY, maxXY) && pArray->at(i).length() != 0)
        { 
            float dis = (cameraLocation - pArray->at(i)).length();
            tmpMap.insert(make_pair(dis, make_pair(i, pArray->at(i))));
        }
    }

    if (tmpMap.size() == 0)
    {
        return false;
    }

    //hover框
    osg::ref_ptr<osg::Point> point = dynamic_cast<osg::Point*>(m_pGeometry->getOrCreateStateSet()->getAttribute(StateAttribute::Type::POINT));
    float size = point->getSize() * 0.5;
    osg::Vec4 color = Unitl::FromHex(EditerEngine::TiePointColor[0]);
    osg::BoundingBox box(osg::Vec3(-size, -size, -size), osg::Vec3(size, size, size));

    osg::ref_ptr<osg::Node> pNode = Unitl::CreateBoxBorder(&box, color);

    osg::AutoTransform* pAt = new osg::AutoTransform();
    pAt->setAutoScaleToScreen(true);
    pAt->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
    //pAt->setMinimumScale(1.0);
    //pAt->setMaximumScale(2.0);
    pAt->addChild(pNode);

    m_pHoveNode = new osg::PositionAttitudeTransform();
    m_pHoveNode->setPosition(tmpMap.begin()->second.second);
    m_pHoveNode->addChild(pAt);

    if (m_pHoveNode)
    {
        addChild(m_pHoveNode);
    }

    m_eMouseType = MOUSE_TYPE::MOUSE_HOVER;

    return true;
}


bool PointNode::Picked(osg::ref_ptr<osgViewer::Viewer> pViewer, const float& x, const float& y)
{

    if (m_eMouseType == MOUSE_TYPE::MOUSE_PICKED)
    {
        return false;
    }

    osgUtil::SceneView* pSceneView = dynamic_cast<osgViewer::Renderer*>(pViewer->getCamera()->getRenderer())->getSceneView(0);
    if (!pSceneView)
    {
        return false;
    }

    osg::Vec3 minXY, maxXY;
    osg::ref_ptr<Vec3Array> pArray = dynamic_cast<Vec3Array*>(m_pGeometry->getVertexArray());
    minXY.set(x - 5, y - 5, 0);
    maxXY.set(x + 5, y + 5, 0);

    m_vecPointIndex.clear();
    m_vecPoint.clear();

    osg::Vec3 windows;
    std::map<float, std::pair<int, osg::Vec3>> tmpMap;
    osg::Vec3 cameraLocation = osg::Matrix::inverse(pViewer->getCamera()->getViewMatrix()).getTrans();
    //点是否在拾取范围内，有可能拾取到重叠多个点
    for (unsigned i = 0; i < pArray->size(); i++)
    {
        pSceneView->projectObjectIntoWindow(pArray->at(i), windows);
        if (Unitl::IsPointInsideBoundingBox(windows, minXY, maxXY)&& pArray->at(i).length() != 0)
        {
            float dis = (cameraLocation - pArray->at(i)).length();
            tmpMap.insert(make_pair(dis, make_pair(i, pArray->at(i))));
        }
    }

    if (tmpMap.size() == 0)
    {
        return false;
    }

    m_vecPointIndex.push_back(tmpMap.begin()->second.first);   //拾取点的索引
    m_vecPoint.push_back(tmpMap.begin()->second.second);        //保存拾取到的点


    m_eMouseType = MOUSE_TYPE::MOUSE_PICKED;

    

    //点到相片连接线
    {
        //改变点的颜色
        int pointIndex = tmpMap.begin()->second.first;
        auto nodes = m_stTiePoint.IDRelevancyPhoto.at(pointIndex).second;
        for (auto it : m_stTiePoint.IDRelevancyPhoto.at(pointIndex).second) //获取点对应的相片
        {
            osg::ref_ptr<osg::Vec3Array> pArray = new osg::Vec3Array;
            pArray->push_back(it);
            pArray->push_back(tmpMap.begin()->second.second); //距离相机最近点
            osg::ref_ptr<osg::Node> pTiePointLineNode = Unitl::CreateLineGeometry(pArray, Unitl::FromHex(EditerEngine::TiePointColor[3]));
           
            if (pTiePointLineNode)
            {
                addChild(pTiePointLineNode);
                m_vecTiePointLineNode.push_back(pTiePointLineNode);
            }
                                                       
        }

        osg::ref_ptr<osg::Vec4Array> pColor = dynamic_cast<osg::Vec4Array*>(m_pGeometry->getColorArray());
        if (pColor == nullptr)
        {
            return false;
        }
        (*pColor)[pointIndex] = Unitl::FromHex(EditerEngine::TiePointColor[1]);
        m_pGeometry->dirtyGLObjects();


        //拾取事件回调
        std::vector<ST_CALLBACK_ELEMENT_INFO> vecCallback;
        ST_CALLBACK_ELEMENT_INFO callBackInfo;
        callBackInfo.ID = m_stTiePoint.IDRelevancyPhoto.at(pointIndex).first;
        vecCallback.push_back(callBackInfo); EventManager::GetInstance()->notifyEvent({ CALL_BACK_TIEPOINT, &vecCallback },m_pOsgEngine);                                                                               
    }

    return true;
}

bool PointNode::BoxSelect(osg::ref_ptr<osgViewer::Viewer> pViewer, const osg::Vec3& minXY, const osg::Vec3& maxXY)
{
    osg::Vec3 windows;
    osgUtil::SceneView* pSceneView = dynamic_cast<osgViewer::Renderer*>(pViewer->getCamera()->getRenderer())->getSceneView(0);
    if (!pSceneView)
    {
        return false;
    }
    m_vecPoint.clear();
    osg::ref_ptr<Vec3Array> pArray = dynamic_cast<Vec3Array*>(m_pGeometry->getVertexArray());
    osg::ref_ptr<osg::Vec4Array> pColor = dynamic_cast<osg::Vec4Array*>(m_pGeometry->getColorArray());
    if (pColor == nullptr)
    {
        return false;
    }
    for (unsigned i = 0; i < pArray->size(); i++)
    {
        pSceneView->projectObjectIntoWindow(pArray->at(i), windows);
         if (Unitl::IsPointInsideBoundingBox(windows, minXY, maxXY) && pArray->at(i).length() != 0)
        {
            m_vecPointIndex.push_back(i);
            m_vecPoint.push_back(pArray->at(i));
            
            (*pColor)[i] = Unitl::FromHex(EditerEngine::TiePointColor[2]);
            
        }
    }

    if (m_vecPointIndex.size() == 0)
    {
        return false;
    }
    m_pGeometry->dirtyDisplayList();
    m_eMouseType = MOUSE_PICKED;

     //事件通知、联动
    std::vector<ST_CALLBACK_ELEMENT_INFO> vecCallback;
    for (auto it : m_vecPointIndex)
    {
        vecCallback.push_back({ m_stTiePoint.IDRelevancyPhoto.at(it).first , "" });
    } EventManager::GetInstance()->notifyEvent({ CALL_BACK_TIEPOINT, &vecCallback },m_pOsgEngine);                        
    return true;
}

void PointNode::ScaleChild(float value)
{
    
    osg::Point* point = dynamic_cast<osg::Point*>(m_pGeometry->getOrCreateStateSet()->getAttribute(StateAttribute::Type::POINT));
    float size = point->getSize() * value;
    size = size < RENDERPIINT_MINSIZE ? RENDERPIINT_MINSIZE : size;
    point->setSize(size);

    m_pGeometry->dirtyGLObjects();
}

bool  PointNode::PolygonSelect(osg::ref_ptr<osgViewer::Viewer> pViewer, const std::vector<osg::Vec3>& vecPoints)
{
    osg::Vec3 windows;
    osgUtil::SceneView* pSceneView = dynamic_cast<osgViewer::Renderer*>(pViewer->getCamera()->getRenderer())->getSceneView(0);
    if (!pSceneView)
    {
        return false;
    }
    m_vecPoint.clear();

    //点是否在多边形范围内
    osg::ref_ptr<Vec3Array> pArray = dynamic_cast<Vec3Array*>(m_pGeometry->getVertexArray());
    osg::ref_ptr<osg::Vec4Array> pColor = dynamic_cast<osg::Vec4Array*>(m_pGeometry->getColorArray());
    if (pColor == nullptr)
    {
        return false;
    }
    for (unsigned i = 0; i < pArray->size(); i++)
    {
        pSceneView->projectObjectIntoWindow(pArray->at(i), windows);

        if (Unitl::IsPointInPolygon(windows, vecPoints) && pArray->at(i).length() != 0)
        {
            m_vecPointIndex.push_back(i);              //保存拾取点的索引
            m_vecPoint.push_back(pArray->at(i));       //保存拾取到的点

            (*pColor)[i] = Unitl::FromHex(EditerEngine::TiePointColor[2]);   //修改拾取点颜色
        }
    }

    if (m_vecPointIndex.size() == 0)
    {
        return false;
    }

    m_pGeometry->dirtyDisplayList();
    m_eMouseType = MOUSE_PICKED;



    //事件通知、联动
    std::vector<ST_CALLBACK_ELEMENT_INFO> vecCallback;
    for (auto it : m_vecPointIndex)
    {
        vecCallback.push_back({ m_stTiePoint.IDRelevancyPhoto.at(it).first , ""});
    } EventManager::GetInstance()->notifyEvent({ CALL_BACK_TIEPOINT, &vecCallback },m_pOsgEngine);                        
    return true;
}
void PointNode::GetSelectedPointID(std::vector<int>& pointIDs)
{
    for (auto it : m_vecPointIndex)
    {
        pointIDs.push_back(m_stTiePoint.IDRelevancyPhoto.at(it).first);
    }
}															  
