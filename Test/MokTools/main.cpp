

#include <string>
#include <map>
#include "Core/Logging.h"
#include "Core/TaskDef.h"
#include <Core/ATData.h>
#include <Core/Application.h>
#include <Core/BlockObject.h>
#include <Core/Tiling.h>
#include "Core/ReconstructionCommandSet.h"
#include "Core/ReconstructionObject.h"
#include "Core/ReconstructionOptions.h"
#include "Core/CoordinateSystem.h"
#include "Core/File.h"
#include "Core/ProjectObject.h"
#include "Core/ATCommandSet.h"
#include "colmap.h"
using namespace AI3D::CORE;

//
//int RunCopyGCPImgs(int argc, char** argv)
//{
//    std::string xmlfile = argv[1];
//    
//    std::string outdir= argv[2];
//    AI3D::CORE::BlockObject block;
//    // auto atdata = std::make_shared<ATData>();
//    AI3D::CORE::Application::Getinstance().SetProjLibENV();
//
//    int ret = ATCommandSet::LoadBlock(xmlfile, block);
//    if (ret != AI3D_SUCCESS)
//    {
//        std::cout << "load falied " << xmlfile << std::endl;
//        return ret;
//    }
//    auto atdata = block.GetATDataMutual();
//    auto gcpcnt = atdata->GetNumControlPoints();
//    std::cout << "gcpcnt " << gcpcnt << std::endl;
//    if (gcpcnt <= 0)
//        return -1;
//
//
//
//   
//    std::map< image_t, std::string> name_ids;
// 
//    for (auto& iter : atdata->GetControlPointsMutual())
//    {
//        std::vector<AI3D::CORE::TrackElement>& elevector = iter.second.GetObjectPointMutual().GetTrackMutual().GetElementsMutual();
//        for (auto iterele : elevector)
//        {
//            if (!name_ids.count(iterele.image_id))
//            {
//                name_ids[iterele.image_id]=atdata->GetImages()[image_id].GetName();
//            }
//        }
//       
//
//
//    }
//  
//
//    AI3D::CORE::BlockObject::ExportGCPMeasurementsXML1(outfile, newimage_map, cps_map);
//    return 1000;
//}
//????gcp???????,??GCP??????????????ID ??sourcedata.json??id??
int RunMakeGCPIDSame(int argc, char** argv)
{
    std::string xmlfile = argv[1];
    std::string srcfile = argv[2];
    std::string outfile = argv[3];
    AI3D::CORE::BlockObject block;
    // auto atdata = std::make_shared<ATData>();
    AI3D::CORE::Application::Getinstance().SetProjLibENV();

    int ret = ATCommandSet::LoadBlock(srcfile, block);
    if (ret != AI3D_SUCCESS)
    {
        std::cout << "load falied " << srcfile << std::endl;
        return ret;
    }

    EIGEN_STL_UMAP(srsid_t, srs_s) srs_map;
    EIGEN_STL_UMAP(point3D_t, AI3D::CORE::ControlPoint) cps_map;
    EIGEN_STL_UMAP(image_t, std::string) image_map;
    EIGEN_STL_UMAP(image_t, std::string) newimage_map;
    AI3D::CORE::BlockObject::LoadGCPMeasurementsXML1(xmlfile, srs_map, cps_map, image_map);


    auto atdata = block.GetATDataMutual();
    std::map<std::string, std::pair<image_t, image_t>> name_ids;
    for (auto& iter : image_map)
    {
        name_ids[iter.second].first = iter.first;
        name_ids[iter.second].second = -1;
    }
    for (auto& iter : atdata->GetImages())
    {
        auto name1 = File::GetFileNameWithoutExtension(iter.second.GetName());
        if (name_ids.count(name1))
        {
            name_ids.at(name1).second = iter.first;
            newimage_map[iter.first] = iter.second.GetPath() + "/" + iter.second.GetName();
        }
        
        
    }
    for (auto& iter : cps_map)
    {
        std::vector<AI3D::CORE::TrackElement>& vector = iter.second.GetObjectPointMutual().GetTrackMutual().GetElementsMutual();
       
        for (auto& it : vector)
        {
            std::string imgname = image_map.at(it.image_id);
            if (name_ids.count(imgname))
            {
                if (name_ids.at(imgname).first != -1)
                {
                    it.image_id = name_ids.at(imgname).second;
                }
            }
        }
        

    }

    AI3D::CORE::BlockObject::ExportGCPMeasurementsXML1(outfile, newimage_map, cps_map);
    return 1000;
}


//???????xml?????????????????????9?????????;in:merge???xml?????out???????xml?????

int RunEraseDuplicateImageElement(int argc, char** argv)
{
    std::string input_path = argv[1];
    std::string output_path = argv[2];

    AI3D::CORE::BlockObject block, blockout;
    AI3D::CORE::Application::Getinstance().SetProjLibENV();
    auto atdata = std::make_shared<AI3D::CORE::ATData>();
    block.LoadATXML(input_path, atdata, false);
    AI3D::CORE::ATData atdatatemp;
    atdata->EraseDuplicateImages(atdatatemp);
    blockout.MakeBlockFromATData(atdatatemp);
    AI3D::CORE::BlockObject::BlockExportOptions opt;
    opt.export_tiepoint_ = true;
    // std::cout << [0].definition<< " "<< atdata->GetOriginSrs();
    opt.srs_ = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(atdata->GetLocalSrs());
    opt.srs_.ID = 0;
    opt.export_not_registered_ = false;
    opt.export_controlpoint_ = false;

    blockout.ExportATXML(output_path, opt);
    return 1000;
}

//merge?????
int RunBacthEraseDuplicateImageElement(int argc, char** argv)
{
    std::string input_path = argv[1];

    std::string outdir = argv[2];

    auto files = AI3D::CORE::File::GetRecursiveFileList(input_path, "xml");
    std::vector<std::string> xmlfiles;
    ProjectObject project;
    std::set<block_t> ids;
    for (auto& iter : files)
    {

        std::string file = iter;
        BlockObject block;
        auto atdata = std::make_shared<ATData>();
        block.LoadATXML(file, atdata, false);
        block.SetATData(atdata);
        project.AddBlock(&block);
        ids.insert(block.GetId());

    }
    project.MergeBlocks(ids);
    auto MergeBlockId = *project.GetBlockIds().rbegin();
    AI3D::CORE::BlockObject* MergeBlock = project.GetBlock(MergeBlockId);
    AI3D::CORE::ATData atdatatemp;
    MergeBlock->GetATData()->EraseDuplicateImages(atdatatemp);
    MergeBlock->MakeBlockFromATData(atdatatemp);
    AI3D::CORE::BlockObject::BlockExportOptions opt;
    opt.export_tiepoint_ = true;

    // opt.srs_ = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(atdata->GetLocalSrs());
    opt.srs_.ID = 0;
    opt.export_not_registered_ = false;
    opt.export_controlpoint_ = false;

    MergeBlock->ExportATXML(outdir, opt);
    return 1000;
}

