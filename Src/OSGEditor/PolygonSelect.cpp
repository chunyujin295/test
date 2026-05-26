// Copyright Airlook, Inc. All Rights Reserved.
#pragma once

#include "PolygonSelect.h"
#include <osgUtil/Tessellator>
#include <osg/TriangleFunctor>
#include "OsgEngine.h"
#include "EventManager.h"



PolygonSelect::PolygonSelect(OsgEngine* pOsgEngine)
{
    m_pOsgEngine = pOsgEngine;
    m_pGeometry = new osg::Geometry;
    m_pLineGeometry = new osg::Geometry;
    m_pPointsGeometry = new osg::Geometry;

    setDataVariance(Object::DYNAMIC);
    m_pGeometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
}

void PolygonSelect::Init()
{
    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " pointer:"
    //    << std::hex << std::showbase << this << std::dec << std::endl;

    m_pMovePointsGeometry = new osg::Geometry;

    m_pMovePointsGeometry->setVertexArray(new osg::Vec3Array(1));

    osg::ref_ptr<osg::Vec4Array> vc = new osg::Vec4Array;
    vc->push_back(osg::Vec4(0, 1, 0, 1));
    m_pMovePointsGeometry->setColorArray(vc);
    m_pMovePointsGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    m_pMovePointsGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, 1));
    m_pMovePointsGeometry->getOrCreateStateSet()->setAttributeAndModes(new osg::Point(8), osg::StateAttribute::ON);

    addDrawable(m_pMovePointsGeometry);
}

void PolygonSelect::SetStatus(const bool& status)
{
  //  m_bStatus = status;
    if (status)
    {
        Init();
    }
    else
    {
        if (m_pMovePointsGeometry)
        {
            this->removeDrawable(m_pMovePointsGeometry);
            m_pMovePointsGeometry = nullptr;
        }
        Clear();
    }
}

void PolygonSelect::CreatePoint()
{

    this->removeDrawable(m_pPointsGeometry);
    m_pPointsGeometry = nullptr;
    m_pPointsGeometry = new osg::Geometry;

    osg::ref_ptr<osg::Vec3Array> pPoint = new osg::Vec3Array;
    osg::Vec4 color = Unitl::FromHex(EditerEngine::PolygonColor[0]);
    osg::ref_ptr<osg::Vec4Array> vc = new osg::Vec4Array;
    for (int i = 0; i < m_vecPoints.size(); i++)
    {
        pPoint->push_back(m_vecPoints[i]);
        vc->push_back(color);
    }
    m_pPointsGeometry->setVertexArray(pPoint);




    m_pPointsGeometry->setColorArray(vc);
    m_pPointsGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    m_pPointsGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, m_vecPoints.size()));
    m_pPointsGeometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);


    m_pPointsGeometry->getOrCreateStateSet()->setAttributeAndModes(new osg::Point(4.0), osg::StateAttribute::ON);

    addDrawable(m_pPointsGeometry.get());
}


void PolygonSelect::CreateLine()
{

    if (m_vecPoints.size() < 2)
    {
        return;
    }

    this->removeDrawable(m_pLineGeometry);
    m_pLineGeometry = new osg::Geometry;

    osg::ref_ptr<osg::DrawElementsUInt> drawIndexs = new osg::DrawElementsUInt(osg::PrimitiveSet::LINE_LOOP, 0);
    osg::ref_ptr<osg::Vec3Array> pPoint = new osg::Vec3Array;
    for (int i = 0; i < m_vecPoints.size(); i++)
    {
        pPoint->push_back(m_vecPoints[i]);
        drawIndexs->push_back(i);
    }

    m_pLineGeometry->setVertexArray(pPoint);
    osg::Vec4 color = Unitl::FromHex(EditerEngine::PolygonColor[0]);
    osg::ref_ptr<osg::Vec4Array> vc = new osg::Vec4Array;
    vc->push_back(color);
    m_pLineGeometry->setColorArray(vc);
    m_pLineGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    m_pLineGeometry->addPrimitiveSet(drawIndexs);
    m_pLineGeometry->getOrCreateStateSet()->setAttributeAndModes(new osg::LineWidth, osg::StateAttribute::ON);

    addDrawable(m_pLineGeometry.get());
}

void PolygonSelect::CreatePolygon()
{
    if (m_vecPoints.size() < 3)
    {
        return;
    }

    this->removeDrawable(m_pGeometry);
    m_pGeometry = new osg::Geometry;

    osg::ref_ptr<osg::DrawElementsUInt> drawIndexs = new osg::DrawElementsUInt(osg::PrimitiveSet::POLYGON, 0);
    osg::ref_ptr<osg::Vec3Array> pPoint = new osg::Vec3Array;
    for (int i = 0; i < m_vecPoints.size(); i++)
    {
        pPoint->push_back(m_vecPoints[i]);
        drawIndexs->push_back(i);
    }

    m_pGeometry->setVertexArray(pPoint);

    osg::Vec4 PolygonColor = Unitl::FromHex(EditerEngine::PolygonColor[1]);
    osg::ref_ptr<osg::Vec4Array> vc = new osg::Vec4Array;
    vc->push_back(PolygonColor);
    m_pGeometry->setColorArray(vc);
    m_pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    m_pGeometry->addPrimitiveSet(drawIndexs);

    m_pGeometry->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    m_pGeometry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    //解决凹凸多边形问题
    osg::ref_ptr<osgUtil::Tessellator> tscx = new osgUtil::Tessellator;
    tscx->setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
    tscx->setBoundaryOnly(false);
    tscx->setWindingType(osgUtil::Tessellator::TESS_WINDING_ODD);
    tscx->retessellatePolygons(*(m_pGeometry.get()));

    addDrawable(m_pGeometry.get());
}

