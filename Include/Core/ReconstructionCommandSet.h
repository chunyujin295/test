#ifndef _AI3D_CORE_RECONSTRUCTIONPIPELINE_H_
#define _AI3D_CORE_RECONSTRUCTIONPIPELINE_H_
#include <Constants.h>
#include <glog/logging.h>
#include <pugixml.hpp>

#include "PointManager.h"
#include "Core/ATData.h"
#include "Core/BlockObject.h"
#include "Core/Alignment.h"
#include "Core/Types.h"
#include "Core/ReturnCode.h"
#include "Core/String.h"
#include "Core/ReconstructionOptions.h"
#include "Core/ProductionPurpose.h"
#include "Core/ReconstructionObject.h"
#include "Core/Types.h"



namespace AI3D
{
    namespace CORE
    {    
        class AI3D_API ReconstructionCommandSet
        {
        public:
            ReconstructionCommandSet();
            
            
           
            static int SubmitReconstruction(BlockObject* block, reconstruction_t& rid,
                                                     const processing_settings_s& options = processing_settings_s());

            /** Writes reconstruction SRS + RB.bin (deferred from SubmitReconstruction until first production). */
            static int ExportReconstructionViewBin(BlockObject* block, ReconstructionObject* reconstruction);

            static bool  CanSubmitProduction(const ReconstructionObject& object);
            static SubmitResult SubmitProduction(std::string hostname, std::string jobstr, std::string projectpath,
                                                 BlockObject* block, reconstruction_t reconstruction_id,
                                                 production_option_s options, production_t& production_id);
           
            
            static bool CanResubmitProduction(const ReconstructionObject& object, production_t production_id);
            
            static SubmitResult ResubmitProductionJob(std::string hostname, std::string jobstr,
                                                      std::string projectpath, BlockObject* block,
                                                      reconstruction_t reconstruction_id, production_t production_id);
           
            static void GetJobsToCancelled(const BlockObject& object, std::vector<std::pair<std::string, std::string> >& jobs_to_delete);
            static void GetJobsToCancelled(const ReconstructionObject& object, std::vector<std::pair<std::string, std::string> >& jobs_to_delete);
            static void GetJobsToCancelled(const ProductionObject& object, std::vector<std::pair<std::string, std::string> >& jobs_to_delete);
            static bool CanCancelProduction(const ReconstructionObject& object, production_t production_id);
            
            
            
            
                
            static int CancelProductionJob(std::string jobPath, BlockObject* block, reconstruction_t reconstruction_id, production_t production_id);
            static bool CheckJobQueuePath(const std::string& lsMasterJobQueue, std::string& lsPendingJobPath,
                std::string& lsRunningJobPath, std::string& lsCancelledJobPath,
                std::string& lsFailedJobPath, std::string& lsCompletedJobPath, std::string& lsPathSeperator, int& errorCode);
            static bool CanDeleteProduction(const ReconstructionObject& object, production_t production_id);
            static int DeleteProduction(BlockObject* block, const reconstruction_t& reconstruction_id,const production_t& production_id);

            static bool CanDeleteReconstruction(const ReconstructionObject& reconstruction);
            
           
            static int DeleteReconstruction(BlockObject* block, reconstruction_t reconstruction_id);

            static bool CanCloneReconstruction(const ReconstructionObject& reconstruction);
            static int CloneReconstruction(BlockObject* block,const reconstruction_t reconstruction_id, reconstruction_t& clone_reconstruction_id);
            
          
            
            
            static int ResetBoundingboxToDefault(BlockObject* block, reconstruction_t reconstruction_id);
            
           
            static int ResetBoundingboxScopeMode(BlockObject* block, reconstruction_t reconstruction_id, bb_scope_e mode);
            
            
            static bool GetSceneUnit(const ReconstructionObject& reconstruction);
           
            static int UpdateBlockInfo(BlockObject* block, reconstruction_t reconstruction_id);



            static int ResetSpatialFrameworkSRS(BlockObject* block, reconstruction_t reconstruction_id, const srs_s& srs);
            
            
           static  int ResetBoundingBox(ReconstructionObject* reconstruction, const ABBox3d& box);
            static int ResetBoundingBox(BlockObject* block, reconstruction_t reconstruction_id, const ABBox3d& bbox);
            static int Get2DPolygon(const std::vector<Eigen::Vector3d>& polygon3D, std::vector<Eigen::Vector2d>& polygon2D);
            static int ResetBoundary(BlockObject* block, reconstruction_t reconstruction_id,
                const std::vector<Eigen::Vector3d>& points);
            static int ResetBoundaryByFile(BlockObject* block, reconstruction_t reconstruction_id, std::string file, std::string& msg);

            