//????????????????????9??????????????????????????,
// ????????tile??bb???????????????at??????????????????????????????????
//in:xml?????min????????max?????? out:??????
//E:\TestData\testcolmap2gs\aosen\mok\aosen\Block_2\Reconstruction_5\RB.bin 
//E:\TestData\testcolmap2gs\aosen\mok\aosen\Block_2\Reconstruction_5\Tile_+000_+000\1.xml -585.28173828125 -483.09375 -418.07000732421877 -485.28173828125 -383.09375 160.61000061035157
int RunExtractImageByROI(int argc, char** argv)
{
    std::string input_path = argv[1];
    std::string output_path = argv[2];

    AI3D::CORE::BlockObject block, blockout;
    AI3D::CORE::Application::Getinstance().SetProjLibENV();
    auto atdata = std::make_shared<AI3D::CORE::ATData>();
    auto ext = AI3D::CORE::File::GetFileExtension(input_path);
    AI3D::CORE::String::StringToLower(&ext);
    if (ext == ".bin")
    {
        block.LoadATBinary(input_path, atdata);
    }
    else
    {
        block.LoadATXML(input_path, atdata, false);
    }

    auto points = atdata->GetPoints3D();
    ABBox3d box;
    if (argc > 4)
    {

        //  double xmin = std::atof(argv[2]);
        box.min() = Eigen::Vector3d{ std::atof(argv[3]),std::atof(argv[4]),std::atof(argv[5]) };
        box.max() = Eigen::Vector3d{ std::atof(argv[6]),std::atof(argv[7]),std::atof(argv[8]) };
    }
    else
    {
        AI3D::CORE::ProductionOptions def;
        def.load(argv[3]);
        box = def.tilebb_.cast<double>();
    }
    std::set<point3D_t> ids;

    for (const auto& iter : points)
    {
        if (box.contains(iter.second.GetXYZ()))
        {
            ids.insert(iter.first);
        }
    }
    AI3D::CORE::ATData atdatatemp;
    if (ids.empty())
    {
        return 1001;
    }
    atdata->ExtractATDataByTiepoints(ids, atdatatemp);


    blockout.MakeBlockFromATData(atdatatemp);
    AI3D::CORE::BlockObject::BlockExportOptions opt;
    opt.export_tiepoint_ = true;

    opt.srs_ = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(atdata->GetLocalSrs());
    opt.srs_.ID = 0;
    opt.export_not_registered_ = false;
    opt.export_controlpoint_ = false;

    blockout.ExportATXML(output_path, opt);
    return 1000;
}
//bExtractByBB E:\TestData\testcolmap2gs\aosen\mok\aosen\aosen\Block_2\Reconstruction_1\RB.bin 
// E:\TestData\testcolmap2gs\aosen\mok\aosen\aosen\Block_2\Reconstruction_1\
//  E:\TestData\testcolmap2gs\aosen\mok\aosen\aosen\Block_2\Reconstruction_1\Productions\Production_1\*/
int RunBatchExtractImageByROIFromTaskDef(int argc, char** argv)
{
    std::string input_path = argv[1];
    std::string output_path = argv[2];

    AI3D::CORE::BlockObject block, blockout;
    AI3D::CORE::Application::Getinstance().SetProjLibENV();
    auto atdata = std::make_shared<AI3D::CORE::ATData>();
    auto ext = AI3D::CORE::File::GetFileExtension(input_path);
    AI3D::CORE::String::StringToLower(&ext);
    if (ext == ".bin")
    {
        block.LoadATBinary(input_path, atdata);
    }
    else
    {
        block.LoadATXML(input_path, atdata, false);
    }

    auto points = atdata->GetPoints3D();
    auto files = AI3D::CORE::File::GetRecursiveFileList(argv[3]);
    std::cout << "---filesize-  " << __LINE__ << files.size() << std::endl;
    for (auto& file : files)
    {
        ABBox3d box;
        std::string  name = AI3D::CORE::File::GetFileName(file);
        AI3D::CORE::String::StringToLower(&name);
        //if (AI3D::CORE::String::StringContains(name, "task_def"))
        if (AI3D::CORE::String::StringContains(name, TASK_DEF_BIN_PREFIX))
        {
            AI3D::CORE::ProductionOptions def;
            def.load(file);
            box = def.tilebb_.cast<double>();

            std::set<point3D_t> ids;

            for (const auto& iter : points)
            {
                if (box.contains(iter.second.GetXYZ()))
                {
                    ids.insert(iter.first);
                }
            }
            AI3D::CORE::ATData atdatatemp1, atdatatemp;
            if (ids.empty())
            {
                return 1001;
            }
            atdata->ExtractATDataByTiepoints(ids, atdatatemp1);
            auto vmids = atdatatemp1.GetRegImageIds();
            std::set<image_t> mids(vmids.begin(), vmids.end());

            atdata->ExtractATDataByImages(mids, atdatatemp);
            blockout.MakeBlockFromATData(atdatatemp);
            AI3D::CORE::BlockObject::BlockExportOptions opt;
            opt.export_tiepoint_ = true;

            opt.srs_ = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(atdata->GetLocalSrs());
            opt.srs_.ID = 0;
            opt.export_not_registered_ = false;
            opt.export_controlpoint_ = false;
            std::string recname = AI3D::CORE::File::GetParentDir(def.project_path_) + "/" + def.item_path_ + ".xml";
            std::string  filenname = AI3D::CORE::File::GetFileNameWithoutExtension(recname);
            recname = AI3D::CORE::File::GetParentDir(recname);
            std::string outfile = output_path + "/" + filenname + "/" + filenname + ".xml";

            blockout.ExportATXML(outfile, opt);
        }
    }
    return 1000;
}



