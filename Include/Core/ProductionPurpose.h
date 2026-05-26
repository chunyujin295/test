#ifndef _AI3D_CORE_PRODUCTIONPURPOSE_H_
#define _AI3D_CORE_PRODUCTIONPURPOSE_H_
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <mutex>
#include <Eigen/Core>
#include <fstream>
#include <Constants.h>
#include <omp.h>

#include <iostream>
#include <map>
#include <utility>
#include <string>
#include <functional>
#include <memory>
#include  "Core/ReconstructionOptions.h"
#include "Core/CoordinateSystem.h"
namespace AI3D
{
    namespace CORE
    {
        

        struct texture_option_s
        {
            texture_color_source_e color_source_;
            texture_compression_quality_e quality_;
            int maximum_texture_size_ = 2048;
            bool texturesharpning_ = false;

        };
        
        
        

        enum productionpurpose_e
        {
            
            PURPOSE_MESH3D,
            PURPOSE_POINTCLOUD,
            PURPOSE_DSMTDOM,
            PURPOSE_RETOUCHING,
        };

      

        class AI3D_API ProductionPurpose
        {
        public:
            ProductionPurpose() {};
            virtual void MakeFormatOptions() = 0;
            
            ProductionPurpose(production_option_s& options) 
            {
                global_options_ = options;
            };
        protected:
            production_option_s  global_options_;
        };
        
        
        

        class AI3D_API MeshBasePurpose :public ProductionPurpose
        {
        public:
            virtual void MakeFormatOptions()
            {
                bool bhastexture = true;
                bool texturechecked = true;

                opt_.texture_opt_ =std::make_pair(bhastexture, texturechecked) ;
                
            };
           

            struct meshbase_options_s
            {
                std::tuple<bool, bool> texture_opt_;
                
            };
        private:
            meshbase_options_s opt_;
            
        };

        

        class AI3D_API Mesh3DPurpose :public ProductionPurpose
        {
        public:
            Mesh3DPurpose() {};
            Mesh3DPurpose(production_option_s  global_options) 
            {
                 global_options_ = global_options;
            };
            virtual void MakeFormatOptions()
            {
                std::cout << global_options_.cs_.definition_ << std::endl;;
                opt_.formatopt_ = FormatOptions(mesh3d_format_e::MESH3D_FORMAT_OSGB);
               

            }
            virtual void MakeSrsOptions()
            {
                if (opt_.formatopt_.format_ == mesh3d_format_e::MESH3D_FORMAT_3DTILE)
                {
                    

                }
               

            }
           

            struct lod_option_s
            {
                lod_option_s() {};
                mesh3d_lod_type_e lod_type_;
                lod_scope_mode_e lod_scope_mode_;
                double node_size_;
            };
            struct mesh3d_formatoptions_s
            {
                mesh3d_formatoptions_s() {};
                mesh3d_format_e  format_;
                std::tuple<bool,bool,lod_option_s> lod_opt_;
                std::tuple<bool,bool,texture_option_s> texture_opt_;
                std::tuple<bool, bool, int> skirt_opt_;
                std::tuple<bool, bool, double> overlap_opt_;
                mesh_tileoverlap_mode_e overlap_mode_;
                production_advance_opt_s srs_opt_;
                
            };


            struct mesh3d_options_s
            {
                mesh3d_options_s() {};
                mesh3d_formatoptions_s formatopt_;
               
                
            };

           
   
        
           
            const mesh3d_options_s GetOptions() const
            {
                return opt_;
            }
            mesh3d_options_s& GetOptionsMutual()
            {
                return opt_;
            }
            void SetOptions(const mesh3d_options_s& opt)
            {
                opt_ = opt;
            }
          
            
            std::tuple<mesh3d_format_e,
                std::vector<std::pair<mesh3d_format_e, bool>>> GetFormatPolicy(int mode);
        

           static  mesh3d_formatoptions_s FormatOptions(mesh3d_format_e object);
       
           
             static  std::tuple<texture_compression_quality_e,
                std::vector<std::pair<texture_compression_quality_e, bool>>> GetTextureCompressionPolicy(mesh3d_format_e object);
             static std::tuple<mesh3d_lod_type_e,
                std::vector<std::pair<mesh3d_lod_type_e, bool>>> GetLodTypePolicy(mesh3d_format_e object);
            static std::tuple<lod_scope_mode_e,
                std::vector<std::pair<lod_scope_mode_e, bool>>> GetScopeModePolicy(mesh3d_format_e object);
        private:
            mesh3d_options_s opt_;
        };

