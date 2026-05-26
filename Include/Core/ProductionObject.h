
#ifndef _AI3D_CORE_PRODUCTION_H_
#define _AI3D_CORE_PRODUCTION_H_
#include <Constants.h>
#include "Core/BlockInfo.h"
#include "Core/ReconstructionOptions.h"


namespace AI3D
{
    namespace CORE
    {
        
      
        class AI3D_API ReconstructionObject;

        class AI3D_API ProductionObject
        {
            
            

		public:
           
            ProductionObject( production_option_s options,ReconstructionObject *reconstruction_object = nullptr);
            ~ProductionObject();
            void SetId(const production_t& id) { id_ = id; };
            const production_t& GetId() const;
            production_t& GetIdMutual();
            void SetName(const std::string& name);
            const std::string& GetName() const;
            std::string& GetNameMutual();
            void SetPath(std::string& path);
            const std::string GetPath() const;
            std::string GetPathMutual() ;
           

            std::vector<std::string > GetTilesByStatus(const jobsta_e& status);
            std::vector<std::string>& GetOrderedTiles();
            void ReName(std::string name);
            const production_option_s& GetOptions() const ;
            void SetTiles(EIGEN_STL_UMAP(std::string, production_tileinfo_s)& tiles) 
            { 
                tiles_.clear(); 
                tiles_ = tiles;
            };
            
            EIGEN_STL_UMAP(std::string, production_tileinfo_s)& GetTilesMutual();
            const EIGEN_STL_UMAP(std::string, production_tileinfo_s)& GetTiles() const;
            
            const std::string GetIDString() const;

            enum production_format_e GetFormat();
            std::string GetFormatString();

            void SetCompleted();
            bool IsCompleted();

            
        private:           
            EIGEN_STL_UMAP(std::string, production_tileinfo_s) tiles_;
            production_t id_;
            production_option_s options_;
            std::string name_;
            std::string path_;            
            jobsta_e status_;
            ReconstructionObject* reconstruction_object_;
            std::vector<std::string> ordered_tiles_;
            bool bTilesOrdered;
            bool bCompleted;
        };      
    };

   
}
#endif