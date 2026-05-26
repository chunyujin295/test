#ifndef _AI3D_CORE_ATCOMMANDSET_H_
#define _AI3D_CORE_ATCOMMANDSET_H_
#include <Constants.h>
#include <glog/logging.h>
#include <pugixml.hpp>
#include "Core/ATData.h"
#include "Core/BlockObject.h"
#include "Core/Alignment.h"
#include "Core/Types.h"
#include "Core/ReturnCode.h"
#include "Core/String.h"
#include "Core/ATDefinition.h"



namespace AI3D
{
    namespace CORE
    {    
        class AI3D_API ATCommandSet
        {
        public:
            ATCommandSet() {};
           
            static int AddUserTiepoint(ATData& data,image_t image_id,std::string name);
            static int WriteUserTiepointsJson(const ATData& data,const std::string& file);
            static int WriteGCPMeasurementsJson(const ATData& data, const std::string& file);
            static int WritePOSJson(const std::string& file,Eigen::Vector3d sigma,
                const ATData& data, std::set<image_t> fixedids);
            static int WritePOSBin(const std::string& file, Eigen::Vector3d sigma,
                const ATData& data, std::set<image_t> fixedids);
            static int WriteGCPMeasurementsXML(const ATData& data, const std::string& file);
            static bool SaveSourceDataJson1(ATData& Atdata, const std::string file_path, const ATOptions& atoptions, Eigen::Vector3d possigma);
            static bool LoadSourceDataJson(ATData& Atdata, const std::string file_path, Eigen::Vector3d& possigma,std::string skfpath);
            static bool LoadSourceDataBinary(ATData& Atdata, const std::string file_path);
            static bool LoadSourceDataJson1(ATData& Atdata, const std::string file_path);
            static int ReadUserTiepointsJson(ATData& data, const std::string& file);
            static int ReadGCPMeasurementsJson(ATData& data, const std::string& file);
            static int ReadPOSJson(ATData& data, const std::string& file);
            static int ReadPOSBin(ATData& data, const std::string& file);
            static int ReadGCPMeasurementsXML(ATData& data, const std::string& file);
            
            static bool LoadATBinary(const std::string& AT_filepath, std::shared_ptr<ATData>& ATdata);
            static bool ExportATBinary(const std::string& AT_filepath, ATData& data);
            
            
            static bool SaveSourceDataJson(ATData& Atdata, const std::string file_path, Eigen::Vector3d possigma = {-1.0,-1.0,-1.0});
            static bool SaveSourceDataBinary(ATData& Atdata, const std::string file_path, Eigen::Vector3d possigma = { -1.0,-1.0,-1.0 });
            static bool CreateATFiles(ATData& atdata,std::string path, ATOptions& options);
            
            static bool CreateATTaskInfo(std::string hostname, std::string jobpath,
                 std::string blockpath, const AI3D::CORE::BlockObject::Task_Info& taskinfo,std::string& jobstr);

            static AT_complete_status_e  GetATCompleteStatus(const ATData& Atdata);
            static void GetBlockInformation();
            static int LoadBlock(const std::string& file, AI3D::CORE::BlockObject& atdata);

        };


       

        };

   
}
#endif 