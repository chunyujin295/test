#ifndef _AI3D_CORE_ATOPTIONS_H_
#define _AI3D_CORE_ATOPTIONS_H_
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
#include "TaskDef.h"
namespace AI3D
{
    namespace CORE
    {
        enum AT_complete_status_e
        {
            INCOMPLETE_PHOTOS,
            PARTIALLY_COMPLETE_PHOTOS,
            COMPLETE_PHOSTOS,
        };
    
        struct AT_saveoption_s
        {
            bool boutput_tiepoint = true;
            float max_projection_error = 3.0f;
            int min_overlap = 3;
            int max_overlap = 4;
            int max_tiepoint_num = 10000000;
            bool output_rawxml = true;
        };

        enum policies_e
        {
            POLICIES_COMPUTE = 1,
            POLICIES_ADJUST = 2,
            POLICIES_KEEP = 4,
            POLICIES_COMPUTE_ADJUST = POLICIES_COMPUTE | POLICIES_ADJUST,
            POLICIES_COMPUTE_ADJUST_KEEP = POLICIES_COMPUTE | POLICIES_ADJUST | POLICIES_KEEP,
            POLICIES_COMPUTE_KEEP = POLICIES_COMPUTE | POLICIES_KEEP,
            POLICIES_ADJUST_KEEP = POLICIES_ADJUST | POLICIES_KEEP,
        };
        enum policies_object_e
        {
            PO_OBJ_TIEPOINTS,
            PO_OBJ_POSE,
            PO_OBJ_POSITION,
            PO_OBJ_ROTATION,
            PO_OBJ_F,
            PO_OBJ_PPA,
            PO_OBJ_RDIS,
            PO_OBJ_TDIS,
        };

        static std::map< std::string, policies_e> String_Policy_Chinese =
        {
            {"计算" ,POLICIES_COMPUTE},
            {"调整",POLICIES_ADJUST },
            {"保持",POLICIES_KEEP },
        };

        static std::map< std::string,policies_e> String_Policy =
        {                
            {"Compute" ,POLICIES_COMPUTE},
            {"Adjust",POLICIES_ADJUST },
            {"Keep",POLICIES_KEEP },                
        };

        static std::map< policies_e, std::string > Policy_String =
        {
            {POLICIES_COMPUTE,"Compute" },
            {POLICIES_ADJUST,"Adjust" },
            {POLICIES_KEEP,"Keep" },
           
        };
        static std::map< policies_e, std::string > Policy_String_Chinese =
        {
            {POLICIES_COMPUTE,"计算" },
            {POLICIES_ADJUST,"调整" },
            {POLICIES_KEEP,"保持" },

        };
        struct BA_estimation_polices_s
        {
            bool use_user_tiepoints_ =false;
            std::string usertiepoints_path_ = "";
            
            std::string pos_path_="";
            bool use_gcp_=false;
            std::string gcp_path_="";
            bool use_constraints_ = false;
            std::string constraint_path_ = "";
            bool use_image_position_ = false;
            std::string at_path_="";

            policies_e tiepoints_policy_;
            policies_e pos_policy_;
            policies_e f_policy_;
            policies_e ppa_policy_;
            policies_e rdis_policy_;
            policies_e tdis_policy_;

            void ParseJson(rapidjson::Value& jstr)
            {
                
                if (jstr.HasMember("tiepoints_policy"))
                {
                    tiepoints_policy_ = (policies_e)jstr["tiepoints_policy"].GetInt();
                }
                if (jstr.HasMember("pos_policy"))
                {
                    pos_policy_ = (policies_e)jstr["pos_policy"].GetInt();
                }
                if (jstr.HasMember("f_policy"))
                {
                    f_policy_ = (policies_e)jstr["f_policy"].GetInt();
                   
                }
                if (jstr.HasMember("ppa_policy"))
                {
                    ppa_policy_ = (policies_e)jstr["ppa_policy"].GetInt();
                   
                }
                if (jstr.HasMember("rdis_policy"))
                {
                    rdis_policy_ = (policies_e)jstr["rdis_policy"].GetInt();
                }
                if (jstr.HasMember("tdis_policy"))
                {
                    tdis_policy_ = (policies_e)jstr["tdis_policy"].GetInt();
                }
            };

