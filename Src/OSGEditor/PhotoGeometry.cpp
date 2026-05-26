#include "PhotoGeometry.h"

PhotoGeometry::PhotoGeometry(osg::ref_ptr<osgViewer::Viewer> pViewer, bool centrum)
{
    setDataVariance(Object::DYNAMIC);
    setCullingActive(false);

    m_pViewer = pViewer;    
    m_bCameraCentrum = centrum;
    if (centrum)
    {
        m_iStep = 9;
    }
    else
    {
        m_iStep = 4;
    }
}

void PhotoGeometry::Init()
{
    osg::ref_ptr<osg::LineWidth> lineWidth = new osg::LineWidth(1.0);
    getOrCreateStateSet()->setAttributeAndModes(lineWidth.get(), osg::StateAttribute::ON);
    getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    
    osg::ref_ptr<Vec3Array> pArray = dynamic_cast<Vec3Array*>(getVertexArray());
    m_mapIDBackPlaneVertex.clear();
    for (auto it : m_mapIDOffset)
    {
        int id = it.first;
        osg::Vec3Array* pTmpArr = new osg::Vec3Array;
        int offset = it.second * m_iStep;

        pTmpArr->push_back(pArray->at(offset + 0));
        pTmpArr->push_back(pArray->at(offset + 1));
        pTmpArr->push_back(pArray->at(offset + 2));
        pTmpArr->push_back(pArray->at(offset + 3));

        m_mapIDBackPlaneVertex.insert(make_pair(id, new PlaneVertex(m_pViewer, pTmpArr)));
        m_vecPhotosID.push_back(id);
    }
    setCullingActive(true);
}

void PhotoGeometry::SaveID(int id, int offset, CameraCentrum* pCamera)
{
    m_mapIDOffset.insert(make_pair(id, offset));
    m_vecCameraCentrum.insert(make_pair(id, pCamera));
}

void PhotoGeometry::MoveID(int id)
{
     if (m_mapIDOffset.find(id) != m_mapIDOffset.end())
     {
         m_mapIDOffset.erase(id);
         m_vecCameraCentrum.erase(id);
         m_mapIDBackPlaneVertex.erase(id);
     }
}

void PhotoGeometry::Reset()
{

    for (auto it : m_vecCurrentPickedID)
    {
        if (m_vecCameraCentrum.find(it) != m_vecCameraCentrum.end())
        {
            int offset = m_mapIDOffset.find(it)->second * m_iStep;
            CameraCentrum* pCameraInfo= m_vecCameraCentrum.find(it)->second;

            osg::Vec4 planeColor, centrumColor;
            GetGeometryColor(MOUSE_NONE, pCameraInfo->cameraInfo.aerType, planeColor, centrumColor);          

            UpdateCentrumColor(offset, planeColor, centrumColor);
        }
    }

    Update();

    m_vecCurrentPickedID.clear();
}

void PhotoGeometry::Reset(const std::vector<int>& vecPhotoID)
{
    for (auto it : vecPhotoID)
    {
        if (m_vecCameraCentrum.find(it) == m_vecCameraCentrum.end())
        {
            continue;
        }
        
        int offset = m_mapIDOffset.find(it)->second * m_iStep;
        CameraCentrum* pCameraInfo = m_vecCameraCentrum.find(it)->second;

        osg::Vec4 planeColor, centrumColor;
        GetGeometryColor(MOUSE_NONE, pCameraInfo->cameraInfo.aerType, planeColor, centrumColor);

        UpdateCentrumColor(offset, planeColor, centrumColor);
    }

    Update();

    m_vecCurrentPickedID.clear();
}


string PhotoGeometry::Hover(const osg::Vec3& point)
{
    int Id = -1;
    for (auto it : m_mapIDBackPlaneVertex)
    {
        int id = it.first;                            
        if (it.second->IsInside(point))
        {
            Id = id;
            break;
        }
        
    }
    if (Id == -1)
    {
        return "";
    }
    
    if(std::find(m_vecCurrentPickedID.begin(), m_vecCurrentPickedID.end(), Id) != m_vecCurrentPickedID.end())
    {
        return "";
    }

    m_vecCurrentPickedID.push_back(Id);

    osg::Vec4 planeColor, centrumColor;
    CameraCentrum* pCameraInfo = m_vecCameraCentrum.find(Id)->second;
    GetGeometryColor(MOUSE_HOVER, pCameraInfo->cameraInfo.aerType, planeColor, centrumColor);

    int offset = m_mapIDOffset.find(Id)->second * m_iStep;
    UpdateCentrumColor(offset, planeColor, centrumColor);
   
    Update();

    return pCameraInfo->cameraInfo.ImagePath;
}

