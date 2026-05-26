
#include <sqlite3.h>
#include <iostream>
#include <fstream>
#include "Core/ProjectObject.h"
#include "Core/BlockObject.h"
#include "Core/String.h"
#include "Core/Timer.h"
#include "Core/File.h"
#include "Core/Application.h"
#include "Core/CoordinateSystem.h"
#include <glog/logging.h>

//观测值到控制点光束的垂线距离
//double cal_dist_gcpray()
//{
//    //影像
//    MVSCamera camera(MVSCamera::DISTORT_TYPE::OPENCV_K1K2K3P1P2, 100, 100, nullptr, 0, 0);
//    MVSImage img(&camera, 0, ProjectionMatrix());
//    MVSView view(&img, 0);
//
//    Point3d gcp_grd; Point2d gcd_ondist_obr;
//
//    Point2d gcp_grd_pix = view.TransformPointW2I(gcp_grd);
//
//    Eigen::Matrix3d scale_K_inverse = view.GetKMatrix().inverse();
//    //观测值、控制点、影像位置 转换到摄像机坐标系下
//    Point3d _CAM_obr = scale_K_inverse * gcd_ondist_obr.homogeneous();
//    Point3d _CAM_gcp = scale_K_inverse * gcp_grd_pix.homogeneous();
//
//    //计算垂直距离
//    Eigen::Vector4d vec1(_CAM_gcp[0], _CAM_gcp[1], _CAM_gcp[2], 0);
//    Eigen::Vector4d vec2(_CAM_obr[0], _CAM_obr[1], _CAM_obr[2], 0);
//
//    return vec1.cross3(vec2).norm();
//}


using namespace std;

 
using namespace AI3D::CORE;
//idx:0:current tab id ,

void ChangeTab(bool bAT, bool HasImages, bool HasGCPs, std::map<int, std::pair<bool, bool> >& tabstatus,std::vector<int>& idx)
{
 
    tabstatus[1].second = false;;
    tabstatus[2].second = false;;
 
    tabstatus[0].second = false;;
    tabstatus[3].second =  true;;
    int currentid = 3;
 
        //  tabstatus[3] = std::make_pair(false, false);
        if (HasGCPs)
        {
            bool bgcp_insert = tabstatus[2].second;
            tabstatus[2] = std::make_pair(false, true);
            currentid = 2;
        }
        if (HasImages)
        {
            tabstatus[1] = std::make_pair(false, true);
            currentid = 1;
        }
    


    if (bAT)
    {
        tabstatus[1] = std::make_pair(false, true);
        tabstatus[0] = std::make_pair(false, true);
        currentid = 0;
    }
    std::cout << std::endl;
    idx.push_back(currentid);
    std::cout << " current "<< currentid << " ";
    for (auto& it : tabstatus)
    {
        if (it.second.second || it.second.first)
        {
            idx.push_back(it.first);
            std::cout << it.first  << " ";
            it.second.first = true;
        }
       
    }
    std::cout << std::endl;
}