void ToColmapForGS(std::string indir, std::string out, std::string atout)
{
    AI3D::CORE::BlockObject block;
    std::string file = indir;//???????

    std::string out1 = atout;
    out1 = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(out1)));
    out = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(out)));
    AI3D::CORE::Application::Getinstance().SetProjLibENV();
    AI3D::CORE::File::CreateDirIfNotExists(out, true);
    auto atdata = std::make_shared<AI3D::CORE::ATData>();

    auto ext = AI3D::CORE::File::GetFileExtension(file);
    AI3D::CORE::String::StringToLower(&ext);
    if (ext == "bin")
    {
        block.LoadATBinary(file, atdata);
    }
    else
    {
        block.LoadATXML(file, atdata, false);
    }
    block.SetATData(atdata);
    //?????????????????
    std::string undistortpath = out + "/images/";
    AI3D::CORE::File::CreateDirIfNotExists(undistortpath);
    AI3D::CORE::UndistortCameraOptions_s undistopt;
    block.UndistortBlock(undistortpath, undistopt);

    //?????
    std::string campath = out1 + "/sparse/0/";
    AI3D::CORE::File::CreateDirIfNotExists(campath);
    colmap::Reconstruction rec(*block.GetATData().get());

    rec.Write(campath);
    std::cout << " write end " << std::endl;
    //????????????????????????colmap???????????????colmap???????????cc???????
    AI3D::CORE::BlockObject::BlockExportOptions opt;
    opt.srs_ = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(atdata->GetLocalSrs());
    opt.export_tiepoint_ = true;
    block.ExportATXML(campath + "new.xml", opt);
}

//??xml???colmap??????in:xml,out???????????????
//
int RunToGuassColmap(int argc, char** argv)
{
    std::string input_path = argv[1];
    std::string outimgput_path = argv[2];
    std::string outputsfm_path1 = argv[3];
    /* input_path = UTF82GBK(input_path);
     outimgput_path = UTF82GBK(outimgput_path);
     outputsfm_path1 = UTF82GBK(outputsfm_path1);*/
    ToColmapForGS(input_path, outimgput_path, outputsfm_path1);
    return 1000;
}

int RunWindowsFc(int argc, char** argv)
{
    std::string input_path1 = argv[1];
    std::string input_path2 = argv[2];
    std::string outbat_path = argv[3];
    std::string outbatname = argv[4];
    auto files1 = AI3D::CORE::File::GetFileList(input_path1);

    std::string batfile = outbat_path + "/" + outbatname;
    batfile += ".bat";
    std::ofstream txtfile = File::OpenOfstreamUtf8(batfile, std::ios::trunc);

    for (auto& file : files1)
    {
        std::string name = AI3D::CORE::File::GetFileName(file);
        auto file2 = input_path2 + "/" + name;



        {
            txtfile << "fc  " << file << " " << file2 << " >> " << batfile << ".txt";

            txtfile << std::endl;;
        }

        // ToColmapForGS(xmlfile,outdir, atout);
    }
    txtfile.close();
    return 1000;
}


//?????????GS??views??????????????????
int RunBatchToGuassColmap(int argc, char** argv)
{
    std::string input_path = argv[1];
    std::string outbat_path = AI3D::CORE::Application::Getinstance().GetAPPPath();
    std::string outdir = argv[2];

    auto dirs = AI3D::CORE::File::GetDirList(input_path);
    for (std::vector<std::string>::iterator iter = dirs.begin();
        iter != dirs.end();)
    {

        std::string dir = *iter;
        dir = AI3D::CORE::File::GetDirName(dir, true);
        AI3D::CORE::String::StringToLower(&dir);

        if (strstr(dir.c_str(), "model") != NULL ||
            strstr(dir.c_str(), "tile") != NULL)
        {
            iter++;
        }
        else
        {
            iter = dirs.erase(iter);

        }
    }
    std::string batfile = outbat_path + "/batchToGS.bat";
    std::ofstream txtfile = File::OpenOfstreamUtf8(batfile, std::ios::trunc);
    outdir = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(outdir)));
    for (auto& dir : dirs)
    {
        std::string file = dir + "/" + PRODUCTIONVIEWIDSBIN;
        if (!AI3D::CORE::File::ExistsFile(file))
        {
            continue;
        }

        std::string tilename = AI3D::CORE::File::GetDirName(dir, true);

        std::string xmlfile = dir + "/" + tilename + ".xml";

        /* std::string outdir = argv[2];*/
        std::string atout = outdir + tilename;
        AI3D::CORE::File::CreateDirIfNotExists(atout);


        {
            txtfile << "MokTools xmlToGS  " << xmlfile << " " << outdir << " " << atout;

            txtfile << std::endl;;
        }

        // ToColmapForGS(xmlfile,outdir, atout);
    }
    txtfile.close();
    return 1000;
}
//??taskdef????????????????
int RunCopyImgFromTaskDef(int argc, char* argv[])
{
    ATTaskInfo attask;
    std::string file = argv[1];
    attask.load(file);

    std::cout << attask.task_.imgIds_.size()<< " "<< argv[4] << std::endl;;
    auto imgidskeep = attask.task_.imgIds_;
    if (imgidskeep.empty())
    {
        std::cout << "Nothing here." << std::endl;
    }
    //?????????????????????????????????????????sourcedata.json???????xml???
    std::string sourcefile = argv[2];
    std::string outdir = argv[3];
    int mode = 0;
    std::string mask="";
    if (argc > 4)
    {
        mask = argv[4];
       
    }
    mode = (mask == ".skf" ? 1 : 0);
    if(mode == 1)
        std::cout << "copy feature files." << std::endl;
    else
    {
        std::cout << "copy images." << std::endl;
    }
    
    AI3D::CORE::BlockObject block;
    ATCommandSet::LoadBlock(sourcefile, block);
    auto atdata = block.GetATData();
  

  

    std::set<image_t> imgidsremove;
  
    if (mode == 1)
    {
        //std::string projectpath =AI3D::CORE::File::GetParentDir(attask.projectFile2_);
        std::string projectpath = AI3D::CORE::File::GetParentDir(attask.projectFile_);
        int count = 0;
        std::vector<std::string> imagesfile;
        for (auto& iter : imgidskeep)
        {
            //????????????;
            //?????????????????????????????hash
            std::string path = atdata->GetImage(iter).GetPath();
            path = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(path));
            std::string imagefullname = path + atdata->GetImage(iter).GetName();
            projectpath = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(projectpath));
            std::string featurefolder = projectpath + "feature/";
            std::string hashfile = AI3D::CORE::String::ToSHA256(imagefullname+"kn20000");
            std::string subfolder = hashfile.substr(0, 2);
            std::string filehashname = hashfile.substr(2);
            std::string featurefile = featurefolder + subfolder + "/" + filehashname + ".skf";
            std::vector<std::string> keyfiles;
            keyfiles.push_back(featurefile);
            bool ret = false;
            if (AI3D::CORE::File::IsFileExistent(featurefile))
            {


                AI3D::CORE::File::CreateDirIfNotExists(outdir + "/feature/" + subfolder);

                ret = AI3D::CORE::File::CopyFiles(keyfiles, outdir + "/feature/" + subfolder, false);
            }
           if (ret)
           {
               count++;
           }
           else
           {
               imagesfile.push_back(imagefullname);
           }
           
        }
        int failednum = imgidskeep.size() - count;
        std::cout << "total " << imgidskeep.size() << " successed " << count << " failed " << count << std::endl;
        if (failednum > 0)
        {
            for (auto& iter : imagesfile)
            {
                std::cout << iter << std::endl;
            }
        }

    }
    else
    {
        std::vector<std::string> imagesfile;
        for (auto& iter : imgidskeep)
        {
            imagesfile.push_back(atdata->GetImage(iter).GetPath() + "/" + atdata->GetImage(iter).GetName());
        }
        AI3D::CORE::File::CopyFiles(imagesfile, outdir, false);
    }

   

    return 1000;
}

