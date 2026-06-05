
#include "PhotosNodeManager.h"
#include "OSGEditor/OsgEngine.h"
#include "EventManager.h"
#include "thread"
#include <future>
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif
PhotosNodeManager::PhotosNodeManager(OsgEngine *pOsgEngine, osg::ref_ptr<osgViewer::Viewer> pViewer): CustomNode(-1,"", Element_Type::ELEMENT_PHOTOS)
{
    m_fVirtualDepth = 2.0;//add by 
    m_pOsgEngine = pOsgEngine;
    m_pViewer = pViewer;
    m_pRootGeodeSwitch = new osg::Switch;
    addChild(m_pRootGeodeSwitch);

}

PhotosNodeManager::~PhotosNodeManager() = default;

void PhotosNodeManager::SetViewer(osg::ref_ptr<osgViewer::Viewer> pViewer)
{
    m_pViewer = pViewer;
}

void PhotosNodeManager::Add(const std::vector<ST_CAMERA_INFO>& vecCamera)
{       
    m_vecCameraCentrum.clear();
    m_vecCameraNoCentrum.clear();

    for (auto it : vecCamera)
    {
        m_pTotalCameraInfoMap.insert(make_pair(it.ID, it));
        InitCamera(it);
    }

    InitCentrumGeometry();
    InitNoCentrumGeometry();
}

void PhotosNodeManager::InitCamera(const ST_CAMERA_INFO& camera)
{

    const float image_width = 0.03f * camera.mSize;//image_size * camera.Width() / 1024.0f;
    int width = camera.Image_Width;
    int height = camera.Image_Height;
    const float image_height =
        image_width * static_cast<float>(height) / width;

    const float image_extent = std::max(image_width, image_height);
    const float camera_extent = std::max(width, height);
    const float camera_extent_world =
        static_cast<float>((camera_extent) / camera.FocalPixel);


    m_fVirtualDepth = image_extent / camera_extent_world * mScale;

    float Vfov = std::atan(camera.Image_Height * 0.5 / camera.FocalPixel);
    float Hfov = std::atan(camera.Image_Width * 0.5 / camera.FocalPixel);
    float LeftOffset = std::tan(Hfov) * m_fVirtualDepth;
    float UpOffset = std::tan(Vfov) * m_fVirtualDepth;
    CameraCentrum stCameraCentrum;
    bool bCentrum = !camera.mt.isIdentity();
    stCameraCentrum.bCentrum = bCentrum;
    stCameraCentrum.cameraInfo = camera;
    if (bCentrum)
    {
        osg::Matrixd mt = camera.mt;
        mt.preMultTranslate(-camera.Center);
        osg::Matrix matrix = osg::Matrixd::inverse(mt);
        stCameraCentrum.mt = matrix;

        osg::Vec3 cameraLocation(matrix.getTrans());

        osg::Vec3 eye, center, up;
        matrix.getLookAt(eye, center, up);

        osg::Vec3 front(eye - center);
        front.normalize();
        up.normalize();


        osg::Vec3 right(front ^ up);
        right.normalize();

        osg::Vec3 newCenter = cameraLocation + front * m_fVirtualDepth;

        osg::Vec3 leftBottom = newCenter - right * LeftOffset - up * UpOffset;
        osg::Vec3 rightBottom = newCenter + right * LeftOffset - up * UpOffset;
        osg::Vec3 rightTop = newCenter + right * LeftOffset + up * UpOffset;
        osg::Vec3 lefttop = newCenter - right * LeftOffset + up * UpOffset;

        stCameraCentrum.location = cameraLocation;
        stCameraCentrum.center = newCenter;
        stCameraCentrum.FrontPlane->push_back(leftBottom);
        stCameraCentrum.FrontPlane->push_back(rightBottom);
        stCameraCentrum.FrontPlane->push_back(rightTop);
        stCameraCentrum.FrontPlane->push_back(lefttop);

        stCameraCentrum.BackPlane->push_back(osg::Vec3(cameraLocation - right * LeftOffset*2 - up * UpOffset*2));
        stCameraCentrum.BackPlane->push_back(osg::Vec3(cameraLocation + right * LeftOffset*2 - up * UpOffset*2));
        stCameraCentrum.BackPlane->push_back(osg::Vec3(cameraLocation + right * LeftOffset*2 + up * UpOffset*2));
        stCameraCentrum.BackPlane->push_back(osg::Vec3(cameraLocation - right * LeftOffset*2 + up * UpOffset*2));

        m_vecCameraCentrum.push_back(stCameraCentrum);

    }
    else
    {
        stCameraCentrum.location = camera.Center;
        LeftOffset *= 2;
        UpOffset *= 2;
        stCameraCentrum.BackPlane->push_back(camera.Center + osg::Vec3(-LeftOffset, -UpOffset, 0));
        stCameraCentrum.BackPlane->push_back(camera.Center + osg::Vec3(LeftOffset, -UpOffset, 0));
        stCameraCentrum.BackPlane->push_back(camera.Center + osg::Vec3(LeftOffset, UpOffset, 0));
        stCameraCentrum.BackPlane->push_back(camera.Center + osg::Vec3(-LeftOffset, UpOffset, 0));

        m_vecCameraNoCentrum.push_back(stCameraCentrum);
    }
}

