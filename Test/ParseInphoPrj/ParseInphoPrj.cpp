#include <sstream>
#include <iostream>
#include <fstream>
#include <set>
#include <Core/ProjectObject.h>
#include "Core/File.h"
//需要测试的几项
// 测试排序。测试删除后id
 


#include <stdio.h>

using namespace AI3D::CORE;


void ParseProjectInfo(std::ifstream& ifs, std::shared_ptr<ProjectObject>& project)
{

    AI3D::CORE::BlockObject* block = new AI3D::CORE::BlockObject("");

    block->SetStatus(jobsta_e::STATUS_NEW);

    project->AddBlock(block);
    std::string line, item;
    while (std::getline(ifs, line))
    {
        auto strvec = String::StringSplit(line, ":");
        if (strvec.empty())
            continue;
        std::string firststr = strvec[0].substr(0, 1);
        if (firststr == "$")//代表有效变量
        {
            String::StringTrim(firststr, "$");//需增加一个接口就是左侧去除指定的符号
            if (strvec.size() > 1)
            {
                if (firststr == "PROJECT_NAME")
                {
                    project->GetNameMutual() = strvec[1];
                }
            }
        }
        else if (firststr == "END")
        {
            return;
        }
    }
}

void ParseAATInfo(std::ifstream& ifs, std::shared_ptr<ProjectObject>& project)
{
    if (!project->GetBlock(0))
    {
        return;
    }
    AI3D::CORE::BlockObject* block = project->GetBlock(0);
    std::shared_ptr<ATData> ATdata = std::make_shared<ATData>();
    PhotoGroup pg;
    std::string line, item;
    while (std::getline(ifs, line))
    {
        auto strvec = String::StringSplit(line, ":");
        if (strvec.empty())
            continue;
        std::string firststr = strvec[0].substr(0, 1);
        if (firststr == "$")//代表有效变量
        {
            String::StringTrim(firststr, "$");//需增加一个接口就是左侧去除指定的符号
            if (strvec.size() > 1)
            {
                if (firststr == "STRIPS")
                {
                    while (std::getline(ifs, line))
                    {
                        if (line == "$END_STRIPS")
                        {
                            break;
                        }
                        else
                        {
                            std::string::size_type pos = line.find("ElementPhoto");
                            if (pos != std::string::npos)
                            {
                                std::vector<std::string> stripstrs;
                                std::string longstr = line;
                                stripstrs.push_back(line);
                                while (std::getline(ifs, line))
                                {
                                    std::string::size_type pos = line.find("}");
                                    if (pos != std::string::npos)
                                    {
                                        stripstrs.push_back(line);
                                        longstr +=" "+ line;
                                        break;
                                    }
                                    else
                                    {
                                        longstr += " " + line;
                                        stripstrs.push_back(line);
                                    }
                                }
                                if (!stripstrs.empty())
                                {
                                   /* std::stringstream line_stream(line);
                                    std::getline(line_stream, item, ' ');*/
                                    auto strvec = String::StringSplit(line, " ");
                                    int i = 0;
                                    for (;i<strvec.size();i++)
                                    {
                                        if (strvec[i] == "{")
                                        {
                                            break;
                                        }
                                    }
                                    pg.SetName("PhotoGroup0");
                                    pg.SetId(0);
                                    EIGEN_STL_UMAP(image_t, Image) image_map;
                                    std::vector<Image> imagevecs;
                                    int imageid = 0;
                                    for (; i < strvec.size()-1; i++)
                                    {
                                        Image image;
                                        image.SetPhotoGroupID(0);
                                        image.SetCameraId(0);
                                        image.SetName(strvec[i]);
                                        image.SetImageId(imageid);
                                        //image.SetRegistered(1);
                                        imageid++;
                                    }
                                }
                            }
                           
                        }
                    }
                   
                }
            }
        }
        else if (firststr == "END")
        {
            return;
        }
    }
}

int main(int argc, char** argv)
{
    std::string file = "D:/MyLearning/CHY/TEST/Proj/testat/0108003-N.prj";
    std::ifstream ifs = File::OpenIfstreamUtf8(file, std::ios::in);
    if (!ifs.is_open())
        return 0;
    std::string line,item;

    
    while (std::getline(ifs, line))
    {
        /*std::getline(ifs, line);*/

        std::stringstream line_str(line);
        std::cout << line << std::endl;
        auto strvec = String::StringSplit(line, " ");

        std::string firststr = strvec[0].substr(0, 1);
        if (firststr == "$")//代表有效变量
        {
            //if (strvec.size() > 1)//有效信息字段
            {
                //去除该字符
                String::StringTrim(firststr, "$");//需增加一个接口就是左侧去除指定的符号
                std::shared_ptr<ProjectObject> project = std::make_shared<ProjectObject>();
                if (firststr == "PROJECT")
                {
                    ParseProjectInfo(ifs, project);
                }
                else if (firststr == "AAT")
                {
                    ParseAATInfo(ifs, project);
                }
                else if (firststr == "PHOTO")//有很多张影像可根据影像的数量来决定
                {
                    ParsePhotoInfo(ifs, project);
                }
                else if (firststr == "CAMERA_DEFINITION")
                {
                    ParseCameraInfo(ifs, project);
                }
                else if (firststr == "DTM")//有多个DTM
                {
                    ParseDTMInfo(ifs, project);
                }
                else if (firststr == "DTMPAR")//有多个DTM
                {
                    ParseDTMPARInfo(ifs, project);
                }
                else if (firststr == "CONTROL_POINTS")//有多个DTM
                {
                    ParseGCPsInfo(ifs, project);
                }
                else if (firststr == "NAVIGATION")//有多个DTM
                {
                    ParseNAVIInfo(ifs, project);
                }
            }
            ifs.close();
        }

    }
//特点#为注释行
    //$为变量行，:为分隔符
  
       
    
    return 0;
}