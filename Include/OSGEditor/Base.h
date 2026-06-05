/*! @file
********************************************************************************
<PRE>
模块名       : <EarthEngine>
文件名       : <Base.h>
相关文件     : <>
文件实现功能 : <公共声明>
作者         : <bfzhao>
版本         : <1.0>
日期         : 2014.10.20
</PRE>
*******************************************************************************/
#ifndef _BASE_H_
#define _BASE_H_
#include <windows.h> 

#include <osgDB/readfile>
#include <osgDB/writefile>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/object>
#include <osg/Node>
#include <osg/group>
//#include <osg/fog>
#include <osg/view>
#include <osg/MatrixTransform>  
#include <osg/ProxyNode>
//#include <osg/Image>
#include <osg/StateSet>
#include <osg/TextureCubeMap>
#include <osgText/Text>
#include <osg/TexGen>
#include <osg/TexEnvCombine>
#include <osg/BlendColor>
#include <osg/BlendFunc>
#include <osg/PositionAttitudeTransform>

#include <osg/ImageStream>
#include <osg/CullFace>
#include <osg/Point>
#include <osg/Material>
#include <osg/AutoTransform>
//#include <osg/LineStipple>
//#include <osgFX/Outline>

#include <osg/ComputeBoundsVisitor>
#include <osg/DeleteHandler>
#include <osg/ShapeDrawable>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <osgViewer/Viewer>
#include <osgViewer/Renderer>
#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>  
#include <osgUtil/optimizer>
#include <osgUtil/HighlightMapGenerator>
#include <osgUtil/LineSegmentIntersector>
#include <osgUtil/SceneView>

#include <osgGA/StateSetManipulator>  
#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIEventHandler>
#include <osgGA/TrackballManipulator>
#include <osgSim/ShapeAttribute>



#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <Windows.h>
#include <iomanip>

using namespace osg;
using namespace osgSim;
using namespace osgViewer;


using namespace std;

#ifdef OSGUTIL_RENDERBACKEND_USE_REF_PTR
#undef OSGUTIL_RENDERBACKEND_USE_REF_PTR
#endif

#define MAX_DISTANCE 25000000

#ifdef DLL_API
#undef DLL_API
#endif

#if defined(MoldAIOSGEditor_EXPORTS)
#define OSGEDITOR_DLL_EXPORT __declspec(dllexport)
#else
#define OSGEDITOR_DLL_EXPORT __declspec(dllimport)
#endif

#define DLL_API OSGEDITOR_DLL_EXPORT
#define OSGEDITOR_DLL_API_GUARD

// DLL export policy (MoldAIOSGEditor):
// - OSGEDITOR_CLASS_API: only on OsgEngine, EventInfo, EventBaseServer, EventManager.
//   Do NOT use "class DLL_API" (MSVC C4273). Implementations: include OsgEditorDllBuild.h first.
// - OSGEDITOR_INTERNAL_CLASS: PhotosNodeManager, CustomNode, PointNode, Unitl, LODTree, ...
//   No dllexport/dllimport; use OsgEngine facade APIs from Gui/other DLLs.
#if defined(MoldAIOSGEditor_EXPORTS)
#define OSGEDITOR_CLASS_API __declspec(dllexport)
#else
#define OSGEDITOR_CLASS_API __declspec(dllimport)
#endif
#define OSGEDITOR_INTERNAL_CLASS

#if defined(_MSC_VER)
#ifndef OSGEDITOR_CPP_METHOD
#define OSGEDITOR_CPP_METHOD OSGEDITOR_DLL_EXPORT
#endif
#else
#ifndef OSGEDITOR_CPP_METHOD
#define OSGEDITOR_CPP_METHOD
#endif
#endif

template <class T>
T convertFromString(T &value, const std::string &s) 
{
    if (s.length() == 0 || s.empty())
    {
        value = 0;
        return 0;
    }

    std::stringstream ss(s);
    ss >> value;
    return value;
};

template <class T> string convertToString(T &value, int precision = 9)
{
        std::stringstream ss;
        ss << std::setprecision(precision) << value;
        return ss.str();
}

static FORCEINLINE int HexDigit(TCHAR c)
{
    int Result = 0;

    if (c >= '0' && c <= '9')
    {
        Result = c - '0';
    }
    else if (c >= 'a' && c <= 'f')
    {
        Result = c + 10 - 'a';
    }
    else if (c >= 'A' && c <= 'F')
    {
        Result = c + 10 - 'A';
    }
    else
    {
        Result = 0;
    }

    return Result;
}

