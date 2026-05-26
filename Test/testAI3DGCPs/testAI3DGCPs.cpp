

#include <iostream>
#include <fstream>
#include "Core/ATData.h"
#include "Core/ProjectObject.h"
#include "Core/ControlPoint.h"
#include <set>
#include "Core/CamerasGenerator.h"
//#include "Core/ExifIO.h"
using namespace AI3D::CORE;
//需要测试的几项
// 测试排序。测试删除后id
 




int main(int argc, char** argv)
{
    

    std::string file1 = "e:/1.jpg";
    std::string file2 = "e:/2.jpg";
    std::string file3 = "e:/3.jpg";
    ControlPoints gcps;
    const std::string gcp_filepath_txt = "D:/MyLearning/Learning_Materials/run/camera/gcps.txt";
    const std::string gcp_intputpath_json = "D:/MyLearning/Learning_Materials/run/camera/GCP.json";
    const std::string gcp_outputpath_json = "D:/MyLearning/Learning_Materials/run/camera/GCP_Bak.json";
    //导入后变换到基准坐标系下；
    gcps.LoadText(gcp_filepath_txt);
   // if (gcps.LoadJsonToATData(gcp_intputpath_json)!=0)
    //{
   //     std::cerr << "Json文件读写错误" << std::endl;
    //}
	//if (gcps.LoadJson(gcp_intputpath_json) != 0)
	//{
	//	std::cerr << "Json文件读写错误" << std::endl;
	//}
    //给每个点添加观测值
    BlockObject block;
    std::shared_ptr<ATData> atdata;
    atdata.reset(new ATData());
    block.SetATData(atdata);
    //添加9张影像
    std::vector<std::string> imagefiles =
    {
        "D:/MyLearning/Learning_Materials/run/camera/center/NMW209XS0850.jpg",
        "D:/MyLearning/Learning_Materials/run/camera/center/NMW209XS0851.jpg",
        "D:/MyLearning/Learning_Materials/run/camera/center/NMW209XS0852.jpg",
        "D:/MyLearning/Learning_Materials/run/camera/center/NMW209XS0853.jpg",
        "D:/MyLearning/Learning_Materials/run/camera/center/NMW209XS0854.jpg",
        "D:/MyLearning/Learning_Materials/run/camera/center/NMW209XS0855.jpg",
        "D:/MyLearning/Learning_Materials/run/camera/center/NMW209XS0856.jpg",
        "D:/MyLearning/Learning_Materials/run/camera/center/NMW209XS0857.jpg",
        "D:/MyLearning/Learning_Materials/run/camera/center/NMW209XS0858.jpg",
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
	ele.xy = x;
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
	ele.xy = x;
	vec_track_ele.push_back(ele);

	//第三個观测值
	Image image2;
	image2.SetImageId(indexImages[2].id_);
	x.x() = 1500.0;
	x.y() = 3500.0;
	ptid = image2.AddPoints2D(x);
	ele.image_id = image2.GetImageId();
	ele.point2D_idx = ptid;
	ele.xy = x;
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
	ele.xy = x;
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
	ele.xy = x;
	vec_track_ele1.push_back(ele);

	//第三個观测值
	Image image5;
	image5.SetImageId(indexImages[5].id_);
	x.x() = 1500.0;
	x.y() = 3500.0;
	ptid = image5.AddPoints2D(x);
	ele.image_id = image5.GetImageId();
	ele.point2D_idx = ptid;
	ele.xy = x;
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
	ele.xy = x;
	std::vector<TrackElement> vec_track_ele2;
	vec_track_ele2.push_back(ele);

	//第二個观测值
	Image image7;
	image7.SetImageId(indexImages[7].id_);
	x.x() = 1000.0;
	x.y() = 3000.0;
	ptid = image7.AddPoints2D(x);
	ele.image_id = image7.GetImageId();
	ele.point2D_idx = ptid;
	ele.xy = x;
	vec_track_ele2.push_back(ele);

	//第三個观测值
	Image image8;
	image8.SetImageId(indexImages[8].id_);
	x.x() = 1500.0;
	x.y() = 3500.0;
	ptid = image8.AddPoints2D(x);
	ele.image_id = image8.GetImageId();
	ele.point2D_idx = ptid;
	ele.xy = x;
	vec_track_ele2.push_back(ele);

	ControlPoint& gcp2 = gcps.GetPointsMutual().at(2);
	Track track2;
	track2.AddElements(vec_track_ele2);
	gcp2.GetObjectPoint().get()->SetTrack(track2);



    block.AddImage(image);
    block.AddImage(image1);
    block.AddImage(image2);
    block.AddImage(image3);
    block.AddImage(image4);
    block.AddImage(image5);
    block.AddImage(image6);
    block.AddImage(image7);
    block.AddImage(image8);

    srs_s srs;
    // gcps.SaveText();
    gcps.SaveJsonFromATData(gcp_outputpath_json,block.GetOriginAT(),srs);
    //gcps.SaveJson(gcp_outputpath_json,srs);



    

    return 0;
}