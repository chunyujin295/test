#ifndef _AI3D_CORE_APPLICATION_H_
#define _AI3D_CORE_APPLICATION_H_

#include <vector>

#include <Eigen/Core>

#include "Constants.h"
#include "Core/ATData.h"
#include "Core/BlockObject.h"
#include "Core/ATOptions.h"

namespace AI3D
{
	namespace CORE
	{
		struct appconfig_s
		{
			ATOptions at_options;
			std::string version = "";
			double focal_length = 42.0;
			int debug_level;
			bool to_gs = false;
			int matchMaxImgNum = 2000;
			int keyMaxImgNum = 8000;
			int matchTaskNum = 1;
			int tile_point_threshold = 0;

		};
		
		struct jconfigopt_s
		{
			ATOptions atopt;
			int debug_level = 1;
			int reconstruct_mode = atopt.reconstruct_mode;
			int ba1_grid_count = atopt.sfmsettings.grid_count_1;
			int ba2_grid_count = atopt.sfmsettings.grid_count_2;
			int max_feature_count_1 = atopt.sfmsettings.max_feature_count_1;
			int max_feature_count_2 = atopt.sfmsettings.max_feature_count_2;
			int sfm_mode = atopt.sfmsettings.sfm_mode;
			int min_overlap = atopt.saveoptions.min_overlap;
			int max_overlap = atopt.saveoptions.max_overlap;
			int max_tiepoint_num = atopt.saveoptions.max_tiepoint_num;
			std::string version;
			int releaselevel = 0;
			int language = 0; 
			int point_threadHold = 0;
			int distribution = 0;
			int blockat = -1;
			bool to_gs = false;
			int use_gs = 0;
			int gs_iteration = 30000;
			int gs_resolution = 0;
			int use_tdom_dsm = 0;
			float bb_cut_ovelap = -1.;
			

			double focal_length = 42;
			bool boutput_tiepoint = atopt.saveoptions.boutput_tiepoint;
			float max_projection_error = atopt.saveoptions.max_projection_error;

			
			bool output_rawxml = atopt.saveoptions.output_rawxml;
			int keyMaxImgNum = 2000;
			int matchMaxImgNum = 8000;
			int matchTaskNum = -1;
			int maxthreads_num = 0;

			bool LoadXML(std::string xml_file_path)
			{
				pugi::xml_document doc;
				if (doc.load_file(xml_file_path.c_str()).status != pugi::xml_parse_status::status_ok)
				{
					LOGE("Load XML file error!");
					return false;
				}
				pugi::xml_node config = doc.child("Conf");
				if (!config)
				{
					LOGE("No Config!");
					return false;
				}
				if (config.child("UG"))
				{
					pugi::xml_node useGauss_node = config.child("UG");
					use_gs = useGauss_node.text().as_int();
				}
				if (config.child("GI"))
				{
					pugi::xml_node gauss_iteration_node = config.child("GI");
					gs_iteration = gauss_iteration_node.text().as_int();
				}
				if (config.child("GR"))
				{
					pugi::xml_node gauss_resolution_node = config.child("GR");
					gs_resolution = gauss_resolution_node.text().as_int();
				}
				if (config.child("UT"))
				{
					pugi::xml_node useTDOM_DSM_node = config.child("UT");
					use_tdom_dsm = useTDOM_DSM_node.text().as_int();
				}
				if (config.child("KN"))
				{
					pugi::xml_node keyMaxImgNum_node = config.child("KN");
					keyMaxImgNum = keyMaxImgNum_node.text().as_int();
				}
				if (config.child("MN"))
				{
					pugi::xml_node matchMaxImgNum_node = config.child("MN");
					matchMaxImgNum = matchMaxImgNum_node.text().as_int();
				}
				if (config.child("RL"))
				{
					pugi::xml_node level_node = config.child("RL");
					releaselevel = level_node.text().as_int();
				}
				if (config.child("UD"))
				{
					pugi::xml_node distribution_node = config.child("UD");
					distribution = distribution_node.text().as_int();
				}
				
				if (config.child("LI"))
				{
					pugi::xml_node language_node = config.child("LI");
					language = language_node.text().as_int();
				}
				if (config.child("PNT"))
				{
					pugi::xml_node pThread_node = config.child("PNT");
					point_threadHold = pThread_node.text().as_int();
				}
				if (config.child("TG"))
				{
					pugi::xml_node language_node = config.child("TG");
					to_gs = language_node.text().as_bool();
				}
				if (config.child("BN"))
				{
					pugi::xml_node at_node = config.child("BN");
					blockat = at_node.text().as_int();
				}

				if (config.child("BO"))
				{
					pugi::xml_node cut_node = config.child("BO");
					bb_cut_ovelap = cut_node.text().as_float();
				}
				
				pugi::xml_node matchTaskNum_node = config.child("MTN");
				matchTaskNum = matchTaskNum_node.text().as_int();

				

				pugi::xml_node debug_level_node = config.child("DL");
				debug_level = debug_level_node.text().as_int();

				
				pugi::xml_node  ba1_grid_count_node = config.child("BAC");
				ba1_grid_count = ba1_grid_count_node.text().as_int();
				pugi::xml_node  ba2_grid_count_node = config.child("BAR");
				ba2_grid_count = ba2_grid_count_node.text().as_int();
				pugi::xml_node  max_feature_count_1_node = config.child("BACN");
				max_feature_count_1 = max_feature_count_1_node.text().as_int();
				pugi::xml_node  max_feature_count_2_node = config.child("BARN");
				max_feature_count_2 = max_feature_count_2_node.text().as_int();

				pugi::xml_node version_node = config.child("VS");
				version = version_node.text().as_string();

				pugi::xml_node focal_length_node = config.child("FL");
				focal_length = focal_length_node.text().as_float();
				pugi::xml_node  max_tiepoint_num_node = config.child("MTN");
				max_tiepoint_num = max_tiepoint_num_node.text().as_int();
				pugi::xml_node  max_overlap_node = config.child("MAO");
				max_overlap = max_overlap_node.text().as_int();
				pugi::xml_node  min_overlap_node = config.child("MIO");
				min_overlap = min_overlap_node.text().as_int();
				pugi::xml_node  sfm_mode_node = config.child("SM");
				sfm_mode = sfm_mode_node.text().as_int();

				pugi::xml_node  boutput_tiepoint_node = config.child("BOT");
				boutput_tiepoint = boutput_tiepoint_node.text().as_bool();

				pugi::xml_node max_projection_error_node = config.child("MPE");
				max_projection_error = max_projection_error_node.text().as_float();


				

				pugi::xml_node  output_rawxml_node = config.child("OPX");
				output_rawxml = output_rawxml_node.text().as_bool();
				return true;
			}

