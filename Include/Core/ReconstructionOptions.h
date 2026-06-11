
#ifndef _AI3D_CORE_RECONSTRUCTIONOPTIONS_H_
#define _AI3D_CORE_RECONSTRUCTIONOPTIONS_H_
#include <Constants.h>
#include <glog/logging.h>
#include <pugixml.hpp>
#include "Core/ATData.h"
#include "Core/TaskDef.h"
#include "Core/Alignment.h"
#include "Core/Types.h"
#include "Core/ReturnCode.h"
#include "Core/String.h"

#include "Core/Rapidjson.h"
#include "Core/CoordinateSystem.h"
#include "DataStruct.h"
#include  "Core/ProductionTemplate.h"
#define DEFAULTCOLOR 128
namespace AI3D
{
    namespace CORE
    {

        enum untexture_policy_e
        {
            UNTEX_COLOR_FILLED,
            UNTEX_COLOR_INPAITING,
        };
        enum holefilling_policy_e
        {
            HOLEFILL_SMALL,
            HOLEFILL_ALL,
        };


        struct tile_info_s
        {

            enum class reconst_status_e 
            {
                RE_STA_UNPROCESSED,
                RE_STA_COMPLETED,
                

            };

            tile_info_s() {};
            tile_info_s(std::string name, ABBox3f bb)
            {
                name_ = name;
                bb_ = bb;
            }

            bool isempty = false;
            int index_ = 0;
            ABBox3f bb_;
            
           
            std::string name_;
            
             

            float ram_estimated_ = 0.0;
            std::set<image_t> image_ids_;
            std::set<point3D_t> point_ids_;

            
            
           

            
              
            reconst_status_e reference_model_status_;
            
            retouching_status_e retouching_status_;
            
            void CreateJson(rapidjson::Value& jstr, rapidjson::Document& doc)
            {
                rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
                if (index_ > 0)
                    jstr.AddMember("index", rapidjson::Value((int)index_), allocator);
                jstr.AddMember("name", rapidjson::Value(name_.c_str(), allocator), allocator);
                jstr.AddMember("status", rapidjson::Value((int)reference_model_status_), allocator);
                jstr.AddMember("isempty", rapidjson::Value(isempty), allocator);
                bbox_s bb(bb_.cast<double>());
                if (bb.isValid())
                {
                    rapidjson::Value bbcostumjson(rapidjson::kObjectType);
                    bb.CreateJson(bbcostumjson, doc);
                    jstr.AddMember("boundingbox", bbcostumjson, allocator);
                }

                

                
                std::string idsstr = "";
                for (auto& iter : image_ids_)
                {
                    idsstr += std::to_string(iter) + " ";
                }
                jstr.AddMember("imageids", rapidjson::Value(idsstr.c_str(), allocator), allocator);


            }
            void ParseJson(rapidjson::Value& jstr)
            {
                if (jstr.HasMember("name"))
                {
                    name_ = jstr["name"].GetString();
                }
                if (jstr.HasMember("index"))
                {
                    index_ = jstr["index"].GetInt();
                }
                if (jstr.HasMember("status"))
                {
                    reference_model_status_ = reconst_status_e(jstr["status"].GetInt());
                }
                if (jstr.HasMember("boundingbox"))
                {
                    bbox_s bb;
                    bb.ParseJson(jstr["boundingbox"]);
                    bb_ = bb.toABBox3d().cast<float>();
                }
                if (jstr.HasMember("isempty"))
                {
                    isempty = jstr["isempty"].GetBool();
                    
                }
                if (jstr.HasMember("imageids"))
                {
                    std::string idsstr = "";

                    idsstr = jstr["imageids"].GetString();
                    auto idsvec = String::StringSplit(idsstr, " ");
                    for (auto& iter : idsvec)
                    {

                        auto id = std::atoi(iter.c_str());
                        image_ids_.insert(id);
                    }
                }
            }



        };


        enum production_purpose_e
        {
            EXPORT_3D_MESH,
            EXPORT_3D_POINT_CLOUD,
            EXPORT_ORTHOPHOTO_DSM,
            EXPORT_3D_MESH_FOR_EXTERNAL_RETOUCHING,
            EXPORT_POINTCLOUD_GDGS,
            EXPORT_POINTCLOUD_BASEGS,
            
        };

        /** BaseGS scene scale (场景规模), maps to backend --percent_dense. */
        enum basegs_scene_scale_e
        {
            BASEGS_SCENE_SCALE_SMALL = 0,  // 小型场景
            BASEGS_SCENE_SCALE_AUTO = 1,   // 自动
            BASEGS_SCENE_SCALE_LARGE = 2,  // 大型场景
        };

        /** BaseGS quality preset (低 / 中 / 高). */
        enum basegs_quality_mode_e
        {
            BASEGS_QUALITY_FAST = 0,       // 低：快速模式
            BASEGS_QUALITY_BALANCED = 1,   // 中：均衡模式
            BASEGS_QUALITY_HIGH = 2,       // 高：高精模式
        };

        /** BaseGS compute device: only "GPU" or "CPU" (JSON / CLI value). */
        enum basegs_data_device_e
        {
            BASEGS_DATA_DEVICE_GPU = 0,
            BASEGS_DATA_DEVICE_CPU = 1,
        };

