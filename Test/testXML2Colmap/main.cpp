#include <string>
#include <filesystem>

#include <Core/BlockObject.h>
#include "Core/File.h"

#include <Core/Tiling.h>
#include "Core/ReconstructionCommandSet.h"
#include "Core/ReconstructionObject.h"
#include "Core/ReconstructionOptions.h"
#include "Core/CoordinateSystem.h"
#include "colmap.h"
using namespace AI3D::CORE;
void RenameWithTab(std::string path,std::set< std::pair<std::string, std::string>>& namepairs)
{
    std::ifstream file = File::OpenIfstreamUtf8(path, std::ios::in);
    CHECK(file.is_open()) << path;

    std::string line;
    std::string item;

    while (std::getline(file, line))
    {
        AI3D::CORE::String::StringTrim(&line);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::stringstream line_stream1(line);

        // ID
        std::getline(line_stream1, item, '	');
        std::string outdir = (item);

        std::getline(line_stream1, item, '	');

        std::string indir = (item);
        namepairs.insert(std::make_pair(outdir, indir));

    }
    file.close();
}
void RenameWithSpace(std::string path, std::set< std::pair<std::string, std::string>>& namepairs)
{
    std::ifstream file = File::OpenIfstreamUtf8(path, std::ios::in);
    CHECK(file.is_open()) << path;

    std::string line;
    std::string item;

    while (std::getline(file, line))
    {
        AI3D::CORE::String::StringTrim(&line);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::stringstream line_stream1(line);

        // ID
        std::getline(line_stream1, item, ' ');
        std::string outdir = (item);

        std::getline(line_stream1, item, ' ');

        std::string indir = (item);
        namepairs.insert(std::make_pair(outdir, indir));

    }
    file.close();
}
void Rename(int argc, char* argv[])
{
    
   std::set< std::pair<std::string, std::string>> namepairs;
    std::string basepath = argv[1];
    std::string path = basepath+"/name.txt";
    
    path = File::EnsureUnifySlash(path);
    std::cout << " begin file " << path << std::endl;
    RenameWithTab(path,namepairs);
    if (namepairs.empty())
    {
        RenameWithSpace(path, namepairs);
    }
    if (namepairs.empty())
    {
        std::cout << "sorry " << std::endl;
        return;
    }

    std::cout << "---- total item " << namepairs.size() << std::endl;

    
    int num = 0;
    for (auto& iter : namepairs)
    {
        std::cout << "----  " << __LINE__ << std::endl;
        std::string indir = iter.second;
        std::string outdir = iter.first;
        std::string filepath = basepath + "/"+indir+"/";
        std::string fileoutpath = basepath  + outdir + "/";
        filepath = File::EnsureUnifySlash(filepath);
        if (!File::ExistsDir(filepath))
        {
            continue;
        }
        auto files = File::GetRecursiveFileList(filepath);
        std::cout << "---filesize-  " << __LINE__<< files.size() << std::endl;
        for (auto& file : files)
        {
            std::string filename = File::GetFileNameWithoutExtension(file);

            if (filename != indir)
            {
                continue;
            }
            std::string root, ext;
            File::SplitFileExtension(file,&root,&ext);
            std::string filepath = File::GetParentDir(file);

            std::string tagfile = filepath+ "/"+outdir + ext;
            if (!File::ExistsFile(file))
            {
                continue;
            }
            std::filesystem::rename(File::BoostPathFromUtf8(file), File::BoostPathFromUtf8(tagfile));
            std::string tagdir = String::StringReplace(tagfile, indir, outdir);
          
            std::cout << "=="<<file <<" rename  file" << tagfile << std::endl;
           
  
        }
        if (!File::ExistsPath(filepath))
        {
            continue;
        }
        if (File::ExistsPath(fileoutpath))
        {
            continue;
        }
        std::cout << "" << num << " ====rename path  " << filepath << " " << fileoutpath << std::endl;
        std::filesystem::rename(File::BoostPathFromUtf8(filepath), File::BoostPathFromUtf8(fileoutpath));
      
        num++;
       
    }
}
#include "Core/KML.h"
#include "Core/VectorFile.h"
#include "Core/Types.h"
int main77(int argc, char* argv[])
{
    char name1[256];
    char name[256];
    int x = 2000; int y = 1000; int z = 1000;
    sprintf(name, "Tile_%+04d_%+04d_%+04d", x, y, z);
    sprintf(name1, "Tile_%+04d_%+04d", x, y, z);
    AI3D::CORE::BlockObject block;

    auto atdata = std::make_shared<ATData>();
    block.LoadATXML("D:/jiaojie/test/bugchi/017TY/block_AT-TY2500.xml", atdata);
    clock_t t0, t1, t2;
    t0 = clock();
    atdata->GenPreviewImages("D:/jiaojie/test/bugchi/017TY/test/",atdata->GetImageIdsSet());
    t1 = clock();
    t2 = t1 - t0;

    srs_s srs1;
    srs1.type = GEOGRAPHIC;
    if (srs1.type != PROJECTION && srs1.type != LOCAL)
    {
        std::cout << 1 << std::endl;
    }
    double lon = 114.0;
    double lat = 36.0;
    const int lon_zone = 1 + floor((lon + 180) / 6);
    double lon_0 = (3 + 6 * (lon_zone - 1) - 180) * M_PI / 180.0;
    int code = int(32700 - round((45 + lat) / 90) * 100 + round((183 + lon) / 6));
     std::string  projdbfile = "D:/jiaojie/mok/0.00.023/build/MoldAI/2.03.008/newpackage/bin/data";//proj.db
	std::string strEnv = "PROJ_LIB=" + projdbfile;
	int status = putenv(strEnv.c_str());

    std::string file1 = "D:/jiaojie/test/yueshu/yueshu_1.kml";
    std::string file2 = "D:/jiaojie/test/yueshu/Reconstruction_2Tiling.kml";


    auto poly1 = ReadPolygonsFromVecFile(file1);
    auto poly2 = ReadPolygonsFromVecFile(file2);
    for (auto iter : poly1)
    {
        OGRGeometry* geo1 = ToPolygon(iter);
        for (auto iter2 : poly2)
        {
            OGRGeometry* geo2 = ToPolygon(iter2);
            auto a = geo1->Intersection(geo2);
            auto b = geo1->Intersects(geo2);
            auto c = geo1->Intersect(geo2);
            std::cout << a << " b " << b << " c " << c << std::endl;

        }
    }
    /*std::vector<Eigen::Vector2d> kmlboundary, kmlboundary_;
    for (int i = 0; i < 3; i++)
    {
        Eigen::Vector2d xy(i,i+1);
        kmlboundary.push_back(xy);
        Eigen::Vector2d xyz(i+1, i + 2);
        kmlboundary_.push_back(xyz);
    }
    OGRGeometry* ploygon = ToPolygon(kmlboundary);
    OGRGeometry* bd = ToPolygon(kmlboundary);
    OGRGeometry* b = bd->Intersection(ploygon);
    if (b != nullptr)
    {
        std::cout << 1 << std::endl;
    }*/
    return 0;
}
#include "Core/TaskDef.h"
int main666(int argc, char* argv[])
{
    //输入是文件夹，搜索该文件夹下所有的子目录，如果含有temp则记录
    std::string path = argv[1];
 //   auto a =GBK2UTF8(path);
  //  path = UTF82GBK(path);// AI3D::CORE::String::Utf8ToLocale(path);
    path = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(path));
    auto dirs = File::GetRecursiveDirList(path);// File::GetDirList(path);

    std::cout << " input path  is " << path << std::endl;
    std::cout << " dir count  is " << dirs.size() << std::endl;
    std::set<std::string> filesgotfull, filesgot;

    int tiledircount = dirs.size();
    int atfilescount = 0;
    for (auto& dir : dirs)
    {
        std::string dirtemp = dir;
      //  dirtemp = AI3D::CORE::String::Utf8ToLocale(dirtemp);
        dirtemp = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(dirtemp));
        AI3D::CORE::String::StringRemove(dirtemp,path);
        AI3D::CORE::String::StringToLower(&dirtemp);

        if (strstr(dirtemp.c_str(), "temp") != NULL)
        {
            dir = AI3D::CORE::File::EnsureUnifySlash(dir);
            filesgotfull.insert(dir);

            auto strs = AI3D::CORE::String::StringSplit(dirtemp, "/");
            if (strs.size() < 1)
            {
                continue;
            }
            std::string temp = strs[0];// strs[strs.size() - 3];
            filesgot.insert(temp);

        }

    }
    /*if (filesgotfull.size() != filesgot.size())
    {
        std::cout << "Error size,full " << filesgotfull.size() << " "<< filesgot.size() << std::endl;
        return 1;
    }*/
    std::cout << " total temp files count is  " << filesgot.size() << std::endl;
    std::string pathout =path /*argv[2]*/;

    /*std::string filefull = pathout + "/fulllist.txt";
    
    std::ofstream out = File::OpenOfstreamUtf8(filefull, std::ios::trunc);
    if (!out.is_open())
    {
        std::cout << "filefull " << filefull << " open failed ." << std::endl;
        return 1;
    }

    for (auto& iter : filefull)
    {
       
        out << iter << std::endl;
    }
    out.close();    */
    
    std::string fileshort = pathout + "/list.txt";
    //fileshort = AI3D::CORE::String::LocaleToUtf8(fileshort);
    std::ofstream out2 = File::OpenOfstreamUtf8(fileshort, std::ios::trunc);
    if (!out2.is_open())
    {
        
        std::cout << "file " << fileshort << " open failed ." << std::endl;
        return 1;
    }

    for (std::string iter : filesgot)
    {
      //  iter = AI3D::CORE::String::LocaleToUtf8(iter);
        out2 << iter << std::endl;
    }
    out2.close();
    
    std::cout << path << " Success " << std::endl;
    return 0;
}

