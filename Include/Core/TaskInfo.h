#ifndef _AI3D_CORE_TASKINFO_H_
#define _AI3D_CORE_TASKINFO_H_
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <mutex>
#include <Eigen/Core>
#include <fstream>
#include <Constants.h>
#include <omp.h>

#include "Core/ATData.h"
#include  "Core/ATOptions.h"
#include "Core/ReturnCode.h"
#include "Core/Application.h"
#include "Core/ReconstructionOptions.h"
#include "Core/StringResource.h"
#include "Core/File.h"
namespace AI3D
{
    namespace CORE
    {
        enum mile_stone_e
        {
            MS_GENATTASK,
            MS_FEATUREDETECTION,
            MS_PAIRSELECTION,
            MS_MATCHPAIRS, MS_SFM,
            MS_RECONSTRUCTION, MS_BACTHPREPARE
        };
        static std::map<  mile_stone_e, std::string > MileStoneStringToshow =
        {
            {mile_stone_e::MS_GENATTASK,"starting AT" },
            {mile_stone_e::MS_FEATUREDETECTION,"keypoints extracting" },
            {mile_stone_e::MS_PAIRSELECTION, "pair selecting"},
            {mile_stone_e::MS_MATCHPAIRS,"pair matching"},
            {mile_stone_e::MS_SFM,"sfm"},

            {mile_stone_e::MS_RECONSTRUCTION,"starting  Reconstruction"},
            {mile_stone_e::MS_BACTHPREPARE,"starting  BatchPrePare"}
            
        };



        static std::map< mile_stone_e, std::string > milestone_function =
        {
            {mile_stone_e::MS_GENATTASK,"RunGenTasks" },
            {mile_stone_e::MS_FEATUREDETECTION,"RunFeatureDetection" },
            {mile_stone_e::MS_PAIRSELECTION,"RunPairSelection" },
            {mile_stone_e::MS_MATCHPAIRS,"RunMatchPairs"},
            {mile_stone_e::MS_SFM,"RunSfM"},
           
            {mile_stone_e::MS_RECONSTRUCTION,"RunReconstruction"},
            {mile_stone_e::MS_BACTHPREPARE,"RunBatchPrepare"}
        };

        struct AI3D_API  task_base_info_s
        {
            task_base_info_s() {};
            std::string item_path_ = "";
            std::string job_;
            std::string project_path_ = "";
            int sdebug_ = 1;
            task_base_info_s(std::string item_path,std::string job,std::string project_path)
            {
                item_path_ = item_path;
                job_ = job;
                project_path_ = project_path;
            }
            void CreateJson( rapidjson::Document& document)
            {
                rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
                if (item_path_ != "")
                {
                    document.AddMember("blockItem", rapidjson::Value(item_path_.c_str(), allocator), allocator);
                }
                if (job_ != "")
                {
                    document.AddMember("job", rapidjson::Value(job_.c_str(), allocator), allocator);
                }
                if (sdebug_ > 0)
                {
                    document.AddMember("sdebug", rapidjson::Value(sdebug_), allocator);
                }
                if (project_path_ != "")
                {
                    project_path_ = File::EnsureUnifySlash(project_path_);
                    
                    document.AddMember("projectPath", rapidjson::Value(project_path_.c_str(), allocator), allocator);
                }
            }
        };


        struct task_metadata_s
        {
            task_metadata_s() {};
            std::string msg_ = "";
            std::string functionname_ = "";
            std::string name_ = "";
            int id_;
            int type_;
            int keyMaxImgNum_ = 2000;
            int matchMaxImgNum_ = 8000;
            task_metadata_s(std::string msg, std::string functionname, std::string name, int type,int id)
            {
                msg_ = msg;
                functionname_ = functionname;
                name_ = name;
                type_ = type;
                id_ = id;
            };


            void CreateJson(rapidjson::Value& jstr, rapidjson::Document& document)
            {
                rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
                jstr.AddMember("msg", rapidjson::Value(msg_.c_str(), allocator), allocator);
                jstr.AddMember("function", rapidjson::Value(functionname_.c_str(), allocator), allocator);
                jstr.AddMember("name", rapidjson::Value(name_.c_str(), allocator), allocator);
                jstr.AddMember("type", rapidjson::Value(type_), allocator);
                jstr.AddMember("id", rapidjson::Value(id_), allocator);
                
                {
                    jstr.AddMember("keyMaxImgNum", rapidjson::Value(keyMaxImgNum_), allocator);
                }
                {
                    jstr.AddMember("matchMaxImgNum", rapidjson::Value(matchMaxImgNum_), allocator);
                }
            }

        };

        
        struct AI3D_API task_info_s
        {