        inline const char* BaseGsDataDeviceToString(basegs_data_device_e device)
        {
            return device == BASEGS_DATA_DEVICE_CPU ? "CPU" : "GPU";
        }

        inline basegs_data_device_e BaseGsDataDeviceFromString(const std::string& value)
        {
            if (value == "CPU" || value == "cpu")
                return BASEGS_DATA_DEVICE_CPU;
            return BASEGS_DATA_DEVICE_GPU;
        }

        /** BaseGS training parameters (backend CLI field names). */
        struct basegs_params_s
        {
            int iterations_ = 15000;
            int resolution_ = 2048;
            basegs_scene_scale_e scene_scale_ = BASEGS_SCENE_SCALE_AUTO;
            basegs_data_device_e data_device_ = BASEGS_DATA_DEVICE_GPU;
            bool antialiasing_ = true;
            int densify_from_iter_ = 500;
            int densify_until_iter_ = 12000;
            int densification_interval_ = 100;
            float densify_grad_threshold_ = 0.0008f;
            int opacity_reset_interval_ = 3000;
            float lambda_dssim_ = 0.15f;
            int sh_degree_ = 3;

            float GetPercentDense() const
            {
                switch (scene_scale_)
                {
                case BASEGS_SCENE_SCALE_SMALL:
                    return 0.02f;
                case BASEGS_SCENE_SCALE_LARGE:
                    return 0.005f;
                case BASEGS_SCENE_SCALE_AUTO:
                default:
                    return 0.01f;
                }
            }
        };

        inline basegs_params_s GetBaseGsParamsFast()
        {
            basegs_params_s p;
            p.iterations_ = 7000;
            p.resolution_ = 1024;
            p.scene_scale_ = BASEGS_SCENE_SCALE_SMALL;
            p.data_device_ = BASEGS_DATA_DEVICE_GPU;
            p.antialiasing_ = false;
            p.densify_from_iter_ = 500;
            p.densify_until_iter_ = 5000;
            p.densification_interval_ = 300;
            p.densify_grad_threshold_ = 0.0015f;
            p.opacity_reset_interval_ = 3000;
            p.lambda_dssim_ = 0.1f;
            p.sh_degree_ = 2;
            return p;
        }

        inline basegs_params_s GetBaseGsParamsBalanced()
        {
            basegs_params_s p;
            p.iterations_ = 15000;
            p.resolution_ = 2048;
            p.scene_scale_ = BASEGS_SCENE_SCALE_AUTO;
            p.data_device_ = BASEGS_DATA_DEVICE_GPU;
            p.antialiasing_ = true;
            p.densify_from_iter_ = 500;
            p.densify_until_iter_ = 12000;
            p.densification_interval_ = 100;
            p.densify_grad_threshold_ = 0.0008f;
            p.opacity_reset_interval_ = 3000;
            p.lambda_dssim_ = 0.15f;
            p.sh_degree_ = 3;
            return p;
        }

        inline basegs_params_s GetBaseGsParamsHigh()
        {
            basegs_params_s p;
            p.iterations_ = 30000;
            p.resolution_ = 4096;
            p.scene_scale_ = BASEGS_SCENE_SCALE_LARGE;
            p.data_device_ = BASEGS_DATA_DEVICE_GPU;
            p.antialiasing_ = true;
            p.densify_from_iter_ = 500;
            p.densify_until_iter_ = 25000;
            p.densification_interval_ = 100;
            p.densify_grad_threshold_ = 0.0002f;
            p.opacity_reset_interval_ = 3000;
            p.lambda_dssim_ = 0.2f;
            p.sh_degree_ = 3;
            return p;
        }

        inline basegs_params_s GetBaseGsParamsPreset(basegs_quality_mode_e mode)
        {
            switch (mode)
            {
            case BASEGS_QUALITY_FAST:
                return GetBaseGsParamsFast();
            case BASEGS_QUALITY_HIGH:
                return GetBaseGsParamsHigh();
            case BASEGS_QUALITY_BALANCED:
            default:
                return GetBaseGsParamsBalanced();
            }
        }


        
        struct production_advance_opt_s
        {
            srs_s srs_;
            
            
            std::tuple<bool, bool, Eigen::Vector3d, Eigen::Vector3d> auto_custom_origin_;
            
            void ResetOrigin(const srs_s& oldsrs, srs_s& newsrs)
            {
                
                
                Eigen::Vector3d origin(0, 0, 0);
                Eigen::Vector2d lonlat = CoordinateDescriptor::GetLatLonFromENUDefinition(oldsrs.definition);
                
                double x = lonlat.x();
                double y = lonlat.y();
                double z = 0.0;
                CoordinateTransformer::Transform(1, &x, &y, &z, oldsrs.definition, newsrs.definition);
                std::get<2>(auto_custom_origin_) = Eigen::Vector3d(x, y, z);
                std::get<3>(auto_custom_origin_) = Eigen::Vector3d(x, y, z);
            }
            
        };
        enum geometric_level_e
        {
            GEO_LEVEL_UNKNOWN,
            GEO_LEVEL_EXTRA,
            GEO_LEVEL_H,
            GEO_LEVEL_M,
            

           
        };



        enum production_format_e
        {
            
