#include "Core/Application.h"
#ifdef _MSC_VER
#include <Windows.h>
#else
#include <unistd.h>
#include <dirent.h>
#endif
#include "Util/Statistic.h"
#include "Core/String.h"
#include "Core/File.h"
#include <pugixml.hpp>
#include <boost/program_options.hpp>
#include <filesystem>
#include "boost/regex.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/progress.hpp>
#include <boost/foreach.hpp>
#include "ogrsf_frmts.h"
#include <Core/CoordinateSystem.h>
#include "Core/ATOptions.h"
#include "Core/WorkPath.h"
namespace AI3D
{
	namespace CORE
	{
		

		uint8_t Application::GetNumProcess()
		{
			return omp_get_num_procs();
		}

		std::string Application::GetAPPPath()
		{
			std::string app_path;
			
			try
			{
#ifdef _MSC_VER
				TCHAR buf[MAX_PATH + 1] = { 0 };
				GetModuleFileName(NULL, buf, MAX_PATH);
				std::string path = File::BoostPathToUtf8String(std::filesystem::path(buf).parent_path());
				path = AI3D::CORE::File::EnsureUnitPath(path);
				return path;
#else 
				LPTSTR home = getenv("HOME");
				if (home == NULL)
					return String();
				String name(String(home) + "/app");
				return ensureUnifySlash(name);
#endif 
			}
			catch (const std::filesystem::filesystem_error& fse)
			{
				std::ostringstream oss;
				oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
				LOGI(oss.str());
			}
			catch (std::exception& ex)
			{
				std::ostringstream oss;
				oss << "exception:" << ex.what();
				LOGI(oss.str());
			}

			return app_path;
		}
		std::string Application::GetCameraDBPath()
		{
			const std::string cameradb_path = GetAPPPath() + "/" + CAMERA_DATABASE;
			return cameradb_path;
		}

		std::string  Application::GetProjInnerSrsFullPath()
		{
			std::string path =GetProjDBPath() + "/" + SRS_DATABASE;
			path = File::EnsureUnifySlash(path);
			return path;
		}
		std::string  Application::GetProjUserSrsFullPath()
		{
			std::string path = GetProjDBPath() + "/" + USER_DATABASE;
			path = File::EnsureUnifySlash(path);
			return path;
		}
		std::string Application::GetProjDBPath()
		{

			std::string proj_path;
			std::vector<std::string> paths;
			proj_path = GetAPPPath();
			
			paths.push_back(proj_path);
			paths.push_back(proj_path+"/data/");
			std::string validpath = "";
			for (int i = 0; i < paths.size(); i++)
			{
				std::string projdbpath = paths[i] + PROJ_DATABASE;
				bool ret = File::ExistsDir(paths[i]) && File::ExistsFile(projdbpath);
				if (ret)
				{
					validpath = paths[i];

					return validpath;

				}
			}

			return validpath;
		}
		std::string Application::GetGDALPath()
		{
			std::string gdal_path;
			gdal_path = GetAPPPath() + "/data";
			return gdal_path;
		}
		void Application::SetUpGDALSettings()
		{
			GDALAllRegister();
			std::string gdalDir = AI3D::CORE::Application::Getinstance().GetGDALPath();
			const char* paths[] = {gdalDir.c_str(),nullptr};
			OSRSetPROJSearchPaths(paths);
			CPLSetConfigOption("GDAL_DATA",paths[0]);
			CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
			CPLSetConfigOption("SHAPE_ENCODING", "");
		}
		bool Application::SetProjLibENV()
		{
			if (GetProjDBPath() != "")
			{
				std::string strEnv = "PROJ_LIB=" + GetProjDBPath();
				int status = putenv(strEnv.c_str());
				if (status != 0)
				{
					return false;
				}
				return true;
			}
			return false;
		}
		std::string Application::GetLogPath()
		{
			std::string log_path;
			log_path = GetAPPPath();
			return log_path;
		}
		std::string Application::GetConfigPath()
		{
			std::string configStr;
			configStr = GetAPPPath();
			String::StringReplace(configStr, "\\", "/");
			return configStr;
		}

		int Application::GetDistribution()
		{
			return distribution_;
		}

		bool Application::isGDGSEnable() {
			return useGDGS_;
		}