int RunCopyImgFromXML(int argc, char* argv[])
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
        imagesfile.push_back(atdata->GetImage(iter).GetPath() + "/" + atdata->GetImage(iter).GetName());
    }

    AI3D::CORE::File::CopyFiles(imagesfile, argv[2], false);

    return 1000;
}

int RunExtractFromBaseXML(int argc, char** argv)
{
    AI3D::CORE::BlockObject block, blockref;
    // std::string basepath = "E:/TestData/aosen/";
    std::string importxmlfile = argv[1];// "neiye07-08-09-4548.xml";
    std::string refxmlfile = argv[2];// "base.xml";
    auto atdata = std::make_shared<ATData>();
    block.LoadATXML(importxmlfile, atdata, false);
    block.SetATData(atdata);
    auto atdataref = std::make_shared<ATData>();
    blockref.LoadATXML(refxmlfile, atdataref, false);
    blockref.SetATData(atdataref);


    std::set<image_t> image_ids;
    for (auto& imageref : atdataref->GetImages())
    {
        std::string nameref = imageref.second.GetName();
        AI3D::CORE::String::StringToLower(&nameref);
        for (auto& image : atdata->GetImages())
        {
            std::string name = image.second.GetName();
            AI3D::CORE::String::StringToLower(&name);
            if (nameref == name)
            {
                image_ids.insert(image.second.GetImageId());
                break;
            }
        }
    }
    ATData data;

    atdata->ExtractATDataByImages(image_ids, data);


    AI3D::CORE::BlockObject blocktemp;
    blocktemp.SetId(0);

    /* Eigen::Vector3d offset{447731,4431317,0};

     for (auto& iter : data.GetImagesMutual())
     {
         iter.second.GetPositionMutual() -= offset;
     }
     for (auto& iter : data.GetPoints3DMutual())
     {
         iter.second.GetXYZMutual() -= offset;
     }*/

    blocktemp.MakeBlockFromATData(data);



    AI3D::CORE::BlockObject::BlockExportOptions opt;
    opt.export_tiepoint_ = true;
    opt.srs_ = block.GetBlockSRS();// AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(data.GetLocalSrs());
    opt.srs_.ID = 0;
    opt.export_not_registered_ = false;
    opt.export_controlpoint_ = false;
    std::string atxmlfile = argv[3];// basepath + "/neiye07_08.xml";
    blocktemp.ExportATXML(atxmlfile, opt);
    return 1000;
}

int RunParseImagesViewBin(int argc, char** argv)
{
    std::string input_path = argv[1];
    std::string output_path = argv[2];



    std::set<image_t> ids;
    AI3D::CORE::ATData::LoadViewsBin(input_path, ids);


    std::ofstream txtfile = File::OpenOfstreamUtf8(output_path, std::ios::trunc);
    txtfile << ids.size() << std::endl;
    for (auto& iter1 : ids)
    {
        txtfile << iter1 << std::endl;;
    }
    txtfile.close();

    return 1000;
}
// ???sourcedata ???? xml??id??argv[1]:???id???????????????????????????
//e.g. sameID E:\TestData\?????\0425-song0605\Block_7(Block_2)-export-raw.xml E:\TestData\?????\0425-song0605\source_data(2).json E:\TestData\?????\0425-song0605\1.xml
int RunMakeIDSame(int argc, char** argv)
{
    std::string basefile = argv[1];
    std::string srcfile = argv[2];
    std::string outfile = argv[3];
    //basefile = /*GBK2UTF8*/(basefile);
    //srcfile = /*GBK2UTF8*/(basefile);
   // outfile = UTF82GBK(outfile);
    //
    AI3D::CORE::BlockObject block, blocktemp;

    int ret = ATCommandSet::LoadBlock(basefile, block);
    if (ret != AI3D_SUCCESS)
    {
        std::cout << "load falied " << basefile << std::endl;
        return ret;
    }
    //ATData atdata1;
    ret = ATCommandSet::LoadBlock(srcfile, blocktemp);
    if (ret != AI3D_SUCCESS)
    {
        std::cout << "load falied " << basefile << std::endl;
        return ret;
    }
    auto atdata = block.GetATDataMutual();
    auto atdata1 = blocktemp.GetATDataMutual();
    //ATCommandSet::LoadSourceDataJson1(atdata1, srcfile);
    auto imagesbase = atdata->GetImages();
    auto imagessrc = atdata1->GetImages();
    std::map<std::string, std::pair<image_t, image_t>> ids;
    std::set<image_t> idsetbase, idsetsrc;
    for (auto& iter : imagesbase)
    {
        std::string name = iter.second.GetName();
        name = AI3D::CORE::File::GetFileNameWithoutExtension(name);
        ids[name].first = iter.first;
        ids[name].second = -1;
        idsetbase.insert(iter.first);
    }

    for (auto& iter : imagessrc)
    {
        std::string name = iter.second.GetName();
        name = AI3D::CORE::File::GetFileNameWithoutExtension(name);
        if (ids.count(name))
        {
            ids[name].second = iter.first;
        }
        else
        {
            ids[name].first = -1;
        }
        idsetsrc.insert(iter.first);
    }
    int count = 0;
    for (auto& iter : ids)
    {
        if (iter.second.first != iter.second.second && (iter.second.first != -1)/* && (iter.second.second != -1)*/)
        {
            count++;
            //  std::cout << " " << iter.second.first << std::endl;
        }
    }
    std::cout << "total different " << count << std::endl;
    if (count == 0)
    {
        //
        std::cout << "use the raw file please. " << std::endl;
        //  return 1000;
    }
    image_t beginid = std::max(*idsetbase.rbegin(), *idsetsrc.rbegin());
    //??Ixml??????????????????????????
    std::set<image_t> lastids;
    EIGEN_STL_UMAP(image_t, Image) images;
    std::map<group_t, std::set<image_t> > g_imgids;


    for (auto& iter : imagesbase)
    {
        auto image = iter.second;
        std::string name = iter.second.GetName();
        name = AI3D::CORE::File::GetFileNameWithoutExtension(name);
        if (ids.count(name))
        {
            auto id = ids[name].second;
            if (id == -1)
            {
                beginid++;
                id = beginid;

            }
            image.SetImageId(id);
            images[id] = image;
            g_imgids[image.GetPhotoGroupID()].insert(id);
        }

    }
    atdata->GetImagesMutual() = images;
    block.SetATData(atdata);
    EIGEN_STL_UMAP(group_t, PhotoGroup)& groups = block.GetPhotoGroupsMutual();
    for (auto& group : groups)
    {
        group.second.SetGroupImage(g_imgids.at(group.first));
    }
    AI3D::CORE::BlockObject::BlockExportOptions opt;

    opt.srs_ = block.GetBlockSRS();

    opt.export_not_registered_ = false;
    opt.export_controlpoint_ = false;
    std::string atxmlfile = outfile;// basepath + "/neiye07_08.xml";
    block.ExportATXML(atxmlfile, opt);
    return 1000;
}