            PRODUCTION_FORMAT_UNKNOWN,
            PRODUCTION_MESH_FORMAT_OSGB = 1,
            PRODUCTION_MESH_FORMAT_OBJ = 1 << 2,
            PRODUCTION_MESH_FORMAT_3DTILES = 1 << 3,
            PRODUCTION_MESH_FORMAT_PLY = 1 << 4,
            
            PRODUCTION_POINTCLOUD_FORMAT_OSGB = 1 << 5,
            PRODUCTION_POINTCLOUD_FORMAT_PLY = 1 << 6,
            PRODUCTION_POINTCLOUD_FORMAT_LAS = 1 << 7,
            PRODUCTION_4D_FORMAT_TDOMDSM = 1 << 8,
            PRODUCTION_4D_FORMAT_RAPIDTDOMDSM = 1 << 9,
            PRODUCTION_4D_FORMAT_MESHTDOMDSM = 1 << 10,
            PRODUCTION_4D_FORMAT_FASTMOSAIC = 1 << 11,
            PRODUCTION_POINTCLOUD_GDGS = 1 << 12,
            PRODUCTION_POINTCLOUD_BASEGS = 1 << 13,
            PRODUCTION_MESH_LOD = PRODUCTION_MESH_FORMAT_OSGB | PRODUCTION_MESH_FORMAT_3DTILES,
            PRODUCTION_MESH = PRODUCTION_MESH_LOD | PRODUCTION_MESH_FORMAT_OBJ | PRODUCTION_MESH_FORMAT_PLY,
            PRODUCTION_POINTCLOUD = PRODUCTION_POINTCLOUD_FORMAT_OSGB | PRODUCTION_POINTCLOUD_FORMAT_PLY | PRODUCTION_POINTCLOUD_FORMAT_LAS,
            PRODUCTION_4D = PRODUCTION_4D_FORMAT_TDOMDSM | PRODUCTION_4D_FORMAT_RAPIDTDOMDSM | PRODUCTION_4D_FORMAT_MESHTDOMDSM,
            PRODUCTION_RAPID = PRODUCTION_4D_FORMAT_FASTMOSAIC | PRODUCTION_4D_FORMAT_RAPIDTDOMDSM,
        };
        static std::map<  std::string, production_format_e > StringForProductionFormat =
        {
            {"Gauss Splatting(GD)",PRODUCTION_POINTCLOUD_GDGS},
            {"Gauss Splatting(Base)",PRODUCTION_POINTCLOUD_BASEGS},
            {"Mesh OSGB",PRODUCTION_MESH_FORMAT_OSGB},
            {"Point Cloud OSGB",PRODUCTION_POINTCLOUD_FORMAT_OSGB},
            {"Mesh  OBJ", PRODUCTION_MESH_FORMAT_OBJ},
            {"Mesh Cesium 3D Tiles",PRODUCTION_MESH_FORMAT_3DTILES},
            {"Mesh PLY" ,PRODUCTION_MESH_FORMAT_PLY},
            {"Point Cloud PLY" ,PRODUCTION_POINTCLOUD_FORMAT_PLY},
            {"Point Cloud LAS",PRODUCTION_POINTCLOUD_FORMAT_LAS},
            {"TDOM/DSM",PRODUCTION_4D_FORMAT_TDOMDSM},
            {"RAPIDTDOM/DSM",PRODUCTION_4D_FORMAT_RAPIDTDOMDSM},
            
             {"MESH TDOM/DSM",PRODUCTION_4D_FORMAT_MESHTDOMDSM},
             {"FAST MOSAIC",PRODUCTION_4D_FORMAT_FASTMOSAIC},
        };

        static std::map<  production_format_e, std::string> ProductionFormatStringToShow =
        {
            {PRODUCTION_POINTCLOUD_GDGS,"Gauss Splatting(GD)"},
            {PRODUCTION_POINTCLOUD_BASEGS,"Gauss Splatting(Base)"},
            {PRODUCTION_MESH_FORMAT_OSGB,"Mesh OSGB"},
            {PRODUCTION_POINTCLOUD_FORMAT_OSGB,"Point Cloud OSGB"},
            { PRODUCTION_MESH_FORMAT_OBJ,"Mesh  OBJ"},
            {PRODUCTION_MESH_FORMAT_3DTILES,"Mesh Cesium 3D Tiles"},
            {PRODUCTION_MESH_FORMAT_PLY,"Mesh PLY" },
            {PRODUCTION_POINTCLOUD_FORMAT_PLY,"Point Cloud PLY" },
            {PRODUCTION_POINTCLOUD_FORMAT_LAS,"Point Cloud LAS"},
            {PRODUCTION_4D_FORMAT_TDOMDSM,"TDOM/DSM"},
            {PRODUCTION_4D_FORMAT_RAPIDTDOMDSM,"RAPIDTDOM/DSM"},
            
              {PRODUCTION_4D_FORMAT_MESHTDOMDSM,"MESH TDOM/DSM"},
              {PRODUCTION_4D_FORMAT_FASTMOSAIC,"FAST MOSAIC"},
        };