		bool Application::isBaseGSEnable() {
			bool result = useBaseGS_ && isBaseGSPluginExist();
			return result;
		}

		bool Application::isBaseGSPluginExist() {
			std::string gsPluginFile = GetAPPPath() + "/plugin/MoldAIGauss.exe";
			if (File::ExistsPath(gsPluginFile)) {
				return true;
			}
			else {
				return false;
			}
		}

		int Application::getGSIteration() {
			return gs_iteration;
		}

		int Application::getGSResolution() {
			return gs_resolution;
		}

		bool Application::isTDOM_DSMEnable() {
			return useTDOM_DSM_;
		}

		int Application::GetReleaseLevel()
		{
			return releaseversion_;
		}
		
		void Application::SetRecentSRSsFile(std::string& File)
		{

		}
		
		void  Application::GetRecentProjectFiles()
		{

		}

		std::string Application::GetConfigFile()
		{
			std::string configStr = GetConfigPath();
			String::StringTrim(configStr, "/");
			return  File::EnsureUnifySlash(configStr + "/MoldAIConfig.ini");
		}
		

		void Application::ExportConfig()
		{
			jconfigopt_s opt;
			opt.SaveXML(GetConfigFile());
		}


		bool Application::ExportAppJson()
		{
			std::string machineCode ;
			std::string postFix = "";
			if (STAT_USE_BIN) {
				postFix = BINFILE_POSTFIX;
			}
			else {
				postFix = JSONFILE_POSTFIX;
			}
			
			std::string MasterJson = machineCode + STAT_MASTER_POSTFIX + postFix;
			std::string EngineJson = machineCode + STAT_ENGINE_POSTFIX + postFix;
			return true;
		}


		
		appconfig_s Application::ParseConfig()
		{
			static appconfig_s saved_options;
			static bool bParsedConfig = false;

			
			jconfigopt_s opt;
			opt.LoadXML(GetConfigFile());
			saved_options.keyMaxImgNum = opt.keyMaxImgNum;
			saved_options.matchMaxImgNum = opt.matchMaxImgNum;
			
			saved_options.at_options.maxthreads_num = opt.maxthreads_num;
			saved_options.focal_length = opt.focal_length;
			saved_options.debug_level = opt.debug_level;
			saved_options.version = opt.version;
			if (opt.use_gs == 0) {
				useGDGS_ = false;
				useBaseGS_ = false;
			}
			else if (opt.use_gs == 1) {
				useGDGS_ = true;
				useBaseGS_ = false;
			}
			else {
				useGDGS_ = false;
				useBaseGS_ = true;
			}
			if (opt.use_tdom_dsm == 0) {
				useTDOM_DSM_ = false;
			}
			else {
				useTDOM_DSM_ = true;
			}
			gs_resolution = opt.gs_resolution;
			gs_iteration = opt.gs_iteration;
			
			
			saved_options.at_options.sfmsettings.grid_count_1 = opt.ba1_grid_count;
			saved_options.at_options.sfmsettings.grid_count_2 = opt.ba2_grid_count;
			saved_options.at_options.sfmsettings.max_feature_count_1 = opt.max_feature_count_1;
			saved_options.at_options.sfmsettings.max_feature_count_2 = opt.max_feature_count_2;
			saved_options.at_options.saveoptions.min_overlap = opt.min_overlap;
			saved_options.at_options.saveoptions.max_overlap = opt.max_overlap;
			saved_options.at_options.saveoptions.max_tiepoint_num = opt.max_tiepoint_num;
			saved_options.at_options.saveoptions.boutput_tiepoint = opt.boutput_tiepoint;
			saved_options.at_options.saveoptions.max_projection_error = opt.max_projection_error;
			saved_options.at_options.sfmsettings.sfm_mode = (sfm_mode_e)opt.sfm_mode;
			
			saved_options.at_options.saveoptions.output_rawxml = opt.output_rawxml;
			releaseversion_ = opt.releaselevel;
			distribution_ = opt.distribution;
			int language = opt.language;
			if (language > 0)
				BlockObject::setChineseVersion();
			saved_options.at_options.blockat = opt.blockat;
			bParsedConfig = true;
			saved_options.to_gs = opt.to_gs;
			saved_options.tile_point_threshold = opt.point_threadHold;
			return saved_options;
		}

		

	}
}  