int main111(int argc, char* argv[])
{
    Rename(argc, argv);
    return 0;
    //读取txt

    


    {

        auto atdata2 = std::shared_ptr<ATData>();

        if (atdata2 /*==nullptr*/)
        {
            std::cout << 1 << std::endl;
        }
        else
        {
            std::cout << 2 << std::endl;
        }
        AI3D::CORE::BlockObject block2;//jiaojie/test/Block_18
        std::string atbin2 = "C:/data/Projects/NewProject/Block_18/SCSFR.bin";// "C:/data/Projects/NewProject/Block_50/Block_50.bin";
        block2.LoadATBinary(atbin2, atdata2);


        auto atdata1 = std::make_shared<ATData>();
        AI3D::CORE::BlockObject block;//jiaojie/test/Block_18
        std::string atbin1 = "D:/jiaojie/test/1030/1030/Block_10/Block_10.bin";// "D:/jiaojie/test/1025/Block_38.bin";// "C:/data/Projects/NewProject/Block_50/Block_50.bin";
        block.LoadATBinaryWithoutTiepoints(atbin1, atdata1);
        block.LoadTiepointsBinary("D:/jiaojie/test/1030/1030/Block_10/CP.bin", atdata1);
        AI3D::CORE::BlockObject::BlockExportOptions opt;
        opt.export_tiepoint_ = true;
        opt.srs_ = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(atdata1->GetLocalSrs());
        opt.srs_.ID = 0;
       
        block.ExportATXML( "D:/jiaojie/test/1030/1030/Block_10/Block_10GCP.xml", opt);
    }
    colmap::Reconstruction rec1;
    std::string dir =argv[1];
    rec1.Read(dir);
    std::string imagpath = "C:/data/Projects/NewProject66666/777/Block_5/RU-1400/";
    {      
        auto data = rec1.GetATData();
        AI3D::CORE::BlockObject block1(dir);
        block1.SetId(0);

        block1.MakeBlockFromATData(data);


        std::string dir2 = argv[2];
        AI3D::CORE::BlockObject block;
       
        auto atdata = std::make_shared<ATData>();
        block.LoadATXML(dir2, atdata);
        
        std::set<image_t> imgidskeep = atdata->GetImagesIdSet();
        std::set<image_t> imgidsraw = data.GetImagesIdSet();
        std::set<image_t> imgidsremove;
        std::vector<std::string> imagesfile;
        for (auto& iter : imgidsraw)
        {
            if (!imgidskeep.count(iter))
                imgidsremove.insert(iter);
            else
            {
                imagesfile.push_back(imagpath + "/"+ data.GetImage(iter).GetName());
            }

        }
        block1.RemoveImages(imgidsremove);
        colmap::Reconstruction rec(*block1.GetATData().get());
        rec.Write(argv[3]);
        
        AI3D::CORE::File::CopyFiles(imagesfile, argv[3],false);
    }
    return 0;
}