            void CreateJson(rapidjson::Value& jstr, rapidjson::Document& document)
            {
                rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
               
                jstr.AddMember("use_user_tiepoints", rapidjson::Value(use_user_tiepoints_), allocator);
                if (use_user_tiepoints_ && usertiepoints_path_!="")
                {
                    jstr.AddMember("manual_ties_path", rapidjson::Value(usertiepoints_path_.c_str(), allocator), allocator); // was: GBK2UTF8
                }
                jstr.AddMember("use_gcp", rapidjson::Value(use_gcp_), allocator);
                if (use_gcp_&& gcp_path_!="")
                {
                    jstr.AddMember("control_point_path", rapidjson::Value(gcp_path_.c_str(), allocator), allocator); // was: GBK2UTF8
                }
                jstr.AddMember("tiepoints_policy", rapidjson::Value((int)tiepoints_policy_), allocator);

               
                {
                    jstr.AddMember("pos_policy", rapidjson::Value((int)pos_policy_), allocator);
                    jstr.AddMember("f_policy", rapidjson::Value((int)f_policy_), allocator);
                }
                jstr.AddMember("ppa_policy", rapidjson::Value((int)ppa_policy_), allocator);
                jstr.AddMember("rdis_policy", rapidjson::Value((int)rdis_policy_), allocator);
                jstr.AddMember("tdis_policy", rapidjson::Value((int)tdis_policy_), allocator);
                
               

                if (use_image_position_ && pos_path_ != "")
                {
                    jstr.AddMember("use_image_position", rapidjson::Value(use_image_position_), allocator);
                    jstr.AddMember("image_pos_list", rapidjson::Value(pos_path_.c_str(), allocator), allocator); // was: GBK2UTF8
                }
                if (at_path_ != "")
                {
                    jstr.AddMember("at_path", rapidjson::Value(at_path_.c_str(), allocator), allocator); // was: GBK2UTF8
                }
            }

        };
        struct sfmsettings_s
        {
            BA_estimation_polices_s bapolicies;
            int grid_count_1 = 20;
            int grid_count_2 = 30;
            int max_feature_count_1 = 400;
            int max_feature_count_2 = 1000;

            sfm_mode_e sfm_mode = SFM_GLOBAL;
            Eigen::Vector3d pos_sigma{ 10,10,10 };

           


        };
        struct AI3D_API ATOptions
        {
            int blockat = -1;
            point2D_t feature_num = 20000;

            pair_selection_mode_e reconstruct_mode = PAIR_NORMAL;

            sfm_align_mode_e align_mode = alignmode_e::ALIGN_ARBITRARY;
            sfmsettings_s sfmsettings;
            std::string at_name = "Block_AT";
            int maxthreads_num = 0;
            
            AT_saveoption_s saveoptions;