bool PhotosNodeManager::IsEmpty()
{

   return m_vecTotalPhotoGeometry.empty();
}
void PhotosNodeManager::InitCentrumGeometry()
{
    int num = 9;
    osg::ref_ptr<PhotoGeometry> pGeometry = nullptr;
    osg::ref_ptr<osg::Vec3Array> varr = nullptr;
    osg::ref_ptr<osg::Vec4Array> carr = nullptr;
    osg::DrawElementsUInt* planeIndexs = nullptr;
    osg::DrawElementsUInt* centrumIndexs = nullptr;
    std::vector< osg::ref_ptr<PhotoGeometry>> tmpGeometry;

    int indexOffset = 0;
    for (int i = 0 ; i < m_vecCameraCentrum.size(); i++)
    {
        CameraCentrum* pCameraCentrum = &m_vecCameraCentrum[i];
        
        osg::Vec4 planeColor, centrumColor;
        GetGeometryColor(MOUSE_TYPE::MOUSE_NONE,pCameraCentrum->cameraInfo.aerType, planeColor, centrumColor);

        if (i % 10 == 0)
        {
            pGeometry = new PhotoGeometry(m_pViewer);
            varr = new osg::Vec3Array;
            carr = new osg::Vec4Array;
            planeIndexs = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
            centrumIndexs = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, 0);

            pGeometry->setVertexArray(varr);
            pGeometry->setColorArray(carr);
            pGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
            pGeometry->addPrimitiveSet(planeIndexs);
            pGeometry->addPrimitiveSet(centrumIndexs);
            tmpGeometry.push_back(pGeometry);


            osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
            pGeode->addDrawable(pGeometry);            

            m_pRootGeodeSwitch->addChild(pGeode);

            m_vecTotalPhotoGeometry.push_back(pGeometry);

            indexOffset = 0;
        }

        pGeometry->SaveID(pCameraCentrum->cameraInfo.ID, indexOffset, pCameraCentrum);


        varr->push_back(pCameraCentrum->BackPlane->at(0));
        varr->push_back(pCameraCentrum->BackPlane->at(1));
        varr->push_back(pCameraCentrum->BackPlane->at(2));
        varr->push_back(pCameraCentrum->BackPlane->at(3));

        varr->push_back(pCameraCentrum->FrontPlane->at(0));
        varr->push_back(pCameraCentrum->FrontPlane->at(1));
        varr->push_back(pCameraCentrum->FrontPlane->at(2));
        varr->push_back(pCameraCentrum->FrontPlane->at(3));
        varr->push_back(pCameraCentrum->location);

        int vIndex = indexOffset * num;
        planeIndexs->push_back(vIndex + 0); planeIndexs->push_back(vIndex +  1); planeIndexs->push_back(vIndex +  2); planeIndexs->push_back(vIndex +  3);
        carr->push_back(planeColor); carr->push_back(planeColor); carr->push_back(planeColor); carr->push_back(planeColor);

        centrumIndexs->push_back(vIndex +  4); centrumIndexs->push_back(vIndex +  5); 
        centrumIndexs->push_back(vIndex +  8); centrumIndexs->push_back(vIndex +  5); 
        centrumIndexs->push_back(vIndex +  6); centrumIndexs->push_back(vIndex +  8);
        centrumIndexs->push_back(vIndex +  6); centrumIndexs->push_back(vIndex +  7); 
        centrumIndexs->push_back(vIndex +  8); centrumIndexs->push_back(vIndex +  7);
        centrumIndexs->push_back(vIndex +  4); centrumIndexs->push_back(vIndex +  8);
        centrumIndexs->push_back(vIndex + 5); centrumIndexs->push_back(vIndex + 6);
        centrumIndexs->push_back(vIndex + 4); centrumIndexs->push_back(vIndex + 7);

        carr->push_back(centrumColor); carr->push_back(centrumColor); carr->push_back(centrumColor); carr->push_back(centrumColor); carr->push_back(centrumColor);
        indexOffset++;
    }

    for (auto it : tmpGeometry)
    {
        it->Init();
        //it->computeBound();
    }
}

