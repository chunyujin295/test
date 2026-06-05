#include "OSGEditor/PickEventHandler.h"
#include "OSGEditor/EventManager.h"
#include "OSGEditor/OsgEngine.h"
#include "OSGEditor/PhotosNodeManager.h"

static osg::Vec3 screenToWorld(osgViewer::Viewer* viewer, double dx, double dy)
{
    osg::Camera* camera = viewer->getCamera();
    osg::Matrix viewMat = camera->getViewMatrix(); //获取当前视图矩阵
    osg::Matrix projMat = camera->getProjectionMatrix();//获取投影矩阵
    osg::Matrix windMat = camera->getViewport()->computeWindowMatrix();//获取窗口矩阵
    osg::Matrix MVPW = viewMat * projMat * windMat;

    osg::Matrix inverseMVPW = osg::Matrix::inverse(MVPW);
    osg::Vec3 mouseWorld = osg::Vec3(dx, dy, 0) * inverseMVPW;
    return mouseWorld;
}

PickEventHandler::PickEventHandler(OsgEngine* pOsgEngine) :m_pOsgEngine(pOsgEngine), m_pTimer(new Timer)
{

    Init();
}

void PickEventHandler::Init()
{
    m_pBoxSelect = new BoxSelect(m_pOsgEngine);
    m_pPolygonSelect = new PolygonSelect(m_pOsgEngine);

    m_pBoxCamera = new osg::Camera;
    m_pBoxCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    m_pBoxCamera->setClearMask(GL_DEPTH_BUFFER_BIT);
    m_pBoxCamera->setRenderOrder(osg::Camera::POST_RENDER);
    m_pBoxCamera->setAllowEventFocus(false);
    m_pBoxCamera->addChild(m_pBoxSelect);
    m_pBoxCamera->addChild(m_pPolygonSelect);

    m_pOsgEngine->GetRootNode()->addChild(m_pBoxCamera);


}

