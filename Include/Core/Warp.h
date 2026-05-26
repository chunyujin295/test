#ifndef AI3D_CORE_WARP_H_
#define AI3D_CORE_WARP_H_

#include "Core/Camera.h"
#include "Core/Alignment.h"
#include "Core/Bitmap.h"

namespace AI3D {
    namespace CORE
    {
        


        void WarpImageBetweenCameras(const Camera& source_camera,
            const Camera& target_camera,
            const Bitmap& source_image, Bitmap* target_image);

        
        
        
        void WarpImageWithHomography(const Eigen::Matrix3d& H,
            const Bitmap& source_image, Bitmap* target_image);

        
        
        
        
        void WarpImageWithHomographyBetweenCameras(const Eigen::Matrix3d& H,
            const Camera& source_camera,
            const Camera& target_camera,
            const Bitmap& source_image,
            Bitmap* target_image);

        
        void ResampleImageBilinear(const float* data, const int rows, const int cols,
            const int new_rows, const int new_cols,
            float* resampled);

        
        
        

        
        
        
        
    }
}  

#endif  
