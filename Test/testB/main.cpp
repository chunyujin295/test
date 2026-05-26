#include <string>

#include <Core/BlockObject.h>
#include "Core/File.h"

#include <Core/Tiling.h>
#include "Core/ReconstructionCommandSet.h"
#include "Core/ReconstructionObject.h"
#include "Core/ReconstructionOptions.h"
#include "Core/CoordinateSystem.h"
using namespace AI3D::CORE;


struct version_copy_s
{
    version_copy_s(std::string version)
    {
        auto strs = AI3D::CORE::String::StringSplit(version, ".");

        std::string versionwithoutdot = "";
        for (auto& iter : strs)
        {
            versionwithoutdot += iter;
        }
        versionCode = std::atoi(versionwithoutdot.c_str());
        versionName = version;


    };
    int versionCode;
    std::string versionName = "";
    std::string description = "fourt release for OTA";
       
   
    bool save(std::string file)
    {
        std::string json_str;
        rapidjson::Document document;
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        document.SetObject();

        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

       
        document.AddMember("versionCode", rapidjson::Value(versionCode), allocator);
        document.AddMember("versionName", rapidjson::Value(versionName.c_str(), allocator), allocator);
        document.AddMember("description", rapidjson::Value(description.c_str(), allocator), allocator);
        document.Accept(writer);
        json_str = buffer.GetString();
        std::ofstream ofs = File::OpenOfstreamUtf8(file, std::ios::out);
        if (ofs.fail())
            return false;

        ofs << json_str;
        ofs.close();
        return true;
    }
    bool load(std::string file_path)
    {
        std::string blkcontent;

        bool ret = RapidJson::ReadFile(file_path, blkcontent);
        if (!ret)
        {
            LOGE(String::StringPrintf("File: %s was Read Error", file_path));
            return false;
        }

        rapidjson::Document doc_blk;

        if (doc_blk.Parse(blkcontent.data()).HasParseError())
        {
            LOGE(String::StringPrintf("%s :parse block file  error!", file_path));
            return false;
        }

        if (!doc_blk.IsObject())
        {
            LOGE("Parse block file error!");
            return false;
        }
        if (doc_blk.HasMember("versionCode"))
        {
            versionCode = (doc_blk["versionCode"].GetInt());
        }

        if (doc_blk.HasMember("versionName"))
        {
            versionName = (doc_blk["versionName"].GetString());
        }

        if (doc_blk.HasMember("description"))
        {
            description = (doc_blk["description"].GetString());
        }
        
    }
    //Eigen::Vector3u untexturecolor_;


};

//???????????��????????????��?? / newpackage / bin/**.exe??
//??��?????????1??D:\jiaojie\mok\0.00.017\build\Mohacker\Bin\x64\Release?exe?????????
// ????????????��?dll??exe??png??ini??sensor_width_camera_database.txt??version.json???????????
//????2?????????��????????��??????????????????��?
//???????????????: ?��??/newpackage/bin/????????????????????????��??��?????????????????ini??version.json??

int main(int argc, char* argv[])
{
    std::string inputdir = argv[1];
    std::string outputdir = argv[2];
  
    std::string version = argv[3];

    outputdir += version + "/newpackage/bin/";
    AI3D::CORE::File::CreateDirIfNotExists(outputdir,true);
    //????????????????????????????
    //????????????��???
    std::vector<std::string> dirs;
    dirs.push_back("data");
    dirs.push_back("imageformats");
    dirs.push_back("osgPlugins-3.6.5");
    dirs.push_back("platforms");
    int count = 0;
    for (auto& dir : dirs)
    {

        std::string datadir = inputdir + "/" + dir;
        
        if (AI3D::CORE::File::ExistsPath(datadir) && AI3D::CORE::File::ExistsDir(datadir))
        {
            std::string dataoutdir = outputdir + "/"+dir;
            AI3D::CORE::File::CreateDirIfNotExists(dataoutdir, true);
            AI3D::CORE::File::CopyDirectory(datadir, dataoutdir, false);
        }
        count++;
    }
   
   
    std::vector<std::string > files;
    std::vector<std::string> extens;
    extens.push_back(".exe");
    extens.push_back(".dll");
    extens.push_back(".png");
    extens.push_back(".ini");
    extens.push_back(".bat");
    for (auto& exten : extens)
    {
        auto filevec = File::GetFileList(inputdir, exten);
        files.insert(files.end(), filevec.begin(), filevec.end());
    }
    
  
    //????????
    std::string camdatafile = inputdir + "/sensor_width_camera_database.txt";
    std::string verisonfile = inputdir + "/version.json";
    files.push_back(camdatafile);
    files.push_back(verisonfile);
    AI3D::CORE::File::CopyFiles(files, outputdir, false);
    //?????version.json??jconfig.json ??????���??
    version_copy_s versioncopy(version);
    verisonfile = outputdir + "/version.json";
    versioncopy.save(verisonfile);
    std::string jconfigfile = inputdir + "/JsonConfig.ini";
    AI3D::CORE::jconfigopt_s opt;
    auto config = opt.LoadXML(jconfigfile);
    opt.version = version;
    jconfigfile = outputdir + "/JsonConfig.ini";
    opt.SaveXML(jconfigfile);




    return 0;
}