void PhotosNodeManager::InitNoCentrumGeometry()
{
    int num = 4;
    osg::ref_ptr<PhotoGeometry> pGeometry = nullptr;
    osg::ref_ptr<osg::Vec3Array> varr = nullptr;
    osg::ref_ptr<osg::Vec4Array> carr = nullptr;
    osg::DrawElementsUInt* planeIndexs = nullptr;
    osg::DrawElementsUInt* centrumIndexs = nullptr;
    std::vector< osg::ref_ptr<PhotoGeometry>> tmpGeometry;

    int indexOffset = 0;
    for (int i = 0; i < m_vecCameraNoCentrum.size(); i++)
    {
        CameraCentrum* pCameraCentrum = &m_vecCameraNoCentrum[i];

        if (i % 10 == 0)
        {
            pGeometry = new PhotoGeometry(m_pViewer, false);
            varr = new osg::Vec3Array;
            carr = new osg::Vec4Array;
            planeIndexs = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
            pGeometry->setVertexArray(varr);
            pGeometry->setColorArray(carr);
            pGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
            pGeometry->addPrimitiveSet(planeIndexs);
            tmpGeometry.push_back(pGeometry);

            osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
            pGeode->addDrawable(pGeometry);
            m_pRootGeodeSwitch->addChild(pGeode);
            m_vecTotalPhotoGeometry.push_back(pGeometry);
            indexOffset = 0;
        }

        pGeometry->SaveID(pCameraCentrum->cameraInfo.ID, indexOffset, pCameraCentrum);

        osg::Vec4 planeColor, centrumColor;
        GetGeometryColor(MOUSE_TYPE::MOUSE_NONE,pCameraCentrum->cameraInfo.aerType, planeColor, centrumColor);

        //planeColor.set(1, 0, 0, 1);
        varr->push_back(pCameraCentrum->BackPlane->at(0));
        varr->push_back(pCameraCentrum->BackPlane->at(1));
        varr->push_back(pCameraCentrum->BackPlane->at(2));
        varr->push_back(pCameraCentrum->BackPlane->at(3));

        int vIndex = indexOffset * num;
        planeIndexs->push_back(vIndex + 0); planeIndexs->push_back(vIndex + 1); planeIndexs->push_back(vIndex + 2); planeIndexs->push_back(vIndex + 3);
        carr->push_back(planeColor); carr->push_back(planeColor); carr->push_back(planeColor); carr->push_back(planeColor);
        indexOffset++;
    }

    for (auto it : tmpGeometry)
    {
        it->Init();
    }
}