        static std::map<  production_format_e, std::string> ProductionFormatStringToProcessing =
        {
            {PRODUCTION_POINTCLOUD_GDGS,"POINTCLOUDGDGS"},
            {PRODUCTION_POINTCLOUD_BASEGS,"POINTCLOUDBASEGS"},
            {PRODUCTION_MESH_FORMAT_OSGB,"MESHOSGB"},
            {PRODUCTION_POINTCLOUD_FORMAT_OSGB,"POINTCLOUDOSGB"},
            { PRODUCTION_MESH_FORMAT_OBJ,"MESHOBJ"},
            {PRODUCTION_MESH_FORMAT_3DTILES,"MESH3DTILES"},
            {PRODUCTION_MESH_FORMAT_PLY,"MESHPLY" },
            {PRODUCTION_POINTCLOUD_FORMAT_PLY,"POINTCLOUDPLY" },
            {PRODUCTION_POINTCLOUD_FORMAT_LAS,"POINTCLOUDLAS"},
            {PRODUCTION_4D_FORMAT_TDOMDSM,"TDOMDSM"},
            {PRODUCTION_4D_FORMAT_RAPIDTDOMDSM,"RAPIDTDOMDSM"},
            
             {PRODUCTION_4D_FORMAT_MESHTDOMDSM,"MESHTDOMDSM"},
             {PRODUCTION_4D_FORMAT_FASTMOSAIC,"FASTMOSAIC"},
        };
        static std::map<  std::string, production_format_e > ProductionFormatStringFromProcessing =
        {
            {"POINTCLOUDGDGS",PRODUCTION_POINTCLOUD_GDGS},
            {"POINTCLOUDBASEGS",PRODUCTION_POINTCLOUD_BASEGS},
            {"MESHOSGB",PRODUCTION_MESH_FORMAT_OSGB},
            {"POINTCLOUDOSGB",PRODUCTION_POINTCLOUD_FORMAT_OSGB},
            { "MESHOBJ",PRODUCTION_MESH_FORMAT_OBJ},
            {"MESH3DTILES",PRODUCTION_MESH_FORMAT_3DTILES},
            {"MESHPLY",PRODUCTION_MESH_FORMAT_PLY },
            {"POINTCLOUDPLY",PRODUCTION_POINTCLOUD_FORMAT_PLY},
            {"POINTCLOUDLAS",PRODUCTION_POINTCLOUD_FORMAT_LAS},
            {"TDOMDSM",PRODUCTION_4D_FORMAT_TDOMDSM},
            {"RAPIDTDOMDSM",PRODUCTION_4D_FORMAT_RAPIDTDOMDSM},
            
             {"MESHTDOM/DSM",PRODUCTION_4D_FORMAT_MESHTDOMDSM},
               {"FASTMOSAIC",PRODUCTION_4D_FORMAT_FASTMOSAIC},
        };

        
        struct coordinate_descriptor_s
        {
            std::string definition_ = srs_s().definition;
            Eigen::Vector3d origin_ = { 0.,0.,0. };

            void CreateXml(std::string file)
            {
                pugi::xml_document doc;
                pugi::xml_node declaration_node = doc.append_child(pugi::node_declaration);

                declaration_node.append_attribute("version") = "1.0";
                declaration_node.append_attribute("encoding") = "utf-8";

                pugi::xml_node ModelMetadata = doc.append_child("ModelMetadata");
                ModelMetadata.append_attribute("version") = "1";

                if (CoordinateDescriptor::GetSRSFromDefinition(definition_).type != LOCAL)
                {
                    pugi::xml_node srs = ModelMetadata.append_child("SRS");
                    srs.append_child(pugi::node_pcdata).set_value(definition_.c_str());
                    pugi::xml_node srsorigin = ModelMetadata.append_child("SRSOrigin");
                    std::string originstr = std::to_string(origin_.x()) + "," +
                        std::to_string(origin_.y()) + "," + std::to_string(origin_.z());
                    srsorigin.append_child(pugi::node_pcdata).set_value(originstr.c_str());
                }
                bool saveSucceed = doc.save_file(file.c_str());
                if (!saveSucceed)
                {
                    LOG(ERROR) << "saving" + file + "  failed!";
                    return;
                }

            }

            void CreateJson(rapidjson::Value& jstr, rapidjson::Document& doc)
            {
                rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
                jstr.AddMember("definition", rapidjson::Value(definition_.c_str(), allocator), allocator);
                rapidjson::Value item(rapidjson::kArrayType);
                item.PushBack(origin_.x(), allocator);
                item.PushBack(origin_.y(), allocator);
                item.PushBack(origin_.z(), allocator);
                jstr.AddMember("origin", item, allocator);

            }
            void ParseJson(const rapidjson::Value& jstr)
            {
                if (jstr.HasMember("definition"))
                {
                    definition_ = jstr["definition"].GetString();
                }
                if (jstr.HasMember("origin"))
                {
                    auto originjson = jstr["origin"].GetArray();
                    origin_.x() = originjson[0].GetDouble();
                    origin_.y() = originjson[1].GetDouble();
                    origin_.z() = originjson[2].GetDouble();
                }
            }
        };



        struct constraint_info_s
        {
            enum constraint_type_e
            {
                CONSTRAINT_SURFACE,
            };
            struct polygon_info_s
            {
                int id_;
                std::string name_;
                ABBox3d box_;
                std::vector<Eigen::Vector3d> points_;