typedef enum ENGINE_TYPE
{
    TYPE_AEROTRIANGULATION = 0X01,
    TYPE_REBUILD = 0X02
}EngineType;

//回调事件类型
//@ 分别是代表什么操作
enum CALLBACK_EVENT_TYPE {
    CALL_BACK_NONE = 0,
    CALL_BACK_SELECT_PHOTO = 1, //相片选择事件回调类型
    CALL_BACK_SELECT_TILE,       //切片选择事件回调类型
    CALL_BACK_ROI_BOX_DRAG,        //兴趣框拖拽事件回调
    CALL_BACK_ROI_POLYGON_DRAG,   //兴趣框多面体拖拽事件回调
    CALL_BACK_CAMERA, 
    CALL_BACK_TIEPOINT,            //点云拾取回调
    CALL_BACK_REMOVE_PHOTO,
    CALL_BACK_REMOVE_TIEPOINTS,
    CALL_BACK_REMOVE,
    CALL_BACK_RIGHT_SELECT_PHOTO,
    CALL_BACK_SELECT_PHOTO_WINDOWS,// 选中照片后右鍵事件
	CALL_BACK_OSGB_LOADED				  
};

//要素类型定义
typedef enum ELEMENT_LAYER_TYPE
{
    ELEMENT_NONE = 0x00,
    ELEMENT_PHOTOS = 1,  //照片、相机
    ELEMENT_TIEPOINTS = 1 << 2,     //点云
    ELEMENT_SURVEY_POINTS = 1 << 3, //像控点
    ELEMENT_TILE = 1 << 4,      //切块
    ELEMENT_ROI = 1 << 5,       //兴趣框
    ELEMENT_MODEL = 1<<6  ,      //普通3D模型
    ELEMENT_POLYGON = 1 << 7      //多边形/水域约束
}Element_Type;
//选择类型定义
typedef enum SELECT_TYPE {
    SELECT_NONE = 0X00,
    SELECT_ONE = 0X01,  //单选
    SELECT_BOX = 0X02,  //框选
    SELECT_POLYGON = 0X04   //多边形选择
}Select_Type;

//空三类型定义
typedef enum AEROTRIANGULATION_TYPE
{
    AER_NONE = 0X00,
     AER_FRONT = 0X01,       //空三前类型
    AER_BACK = 0X02,        //空三后默认类型
    AER_BACK_FAIL = 0X04,   //空三后失败
    AER_BACK_SUCCESS = 0X08 //空三后成功
}Aerotriangulation_Type;

typedef enum AEROTRIANGULATION_POSE_TYPE
{
    AER_POSE_NONE = 0X00,
    AER_NOTCALIBRATED_IN_NONCALIBRATEDBLOCK = 0X01,       //空三前类型
    AER_CALIBRATED_NOTIEPOINTS = 0X02,        //空三后但是没有tiepoints
    AER_NOTCALIBRATED_IN_CALIBRATEDBLOCK = 0X04,   //空三后失败
    AER_CALIBRATED = 0X08 //空三后成功
}Aerotriangulation_POSE_Type;



typedef enum MOUSE_TYPE
{
    MOUSE_NONE = 0X00,
    MOUSE_HOVER = 0X01,  //鼠标hover类型
    MOUSE_PICKED = 0X02  //鼠标点选类型
}Mouse_Type;

//视图方向定义
enum ModelViewType
{
    MODEL_FRONT = 0X01,   //正前方
   MODEL_BACK,           //背面
   MODEL_LEFT,           //左
   MODEL_RIGHT,          //右
   MODEL_UP,             //上
   MODEL_DOWN            //下
};


enum SECENE_OPERATION_TYPE
{
    SECENE_MODE_NONE = 0,
    SECENE_MODE_MOVE,  //平移
    SECENE_MODE_ROTATE     //旋转
};

//回调事件信息结构
typedef struct   CallBack_Element_Info
{
    int ID;
    int photoID;
    std::string name;

    osg::BoundingBox bbox;

    CallBack_Element_Info() {
        ID = 0;
        name = "";
        photoID = 0;
    };

    CallBack_Element_Info(const int id, const string& name)
    {
        this->ID = id;
        this->name = name;
    }

}ST_CALLBACK_ELEMENT_INFO;



//照片、相机属性结构
typedef struct CameraInfo
{
    int ID;
    double Image_Width;
    double Image_Height;
    double FocalPixel;
    std::string ImagePath;
    osg::Matrix mt;
    osg::Vec3 Center;
    float Depth;
    Aerotriangulation_Type aerType;
    float mSize;
}ST_CAMERA_INFO;