void PhotosNodeManager::ScaleCentrumGeometry()
{
    int num = 9;
    int indexOffset = 0;
    int tmpNum = 0;
    osg::ref_ptr<PhotoGeometry> pGeometry = nullptr;
    osg::ref_ptr<osg::Vec3Array> varr = nullptr;
    for (int i = 0; i < m_vecCameraCentrum.size(); i++)
    {
        CameraCentrum* pCameraCentrum = &m_vecCameraCentrum[i];
        if (i % 10 == 0)
        {
            pGeometry = m_vecTotalPhotoGeometry.at(tmpNum);
            varr = dynamic_cast<Vec3Array*> (pGeometry->getVertexArray());
            varr->clear();
            tmpNum++;
        }

        varr->push_back(pCameraCentrum->BackPlane->at(0));
        varr->push_back(pCameraCentrum->BackPlane->at(1));
        varr->push_back(pCameraCentrum->BackPlane->at(2));
        varr->push_back(pCameraCentrum->BackPlane->at(3));

        varr->push_back(pCameraCentrum->FrontPlane->at(0));
        varr->push_back(pCameraCentrum->FrontPlane->at(1));
        varr->push_back(pCameraCentrum->FrontPlane->at(2));
        varr->push_back(pCameraCentrum->FrontPlane->at(3));
        varr->push_back(pCameraCentrum->location);
    }

    for (int i = 0; i < m_vecCameraNoCentrum.size(); i++)
    {
        CameraCentrum* pCameraCentrum = &m_vecCameraNoCentrum[i];

        if (i % 10 == 0)
        {
            pGeometry = m_vecTotalPhotoGeometry.at(tmpNum);
            varr = dynamic_cast<Vec3Array*> (pGeometry->getVertexArray());
            varr->clear();
            tmpNum++;
        }

        varr->push_back(pCameraCentrum->BackPlane->at(0));
        varr->push_back(pCameraCentrum->BackPlane->at(1));
        varr->push_back(pCameraCentrum->BackPlane->at(2));
        varr->push_back(pCameraCentrum->BackPlane->at(3));
    }

    for (auto it : m_vecTotalPhotoGeometry)
    {
        it->dirtyDisplayList();
    }

}


void PhotosNodeManager::Reset()
{
    if (m_eMouseType == MOUSE_TYPE::MOUSE_NONE)
    {
        return;
    }

    for(auto it : m_vecSelectedPhotoGeometry)
    {
        PhotoGeometry* pPhotoGeometry = dynamic_cast<PhotoGeometry*>(it.get());
        if (pPhotoGeometry)
        {
            pPhotoGeometry->Reset();
        }
    }

    if (m_pPickedCentrumGeode)
    {
        this->removeChild(m_pPickedCentrumGeode);
        m_pPickedCentrumGeode = nullptr;
    }
    
    m_vecSelectedPhotoGeometry.clear();
    this->m_iID = -1;
    this->m_strName = "";

    m_eMouseType = MOUSE_TYPE::MOUSE_NONE;
}

void PhotosNodeManager::Reset(const std::vector<int>& vecPhotoID)
{
    for (auto it : m_vecTotalPhotoGeometry)
    {
        it->Reset(vecPhotoID);
    }

    m_eMouseType = MOUSE_TYPE::MOUSE_NONE;
}

void PhotosNodeManager::Hover()
{
    if (m_eMouseType != MOUSE_TYPE::MOUSE_NONE)
    {
        return;
    }

    if (m_pSelectGeometry)
    {
        PhotoGeometry* pPhotoGeometry = dynamic_cast<PhotoGeometry*>(m_pSelectGeometry.get());

        this->m_strName = pPhotoGeometry->Hover(m_PickedPoint);

        m_vecSelectedPhotoGeometry.push_back(pPhotoGeometry);
    }

    m_eMouseType = MOUSE_TYPE::MOUSE_HOVER;
}

void PhotosNodeManager::Picked(const SELECT_TYPE& type)
{
    if (m_eMouseType == MOUSE_TYPE::MOUSE_PICKED)
    {
        return;
    }

    if (m_pSelectGeometry)
    {
        PhotoGeometry* pPhotoGeometry = dynamic_cast<PhotoGeometry*>(m_pSelectGeometry.get());
        if (pPhotoGeometry == nullptr)
        {
            return;
        }
        std::pair<int&, string&> info(this->m_iID, this->m_strName);
        m_pPickedCentrumGeode = pPhotoGeometry->Picked(m_PickedPoint, info, type);
        if (m_pPickedCentrumGeode)
        {
            addChild(m_pPickedCentrumGeode);
        }
        pPhotoGeometry->Update();
        m_vecSelectedPhotoGeometry.push_back(pPhotoGeometry);
    }

    m_eMouseType = MOUSE_TYPE::MOUSE_PICKED;
}


