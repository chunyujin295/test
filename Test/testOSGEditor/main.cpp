
#include <osgUtil/Optimizer>
//read image
#include <osg/DrawPixels>
#include <osg/Geode>
#include <osgDB/ReadFile>
#include <osg/Node>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <osgUtil/Optimizer>
#include "windows.h"
#include <QApplication>

#include "ViewerQT.h"
#include "MainWindow.h"
#include <osgUtil/Optimizer>
#include "ui_MainWindow.h"
#include "Core/File.h"

#include <QPushButton>


MWindow* gMainWindow = NULL;
int main(int argc, char* argv[])
{



	QApplication a(argc, argv);
	gMainWindow = new MWindow();
	//QWidget* win = new QWidget();

	gMainWindow->resize(600, 400);
	int ret = 1;
	//gMainWindow->showMaximized();
	//win->showMaximized();
	gMainWindow->show();


	ret = a.exec();
	return ret;
	

}

//以下为彬锋的原始的
//
////
//#include <windows.h>
//#include "OSGEditor/Base.h"
//#include "OSGEditor/Unitl.h"
//
//#include <iostream>
//#include "OSGEditor/OsgEngine.h"
//
//
//
//class KeyboardHandler :public osgGA::GUIEventHandler
//{
//
//public:
//    KeyboardHandler(OsgEngine* pOsgEngine) :m_pOsgEngine(pOsgEngine)
//    {
//        std::string modelPath /*= "D:\\works\\src\\OSG\\OpenSceneGraph-Data\\lz.osg"*/;
//        m_pModelNode = pOsgEngine->LoadOsgModel(modelPath);
//        pOsgEngine->DrawBoundBox(m_pModelNode);
//        pOsgEngine->LookAtModel(m_pModelNode, ModelViewType::MODEL_FRONT);
//    }
//    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object* obj, osg::NodeVisitor* nv)
//    {
//        osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
//        if (!viewer)return false;
//
//        switch (ea.getEventType())
//        {
//        case osgGA::GUIEventAdapter::KEYDOWN:
//            if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Space)
//            {
//
//            }
//            else
//            {
//                if (ea.getKey() == '0')
//                {
//                    string a;
//                    //m_pOsgEngine->LoadOsgModel(a);
//                    m_pOsgEngine->LookAtModel(m_pModelNode, ModelViewType::MODEL_FRONT);
//                }
//                else if (ea.getKey() == '1')
//                {
//                    string a;
//                    //m_pOsgEngine->LoadOsgModel(a);
//                    m_pOsgEngine->LookAtModel(m_pModelNode, ModelViewType::MODEL_BACK);
//                }
//                else if (ea.getKey() == '2')
//                {
//                    m_pOsgEngine->LookAtModel(m_pModelNode, ModelViewType::MODEL_LEFT);
//                }
//                else if (ea.getKey() == '3')
//                {
//                    m_pOsgEngine->LookAtModel(m_pModelNode, ModelViewType::MODEL_RIGHT);
//                }
//                else if (ea.getKey() == '4')
//                {
//                    m_pOsgEngine->LookAtModel(m_pModelNode, ModelViewType::MODEL_UP);
//                }
//                else if (ea.getKey() == '5')
//                {
//                    m_pOsgEngine->LookAtModel(m_pModelNode, ModelViewType::MODEL_DOWN);
//                }
//                return true;
//            }
//            break;
//        default:break;
//        }
//        return false;
//    }
//
//private:
//    OsgEngine* m_pOsgEngine;
//    osg::ref_ptr<osg::Node>m_pModelNode;
//};
//
//void Stringsplit(const string& str, const char split, vector<float>& res)
//{
//    if (str == "")      return;
//
//    string strs = str + split;
//    size_t pos = strs.find(split);
//
//    while (pos != strs.npos)
//    {
//        string temp = strs.substr(0, pos);
//        if (!temp.empty())
//        {
//            res.push_back(std::stof(temp));
//        }
//
//        strs = strs.substr(pos + 1, strs.size());
//        pos = strs.find(split);
//    }
//}
//
////test 