        class AI3D_API PointCloudPurpose :public ProductionPurpose
        {
        public:
            PointCloudPurpose() {};
            PointCloudPurpose(production_option_s  global_options)
            {
                global_options_ = global_options;
            };
            virtual void MakeFormatOptions()
            {

                srs_s srs = CoordinateDescriptor::GetSRSFromDefinition(global_options_.cs_.definition_);
                opt_.target_srs_ = srs;
                opt_.format_opt_.format_ = pointcloud_format_e::POINTCLOUD_FORMAT_LAS;
                opt_.format_opt_.sampling_unit_ = pointsampling_unit_e::POINT_SAMPLING_METER;
            }
          
          
            struct point_formatoption_s
            {
                pointcloud_format_e format_;
                pointsampling_unit_e sampling_unit_;
            };

            struct pointcloud_options_s
            {
                srs_s target_srs_;
                point_formatoption_s format_opt_;
              
            };

        public:
            const pointcloud_options_s GetOptions() const
            {
                return opt_;
            }
            pointcloud_options_s& GetOptionsMutual()
            {
                return opt_;
            }
            void SetOptions(const pointcloud_options_s& opt)
            {
                opt_ = opt;
            }

        private:
            pointcloud_options_s opt_;
        };

        class AI3D_API TDOMDSMPurpose :public ProductionPurpose
        {
        public:
            TDOMDSMPurpose() {};
          
            TDOMDSMPurpose(production_option_s  global_options)
            {
                global_options_ = global_options;
            };
            virtual void MakeFormatOptions()
            {
                FormatOptions();
              
            }
          

            struct dsmtdom_formatoption_s
            {
                tdom_mode_e mode_;
                dsm_format_e dsm_format_;
                tdom_format_e tdom_format_;
                float resolution_;
                int image_max_dim_ = 4096;
                
            };

           
            struct tdomdsm_options_s
            {
                srs_s target_srs_;
                dsmtdom_formatoption_s format_opt_;

            };
          
            tdomdsm_options_s FormatOptions();
            float  ComputeResolution();
        public:
            const tdomdsm_options_s GetOptions() const
            {
                return opt_;
            }
            tdomdsm_options_s& GetOptionsMutual()
            {
                return opt_;
            }
             void SetOptions(const tdomdsm_options_s& opt)
            {
                opt_ = opt;
            }

        private:
            tdomdsm_options_s opt_;
        };

        class AI3D_API RetouchingPurpose :public ProductionPurpose
        {
        public:
            struct retouching_option_s
            {
                texture_option_s texture_opts_;
                production_advance_opt_s advance_srs_;
            };
            RetouchingPurpose() {};

            RetouchingPurpose(production_option_s  global_options)
            {
                global_options_ = global_options;
            };
            virtual void MakeFormatOptions()
            {
                opt_.texture_opts_.color_source_ = texture_color_source_e::MESH_TEXTURE;
                opt_.texture_opts_.maximum_texture_size_ = 8192;
                opt_.texture_opts_.quality_ = texture_compression_quality_e::QUALITY_JPEG_75;
                opt_.texture_opts_.texturesharpning_ = true;
                srs_s srs = CoordinateDescriptor::GetSRSFromDefinition(global_options_.cs_.definition_);
                opt_.advance_srs_.srs_ = srs;
                opt_.advance_srs_.auto_custom_origin_ =std::make_tuple(true,true,Eigen::Vector3d(0,0,0), Eigen::Vector3d(0, 0, 0 ));
                
            }
            const retouching_option_s GetOptions() const
            {
                return opt_;
            }
            retouching_option_s& GetOptionsMutual()
            {
                return opt_;
            }
            void SetOptions(const retouching_option_s& opt)
            {
                opt_ = opt;
            }
           
        private:
            retouching_option_s opt_;
        };

        class AI3D_API ProductionDefinition
        {
        public:
           
            ProductionDefinition(production_option_s& options) ;
            const production_option_s GetOptions() const;
            production_option_s& GetOptionsMutual();
           

            const ProductionPurpose* GetPurpose() const;
            ProductionPurpose* GetPurposeMutual() ;
            void SetPurpose(ProductionPurpose* purpose);
            void SetProductionName(const std::string& name);
            void SetProductionDestination(const std::string& destination);
        private:
            production_option_s options_;
            ProductionPurpose* purpose_ = nullptr;
        };

        static std::map<std::string, std::function<std::shared_ptr<void>()>> class_map;
        static std::shared_ptr<void> ClassFromName(std::string str)
        {
            if (class_map.find(str) == class_map.end())
                return nullptr;
            return class_map[str]();
        }

        class Register {
        public:
            Register(std::string str, std::function<std::shared_ptr<void>()> func)
            {
                class_map.insert(std::make_pair(str, func));
            }
        };
#define REGISTER(classname) \
        class Register##classname { \
            public:                  \
            static std::shared_ptr<classname> instance(){  \
              return std::make_shared<classname>();       \
            }  \
        };\
       auto temp##classname= Register(std::string(#classname), Register##classname::instance);

       
    }
}
#endif