//因Nerf测试需要
//将xml转成colmap的结果；
//输出成果包括两大类：1 enu后的坐标；2：归一化的坐标
#include "Core/CoordinateSystem.h"
#include <GeographicLib/Geoid.hpp>
using namespace GeographicLib;
int main99(int argc, char* argv[])
{
    std::string pstr = "D:/jiaojie/thirdparty/third_party/Windows/vc142/proj/6.3.2/data/";
    std::string strEnv = "PROJ_LIB=" + pstr;
    int status = putenv(strEnv.c_str());
    double lat = 39.808981, lon = 116.283774, height_above_geoid = -56.13;
    std::string src_srs = "EPSG:4326+5773";
    std::string dst_srs = "EPSG:4326";
    double x, y, z;
    AI3D::CORE::CoordinateTransformer::Transform(lon,lat, height_above_geoid,x,y,z, src_srs, dst_srs);
    
    Geoid egm96("egm96-5");
   
    double
        geoid_height = egm96(lat, lon),
        height_above_ellipsoid = (height_above_geoid +
            Geoid::GEOIDTOELLIPSOID * geoid_height);
    
    std::cout << z-(-66.339502) <<" "<< height_above_ellipsoid-(-66.339502) << std::endl;



    colmap::Reconstruction rec1;
    std::string dir =  "D:/SOFT/COLMAP-3.6-windows-cuda/test/comap/";
    if (0)
    {
        rec1.Read(dir);
       // rec1.Write("D:/SOFT/COLMAP-3.6-windows-cuda/test/rawbin/text/");
        auto data = rec1.GetATData();
        AI3D::CORE::BlockObject block1(dir);
        block1.SetId(0);

        block1.MakeBlockFromATData(data);
        AI3D::CORE::BlockObject::BlockExportOptions opt;
        opt.export_tiepoint_ = true;
        opt.srs_ = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(data.GetLocalSrs());
        opt.srs_.ID = 0;
        opt.export_not_registered_ = false;
        opt.export_controlpoint_ = false;
        block1.ExportATXML(dir + "/views.xml", opt);
    }
    AI3D::CORE::BlockObject block;
    std::string file = argv[1];//输入参数
    std::string out =argv[2];
    AI3D::CORE::Application::Getinstance().SetProjLibENV();
    AI3D::CORE::File::CreateDirIfNotExists(out, true);
    auto atdata = std::make_shared<ATData>();
    block.LoadATXML(file, atdata);
    //高德需要
    if(0)
    {
        //输出相机信息
        std::string camerafile = "D:/jiaojie/test/xingguangdao/gaode/camera.txt";
        std::ofstream file = File::OpenOfstreamUtf8(camerafile, std::ios::trunc);
        CHECK(file.is_open()) << camerafile;
        file << "ID FocalLength(mm),FocalLength(pixel) ,CCDsize(um),x0(pix),y0(pix),(distortion in opencv:k1,k2,p1,p2,k3)" << std::endl;;

        // Ensure that we don't loose any precision by storing in text.
        file.precision(17);

       
        auto cameras_ = atdata.get()->GetCameras();
        for (const auto& iter : cameras_)
        {
            auto camera = iter.second;
            std::ostringstream line;
            line.precision(17);

          line << camera.GetCameraId()<< " "<< camera.GetFocalLengthMM() << " " << camera.GetParams()[0] << " 3.9 "<< camera.GetParams()[2] <<
                " "<< camera.GetParams()[3]<<" "<< camera.GetParams()[4]<< " "<<camera.GetParams()[5]
                <<" "<< camera.GetParams()[7]<< " "<< camera.GetParams()[6]<<" "<< camera.GetParams()[8]         ;

           
            std::string line_string = line.str();
            line_string = line_string.substr(0, line_string.size() - 1);

            file << line_string << std::endl;
        }
        file.close();
        auto images_ = atdata.get()->GetImages();
        std::vector<Eigen::Vector3d> poses, poses1; std::vector<Eigen::Matrix3d> rotations, rot1;
        for (auto iter : images_)
        {
            auto pose = iter.second.GetPosition();
            auto r = iter.second.GetRotationMatrix();
            poses.push_back(pose);
            rotations.push_back(r);
           
        }

       /* auto srcsrs = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(atdata->GetLocalSrs());
        auto dstsrs = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(GEO84SRS);
        AI3D::CORE::CoordinateTransformer::TransformRotation(poses.size(), poses, rotations, srcsrs, dstsrs);*/
        int count = 0;
        std::string imgfile = "D:/jiaojie/test/xingguangdao/gaode/images.txt";
        std::ofstream fileimg = File::OpenOfstreamUtf8(imgfile, std::ios::trunc);
        fileimg << "ImageID ImageName CameraID X(lon) Y(lat) Z(alt) O P K" << std::endl;;

        // Ensure that we don't loose any precision by storing in text.
        fileimg.precision(17);
        for (const auto& iter : images_)
        {
            auto image = iter.second;
            std::ostringstream line;
            line.precision(17);
            double o, p, k;

            
            AlgorithmBase::ConvertRotmat2OPK(image.GetRotationMatrix().transpose(),o,p,k);
            
           
            line << image.GetImageId() << " "<<image.GetName()<<" " << image.GetCameraId() << "  " << poses[count].x() <<
                " " << poses[count].y() << " " << poses[count].z() << " " << o* DEG_PER_RAD
                << " " << p* DEG_PER_RAD << " " <<k* DEG_PER_RAD;


            std::string line_string = line.str();
            line_string = line_string.substr(0, line_string.size() - 1);

            fileimg << line_string << std::endl;
            count++;
        }
        fileimg.close();
    }
    if (0)
    {
        std::vector<Eigen::Vector3d> poses, poses1; std::vector<Eigen::Matrix3d> rotations, rot1;
        for (auto iter : atdata->GetImages())
        {
            auto pose = iter.second.GetPosition();
            auto r = iter.second.GetRotationMatrix();
            poses.push_back(pose);
            rotations.push_back(r);
            break;
        }

        auto srcsrs = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(atdata->GetLocalSrs());
        auto dstsrs = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(GEO84SRS);
        std::cout << srcsrs.definition << " " << dstsrs.definition << std::endl;
        AI3D::CORE::CoordinateTransformer::TransformRotation(poses.size(), poses, rotations, srcsrs, dstsrs);
        std::cout << rotations[0] << std::endl;
        poses1 = poses;
        rot1 = rotations;
        AI3D::CORE::CoordinateTransformer::TransformRotation(poses.size(), poses1, rot1, srcsrs, dstsrs, 1);
        std::cout << rot1[0] << std::endl;
        //转到enu
        srs_s srs = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(atdata->GetLocalSrs());
        srs_s destsrs;
        bool blocal = srs.type == LOCAL_ENU || srs.type == LOCAL;
        if (!blocal)
        {
            if (srs.type != LOCAL)
            {
                destsrs = atdata->GetDefaultEnuSRS();
                if (atdata->HasTiepoints())
                {
                    atdata->TransFormTiepoints(srs.definition, destsrs.definition);
                }
                atdata->TransFormImages(srs.definition, destsrs.definition);
            }
        }
    }
    if (0)
    {
        Eigen::Vector3d offset;
        atdata->ComputePositionOffsetByAvgCenter(offset);
        for (auto& iter : atdata->GetImagesMutual())
        {
            iter.second.GetPositionMutual() -= offset;
        }
        for (auto& iter : atdata->GetPoints3DMutual())
        {
            iter.second.GetXYZMutual() -= offset;
        }
    }
    block.SetATData(atdata);
  //做完畸变差改正给他们
   /* std::string undistortpath = out + "/RU/";
    AI3D::CORE::File::CreateDirIfNotExists(undistortpath);*/
    /// block.UndistortBlock(undistortpath);
    //先转到
    std::string campath = out /*+ "/500/"*/;
    AI3D::CORE::File::CreateDirIfNotExists(campath);
  //  colmap::Reconstruction rec(*block.GetATData().get());
    colmap::Reconstruction rec(*atdata);
    rec.Write(campath);
    //目前可能存在坐标系不一样，对比colmap生成的同一个数据在colmap中的显示和在cc中的显示


    return 0;
}
//测试数据 D:\jiaojie\test\baidu/sjgy-block_AT_enu.xml D:\jiaojie\test\baidu\guasssplatting\sjgy/colmap/
//因Nerf测试需要
//将xml转成colmap的结果；
//输出成果包括两大类：1 enu后的坐标；2：归一化的坐标
#include "Core/CoordinateSystem.h"
#include <GeographicLib/Geoid.hpp>
//输出GS用的colmap文件，因为GS要求影像宽度在1~1.6K内，当然其程序可以自动更改，
//在此流程中则由外部变换；且需要做完畸变差改正的
void ToColmapForGS(int argc, char* argv[])
{
    AI3D::CORE::BlockObject block;
    std::string file = argv[1];//输入参数
    std::string out = argv[2];
    AI3D::CORE::Application::Getinstance().SetProjLibENV();
    AI3D::CORE::File::CreateDirIfNotExists(out, true);
    auto atdata = std::make_shared<ATData>();
    block.LoadATXML(file, atdata, false);
    //atdata->Normalize();
  /*  for (auto& iter : atdata->GetCamerasMutual())
    {
        iter.second.SetModelId(1);
       
    }*/

    block.SetATData(atdata);
    if (1)
    {




        //做完畸变差改正给他们
        std::string undistortpath = out + "/images/";
        AI3D::CORE::File::CreateDirIfNotExists(undistortpath);
        UndistortCameraOptions_s undistopt;

        // undistopt.max_image_size = 1600;
        if (argc > 3)
        {
            int imgsize = std::atoi(argv[3]);
            // 
            if (imgsize > 0)
                undistopt.max_image_size = imgsize;
        }
        block.UndistortBlock(undistortpath, undistopt);
    }
    //先转到
    std::string campath = out + "/sparse/0/";
    AI3D::CORE::File::CreateDirIfNotExists(campath);
    colmap::Reconstruction rec(*block.GetATData().get());

     rec.Write(campath);
     std::cout << " write end " << std::endl;
     //目前可能存在坐标系不一样，对比colmap生成的同一个数据在colmap中的显示和在cc中的显示
    AI3D::CORE::BlockObject::BlockExportOptions opt;
    opt.export_tiepoint_ = true;
    block.ExportATXML(campath + "new.xml", opt);
}