//以下为彬锋的原始的
//
////
//#include <windows.h>
//#include "OSGEditor/Base.h"
//#include "OSGEditor/Unitl.h"
//
//#include <iostream>
//#include "OSGEditor/OsgEngine.h"
//
//
//
//class KeyboardHandler :public osgGA::GUIEventHandler
//{
//
//public:
//    KeyboardHandler(OsgEngine* pOsgEngine) :m_pOsgEngine(pOsgEngine)
//    {
//        std::string modelPath /*= "D:\\works\\src\\OSG\\OpenSceneGraph-Data\\lz.osg"*/;
//        m_pModelNode = pOsgEngine->LoadOsgModel(modelPath);
//        pOsgEngine->DrawBoundBox(m_pModelNode);
//        pOsgEngine->LookAtModel(m_pModelNode, ModelViewType::MODEL_FRONT);
//    }
//    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object* obj, osg::NodeVisitor* nv)
//    {
//        osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
//        if (!viewer)return false;
//
//        switch (ea.getEventType())
//        {
//        case osgGA::GUIEventAdapter::KEYDOWN:
//            if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Space)
//            {
//
//            }
//            else
//            {
//                if (ea.getKey() == '0')
//                {
//                    string a;
//                    //m_pOsgEngine->LoadOsgModel(a);
//                    m_pOsgEngine->LookAtModel(m_pModelNode, ModelViewType::MODEL_FRONT);
//                }
//                else if (ea.getKey() == '1')
//                {
//                    string a;
//                    //m_pOsgEngine->LoadOsgModel(a);
//                    m_pOsgEngine->LookAtModel(m_pModelNode, ModelViewType::MODEL_BACK);
//                }
//                else if (ea.getKey() == '2')
//                {
//                    m_pOsgEngine->LookAtModel(m_pModelNode, ModelViewType::MODEL_LEFT);
//                }
//                else if (ea.getKey() == '3')
//                {
//                    m_pOsgEngine->LookAtModel(m_pModelNode, ModelViewType::MODEL_RIGHT);
//                }
//                else if (ea.getKey() == '4')
//                {
//                    m_pOsgEngine->LookAtModel(m_pModelNode, ModelViewType::MODEL_UP);
//                }
//                else if (ea.getKey() == '5')
//                {
//                    m_pOsgEngine->LookAtModel(m_pModelNode, ModelViewType::MODEL_DOWN);
//                }
//                return true;
//            }
//            break;
//        default:break;
//        }
//        return false;
//    }
//
//private:
//    OsgEngine* m_pOsgEngine;
//    osg::ref_ptr<osg::Node>m_pModelNode;
//};
//
//void Stringsplit(const string& str, const char split, vector<float>& res)
//{
//    if (str == "")      return;
//
//    string strs = str + split;
//    size_t pos = strs.find(split);
//
//    while (pos != strs.npos)
//    {
//        string temp = strs.substr(0, pos);
//        if (!temp.empty())
//        {
//            res.push_back(std::stof(temp));
//        }
//
//        strs = strs.substr(pos + 1, strs.size());
//        pos = strs.find(split);
//    }
//}
//
////test 
//int main()
//{
//    OsgEngine* pOsgEngine = OsgEngine::getInstance();
//
//    pOsgEngine->initViewer();
//
//
//    //添加相机视椎体
//    {
//        std::string tmpstr = Unitl::GetCurrentDir() + "/Data/Image/test.png";
//
//        //相机旋转矩阵
//        ST_CAMERA_INFO stCamera;
//        stCamera.mt.set(
//            0.1213521510362625, 0.99260920286178589, -0.00077269220491870002, 0,
//            0.83161652088165283, -0.1012448072433472, 0.54604345560073853, 0,
//            0.54192954301834106, -0.066906131803989397, -0.83775651454925537, 0,
//            0, 0, 0, 1);
//        stCamera.Center.set(1392.4157728820896, -70.270492220997596, 965.27860559475539);
//        stCamera.Image_Width = 6000;
//        stCamera.Image_Height = 4000;
//        stCamera.FocalPixel = 8880.79;
//        stCamera.Depth = 267.573384;
//        stCamera.ImagePath = tmpstr;
//
//        osg::ref_ptr<osg::Node> pNode = pOsgEngine->AddCameraCentrum(stCamera);
//        //pOsgEngine->Remove(pNode);
//    }
//
//    //控制点
//    {
//        std::string tmpstr = "name";
//        osg::Vec3 center(151.243, -94.2677, 800);
//        osg::ref_ptr<osg::Node> pNode1 = pOsgEngine->AddControlPoint(center, tmpstr, 1);
//
//        center = osg::Vec3(703.255, -114.686, 754.981);
//        osg::ref_ptr<osg::Node> pNode2 = pOsgEngine->AddControlPoint(center, tmpstr, 2);
//        //pOsgEngine->Remove(pNode2);
//    }
//
//    //point cloud
//    {
//        osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
//
//        osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
//        ifstream ff;
//        std::string ptfile = Unitl::GetCurrentDir() + "/data/block_AT-TY2500pt.txt";
//        ff.open(ptfile);
//
//        std::string len;
//        getline(ff, len);
//        while (getline(ff, len))    // 以split为分隔符
//        {
//            vector<float> res;
//            Stringsplit(len, ' ', res);
//            vertices->push_back(osg::Vec3(res[0], res[1], res[2]));
//            colors->push_back(osg::Vec4(res[3] / 255, res[4] / 255, res[5] / 255, 1.f));
//        }
//
//        osg::ref_ptr<osg::Node> pNode = pOsgEngine->LoadPointCloud(*vertices, *colors);
//        osg::ref_ptr<osg::Node> pBox = pOsgEngine->DrawBoundBox(pNode);
//        pOsgEngine->LookAtModel(pNode, ModelViewType::MODEL_FRONT);
//
//        //删除点云
//        //pOsgEngine->Remove(pNode);
//        //pOsgEngine->Remove(pBox);
//    }
//
//
//    pOsgEngine->Run();
//
//    return 0;
//}
//