int RunSaveSourceData2Xml(int argc, char** argv)
{
    std::string basefile = argv[1];
    std::string outfile = argv[2];
    AI3D::CORE::BlockObject block, blockout;

    int ret = ATCommandSet::LoadBlock(basefile, block);
    if (ret != AI3D_SUCCESS)
    {
        std::cout << "load falied " << basefile << std::endl;
        return ret;
    }
    auto atdata = block.GetATDataMutual();
    blockout.MakeBlockFromATData(*block.GetATDataMutual());
    AI3D::CORE::BlockObject::BlockExportOptions opt;
    opt.export_tiepoint_ = true;
    // std::cout << [0].definition<< " "<< atdata->GetOriginSrs();
    opt.srs_ = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(atdata->GetLocalSrs());
    opt.srs_.ID = 0;
    opt.export_not_registered_ = false;
    opt.export_controlpoint_ = false;

    blockout.ExportATXML(outfile, opt);
    //
    //AI3D::CORE::BlockObject::BlockExportOptions opt;
    //MakeBlockFromATData
    ///*opt.srs_ = block.GetBlockSRS();

    //opt.export_not_registered_ = false;
    //opt.export_controlpoint_ = false;*/
   
    //block.ExportATDataToXML(outfile, opt,*block.GetATDataMutual());
   
    return 1000;
}

int RunXml2SaveSourceData(int argc, char** argv)
{
    std::string basefile = argv[1];
    std::string outfile = argv[2];
    AI3D::CORE::BlockObject block, blocktemp;

    int ret = ATCommandSet::LoadBlock(basefile, block);
    if (ret != AI3D_SUCCESS)
    {
        std::cout << "load falied " << basefile << std::endl;
        return ret;
    }
    Eigen::Vector3d possigma{ 10,10,10 };
    auto atdata = block.GetATDataMutual();
    ATOptions atoptions;
    ATCommandSet::SaveSourceDataJson(*atdata.get(), outfile,  possigma);
    return 1000;
}

int RunSourceData2Txt(int argc, char** argv)
{
    std::string basefile = argv[1];
    std::string outfile = argv[2];

    AI3D::CORE::BlockObject block, blocktemp;

    int ret = ATCommandSet::LoadBlock(basefile, block);
    if (ret != AI3D_SUCCESS)
    {
        std::cout << "load falied " << basefile << std::endl;
        return ret;
    }

    std::ofstream txtfile = File::OpenOfstreamUtf8(outfile, std::ios::trunc);
    if (!txtfile.is_open())
    {

        return 1001;
    }
    auto atdata = block.GetATDataMutual();

    auto imagesbase = atdata->GetImages();
    for (auto& iter : imagesbase)
    {
        auto image = iter.second;
        txtfile << image.GetPositionMutual().x() << " " << image.GetPositionMutual().y() << " " << image.GetPositionMutual().z() << std::endl;
    }
    txtfile.close();

    return 1000;
}

template<typename Scalar, int Dim>
using Vector = Eigen::Matrix<Scalar, Dim, 1>;

typedef Vector<uint8_t, 3> Color3b;
int ReadVpc(std::string& infile, ATData& ATdata)
{


    FILE* pf = fopen(infile.c_str(), "rb");
    int num_pts;
    fread(&num_pts, sizeof(int), 1, pf);

    std::vector<Eigen::Vector3f> points(num_pts);
    // auto& tiepoints = ATdata.GetPoints3DMutual();
    /* for(auto& iter :tiepoints)
     {
         ATdata.DeletePoint3D(iter.first);
     }*/
    point3D_t index_point3d = 0;
    auto images = ATdata.GetImagesMutual();
    auto cameras = ATdata.GetCamerasMutual();
    std::vector<std::vector<std::pair<image_t, float> > > point_views(num_pts);
    for (int i = 0; i < num_pts; ++i)
    {

        fread(points[i].data(), sizeof(float), 3, pf);
        //  std::cout << points[i].x() << std::endl;
        Eigen::Vector3d xyz = points[i].cast<double>();
        int numview;
        fread(&numview, sizeof(int), 1, pf);
        point_views[i].resize(numview);
        std::vector<TrackElement> vec_trackele;
        for (int j = 0; j < numview; ++j)
        {
            image_t id;
            float w;
            fread(&id, sizeof(image_t), 1, pf);
            fread(&w, sizeof(float), 1, pf);
            if (!ATdata.GetImages().count(id))
            {
                continue;
            }
            point3D_t index_point3d;
            Eigen::Vector2d uv;
            auto& image = images[id];
            auto& camera = cameras[image.GetCameraId()];
            auto estimated_xy = AlgorithmBase::ProjectPointToImage(xyz,
                image.GetProjectionMatrix(),
                camera, false);
            Image& img = ATdata.GetImageMutual(id);
            TrackElement trackelement;
            trackelement.image_id = id;
            trackelement.point2D_idx = img.AddPoints2D(uv);
            img.SetPoint3DForPoint2D(trackelement.point2D_idx, index_point3d);
            trackelement.xy = uv;
            vec_trackele.push_back(trackelement);
        }


        Point3D point3d;


        //1.???????
        Track track;




        track.AddElements(vec_trackele);
        point3d.SetId(index_point3d);
        point3d.SetTrack(track);

        point3d.SetXYZ(xyz);


        ATdata.GetPoints3DMutual().insert(std::make_pair(index_point3d, point3d));
        index_point3d++;
    }
    fclose(pf);
    std::string colorfile = File::GetParentDir(infile) + "/points_rgb.bin";
    if (File::ExistsFile(colorfile))
    {
        FILE* fp = fopen(colorfile.c_str(), "rb");

        int rgb_num_pts;
        fread(&rgb_num_pts, sizeof(int), 1, fp);
        if (num_pts != rgb_num_pts)
        {
            return 1001;
        }
        index_point3d = 0;
        std::vector<Color3b> rgbpoints(rgb_num_pts);
        for (int i = 0; i < rgb_num_pts; ++i)
        {

            fread(rgbpoints[i].data(), sizeof(uint8_t), 3, fp);
            ATdata.GetPoints3DMutual().at(index_point3d).SetColor(Eigen::Vector3i{ rgbpoints[index_point3d].x(),rgbpoints[index_point3d].y(),rgbpoints[index_point3d].z() });
            index_point3d++;
        }
        fclose(fp);
    }
    return 1000;
}
//????????????colmap???????????gs???
//????????????????????????
int RunParsePointcloudVpc(int argc, char** argv)
{
    std::string xmlfile = argv[1];
    std::string vpcfile = argv[2];
    std::string campath = argv[3];
    AI3D::CORE::BlockObject  block;
    auto atdata = std::make_shared<ATData>();
    block.LoadATXML(xmlfile, atdata, false, false);
    block.SetATData(atdata);
    ReadVpc(vpcfile, *atdata.get());
    AI3D::CORE::File::CreateDirIfNotExists(campath);
    colmap::Reconstruction rec(*atdata.get());
    rec.Write(campath);
    return 1000;
}