void TestCR2ToJPG(int argc, char* argv[])
{
    std::string inpath = argv[1];
    std::string outpath = argv[2];
    Bitmap bitmap;
    auto files = File::GetFileList(inpath);
    for (auto file : files)
    {
        bool ret = bitmap.Read(file);
        if (ret)
        {
            std::string outfile = outpath + "/" + File::GetFileNameWithoutExtension(file) + ".jpg";
            File::EnsureUnifySlash(outfile);
            bitmap.Write(outfile);
        }
    }
    return;
}

int TestCopy(int argc, char* argv[])
{
    {
        AI3D::CORE::BlockObject block1, block;
        auto atdata = std::make_shared<ATData>();
        block.LoadATXML(argv[1], atdata, false);
        block.SetATData(atdata);
       
       
       std::set<image_t> imgidskeep = atdata->GetImagesIdSet();
      
       std::set<image_t> imgidsremove;
       std::vector<std::string> imagesfile;
       for (auto& iter : imgidskeep)
       {
          
           {
               imagesfile.push_back(atdata->GetImage(iter).GetPath()+"/"+ atdata->GetImage(iter).GetName());
           }

       }
     
       AI3D::CORE::File::CopyFiles(imagesfile, argv[2], false);
   }
    return 1000;
}
int TestCopyByImageName(int argc, char* argv[])
{
    {
        AI3D::CORE::BlockObject block1, block;
        auto atdata = std::make_shared<ATData>();
        block.LoadATXML(argv[1], atdata, false);
        block.SetATData(atdata);
        auto atdata1 = std::make_shared<ATData>();
        block1.LoadATXML(argv[2], atdata1, false);
        block1.SetATData(atdata1);

        std::set<image_t> imgidskeep = atdata->GetImagesIdSet();
        std::set<image_t> imgidskeep1 = atdata1->GetImagesIdSet();
        std::set<image_t> imgidsremove;
        std::vector<std::string> imagesfile;
        for (auto& iter : imgidskeep)
        {
            std::string name = atdata->GetImage(iter).GetName();
            AI3D::CORE::String::StringToLower(&name);
            
            for (auto& iter1 : imgidskeep1)
            {
                std::string name1 = atdata1->GetImage(iter1).GetName();
                AI3D::CORE::String::StringToLower(&name1);
                if(name == name1)
                 imagesfile.push_back(atdata1->GetImage(iter1).GetPath() + "/" + atdata1->GetImage(iter1).GetName());
            }

        }

        AI3D::CORE::File::CopyFiles(imagesfile, argv[3], false);
    }
    return 1000;
}

