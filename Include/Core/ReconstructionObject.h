
#ifndef _AI3D_CORE_RECONSTRUCTIONOBJECT_H_
#define _AI3D_CORE_RECONSTRUCTIONOBJECT_H_
#include <Constants.h>
#include <glog/logging.h>
#include "Core/ATData.h"

#include "Core/Types.h"
#include "Core/ReturnCode.h"
#include "Core/String.h"

#include "Core/ProductionObject.h"
#include "Core/Tiling.h"
#include "Core/BlockInfo.h"

namespace AI3D
{
    namespace CORE
    {
        enum bbox_calc_mode_e
        {
            BBCALC_BY_TIEPOINTS,
            BBCALC_BY_VIEWS,
            BBCALC_BY_VIEW_FRUSTUMS,
        };
       

        static ABBox3d ConvertToBBox(std::vector<std::vector<Eigen::Vector3d> > points)
        {
            ABBox3d newbox;
           
            for (int i = 0; i < points.size(); i++)
            {
               
                for (int j = 0; j < points[i].size(); j++)
                {
                    newbox.extend(points[i][j]); 
                }
            
            }
            return newbox;
        }
        class AI3D_API ReconstructionObject
        {
        public:
            
            

            ReconstructionObject(const ReconstructionObject& object);
            void CopyAll(const ReconstructionObject& object);
            void CopyBase(const ReconstructionObject& object);
            ReconstructionObject(block_t blockid) ;
            ReconstructionObject(const ATData& atdata, block_t blockid);
            ReconstructionObject(ATData&& atdata, block_t blockid);
            ~ReconstructionObject();
            
            void SetId(const reconstruction_t& id);
            const reconstruction_t& GetId() const;
            reconstruction_t& GetIdMutual();

            void SetName(std::string name);
            const std::string& GetName() const;
            std::string& GetNameMutual();
            void ReName(std::string name);

            void SetPath(std::string& path);
            std::string GetPath();

            const ABBox3d ComputeGlobalBoxCustom();
            
            

            
            
            
            
            void SetBaseSrs(const srs_s& srs);
            const srs_s& GetBaseSrs() const;
            srs_s& GetBaseSrsMutual();
            ProductionObject* GetProduction(production_t id);
            ProductionObject* GetProductionMutual(production_t id);
            void SetCustomSrs(const srs_s& srs);
            const srs_s& GetCustomSrs() const;
            srs_s& GetCustomSrsMutual();
            

            
            

         

            void SetBoundingBoxCustom(const ABBox3d& bb);
            const ABBox3d& GetBoundingBoxCustom() const;
            ABBox3d& GetBoundingBoxCustomMutual();
            

            
           

            void SetBoundaryCustom(const std::vector < std::vector<Eigen::Vector2d>>& bb);
            const std::vector < std::vector<Eigen::Vector2d>>& GetBoundaryCustom() const;
            std::vector < std::vector<Eigen::Vector2d>>& GetBoundaryCustomMutual();
            bool HasBoundary();

            
            int LoadBoundaryExternKml(const std::string& file);
            int SaveBoundaryExternKml(const std::string& file);
            

             
            

            void SetConstraintCustom(const std::vector<constraint_info_s>& bb);
            const std::vector<constraint_info_s>& GetConstraintCustom() const;
            std::vector<constraint_info_s>& GetConstraintCustomMutual();

            
          
             

             
              
            int StatisticsNumEmptyTiles();
            
            int GetNumTiles(bool discardempty);
            int GetNumTiles();
            bool HasTiles() const;
            bool HasConstraints() const;
            
            bool UpdateTileInfos();
            
            int GetNumTilesByStatus(const jobsta_e& status);
            int GetNumTilesRetouched();
            std::vector<std::string > GetTilesByStatus(production_t production_id, const jobsta_e& status);
            void UpdateConstraint(const std::vector<constraint_info_s>& cinfos_touse);
            void RunTiling();
            void WriteTiles(const std::string& path);
            

            const ATData &GetATData() const { return atdata_; };
            ATData &GetATDataMutual()  { return atdata_; };
            void SetATData(ATData& atdata) { atdata_ = atdata; };
            void SetATDataCustom(ATData& atdata) { atdata_custom_ = atdata; };
            const ATData &GetATDataCustom() const { return atdata_custom_; };
            ATData &GetATDataCustomMutual() { return atdata_custom_; };
            
           
            production_t GenerateValidProductionId();
            bool ExistsProduction(production_t pro_id);
            bool ExistProductionId(const production_t& id);
            void AddProduction(ProductionObject* object);
            const std::set<std::string> GetTilesName(bool bdiscardempty = true);
            

             const block_t  GetBlockId() const  { return block_id_; }  ;

             
             

             
             

             
             
             
             
             
             
             
             
             
             


             void SetTilesCustom( EIGEN_STL_UMAP(std::string, tile_info_s) tiles)
             { 
               
                 tiles_custom_ = tiles;
             }

             const EIGEN_STL_UMAP(std::string, tile_info_s)& GetTilesCustom() const
             { 
             
                return tiles_custom_;
             } 
             EIGEN_STL_UMAP(std::string, tile_info_s)& GetTilesCustomMutual()
             {
               
                 return tiles_custom_;
             }
            EIGEN_STL_UMAP(production_t, ProductionObject*) GetProductionsMutual()  {
                 return  productions_;
             };
           const EIGEN_STL_UMAP(production_t, ProductionObject*) GetProductions()const {
                return  productions_;
            };

             
           
            std::string GetIDString();
            
            
            void SetProcessingSettings(const processing_settings_s& settings);
            const processing_settings_s& GetProcessingSettings() const {
                return processing_settings_;
            };
            processing_settings_s& GetProcessingSettingsMutual();
            Tiling* GetTilingDiscriptorMutual();
           const Tiling* GetTilingDiscriptor() const;
            void SetTilingDisriptor(Tiling* tile);

            void ToTaskInfo(blk_recontruction_info_s& info);



            
           
          
            void TransformFromATDataCustomToBase();
            void TransformFromATDataBaseToCustom();
           
           
            static void TransformFromConstraint(std::vector<constraint_info_s>& cinfos, const std::string& src_srs,const std::string& des_srs);
            
            
           
            
            
           
            void OrderTiles();
            std::vector<std::string> GetOrderedTiles();
            
        
            
           
            
            bool SaveConstraints();
            bool SaveGlobalConstraintFile(const std::string& path);
            bool LoadGlobalConstraintFile(const std::string& path);
            bool DeleteProduction(production_t id);
            bool HasProductions();

            void SetWidget(void* pWidget);
            void* GetWidget();



            void GetCustomTilingSrs(std::string& definition,Eigen::Vector3d& origin);
        private:

            
            ATData atdata_;
            ATData atdata_custom_;
            
             
            srs_s srs_base_;
            srs_s srs_custom_;
            reconstruction_t id_;
            std::string name_;
            
           
           
            ABBox3d boundingbox_custom_;
       

            
           
            std::vector < std::vector<Eigen::Vector2d>> boundary_custom_;
            
            
             
          
           
            std::vector<constraint_info_s> constraint_custom_;
            
           
            
            
            EIGEN_STL_UMAP(std::string, tile_info_s) tiles_custom_;
            EIGEN_STL_UMAP(production_t, ProductionObject*) productions_;
            Tiling* tiling_discriptor_ = nullptr;
           
          
            processing_settings_s processing_settings_;
            block_t block_id_;
            std::string path_;
            void* pWidget = nullptr;
        };


       

        };

   
}
#endif