void PhotosNodeManager::Picked(const std::vector<int>& vecPhotoID)
{
    if (m_eMouseType == MOUSE_TYPE::MOUSE_PICKED)
    {
        return;
    }

    for (auto it : m_vecTotalPhotoGeometry)
    {
        for (auto id : vecPhotoID)
        {
            if (it->IsExist(id))
            {
                m_pPickedCentrumGeode = it->Picked(id, m_pOsgEngine->GetCurrentSelectType());
                if (m_pPickedCentrumGeode)
                {
                    addChild(m_pPickedCentrumGeode);
                }
            }
        }

    }

    for (auto it : m_vecTotalPhotoGeometry)
    {
        if (it->IsSelected())
        {
            it->Update();
            m_vecSelectedPhotoGeometry.push_back(it);
        }
    }

    m_eMouseType = MOUSE_TYPE::MOUSE_PICKED;

}
//
//void PhotosNodeManager::RemoveChild()
//{
//    std::vector<osg::ref_ptr<PhotoGeometry>> tmpDeleteGeometry;
//    for (auto it : m_vecSelectedPhotoGeometry)
//    {
//        osg::ref_ptr<PhotoGeometry> pPhotoGeometry = dynamic_cast<PhotoGeometry*>(it.get());
//        pPhotoGeometry->RemoveChild();
//        if (pPhotoGeometry->IsDelete())
//        {
//            tmpDeleteGeometry.push_back(pPhotoGeometry);             
//        }
//    }
//
//    m_vecSelectedPhotoGeometry.clear();
//
//    if (m_pPickedCentrumGeode)
//    {
//        this->removeChild(m_pPickedCentrumGeode);
//        m_pPickedCentrumGeode = nullptr;
//    }
//
//    m_eMouseType = MOUSE_TYPE::MOUSE_NONE;
//
//    //删除节点
//    for (auto it : tmpDeleteGeometry)
//    {                               
//        std::vector<int>* tmpPhototsID = it->GetPhotosID();
//        for (auto id : *tmpPhototsID)
//        {
//            m_pTotalCameraInfoMap.erase(id);
//        }
//        auto itt = std::find(m_vecTotalPhotoGeometry.begin(), m_vecTotalPhotoGeometry.end(), it);
//        m_vecTotalPhotoGeometry.erase(itt);
//
//        m_pRootGeodeSwitch->removeChild(it);
//        
//    }
//}

void PhotosNodeManager::RemoveChild(const int& id)
{
    if (id == -1)

    {
        std::vector<osg::ref_ptr<PhotoGeometry>> tmpDeleteGeometry;
        for (auto it : m_vecSelectedPhotoGeometry)

        {
            osg::ref_ptr<PhotoGeometry> pPhotoGeometry = dynamic_cast<PhotoGeometry*>(it.get());
            pPhotoGeometry->RemoveChild();
            if (pPhotoGeometry->IsDelete())
            {
                tmpDeleteGeometry.push_back(pPhotoGeometry);
            }
        }
        m_vecSelectedPhotoGeometry.clear();
        if (m_pPickedCentrumGeode)
        {
            this->removeChild(m_pPickedCentrumGeode);
            m_pPickedCentrumGeode = nullptr;
        }


        m_eMouseType = MOUSE_TYPE::MOUSE_NONE;

        //删除节点
        for (auto it : tmpDeleteGeometry)
        {
            std::vector<int>* tmpPhototsID = it->GetPhotosID();
            for (auto id : *tmpPhototsID)
            {
                m_pTotalCameraInfoMap.erase(id);
            }
            auto itt = std::find(m_vecTotalPhotoGeometry.begin(), m_vecTotalPhotoGeometry.end(), it);
            m_vecTotalPhotoGeometry.erase(itt);

            m_pRootGeodeSwitch->removeChild(it);

        }
    }
    else
    {
        for (auto it : m_vecTotalPhotoGeometry)
        {
            if (it->IsExist(id))
            {
                it->RemoveChild(id);
                m_pTotalCameraInfoMap.erase(id);
            }
        }
    }
}
void PhotosNodeManager::Clear()
{
    if (m_pPickedCentrumGeode)
    {
        this->removeChild(m_pPickedCentrumGeode);
        m_pPickedCentrumGeode = nullptr;
    }
    m_pRootGeodeSwitch->removeChildren(0, m_pRootGeodeSwitch->getNumChildren());

    m_vecCameraCentrum.clear();
    m_vecCameraNoCentrum.clear();                    
    m_vecSelectedPhotoGeometry.clear();
    m_vecTotalPhotoGeometry.clear();
    m_pTotalCameraInfoMap.clear();

}
void PhotosNodeManager::RemoveAll()
{
    Clear();
}