int TestCopyByImageID(int argc, char* argv[])
{
    {
        AI3D::CORE::BlockObject block1, block;
        auto atdata = std::make_shared<ATData>();
        block.LoadATXML(argv[1], atdata, false);
        block.SetATData(atdata);
        auto atdata1 = std::make_shared<ATData>();
        block1.LoadATXML(argv[2], atdata1, false);
        block1.SetATData(atdata1);
        
        std::set<image_t> imgidskeep = atdata->GetImagesIdSet();

        std::set<image_t> imgidsremove;
        std::vector<std::string> imagesfile;
        for (auto& iter : imgidskeep)
        {

            {
                imagesfile.push_back(atdata1->GetImage(iter).GetPath() + "/" + atdata1->GetImage(iter).GetName());
            }

        }

        AI3D::CORE::File::CopyFiles(imagesfile, argv[3], false);
    }
    return 1000;
}


#include "Core/ATCommandSet.h"
int ExtractSourceData(int argc, char* argv[])
{
    AI3D::CORE::ATData Atdata;
    Eigen::Vector3d possigma;
    std::string skfpath = argv[2];
    bool ret = AI3D::CORE::ATCommandSet::LoadSourceDataJson(Atdata,argv[1], possigma, skfpath);
    if (ret)
    {
        ATOptions atoptions;
        AI3D::CORE::ATCommandSet::SaveSourceDataJson1(Atdata, argv[3], atoptions, possigma);
    }
    return 1000;
}


int LoadColmapProj(int argc, char* argv[])
{
    colmap::Reconstruction rec;
    rec.Read(argv[1]);
    AI3D::CORE::ATData Atdata;
    Atdata = rec.GetATData();

   
    //rec.Write(argv[2]); 

    AI3D::CORE::BlockObject  block;
  
    block.MakeBlockFromATData(Atdata);
    AI3D::CORE::BlockObject::BlockExportOptions opt;
    opt.export_tiepoint_ = true;
    std::string outfile = argv[2];
    outfile+="/new.xml";
    block.ExportATXML(outfile, opt);
    return 1;
}


void testLoadatbin()
{
    std::shared_ptr<ATData> Atdata = std::make_shared<ATData>();
    //AI3D::CORE::ATCommandSet::LoadATBinary("C:/data/Projects/NewProject/Block_3/SCSFR.bin", Atdata);
}

//测试dj的ypr
//测试一下dj的ypr和cc的ypr的关系
//-0.0558767767943726 
//-0.997840521783679 
//0.0345264956453227 
//-0.731644125687582 
//0.0173908752892901 
//-0.681464915313716 
//0.679392860693964 
//-0.0633391706856585 
//-0.731035902192858 


