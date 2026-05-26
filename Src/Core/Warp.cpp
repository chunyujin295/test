#include "Core/Warp.h"


#include "Core/Logging.h"

namespace AI3D {
    namespace CORE
    {
        namespace {

            float GetPixelConstantBorder(const float* data, const int rows, const int cols,
                const int row, const int col) {
                if (row >= 0 && col >= 0 && row < rows && col < cols) {
                    return data[row * cols + col];
                }
                else {
                    return 0;
                }
            }

        }  

        void WarpImageBetweenCameras(const Camera& source_camera,
            const Camera& target_camera,
            const Bitmap& source_image, Bitmap* target_image) {
            CHECK_EQ(source_camera.GetWidth(), source_image.GetWidth());
            CHECK_EQ(source_camera.GetHeight(), source_image.GetHeight());
            CHECK_NOTNULL(target_image);

            target_image->Allocate(static_cast<int>(source_camera.GetWidth()),
                static_cast<int>(source_camera.GetHeight()),
                source_image.IsRGB());

            
            
            Camera scaled_target_camera = target_camera;
            if (target_camera.GetWidth() != source_camera.GetWidth() ||
                target_camera.GetHeight() != source_camera.GetHeight()) {
                scaled_target_camera.Rescale(source_camera.GetWidth(), source_camera.GetHeight());
            }

            Eigen::Vector2d image_point;
            for (int y = 0; y < target_image->GetHeight(); ++y) {
                image_point.y() = y + 0.5;
                for (int x = 0; x < target_image->GetWidth(); ++x) {
                    image_point.x() = x + 0.5;

                    
                    const Eigen::Vector2d world_point =
                        scaled_target_camera.ImageToWorld(image_point);
                    const Eigen::Vector2d source_point =
                        source_camera.WorldToImage(world_point);

                    BitmapColor<float> color;
                    if (source_image.InterpolateBilinear(source_point.x() - 0.5,
                        source_point.y() - 0.5, &color)) {
                        target_image->SetPixel(x, y, color.Cast<uint8_t>());
                    }
                    else {
                        target_image->SetPixel(x, y, BitmapColor<uint8_t>(0));
                    }
                }
            }

            if (target_camera.GetWidth() != source_camera.GetWidth() ||
                target_camera.GetHeight() != source_camera.GetHeight()) {
                target_image->Rescale(target_camera.GetWidth(), target_camera.GetHeight());
            }
        }

        void WarpImageWithHomography(const Eigen::Matrix3d& H,
            const Bitmap& source_image, Bitmap* target_image) {
            CHECK_NOTNULL(target_image);
            CHECK_GT(target_image->GetWidth(), 0);
            CHECK_GT(target_image->GetHeight(), 0);
            CHECK_EQ(source_image.IsRGB(), target_image->IsRGB());

            Eigen::Vector3d target_pixel(0, 0, 1);
            for (int y = 0; y < target_image->GetHeight(); ++y) {
                target_pixel.y() = y + 0.5;
                for (int x = 0; x < target_image->GetWidth(); ++x) {
                    target_pixel.x() = x + 0.5;

                    const Eigen::Vector2d source_pixel = (H * target_pixel).hnormalized();

                    BitmapColor<float> color;
                    if (source_image.InterpolateBilinear(source_pixel.x() - 0.5,
                        source_pixel.y() - 0.5, &color)) {
                        target_image->SetPixel(x, y, color.Cast<uint8_t>());
                    }
                    else {
                        target_image->SetPixel(x, y, BitmapColor<uint8_t>(0));
                    }
                }
            }
        }

        void WarpImageWithHomographyBetweenCameras(const Eigen::Matrix3d& H,
            const Camera& source_camera,
            const Camera& target_camera,
            const Bitmap& source_image,
            Bitmap* target_image) {
            CHECK_EQ(source_camera.GetWidth(), source_image.GetWidth());
            CHECK_EQ(source_camera.GetHeight(), source_image.GetHeight());
            CHECK_NOTNULL(target_image);

            target_image->Allocate(static_cast<int>(source_camera.GetWidth()),
                static_cast<int>(source_camera.GetHeight()),
                source_image.IsRGB());

            
            
            Camera scaled_target_camera = target_camera;
            if (target_camera.GetWidth() != source_camera.GetWidth() ||
                target_camera.GetHeight() != source_camera.GetHeight()) {
                scaled_target_camera.Rescale(source_camera.GetWidth(), source_camera.GetHeight());
            }

            Eigen::Vector3d image_point(0, 0, 1);
            for (int y = 0; y < target_image->GetHeight(); ++y) {
                image_point.y() = y + 0.5;
                for (int x = 0; x < target_image->GetWidth(); ++x) {
                    image_point.x() = x + 0.5;

                    
                    const Eigen::Vector3d warped_point = H * image_point;
                    const Eigen::Vector2d world_point =
                        target_camera.ImageToWorld(warped_point.hnormalized());
                    const Eigen::Vector2d source_point =
                        source_camera.WorldToImage(world_point);

                    BitmapColor<float> color;
                    if (source_image.InterpolateBilinear(source_point.x() - 0.5,
                        source_point.y() - 0.5, &color)) {
                        target_image->SetPixel(x, y, color.Cast<uint8_t>());
                    }
                    else {
                        target_image->SetPixel(x, y, BitmapColor<uint8_t>(0));
                    }
                }
            }

            if (target_camera.GetWidth() != source_camera.GetWidth() ||
                target_camera.GetHeight() != source_camera.GetHeight()) {
                target_image->Rescale(target_camera.GetWidth(), target_camera.GetHeight());
            }
        }

        void ResampleImageBilinear(const float* data, const int rows, const int cols,
            const int new_rows, const int new_cols,
            float* resampled) {
            CHECK_NOTNULL(data);
            CHECK_NOTNULL(resampled);
            CHECK_GT(rows, 0);
            CHECK_GT(cols, 0);
            CHECK_GT(new_rows, 0);
            CHECK_GT(new_cols, 0);

            const float scale_r = static_cast<float>(rows) / static_cast<float>(new_rows);
            const float scale_c = static_cast<float>(cols) / static_cast<float>(new_cols);

            for (int r = 0; r < new_rows; ++r) {
                const float r_i = (r + 0.5f) * scale_r - 0.5f;
                const int r_i_min = std::floor(r_i);
                const int r_i_max = r_i_min + 1;
                const float d_r_min = r_i - r_i_min;
                const float d_r_max = r_i_max - r_i;

                for (int c = 0; c < new_cols; ++c) {
                    const float c_i = (c + 0.5f) * scale_c - 0.5f;
                    const int c_i_min = std::floor(c_i);
                    const int c_i_max = c_i_min + 1;
                    const float d_c_min = c_i - c_i_min;
                    const float d_c_max = c_i_max - c_i;

                    
                    const float value1 =
                        d_c_max * GetPixelConstantBorder(data, rows, cols, r_i_min, c_i_min) +
                        d_c_min * GetPixelConstantBorder(data, rows, cols, r_i_min, c_i_max);
                    const float value2 =
                        d_c_max * GetPixelConstantBorder(data, rows, cols, r_i_max, c_i_min) +
                        d_c_min * GetPixelConstantBorder(data, rows, cols, r_i_max, c_i_max);

                    
                    resampled[r * new_cols + c] = d_r_max * value1 + d_r_min * value2;
                }
            }
        }

        
        
        
        
        
        
        
        
        
        

    
    
    
    
    
    
    
    
    
    
    

    
    

    
    
    
    
    

    
    

    
    
    

}

}  