//0831

//
#include <windows.h>
#include "OSGEditor/Base.h"
#include "OSGEditor/Unitl.h"

#include <iostream>
#include "OSGEditor/OsgEngine.h"
#include "OSGEditor/Base.h"
#include "OSGEditor/EventManager.h"


class KeyboardHandler :public osgGA::GUIEventHandler
{

public:
    KeyboardHandler(OsgEngine* pOsgEngine) :m_pOsgEngine(pOsgEngine)
    {
        std::string modelPath = "D:\\works\\src\\OSG\\OpenSceneGraph-Data\\lz.osg";
        m_pModelNode = pOsgEngine->LoadOsgModel(modelPath);
        pOsgEngine->DrawBoundBox(m_pModelNode);
        pOsgEngine->LookAtModel(m_pModelNode, ModelViewType::MODEL_FRONT);
    }
    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object* obj, osg::NodeVisitor* nv)
    {
        osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
        if (!viewer)return false;

        switch (ea.getEventType())
        {
        case osgGA::GUIEventAdapter::KEYDOWN:
            if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Space)
            {

            }
            else
            {
                if (ea.getKey() == '0')
                {
                    string a;
                    //m_pOsgEngine->LoadOsgModel(a);
                    m_pOsgEngine->LookAtModel(m_pModelNode, ModelViewType::MODEL_FRONT);
                }
                else if (ea.getKey() == '1')
                {
                    string a;
                    //m_pOsgEngine->LoadOsgModel(a);
                    m_pOsgEngine->LookAtModel(m_pModelNode, ModelViewType::MODEL_BACK);
                }
                else if (ea.getKey() == '2')
                {
                    m_pOsgEngine->LookAtModel(m_pModelNode, ModelViewType::MODEL_LEFT);
                }
                else if (ea.getKey() == '3')
                {
                    m_pOsgEngine->LookAtModel(m_pModelNode, ModelViewType::MODEL_RIGHT);
                }
                else if (ea.getKey() == '4')
                {
                    m_pOsgEngine->LookAtModel(m_pModelNode, ModelViewType::MODEL_UP);
                }
                else if (ea.getKey() == '5')
                {
                    m_pOsgEngine->LookAtModel(m_pModelNode, ModelViewType::MODEL_DOWN);
                }
                return true;
            }
            break;
        default:break;
        }
        return false;
    }

private:
    OsgEngine* m_pOsgEngine;
    osg::ref_ptr<osg::Node>m_pModelNode;
};

