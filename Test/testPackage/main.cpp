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

       
        document.AddMember("vi", rapidjson::Value(versionCode), allocator);
        document.AddMember("vs", rapidjson::Value(versionName.c_str(), allocator), allocator);
        document.AddMember("info", rapidjson::Value(description.c_str(), allocator), allocator);
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

        bool ret = RapidJsonCore::ReadFile(file_path, blkcontent);
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
        if (doc_blk.HasMember("vi"))
        {
            versionCode = (doc_blk["vi"].GetInt());
        }

        if (doc_blk.HasMember("vs"))
        {
            versionName = (doc_blk["vs"].GetString());
        }

        if (doc_blk.HasMember("info"))
        {
            description = (doc_blk["info"].GetString());
        }
        
    }
    //Eigen::Vector3u untexturecolor_;


};

//该文件用于每次打包给测试，准则：版本号 / newpackage / bin/**.exe等
//使用方法：参数1：D:\jiaojie\mok\0.00.017\build\Mohacker\Bin\x64\Release为exe所在的目录，
// 把此目录下的文件夹、dll、exe、png、ini、sensor_width_camera_database.txt、version.json全部复制一份，
//参数2：输出文件夹，输出文件夹不存在的话就创建该文件夹，
//并在该文件夹下创建: 版本号/newpackage/bin/目录，并将上述拷贝至该文件夹；版本号手动输入并同步更新到ini和version.json里

int main(int argc, char* argv[])
{
    std::string inputdir = argv[1];
    std::string outputdir = argv[2];
  
    std::string version = argv[3];

    outputdir += version + "/newpackage/bin/";
    AI3D::CORE::File::CreateDirIfNotExists(outputdir,true);
    //获取输入目录下的所有需要拷贝的文件
    //拷贝整个文件夹的有
    std::vector<std::string> dirs;
    dirs.push_back("data");
    dirs.push_back("imageformats");
    dirs.push_back("osgPlugins-3.6.5");
    dirs.push_back("platforms");
    dirs.push_back("au");
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
    
  
    //特定的文件
    std::string camdatafile = inputdir + "/CD.txt";
    std::string verisonfile = inputdir + "/version.json";
    files.push_back(camdatafile);
    files.push_back(verisonfile);
    AI3D::CORE::File::CopyFiles(files, outputdir, false);
    //读一下version.json和jconfig.json 更改一下版本号
    version_copy_s versioncopy(version);
    verisonfile = outputdir + "/version.json";
    versioncopy.save(verisonfile);
    std::string jconfigfile = inputdir + "/MoldAIConfig.ini";
    AI3D::CORE::jconfigopt_s opt;
    auto config = opt.LoadXML(jconfigfile);
    opt.version = version;
    jconfigfile = outputdir + "/MoldAIConfig.ini";
    opt.SaveXML(jconfigfile);




    return 0;
}