int main(int argc, char** argv)
{
    

    sqlite3* db;

    int result;

    char* errmsg = NULL;

    char** dbResult; //是 char ** 类型，两个*号

    int nRow, nColumn;

    int i, j;

    int index;



    result = sqlite3_open("D:/MyLearning/proj.db", &db);

    if (result != SQLITE_OK)

    {

        //数据库打开失败

        return -1;

    }

    //数据库操作代码

    //假设前面已经创建了 MyTable_1 表

    //开始查询，传入的 dbResult 已经是 char **，这里又加了一个 & 取地址符，传递进去的就成了 char ***
   // projected_crs table_name geodetic_crs
    result = sqlite3_get_table(db, "select * from crs_view where table_name='geodetic_crs'", &dbResult, &nRow, &nColumn, &errmsg);
     //result = sqlite3_get_table(db, "DROP VIEW crs_view", &dbResult, &nRow, &nColumn, &errmsg);
    if (SQLITE_OK == result)

    {

        //查询成功

        index = nColumn; //前面说过 dbResult 前面第一行数据是字段名称，从 nColumn 索引开始才是真正的数据

        printf("查到 % d条记录\n", nRow);
    }
  //  auto Atdata = std::make_shared<ATData>();
  //  google::InitGoogleLogging("");
  //  std::string log_dir = Application::Getinstance().GetAPPPath() + "/log.txt";
  //  FLAGS_alsologtostderr = true;
  //  google::SetLogDestination(google::GLOG_INFO, log_dir.c_str());
  //  google::SetStderrLogging(google::GLOG_INFO);
  //  FLAGS_logbufsecs = 0;
  // /* Eigen::Vector3d b{ 0.000077777,0.05555555,0.3333333 };
  //  char buf[1024];
  //  sprintf( buf,"2东: %.5f, 北: %.5f,%0.5f\n", b[1],b[2],b[3]);
  //  std::string ah(buf);
  //  std::cout << ah << std::endl;*/
  //  AI3D::CORE::Timer time;
  ////  time.Restart();
  //  AI3D::CORE::Application::Getinstance();

  //  //std::string  projdbfile = "D:/Code/ThirdParty/TOGIT/2/proj-6.3.2.tar/proj-6.3.2/build/data/";//proj.db

  //  //std::string strEnv = "PROJ_LIB=" + projdbfile;
  //  //int status = putenv(strEnv.c_str()); ="D:/TestData/xinghan-gps-2704/photo/" ;//
  //  
  //  std::string inpath ="D:/TestData/xinghan-gps-2704/photo/" ;// "D:/TestData/cc/test/testprevies/";//argv[1];//
  //  BlockObject block(inpath);
  //  block.SetPath(inpath);
  //  std::vector<std::string> image_extension;
  //  image_extension.push_back(".JPG");
  //  ////+ "/Back/"
  //  block.AddImages(inpath /*+ "Right/"*/, image_extension);
  // 
  ////  block.AddImages(inpath /*+ "/Right/"*/, image_extension);
  //  std::string xmlfile = "D:/TestData/cc/testpreview/block_AT.xml";// argv[2];
  /////*  block.AddImages("D:/TestData/xinghan-gps-2704/photo/");
  //// 
  ////  ProjectObject project;
  ////  project.NewProject("1.tri", inpath);

  ////  project.AddBlock(block);*/
  // 
  //  time.Start();

  //  std::shared_ptr<AI3D::CORE::ATData> atdata = std::make_shared<AI3D::CORE::ATData>();
  //// 
  //  block.LoadATXML(xmlfile,atdata);//"D:/TestData/cc/test/testprevies/4326.xml"
  //  time.PrintSeconds();
  //  block.SetATData(atdata);
  //  BlockObject::BlockExportOptions opt;
  //  opt.srs_.definition = "EPSG:32650";// "ENU:29.40552,105.02035";
  //  opt.srs_.name = opt.srs_.definition;
  //  opt.srs_.type = PROJECTION;
  //  opt.srs_.ID = 1;
  //  opt.export_tiepoint_ = true;
  //  block.ExportATXML("D:/TestData/cc/FQ.xml"/*argv[3]*//*, opt*/);
  //  time.PrintMinutes();

    return 0;
}

//
//int main3(int argc, char** argv)
//{
//    google::InitGoogleLogging("");
//    std::string log_dir = Application::Getinstance().GetAPPPath() + "/log.txt";
//    FLAGS_alsologtostderr = true;
//    google::SetLogDestination(google::GLOG_INFO, log_dir.c_str());
//    google::SetStderrLogging(google::GLOG_INFO);
//    FLAGS_logbufsecs = 0;
//
//    AI3D::CORE::Timer time;
//    time.Restart();
//  
//   
//    std::string  projdbfile = "D:/Code/ThirdParty/TOGIT/2/proj-6.3.2.tar/proj-6.3.2/build/data/";//proj.db
// 
//    std::string strEnv = "PROJ_LIB=" + projdbfile;
//    int status = putenv(strEnv.c_str());
//    BlockObject block;
//    std::string inpath = argv[1];//"D:/TestData/cc/test/testprevies/";//
//   
//
//    std::string xmlfile= argv[2];
//    
//    block.SetPath(inpath);
//    block.LoadATXML(xmlfile);//"D:/TestData/cc/test/testprevies/4326.xml"
//    BlockObject::BlockExportOptions opt;
//    block.ExportATXML(argv[3]);
//    time.PrintMinutes();
//
//    return 0;
//}