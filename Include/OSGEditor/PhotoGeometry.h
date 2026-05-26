#pragma once
#include "Base.h"
#include "Unitl.h"

static void GetGeometryColor(MOUSE_TYPE eventType, Aerotriangulation_Type type, osg::Vec4& plane, osg::Vec4& centrum)
{
    std::string PlaneColor = "FFFFFF";
    std::string CentrumColor = "FFFFFF";
    if (eventType == MOUSE_TYPE::MOUSE_NONE)
    {
        if (type == Aerotriangulation_Type::AER_FRONT)
        {
            PlaneColor = EditerEngine::FrontDefaultColor[0];
            CentrumColor = EditerEngine::FrontDefaultColor[1];
        }
        else if (type == Aerotriangulation_Type::AER_BACK_SUCCESS)
        {
            PlaneColor = EditerEngine::BackDefaultColor[0];
            CentrumColor = EditerEngine::BackDefaultColor[1];
        }
        else if (type == Aerotriangulation_Type::AER_BACK_FAIL) {
            PlaneColor = EditerEngine::BackFailDefaultColor[0];
            CentrumColor = EditerEngine::BackFailDefaultColor[1];
        }
    }
    else if(eventType == MOUSE_TYPE::MOUSE_HOVER)
    {
        if (type == Aerotriangulation_Type::AER_FRONT)
        {
            PlaneColor = EditerEngine::FrontHoverColor[0];
            CentrumColor = EditerEngine::FrontHoverColor[1];
        }
        else if (type == Aerotriangulation_Type::AER_BACK_SUCCESS)
        {
            PlaneColor = EditerEngine::BackHoverColor[0];
            CentrumColor = EditerEngine::BackHoverColor[1];
        }
        else if (type == Aerotriangulation_Type::AER_BACK_FAIL) {
            PlaneColor = EditerEngine::BackFailHoverColor[0];
            CentrumColor = EditerEngine::BackFailHoverColor[1];
        }
    }
    else if (eventType == MOUSE_TYPE::MOUSE_PICKED)
    {
        if (type == Aerotriangulation_Type::AER_FRONT)
        {
            PlaneColor = EditerEngine::FrontSelectedColor[0];
            CentrumColor = EditerEngine::FrontSelectedColor[1];
        }
        else if (type == Aerotriangulation_Type::AER_BACK_SUCCESS)
        {
            PlaneColor = EditerEngine::BackSelectedColor[0];
            CentrumColor = EditerEngine::BackSelectedColor[1];
        }
        else if (type == Aerotriangulation_Type::AER_BACK_FAIL) {
            PlaneColor = EditerEngine::BackFailSelectedColor[0];
            CentrumColor = EditerEngine::BackFailSelectedColor[1];
        }
    }

    plane = Unitl::FromHex(PlaneColor);
    centrum = Unitl::FromHex(CentrumColor);


}

class PlaneVertex
{
public:
    PlaneVertex(osg::ref_ptr<osgViewer::Viewer> pViewer, osg::ref_ptr<osg::Vec3Array> pArr)
    {
        m_pViewer = pViewer;
        m_pVertex = pArr;

        for (int i=0; i< 4; i++)
        {
            m_center += m_pVertex->at(i);
        }

        m_center /= 4.0;
    };

    bool IsValid()
    {
        osgUtil::SceneView* pSceneView = dynamic_cast<osgViewer::Renderer*>(m_pViewer->getCamera()->getRenderer())->getSceneView(0);
        osg::Vec3 xy;
        pSceneView->projectObjectIntoWindow(m_center, xy);
        if (xy.x() < 0 || xy.y() < 0 )
        {
            return false;
        }
        if (m_pViewer->getCamera()->getViewport() == nullptr)
        {
            return false;
        }
        int width = m_pViewer->getCamera()->getViewport()->width();
        int height = m_pViewer->getCamera()->getViewport()->height();
        if (xy.x() > width || xy.y() > height)
        {
            return false;
        }

        return true;
    }