                bool ParsePoints(const std::string& file)
                {

                    std::ifstream in1 = File::OpenIfstreamUtf8(file, std::ios::in);
                    if (!in1.is_open())
                    {
                        return false;
                    }
                    points_.clear();
                    std::string line1;
                    std::string item1;
                    while (std::getline(in1, line1))
                    {

                        AI3D::CORE::String::StringTrim(&line1);

                        if (line1.empty() || line1[0] == '#') {
                            continue;
                        }

                        std::stringstream line_stream(line1);

                        std::getline(line_stream, item1, ' ');
                        double x = std::stoll(item1);

                        std::getline(line_stream, item1, ' ');
                        double y = std::stoll(item1);

                        std::getline(line_stream, item1, ' ');
                        double z = std::stoll(item1);
                        points_.push_back(Eigen::Vector3d{ x,y,z });

                    }

                    in1.close();
                    return true;
                }
                bool SerializePoints(const std::string& file)
                {
                    std::ofstream out = File::OpenOfstreamUtf8(file, std::ios::trunc | std::ios::out);
                    if (!out.is_open())
                    {
                        return false;
                    }

                    for (auto& iter : points_)
                    {
                        out.precision(17);
                        out << iter.x() << " " << iter.y() << " " << iter.z() << std::endl;
                    }
                    out.close();
                    return true;
                }

            };
            std::string name_;
            constraint_type_e type_;
            std::string descriptions_;
            std::vector<polygon_info_s> polygons_;
            
            void ReName(std::string name) { name_ = name; }
            void SavePolygonsDebug(const std::string& path)
            {
                std::string name = name_;
                int cnttoout = 0;
                for (auto& iter_p : polygons_)
                {
                    auto filename = path + "/" + std::to_string(cnttoout) + POLYGONEXT;
                    File::EnsureUnifySlash(filename);
                    iter_p.SerializePoints(filename);
                    cnttoout++;
                }

            }
            void SavePolygons(const std::string& path)
            {
                std::string name = name_;
                int cnttoout = 0;
                for (auto& iter_p : polygons_)
                {
                    auto filename = path + "/" + iter_p.name_ + POLYGONEXT;
                    File::EnsureUnifySlash(filename);
                    iter_p.SerializePoints(filename);
                    cnttoout++;
                }

            }

            void UpdatePolgonNames(int index)
            {
                if (index < 0)
                    return;
                std::string idxstr = std::to_string(index);
                int cnt = 0;
                for (auto& iter : polygons_)
                {
                    std::string polycntstr = std::to_string(cnt);
                    std::string namestr = "mesh" + idxstr + "(" + polycntstr + ")";
                    iter.name_ = namestr;
                    cnt++;
                }


            }
            void UpdatePolygonBoxes()
            {

                for (auto& iter : polygons_)
                {
                    iter.box_ = PointsToBox(iter.points_);
                    MakeBoundingBoxValid(iter.box_);
                }


            }
            

           
        };
        struct processing_settings_s
        {
            bool bdiscard_emptytiles_ = true;
            holefilling_policy_e hollfilling_ = holefilling_policy_e::HOLEFILL_ALL;
            Eigen::Vector3i  texture_fill_color_ = Eigen::Vector3i(DEFAULTCOLOR, DEFAULTCOLOR, DEFAULTCOLOR);
            geometric_level_e level_ = geometric_level_e::GEO_LEVEL_EXTRA;
            untexture_policy_e untex_policy_ = untexture_policy_e::UNTEX_COLOR_INPAITING;
            bool bcolorbalance_ = true;
            void CreateJson(rapidjson::Value& jstr, rapidjson::Document& doc)
            {
                rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
                jstr.AddMember("Geometric_Level", rapidjson::Value((int)level_), allocator);
                jstr.AddMember("ColorBalanced", rapidjson::Value(bcolorbalance_), allocator);
                jstr.AddMember("Untexture_Fill_Mode", rapidjson::Value((int)untex_policy_), allocator);
                if (untex_policy_ == untexture_policy_e::UNTEX_COLOR_FILLED)
                {

                    rapidjson::Value jscolor(rapidjson::kArrayType);
                    for (int n = 0; n < 3; n++)
                    {
                        jscolor.PushBack(texture_fill_color_[n], allocator);
                    }
                    jstr.AddMember("Texture_Fill_Color", jscolor, allocator);
                }
                jstr.AddMember("DiscardEmptyTiles", rapidjson::Value(bdiscard_emptytiles_), allocator);
                jstr.AddMember("HoleFillingMode", rapidjson::Value(int(hollfilling_)), allocator);

            }
            void ParseJson(rapidjson::Value& jstr)
            {
                if (jstr.HasMember("HoleFillingMode"))
                {
                    hollfilling_ = holefilling_policy_e(jstr["HoleFillingMode"].GetInt());
                }

                if (jstr.HasMember("Geometric_Level"))
                {
                    level_ = geometric_level_e(jstr["Geometric_Level"].GetInt());
                }

                if (jstr.HasMember("Untexture_Fill_Mode"))
                {
                    untex_policy_ = untexture_policy_e(jstr["Untexture_Fill_Mode"].GetInt());
                    if (untex_policy_ == untexture_policy_e::UNTEX_COLOR_FILLED)
                    {
                        if (jstr.HasMember("Texture_Fill_Color"))
                        {
                            rapidjson::Value& jscolor = jstr["Texture_Fill_Color"];
                            if (jscolor.Size() == 3)
                            {
                                for (int n = 0; n < 3; n++)
                                {
                                    texture_fill_color_[n] = jscolor[n].GetInt();
                                }
                            }
                        }
                        else
                        {
                            LOGE("mode is fill type,but there is no color values.");
                        }
                    }

                }



                if (jstr.HasMember("ColorBalanced"))
                {
                    bcolorbalance_ = (jstr["ColorBalanced"].GetBool());
                }
                if (jstr.HasMember("DiscardEmptyTiles"))
                {
                    bdiscard_emptytiles_ = (jstr["DiscardEmptyTiles"].GetBool());
                }
            }
            


        };


        
        struct production_option_s
        {

