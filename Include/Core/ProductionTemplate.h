
#ifndef _AI3D_CORE_PRODUCTIONTEMPLATE_H_
#define _AI3D_CORE_PRODUCTIONTEMPLATE_H_
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


namespace AI3D
{
    namespace CORE
    {
        enum pointcloud_format_e
        {
            POINTCLOUD_FORMAT_LAS,
        };

        enum pointsampling_unit_e
        {
            POINT_SAMPLING_PIXEL,
            POINT_SAMPLING_METER,
        };

        enum texture_color_source_e
        {
            MESH_TEXTURE,
            OPTIMIZE_TEXTURE,
        };
        enum texture_compression_quality_e
        {
            QUALITY_JPEG_100 = 0,
            QUALITY_JPEG_90,
            QUALITY_JPEG_75,
            QUALITY_JPEG_50,
            QUALITY_JPEG_SIZE,
        };

        enum mesh3d_format_e
        {
            MESH3D_FORMAT_OSGB = 0,
            MESH3D_FORMAT_3DTILE,
            MESH3D_FORMAT_PLY,
            MESH3D_FORMAT_OBJ,
            MESH3D_FORMAT_SIZE,
        };
        enum mesh3d_lod_type_e
        {
            
            MESH3D_LOD_ADAPTIVETREE = 0,
            MESH3D_LOD_QUADTREE ,
            MESH3D_LOD_TREESIZE,
        };

        enum lod_scope_mode_e
        {
            LOD_TILE_WISE,
            LOD_TILE_ACROSSTILE,
        };
        enum mesh_tileoverlap_mode_e
        {
            MESH3D_TILEOVERL_MODE_OVERLAP,
            MESH3D_SKIRT_MODE_SKIRT,
        };
        enum tdom_mode_e
        {
            NORMAL,
            RAPIDMOSAIC,
            LOW,
            FASTMOSAIC,
        };

        enum dsm_format_e
        {
            DSM_FORMAT_TIFFGEOTIFF,
            DSM_FORMAT_XYZ,
        };

        enum tdom_format_e
        {
            TDOM_FORMAT_TIFFGEOTIFF,
            TDOM_FORMAT_JPEG,
        };
        
        enum gs_scene_e
        {
            GS_SCENE_FLY,
            GS_SCENE_INDOOR,
            GS_SCENE_OBJECT,
        };

        enum gs_3d_format_e
        {
            GS_3D_FORMAT_PLY,
            GS_3D_FORMAT_SPLAT,
        };
       

        
       

            
        
        
        
        

        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        

        
        
        
        
        

        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        

        
        
        
        
        
        
        
        
        
        
        
        


        
        
        
        
        
        
        
        
        

        
        
        
        
        

        
        
        
        
        
        

        
        

        
        
        
        
        

        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        

        
        
        
        
        
        
        
        
        
        
        
        

        
        
        
        
        
        
        
        
        
        
        
        
        
        

        
        
        
        
        
        
        
        
        


        

        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        

        
        
        
        
        
        
        
        
        

        
        
        
        
        

        
        
        
        
        
        

        
        

        
        
        
        
        

        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        

        
        
        
        
        
        
        
        
        
        
        
        


        
        
        
        
        
        
        
        

        
        

        
        
        
        
        
        
        
        
        
        
        
        
        
        
        

        

        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        

        
        
        
        
        
        
        
        
        
        
        
        
        
        

        

        
        
        
        
        
        
        
        
        

        


        

        

        
        
        
        
        
        
        
        
        
        

        
        


        
        
        

        
        
        
        
        
        

        
        
        


        
        
        
        

        
        

        
        
        
        
        

        
        

        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        

        
        
        
        
        
        
        
        
        
        
        

        

    }
}
#endif