//<Yaw>95.3262370461158 < / Yaw >
//<Pitch>-46.9733078448463 < / Pitch >
//< Roll>2.90041695221237 < / Roll >
//经过测试转给高德的可以直接用cc的输出结果
void TestDJypr()
{

    

    //dj的ypr转R
    {
        double yaw = -112.30;
        double pitch = 0.;
        double roll = 0.0;


        //转到了自己的坐标系
        Eigen::Vector3d ypr = Eigen::Vector3d(FD2R(yaw), FD2R(pitch), FD2R(roll));
        yaw = ypr.x();
        pitch = ypr.y();
        roll = ypr.z();
        {
            Eigen::Matrix3d R1;
            //此步骤用的是CC上的公式
            R1(0, 0) = cos(yaw) * cos(roll) - sin(yaw) * sin(pitch) * sin(roll);
            R1(0, 1) = -sin(yaw) * cos(roll) - cos(yaw) * sin(pitch) * sin(roll);
            R1(0, 2) = cos(pitch) * sin(roll);
            R1(2, 0) = sin(yaw) * cos(pitch);
            R1(2, 1) = cos(yaw) * cos(pitch);
            R1(2, 2) = sin(pitch);
            R1(1, 0) = cos(yaw) * sin(roll) + sin(yaw) * sin(pitch) * cos(roll);
            R1(1, 1) = -sin(yaw) * sin(roll) + cos(yaw) * sin(pitch) * cos(roll);
            R1(1, 2) = -cos(pitch) * cos(roll);
            std::cout << "0 " << R1 << std::endl;

                pitch = asin(R1(2, 2));						//仰俯角Pitch在(-90， +90)度之间
               yaw = atan(R1(2, 0) / R1(2, 1));			//航偏角Yaw在(-180， +180)度之间

               roll = atan(-R1(0, 2) / R1(2, 2));		//滚动角Rolling在(-180， +180)度之间

               double sinpitch = R1(2, 2);
               double cospitch = cos(pitch);

               double sinrolling = R1(0, 2) / cospitch;
               double cosrolling = -R1(1, 2) / cospitch;

               double sinyaw = R1(2, 0) / cospitch;
               double cosyaw = R1(2, 1) / cospitch;
               //分母为0时的处理

               if (cosrolling < 0)
               {
                   if (sinrolling < 0)
                   {
                       roll -= M_PI;
                   }
                   //else if (sinrolling > 0)//第二象限
                   //{
                   //    roll += M_PI;
                   //}
                   else
                   {
                       roll = M_PI;
                   }
               }
               if (cosrolling > 0)
               {
                   if (sinrolling < 0)//第二象限
                   {
                       roll = 2*M_PI-roll;
                   }
               }
                if (sinrolling == 0)
                    roll = 0.0;

               if (cosyaw < 0)
               {
                   if (sinyaw < 0)
                   {
                       yaw -= M_PI;
                   }
                 /*  else if (sinyaw > 0)
                   {
                       yaw += M_PI;
                   }*/
                   else
                   {
                       yaw = M_PI;
                   }
               }
               yaw = R2FD(yaw);
               pitch = R2FD(pitch);
               roll = R2FD(roll);
               std::cout << "0 " << yaw << " "<< pitch << " "<<roll << std::endl;
            
        }
        {
            Eigen::Matrix3d R1;
            R1(0, 0) = cos(yaw) * cos(roll) + sin(yaw) * sin(pitch) * sin(roll);
            R1(0, 1) = -sin(yaw) * cos(roll) + cos(yaw) * sin(pitch) * sin(roll);
            R1(0, 2) = -cos(pitch) * sin(roll);
            R1(1, 0) = sin(yaw) * cos(pitch);
            R1(1, 1) = cos(yaw) * cos(pitch);
            R1(1, 2) = sin(pitch);
            R1(2, 0) = cos(yaw) * sin(roll) - sin(yaw) * sin(pitch) * cos(roll);
            R1(2, 1) = -sin(yaw) * sin(roll) - cos(yaw) * sin(pitch) * cos(roll);
            R1(2, 2) = cos(pitch) * cos(roll);


            std::cout << R1 << std::endl;
            std::cout << R1.transpose() << std::endl;

            {
                //转到了国外坐标系
                double omega = M_PI; double phi = 0.; double kappa = 0.;
                Eigen::Matrix3d R2;
                R2(0, 0) = cos(phi) * cos(kappa);
                R2(1, 0) = cos(omega) * sin(kappa) + sin(omega) * sin(phi) * cos(kappa);
                R2(2, 0) = sin(omega) * sin(kappa) - cos(omega) * sin(phi) * cos(kappa);
                R2(0, 1) = -cos(phi) * sin(kappa);
                R2(1, 1) = cos(omega) * cos(kappa) - sin(omega) * sin(phi) * sin(kappa);
                R2(2, 1) = sin(omega) * cos(kappa) + cos(omega) * sin(phi) * sin(kappa);
                R2(2, 0) = sin(phi);
                R2(2, 1) = -sin(omega) * cos(phi);
                R2(2, 2) = cos(omega) * cos(phi);
                std::cout <<" 1 " << R1 * R2 << std::endl;
            }
        }
        auto R = AI3D::CORE::AlgorithmBase::YPRToRotationInner(ypr);
        std::cout << R << std::endl;


        
        std::cout << yaw << " "<< pitch << " "<<roll << std::endl;
        auto yprnew = AI3D::CORE::AlgorithmBase::RotationInnerToYPR(R);
        std::cout << yprnew << std::endl;
    }
    {

        double yaw = 95.3262370461158;
        double pitch = -46.9733078448463;
        double roll = 2.90041695221237;

        Eigen::Vector3d ypr = Eigen::Vector3d(FD2R(yaw), FD2R(pitch), FD2R(roll));
        auto R = AI3D::CORE::AlgorithmBase::YPRToRotationInner(ypr);
        std::cout << R << std::endl;
    }
}
//用我们的空三结果转成高德的