            std::string settings_str_ = "";

            production_format_e production_format_ = production_format_e::PRODUCTION_MESH_FORMAT_OSGB;
            std::string destination_ = "";
            std::vector<std::string> tiles_;
            
            std::string name_ = "";
            production_t id_ = kInvalidProductionId;
            coordinate_descriptor_s cs_;
            float avgresolution_ = -1;
            bool unit_ = 0;
            tiling_mode_e splitRule_; 

            tiling_mode_e getSplitRule()
            {
                return splitRule_;
            }


            std::string GetFormatString()
            {
                return ProductionFormatStringToShow.at(production_format_);
            }

            bool HasSrs()
            {
                srs_s srs = CoordinateDescriptor::GetSRSFromDefinition(cs_.definition_);
                return srs.type == LOCAL;

            }

            void CreateJson(rapidjson::Value& jstr, rapidjson::Document& doc)
            {

                rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
                if (!settings_str_.empty())
                {
                    rapidjson::Value aparsejstr(rapidjson::kStringType);
                    aparsejstr.SetString(settings_str_.c_str(), allocator);
                    jstr.AddMember("modelingsettings", aparsejstr, allocator);
                }
                

                jstr.AddMember("production_format", rapidjson::Value(ProductionFormatStringToProcessing.at(production_format_).c_str(), allocator), allocator);
                if (destination_ != "")
                {
                    jstr.AddMember("destination", rapidjson::Value(destination_.c_str(), allocator), allocator); // was: GBK2UTF8
                }
                
                if (name_ != "")
                {
                    jstr.AddMember("name", rapidjson::Value(name_.c_str(), allocator), allocator); // was: GBK2UTF8
                }
                

                rapidjson::Value tilejson(rapidjson::kArrayType);
                for (auto& iter : tiles_)
                {
                    tilejson.PushBack(rapidjson::Value(iter.c_str(), allocator), allocator);
                }
                jstr.AddMember("tiles", tilejson, allocator);
            }

            void ParseJson(const rapidjson::Value& jstr)
            {
                if (jstr.HasMember("modelingsettings"))
                {
                    settings_str_ = jstr["modelingsettings"].GetString();
                    rapidjson::Document doc;
                    if (doc.Parse(settings_str_.data()).HasParseError())
                    {
                        LOGE("Parse model setting ERROR!");
                        return;
                    }
                    
                    if (doc.HasMember("srs_definition"))
                    {
                        auto value = doc["srs_definition"].GetString();

                        cs_.definition_ = value;
                    }
                    if (doc.HasMember("coordinate_origin"))
                    {
                        auto value = doc["coordinate_origin"].GetArray();
                        if (value.Size() != 3)
                        {
                            return;
                        }
                        Eigen::Vector3d xyz;
                        xyz.x() = value[0].GetDouble();
                        xyz.y() = value[1].GetDouble();
                        xyz.z() = value[2].GetDouble();
                        cs_.origin_ = xyz;

                    }

                }

                

                if (jstr.HasMember("production_format"))
                {
                    auto formatstr = (jstr["production_format"].GetString());
                    if (ProductionFormatStringFromProcessing.count(formatstr))
                    {
                        production_format_ = ProductionFormatStringFromProcessing.at(formatstr);
                    }
                }
                if (jstr.HasMember("srs"))
                {
                    cs_.ParseJson(jstr["srs"]);
                }
                if (jstr.HasMember("name"))
                {
                  // name_ = UTF82GBK(jstr["name"].GetString());
                   name_ = jstr["name"].GetString();

                }
                if (jstr.HasMember("destination"))
                {
                  // destination_ = UTF82GBK(jstr["destination"].GetString());
                   destination_ = jstr["destination"].GetString();

                }


                if (jstr.HasMember("tiles"))
                {
                    auto tilesjson = jstr["tiles"].GetArray();
                    for (unsigned i = 0; i < tilesjson.Size(); i++)
                    {
                        tiles_.push_back(tilesjson[i].GetString());
                    }
                }

            }

        };

        struct production_tileinfo_s
        {
            std::string name_;
            std::string jobstr_;
            jobsta_e status_;
            
        };





        
        struct ProductionOptions
        {

            
            production_option_s    production_settings_;
            
            std::string item_path_ = "";
            std::string job_;
            std::string project_path_ = "";
            ABBox3f tilebb_;
            
            
            std::string tiling_srs_def_;
            
          
            
            std::vector < std::vector<Eigen::Vector2d>> boundary_custom_;
            
             
            std::string function_ = "RunReconstruction";
            int taskid_ = 0;
            int sdebug_ = 1;
            


            processing_settings_s process_settings_;