void PolygonSelect::UpdateSelect(const osg::Vec3& pixel)
{

    if (m_vecPoints.size() > 1)
    {
        m_vecPoints.erase(--m_vecPoints.end());
    }

    m_vecPoints.push_back(pixel);
    m_iNum = m_vecPoints.size();

    CreatePoint();
    CreateLine();
    CreatePolygon();
}

void PolygonSelect::UpdateMove(const osg::Vec3& pixel)
{
    osg::ref_ptr<Vec3Array> pArray = dynamic_cast<Vec3Array*>(m_pMovePointsGeometry->getVertexArray());
    pArray->at(0) = pixel;
    osg::Vec4Array* pColor = dynamic_cast<osg::Vec4Array*>(m_pMovePointsGeometry->getColorArray());
    (*pColor)[0].set(0, 1, 0, 1);

    m_pMovePointsGeometry->dirtyGLObjects();

    if (m_vecPoints.size() == m_iNum)
    {
        m_vecPoints.push_back(pixel);
    }
    else
    {
        m_vecPoints.at(m_iNum).set(pixel);
    }

    CreatePoint();
    CreateLine();
    CreatePolygon();

}

void PolygonSelect::Clear()
{
    m_iNum = 0;
    if (m_vecPoints.size() > 3)
    {
        m_vecPoints.erase(--m_vecPoints.end());
    }
    else
    {
        m_vecPoints.clear();
        this->removeDrawable(m_pPointsGeometry);
        this->removeDrawable(m_pLineGeometry);
        this->removeDrawable(m_pGeometry);
        return;
    }
    const ELEMENT_LAYER_TYPE& type = m_pOsgEngine->GetCurrentElementType();
    osg::ref_ptr<CustomNode> pGroup = m_pOsgEngine->GetElementLayerRoot(type);
    std::vector<ST_CALLBACK_ELEMENT_INFO> vecCallback;
    if (type == ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS)
    {
        for (auto it = pGroup->GetAllChild()->begin(); it != pGroup->GetAllChild()->end(); it++)
        {
            PointNode* pointNode = dynamic_cast<PointNode*>(it->second);
            if (pointNode && pointNode->PolygonSelect(m_pOsgEngine->GetViewer(), m_vecPoints))
            {
                m_pOsgEngine->GetPickedNode()->push_back(pointNode);
            }
        }
        m_vecPoints.clear();
        this->removeDrawable(m_pPointsGeometry);
        this->removeDrawable(m_pLineGeometry);
        this->removeDrawable(m_pGeometry);
        return;
    }
    else if (type == ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS)
    {
        PhotosNodeManager* pGroup = dynamic_cast<PhotosNodeManager*>(m_pOsgEngine->GetElementLayerRoot(type).get());
        pGroup->PolygonSelect(m_pOsgEngine->GetViewer(), m_vecPoints);
        m_pOsgEngine->GetPickedNode()->push_back(pGroup);

        m_vecPoints.clear();
        this->removeDrawable(m_pPointsGeometry);
        this->removeDrawable(m_pLineGeometry);
        this->removeDrawable(m_pGeometry);
        return;
    }
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

        if (Unitl::IsPointInPolygon(windows, m_vecPoints))
        {
            //std::cout << "ID: " << pCustomNode->m_iID<<" Name: "<< pCustomNode->m_strName  << std::endl;                     
            if (pCustomNode && pCustomNode->GetElementType() == type)
            {
                pCustomNode->Picked(m_pOsgEngine->GetCurrentSelectType()); //选中、改变颜色状态
                m_pOsgEngine->GetPickedNode()->push_back(pCustomNode);

                //事件通知、联动
                ST_CALLBACK_ELEMENT_INFO callbackinfo;
                callbackinfo.ID = pCustomNode->m_iID;
                callbackinfo.name = pCustomNode->m_strName;
                vecCallback.push_back(callbackinfo);
            }
        }

    }

    m_vecPoints.clear();

    this->removeDrawable(m_pPointsGeometry);
    this->removeDrawable(m_pLineGeometry);
    this->removeDrawable(m_pGeometry);

    m_pPointsGeometry = nullptr;
    m_pLineGeometry = nullptr;
    m_pGeometry = nullptr;

    if (vecCallback.size() == 0)
    {
        return;
    }

    switch (type)
    {
   
    case::ELEMENT_LAYER_TYPE::ELEMENT_TILE:
    {
        EventManager::GetInstance()->notifyEvent({ CALL_BACK_SELECT_TILE, &vecCallback },m_pOsgEngine);

    }break;
    case::ELEMENT_LAYER_TYPE::ELEMENT_ROI:
    {
        EventManager::GetInstance()->notifyEvent({ CALL_BACK_ROI_BOX_DRAG, &vecCallback },m_pOsgEngine);
    }break;
    default:
        break;
    }
}


void PolygonSelect::Cancel()
{

    m_vecPoints.clear();
    m_iNum = 0;
    this->removeDrawable(m_pPointsGeometry);
    this->removeDrawable(m_pLineGeometry);
    this->removeDrawable(m_pGeometry);

    m_pPointsGeometry = nullptr;
    m_pLineGeometry = nullptr;
    m_pGeometry = nullptr;

}