void PhotosNodeManager::RemoveSelectedPhoto()
{
    m_vecSelectedPhotoGeometry.clear();

    if (m_pPickedCentrumGeode)
    {
        ///  
        this->removeChild(m_pPickedCentrumGeode);

        m_pPickedCentrumGeode = nullptr;
    }

    m_pSelectGeometry = nullptr;

    m_eMouseType = MOUSE_TYPE::MOUSE_NONE;
}

void PhotosNodeManager::Scale(float value)
{
    //add by  following two lines
    mScale *= value;   
    mScale = std::max(1e-1f, std::min(1e6f, mScale));
   // m_fVirtualDepth *= value; //commented by 
    m_vecCameraCentrum.clear();
    m_vecCameraNoCentrum.clear();

    for (auto it : m_pTotalCameraInfoMap)
    {
        InitCamera(it.second);
    }
    ScaleCentrumGeometry();
}

void PhotosNodeManager::Visible(bool value)
{
    if (value)
    {
        m_pRootGeodeSwitch->setAllChildrenOn();
    }
    else
    {
        m_pRootGeodeSwitch->setAllChildrenOff();

    }
}

//矩形框选
bool PhotosNodeManager::BoxSelect(osg::ref_ptr<osgViewer::Viewer> pViewer, const osg::Vec3& minXY, const osg::Vec3& maxXY)
{
    for (auto it : m_vecTotalPhotoGeometry)
    {
        it->BoxSelect(minXY, maxXY);

        if (it->IsSelected())
        {
            it->Update();
            m_vecSelectedPhotoGeometry.push_back(it);
        }
    }

   if (m_vecSelectedPhotoGeometry.size() > 0)
   {
       m_eMouseType = MOUSE_TYPE::MOUSE_PICKED;
       EventCallBack();
       return true;
   }

    return false;
}

//多边形框选
bool PhotosNodeManager::PolygonSelect(osg::ref_ptr<osgViewer::Viewer> pViewer, const std::vector<osg::Vec3>& vecPoints)
{

    for (auto it : m_vecTotalPhotoGeometry)
    {
        it->PolygonSelect(vecPoints);

        if (it->IsSelected())
        {
            it->Update();
            m_vecSelectedPhotoGeometry.push_back(it);
        }
    }

    if (m_vecSelectedPhotoGeometry.size() > 0)
    {
        m_eMouseType = MOUSE_TYPE::MOUSE_PICKED;
        EventCallBack();
        return true;
    }


    return true;
}


void PhotosNodeManager::GetPickedPhotosID(std::vector<int>* vecPhotosID)
{
    for (auto it : m_vecSelectedPhotoGeometry)
    {
        vecPhotosID->insert(vecPhotosID->end(),it->GetPickedID()->begin(), it->GetPickedID()->end());
    }

    return ;
}

void PhotosNodeManager::EventCallBack()
{
    std::vector<ST_CALLBACK_ELEMENT_INFO> vecCallback;
    for (auto it : m_vecSelectedPhotoGeometry)
    {
        for (auto id : *it->GetPickedID())
        {
            vecCallback.push_back({id,""});
        }

    }

    OsgEngine* pOsgEngine = m_pOsgEngine;
    async(launch::async, [&vecCallback, pOsgEngine]() { EventManager::GetInstance()->notifyEvent({ CALL_BACK_SELECT_PHOTO, &vecCallback }, pOsgEngine);

        });
}
