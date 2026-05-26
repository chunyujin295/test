
#include "Core/ProductionObject.h"
#include "Core/ReconstructionObject.h"


namespace AI3D
{
    namespace CORE
    {      
            ProductionObject::ProductionObject( production_option_s options,ReconstructionObject *reconstruction_object)
            {
                bTilesOrdered = false;
                bCompleted = false;
                reconstruction_object_ = reconstruction_object;
               
                options_ = options;

                for(auto& iter:options.tiles_)
                { 
                    production_tileinfo_s info;
                    info.name_ = iter;
                    info.status_ = jobsta_e::STATUS_PENDDING;
                    tiles_[iter] = info;
                }
             
              
                if (options.id_ != kInvalidProductionId)
                {
                    id_ = options.id_;
                }
                else
                {
                    id_ = kInvalidProductionId;
                }
                if (options.name_ != "")
                {
                    name_ = options.name_;
                }
                else
                {
                    name_ = PRODUCTION_PREFIX + std::to_string(id_);
                }
                status_ = jobsta_e::STATUS_UNKNOWN;
            }
            ProductionObject::~ProductionObject()
            {
                tiles_.clear();
            }
            const production_t& ProductionObject::GetId() const
            {
                return id_;
            }
            production_t& ProductionObject::GetIdMutual()
            {
                return id_;
            }
            void ProductionObject::SetName(const std::string& name)
            {
                name_ = name;
            }
            const std::string& ProductionObject::GetName() const
            {
                return name_;
            }
            std::string& ProductionObject::GetNameMutual()
            {
                return name_;
            }

            void ProductionObject::SetPath(std::string& path)
            {
                path_ = path;
            }
           const std::string ProductionObject::GetPath() const
            {
                return path_;
            }

            std::string ProductionObject::GetPathMutual()
            {
                return path_;
            }
            std::vector<std::string > ProductionObject::GetTilesByStatus(const jobsta_e& status)
            {
                std::vector<std::string > tilesresult;
                if (GetTiles().empty())
                {
                    LOGE("Fatal error no tiles ");
                    return std::vector<std::string >();
                }

                for (auto& iter : GetTiles())
                {
                    if (iter.second.status_ == status)
                    {
                        tilesresult.push_back(iter.second.name_);
                    }

                }
                return tilesresult;
            }

            std::vector<std::string>& ProductionObject::GetOrderedTiles()
            {
                if (!ordered_tiles_.empty())
                    return ordered_tiles_;

                std::vector<std::string> tiles = this->reconstruction_object_->GetOrderedTiles();
                auto tiles_in_production = GetTilesMutual();
             
                std::map<int, std::string> index_tilename;
                for (int index = 0; index < tiles.size(); index++)
                {
                    std::string tilename = tiles[index];
                    if (!tiles_in_production.count(tilename))
                    {
                        continue;
                    }
                   
                    ordered_tiles_.push_back(tilename);
                }

                return ordered_tiles_;
            }
            void ProductionObject::ReName(std::string name)
            {
                name_ = name;
            }
          

            const production_option_s& ProductionObject::GetOptions() const
            {
                return options_;
            }

            production_format_e ProductionObject::GetFormat()
            {
                return options_.production_format_;
            }

            std::string ProductionObject::GetFormatString()
            {
                return options_.GetFormatString();
            }

            EIGEN_STL_UMAP(std::string, production_tileinfo_s)& ProductionObject::GetTilesMutual()
            {
                return tiles_;
            }


            const EIGEN_STL_UMAP(std::string, production_tileinfo_s)& ProductionObject::GetTiles() const
            {
                return tiles_;
            }

            const std::string ProductionObject::GetIDString() const
            {
                return PRODUCTION_PREFIX + std::to_string(id_);
            }

            void ProductionObject::SetCompleted()
            {
                bCompleted = true;
            }

            bool ProductionObject::IsCompleted()
            {
                return bCompleted;
            }

            
            
            
            
            
            
            
            
            
            
    };

   
}
