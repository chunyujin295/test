
#ifndef _AI3D_CORE_PROJECT_H_
#define _AI3D_CORE_PROJECT_H_
#include <Constants.h>
#include "Core/ATData.h"
#include "Core/alignment.h"
#include "Core/Types.h"
#include "Core/BlockObject.h"
#include "Core/Rapidjson.h"
#include "Core/File.h"

namespace AI3D
{
    namespace CORE
    {

        class AI3D_API ProjectObject
        {
        public:
            ProjectObject();
            
            
            
            
            int NewProject(std::string name, std::string path);

            
            
            
            void AddBlock(BlockObject* block); 
            
            
            
            bool ImportBlock(const std::string& name);
            
            
            block_t GetNumBlocks();
            
            

            class BlockObject* GetBlock(block_t id);
            class BlockObject* GetBlockMutual(block_t id);
            class BlockObject* GetBlockByImportFilename(const std::string& importFilename);

            
            
            bool DeleteBlock(const block_t id);
            
            
             
            std::string GenerateValidBlockName(std::string rawname);
            bool CloneBlock(block_t &id, bool AT_USED = false,std::string prefix = "-copy");
            bool CloneBlock(BlockObject* block, std::string prefix);

            bool MergeAndAdjustBlocks(const std::set<block_t>& blockids);

            
            bool MergeBlocks(const std::set<block_t>& blockids);
            
            class BlockObject* GetCurrentBlock();
            
            void Clear();
            
            
            const std::string GetName()const;
            const std::string GetPath()const;
            const std::string GetFullName() const;
            std::string& GetNameMutual();
            std::set<block_t> GetBlockIds() {  return blockids_; };
            
            
            void ReName(std::string name);

            
            bool Check();
            bool ExistsBlock(block_t block_id);
            block_t GenerateValidBlockId();
            
            
            bool Load(const std::string& path);
           
            bool LoadJson(const std::string& path);

			int SaveJson(const std::string& path, savetype_e savetype);

            bool LoadBin(const std::string& path);

            int SaveBin(const std::string& path, savetype_e savetype);

            int Save(savetype_e savetype);
            bool LoadBlockData(block_t block_id);
			bool SaveBLK(block_t block_id, savetype_e savetype);
            EIGEN_STL_UMAP(block_t, std::vector<int>) GetBlocksStatisics();
            void SetBlocksStatisics(const std::pair<block_t, std::vector<int>>& blocks_statistics);

            ~ProjectObject()
            {
                for (auto& block_ptr : blocks_)
                {
                    delete block_ptr.second;
                }
            }

            EIGEN_STL_UMAP(block_t, BlockObject*) GetBlocksMutual()
            {
                return blocks_;
            }
            EIGEN_STL_UMAP(block_t, BlockObject*) const GetBlocks() const
            {
                return blocks_;
            }
        private:
            
            std::string name_ = "";
            
            std::string path_ = "";
            
            EIGEN_STL_UMAP(block_t, BlockObject*) blocks_;

            
            EIGEN_STL_UMAP(block_t, std::vector<int>) blocks_statistics_;

            std::set<srsid_t> srs_ids_;
            
            std::set<block_t> blockids_;
           
        };
    }
}
#endif