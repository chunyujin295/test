
#ifndef _AI3D_CORE_BLOCKINFO_H_
#define _AI3D_CORE_BLOCKINFO_H_
#include "Core/ReconstructionOptions.h"
namespace AI3D
{
    namespace CORE
    {

          

        struct AI3D_API blk_reconst_production_info_s
        {
            
            production_t id_;
            production_option_s options_;
            std::string name_;

            int consumed = 0;        // 实际消耗积分
            int refunded = 0;        // 返还积分
            int total_balance = 0;   // 结算后余额
            int available_points = 0;//可用积分

            void CreateJson(rapidjson::Value& jstr, rapidjson::Document& doc) 
            {
                rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
                jstr.AddMember("id", rapidjson::Value(id_), allocator);
                if (name_ != "")
                {
                    jstr.AddMember("name", rapidjson::Value(name_.c_str(), allocator), allocator); // was: GBK2UTF8
                }
               

                
                rapidjson::Value optionjson(rapidjson::kObjectType);
                options_.CreateJson(optionjson, doc);
                jstr.AddMember("settings", optionjson, allocator);
                jstr.AddMember("consumed", rapidjson::Value(consumed), allocator);
                jstr.AddMember("refunded", rapidjson::Value(refunded), allocator);
                jstr.AddMember("total_balance", rapidjson::Value(total_balance), allocator);
                jstr.AddMember("available_points", rapidjson::Value(available_points), allocator);


            }
            void ParseJson(const rapidjson::Value& jstr) 
            {
                
                if (jstr.HasMember("id"))
                {
                    id_ = (jstr["id"].GetInt());
                }

                if (jstr.HasMember("name"))
                {
                  // name_ = UTF82GBK(jstr["name"].GetString());
                   name_ = jstr["name"].GetString();
                }


                if (jstr.HasMember("settings"))
                {
                   options_.ParseJson(jstr["settings"]);
                }
                if (jstr.HasMember("consumed")) consumed = jstr["consumed"].GetInt();
                if (jstr.HasMember("refunded")) refunded = jstr["refunded"].GetInt();
                if (jstr.HasMember("total_balance")) total_balance = jstr["total_balance"].GetInt();
                if (jstr.HasMember("available_points")) available_points = jstr["available_points"].GetInt();


            }
        };
        
        struct AI3D_API blk_recontruction_info_s
        {
            
            reconstruction_t id_;
            std::string name_;
            
            processing_settings_s processing_settings_;

            
            srs_s srs_custom_;
            ABBox3d boundingbox_custom_;
            std::vector < std::vector<Eigen::Vector2d>> boundary_custom_;
            EIGEN_STL_UMAP(std::string, tile_info_s) tiles_;
            tiling_param_s tile_params_;


           
            


            std::vector<blk_reconst_production_info_s> production_infos_;