typedef struct BoundingBox {
    int ID;
     std::string name;
    Vec3 minXYZ;
    Vec3 maxXYZ;
    int type; //1: 普通box  2:tile box
}ST_BOUNDINGBOX;
//多面体box
typedef struct PolygonBox {
    int ID;
    std::string name;
    std::vector<Vec3> points;
    float minHeight;
    float maxHeight;
}ST_POLYGON_BOX;
#define RENDERPIINT_MINSIZE 0.1
#define RENDERPIINT_DEFAULTSIZE 0.5
 
//点云结构体
typedef struct TiePoint {
    int ID;                    // necessary, batch id

    std::string name;          //点云名称  optional
    float size = RENDERPIINT_DEFAULTSIZE;                //点大小
    osg::ref_ptr<osg::Vec3Array> points;     //点位置
    osg::ref_ptr<osg::Vec4Array> colors;     //点颜色
    std::vector<std::pair<int, std::vector<osg::Vec3> >> IDRelevancyPhoto;     //点和照片关系 <点ID，影像ID>
    std::map<int, std::vector<int> > PhotoRelevancyID;                         //照片和点关系 <影像ID，点ID>                        //照片和点关系
    std::map<int, int> PointIDRelevancyIndex;  //点id和点索引关系<点ID，索引>
    TiePoint() {
        ID = 0;
        name = "";
        size = RENDERPIINT_DEFAULTSIZE;

    }

}ST_TIEPOINT;

struct CameraCentrum
{
    ST_CAMERA_INFO cameraInfo;
    osg::Matrix mt;
    osg::Vec3 location;     //相机中心位置
    osg::Vec3 center;       //远平面中心点
    osg::ref_ptr<osg::Vec3Array> FrontPlane = new osg::Vec3Array;    //远平面顶点[0]leftBottom [1]rightBottom [2]rightTop [3] leftTop
    osg::ref_ptr<osg::Vec3Array> BackPlane = new osg::Vec3Array;    //卡片顶点 [0]leftBottom [1]rightBottom [2]rightTop [3] leftTop
    bool bCentrum;
};                  


namespace EditerEngine
{
    //空三前
    static std::vector<std::string> FrontDefaultColor = { "3A91DC"/*卡片颜色*/,"FFED88"/*视椎体颜色*/ };
    static std::vector<std::string> FrontHoverColor = { "DA5454","F7A56F" };
      
    static std::vector<std::string> FrontSelectedColor = { "AA1515","7B3100" };
    //空三后失败
    static std::vector<std::string> BackFailDefaultColor = { "EF4A267F","FFFFFF" };
    static std::vector<std::string> BackFailHoverColor = { "FF69067F","FF6906" };
    static std::vector<std::string> BackFailSelectedColor = { "4949497F","434343" };
    //空三后成功
    static std::vector<std::string> BackDefaultColor = { "59FF534D","FFED88" };//"FFFFFF"
    static std::vector<std::string> BackHoverColor = { "EBFF534D","FFFFFF" };//"FFED88"
    static std::vector<std::string> BackSelectedColor = {"FF68534D","DA4A4A"};// { "12FFD51E", "12FFD51E" };//FF68534D


    //点云
    static std::vector<std::string> TiePointColor = { "FFED88"/*hover*/, "3A91DC"/*picked*/, "3A91DC" /*多选颜色*/, "43F8EE"/*连接线颜色*/ };// { "FFED88"/*hover*/, "12FFD5"/*picked*/, "12FFD5" /*多选颜色*/, "43F8EE"/*连接线颜色*/ };


    //框选多边形颜色
    static std::vector<std::string> PolygonColor = { "12FFD5","12FFD526" };
    //box color
    static std::vector<std::string> BoundingBoxColor = { "4AE9FF","4AE9FF4D" };
    //Tile box color
    static std::vector<std::string> TileDefaultColor = { "8F8F8F","FFFFFF00" };
    static std::vector<std::string> TileHoverColor = { "74EEBF","" };
    static std::vector<std::string> TileSelectColor = { "FFFFFF","FFFFFF66" };

    //ROI兴趣框
    static std::vector<std::string> ROIDefaultColor = { "4AE9FF","4AE9FF00" };
    static std::vector<std::string> ROIHoverColor = { "4AE9FF","4AE9FF4D" };
    static std::vector<std::string> ROISelectColor = { "","" };
    //多边形水域
    static std::vector<std::string> PolygonWaterDefaultColor = { "23FFAB","23FFAB33" };

};


#endif