void Stringsplit(const string& str, const char split, vector<float>& res)
{
    if (str == "")      return;

    string strs = str + split;
    size_t pos = strs.find(split);

    while (pos != strs.npos)
    {
        string temp = strs.substr(0, pos);
        if (!temp.empty())
        {
            res.push_back(std::stof(temp));
        }

        strs = strs.substr(pos + 1, strs.size());
        pos = strs.find(split);
    }
}

//call back event test
class CallbackEventTest : public EventBaseServer
{
public:
    CallbackEventTest() {};
    ~CallbackEventTest() {};

    virtual void callBackEvent(CALLBACK_EVENT_TYPE type, const EventInfo& info)
    {

        if (type == CALLBACK_EVENT_TYPE::CALL_BACK_PICK_PHOTO)
        {
            st_photo* stPhoto = (st_photo*)info.getEventInfo();
            if (stPhoto)
            {
                std::cout << stPhoto->ID << " " << stPhoto->name << std::endl;
            }
        }

        if (type == CALLBACK_EVENT_TYPE::CALL_BACK_SELECT_PHOTO)
        {
            std::vector<st_photo>* stPhoto = (std::vector<st_photo>*)info.getEventInfo();
            for (auto it : *stPhoto)
            {
                std::cout << it.ID << " " << it.name << std::endl;
            }
        }
    }

private:


};

