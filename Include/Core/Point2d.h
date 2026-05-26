
#ifndef _AI3D_CORE_POINT2D_H_
#define _AI3D_CORE_POINT2D_H_

#include <Eigen/Core>
#include <opencv2/opencv.hpp>
#include "Core/Alignment.h"
#include "Core/Types.h"
#include<Constants.h>
namespace AI3D
{
	namespace CORE
	{
		
		
		class AI3D_API Point2D 
		{
		public:
			EIGEN_MAKE_ALIGNED_OPERATOR_NEW

				Point2D();
			Point2D(const Eigen::Vector2d& xy, point3D_t point3D_id);

			
			inline const Eigen::Vector2d& GetXY() const;
			inline Eigen::Vector2d& GetXYMutual();
			inline double GetX() const;
			inline double GetY() const;
			inline void SetXY(const Eigen::Vector2d& xy);

			
			
			inline point3D_t GetPoint3DId() const;
			inline bool HasPoint3D() const;
			inline void SetPoint3DId(const point3D_t point3D_id);

			cv::Vec3b& GetColorMutual() { return color_; };
			const cv::Vec3b& GetColor() { return color_; };
		private:
			
			Eigen::Vector2d xy_;

			
			
			point3D_t point3D_id_;
			cv::Vec3b color_;
		};

	}
} 

EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION_CUSTOM(AI3D::CORE::Point2D)

#endif  