            bool saveBin(const std::string& file) {
                std::ofstream out = File::OpenOfstreamUtf8(file, std::ios::binary);
                
                if (!out.is_open()) {
                    LOGE("Save taskdef bin failed!");
                    return false;
                }
                SPTaskInfoFile spTaskInfoFile;
                spTaskInfoFile.hasATParam = false;
                spTaskInfoFile.hasRecParam = true;      
                std::string blockItem = item_path_;
                std::string projectfile = project_path_;
                spTaskInfoFile.jobName = job_;
#ifdef WIN32
                // blockItem = GBK2UTF8(blockItem);
                // projectfile = GBK2UTF8(projectfile);
#endif 
                spTaskInfoFile.blockItem = blockItem;
                spTaskInfoFile.projectfile = projectfile;
                spTaskInfoFile.sdebug = sdebug_;
                spTaskInfoFile.ROISrs = tiling_srs_def_;

                spTaskInfoFile.hasGlobalBBox = false;
                spTaskInfoFile.hasROI = false;
                spTaskInfoFile.reconData.Geometric_Level = (int)(process_settings_.level_);
                spTaskInfoFile.reconData.ColorBalanced = process_settings_.bcolorbalance_;
                int untexPolicy = (int)(process_settings_.untex_policy_);
                spTaskInfoFile.reconData.Untexture_Fill_Mode = untexPolicy;
                if ((untexture_policy_e)untexPolicy == untexture_policy_e::UNTEX_COLOR_FILLED)
                {
                    for (int n = 0; n < 3; n++)
                    {
                        spTaskInfoFile.reconData.Texture_Fill_Color[n] = process_settings_.texture_fill_color_[n];
                    }

                }
                spTaskInfoFile.reconData.DiscardEmptyTiles = process_settings_.bdiscard_emptytiles_;
                spTaskInfoFile.reconData.HoleFillingMode = (int)(process_settings_.hollfilling_);
                spTaskInfoFile.reconData.productionVec.clear();
                ProductionData productionData;
                productionData.modelingsettings = production_settings_.settings_str_;
                productionData.production_format = ProductionFormatStringToProcessing.at(production_settings_.production_format_);
                productionData.destination = production_settings_.destination_;
                std::string productionName = production_settings_.name_;
#ifdef WIN32
                // productionName = GBK2UTF8(productionName);
#endif 
                productionData.name = productionName;
                productionData.tiles.clear();
                for (auto& iter : production_settings_.tiles_)
                {
                    productionData.tiles.push_back(iter);
                }
                productionData.tileSize = production_settings_.tiles_.size();
                spTaskInfoFile.reconData.productionVec.push_back(productionData);
                spTaskInfoFile.reconData.productionNum = spTaskInfoFile.reconData.productionVec.size();
                spTaskInfoFile.reconData.hasBoundary = false;
                if (!boundary_custom_.empty()) {
                    spTaskInfoFile.reconData.hasBoundary = true;
                    spTaskInfoFile.reconData.boundary_custom_.clear();
                    spTaskInfoFile.reconData.boundary_level1_size = boundary_custom_.size();
                    for (int index = 0; index < boundary_custom_.size(); index++)
                    {
                        std::vector< std::vector<double> > leve2;
                        spTaskInfoFile.reconData.boundary_level2_size = boundary_custom_[index].size();
                        for (int indexj = 0; indexj < boundary_custom_[index].size(); indexj++)
                        {
                            auto boundary = boundary_custom_[index][indexj];
                            std::vector<double> leve3;
                            leve3.push_back(boundary.x());
                            leve3.push_back(boundary.y());
                            leve2.push_back(leve3);
                        }
                        spTaskInfoFile.reconData.boundary_custom_.push_back(leve2);
                    }
                }
                spTaskInfoFile.hasGlobalBBox = false;
                if (!tilebb_.isEmpty())
                {
                    
                    spTaskInfoFile.reconData.boundingbox_custom.hasBBbox = true;
                    bbox_s bb(tilebb_.cast<double>());
                    
                    spTaskInfoFile.reconData.boundingbox_custom.max[0] = bb.xmax_;
                    spTaskInfoFile.reconData.boundingbox_custom.max[1] = bb.ymax_;
                    spTaskInfoFile.reconData.boundingbox_custom.max[2] = bb.zmax_;
                    spTaskInfoFile.reconData.boundingbox_custom.min[0] = bb.xmin_;
                    spTaskInfoFile.reconData.boundingbox_custom.min[1] = bb.ymin_;
                    spTaskInfoFile.reconData.boundingbox_custom.min[2] = bb.zmin_;
                }
                else {
                    LOGE("===========no bbox");
                }

                std::string firstId = "0";
                std::string firstTask = TASK_DEF_BIN_PREFIX + firstId;

                spTaskInfoFile.taskMetaData.id = taskid_;
                spTaskInfoFile.taskMetaData.msg = "Reconstruction";
                spTaskInfoFile.taskMetaData.name = firstTask;
                spTaskInfoFile.taskMetaData.type = 4;
                spTaskInfoFile.taskMetaData.functionName = "RunReconstruction";

                spTaskInfoFile.Serialize(out);
                out.close();
                return true;
            }