            task_base_info_s base_info_;
            ATOptions at_options_;
            task_metadata_s task_metadata_;
            ProductionOptions production_options_;
            task_info_s() {};
            task_info_s(task_base_info_s base_info, task_metadata_s task_metadata)
            {
                base_info_ = base_info;
                task_metadata_ = task_metadata;
            }

            void LoadJson(const std::string& path)
            {
                return;
            }

            
            bool WriteToBin(std::string file)
            {
                std::ofstream out = File::OpenOfstreamUtf8(file, std::ios::binary);
                
                if (!out.is_open()) {
                    LOGE("Save taskdef bin failed!");
                    return false;
                }
                SPTaskInfoFile sPTaskInfoFile;                  
                sPTaskInfoFile.hasATParam = true;
                sPTaskInfoFile.hasRecParam = false;
                std::string blockItem = base_info_.item_path_;
                std::string projectfile = base_info_.project_path_;
                sPTaskInfoFile.jobName = base_info_.job_;
#ifdef WIN32
                // blockItem = GBK2UTF8(blockItem);
                // projectfile = GBK2UTF8(projectfile);
#endif 
                sPTaskInfoFile.blockItem = blockItem;
                sPTaskInfoFile.projectfile = projectfile;
                sPTaskInfoFile.hasAT = false;
                sPTaskInfoFile.hasGCP = false;
                sPTaskInfoFile.sdebug = base_info_.sdebug_;

                sPTaskInfoFile.taskMetaData.id = task_metadata_.id_;
                sPTaskInfoFile.taskMetaData.msg = task_metadata_.msg_;
                sPTaskInfoFile.taskMetaData.name = task_metadata_.name_;
                sPTaskInfoFile.taskMetaData.type = task_metadata_.type_;
                sPTaskInfoFile.taskMetaData.imgIds.clear();
                sPTaskInfoFile.taskMetaData.imagNum = 0;
                sPTaskInfoFile.taskMetaData.depends.clear();
                sPTaskInfoFile.taskMetaData.dependNum = 0; 
                sPTaskInfoFile.taskMetaData.functionName = task_metadata_.functionname_;
                sPTaskInfoFile.taskMetaData.keyMaxImgNum = task_metadata_.keyMaxImgNum_;
                sPTaskInfoFile.taskMetaData.matchMaxImgNum = task_metadata_.matchMaxImgNum_;

                
                sPTaskInfoFile.atSetting.keyNum = at_options_.feature_num;
                sPTaskInfoFile.atSetting.maxthreads_num = at_options_.maxthreads_num;
                sPTaskInfoFile.atSetting.minOverlap = at_options_.saveoptions.min_overlap;
                sPTaskInfoFile.atSetting.maxOverlap = at_options_.saveoptions.max_overlap;
                sPTaskInfoFile.atSetting.maxTieptNum = at_options_.saveoptions.max_tiepoint_num;
                sPTaskInfoFile.atSetting.mode = at_options_.sfmsettings.sfm_mode;
                sPTaskInfoFile.atSetting.ba1_grid_count = at_options_.sfmsettings.grid_count_1;
                sPTaskInfoFile.atSetting.ba2_grid_count = at_options_.sfmsettings.grid_count_2;
                sPTaskInfoFile.atSetting.max_feature_count_1 = at_options_.sfmsettings.max_feature_count_1;
                sPTaskInfoFile.atSetting.max_feature_count_2 = at_options_.sfmsettings.max_feature_count_2;
                sPTaskInfoFile.atSetting.output_tiepoint = at_options_.saveoptions.boutput_tiepoint;
                sPTaskInfoFile.atSetting.max_projection_error = at_options_.saveoptions.max_projection_error;
                sPTaskInfoFile.atSetting.reconstruct_mode = at_options_.reconstruct_mode;
                sPTaskInfoFile.atSetting.output_rawxml = at_options_.saveoptions.output_rawxml;
                sPTaskInfoFile.atSetting.use_user_tiepoints = at_options_.sfmsettings.bapolicies.use_user_tiepoints_;
                
                std::string tmpTilePointPath = at_options_.sfmsettings.bapolicies.usertiepoints_path_;
                if (sPTaskInfoFile.atSetting.use_user_tiepoints && tmpTilePointPath != "") {
                    sPTaskInfoFile.atSetting.use_user_tiepoints = true;
                    sPTaskInfoFile.atSetting.usertiepoints_path_ = tmpTilePointPath;
                }
                else {
                    sPTaskInfoFile.atSetting.use_user_tiepoints = false;
                }
                sPTaskInfoFile.atSetting.use_gcp = at_options_.sfmsettings.bapolicies.use_gcp_;
                std::string tmpGcpPath = at_options_.sfmsettings.bapolicies.gcp_path_;
                if (sPTaskInfoFile.atSetting.use_gcp && tmpGcpPath != "") {
                    sPTaskInfoFile.atSetting.use_gcp = true;
                    sPTaskInfoFile.atSetting.control_point_path = tmpGcpPath;
                }
                else {
                    sPTaskInfoFile.atSetting.use_gcp = false;
                }
                sPTaskInfoFile.atSetting.use_constraint = at_options_.sfmsettings.bapolicies.use_constraints_;
                std::string tmpConstraintPath = at_options_.sfmsettings.bapolicies.constraint_path_;
                if (sPTaskInfoFile.atSetting.use_constraint && tmpConstraintPath != "") {
                    sPTaskInfoFile.atSetting.use_constraint = true;
                    sPTaskInfoFile.atSetting.constraint_path = tmpConstraintPath;
                }
                else {
                    sPTaskInfoFile.atSetting.use_constraint = false;
                }
                sPTaskInfoFile.atSetting.tiepoints_policy = at_options_.sfmsettings.bapolicies.tiepoints_policy_;
                sPTaskInfoFile.atSetting.pos_policy = at_options_.sfmsettings.bapolicies.pos_policy_;
                sPTaskInfoFile.atSetting.ppa_policy = at_options_.sfmsettings.bapolicies.f_policy_;
                sPTaskInfoFile.atSetting.rdis_policy = at_options_.sfmsettings.bapolicies.ppa_policy_;
                sPTaskInfoFile.atSetting.f_policy = at_options_.sfmsettings.bapolicies.rdis_policy_;
                sPTaskInfoFile.atSetting.tdis_policy = at_options_.sfmsettings.bapolicies.tdis_policy_;
                sPTaskInfoFile.atSetting.use_image_position_ = at_options_.sfmsettings.bapolicies.use_image_position_;
                std::string tmp_image_pos_list = at_options_.sfmsettings.bapolicies.pos_path_;
                if (sPTaskInfoFile.atSetting.use_image_position_ && tmp_image_pos_list != "") {
                    sPTaskInfoFile.atSetting.use_image_position_ = true;
                    sPTaskInfoFile.atSetting.image_pos_list = tmp_image_pos_list;
                }
                else {
                    sPTaskInfoFile.atSetting.use_image_position_ = false;
                }
                sPTaskInfoFile.atSetting.hasATPath = false;
                std::string tmpATPath = at_options_.sfmsettings.bapolicies.at_path_;
                if (tmpATPath != "") {
                    sPTaskInfoFile.atSetting.hasATPath = true;
                    sPTaskInfoFile.atSetting.at_path = tmpATPath;
                }

                sPTaskInfoFile.Serialize(out);

                out.close();
               
                
                LOGI("***************************** Writing task_def_0.json ************* ");

                return true;
            }

            bool WriteToJson(std::string path)
            {
                auto config = Application::Getinstance().ParseConfig();
               
                rapidjson::Document document;
                document.SetObject();
                rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
                

                rapidjson::Value meta_data(rapidjson::kObjectType);
                task_metadata_.CreateJson(meta_data, document);


              
                rapidjson::Value settings(rapidjson::kObjectType);

                at_options_.WriteToJson(settings, document);
                base_info_.CreateJson(document);
                document.AddMember("meta_data", meta_data, allocator);
                document.AddMember("settings", settings, allocator);




                
                LOGI("***************************** Writing task_def_0.json ************* ");
              

                if (RapidJsonCore::SaveFile(path, document) == SAVE_FILE_FAILED)
                {
                    LOGE("Writing task 0 failed!");
                    return false;
                }
                return true;
            }
        };
    }
}
#endif