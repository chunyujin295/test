

#include "Core/ProductionPurpose.h"

namespace AI3D
{
    namespace CORE
    {
        

        std::tuple<texture_compression_quality_e, std::vector<std::pair<texture_compression_quality_e, bool> > >
            Mesh3DPurpose::GetTextureCompressionPolicy(mesh3d_format_e object)
        {

            texture_compression_quality_e defaultcompress = texture_compression_quality_e::QUALITY_JPEG_75;
            std::vector<std::pair<texture_compression_quality_e, bool> > vecitems(QUALITY_JPEG_SIZE);
            for (int i = 0; i < QUALITY_JPEG_SIZE; i++)
            {
                vecitems[i].first = texture_compression_quality_e(i);
                vecitems[i].second = true;
            }
            if (object == mesh3d_format_e::MESH3D_FORMAT_OSGB)
            {

            }
            else if (object == mesh3d_format_e::MESH3D_FORMAT_3DTILE)
            {

            }
            else if (object == mesh3d_format_e::MESH3D_FORMAT_OBJ)
            {

            }
            else if (object == mesh3d_format_e::MESH3D_FORMAT_PLY)
            {

            }

            return std::make_tuple(defaultcompress, vecitems);
        }

        std::tuple<mesh3d_lod_type_e, std::vector<std::pair<mesh3d_lod_type_e, bool>>>
            Mesh3DPurpose::GetLodTypePolicy(mesh3d_format_e object)
        {
            mesh3d_lod_type_e defaultlodtype = mesh3d_lod_type_e::MESH3D_LOD_ADAPTIVETREE;
            std::vector<std::pair<mesh3d_lod_type_e, bool> > vecitems(QUALITY_JPEG_SIZE);

            if (object == mesh3d_format_e::MESH3D_FORMAT_OSGB)
            {
                for (int i = 0; i < MESH3D_LOD_TREESIZE; i++)
                {
                    vecitems[i].first =mesh3d_lod_type_e(i);
                    vecitems[i].second = true;
                }
            }
            else if (object == mesh3d_format_e::MESH3D_FORMAT_3DTILE)
            {
                for (int i = 0; i < MESH3D_LOD_TREESIZE; i++)
                {
                    vecitems[i].first = mesh3d_lod_type_e(i);
                    vecitems[i].second = false;
                }

            }
            else if (object == mesh3d_format_e::MESH3D_FORMAT_OBJ)
            {
                vecitems.clear();
            }
            else if (object == mesh3d_format_e::MESH3D_FORMAT_PLY)
            {
                vecitems.clear();
            }

            return std::make_tuple(defaultlodtype, vecitems);
        }
        std::tuple<lod_scope_mode_e, std::vector<std::pair<lod_scope_mode_e, bool>>>
            Mesh3DPurpose::GetScopeModePolicy(mesh3d_format_e object)
        {
            lod_scope_mode_e defaultscaopemode = lod_scope_mode_e::LOD_TILE_WISE;
            std::vector<std::pair<lod_scope_mode_e, bool> > vecitems(QUALITY_JPEG_SIZE);

            if (object == mesh3d_format_e::MESH3D_FORMAT_OSGB)
            {
                for (int i = 0; i < MESH3D_LOD_TREESIZE; i++)
                {
                    vecitems[i].first = lod_scope_mode_e(i);
                    vecitems[i].second = true;
                }
            }
            else if (object == mesh3d_format_e::MESH3D_FORMAT_3DTILE)
            {

                for (int i = 0; i < MESH3D_LOD_TREESIZE; i++)
                {
                    vecitems[i].first = lod_scope_mode_e(i);
                    vecitems[i].second = false;
                }

            }
            else if (object == mesh3d_format_e::MESH3D_FORMAT_OBJ)
            {
                vecitems.clear();
            }
            else if (object == mesh3d_format_e::MESH3D_FORMAT_PLY)
            {
                vecitems.clear();
            }

            return std::make_tuple(defaultscaopemode, vecitems);
        }




        Mesh3DPurpose::mesh3d_formatoptions_s Mesh3DPurpose::FormatOptions(mesh3d_format_e object)
        {
            Mesh3DPurpose::mesh3d_formatoptions_s info;

            bool  bhas_tex_opt = true;
            bool bhas_lod_opt = true;
            bool blod_checked = true;
            bool bhas_skirt_opt = true;
            bool bskirt_checked = true;
            bool bhas_overlap_opt = true;
            bool boverlap_checked = true;
            mesh3d_lod_type_e lod_type;
            lod_scope_mode_e lod_scope_mode;
            if (object == mesh3d_format_e::MESH3D_FORMAT_OSGB)
            {
                info.format_ = mesh3d_format_e::MESH3D_FORMAT_OSGB;


            }
            else if (object == mesh3d_format_e::MESH3D_FORMAT_3DTILE)
            {
                info.format_ = mesh3d_format_e::MESH3D_FORMAT_3DTILE;

            }
            else if (object == mesh3d_format_e::MESH3D_FORMAT_OBJ)
            {
                info.format_ = mesh3d_format_e::MESH3D_FORMAT_OBJ;
                bhas_lod_opt = false;



            }
            else if (object == mesh3d_format_e::MESH3D_FORMAT_PLY)
            {
                info.format_ = mesh3d_format_e::MESH3D_FORMAT_PLY;
                bhas_lod_opt = false;
            }

            auto textuple = GetTextureCompressionPolicy(object);
            
            if (bhas_lod_opt)
            {
                lod_option_s opt;
                auto lodtypetuple = GetLodTypePolicy(object);

                opt.lod_type_ = std::get<0>(lodtypetuple);
                auto scopemodetuple = GetScopeModePolicy(object);
                opt.lod_scope_mode_ = std::get<0>(scopemodetuple);
                info.lod_opt_ = std::make_tuple(bhas_lod_opt, blod_checked, opt);
            }
            return info;
        }
        float TDOMDSMPurpose::ComputeResolution()
        {
            return 0.0;
        }

        TDOMDSMPurpose::tdomdsm_options_s TDOMDSMPurpose::FormatOptions()
        {

            TDOMDSMPurpose::tdomdsm_options_s opts;
            opts.target_srs_ = CoordinateDescriptor::GetSRSFromDefinition(global_options_.cs_.definition_);
            opts.format_opt_.mode_ = tdom_mode_e::NORMAL;
            opts.format_opt_.dsm_format_ = dsm_format_e::DSM_FORMAT_TIFFGEOTIFF;
            opts.format_opt_.tdom_format_ = tdom_format_e::TDOM_FORMAT_TIFFGEOTIFF;
            
            opts.format_opt_.resolution_ = ComputeResolution();
            return opts;
        }

        
        ProductionDefinition::ProductionDefinition(production_option_s& options)
        {
            options_ = options;
            purpose_ = new Mesh3DPurpose(options);
            
        }
        const production_option_s ProductionDefinition::GetOptions() const
        {
            return options_;
        }
        production_option_s& ProductionDefinition::GetOptionsMutual()
        {
            return options_;
        }
        



        void ProductionDefinition::SetProductionName(const std::string& name)
        {
            options_.name_ = name;
        }
        void ProductionDefinition::SetProductionDestination(const std::string& destination)
        {
            options_.destination_ = destination;
        }
        const ProductionPurpose* ProductionDefinition::GetPurpose() const
        {
            return purpose_;
        }
        ProductionPurpose* ProductionDefinition::GetPurposeMutual()
        {
            return purpose_;
        }
        void ProductionDefinition::SetPurpose(ProductionPurpose* purpose)
        {
            purpose_ = purpose;
        }

        

    }
}