			void SaveXML(std::string xml_file_path)
			{
				pugi::xml_document doc;
				pugi::xml_node declaration_node = doc.append_child(pugi::node_declaration);

				declaration_node.append_attribute("version") = "1.0";
				declaration_node.append_attribute("encoding") = "utf-8";

				pugi::xml_node config = doc.append_child("Conf");
				

				pugi::xml_node debug_level_node = config.append_child("DL");
				debug_level_node.append_child(pugi::node_pcdata).set_value(std::to_string(debug_level).c_str());
				

				pugi::xml_node useGauss_node = config.append_child("UG");
				useGauss_node.append_child(pugi::node_pcdata).set_value(std::to_string(use_gs).c_str());
				

				pugi::xml_node gauss_iter_node = config.append_child("GI");
				gauss_iter_node.append_child(pugi::node_pcdata).set_value(std::to_string(gs_iteration).c_str());
				

				pugi::xml_node gauss_resolution_node = config.append_child("GR");
				gauss_resolution_node.append_child(pugi::node_pcdata).set_value(std::to_string(gs_resolution).c_str());
				

				pugi::xml_node useTDOM_DSM_node = config.append_child("UT");
				useTDOM_DSM_node.append_child(pugi::node_pcdata).set_value(std::to_string(use_tdom_dsm).c_str());
				

				
				if (releaselevel >= 0)
				{
					pugi::xml_node rl_node = config.append_child("RL");
					rl_node.append_child(pugi::node_pcdata).set_value(std::to_string(releaselevel).c_str());
					
				}

				if (distribution >= 0)
				{
					pugi::xml_node dis_node = config.append_child("UD");
					dis_node.append_child(pugi::node_pcdata).set_value(std::to_string(distribution).c_str());
					
				}

				if (language >= 0)
				{
					pugi::xml_node language_node = config.append_child("LI");
					language_node.append_child(pugi::node_pcdata).set_value(std::to_string(language).c_str());
					
				}

				if (point_threadHold >= 0)
				{
					pugi::xml_node pThread_node = config.append_child("PNT");
					pThread_node.append_child(pugi::node_pcdata).set_value(std::to_string(point_threadHold).c_str());
				}

				
				{
					pugi::xml_node matchmaximgnum_node = config.append_child("MN");
					matchmaximgnum_node.append_child(pugi::node_pcdata).set_value(std::to_string(matchMaxImgNum).c_str());
					
					
				}
				
				{
					pugi::xml_node keymaximgnum_node = config.append_child("KN");
					keymaximgnum_node.append_child(pugi::node_pcdata).set_value(std::to_string(keyMaxImgNum).c_str());
					

				}
				pugi::xml_node ba1_grid_count_node = config.append_child("BAC");
				ba1_grid_count_node.append_child(pugi::node_pcdata).set_value(std::to_string(ba1_grid_count).c_str());
				

				pugi::xml_node ba2_grid_count_node = config.append_child("BAR");
				ba2_grid_count_node.append_child(pugi::node_pcdata).set_value(std::to_string(ba2_grid_count).c_str());
				

				pugi::xml_node max_feature_count_1_node = config.append_child("BACN");
				max_feature_count_1_node.append_child(pugi::node_pcdata).set_value(std::to_string(max_feature_count_1).c_str());
				

				pugi::xml_node max_feature_count_2_node = config.append_child("BARN");
				max_feature_count_2_node.append_child(pugi::node_pcdata).set_value(std::to_string(max_feature_count_2).c_str());
				

				pugi::xml_node sfm_mode_node = config.append_child("SM");
				sfm_mode_node.append_child(pugi::node_pcdata).set_value(std::to_string(sfm_mode).c_str());
				

				pugi::xml_node version_node = config.append_child("VS");
				version_node.append_child(pugi::node_pcdata).set_value(version.c_str());
				

				pugi::xml_node overlap_node = config.append_child("BO");
				overlap_node.append_child(pugi::node_pcdata).set_value(std::to_string(bb_cut_ovelap).c_str());
				

				pugi::xml_node blockat_node = config.append_child("BN");
				blockat_node.append_child(pugi::node_pcdata).set_value(std::to_string(blockat).c_str());
				

				pugi::xml_node focal_length_node = config.append_child("FL");
				focal_length_node.append_child(pugi::node_pcdata).set_value(std::to_string(focal_length).c_str());
				

				pugi::xml_node max_tiepoint_num_node = config.append_child("MTN");
				max_tiepoint_num_node.append_child(pugi::node_pcdata).set_value(std::to_string(max_tiepoint_num).c_str());
				

				pugi::xml_node max_overlap_node = config.append_child("MAO");
				max_overlap_node.append_child(pugi::node_pcdata).set_value(std::to_string(max_overlap).c_str());
				

				pugi::xml_node min_overlap_node = config.append_child("MIO");
				min_overlap_node.append_child(pugi::node_pcdata).set_value(std::to_string(min_overlap).c_str());
				

				pugi::xml_node boutput_tiepoint_node = config.append_child("BOT");
				boutput_tiepoint_node.append_child(pugi::node_pcdata).set_value(boutput_tiepoint ? "true" : "false");
				

				pugi::xml_node max_projection_error_node = config.append_child("MPE");
				max_projection_error_node.append_child(pugi::node_pcdata).set_value(std::to_string(max_projection_error).c_str());
				



				

				pugi::xml_node output_rawxml_node = config.append_child("OPX");
				output_rawxml_node.append_child(pugi::node_pcdata).set_value(std::to_string(output_rawxml).c_str());
				

				bool saveSucceed = doc.save_file(xml_file_path.c_str());
				if (!saveSucceed)
				{
					LOG(ERROR) << "Save Config failed!";
				}
			}
		};
		class AI3D_API Application
		{
		public:
			
