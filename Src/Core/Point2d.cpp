















#include "Core/Point2d.h"

namespace AI3D
{
	namespace CORE
	{
		Point2D::Point2D()
			: xy_(Eigen::Vector2d::Zero()), point3D_id_(kInvalidPoint3DId) {}
		Point2D::Point2D(const Eigen::Vector2d& xy, point3D_t point3D_id)
			: xy_(xy), point3D_id_(point3D_id) {}

		



		const Eigen::Vector2d& Point2D::GetXY() const 
		{
			return xy_; 
		}

		Eigen::Vector2d& Point2D::GetXYMutual() 
		{
			return xy_; 
		}

		double Point2D::GetX() const
		{ 
			return xy_.x(); 
		}

		double Point2D::GetY() const
		{
			return xy_.y(); 
		}

		void Point2D::SetXY(const Eigen::Vector2d& xy) 
		{ 
			xy_ = xy; 
		}

		point3D_t Point2D::GetPoint3DId() const 
		{
			return point3D_id_; 
		}

		bool Point2D::HasPoint3D() const 
		{ 
			return point3D_id_ != kInvalidPoint3DId; 
		}

		void Point2D::SetPoint3DId(const point3D_t point3D_id) 
		{
			point3D_id_ = point3D_id;
		}
	}
} 