            void CreateJson(rapidjson::Value& value, rapidjson::Document& doc) 
            {
                auto& allocator = doc.GetAllocator();
                auto reconstruction_id = id_;
                if (reconstruction_id != kInvalidReconstructionId)
                {
                    value.AddMember("id", rapidjson::Value(reconstruction_id), allocator);
                }
                if (name_ != "")
                {
                    value.AddMember("name", rapidjson::Value(name_.c_str(), allocator), allocator); // was: GBK2UTF8
                }
                
                if (srs_custom_.isValid())
                {
                    rapidjson::Value srsjson(rapidjson::kObjectType);
                    srs_custom_.CreateJson(srsjson, doc);
                    value.AddMember("srs_custom", srsjson, allocator);
                }
                

                bbox_s bb(boundingbox_custom_);
                if (bb.isValid())
                {
                    rapidjson::Value bbcostumjson(rapidjson::kObjectType);
                    bb.CreateJson(bbcostumjson, doc);
                    value.AddMember("boundingbox_custom", bbcostumjson, allocator);
                }
                
                if (!boundary_custom_.empty())
                {

                    {
                        rapidjson::Value boundarysjson(rapidjson::kArrayType);
                        for (int index = 0; index < boundary_custom_.size(); index++)
                        {
                            rapidjson::Value boundaryjson(rapidjson::kArrayType);
                            for (int indexj = 0; indexj < boundary_custom_[index].size(); indexj++)
                            {
                                auto boundary = boundary_custom_[index][indexj];
                                std::string ptstr = std::to_string(boundary.x()) + "," +
                                    std::to_string(boundary.y());
                                boundaryjson.PushBack(rapidjson::Value(ptstr.c_str(), allocator), allocator);
                            }
                            boundarysjson.PushBack(boundaryjson, allocator);
                        }
                        value.AddMember("boundary_custom", boundarysjson, allocator);
                    }

                    
                }
                


                
                
                rapidjson::Value tilingjson(rapidjson::kObjectType);
                tile_params_.CreateJson(tilingjson, doc);
                value.AddMember("tilling", tilingjson, allocator);

                
                if (!tiles_.empty())
                {
                    rapidjson::Value tilesjson(rapidjson::kArrayType);


                    for (auto& tile : tiles_)
                    {
                        rapidjson::Value tilejson(rapidjson::kObjectType);
                        tile.second.CreateJson(tilejson, doc);
                        tilesjson.PushBack(tilejson, allocator);

                    }
                    value.AddMember("tiles", tilesjson, allocator);
                }
                
                if (!production_infos_.empty())
                {
                    rapidjson::Value productionsjson(rapidjson::kArrayType);


                    for (auto& production : production_infos_)
                    {
                        rapidjson::Value productionjson(rapidjson::kObjectType);
                        production.CreateJson(productionjson, doc);
                        productionsjson.PushBack(productionjson, allocator);

                    }
                    value.AddMember("productions", productionsjson, allocator);
                }



                rapidjson::Value settingjson(rapidjson::kObjectType);
                processing_settings_.CreateJson(settingjson, doc);
                value.AddMember("settings", settingjson, allocator);

            }

            void  ParseJson(rapidjson::Value& jstr)
            {
                if (jstr.HasMember("id"))
                {
                    id_ = jstr["id"].GetInt();

                }
                if (jstr.HasMember("name"))
                {
                    name_ = jstr["name"].GetString();
                    // name_ = UTF82GBK(name_);

                }
                if (jstr.HasMember("settings"))
                {
                    processing_settings_.ParseJson(jstr["settings"]);
                }

                if (jstr.HasMember("srs_custom"))
                {
                    srs_custom_.ParseJson(jstr["srs_custom"]);
                }
                if (jstr.HasMember("boundingbox_custom"))
                {
                    bbox_s bb;
                    bb.ParseJson(jstr["boundingbox_custom"]);
                    boundingbox_custom_ = bb.toABBox3d();
                }
                if (jstr.HasMember("boundary_custom"))
                {
                    tiling_param_s param;
                    boundary_custom_.clear();
                    rapidjson::Value& value = jstr["boundary_custom"];
                    for (auto& iter1 : value.GetArray())
                    {
                        rapidjson::Value& boundaryjson2 = iter1;
                        std::vector<Eigen::Vector2d> bd(boundaryjson2.Size());
                        for (unsigned index = 0; index < boundaryjson2.Size(); index++)
                        {

                            auto str = boundaryjson2[index].GetString();
                            auto strs = AI3D::CORE::String::StringSplit(str, ",");
                            if (strs.size() != 2)
                            {
                                continue;
                            }

                            bd[index][0] = std::atof(strs[0].c_str());
                            bd[index][1] = std::atof(strs[1].c_str());
                        }
                       
                        boundary_custom_.push_back(bd);
                    }
                }

                if (jstr.HasMember("tilling"))
                {
                    tiling_param_s param;

                    rapidjson::Value& value = jstr["tilling"];
                    param.ParseJson(value);
                    tile_params_ = param;
                }
                if (jstr.HasMember("tiles"))
                {
                    tiles_.clear();
                    auto tilesjson = jstr["tiles"].GetArray();
                    for (unsigned i = 0; i < tilesjson.Size(); i++)
                    {
                        tile_info_s tileinfo;
                        tileinfo.ParseJson(tilesjson[i]);
                        tiles_[tileinfo.name_] = tileinfo;

                    }
                }
                if (jstr.HasMember("productions"))
                {
                   production_infos_.clear();
                    auto psjson = jstr["productions"].GetArray();
                    for (unsigned i = 0; i < psjson.Size(); i++)
                    {
                        blk_reconst_production_info_s pinfo;
                        pinfo.ParseJson(psjson[i]);
                        production_infos_.push_back(pinfo);

                    }
                }

                
            }


            
        };
    }
}
#endif