            bool save(const std::string& file)
            {
                std::string json_str;
                rapidjson::Document document;
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

                document.SetObject();

                rapidjson::Document::AllocatorType& allocator = document.GetAllocator();



                if (project_path_ != "")
                {
                    project_path_ = File::EnsureUnifySlash(project_path_);
                    document.AddMember("projectPath", rapidjson::Value(project_path_.c_str(), allocator), allocator); // was: GBK2UTF8
                }

                if (item_path_ != "")
                {
                    document.AddMember("blockItem", rapidjson::Value(item_path_.c_str(), allocator), allocator); // was: GBK2UTF8
                }

                if (job_ != "")
                {
                    document.AddMember("job", rapidjson::Value(job_.c_str(), allocator), allocator); // was: GBK2UTF8
                }
                
                 

               
                rapidjson::Value settingsjson(rapidjson::kObjectType);
                process_settings_.CreateJson(settingsjson, document);
                document.AddMember("process_settings", settingsjson, allocator);

                {
                    {
                        
                        
                        
                        

                        rapidjson::Value val(rapidjson::kObjectType);
                        std::string firstId = "0";
                        std::string firstTask = TASK_DEF_BIN_PREFIX + firstId;
                        val.AddMember("id", rapidjson::Value(taskid_), allocator);
                        val.AddMember("msg", rapidjson::Value("Reconstruction", allocator), allocator);
                        val.AddMember("name", rapidjson::Value(firstTask.c_str(), allocator), allocator);
                        val.AddMember("type", rapidjson::Value(4), allocator);


                        val.AddMember("function", rapidjson::Value("RunReconstruction", allocator), allocator);

                        document.AddMember("meta_data", val, allocator);
                    }
                }

                document.AddMember("sdebug", rapidjson::Value(sdebug_), allocator);

                
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
                        document.AddMember("boundary", boundarysjson, allocator);
                    }
                }
                rapidjson::Value prosettingsjson(rapidjson::kObjectType);
                production_settings_.CreateJson(prosettingsjson, document);
                document.AddMember("production_settings", prosettingsjson, allocator);


                if (!tilebb_.isEmpty())
                {
                    bbox_s bb(tilebb_.cast<double>());
                    rapidjson::Value bbjson(rapidjson::kObjectType);
                    bb.CreateJson(bbjson, document);
                    document.AddMember("bbox", bbjson, allocator);
                }
                


                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 

                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 
                 

                 
                 
                 
                 
                 
                 
                 
                 
                 
                 

                 

                 
                 


                 
                 
                 
                 
                document.AddMember("ROISrs", rapidjson::Value(tiling_srs_def_.c_str(), allocator), allocator);
                document.Accept(writer);
                json_str = buffer.GetString();
                std::ofstream ofs = File::OpenOfstreamUtf8(file, std::ios::out);
                if (ofs.fail())
                    return false;

                ofs << json_str;
                ofs.close();
                return true;

            }
            bool load(const std::string& file_path)
            {

                std::string blkcontent;

                bool ret = RapidJsonCore::ReadFile(file_path, blkcontent);
                if (!ret)
                {
                    LOGE(String::StringPrintf("File: %s was Read Error", file_path));
                    return false;
                }

                rapidjson::Document doc_blk;

                if (doc_blk.Parse(blkcontent.data()).HasParseError())
                {
                    LOGE(String::StringPrintf("%s :parse block file  error!", file_path));
                    return false;
                }

                if (!doc_blk.IsObject())
                {
                    LOGE("Parse block file error!");
                    return false;
                }



                if (doc_blk.HasMember("blockItem"))
                {
                  // item_path_ = UTF82GBK(doc_blk["blockItem"].GetString());
                   item_path_ = doc_blk["blockItem"].GetString();

                }
                


                if (doc_blk.HasMember("projectPath"))
                {
                  // project_path_ = UTF82GBK(doc_blk["projectPath"].GetString());
                   project_path_ = doc_blk["projectPath"].GetString();
                    project_path_ = File::EnsureUnifySlash(project_path_);

                }
                if (doc_blk.HasMember("job"))
                {
                  // job_ = UTF82GBK(doc_blk["job"].GetString());
                   job_ = doc_blk["job"].GetString();

                }
                if (doc_blk.HasMember("sdebug"))
                {
                    sdebug_ = (doc_blk["sdebug"].GetInt());

                }

                if (doc_blk.HasMember("bbox"))
                {
                    bbox_s bb;
                    bb.ParseJson(doc_blk["bbox"]);
                    tilebb_ = bb.toABBox3d().cast<float>();

                }
                if (doc_blk.HasMember("boundary"))
                {
                    rapidjson::Value& boundaryjson1 = doc_blk["boundary"];
                    for (auto& iter1 : boundaryjson1.GetArray())
                    {
                        rapidjson::Value& boundaryjson2 = iter1;
                        std::vector<Eigen::Vector2d> bd(boundaryjson2.Size());
                        for (unsigned index = 0; index < boundaryjson2.Size(); index++)
                        {

                            auto str = boundaryjson2[index].GetString();
                            auto strs = AI3D::CORE::String::StringSplit(str, ",");
                            if (strs.size() != 2)
                            {
                                return false;
                            }

                            bd[index][0] = std::atof(strs[0].c_str());
                            bd[index][1] = std::atof(strs[1].c_str());
                        }
                        boundary_custom_.push_back(bd);
                    }
                }
                

                if (doc_blk.HasMember("production_settings"))
                {

                    production_settings_.ParseJson(doc_blk["production_settings"]);


                }

                return true;
            }
        };
    }

}

#endif