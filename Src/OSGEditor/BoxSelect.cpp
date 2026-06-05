// Copyright Airlook, Inc. All Rights Reserved.
#pragma once

#include "OSGEditor/BoxSelect.h"
#include "OSGEditor/PhotosNodeManager.h"
#include "OSGEditor/EventManager.h"

BoxSelect::BoxSelect(OsgEngine* pOsgEngine) :m_pOsgEngine(pOsgEngine)
{
    m_pSelectedNode = m_pOsgEngine->GetPickedNode();
}

void BoxSelect::Init()
{
    m_pGeometry = new osg::Geometry;
    m_pLineGeometry = new osg::Geometry;

    setDataVariance(osg::Object::DYNAMIC);

    {
        osg::DrawElementsUInt* drawIndexs = new osg::DrawElementsUInt(osg::PrimitiveSet::POLYGON, 0);
        drawIndexs->push_back(3);
        drawIndexs->push_back(2);
        drawIndexs->push_back(1);
        drawIndexs->push_back(0);
        m_pGeometry->addPrimitiveSet(drawIndexs);

        osg::Vec4 color = Unitl::FromHex(EditerEngine::PolygonColor[1]);
        osg::ref_ptr<osg::Vec4Array> vc = new osg::Vec4Array;
        vc->push_back(color); vc->push_back(color); vc->push_back(color); vc->push_back(color);
        m_pGeometry->setColorArray(vc);
        m_pGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

        m_pGeometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
        m_pGeometry->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        m_pGeometry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

        addDrawable(m_pGeometry);

    }

    {
        osg::Vec4 color = Unitl::FromHex(EditerEngine::PolygonColor[0]);
        osg::DrawElementsUInt* drawIndexs = new osg::DrawElementsUInt(osg::PrimitiveSet::POLYGON, 0);
        drawIndexs->push_back(3);
        drawIndexs->push_back(2);
        drawIndexs->push_back(1);
        drawIndexs->push_back(0);
        m_pLineGeometry->addPrimitiveSet(drawIndexs);

        osg::ref_ptr<osg::Vec4Array> vc = new osg::Vec4Array;
        vc->push_back(color); vc->push_back(color); vc->push_back(color); vc->push_back(color);

        m_pLineGeometry->setColorArray(vc);
        m_pLineGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

        osg::ref_ptr<osg::LineWidth> lineWidth = new osg::LineWidth;
        lineWidth->setWidth(1.0f);
        m_pLineGeometry->getOrCreateStateSet()->setAttributeAndModes(lineWidth, osg::StateAttribute::ON);

        osg::ref_ptr<osg::PolygonMode> polygonMode = new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
        m_pLineGeometry->getOrCreateStateSet()->setAttributeAndModes(polygonMode);

        addDrawable(m_pLineGeometry);

    }
}

void BoxSelect::Update(const Vec2& start, const Vec2& end)
{
    if (m_pGeometry == nullptr || m_pLineGeometry == nullptr)
    {
        Init();
    }

    osg::ref_ptr<osg::Vec3Array> vec = new osg::Vec3Array;
    vec->push_back(osg::Vec3(start.x(), start.y(), 0));
    vec->push_back(osg::Vec3(start.x(), end.y(), 0));
    vec->push_back(osg::Vec3(end.x(), end.y(), 0));
    vec->push_back(osg::Vec3(end.x(), start.y(), 0));
    m_pGeometry->setVertexArray(vec);
    m_pGeometry->dirtyDisplayList();


    m_pLineGeometry->setVertexArray(vec);
    m_pLineGeometry->dirtyDisplayList();

    m_vecFristPoint = start;
    m_vecLastPoint = end;
}