//????????
typedef cv::Mat_<float> cvDepthMap;
typedef cv::Mat_<uint16_t> cvNormalMapUshort;
int RunParseDepth(int argc, char** argv)
{
    std::string infile = argv[1];
    //std::string outfile = argv[2];

    FILE* pf = fopen(infile.c_str(), "rb");
    int cols = 0, rows = 0;
    fread(&cols, sizeof(int), 1ll, pf);
    fread(&rows, sizeof(int), 1ll, pf);
    cvDepthMap depthmap = cv::Mat(rows, cols, CV_32F);
    cvNormalMapUshort normalmap = cv::Mat(rows, cols, CV_16U);
    fread(normalmap.data, normalmap.step * normalmap.rows, 1ll, pf);
    fread(depthmap.data, depthmap.step * depthmap.rows, 1ll, pf);
    fclose(pf);
    /*for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {

            std::cout << depthmap.at<float>(i, j) << " n "<< normalmap.at<uint16_t>(i, j) << std::endl;;

        }
    }*/
    return 1000;
}

int RunComputeGCPError(int argc, char** argv)
{
    std::string xmlfile = argv[1];
    auto atdata = std::make_shared<ATData>();
    BlockObject block;
    block.LoadATXML(xmlfile, atdata);
    block.SetATData(atdata);
    std::map<point3D_t, std::map < image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >> gcp_error_map, gcp_error_map1;

    atdata->UpdataGCPGlobalErrorInfo(gcp_error_map1);
    for (auto& iter : atdata->GetControlPoints())
    {
        if (iter.second.GetName() == "07350J3-01-002-003")
        {
            std::cout << iter.first<< " " << iter.second.GetId() << std::endl;
            for (auto& iter1 : gcp_error_map1[iter.first])
            {
                std::cout << "image id " << iter1.first << " "<< atdata->GetImage(iter1.first).GetName() << " " << iter1.second.first.x() << " "
                    << iter1.second.first.y()<<" "<< iter1.second.second.first << std::endl;
            }

           
        }
    }
    /*for (auto& iter : gcp_error_map1)
    {
        if (iter.first == 141)
        {
            std::cout << iter.second << std::endl;
        }
    }*/

    return 1000;
}
#include "rapidjson/filewritestream.h"
#include <rapidjson/writer.h>
#include <cstdio>
namespace {

    typedef std::function<int(int, char**)> command_func_t;

    int ShowHelp(
        const std::vector<std::pair<std::string, command_func_t>>& commands) {
        using namespace colmap;

        
       

        std::cout << "Usage:" << std::endl;
        std::cout << "  colmap [command] [options]" << std::endl << std::endl;

        std::cout << "Documentation:" << std::endl;
        std::cout << "  https://colmap.github.io/" << std::endl << std::endl;

        std::cout << "Example usage:" << std::endl;
        std::cout << "  colmap help [ -h, --help ]" << std::endl;


        std::cout << "  MokTools eraseDup d:/in.xml d:/out.xml "
            << std::endl;
        std::cout << "  MokTools xmlToGS d:/in.xml d:/outimagesdir/ d:/sparsedir/ "
            << std::endl;
        std::cout << "  MokTools extractByBB d:/in.xml d:/outdir/ min max  "
            << std::endl;
        std::cout << "  MokTools batXmlToGS d:/tileparentdir d:/outdir/ "
            << std::endl;
        std::cout << "  MokTools viewsBin2Txt d:/in.bin d:/out.txt  "
            << std::endl;
        std::cout << "  MokTools bExtractByBB d:/in.bin d:/reconsdir d:/productiondir  "
            << std::endl;
        std::cout << "  MokTools extractFromBaseXML d:/in.xml d:/ref.xml d:/out.xml  "
            << std::endl;
        std::cout << "  MokTools src2Txt d:/in.json d:/out.txt"
            << std::endl;
        std::cout << "  MokTools saveXml2Src d:/in.xml d:/out.json"
            << std::endl;
        std::cout << "  MokTools copyImgXml d:/in.xml d:/outdir/"
            << std::endl;
        std::cout << "  MokTools parseDepth d:/in.dat"
            << std::endl;
        std::cout << "  MokTools parseVpc d:/in.xml d:/in.vpc "
            << std::endl;
        std::cout << "  MokTools compFile d:/dir1 d:/dir2 d:/out "
            << std::endl;
        std::cout << "  MokTools gcpXMLID d:/gcp.xml d:/source.json d:/out.xml "
            << std::endl;
        std::cout << "  MokTools gcpError d:/gcp.xml "
            << std::endl;
        std::cout << "  MokTools cpImgTaskDef d:/task_def_0.json d:/source_data.json or block.xml d:/outdir '.skf' or null"
            << std::endl;
        
        std::cout << "  ..." << std::endl << std::endl;

        std::cout << "Available commands:" << std::endl;
        std::cout << "  help" << std::endl;
        for (const auto& command : commands) {
            std::cout << "  " << command.first << std::endl;
        }
        std::cout << std::endl;

        return EXIT_SUCCESS;
    }

}  // namespace