void PickEventHandler::Hover(const osgGA::GUIEventAdapter& ea)
{
    if (m_pOsgEngine->GetROIStatus()) //编辑状态下不能hover
    {
        return;
    }

    //鼠标hover策略
    if ((abs(m_fHoverX - ea.getX()) > 5 || abs(m_fHoverY - ea.getY()) > 5) && m_bHover)
    {
        m_fHoverX = ea.getX();
        m_fHoverY = ea.getY();
        m_bHover = false;

        m_pOsgEngine->GetRootNode()->removeChild(m_pHoverTextNode);
        m_pHoverTextNode = nullptr;                        
        if (m_pHoverNode && m_pHoverNode->m_eMouseType == MOUSE_TYPE::MOUSE_HOVER)
        {
            m_pHoverNode->Reset();
            m_pHoverNode = nullptr;
        }
        else //点云图层hover
        {
            CustomNode* pNode = m_pOsgEngine->GetElementLayerRoot(ELEMENT_TIEPOINTS);
            if (pNode)
            {
                for (auto it = pNode->GetAllChild()->begin(); it != pNode->GetAllChild()->end(); it++)
                {
                    PointNode* pointNode = dynamic_cast<PointNode*>(it->second);
                    pointNode->Hover(m_pOsgEngine->GetViewer(), ea.getX(), ea.getY());
                }

            }
        }
    }
    else
    {
        if (m_pTimer->time_s() < 0.3)   //鼠标静止1s触发hover
        {
            return;
        }

        m_pTimer->setStartTick();

        if (m_bHover)
        {
            return;
        }

        m_bHover = true;
        m_fHoverX = ea.getX();
        m_fHoverY = ea.getY();

        m_pHoverNode = Hover(m_pView, m_fHoverX, m_fHoverY);
        if (m_pHoverNode.valid() && m_pHoverNode->m_eMouseType == MOUSE_NONE)
        {
            m_pHoverNode->Hover();
            osgUtil::SceneView* pSceneView = dynamic_cast<osgViewer::Renderer*>(m_pView->getCamera()->getRenderer())->getSceneView(0);
            osg::Vec3 worldLocation;
            pSceneView->projectWindowIntoObject(osg::Vec3(ea.getX(), ea.getY(), 0), worldLocation);
            m_pHoverTextNode = Unitl::HoverText(m_pHoverNode->m_strName, worldLocation/*screenToWorld(m_pOsgEngine->GetViewer(), ea.getX(), ea.getY())*/);
            m_pOsgEngine->GetRootNode()->addChild(m_pHoverTextNode);
        }
        else  //点云图层hover
        {
            CustomNode* pNode = m_pOsgEngine->GetElementLayerRoot(ELEMENT_TIEPOINTS);
            if (pNode)
            {
                for (auto it = pNode->GetAllChild()->begin(); it != pNode->GetAllChild()->end(); it++)
                {
                    PointNode* pointNode = dynamic_cast<PointNode*>(it->second);
                    if (pointNode && pointNode->Hover(m_pOsgEngine->GetViewer(), ea.getX(), ea.getY(), true))
                    {
                        break;

                    }
                }
            }
        }
    }
}
//@CHY @zhaobinfeng 此处详细说明
bool PickEventHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* nv)
{
    if (m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_NONE)
    {
//        return false;
    }
    m_pView = dynamic_cast<osgViewer::View*>(&aa);
    if (m_pView == NULL)
    {
        return false;
    }
    if (ea.getEventType() == osgGA::GUIEventAdapter::FRAME && ea.getButtonMask() == 0 && (ea.getX() != 0 && ea.getY() != 0)) //hover事件
    {
        m_fLastX = ea.getXnormalized();
        m_fLastY = ea.getYnormalized();
        Hover(ea);
        m_bMove = false;
    }
    if (ea.getEventType() == osgGA::GUIEventAdapter::MOVE && m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_POLYGON)
    {

        m_pBoxCamera->setProjectionMatrix(osg::Matrixd::ortho2D(0, m_pOsgEngine->GetViewer()->getCamera()->getViewport()->width(), 0, m_pOsgEngine->GetViewer()->getCamera()->getViewport()->height()));
        m_pPolygonSelect->UpdateMove(osg::Vec3(ea.getX(), ea.getY(), 0));
    }

    if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH)
    {
        m_fPickX = ea.getX();
        m_fPickY = ea.getY();
        m_eLastEventType = ea.getEventType();
        if (ea.getButton() == osgGA::GUIEventAdapter::MouseButtonMask::LEFT_MOUSE_BUTTON)   //鼠标左键事件
        {
            m_bMouseLeft = true;
            m_pBoxCamera->setProjectionMatrix(osg::Matrixd::ortho2D(0, m_pOsgEngine->GetViewer()->getCamera()->getViewport()->width(), 0, m_pOsgEngine->GetViewer()->getCamera()->getViewport()->height()));
        }
        else  if (ea.getButton() == osgGA::GUIEventAdapter::MouseButtonMask::RIGHT_MOUSE_BUTTON)        //鼠标右键事件
        {
            m_bMouseRight = true;
        }
        else if (ea.getButton() == osgGA::GUIEventAdapter::MouseButtonMask::MIDDLE_MOUSE_BUTTON) //鼠标中键
        {
            m_bMouseMid = true;
        }
    }
    else if (ea.getEventType() == osgGA::GUIEventAdapter::MOVE)
    {
        m_bMove = true;
        m_pTimer->setStartTick();
    }
    else if (ea.getEventType() == osgGA::GUIEventAdapter::DRAG)
    {
        m_eLastEventType = ea.getEventType();
        if (m_pOsgEngine->GetSceneOperationType() == SECENE_OPERATION_TYPE::SECENE_MODE_MOVE)
        {
            osg::ref_ptr<osgGA::OrbitManipulator> pManipulator = (osgGA::OrbitManipulator*)(m_pOsgEngine->GetViewer()->getCameraManipulator());
            float offset = -0.3f * pManipulator->getDistance();
            double dx = offset * (ea.getXnormalized() - m_fLastX);
            double dy = offset * (ea.getYnormalized() - m_fLastY);
            double dz = 0.0;

            osg::Vec3 dv(dx, dy, dz);
            osg::Quat q = pManipulator->getRotation();

            Matrix rotation_matrix;
            rotation_matrix.makeRotate(q);

            osg::Vec3 center = pManipulator->getCenter();
            center += dv * rotation_matrix;

            pManipulator->setCenter(center);

            return true;
        }
        else if (m_pOsgEngine->GetSceneOperationType() == SECENE_OPERATION_TYPE::SECENE_MODE_ROTATE)
        {
            if (m_bMouseRight || m_bMouseMid)
            {
                return true;
            }
            return false;
        }
    }
    else if (ea.getEventType() == osgGA::GUIEventAdapter::DOUBLECLICK && m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_ONE)
    {

        osg::ref_ptr<osgGA::GUIEventAdapter> pEvent = new osgGA::GUIEventAdapter;

        pEvent->setEventType(osgGA::GUIEventAdapter::SCROLL);
        pEvent->setButton(osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON);  //  鼠标按键
        if (ea.getButton() == osgGA::GUIEventAdapter::MouseButtonMask::LEFT_MOUSE_BUTTON)   //左键放大
        {
            pEvent->setScrollingMotion(osgGA::GUIEventAdapter::SCROLL_DOWN);   // 滚动方向
        }
        else if (ea.getButton() == osgGA::GUIEventAdapter::MouseButtonMask::RIGHT_MOUSE_BUTTON) //右键缩小
        {
            pEvent->setScrollingMotion(osgGA::GUIEventAdapter::SCROLL_UP);
        }

        m_pView->getEventQueue()->addEvent(pEvent);
        m_bMouseLeft = m_bMouseRight = m_bMouseMid = false;
        return true;
    }
    else if (ea.getEventType() == osgGA::GUIEventAdapter::SCROLL)
    {
        if (m_bSelectPicked)
        {
            return true;
        }
        m_pTimer->setStartTick();
        if (m_pOsgEngine->GetSceneOperationType() == SECENE_OPERATION_TYPE::SECENE_MODE_ROTATE || m_pOsgEngine->GetSceneOperationType() == SECENE_OPERATION_TYPE::SECENE_MODE_MOVE)
        {
            return true;
        }

        //屏蔽鼠标滚轮
       if (ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON/* && 
            (m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_ONE
                ||m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_BOX
                || m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_POLYGON)*/)
        {

            return false;
        }

        if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT)
        {
            if (osgGA::GUIEventAdapter::SCROLL_DOWN == ea.getScrollingMotion())
            {
               // m_pOsgEngine->ScaleElement(1.1);
                m_pOsgEngine->ScalePhotosElement(1.1);
            }
            else if (osgGA::GUIEventAdapter::SCROLL_UP == ea.getScrollingMotion())
            {
               // m_pOsgEngine->ScaleElement(0.9);
                m_pOsgEngine->ScalePhotosElement(0.9);
            }
            return true;
        }
        if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL)
        {
            if (osgGA::GUIEventAdapter::SCROLL_DOWN == ea.getScrollingMotion())
            {

                m_pOsgEngine->ScaleTiePointsElement(1.1);
            }
            else if (osgGA::GUIEventAdapter::SCROLL_UP == ea.getScrollingMotion())
            {
                m_pOsgEngine->ScaleTiePointsElement(0.9);
            }
            return true;
        }
        //自定义滚轮方向
        {
            osg::ref_ptr<osgGA::GUIEventAdapter> pEvent = new osgGA::GUIEventAdapter;
            if (ea.getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_DOWN)
            {
                pEvent->setEventType(osgGA::GUIEventAdapter::SCROLL);
                pEvent->setButton(osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON);  //  鼠标按键
                pEvent->setScrollingMotion(osgGA::GUIEventAdapter::SCROLL_UP);

            }
            else  if (ea.getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_UP)
            {
                pEvent->setEventType(osgGA::GUIEventAdapter::SCROLL);
                pEvent->setButton(osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON);  //  鼠标按键
                pEvent->setScrollingMotion(osgGA::GUIEventAdapter::SCROLL_DOWN);
            }
            m_pView->getEventQueue()->addEvent(pEvent);

        }
        return true;
    }
    else if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
    {
        if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Delete || ea.getKey() == 127)
        {
            if (m_pOsgEngine->bCanDelete)
            {
                //键盘删除事件          
                std::vector<osg::ref_ptr<CustomNode>>* vecPickedNode = m_pOsgEngine->GetPickedNode();
                if (vecPickedNode && vecPickedNode->size() > 0)
                {
                    osg::ref_ptr<CustomNode> firstCustomNode = vecPickedNode->at(0);

                    if (firstCustomNode->GetElementType() == ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS)
                    {
                        m_pOsgEngine->RemovePickedPhotosFromATSide();
                    }
                    else if (firstCustomNode->GetElementType() == ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS)
                    {
                        m_pOsgEngine->RemovePickedTiePointsFromATSide();
                    }
                    else
                    {
                        // ignore other element types.
                    }
                }

                m_pOsgEngine->RemovePickedNode();
            }

            return false;
        }
        if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Escape || ea.getKey() == 27)
        {
            if (m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_POLYGON)
            {
                m_pPolygonSelect->Cancel();
            }
            return true;
        }
    }

    switch (m_pOsgEngine->GetCurrentElementType())
    {
    case ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS:
    {
        return Photos(ea, aa);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_SURVEY_POINTS:
    {
        return SurveyPoints(ea, aa);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS:
    {
        return TiePoints(ea, aa);;
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_TILE:
    {
        return Tile(ea, aa);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_ROI:
    {
        return ROI(ea, aa);
    }break;
    case ELEMENT_LAYER_TYPE::ELEMENT_MODEL:
    {

    }break;
    default:
        break;
    }

    return false;
}

osg::ref_ptr <CustomNode> PickEventHandler::Hover(osgViewer::View* pview, float x, float y)
{
    osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector = new osgUtil::LineSegmentIntersector(osgUtil::Intersector::WINDOW, x, y);

    osgUtil::IntersectionVisitor visitor(intersector);
    m_pView->getCamera()->accept(visitor);

    if (intersector && intersector->containsIntersections())
    {
        const osgUtil::LineSegmentIntersector::Intersection& intersection = intersector->getFirstIntersection();
        // 处理拾取到的模型
        osg::NodePath nodePath = intersection.nodePath;
        for (int i = 0; i < nodePath.size(); i++)
        {
            osg::ref_ptr <CustomNode> pCustomNode = dynamic_cast<CustomNode*>(nodePath[i]);
            if (pCustomNode.valid() && pCustomNode->GetElementType() != ELEMENT_LAYER_TYPE::ELEMENT_NONE && pCustomNode->m_eMouseType == MOUSE_NONE)
            {
                osg::ref_ptr<osg::Geometry> pGeode = intersection.drawable->asGeometry();
                if (pGeode.valid())
                {
                    pCustomNode->Reset();
                    pCustomNode->m_pSelectGeometry = pGeode.get();
                    pCustomNode->m_PickedPoint = intersection.getWorldIntersectPoint();
                }
                return pCustomNode.get();
            }
        }
    }
    return nullptr;
}

osg::ref_ptr <CustomNode> PickEventHandler::Pick(osgViewer::View* pview, float x, float y)
{
    osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector = new osgUtil::LineSegmentIntersector(osgUtil::Intersector::WINDOW, x, y);

    osgUtil::IntersectionVisitor visitor(intersector);
    pview->getCamera()->accept(visitor);
    //bool bClicked = false;
    if (intersector && intersector->containsIntersections())
    {
        const osgUtil::LineSegmentIntersector::Intersections& intersections = intersector->getIntersections();

        for (auto intersection = intersections.begin(); intersection != intersections.end(); intersection++)
        {
            // 处理拾取到的模型
            osg::NodePath nodePath = intersection->nodePath;
            for (int i = 0; i < nodePath.size(); i++)
            {
                osg::ref_ptr <CustomNode> pCustomNode = dynamic_cast<CustomNode*>(nodePath[i]);
                if (pCustomNode.valid() && pCustomNode->GetElementType() == m_pOsgEngine->GetCurrentElementType())
                {
                    osg::ref_ptr<osg::Geometry> pGeode = intersection->drawable->asGeometry();
                    if (pGeode)
                    {
                        pCustomNode->Reset();
                        pCustomNode->m_pSelectGeometry = pGeode.get();
                        pCustomNode->m_PickedPoint = intersection->getWorldIntersectPoint();
                    }

                    return pCustomNode.get();
                }
            }
        }
    }
    return nullptr;
}

void PickEventHandler::Clear()
{
    for (auto it : *m_pOsgEngine->GetPickedNode())
    {
        it->Reset();
    }

    m_pOsgEngine->GetPickedNode()->clear();

    m_bSelectPicked = false;
    m_bMouseLeft = false;
    m_bMouseRight = false;
    m_bMouseMid = false;    
    m_bHover = false;
    m_bDrag = false;
    m_bMove = false;
    m_bROIDragStatus = false;  
}

void PickEventHandler::Clear(const osgGA::GUIEventAdapter& ea)
{
    if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL || ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT) //键盘事件
    {
        return;
    }

    for (auto it : *m_pOsgEngine->GetPickedNode())
    {
        it->Reset();
    }

    m_pOsgEngine->GetPickedNode()->clear();
}


//元素图层
bool PickEventHandler::Photos(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::RELEASE:
    {
        if (m_eLastEventType == osgGA::GUIEventAdapter::EventType::PUSH && m_bMouseRight &&
            (m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_BOX || m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_POLYGON)) //右键释放触发清除
        {
            if (!m_bSelectPicked)
            {
                Clear(ea);
            }
        }

        if (m_eLastEventType == osgGA::GUIEventAdapter::EventType::PUSH && m_bMouseRight && m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_ONE)
        {
            //右键通知弹窗事件
            std::vector<osg::ref_ptr<CustomNode>>* pPickedNode = m_pOsgEngine->GetPickedNode();
            if (pPickedNode->size() >= 1)
            {
                ST_CALLBACK_ELEMENT_INFO photo;
                photo.ID = pPickedNode->at(0)->m_iID;
                photo.name = pPickedNode->at(0)->m_strName;
                std::vector<ST_CALLBACK_ELEMENT_INFO> vecCallback;
                vecCallback.push_back(photo); EventManager::GetInstance()->notifyEvent({ CALL_BACK_SELECT_PHOTO_WINDOWS, &vecCallback }, m_pOsgEngine);
            }
        }

        if (m_bMouseLeft && m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_BOX)  //框选结束
        {
            if (m_bSelectPicked)
            {
                clock_t t11, t21, t31;

                t11 = clock();


                //框选结束，获取框选节点并改变状态
                m_pBoxSelect->End(m_pOsgEngine->GetViewer()->getCamera(), m_pOsgEngine->GetCurrentElementType());
                t21 = clock();
                t31 = t21 - t11;
                std::cout << "box select time: " << t31 * 0.001 << std::endl;
                //点云改变颜色
                std::vector<int> tmpID;
                if (m_pOsgEngine->GetPickedNode()->size() > 0)
                {
                    PhotosNodeManager* pPhoto = dynamic_cast<PhotosNodeManager*>(m_pOsgEngine->GetPickedNode()->at(0).get());
                    pPhoto->GetPickedPhotosID(&tmpID);
                }
                m_pOsgEngine->SetSelectElement(ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS, tmpID);

                t31 = clock();

                t31 = t31 - t21;
                std::cout << "tiepoints time:  " << t31 * 0.001 << std::endl;
            }

            m_bSelectPicked = false;
        }
        else if (m_bMouseRight && m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_POLYGON) //右键多边形选择结束
        {
            if (m_bSelectPicked)


            {
                m_pPolygonSelect->Clear();
                std::vector<int> tmpID;

                if (m_pOsgEngine->GetPickedNode()->size() > 0)
                {
                    PhotosNodeManager* pPhotoManager = dynamic_cast<PhotosNodeManager*>(m_pOsgEngine->GetPickedNode()->at(0).get());
                    pPhotoManager->GetPickedPhotosID(&tmpID);
                }

                ////点云改变颜色
                m_pOsgEngine->SetSelectElement(ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS, tmpID);
            }
            m_bSelectPicked = false;
        }


        if (m_bMouseLeft && m_eLastEventType == osgGA::GUIEventAdapter::EventType::PUSH && m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_ONE)  //单选
        {
            Clear(ea); //连续单选清除上次结果
            osg::ref_ptr <CustomNode> pCustomNode = Pick(m_pView, m_fPickX, m_fPickY);
            if (pCustomNode)
            {
                pCustomNode->Picked();

                ST_CALLBACK_ELEMENT_INFO photo;
                photo.ID = pCustomNode->m_iID;
                photo.photoID = pCustomNode->m_iID;//20250507
                photo.name = pCustomNode->m_strName;
                std::vector<ST_CALLBACK_ELEMENT_INFO> vecCallback;
                vecCallback.push_back(photo); EventManager::GetInstance()->notifyEvent({ CALL_BACK_SELECT_PHOTO, &vecCallback }, m_pOsgEngine);

                m_pOsgEngine->GetPickedNode()->push_back(pCustomNode);

                //点云改变颜色

                m_pOsgEngine->SetSelectElement(ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS, { pCustomNode->m_iID });
            }
        }


        m_bMouseLeft = m_bMouseRight = m_bMouseMid = false;
        }break;
    case osgGA::GUIEventAdapter::PUSH:
    {
        //Clear(ea); //取消选中状态
        if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT)
        {
            break;
        }

        if (m_bMouseLeft && m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_ONE)  //单选
        {
            ////Clear(ea);
            //osg::ref_ptr <CustomNode> pCustomNode = Pick(m_pView, m_fPickX, m_fPickY);
            //if (pCustomNode)
            //{
            //    pCustomNode->Picked();

            //    ST_CALLBACK_ELEMENT_INFO photo;
            //    photo.ID = 123;
            //    photo.name = "test";
            //    std::vector<ST_CALLBACK_ELEMENT_INFO> vecCallback;
            //    vecCallback.push_back(photo);
            //    EventManager::GetInstance()->notifyEvent({ CALL_BACK_SELECT_PHOTO, &vecCallback });
            //    m_pOsgEngine->GetPickedNode()->push_back(pCustomNode);

            //    //点云改变颜色

            //    m_pOsgEngine->SetSelectElement(ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS, { pCustomNode->m_iID});
            //}
        }
        else if (m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_BOX)  //框选
        {
            //Clear(ea);
        }
        else if (m_bMouseLeft && m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_POLYGON)
        {
           if (!m_bSelectPicked) //每次框选开始先清除上次选择结果
           {
                Clear(ea);
            }
            m_pPolygonSelect->UpdateSelect(osg::Vec3(ea.getX(), ea.getY(), 0));
            m_bSelectPicked = true;

        }
    }break;
    case osgGA::GUIEventAdapter::DRAG:
    {
        if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT)//ALT+鼠标拖动
        {
            if (m_bSelectPicked)
            {
                return true;
            }
            return false;
        }
        if (m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_BOX)//框选
        {
        if (!m_bSelectPicked)  //每次框选开始先清除上次选择结果
            {
                Clear(ea);
            }    
            if (m_bMouseLeft)
            {
                m_pBoxSelect->Update(osg::Vec2(m_fPickX, m_fPickY), osg::Vec2(ea.getX(), ea.getY()));
                m_bSelectPicked = true;
            }

            return true;
        }
        else if (m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_POLYGON)
        {
            return true;
        }
    }break;

    default:
        break;
    }

    return false;
}
bool PickEventHandler::Tile(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{

    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::RELEASE:
    {
        if (m_eLastEventType == osgGA::GUIEventAdapter::EventType::PUSH && m_bMouseRight &&
            (m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_BOX ||
                m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_POLYGON)) //右键释放触发清除
        {
            if (!m_bSelectPicked)
            {
                Clear(ea);
            }
        }


        if (m_bMouseLeft && m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_BOX)  //框选结束
        {

            //框选结束，获取框选节点并改变状态
            m_pBoxSelect->End(m_pOsgEngine->GetViewer()->getCamera(), m_pOsgEngine->GetCurrentElementType());
            m_bSelectPicked = false;

        }
        else if (m_bMouseRight && m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_POLYGON) //右键多边形选择结束
        {
            m_pPolygonSelect->Clear();
            m_bSelectPicked = false;
        }

        if (m_bMouseLeft && m_eLastEventType == osgGA::GUIEventAdapter::EventType::PUSH && m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_ONE)  //单选
        {
            Clear(ea);
            osg::ref_ptr <CustomNode> pCustomNode = Pick(m_pView, m_fPickX, m_fPickY);
            if (pCustomNode)
            {
                pCustomNode->Picked();
                m_pOsgEngine->GetPickedNode()->push_back(pCustomNode);

                ST_CALLBACK_ELEMENT_INFO photo;
                photo.ID = 123;
                photo.name = "test";
                std::vector<ST_CALLBACK_ELEMENT_INFO> vecCallback;
                vecCallback.push_back(photo); EventManager::GetInstance()->notifyEvent({ CALL_BACK_SELECT_TILE, &vecCallback },m_pOsgEngine);
            }
        }
        m_bMouseLeft = m_bMouseRight = m_bMouseMid = false;
    }break;
    case osgGA::GUIEventAdapter::PUSH:
    {
        if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT)
        {
            break;
        }

        if (m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_ONE)  //单选
        {

        }
        else if (m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_BOX)  //框选
        {

        }
        else  if (ea.getButton() == osgGA::GUIEventAdapter::MouseButtonMask::LEFT_MOUSE_BUTTON && m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_POLYGON)
        {
            if (!m_bSelectPicked) //每次框选开始先清除上次选择结果
            {
                Clear(ea);
            }
            m_pPolygonSelect->UpdateSelect(osg::Vec3(ea.getX(), ea.getY(), 0));
            m_bSelectPicked = true;
        }
    }
    break;
    case osgGA::GUIEventAdapter::DRAG:
    {
        if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT)//ALT+鼠标拖动
        {
            if (m_bSelectPicked)
            {
                return true;
            }
            return false;
        }
        if (m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_BOX)//框选
        {
            if (!m_bSelectPicked)
            {
                Clear(ea);
            }
            if (m_bMouseLeft)
            {
                m_pBoxSelect->Update(osg::Vec2(m_fPickX, m_fPickY), osg::Vec2(ea.getX(), ea.getY()));
                m_bSelectPicked = true;
            }

            return true;
        }
        else if (m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_POLYGON)
        {
            return true;
        }
    }break;
    default:
        break;
    }

    return false;
}
bool PickEventHandler::TiePoints(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::RELEASE:
    {
        if (m_eLastEventType == osgGA::GUIEventAdapter::EventType::PUSH && m_bMouseRight &&
            (m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_BOX || m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_POLYGON)) //右键释放触发清除
        {
            if (!m_bSelectPicked)
            {
                Clear(ea);
            }
        }

        if (m_bMouseLeft && m_eLastEventType == osgGA::GUIEventAdapter::EventType::PUSH && m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_ONE)  //单选
        {
            Clear(ea);

            CustomNode* pNode = m_pOsgEngine->GetElementLayerRoot(ELEMENT_TIEPOINTS);
            if (pNode)
            {
                for (auto it = pNode->GetAllChild()->begin(); it != pNode->GetAllChild()->end(); it++)
                {
                    PointNode* pointNode = dynamic_cast<PointNode*>(it->second);
                    if (pointNode && pointNode->Picked(m_pOsgEngine->GetViewer(), ea.getX(), ea.getY()))
                    {
                        m_pOsgEngine->GetPickedNode()->push_back(pointNode);
                        break;

                    }
                }
            }
        }

        if (m_bMouseLeft && m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_BOX)  //框选结束
        {
           
            //框选结束，获取框选节点并改变状态
            m_pBoxSelect->End(m_pOsgEngine->GetViewer()->getCamera(), m_pOsgEngine->GetCurrentElementType());
            m_bSelectPicked = false;
        }
        else if (m_bMouseRight && m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_POLYGON) //右键多边形选择结束
        {
            m_pPolygonSelect->Clear();
            m_bSelectPicked = false;
        }

        m_bMouseLeft = m_bMouseRight = m_bMouseMid = m_bDrag = false;
    }break;
    case osgGA::GUIEventAdapter::PUSH:
    {
       

        if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT)
        {
            break;
        }

        if (m_bMouseLeft && m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_POLYGON)
        {
            if (!m_bSelectPicked)
            {
                Clear(ea);
            }
            m_pPolygonSelect->UpdateSelect(osg::Vec3(ea.getX(), ea.getY(), 0));
            m_bSelectPicked = true;
        }
    } break;
    case osgGA::GUIEventAdapter::DRAG:
    {
        m_bDrag = true;
        if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT)//ALT+鼠标拖动
        {
            if (m_bSelectPicked)
            {
                return true;
            }
            return false;
        }

        if (m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_BOX)//框选
        {
            if (!m_bSelectPicked)
            {
                Clear(ea);
            }
            if (m_bMouseLeft)
            {
                m_pBoxSelect->Update(osg::Vec2(m_fPickX, m_fPickY), osg::Vec2(ea.getX(), ea.getY()));
                m_bSelectPicked = true;
            }

            return true;
        }
        else if (m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_POLYGON)
        {
            return true;
        }
    }break;

    default:
        break;
    }
    return false;
}
bool PickEventHandler::SurveyPoints(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{









    return false;
}

bool PickEventHandler::ROI(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::RELEASE:
    {
        if (m_eLastEventType == osgGA::GUIEventAdapter::EventType::PUSH && m_bMouseRight &&
            (m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_BOX || m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_POLYGON)) //右键释放触发清除
        {
            if (!m_bSelectPicked)
            {
                Clear(ea);
            }
        }
        if (m_bMouseLeft && m_eLastEventType == osgGA::GUIEventAdapter::EventType::PUSH && m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_ONE)  //单选
        {
            Clear(ea);

            {
                osg::ref_ptr <CustomNode> pCustomNode = Pick(m_pView, m_fPickX, m_fPickY);
                if (pCustomNode)
                {
                    pCustomNode->Picked();

                    ST_CALLBACK_ELEMENT_INFO photo;
                    photo.ID = pCustomNode->m_iID;
                    photo.name = pCustomNode->m_strName;
                    std::vector<ST_CALLBACK_ELEMENT_INFO> vecCallback;
                    vecCallback.push_back(photo);
                   
                    //EventManager::GetInstance()->notifyEvent({ CALL_BACK_SELECT_TILE, &vecCallback });

                    m_pOsgEngine->GetPickedNode()->push_back(pCustomNode);
                }
            }
        }

        if (m_bMouseLeft && m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_BOX)  //框选结束
        {
            //框选结束，获取框选节点并改变状态
            m_pBoxSelect->End(m_pOsgEngine->GetViewer()->getCamera(), m_pOsgEngine->GetCurrentElementType());
            m_bSelectPicked = false;
        }
        else if (m_bMouseRight && m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_POLYGON) //右键多边形选择结束
        {
            m_pPolygonSelect->Clear();
            m_bSelectPicked = false;
        }

        if (m_pOsgEngine->GetROIStatus())
        {
            if (m_bMouseLeft && m_pOsgEngine->GetPickedNode()->size() > 0 && m_bROIDragStatus)
            {
                osg::ref_ptr<ROINode> pCustomNode = dynamic_cast<ROINode*>(m_pOsgEngine->GetPickedNode()->front().get());
                pCustomNode->DragEnd();
                m_bROIDragStatus = false;
            }
        }

        m_bMouseLeft = m_bMouseRight = m_bMouseMid = false;

    }break;
    case osgGA::GUIEventAdapter::PUSH:
    {


        if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT)
        {
            break;
        }

        if (m_bMouseLeft && m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_ONE)  //单选
        {

           

        }
        else if (m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_BOX)  //框选
        {

        }
        else  if (m_bMouseLeft && m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_POLYGON)
        {
            if (!m_bSelectPicked) //每次框选开始先清除上次选择结果
            {
                Clear(ea);
            }
            m_pPolygonSelect->UpdateSelect(osg::Vec3(ea.getX(), ea.getY(), 0));
            m_bSelectPicked = true;

        }
    } break;
    case osgGA::GUIEventAdapter::DRAG:
    {
        if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT)//ALT+鼠标拖动
        {
            if (m_bSelectPicked)
            {
                return true;
            }
            return false;
        }
        if (m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_BOX)//框选
        {
            if (!m_bSelectPicked) //每次框选开始先清除上次选择结果
            {
                Clear(ea);
            }
            if (m_bMouseLeft)
            {
                m_pBoxSelect->Update(osg::Vec2(m_fPickX, m_fPickY), osg::Vec2(ea.getX(), ea.getY()));
                m_bSelectPicked = true;
            }

            return true;
        }
        else if (m_pOsgEngine->GetCurrentSelectType() == SELECT_TYPE::SELECT_POLYGON)
        {
            return true;
        }

        if (m_pOsgEngine->GetROIStatus())
        {
            float moveLen =(osg::Vec2(m_fPickX, m_fPickY) - osg::Vec2(ea.getX(), ea.getY())).length(); //拖拽误差

            if (m_bMouseLeft && m_pOsgEngine->GetPickedNode()->size() > 0 && moveLen > 2)
            {
                
                osg::ref_ptr<CustomNode> pCustomNode = m_pOsgEngine->GetPickedNode()->front();
                osg::Vec3 eye = m_pOsgEngine->GetViewer()->getCamera()->getInverseViewMatrix().getTrans();
                osg::Vec3 offset = pCustomNode->m_PickedPoint - eye;
                int dist = offset.length();

                osg::Vec3 mouseWorldPos = screenToWorld(m_pOsgEngine->GetViewer(), ea.getX(), ea.getY());
                osg::Vec3 rayDir = mouseWorldPos - eye;
                rayDir.normalize();

                osg::Vec3 curPos = eye + rayDir * dist;
                pCustomNode->Drag(curPos);
                m_bROIDragStatus = true;
            }

            return true;
        }

    }break;

    default:
        break;
    }

    return false;
}


void PickEventHandler::SetCurrentSelectType(const SELECT_TYPE& type)
{

    switch (type)
    {
    case SELECT_TYPE::SELECT_NONE:
    case SELECT_TYPE::SELECT_BOX:
    case SELECT_TYPE::SELECT_ONE:
    {
        m_pPolygonSelect->SetStatus(false);
    }break;
    case SELECT_TYPE::SELECT_POLYGON:
    {
        m_pPolygonSelect->SetStatus(true);
    }break;
    default:
        break;
    }
}