			static Application& Getinstance()
			{
				static Application app;
				return app;
			}
			int GetReleaseLevel();
			int GetDistribution();
			bool isGDGSEnable();
			bool isBaseGSEnable();
			bool isBaseGSPluginExist();
			int getGSIteration();
			int getGSResolution();
			bool isTDOM_DSMEnable();
			uint8_t GetNumProcess();


			std::string GetAPPPath();
			
			std::string GetCameraDBPath();
			
			std::string GetProjDBPath();
			std::string GetProjInnerSrsFullPath();
			std::string GetProjUserSrsFullPath();
			void SetUpGDALSettings();
			std::string GetGDALPath();
			
			bool SetProjLibENV();
			
			std::string GetLogPath();
			
			std::string GetConfigPath();
			
			void SetRecentSRSsFile(std::string& File);
			
			void GetRecentProjectFiles();
			
			std::string GetEnginePath();
			std::string GetEngineConfigFile();
			
			void SetAPPVerison(std::string  version);
			std::string GetAPPVersion();

			bool ExportAppJson();
			
			
			std::string GetConfigFile();
			void ExportConfig();
			appconfig_s ParseConfig();
			

		private:
			std::string version_;
			int releaseversion_ = 0;
			int distribution_ = 0;
			bool useGDGS_ = false;
			bool useBaseGS_ = false;
			bool useTDOM_DSM_ = false;
			int gs_resolution = 0;
			int gs_iteration = 30000;
			int tile_point_threshold = 0;
		};
	}
}  

#endif  