            bool ParseJson(rapidjson::Value& jstr)
            {

                if (jstr.HasMember("align_mode"))
                {
                    align_mode = (sfm_align_mode_e)jstr["align_mode"].GetInt();
                }
                if (jstr.HasMember("pairselection_mode"))
                {
                    reconstruct_mode = (pair_selection_mode_e)jstr["pairselection_mode"].GetInt();
                }
                if (!jstr.HasMember("feature") || !jstr.HasMember("sfm"))
                {
                    LOGE("Block file error!");
                    return false;
                }
                else
                {
                    if (jstr["feature"].HasMember("keyNum"))
                    {
                        feature_num = jstr["feature"]["keyNum"].GetInt();
                    }
                    if (jstr["sfm"].HasMember("maxOverlap") && jstr["sfm"].HasMember("minOverlap"))
                    {
                        saveoptions.min_overlap = jstr["sfm"]["minOverlap"].GetInt();
                        saveoptions.max_overlap = jstr["sfm"]["maxOverlap"].GetInt();
                    }
                    if (jstr["sfm"].HasMember("ba1_grid_count") && jstr["sfm"].HasMember("ba2_grid_count"))
                    {
                        sfmsettings.grid_count_1 = jstr["sfm"]["ba1_grid_count"].GetInt();
                        sfmsettings.grid_count_2 = jstr["sfm"]["ba2_grid_count"].GetInt();
                    }
                    if (jstr["sfm"].HasMember("maxTieptNum"))
                    {
                        saveoptions.max_tiepoint_num = jstr["sfm"]["maxTieptNum"].GetInt();
                    }
                    if (jstr["sfm"].HasMember("max_feature_count_1") && jstr["sfm"].HasMember("max_feature_count_2"))
                    {
                        sfmsettings.max_feature_count_1 = jstr["sfm"]["max_feature_count_1"].GetInt();
                        sfmsettings.max_feature_count_2 = jstr["sfm"]["max_feature_count_2"].GetInt();
                    }
                    if (jstr["sfm"].HasMember("sfm_mode"))
                    {
                        sfmsettings.sfm_mode = sfm_mode_e(jstr["sfm"]["sfm_mode"].GetInt());
                    }
                    if (jstr["sfm"].HasMember("AT_definition"))
                    {
                        sfmsettings.bapolicies.ParseJson(jstr["sfm"]["AT_definition"]);
                    }
                }
                return true;
            };

            void WriteToJson(rapidjson::Value & metadata, rapidjson::Document & document)
                {

                    document.SetObject();
                    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
                    rapidjson::Value feature(rapidjson::kObjectType);
                    feature.AddMember("keyNum", rapidjson::Value(feature_num), allocator);
                    metadata.AddMember("feature", feature, allocator);
                    if (maxthreads_num > 0)
                    {
                        metadata.AddMember("maxthreads_num", rapidjson::Value(maxthreads_num), allocator);
                    }
                    rapidjson::Value sfmsettingsjson(rapidjson::kObjectType);
                    sfmsettingsjson.AddMember("minOverlap", rapidjson::Value(saveoptions.min_overlap), allocator);
                    sfmsettingsjson.AddMember("maxOverlap", rapidjson::Value(saveoptions.max_overlap), allocator);
                    sfmsettingsjson.AddMember("maxTieptNum", rapidjson::Value(saveoptions.max_tiepoint_num), allocator);
                    if (sfmsettings.sfm_mode > 0)
                    {
                        sfmsettingsjson.AddMember("mode", rapidjson::Value(sfmsettings.sfm_mode), allocator);
                    }
                    sfmsettingsjson.AddMember("ba1_grid_count", rapidjson::Value(sfmsettings.grid_count_1), allocator);
                    sfmsettingsjson.AddMember("ba2_grid_count", rapidjson::Value(sfmsettings.grid_count_2), allocator);

                    sfmsettingsjson.AddMember("max_feature_count_1", rapidjson::Value(sfmsettings.max_feature_count_1), allocator);
                    sfmsettingsjson.AddMember("max_feature_count_2", rapidjson::Value(sfmsettings.max_feature_count_2), allocator);
                    sfmsettingsjson.AddMember("output_tiepoint", rapidjson::Value(saveoptions.boutput_tiepoint), allocator);
                    sfmsettingsjson.AddMember("max_projection_error", rapidjson::Value(saveoptions.max_projection_error), allocator);
                    sfmsettingsjson.AddMember("reconstruct_mode", rapidjson::Value(reconstruct_mode), allocator);
                    sfmsettingsjson.AddMember("output_rawxml", rapidjson::Value(saveoptions.output_rawxml), allocator);
                    rapidjson::Value policiesjson(rapidjson::kObjectType);
                    sfmsettings.bapolicies.CreateJson(policiesjson, document);
                    sfmsettingsjson.AddMember("AT_definition", policiesjson, allocator);
                    
                    metadata.AddMember("sfm", sfmsettingsjson, allocator);
                }

            };

        
    }
} 



#endif  

