#include <string>
#include <filesystem>

#include <Core/BlockObject.h>
#include "Core/File.h"
#include <iostream>
using namespace AI3D::CORE;
/*??????????20240207 ???????
* ???????????????????????????????? ???????????????????????????????????????????????
* 
*/

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
//????????20240207????
int main(int argc, char* argv[])
{
    std::set< std::pair<std::string, std::string>> namepairs;
    std::string listfile =  argv[1];
    std::string datapath =  argv[2];//???????????????????
    std::string outpath =  argv[3];
    listfile = File::EnsureUnifySlash(listfile);
    std::cout << " begin file " << listfile << std::endl;
    RenameWithTab(listfile, namepairs);
    if (namepairs.empty())
    {
        RenameWithSpace(listfile, namepairs);
    }
    if (namepairs.empty())
    {
        std::cout << "sorry nothing in "<< listfile << std::endl;
        return 0;
    }
    int nTotalNum = namepairs.size();
    int count = 0;
    int nSuccessNum = 0,nFailedNum=0;
    
    
    //????????????????????????????
    std::string listoutfile = outpath + "/liststatus.txt";
    std::ofstream listout = File::OpenOfstreamUtf8(listoutfile, std::ios::out);
   
    for (auto iter : namepairs)
    {
        count++;
        bool hasxml = false;
        bool hasjpg = false;
        bool hasobj = false;
        bool hasmtl = false;
        std::string srcName = iter.second;
        std::string dstName = iter.first;
        listout << srcName << " " << dstName ;
        std::string srcPath = datapath + "/" + srcName ;
        std::string dstpath = outpath + "/" + dstName;

        //?????????????????
        if (!File::ExistsDir(srcPath))
        {
            std::string imgpossibleSrcPath1 = datapath + "/" + dstName;//????????????????????????
            if (!File::ExistsDir(imgpossibleSrcPath1))
            {
                listout << " Not Exists."<<std::endl;
                continue;
            }
            else
            {
                srcPath = imgpossibleSrcPath1;
                
            }
           
        }
        std::cout << count<<"---Running " << srcPath << "==========" ;
        std::vector<std::string> filelist ,xmlfile;
        std::string srcxml = srcPath+"/metadata.xml";
        std::string dstxml = dstpath + "/metadata.xml";
        File::CreateDirIfNotExists(dstpath,true);
        if (File::ExistsFile(dstxml))
        {
            hasxml = true;
        }
        else
        {
            bool ret = File::CopySingleFile(srcxml, dstxml);
            if (!ret)
            {
                std::cout << "copy " << srcxml << "failed.==========" << std::endl;
                hasxml = false;
                // continue;
            }
            else
            {
                hasxml = true;
            }
        }
        //??????????????
        
       auto filelistjpg =  File::GetRecursiveFileList(srcPath,".jpg");
       filelist.insert(filelist.end(), filelistjpg.begin(), filelistjpg.end());
       auto filelistmtl = File::GetRecursiveFileList(srcPath, ".mtl");
       filelist.insert(filelist.end(), filelistmtl.begin(), filelistmtl.end());
       auto filelistobj = File::GetRecursiveFileList(srcPath, ".obj");
       filelist.insert(filelist.end(), filelistobj.begin(), filelistobj.end());
       for (auto iter1 : filelist)
       {
           //??????????
           auto fname = File::GetFileNameWithoutExtension(iter1);
           auto extension = File::GetFileExtension(iter1);
           if (String::StringContains(extension, "jpg"))
           {
              auto newfile = String::StringReplace(iter1, srcName, dstName);
              newfile = String::StringReplace(newfile, datapath, outpath);
               std::vector<std::string> temp(1, iter1);
               File::CreateDirIfNotExists(File::GetParentDir(newfile), true);
               if (File::ExistsFile(newfile))
               {
                   hasjpg = true;
               }
               else
               {

                   bool ret = File::CopySingleFile(iter1, newfile);
                   if (!ret)
                   {
                       std::cout << "==========---copy " << iter1 << "failed.---==========" << std::endl;
                       hasjpg = false;
                      
                   }
                   else
                   {
                       hasjpg = true;
                   }
               }
               continue;
           }

           std::ifstream in = File::OpenIfstreamUtf8(iter1, std::ios::in);
           std::string outfile;
           outfile = String::StringReplace(iter1, srcName, dstName);
           outfile = String::StringReplace(outfile, datapath, outpath);
           
           File::CreateDirIfNotExists(File::GetParentDir(outfile),true);
           if (File::ExistsFile(outfile))
           {
               
              
           }
           else
           {


               std::ofstream out = File::OpenOfstreamUtf8(outfile, std::ios::out);
               if (!in.is_open())
               {
                   std::cout << "Error opening file " << iter1 << std::endl;
                   continue;
               }
               if (!out.is_open())
               {
                   std::cout << "Error opening file " << outfile << std::endl;;
                   continue;
               }

               while (!in.eof())
               {
                   char buffer[256];
                   in.getline(buffer, 100);
                   //cout << buffer << endl;
                   std::string str = buffer;

                   if (String::StringContains(str, srcName))
                   {
                       std::string newstr;
                       newstr = String::StringReplace(str, srcName, dstName);

                       out << newstr << std::endl;
                   }
                   else
                   {
                       out << buffer << std::endl;
                   }

               }
               in.close();
               out.close();
           }
           std::string ext = File::GetFileExtension(outfile);
           if (String::StringContains(ext, "mtl"))
           {
               hasmtl = true;
           }
           else if (String::StringContains(ext, "obj"))
           {
               hasobj = true;
           }
       }
       std::string msg = " ";
       if (!hasxml)
       {
           msg += "metadata.xml ";
       }
       if (hasjpg && hasmtl && hasobj)
       {
           msg += "success.";
           nSuccessNum++;
       }
       else
       {
           nFailedNum++;
           if (!hasjpg)
           {
               msg += "jpg ";
           }
           if (!hasmtl)
           {
               msg += "mtl ";
           }
           if (!hasobj)
           {
               msg += "obj ";
           }
       }

       listout << msg << std::endl;
       //std::cout << "==========---Running " << srcPath << " End---==========" << std::endl;
       std::cout <<  " Success---==========" << std::endl;
    }
    listout << "Total " << nTotalNum << std::endl; 
    listout << "Success " << nSuccessNum << std::endl;
    listout << "Failed " << nFailedNum << std::endl; 
    listout.close();
    std::cout << "Total " << nTotalNum << " Success "<< nSuccessNum << " Failed "<< nFailedNum << std::endl;
    return 0;
}

