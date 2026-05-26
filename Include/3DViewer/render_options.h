#ifndef COLMAP_SRC_UI_RENDER_OPTIONS_H_
#define COLMAP_SRC_UI_RENDER_OPTIONS_H_

#include <iostream>

namespace AI3D
{
    namespace GUI
    {


        struct item_selected_s
        {
            image_t selected_image_id = kInvalidImageId;
        };
        struct RenderOptions 
        {
            enum ProjectionType {
                PERSPECTIVE,
                ORTHOGRAPHIC,
            };

           
            
            int min_track_len = 2;

            
            double max_error = 2;

            
            int refresh_rate = 1;

            
            
            bool adapt_refresh_rate = true;

            
            bool image_connections = false;

            
            int projection_type = ProjectionType::PERSPECTIVE;

           
           
        };

    }
}

#endif  