    bool IsInside(const osg::Vec3 &point)
    {
        if (m_pVertex == nullptr || !IsValid())
        {
            return false;
        }
        osgUtil::SceneView* pSceneView = dynamic_cast<osgViewer::Renderer*>(m_pViewer->getCamera()->getRenderer())->getSceneView(0);
        std::vector<osg::Vec3> windows = std::vector <osg::Vec3>(4);
        pSceneView->projectObjectIntoWindow(m_pVertex->at(0), windows[0]);
        pSceneView->projectObjectIntoWindow(m_pVertex->at(1), windows[1]);
        pSceneView->projectObjectIntoWindow(m_pVertex->at(2), windows[2]);
        pSceneView->projectObjectIntoWindow(m_pVertex->at(3), windows[3]);

        osg::Vec3 tmpPoint;
        pSceneView->projectObjectIntoWindow(point, tmpPoint);

        if (Unitl::IsPointInPolygon(tmpPoint, windows))
        {
            return true;
        }
        return false;
    };

    bool IsPolygonSide(const osg::Vec3 &minXY, const osg::Vec3 &maxXY)
    {
        if (m_pVertex == nullptr || !IsValid())
        {
            return false;
        }

        osgUtil::SceneView* pSceneView = dynamic_cast<osgViewer::Renderer*>(m_pViewer->getCamera()->getRenderer())->getSceneView(0);

        osg::Vec3 tmpPoint;
        pSceneView->projectObjectIntoWindow(m_center, tmpPoint);

        if (Unitl::IsPointInsideBoundingBox(tmpPoint, minXY, maxXY))
        {
            return true;
        }

        return false;

    };

    bool IsPolygonSide(const std::vector<osg::Vec3>& vecPoints)
    {

        if (m_pVertex == nullptr || !IsValid())
        {
            return false;
        }

        osgUtil::SceneView* pSceneView = dynamic_cast<osgViewer::Renderer*>(m_pViewer->getCamera()->getRenderer())->getSceneView(0);

        osg::Vec3 tmpPoint;
        pSceneView->projectObjectIntoWindow(m_center, tmpPoint);

        if (Unitl::IsPointInPolygon(tmpPoint, vecPoints))
        {
            return true;
        }

        return false;
    }
private:
    osg::ref_ptr<osg::Vec3Array> m_pVertex;
    osg::ref_ptr<osgViewer::Viewer> m_pViewer;

    osg::Vec3 m_center;
    
};

class PhotoGeometry : public osg::Geometry
{
public:
    PhotoGeometry(osg::ref_ptr<osgViewer::Viewer> pViewer, bool centrum = true);

    void Init();

    void SaveID(int id, int offset, CameraCentrum* pCamera);

    void MoveID(int id);

    void Reset();

    void Reset(const std::vector<int>& vecPhotoID);

    string Hover(const osg::Vec3 &point);

    osg::ref_ptr<osg::Geode> Picked(const osg::Vec3& point, std::pair<int &,string &> &info, const SELECT_TYPE& type = SELECT_TYPE::SELECT_ONE);
    osg::ref_ptr<osg::Geode> Picked(const int& id, Select_Type type = SELECT_TYPE::SELECT_NONE);

    void ScaleChild(float value);

    void RemoveChild(const int & photoID = -1);

    std::vector<int> *GetPickedID() { return &m_vecCurrentPickedID; };

    bool IsSelected() { return m_vecCurrentPickedID.size() > 0; };

    void BoxSelect(const osg::Vec3& minXY, const osg::Vec3& maxXY);

    void PolygonSelect(const std::vector<osg::Vec3>& vecPoints);

    void Update();

    bool IsDelete() { return m_mapIDBackPlaneVertex.size() == 0; };

    std::vector<int> *GetPhotosID() { return &m_vecPhotosID; };

    bool IsExist(const int &id);

private:
    void UpdateCentrumColor(int offset, osg::Vec4 planeColor, osg::Vec4 centrumColor);
    osg::ref_ptr<osg::Geode> UpdatePickedCentrun(const ST_CAMERA_INFO &stCamera);

    void RemoveVertex(const int &offset);

private:
    int m_iStep;
    std::map<int, int> m_mapIDOffset;
    Element_Type m_iElementType = Element_Type::ELEMENT_PHOTOS;

    std::map<int, PlaneVertex*> m_mapIDBackPlaneVertex;
    osg::ref_ptr<osgViewer::Viewer> m_pViewer;
    std::map<int, CameraCentrum*> m_vecCameraCentrum;
    std::vector<int> m_vecCurrentPickedID;
    std::vector<int> m_vecPhotosID;
    bool m_bCameraCentrum;
    
};

