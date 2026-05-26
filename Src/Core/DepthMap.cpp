#include "Core/DepthMap.h"

#include "Core/Warp.h"
#include "Core/Math.h"

namespace AI3D {
	namespace CORE {

		DepthMap::DepthMap() : DepthMap(0, 0, -1.0f, -1.0f) {}

		DepthMap::DepthMap(const size_t width, const size_t height,
			const float depth_min, const float depth_max)
			: Mat<float>(width, height, 1),
			depth_min_(depth_min),
			depth_max_(depth_max) {}

		DepthMap::DepthMap(const Mat<float>& mat, const float depth_min,
			const float depth_max)
			: Mat<float>(mat.GetWidth(), mat.GetHeight(), mat.GetDepth()),
			depth_min_(depth_min),
			depth_max_(depth_max) {
			CHECK_EQ(mat.GetDepth(), 1);
			data_ = mat.GetData();
		}

		
		
		
		

		
		
		
		
		

		
		
		

		
		

		
		
		
		
		
		
		
		

		Bitmap DepthMap::ToBitmap(const float min_percentile,
			const float max_percentile) const {
			CHECK_GT(width_, 0);
			CHECK_GT(height_, 0);

			Bitmap bitmap;
			bitmap.Allocate(width_, height_, true);

			std::vector<float> valid_depths;
			valid_depths.reserve(data_.size());
			for (const float depth : data_) {
				if (depth > 0) {
					valid_depths.push_back(depth);
				}
			}

			if (valid_depths.empty()) {
				bitmap.Fill(BitmapColor<uint8_t>(0));
				return bitmap;
			}

			const float robust_depth_min = Percentile(valid_depths, min_percentile);
			const float robust_depth_max = Percentile(valid_depths, max_percentile);

			const float robust_depth_range = robust_depth_max - robust_depth_min;
			for (size_t y = 0; y < height_; ++y) {
				for (size_t x = 0; x < width_; ++x) {
					const float depth = Get(y, x);
					if (depth > 0) {
						const float robust_depth =
							std::max(robust_depth_min, std::min(robust_depth_max, depth));
						const float gray =
							(robust_depth - robust_depth_min) / robust_depth_range;
						const BitmapColor<float> color(255 * JetColormap::GetRed(gray),
							255 * JetColormap::GetGreen(gray),
							255 * JetColormap::GetBlue(gray));
						bitmap.SetPixel(x, y, color.Cast<uint8_t>());
					}
					else {
						
						bitmap.SetPixel(x, y, BitmapColor<uint8_t>(17,19,25));
					}
				}
			}

			return bitmap;
		}

	}  
}  