bool PhotoGeometry::IsExist(const int& id)
{
    if (m_mapIDOffset.find(id) != m_mapIDOffset.end())
    {
        return true;
    }
    return false;
}

osg::ref_ptr<osg::Geode> PhotoGeometry::Picked(const int& id, Select_Type type)
{
    if (m_mapIDOffset.find(id) != m_mapIDOffset.end())
    {
        m_vecCurrentPickedID.push_back(id);

        osg::Vec4 planeColor, centrumColor;
        CameraCentrum* pCameraInfo = m_vecCameraCentrum.find(id)->second;
        GetGeometryColor(MOUSE_PICKED, pCameraInfo->cameraInfo.aerType, planeColor, centrumColor);

        int offset = m_mapIDOffset.find(id)->second * m_iStep;
        UpdateCentrumColor(offset, planeColor, centrumColor);

        if (type == SELECT_TYPE::SELECT_ONE)
        {
            return UpdatePickedCentrun(pCameraInfo->cameraInfo);
        }
    }

    return nullptr;
}

osg::ref_ptr<osg::Geode> PhotoGeometry::Picked(const osg::Vec3& point, std::pair<int&, string&>& info, const SELECT_TYPE& type)
{
    int Id = -1;
    for (auto it : m_mapIDBackPlaneVertex)
    {
        int id = it.first;
        if (it.second->IsInside(point))
        {
            Id = id;
            break;
        }

    }
    if (Id == -1)
    {
        return nullptr;
    }

    m_vecCurrentPickedID.push_back(Id);

    osg::Vec4 planeColor, centrumColor;
    CameraCentrum* pCameraInfo = m_vecCameraCentrum.find(Id)->second;
    GetGeometryColor(MOUSE_PICKED, pCameraInfo->cameraInfo.aerType, planeColor, centrumColor);
    info.first = Id;
    info.second = pCameraInfo->cameraInfo.ImagePath;

    int offset = m_mapIDOffset.find(Id)->second * m_iStep;
    UpdateCentrumColor(offset, planeColor, centrumColor);

    if (type == SELECT_TYPE::SELECT_ONE)
    {
        return UpdatePickedCentrun(pCameraInfo->cameraInfo);
    }


    return nullptr;
}

void PhotoGeometry::UpdateCentrumColor(int offset, osg::Vec4 planeColor, osg::Vec4 centrumColor)
{
    osg::Vec4Array* pColor = dynamic_cast<osg::Vec4Array*>(getColorArray());
    //update plane
    for (int i = offset; i < offset + 4; i++)
    {
        (*pColor)[i] = planeColor;
    }
    // update centrum
    if (m_bCameraCentrum)
    {
        for (int i = offset + 4; i < offset + m_iStep; i++)
        {
            (*pColor)[i] = centrumColor;
        }
    }

}

void PhotoGeometry::Update()
{
    dirtyDisplayList();
}

