#pragma once
#include "Base.h"
#include "CustomNode.h"

class OSGEDITOR_INTERNAL_CLASS Unitl
{
public:
    Unitl();
    static osg::BoundingBox* GetBox(osg::ref_ptr<Vec3Array> vecArray);
    static void AddAxis(Geode* coord, Vec3 pt, std::string text, Vec4 color);
    static osg::ref_ptr<osg::Group> CreateBox(const osg::BoundingBox* pBox, const Vec4& color);
    static osg::ref_ptr<osg::Group> CreateBoxTile(const osg::BoundingBox* pBox, const Vec4& color);

    static osg::ref_ptr<osg::Node> CreateCentrum(const ST_CAMERA_INFO& stCamera);

    static osg::ref_ptr<osg::Geode> CreateImage(const osg::Vec3& corner, const osg::Vec3& width, const osg::Vec3& height, osg::ref_ptr<osg::Image> image);

    static osg::ref_ptr<osg::Geode> CreateLineGeometry(osg::Vec3Array* array, osg::Vec4 color);

    static osg::ref_ptr<osg::Geode> CreatePlane(osg::ref_ptr<osg::Vec3Array> vertex, const Vec4& color, const std::string& name = "");

    static osg::ref_ptr<osg::Geode> CreatePolygonBorder(const PolygonBox& box, const Vec4& color);

    static osg::ref_ptr<osg::Geode> CreatePolygon(osg::ref_ptr<osg::Vec3Array> vertex, const Vec4& color, const std::string& name = "");

    static std::string GetCurrentDir();

    static osg::Vec4 FromHex(const std::string& HexString);

    static osg::ref_ptr<osg::Node> CreateBoxBorder(const osg::BoundingBox* pBox, const Vec4& color);

    static osg::ref_ptr<osg::Node> CreateBoxBorderTile(const osg::BoundingBox* pBox, const Vec4& color);

    static osg::ref_ptr<osg::Node> HoverText(const std::string& name, const osg::Vec3& location);
    static bool IsPointInPolygon(const osg::Vec3& point, const std::vector<osg::Vec3>& polygon);

    static bool IsPointInsideBoundingBox(const osg::Vec3& point, const osg::Vec3& minCorner, const osg::Vec3& maxCorner);

    static void Log(const osg::Vec3& vec)
    {
        std::cout << "x: " << vec.x() << " y: " << vec.y() << " z: " << vec.z() << std::endl;
    };
    
    static osg::ref_ptr<osg::AutoTransform> AddAxisText(Vec3 pt, std::string text, float textlenth = 6.);
};


