

#include <iostream>
#include <fstream>
#include "Core/ATData.h"
#include "Core/ProjectObject.h"
#include "Core/ControlPoint.h"
#include <set>
#include "Core/CamerasGenerator.h"
#include "Core/CoordinateSystem.h"
#include "Core/ExifIO.h"
//#include "base/gps.h"
using namespace AI3D::CORE;
//需要测试的几项
// 测试排序。测试删除后id
 
/*
bool TransFormBetween4326AndECEF(double& x, double& y, double& z, bool from4326 = true)
{
    std::shared_ptr<CoordinateDescriptor> cs_descriptor;
    cs_descriptor.reset(new CoordinateDescriptor);
    std::shared_ptr<CoordinateTransformer> cs_transformer;

    if (from4326)
    {
        bool ret = cs_descriptor->InitialFromEPSG(4326);
        std::cout << ret << std::endl;
        std::shared_ptr<CoordinateDescriptor> src_descriptor;// = std::make_shared<CoordinateDescriptor>(4978);
        src_descriptor.reset(new CoordinateDescriptor);
        src_descriptor->InitialFromEPSG(4978);
        cs_transformer.reset(new CoordinateTransformer(cs_descriptor, src_descriptor));
        if (cs_transformer)
        {
            Eigen::Vector3d dst;
            cs_transformer->Transform(x, y, z, dst(0), dst(1), dst(2));
            x = dst(0);
            y = dst(1);
            z = dst(2);
            return true;
        }
    }
    else
    {
        cs_descriptor->InitialFromEPSG(4978);
        std::shared_ptr<CoordinateDescriptor> src_descriptor = std::make_shared<CoordinateDescriptor>(4326);
        cs_transformer.reset(new CoordinateTransformer(cs_descriptor, src_descriptor));
        if (cs_transformer)
        {
            Eigen::Vector3d dst;
            cs_transformer->Transform(x, y, z, dst(0), dst(1), dst(2));
            x = dst(0);
            y = dst(1);
            z = dst(2);
            return true;
        }
    }
    return false;
}
*/