int main(int argc, char** argv) 
{
    using namespace colmap;
  //  using namespace AI3D::CORE;
  //  BlockObject block;
  //  auto Atdata = std::make_shared<AI3D::CORE::ATData>();
  //  std::string input_path = "G:/Block_6(hukou)-enu.xml";
  //  block.LoadATXML(input_path, Atdata, false);
  //  Eigen::Vector3d possigma = { 10.,10.,10. };
  //   ATCommandSet::SaveSourceDataJson(*Atdata.get(),input_path+".json", possigma);
  //   

  //if(0)
  //  {
  //    
  //     
  //      rapidjson::Document document;
  //      document.SetObject();
  //      rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

  //      rapidjson::Value coordinatejson(rapidjson::kObjectType);
  //      rapidjson::Value camerasjson(rapidjson::kArrayType);
  //      rapidjson::Value imagesjson(rapidjson::kArrayType);
  //      auto atdatasrs = CoordinateDescriptor::GetSRSFromDefinition(Atdata->GetLocalSrs());
  //      if (1)
  //      {
  //          auto srs = Atdata->GetDefaultEnuSRS();

  //          std::string src_srs_definition = Atdata->GetLocalSrs();
  //          Atdata->TransFormImages(src_srs_definition, srs.definition);
  //          atdatasrs = srs;
  //      }
  //      atdatasrs.CreateJson(coordinatejson, document);
  //      Eigen::Vector3d possigma = { -1.0,-1.0,-1.0 };
  //      bool withpos = possigma.x() >= 0.;
  //     
  //      {
  //          auto cameras = Atdata->GetCameras();
  //          for (auto& camiter : cameras)
  //          {

  //              Camera cam = camiter.second/*.GetCamera()*/;
  //              ////?????????????
  //              if (cam.GetFocalLengthX() <= 0)
  //              {
  //                  //????????????????????f_35eq
  //                  double f_35eq = Application::Getinstance().ParseConfig().focal_length;
  //                  LOGW(String::StringPrintf("no focal we set f_35eq = %f", f_35eq));
  //                  double focal_pix = std::max(cam.GetWidth(), cam.GetHeight()) * f_35eq / 36;
  //                  cam.SetFocalLengthX(focal_pix);
  //                  cam.SetFocalLengthY(focal_pix);
  //                  cam.SetPrincipalPointX(cam.GetWidth() / 2);
  //                  cam.SetPrincipalPointY(cam.GetHeight() / 2);

  //              }
  //              rapidjson::Value dst(rapidjson::kObjectType);

  //              // id
  //              dst.AddMember("id", rapidjson::Value(cam.GetCameraId()), allocator);

  //              rapidjson::Value item(rapidjson::kObjectType);
  //              item.AddMember("camera_name", rapidjson::Value(camiter.second.GetCameraName().c_str(), allocator), allocator);
  //              double width, height;
  //              width = height = -DBL_MAX;
  //              width = cam.GetWidth();
  //              height = cam.GetHeight();
  //              if (width == -DBL_MAX || height == -DBL_MAX)
  //              {
  //                  LOGE("camera has no width or height!");
  //                  return false;
  //              }
  //              item.AddMember("width", rapidjson::Value(int(width)), allocator);
  //              item.AddMember("height", rapidjson::Value(int(height)), allocator);

  //              //item.AddMember("sensorSize", rapidjson::Value(cam.sensor_size), allocator);
  //              //item.AddMember("focalLength", rapidjson::Value(cam.focal_length), allocator);
  //              item.AddMember("projection_model", rapidjson::Value(0), allocator);
  //              rapidjson::Value parameters(rapidjson::kArrayType);

  //              //??????Full_OpenCV????
  //              for (int i = 0; i < 4; i++)
  //              {
  //                  parameters.PushBack(cam.GetParams()[i], allocator);
  //              }
  //              parameters.PushBack(cam.GetParams()[4], allocator);//k1
  //              parameters.PushBack(cam.GetParams()[5], allocator);//k2
  //              parameters.PushBack(cam.GetParams()[8], allocator);//k3
  //              parameters.PushBack(cam.GetParams()[7], allocator);//p2//???????
  //              parameters.PushBack(cam.GetParams()[6], allocator);//p1
  //              parameters.PushBack(0, allocator);//p3
  //              item.AddMember("parameters", parameters, allocator);
  //              // item
  //              dst.AddMember("meta_data", item, allocator);

  //              camerasjson.PushBack(dst, allocator);
  //          }
  //      }

  //      // images
  //      {
  //          auto cameras = Atdata->GetCameras();
  //          for (auto& img : Atdata->GetImages())
  //          {
  //              // path
  //              std::string imagefullpath = File::EnsureUnifySlash(img.second.GetPath() + PATH_SEPARATOR_STR + img.second.GetName());

  //              ////??????????????(?????????????????)

  //              //if (!File::IsFileExistent(imagefullpath))
  //              //{
  //              //	continue;
  //              //}

  //              rapidjson::Value dst(rapidjson::kObjectType);

  //              // id
  //              dst.AddMember("id", rapidjson::Value(img.first), allocator);

  //              // item
  //              rapidjson::Value item(rapidjson::kObjectType);
  //              long long timestamp;
  //              std::string TimeOrigin = img.second.GetExifinfo().dateTime;
  //              std::vector<std::string> time_vec = String::StringSplit(TimeOrigin, "- T :");
  //              if (!TimeOrigin.empty())
  //              {
  //                  TimeOrigin.clear();
  //                  uint8_t i_time = 0;
  //                  while (i_time < 6)
  //                  {
  //                      TimeOrigin += time_vec[i_time++];
  //                  }
  //                  timestamp = std::atoll(TimeOrigin.c_str());
  //              }
  //              else
  //              {
  //                  timestamp = 0;
  //              }

  //              item.AddMember("camera_id", rapidjson::Value(img.second.GetCameraId()), allocator);
  //              item.AddMember("capture_time", rapidjson::Value(timestamp), allocator);
  //              item.AddMember("dewrap_flag", rapidjson::Value(img.second.GetDewrapFlag()), allocator);
  //              item.AddMember("width", rapidjson::Value(int(img.second.GetWidth())), allocator);
  //              item.AddMember("height", rapidjson::Value(int(img.second.GetHeight())), allocator);
  //              float f_35eq = 0;
  //              Camera camera;
  //              if (cameras.find(img.second.GetCameraId()) != cameras.end())
  //              {
  //                  camera = cameras[img.second.GetCameraId()];
  //                  if (camera.GetHeight() != img.second.GetHeight() || camera.GetWidth() != img.second.GetWidth())

  //                  {
  //                      LOGE("The width or height of image is not equal to camera!");
  //                      return false;
  //                  }
  //              }
  //              else
  //              {
  //                  LOGE("Image has invalid camera_id in writing SourceData.json!");
  //                  return false;
  //              }
  //              auto config = Application::Getinstance().ParseConfig();
  //              if (camera.GetFocalLengthIn35mm() == 0)
  //              {
  //                  f_35eq = config.focal_length;
  //              }
  //              else
  //              {
  //                  f_35eq = camera.GetFocalLengthIn35mm();
  //              }
  //              //	item.AddMember("focal_length_in_35mm", rapidjson::Value(f_35eq), allocator);
  //              if (withpos)
  //              {
  //                  //item.AddMember("position_constant", rapidjson::Value(false), allocator);

  //                  //rotation
  //                  if (img.second.HasRotationMatrix())
  //                  {
  //                      rapidjson::Value rotation(rapidjson::kArrayType);
  //                      Eigen::Matrix3d R = img.second.GetRotationMatrix();
  //                      for (int i = 0; i < 3; i++)
  //                      {
  //                          for (int j = 0; j < 3; j++)
  //                          {
  //                              rotation.PushBack(R(i, j), allocator);
  //                          }
  //                      }
  //                      item.AddMember("orientation", rotation, allocator);
  //                  }

  //                  // pos

  //                  // pos_sigma


  //                  if (img.second.HasPosition())
  //                  {
  //                      rapidjson::Value pos(rapidjson::kArrayType);

  //                      pos.PushBack(img.second.GetPosition()[0], allocator);
  //                      pos.PushBack(img.second.GetPosition()[1], allocator);
  //                      pos.PushBack(img.second.GetPosition()[2], allocator);

  //                      {
  //                          item.AddMember("pos", pos, allocator);

  //                          if (withpos)
  //                          {
  //                              rapidjson::Value pos_sigma(rapidjson::kArrayType);
  //                              for (int i = 0; i < 3; i++)
  //                              {
  //                                  pos_sigma.PushBack(possigma[i], allocator);
  //                              }
  //                              item.AddMember("pos_sigma", pos_sigma, allocator);
  //                          }
  //                      }
  //                  }
  //              }
  //              dst.AddMember("meta_data", item, allocator);


  //              std::string imagefullpath2 = GBK2UTF8(imagefullpath);
  //              dst.AddMember("path", rapidjson::Value(imagefullpath2.c_str(), allocator), allocator);

  //              imagesjson.PushBack(dst, allocator);
  //          }
  //      }

  //      document.AddMember("coordinate_system", coordinatejson, allocator);
  //      document.AddMember("camera_meta_data", camerasjson, allocator);
  //      document.AddMember("image_meta_data", imagesjson, allocator);
  //     /* Document::AllocatorType& allocator = document.GetAllocator();
  //      document.AddMember("name", Value().SetString("John Doe", allocator), allocator);
  //      document.AddMember("age", 30, allocator);
  //      document.AddMember("is_student", false, allocator);*/

  //      std::string file_path = "G:/2.json";
  //      FILE* fp = fopen(file_path.c_str(), "wb");
  //      char writeBuffer[65536];
  //      rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));

  //      rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
  //      document.Accept(writer);

  //      fclose(fp);
  //  }
  //
    
    InitializeLog(argv);
    AI3D::CORE::Application::Getinstance().SetUpGDALSettings();
    AI3D::CORE::Application::Getinstance().SetProjLibENV();
    /*auto atdata = std::make_shared<ATData>();
    AI3D::CORE::ATCommandSet::LoadATBinary("C:/data/Projects/NewProject/Block_3/SCSFR.bin", atdata);*/
    std::vector<std::pair<std::string, command_func_t>> commands;
    commands.emplace_back("eraseDup", &RunEraseDuplicateImageElement);
    commands.emplace_back("xmlToGS", &RunToGuassColmap);
    commands.emplace_back("batXmlToGS", &RunBatchToGuassColmap);
    commands.emplace_back("extractByBB", &RunExtractImageByROI);
    commands.emplace_back("viewsBin2Txt", &RunParseImagesViewBin);
    commands.emplace_back("bExtractByBB", &RunBatchExtractImageByROIFromTaskDef);
    commands.emplace_back("extractFromBaseXML", &RunExtractFromBaseXML);
    commands.emplace_back("BatEraseDup", &RunBacthEraseDuplicateImageElement);
    commands.emplace_back("sameID", &RunMakeIDSame);
    commands.emplace_back("src2Txt", &RunSourceData2Txt);
    commands.emplace_back("saveXml2Src", &RunXml2SaveSourceData);
    commands.emplace_back("saveSrc2Xml", &RunSaveSourceData2Xml);
    
    commands.emplace_back("copyImgXml", &RunCopyImgFromXML);
    commands.emplace_back("parseDepth", &RunParseDepth);
    commands.emplace_back("parseVpc", &RunParsePointcloudVpc);
    commands.emplace_back("compFile", &RunWindowsFc);
    commands.emplace_back("gcpXMLID", &RunMakeGCPIDSame);
    commands.emplace_back("gcpError", &RunComputeGCPError);
    commands.emplace_back("cpImgTaskDef", &RunCopyImgFromTaskDef);
    

    if (argc == 1) {
        return ShowHelp(commands);
    }

    const std::string command = argv[1];
    if (command == "help" || command == "-h" || command == "--help")
    {
        return ShowHelp(commands);
    }
    else {
        command_func_t matched_command_func = nullptr;
        for (const auto& command_func : commands)
        {
            if (command == command_func.first)
            {
                matched_command_func = command_func.second;
                break;
            }
        }
        if (matched_command_func == nullptr)
        {
            std::cerr <<
                "ERROR: Command `%s` not recognized. To list the "
                "available commands, run `MokTools help`." <<
                command.c_str()
                << std::endl;
            return 1;
        }
        else
        {
            int command_argc = argc - 1;
            char** command_argv = &argv[1];
            command_argv[0] = argv[0];
            return matched_command_func(command_argc, command_argv);
        }
    }

    return ShowHelp(commands);
}