osg::ref_ptr<osg::Geode> PhotoGeometry::UpdatePickedCentrun(const ST_CAMERA_INFO& stCamera)
{
    float Vfov = std::atan(stCamera.Image_Height * 0.5 / stCamera.FocalPixel);
    float Hfov = std::atan(stCamera.Image_Width * 0.5 / stCamera.FocalPixel);
    float LeftOffset = std::tan(Hfov) * stCamera.Depth;
    float UpOffset = std::tan(Vfov) * stCamera.Depth;

    if (m_bCameraCentrum)
    {
        osg::Matrixd mt = stCamera.mt;
        mt.preMultTranslate(-stCamera.Center);
        osg::Matrix matrix = osg::Matrixd::inverse(mt);

        osg::Vec3 cameraLocation(matrix.getTrans());

        osg::Vec3 eye, center, up;
        matrix.getLookAt(eye, center, up);

        osg::Vec3 front(eye - center);
        front.normalize();
        up.normalize();

        osg::Vec3 right(front ^ up);
        right.normalize();

        osg::Vec3 newCenter = cameraLocation + front * stCamera.Depth;

        osg::Vec3 leftBottom = newCenter - right * LeftOffset - up * UpOffset;
        osg::Vec3 rightBottom = newCenter + right * LeftOffset - up * UpOffset;
        osg::Vec3 rightTop = newCenter + right * LeftOffset + up * UpOffset;
        osg::Vec3 lefttop = newCenter - right * LeftOffset + up * UpOffset;


        osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
        osg::ref_ptr<osg::Vec3Array> vectorArray = new osg::Vec3Array;
        vectorArray->push_back(leftBottom);
        vectorArray->push_back(rightBottom);
        vectorArray->push_back(rightTop);
        vectorArray->push_back(lefttop);
        vectorArray->push_back(cameraLocation);

        osg::DrawElementsUInt* drawIndexs2 = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
        drawIndexs2->push_back(0); drawIndexs2->push_back(1); drawIndexs2->push_back(2); drawIndexs2->push_back(3);
        geometry->addPrimitiveSet(drawIndexs2);

        osg::DrawElementsUInt* drawIndexs = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
        drawIndexs->push_back(0); drawIndexs->push_back(1); drawIndexs->push_back(4);
        drawIndexs->push_back(1); drawIndexs->push_back(2); drawIndexs->push_back(4);
        drawIndexs->push_back(2); drawIndexs->push_back(3); drawIndexs->push_back(4);
        drawIndexs->push_back(3); drawIndexs->push_back(0); drawIndexs->push_back(4);

        geometry->addPrimitiveSet(drawIndexs);
        osg::ref_ptr<osg::Vec4Array> vc = new osg::Vec4Array;
        vc->push_back(Unitl::FromHex("63D91132"));//2F8DFF33

        geometry->setVertexArray(vectorArray);
        geometry->setColorArray(vc);
        geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
        geometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
       // geometry->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::ON);   // 开启面剔除

        geometry->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        geometry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
        pGeode->addDrawable(geometry);
        return pGeode;
    }

    return nullptr;
}

void PhotoGeometry::RemoveVertex(const int& offset)
{
    osg::ref_ptr<Vec3Array> pArray = dynamic_cast<Vec3Array*>(getVertexArray());
    osg::Vec4Array* pColor = dynamic_cast<osg::Vec4Array*>(getColorArray());
    //osg::DrawElementsUInt* planeIndexs = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
    //osg::DrawElementsUInt* centrumIndexs = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, 0);

    int vertexIndex = offset * m_iStep;

    for (int i = vertexIndex; i < vertexIndex + m_iStep; i++)
    {           
        pArray->at(i).set(0,0,0);
        pColor->at(i).set(0,0,0,0);
    }
}

//void PhotoGeometry::RemoveChild()
//{
//    for (auto Id : m_vecCurrentPickedID)
//    {
//        if (m_mapIDOffset.find(Id) == m_mapIDOffset.end())
//        {
//            continue;
//        }
//        int offset = m_mapIDOffset.find(Id)->second;
//        RemoveVertex(offset);
//
//        MoveID(Id);
//       
//    }
//    Update();
//    m_vecCurrentPickedID.clear();
//}
void PhotoGeometry::RemoveChild(const int& photoID)
{
    if (photoID == -1)
    {
        for (auto Id : m_vecCurrentPickedID)
        {
            if (m_mapIDOffset.find(Id) == m_mapIDOffset.end())
            {
                continue;
            }
            int offset = m_mapIDOffset.find(Id)->second;
            RemoveVertex(offset);

            MoveID(Id);

        }
        Update();
        m_vecCurrentPickedID.clear();
    }
    else
    {
        int offset = m_mapIDOffset.find(photoID)->second;
        RemoveVertex(offset);

        MoveID(photoID);
        Update();
    }


}
void PhotoGeometry::BoxSelect(const osg::Vec3& minXY, const osg::Vec3& maxXY)
{
    for (auto it : m_mapIDBackPlaneVertex)
    {
        if (it.second->IsPolygonSide(minXY, maxXY))
        {
            Picked(it.first);
        }
    }
}

void PhotoGeometry::PolygonSelect(const std::vector<osg::Vec3>& vecPoints)
{
    for (auto it : m_mapIDBackPlaneVertex)
    {
        if (it.second->IsPolygonSide(vecPoints))
        {
            Picked(it.first);
        }
    }
}