            static int ResetTilingMode(BlockObject* block, reconstruction_t reconstructionid, tiling_mode_e mode);
           
            static int ResetTileSize(BlockObject* block, reconstruction_t reconstruction_id, float value);
            static int ResetTileMAXRamUsage(BlockObject* block, reconstruction_t reconstruction_id, float value);

            static tiling_mode_e GetTilingMode(BlockObject* block, reconstruction_t reconstructionid);
            static float GetTileMAXRamUsage(BlockObject* block, reconstruction_t reconstruction_id);
            static float GetExpectedMaxRamUsageForAJob(ReconstructionObject* block);

            
            
            
            static int ImportConstraintFile(BlockObject* block, reconstruction_t reconstruction_id,
                const std::vector<std::string>& files, std::vector<constraint_info_s>& cinfos_touse, int& progress,std::string& msg,bool bNeedChineseMsg = false);
            
                       static int StaticTilesIntersectionWithConstraint(const EIGEN_STL_UMAP(std::string, tile_info_s)& tiles_got
                , std::vector<constraint_info_s>& cinfos, std::map<int, std::set< int> >& polygon_to_use,
                std::map<std::string,std::pair<int,int>>& tiles);
            
            static int DeleteConstraintsPre(BlockObject* block, reconstruction_t reconstruction_id,const std::vector<int>& indexs,
                std::map<std::string,bool>& tilestocancle);
            static int DeleteConstraintsPost(BlockObject* block, reconstruction_t reconstruction_id, const std::vector<int>& indexs, std::map<std::string, bool> tilestoprocess);

            
            static int DiscardEmptyTile(BlockObject* block, reconstruction_t reconstruction_id);
            

            
          
            static int CreateProductionJobFiles(std::string hostname, std::string jobpath, std::string projectpath,
                                                BlockObject* block, reconstruction_t reconstruction_id, production_t production_id, std::vector<std::string> tiles_to_production, PointFreezeInfo
                                                freezeResult);

            
            static int ResetBoudingBoxCalcMode(BlockObject* block, reconstruction_t reconstruction_id,  bbox_calc_mode_e mode);
            static std::string GenerateFeedbackFile(BlockObject* block, ReconstructionObject* reconstruction, ProductionObject* production, std::string tile_name, const std::string& lsMasterJobQueue = std::string(""), std::string fullPathJobName = std::string(""), int* jobStatus = nullptr);
            
            static std::string GenerateTileFeedbackFile(BlockObject* block_object,ProductionObject* production_object,std::string& tile,std::string& job);

            static std::string MakeProductionTileJobKey(block_t block_id, reconstruction_t reconstruction_id,
                production_t production_id, const std::string& tile_name);
            static std::string ResolveProductionTileJobStr(BlockObject* block, ReconstructionObject* reconstruction,
                ProductionObject* production, const std::string& tile_name, bool update_tile = true);
            static void SyncProductionTileJobStrs(BlockObject* block, ReconstructionObject* reconstruction,
                ProductionObject* production);

            static int ReNameReconstruction(BlockObject* block, reconstruction_t reconstruction_id, const std::string& name);
            static int ReNameProduction(BlockObject* block, reconstruction_t reconstruction_id, production_t production_id, const std::string& name);

            
           
            static void GetProductionSetInformation(ProductionObject* production, std::string& productionid,
                std::string& destination,std::string& srs_str,std::string& originstr);
            
            static void GetProductionSetInformation(ProductionObject* production, std::vector<std::pair<std::string,std::string> >& infos,std::vector<std::string> &translated_infos);

            static void InitProductionOptions(BlockObject* block,ReconstructionObject* reconstruction, production_option_s& options);

 
            
            static void InitProductionDefinitionName(ReconstructionObject* reconstruction, production_option_s& options);
            
            static  Mesh3DPurpose::mesh3d_formatoptions_s ResetMeshFormat(const mesh3d_format_e& format);

         
            
           
        };


       

        };

   
}
#endif