void BoxSelect::End(osg::Camera* pCamera, const Element_Type& type)
{
    this->removeDrawable(m_pLineGeometry);
    this->removeDrawable(m_pGeometry);

    m_pLineGeometry = nullptr;
    m_pGeometry = nullptr;

    if (m_vecFristPoint.length() == 0)
    {
        return;
    }
    //框选结束，获取框选节点并返回
    osg::Vec3 minXY(std::fmin(m_vecFristPoint.x(), m_vecLastPoint.x()), std::fmin(m_vecFristPoint.y(), m_vecLastPoint.y()), 0);
    osg::Vec3 maxXY(std::fmax(m_vecFristPoint.x(), m_vecLastPoint.x()), std::fmax(m_vecFristPoint.y(), m_vecLastPoint.y()), 0);
    m_vecFristPoint.set(0, 0);

    std::vector<ST_CALLBACK_ELEMENT_INFO> vecCallback;
    //点云框选策略
    switch (type)
    {
    case ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS:
    {
        CustomNode* pNode = m_pOsgEngine->GetElementLayerRoot(ELEMENT_TIEPOINTS);
        if (pNode)
        {
            for (auto it = pNode->GetAllChild()->begin(); it != pNode->GetAllChild()->end(); it++)
            {
                PointNode* pointNode = dynamic_cast<PointNode*>(it->second);

                if (pointNode->BoxSelect(m_pOsgEngine->GetViewer(), minXY, maxXY))
                {
                    m_pSelectedNode->push_back(pointNode);
                }

            }
        }
    }
    break;
    case ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS:
    {
        PhotosNodeManager* pNode = dynamic_cast<PhotosNodeManager*>(m_pOsgEngine->GetElementLayerRoot(ELEMENT_PHOTOS).get());
        if (pNode)
        {
            if (pNode->BoxSelect(m_pOsgEngine->GetViewer(), minXY, maxXY))
            {
                m_pSelectedNode->push_back(pNode);
            }
        }
    }
    break;

    default:
    {

        osg::ref_ptr<CustomNode> pGroup = m_pOsgEngine->GetElementLayerRoot(type);
        std::map<int, CustomNode*>* pCustomMap = pGroup->GetAllChild();
        for (auto it = pCustomMap->begin(); it != pCustomMap->end(); it++)
        {
            osg::ref_ptr<CustomNode> pCustomNode = it->second;
            if (!pCustomNode || pCustomNode->GetElementType() != type)
            {
                continue;
            }

            osg::Vec3 center = pCustomNode->getBound().center();
            osg::Vec3 windows;

            osgViewer::Renderer* render = dynamic_cast<osgViewer::Renderer*>(m_pOsgEngine->GetViewer()->getCamera()->getRenderer());
            osgUtil::SceneView* pSceneView = render->getSceneView(0);
            if (!pSceneView)
            {
                continue;
            }
            pSceneView->projectObjectIntoWindow(center, windows);
            if (Unitl::IsPointInsideBoundingBox(windows, minXY, maxXY))
            {
                pCustomNode->Picked(m_pOsgEngine->GetCurrentSelectType()); //选中、改变颜色状态
                m_pSelectedNode->push_back(pCustomNode);

                //事件通知、联动
                ST_CALLBACK_ELEMENT_INFO callbackinfo;
                callbackinfo.ID = pCustomNode->m_iID;
                callbackinfo.name = pCustomNode->m_strName;
                vecCallback.push_back(callbackinfo);
            }
        }
    }
    break;
    }

    if (vecCallback.size() == 0)
    {
        return;
    }

    switch (type)
    {
    //case::ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS:
    //{
    //    EventManager::GetInstance()->notifyEvent({ CALL_BACK_SELECT_PHOTO, &vecCallback },m_pOsgEngine);

    //}break;
    //case::ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS:
    //{
    //    //EventManager::GetInstance()->notifyEvent({ CALL_BACK_TIEPOINT, &vecCallback },m_pOsgEngine);
    //}break;
    //case::ELEMENT_LAYER_TYPE::ELEMENT_SURVEY_POINTS:
    //{

    //}break;
    case::ELEMENT_LAYER_TYPE::ELEMENT_TILE:
    { EventManager::GetInstance()->notifyEvent({ CALL_BACK_SELECT_TILE, &vecCallback },m_pOsgEngine);

    }break;
    case::ELEMENT_LAYER_TYPE::ELEMENT_ROI:
    { EventManager::GetInstance()->notifyEvent({ CALL_BACK_ROI_BOX_DRAG, &vecCallback },m_pOsgEngine);
    }break;
    default:
        break;
    }

    return;
}