int main(int argc, char** argv)
{
    //colmap::GPSTransform gps(colmap::GPSTransform::WGS84);
    
    
    //InitialGDAL("K:/datarecon_git_online_2021_08_27_git2/third_party/Windows/vc141/gdal/3.0.0/data");
    double x1 = 105.017583; //113.138392;// 
    double y1 = 29.406829; //40.974972;// 29.406829;
    double z1 = 347.860459; //1592.90000000;// 347.860459;
    //Eigen::Vector3d ell(y1,x1,z1),bb;
    //std::vector<Eigen::Vector3d> a,b;
    //a.push_back(ell);
    //bb = gps.LLAToUTM(ell);
    //b = gps.EllToXYZ(a);
    //TransFormBetween4326AndECEF(y1,x1,z1);
   
    //std::cout << std::setprecision(19)<< " x " << bb.x() << " y" << bb.y() << "  z " << bb.z() << std::endl;
    //std::cout << " x " << b[0].x() << " y" << b[0].y() << "  z " << b[0].z() << std::endl;
    std::cout << std::setprecision(19) << " x " << x1 << " y" << y1 << "  z " << z1 << std::endl;

    /*
    //std::string file1 = "e:/1.jpg";
    //std::string file2 = "e:/2.jpg";
    //std::string file3 = "e:/3.jpg";
    ControlPoints gcps;
    //导入后变换到基准坐标系下；
    gcps.LoadText();
    //给每个点添加观测值
    BlockObject block;
    //添加9张影像
    std::vector<std::string> imagefiles =
    {
        "d:/1.jpg",
        "d:/2.jpg",
        "d:/3.jpg",
        "d:/4.jpg",
        "d:/5.jpg",
        "d:/6.jpg",
        "d:/7.jpg",
        "d:/8.jpg",
        "d:/9.jpg"
    };
    int count = 0;
    std::vector<IndexImage> indexImages;
    for (auto img : imagefiles)
    {
        
        IndexImage index_image(count, img);
        indexImages.push_back(index_image);
        count++;
    }
    //第一個观测值
    Image image;
    image.SetImageId(indexImages[0].id_);
    Eigen::Vector3d xyz(110.0, 11.0, 11.0);
   // image.SetPosition(xyz);

    Eigen::Vector2d x(1000.0, 2000.0);
    point2D_t ptid = image.AddPoints2D(x);
    TrackElement ele(image.GetImageId(), ptid);
    std::vector<TrackElement> vec_track_ele;
    vec_track_ele.push_back(ele);

    //第二個观测值
    Image image1;
    image1.SetImageId(indexImages[1].id_);
    x.x() = 1000.0;
    x.y() = 3000.0;
    ptid = image1.AddPoints2D(x);
    ele.image_id = image1.GetImageId();
    ele.point2D_idx = ptid;
    vec_track_ele.push_back(ele);

    //第三個观测值
    Image image2;
    image2.SetImageId(indexImages[2].id_);
    x.x() = 1500.0;
    x.y() = 3500.0;
    ptid = image2.AddPoints2D(x);
    ele.image_id = image2.GetImageId();
    ele.point2D_idx = ptid;
    vec_track_ele.push_back(ele);

    ControlPoint& gcp0 = gcps.GetPointsMutual().at(0);
    Track track;
    track.AddElements(vec_track_ele);
    gcp0.GetObjectPoint().get()->SetTrack(track);

    //第二个控制点
    //第一個观测值
    Image image3;
    image3.SetImageId(indexImages[3].id_);
    x.x() = 1000.0;
    x.y() = 3000.0;
    ptid = image3.AddPoints2D(x);
    ele.image_id = image3.GetImageId();
    ele.point2D_idx = ptid;
    std::vector<TrackElement> vec_track_ele1;
    vec_track_ele1.push_back(ele);

    //第二個观测值
    Image image4;
    image4.SetImageId(indexImages[4].id_);
    x.x() = 1000.0;
    x.y() = 3000.0;
    ptid = image4.AddPoints2D(x);
    ele.image_id = image4.GetImageId();
    ele.point2D_idx = ptid;
    vec_track_ele1.push_back(ele);

    //第三個观测值
    Image image5;
    image5.SetImageId(indexImages[5].id_);
    x.x() = 1500.0;
    x.y() = 3500.0;
    ptid = image5.AddPoints2D(x);
    ele.image_id = image5.GetImageId();
    ele.point2D_idx = ptid;
    vec_track_ele1.push_back(ele);

    ControlPoint& gcp1 = gcps.GetPointsMutual().at(1);
    Track track1;
    track1.AddElements(vec_track_ele1);
    gcp1.GetObjectPoint().get()->SetTrack(track1);


    //第三个控制点
    //第一個观测值
    Image image6;
    image6.SetImageId(indexImages[6].id_);
    x.x() = 1000.0;
    x.y() = 3000.0;
    ptid = image6.AddPoints2D(x);
    ele.image_id = image6.GetImageId();
    ele.point2D_idx = ptid;
    std::vector<TrackElement> vec_track_ele2;
    vec_track_ele2.push_back(ele);

    //第二個观测值
    Image image7;
    image4.SetImageId(indexImages[7].id_);
    x.x() = 1000.0;
    x.y() = 3000.0;
    ptid = image7.AddPoints2D(x);
    ele.image_id = image7.GetImageId();
    ele.point2D_idx = ptid;
    vec_track_ele2.push_back(ele);

    //第三個观测值
    Image image8;
    image8.SetImageId(indexImages[8].id_);
    x.x() = 1500.0;
    x.y() = 3500.0;
    ptid = image8.AddPoints2D(x);
    ele.image_id = image8.GetImageId();
    ele.point2D_idx = ptid;
    vec_track_ele2.push_back(ele);

    ControlPoint& gcp2 = gcps.GetPointsMutual().at(2);
    Track track2;
    track2.AddElements(vec_track_ele2);
    gcp2.GetObjectPoint().get()->SetTrack(track2);

    gcps.SaveText();

    block.AddImage(image);
    block.AddImage(image1);
    block.AddImage(image2);
    block.AddImage(image3);
    block.AddImage(image4);
    block.AddImage(image5);
    block.AddImage(image6);
    block.AddImage(image7);
    block.AddImage(image8);
    */



   // Track track;
    

    return 0;
}