void PhotoElementTest()
{
    OsgEngine* pOsgEngine = OsgEngine::getInstance();

    std::string tmpstr = Unitl::GetCurrentDir() + "/Data/Image/test.png";
    int index = 0;
    float offset = 300;

    /**/

    //空三前无相机
    {
        ST_CAMERA_INFO   stCamera;
        stCamera.ID = index++;
        stCamera.Center.set(offset, -70.270492220997596, 965.27860559475539);
        stCamera.Image_Width = 6000;
        stCamera.Image_Height = 4000;
        stCamera.FocalPixel = 8880.79;
        stCamera.ImagePath = tmpstr;
        stCamera.aerType = Aerotriangulation_Type::AER_Front;

        pOsgEngine->AddPhotos(stCamera);

    }
    //空三前有相机
    {
        ST_CAMERA_INFO   stCamera;
        stCamera.mt.set(
            0.1213521510362625, 0.99260920286178589, -0.00077269220491870002, 0,
            0.83161652088165283, -0.1012448072433472, 0.54604345560073853, 0,
            0.54192954301834106, -0.066906131803989397, -0.83775651454925537, 0,
            0, 0, 0, 1);

        stCamera.ID = index++;
        stCamera.Center.set(offset + 100, -70.270492220997596, 965.27860559475539);
        stCamera.Image_Width = 6000;
        stCamera.Image_Height = 4000;
        stCamera.FocalPixel = 8880.79;
        stCamera.Depth = 267.573384;
        stCamera.ImagePath = tmpstr;
        stCamera.aerType = Aerotriangulation_Type::AER_Front;

        pOsgEngine->AddPhotos(stCamera);

    }
    //空三后无相机
    {
        ST_CAMERA_INFO stCamera;
        stCamera.Center.set(offset + 200, -70.270492220997596, 965.27860559475539);
        stCamera.Image_Width = 6000;
        stCamera.Image_Height = 4000;
        stCamera.FocalPixel = 8880.79;
        stCamera.ImagePath = tmpstr;
        stCamera.ID = index++;
        stCamera.aerType = Aerotriangulation_Type::AER_Back_Fail;
        pOsgEngine->AddPhotos(stCamera);
    }

    //空三后失败
    {
        ST_CAMERA_INFO stCamera;
        stCamera.mt.set(
            0.1213521510362625, 0.99260920286178589, -0.00077269220491870002, 0,
            0.83161652088165283, -0.1012448072433472, 0.54604345560073853, 0,
            0.54192954301834106, -0.066906131803989397, -0.83775651454925537, 0,
            0, 0, 0, 1);
        stCamera.Center.set(offset + 300, -70.270492220997596, 965.27860559475539);
        stCamera.Image_Width = 6000;
        stCamera.Image_Height = 4000;
        stCamera.FocalPixel = 8880.79;
        stCamera.Depth = 267.573384;
        stCamera.ImagePath = tmpstr;
        stCamera.ID = index++;
        stCamera.aerType = Aerotriangulation_Type::AER_Back_Fail;

        pOsgEngine->AddPhotos(stCamera);
    }
    //空三后成功
    {

        ST_CAMERA_INFO stCamera;
        stCamera.mt.set(
            0.1213521510362625, 0.99260920286178589, -0.00077269220491870002, 0,
            0.83161652088165283, -0.1012448072433472, 0.54604345560073853, 0,
            0.54192954301834106, -0.066906131803989397, -0.83775651454925537, 0,
            0, 0, 0, 1);
        stCamera.Center.set(offset + 400, -70.270492220997596, 965.27860559475539);
        stCamera.Image_Width = 6000;
        stCamera.Image_Height = 4000;
        stCamera.FocalPixel = 8880.79;
        stCamera.Depth = 267.573384;
        stCamera.ImagePath = tmpstr;
        stCamera.ID = index++;
        stCamera.aerType = Aerotriangulation_Type::AER_Back_Success;

        pOsgEngine->AddPhotos(stCamera);
    }


}
//test 
int main()
{
    OsgEngine* pOsgEngine = OsgEngine::getInstance();

    pOsgEngine->initViewer(0, 0, 1960, 1080);

    //设置当前要素图层
    pOsgEngine->SetElementType(Element_Type::ELEMENT_PHOTOS);
    pOsgEngine->SetSelectType(Select_Type::SELECT_ONE);
    //回调事件注册

    CallbackEventTest* pCallbackEventTest = new CallbackEventTest;
    EventManager::GetInstance()->registerEvent(CALL_BACK_PICK_PHOTO, pCallbackEventTest);
    EventManager::GetInstance()->registerEvent(CALL_BACK_SELECT_PHOTO, pCallbackEventTest);


    //model
    //std::string modelPath = "D:\\works\\src\\OSG\\OpenSceneGraph-Data\\lz.osg";
    //osg::ref_ptr<osg::Node> loadedModel = pOsgEngine->LoadOsgModel(modelPath);
    //pOsgEngine->DrawBoundBox(loadedModel);
    ////设置相机位置，面向模型
    //pOsgEngine->LookAtModel(loadedModel);

    //添加相机视椎体
    {
        PhotoElementTest();
        //pOsgEngine->Remove(pNode);
    }

    //控制点
    {
        osg::ref_ptr<osg::Node> pNode1 = pOsgEngine->AddControlPoint(1, osg::Vec3(151.243, -94.2677, 1000), "Image_1", 1);
        osg::ref_ptr<osg::Node> pNode2 = pOsgEngine->AddControlPoint(2, osg::Vec3(200.243, -94.2677, 1000), "Image_2", 2);
        //pOsgEngine->Remove(pNode2);
    }

    //点云
    {
        osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;

        osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
        ifstream ff;
        ff.open(Unitl::GetCurrentDir() + "/data/block_AT-TY2500pt.txt");

        std::string len;
        getline(ff, len);
        while (getline(ff, len))    // 以split为分隔符
        {
            vector<float> res;
            Stringsplit(len, ' ', res);
            vertices->push_back(osg::Vec3(res[0], res[1], res[2]));
            colors->push_back(osg::Vec4(res[3] / 255, res[4] / 255, res[5] / 255, 1.f));
        }

        osg::ref_ptr<osg::Node> pNode = pOsgEngine->LoadPointCloud(1, *vertices, *colors);
        //osg::ref_ptr<osg::Node> pBox = pOsgEngine->DrawBoundBox(pNode);
        pOsgEngine->LookAtModel(pNode, ModelViewType::MODEL_UP);

        //删除点云
        //pOsgEngine->Remove(pNode);
        //pOsgEngine->Remove(pBox);
    }


    //bounding box test
    {
        ST_BOUNDINGBOX box;
        box.ID = 1;
        box.minXYZ.set(-100, -100, -100);
        box.maxXYZ.set(0, 0, 0);
        box.type = 1;

        ST_BOUNDINGBOX box2;
        box2.ID = 2;
        box2.minXYZ.set(0, 0, 0);
        box2.maxXYZ.set(100, 100, 100);
        box2.type = 2;

        std::vector<ST_BOUNDINGBOX> vectBox;
        vectBox.emplace_back(box);
        vectBox.emplace_back(box2);

        pOsgEngine->DrawBoundBox(vectBox);
    }

    pOsgEngine->Run();

    return 0;
}