void ToGaodeResult(int argc, char* argv[])
{
    {
        AI3D::CORE::BlockObject block;
        std::string file = argv[1];//输入参数
        std::string out = argv[2];
        int outrotformat = 2;
        if (argc > 3)
        {
            outrotformat = std::atoi(argv[3]);
        }
        AI3D::CORE::Application::Getinstance().SetProjLibENV();
        AI3D::CORE::File::CreateDirIfNotExists(out, true);
        auto atdata = std::make_shared<ATData>();
        block.LoadATXML(file, atdata,false);
        block.SetATData(atdata);


      /*  for (auto iter : atdata->GetImages())
        {
            if (iter.second.GetImageId() == 882)
            {
               
                auto ypr = AlgorithmBase::RotationInnerToYPR(iter.second.GetRotationMatrix());
                std::cout << std::setprecision(16)<<iter.second.GetRotationMatrix() << " "<< ypr.x() << " "<< ypr.y() << ypr.z() << std::endl;
                break;
            }
        }*/

        /*AI3D::CORE::BlockObject::BlockExportOptions opt;
        opt.rotformat_ = rot_format_e::ROTFORMAT_YPR;
        opt.srs_.definition = "EPSG:WGS84";
        block.ExportATXML(out+"/OUT.xml", opt);*/
        //高德需要

        {
            //输出相机信息
            std::string camerafile = out+ "/camera.txt";
            std::ofstream file = File::OpenOfstreamUtf8(camerafile, std::ios::trunc);
            CHECK(file.is_open()) << camerafile;
            file << "ID FocalLength(mm),FocalLength(pixel) ,CCDsize(um),x0(pix),y0(pix),(distortion in opencv:k1,k2,p1,p2,k3)" << std::endl;;

            // Ensure that we don't loose any precision by storing in text.
            file.precision(17);


            auto cameras_ = atdata.get()->GetCameras();
            for (const auto& iter : cameras_)
            {
                auto camera = iter.second;
                std::ostringstream line;
                line.precision(17);

                line << camera.GetCameraId() << " " << camera.GetFocalLengthMM() << " " << camera.GetParams()[0] << " 3.9 " << camera.GetParams()[2] <<
                    " " << camera.GetParams()[3] << " " << camera.GetParams()[4] << " " << camera.GetParams()[5]
                    << " " << camera.GetParams()[7] << " " << camera.GetParams()[6] << " " << camera.GetParams()[8];


                std::string line_string = line.str();
                line_string = line_string.substr(0, line_string.size() - 1);

                file << line_string << std::endl;
            }
            file.close();
            auto images_ = atdata.get()->GetImages();
            std::vector<Eigen::Vector3d> poses, poses1; std::vector<Eigen::Matrix3d> rotations, rot1;
            for (auto iter : images_)
            {
                auto pose = iter.second.GetPosition();
                auto r = iter.second.GetRotationMatrix();
                poses.push_back(pose);
                rotations.push_back(r);

            }

            /* auto srcsrs = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(atdata->GetLocalSrs());
             auto dstsrs = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(GEO84SRS);
             AI3D::CORE::CoordinateTransformer::TransformRotation(poses.size(), poses, rotations, srcsrs, dstsrs);*/
            int count = 0;
            std::string imgfile = out+"/images.txt";
            std::ofstream fileimg = File::OpenOfstreamUtf8(imgfile, std::ios::trunc);
           

            // Ensure that we don't loose any precision by storing in text.
            fileimg.precision(17);
            if (outrotformat == 1) 
            {
                fileimg << "ImageID ImageName CameraID X(lon) Y(lat) Z(alt) O P K" << std::endl;;
                for (const auto& iter : images_)
                {
                    auto image = iter.second;
                    std::ostringstream line;
                    line.precision(17);
                    double o, p, k;


                    AlgorithmBase::ConvertRotmat2OPK(image.GetRotationMatrix().transpose(), o, p, k);


                    line << image.GetImageId() << " " << image.GetName() << " " << image.GetCameraId() << "  " << poses[count].x() <<
                        " " << poses[count].y() << " " << poses[count].z() << " " << o * DEG_PER_RAD
                        << " " << p * DEG_PER_RAD << " " << k * DEG_PER_RAD;


                    std::string line_string = line.str();
                    line_string = line_string.substr(0, line_string.size() - 1);

                    fileimg << line_string << std::endl;
                    count++;
                }
            }
            else if (outrotformat == 2)
            {
                fileimg << "ImageID ImageName CameraID X(lon) Y(lat) Z(alt) Y P R" << std::endl;;
                for (const auto& iter : images_)
                {
                    auto image = iter.second;
                    std::ostringstream line;
                    line.precision(17);
                    double o, p, k;


                   auto ypr =  AlgorithmBase::RotationInnerToYPR(image.GetRotationMatrix());
                   //if (iter.second.GetImageId() == 882)
                   //{

                   //  //  auto ypr = AlgorithmBase::RotationInnerToYPR(iter.second.GetRotationMatrix());
                   //    std::cout << std::setprecision(16) << iter.second.GetRotationMatrix() << " " << ypr.x() << " " << ypr.y() << ypr.z() << std::endl;
                   //   // break;
                   //}

                    line << image.GetImageId() << " " << image.GetName() << " " << image.GetCameraId() << "  " << poses[count].x() <<
                        " " << poses[count].y() << " " << poses[count].z() << " " << ypr.x()
                        << " " <<ypr.y() << " " <<ypr.z();


                    std::string line_string = line.str();
                    line_string = line_string.substr(0, line_string.size() - 1);

                    fileimg << line_string << std::endl;
                    count++;
                }
            }
            fileimg.close();
        }
    }
}
//测试统一个点在下视和倾斜影像上计算出来的物方坐标是否一致
int testBaiduGS(int argc, char* argv[])
{
    //tile 004_019 
    //选中了一个点在如下那些影像上：053230400120656-053230400120659，053230400100471-053230400100475，053230400100652，053230400110695-053230400110697
    AI3D::CORE::BlockObject block;
    std::string file = argv[1];//输入参数
   // std::string out = argv[2];
    colmap::Reconstruction rec;
    rec.Read(argv[1]);
    AI3D::CORE::ATData Atdata;
    Atdata = rec.GetATData();
   
    
    block.MakeBlockFromATData(Atdata);
    if(0)
    {
        auto atdata = std::make_shared<ATData>();
        block.LoadATXML(file, atdata, false);
        block.SetATData(atdata);
    }
    std::set<point3D_t> pointIDs;
    for (auto iter : Atdata.GetPoints3D())
    {
        std::vector<std::string> images;
        for (auto ele : iter.second.GetTrackMutual().GetElements())
        {
            std::string name = Atdata.GetImage(ele.image_id).GetName();
            //  std::cout << name << std::endl;
            images.push_back(name);
        }
        int count = 0;
        for (auto imgname : images)
        {
            if (String::StringContains(imgname, "053230400120656"))
            {
                count++;
            }
            if (String::StringContains(imgname,"053230400120657"))
            {
                count++;
            }
            if (String::StringContains(imgname, "053230400120658"))
            {
                count++;
            }
            if (String::StringContains(imgname,"053230400120659"))
            {
                count++;
            }
            if (String::StringContains(imgname,"053230400100471"))
            {
                count++;
            }


            if (String::StringContains(imgname ,"053230400100472"))
            {
                count++;
            }
            if (String::StringContains(imgname, "053230400100473"))
            {
                count++;
            }
            if (String::StringContains(imgname, "053230400100474"))
            {
                count++;
            }
            if (String::StringContains(imgname, "053230400100475"))
            {
                count++;
            }
            if (String::StringContains(imgname, "053230400100652"))
            {
                count++;
            }

            if (String::StringContains(imgname, "053230400110695"))
            {
                count++;
            }
            if (String::StringContains(imgname , "053230400110696"))
            {
                count++;
            }
            if (String::StringContains(imgname , "053230400110697"))
            {
                count++;
            }
        }
        if (count > 12)
        {
            std::cout << std::setprecision(16) << iter.second.GetXYZ().x() << " " << iter.second.GetXYZ().y() << " " << iter.second.GetXYZ().z() << std::endl;
            pointIDs.insert(iter.first);
        }
    }
        if (!pointIDs.empty())
        {
            auto id = pointIDs.begin();
            auto point = Atdata.GetPoints3D().at(*id);
            std::map<camera_t, std::set<image_t>> cam_imgs;
            for (auto ele : point.GetTrackMutual().GetElements())
            {
                Image image = Atdata.GetImage(ele.image_id);
                auto camid = image.GetCameraId();
                cam_imgs[camid].insert(ele.image_id);
            }
            auto cameras = Atdata.GetCameras();
            auto images = Atdata.GetImages();
            std::map<camera_t, Eigen::Vector3f> cam_xyzs;
         
            { 
                for (auto imgs : cam_imgs)
                {
                    std::vector<Eigen::Vector2f> points;
                    std::vector<Eigen::Matrix<float, 3, 4>> poses;
                    Eigen::Vector2d xy;
                    xy.setConstant(NAN);
                    for (auto& imgid : imgs.second)
                    {

                        for (auto ele : point.GetTrackMutual().GetElements())
                        {
                            auto imgid2 = ele.image_id;
                            if (imgid == imgid2)
                            {
                                xy = ele.xy;
                            }
                        }

                        if (xy.hasNaN())
                        {
                            continue;
                        }
                        Camera& camera = cameras[imgs.first];

                        Image image = images[imgid];

                        Eigen::Vector2d  undis_xy = camera.UndistortPixel(xy);

                        points.push_back(Eigen::Vector2f{ undis_xy.x(), undis_xy.y() });

                        {

                            poses.push_back((camera.GetCalibrationMatrix()* image.GetProjectionMatrix()).cast<float>());

                        }
                    }
                    Eigen::Vector3f xyzf = AlgorithmBase::TriangulatePoint(poses, points);
                    cam_xyzs[imgs.first] = xyzf;
                }
                  
                for (auto cam : cam_xyzs)
                {
                    double dx = cam.second.x() - point.GetXYZ().x();
                    double dy = cam.second.y() - point.GetXYZ().y();
                    double dz = cam.second.z() - point.GetXYZ().z();
                    double dxyz = std::sqrt(dx * dx + dy * dy + dz * dz);
                    std::cout << dx<< " "<< dy << " "<<dz <<" "<< dxyz << std::endl;
               }
            }
        }
    
    

    return 1;
}
void TestQvec()
{
    std::vector<double> qvec(4);
    //def qvec2rotmat(qvec) :
    qvec[0] = 0.29753901756292139;
    qvec[1] = 0.65804066906645065;
    qvec[2] = -0.65069597236433152;
    qvec[3] = 0.23462259573857483;

    Eigen::Matrix3d R;
    R << 1 - 2 * qvec[2] * qvec[2] - 2 * qvec[3] * qvec[3],
        2 * qvec[1] * qvec[2] - 2 * qvec[0] * qvec[3],
        2 * qvec[3] * qvec[1] + 2 * qvec[0] * qvec[2],
        2 * qvec[1] * qvec[2] + 2 * qvec[0] * qvec[3],
        1 - 2 * qvec[1] * qvec[1] - 2 * qvec[3] * qvec[3],
        2 * qvec[2] * qvec[3] - 2 * qvec[0] * qvec[1],
        2 * qvec[3] * qvec[1] - 2 * qvec[0] * qvec[2],
        2 * qvec[2] * qvec[3] + 2 * qvec[0] * qvec[1],
        1 - 2 * qvec[1] * qvec[1] - 2 * qvec[2] * qvec[2];
    std::cout << R << std::endl;
      /*  def rotmat2qvec(R) :
                Rxx, Ryx, Rzx, Rxy, Ryy, Rzy, Rxz, Ryz, Rzz = R.flat
                K = np.array([
                    [Rxx - Ryy - Rzz, 0, 0, 0],
                        [Ryx + Rxy, Ryy - Rxx - Rzz, 0, 0],
                        [Rzx + Rxz, Rzy + Ryz, Rzz - Rxx - Ryy, 0],
                        [Ryz - Rzy, Rzx - Rxz, Rxy - Ryx, Rxx + Ryy + Rzz]] ) / 3.0
                eigvals, eigvecs = np.linalg.eigh(K)
                        qvec = eigvecs [[3, 0, 1, 2], np.argmax(eigvals)]
                        if qvec[0] < 0:
                    qvec *= -1
                        return qvec*/
}

