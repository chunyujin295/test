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
     std::string  projdbfile = "D:/jiaojie/mok/0.00.023/build/Mohacker/2.03.008/newpackage/bin/data";//proj.db
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
    //?????????????????????????????????????????????temp????
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
    //???txt

    


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
        std::string atbin2 = "C:/data/Projects/NewProject/Block_18/AT.bin";// "C:/data/Projects/NewProject/Block_50/Block_50.bin";
        block2.LoadATBinary(atbin2, atdata2);


        auto atdata1 = std::make_shared<ATData>();
        AI3D::CORE::BlockObject block;//jiaojie/test/Block_18
        std::string atbin1 = "D:/jiaojie/test/1030/1030/Block_10/Block_10.bin";// "D:/jiaojie/test/1025/Block_38.bin";// "C:/data/Projects/NewProject/Block_50/Block_50.bin";
        block.LoadATBinaryWithoutTiepoints(atbin1, atdata1);
        block.LoadTiepointsBinary("D:/jiaojie/test/1030/1030/Block_10/Tiepoints.bin", atdata1);
        AI3D::CORE::BlockObject::BlockExportOptions opt;
        opt.export_tiepoint_ = true;
        opt.srs_ = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(atdata1->GetLocalSrs());
        opt.srs_.ID = 0;
       
        block.ExportATXML( "D:/jiaojie/test/1030/1030/Block_10/Block_10GCP.xml", opt);
    }
    colmap::Reconstruction rec1;
    std::string dir =argv[1];
    rec1.Read(dir);
    std::string imagpath = "C:/data/Projects/NewProject66666/777/Block_5/undistort-1400/";
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


//??Nerf???????
//??xml???colmap??????
//????????????????1 enu???????2?????????????
int main(int argc, char* argv[])
{





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
    std::string file = argv[1];//???????
    std::string out =argv[2];
    AI3D::CORE::Application::Getinstance().SetProjLibENV();
    AI3D::CORE::File::CreateDirIfNotExists(out, true);
    auto atdata = std::make_shared<ATData>();
    block.LoadATXML(file, atdata);
    //??????
    if(0)
    {
        //?????????
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
        //???enu
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
  //?????????????????
   /* std::string undistortpath = out + "/undistort/";
    AI3D::CORE::File::CreateDirIfNotExists(undistortpath);*/
    /// block.UndistortBlock(undistortpath);
    //?????
    std::string campath = out /*+ "/500/"*/;
    AI3D::CORE::File::CreateDirIfNotExists(campath);
  //  colmap::Reconstruction rec(*block.GetATData().get());
    colmap::Reconstruction rec(*atdata);
    rec.Write(campath);
    //????????????????????????colmap???????????????colmap???????????cc???????


    return 0;
}