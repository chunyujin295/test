#ifndef COLMAP_SRC_FEATURE_TYPES_H_
#define COLMAP_SRC_FEATURE_TYPES_H_

#include <vector>

#include <Eigen/Core>

#include "Core/Types.h"

namespace AI3D
{
	namespace GUI
	{

		struct FeatureKeypoint {
			FeatureKeypoint();
			FeatureKeypoint(const float x, const float y);
			FeatureKeypoint(const float x, const float y, const float scale,
				const float orientation);
			FeatureKeypoint(const float x, const float y, const float a11,
				const float a12, const float a21, const float a22);

			static FeatureKeypoint FromParameters(const float x, const float y,
				const float scale_x,
				const float scale_y,
				const float orientation,
				const float shear);

			
			void Rescale(const float scale);
			void Rescale(const float scale_x, const float scale_y);

			
			float ComputeScale() const;
			float ComputeScaleX() const;
			float ComputeScaleY() const;
			float ComputeOrientation() const;
			float ComputeShear() const;

			
			
			float x;
			float y;

			
			float a11;
			float a12;
			float a21;
			float a22;
		};

		typedef Eigen::Matrix<uint8_t, 1, Eigen::Dynamic, Eigen::RowMajor>
			FeatureDescriptor;

		struct FeatureMatch {
			
			point2D_t point2D_idx1 = kInvalidPoint2DIdx;
			
			point2D_t point2D_idx2 = kInvalidPoint2DIdx;
		};

		typedef std::vector<FeatureKeypoint> FeatureKeypoints;
		typedef Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>
			FeatureDescriptors;
		typedef std::vector<FeatureMatch> FeatureMatches;

		struct GroundControlPoint {
			GroundControlPoint();
			GroundControlPoint(const float x, const float y);
			GroundControlPoint(const float x, const float y, const double a11,
				const double a12, const double a13, const int index);

			float x;
			float y;

			
			double a11;
			double a12;
			double a13;
			int index;

		};
		typedef std::vector<GroundControlPoint> GCPPoints;

	} 
}

#endif  