int main(int argc, char* argv[])
{
   
    
    AI3D::CORE::Application::Getinstance().SetUpGDALSettings();
    AI3D::CORE::Application::Getinstance().SetProjLibENV();
   /* ToGaodeResult(argc, argv);
    return 0;*/
    //
   // TestCopyByImageName(argc, argv);
 if(0)
 {
     TestQvec();
     testBaiduGS(argc, argv);
     return 1;
     TestCopyByImageID(argc, argv);
     return 1;
    double yaw = 73.3;
    double pitch = -33.6;
    double roll = 0.;
    double x = 116.308112138889;
    double y = 39.987085;
    double z = 116.863;

    yaw = FD2R(yaw);
    pitch = FD2R(pitch);
    roll = FD2R(roll);
    Eigen::Vector3d pose{ x,y,z };
    std::vector< Eigen::Vector3d> poses;
    poses.push_back(pose);
    Eigen::Vector3d ypr{ yaw,pitch,roll };
    Eigen::Matrix3d rotation = AlgorithmBase::YPRToRotationInner(ypr);
    std::vector < Eigen::Matrix3d> rotations;
    rotations.push_back(rotation);
    srs_s src_crs;
    src_crs.definition = "EPSG:4326+5773";
    src_crs = CoordinateDescriptor::GetSRSFromDefinition(src_crs.definition);

    srs_s dst_crs;
    dst_crs.definition = "EPSG:4326";
    dst_crs = CoordinateDescriptor::GetSRSFromDefinition(dst_crs.definition);
    // src_crs.type = dst_crs.type;
    Eigen::Matrix3x4d pold;
    pold.block(0, 0, 3, 3) = rotations[0];
    pold.block(0, 3, 3, 1) = -rotations[0] * poses[0];
    Eigen::Matrix3x4d pnew = CoordinateTransformer::TransformProjectMatrix(pold, src_crs.definition, dst_crs.definition);
    rotations[0] = pnew.block(0, 0, 3, 3);
    //AI3D::CORE::CoordinateTransformer::TransformRotation(1,poses, rotations,src_crs,dst_crs,1);

    auto yprnew = AlgorithmBase::RotationInnerToYPR(rotations[0]);
    std::cout << std::setprecision(16) << poses[0] << " " << yprnew.x() << " " << yprnew.y() << " " << yprnew.z() << std::endl;
}
 //  ToGaodeResult(argc, argv);
 //  return 1;
 //   //TestDJypr();
 //   //
 ////   testLoadatbin();
//   LoadColmapProj(argc, argv);

    //

 //   testLoadatbin();


    //

   // ExtractSourceData(argc, argv);
  //  TestCopy(argc, argv);
   ToColmapForGS(argc, argv);

   // TestCopy(argc, argv);
    //ToColmapForGS(argc, argv);

   // TestCR2ToJPG(argc, argv);
    return 0;
}