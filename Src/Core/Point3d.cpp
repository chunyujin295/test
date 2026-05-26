















#include "Core/Point3d.h"

namespace AI3D
{
	namespace CORE
	{
		
		
		Point3D::Point3D() : xyz_(-DBL_MAX, -DBL_MAX, -DBL_MAX), color_(0, 0, 0),
			pixel_rms_(kInvalidError), dist_rms_(kInvalidError),
			id_(kInvalidPoint3DId), name_("") {}
		Point3D::Point3D(const Eigen::Vector3d& xyz, const Eigen::Vector3i color) : 
			xyz_(xyz), color_(color),
			pixel_rms_(kInvalidError), dist_rms_(kInvalidError),
			id_(kInvalidPoint3DId),name_("") {}


		void Point3D::SetName(std::string name)
		{
			name_ = name;
		}
		const std::string Point3D::GetName() const
		{
			return name_;
		}
		std::string Point3D::GetNameMutual()
		{
			return name_;
		}
		
		void Point3D::SetId(point3D_t id)
		{
			id_ = id;
		}
		point3D_t Point3D::GetId() const
		{
			return id_;
		}
		point3D_t Point3D::GetIdMutual() 
		{
			return id_;
		}


		
		void Point3D::SetType(ptt_e type)
		{
			type_ = type;
		}
		

		double Point3D::GetPixelRMS() const
		{
			return pixel_rms_;
		}

		bool Point3D::HasPixelRMS() const
		{
			return pixel_rms_ != kInvalidError;
		}

		double  Point3D::GetDistRMS() const
		{
			return dist_rms_;
		};
		inline bool  Point3D::HasDistRMS() const
		{
			return dist_rms_ != kInvalidError;
			
		};
		void  Point3D::SetDistRMS(const double error)
		{
			dist_rms_ = error;
		}

		void Point3D::SetPixelRMS(const double rms)
		{
			pixel_rms_ = rms;
		}

		bool Point3D::GetStatus()const
		{
			return ischanged_;
		}
		bool &Point3D::GetStatusMutual()
		{
			return ischanged_;
		}

		void Point3D::SetStatus(bool ischanged)
		{
			ischanged_ = ischanged;
		}



		
		
		

		const Eigen::Vector3d& Point3D::GetXYZ() const
		{ 
			return xyz_; 
		}

		Eigen::Vector3d& Point3D::GetXYZMutual()
		{ 
			return xyz_; 
		}

		bool Point3D::HasXYZ()const
		{
			return xyz_.x() != -DBL_MAX && xyz_.y() != -DBL_MAX && xyz_.z() != -DBL_MAX;
		}
		double Point3D::GetXYZ(const size_t idx) const 
		{ 
			return xyz_(idx); 
		}

		double& Point3D::GetXYZMutual(const size_t idx)
		{
			return xyz_(idx); 
		}

		double Point3D::GetX() const
		{
			return xyz_.x(); 
		}

		double Point3D::GetY() const
		{
			return xyz_.y();
		}

		double Point3D::GetZ() const
		{
			return xyz_.z(); 
		}

		void Point3D::SetXYZ(const Eigen::Vector3d& xyz) 
		{
			xyz_ = xyz;
		}

		const Eigen::Vector3i& Point3D::GetColor() const
		{
			return color_; 
		}

		Eigen::Vector3i& Point3D::GetColorMutual()
		{
			return color_; 
		}

		int Point3D::GetColor(const size_t idx) const
		{
			return color_(idx); 
		}

		int& Point3D::GetColorMutual(const size_t idx)
		{
			return color_(idx); 
		}

		bool Point3D::HasElement()
		{
			return track_.GetElements().size() > 0;
		}

		bool Point3D::HasEstimatedXYZ()
		{
			if (estimated_xyz_.x() != -DBL_MAX && estimated_xyz_.y() != -DBL_MAX && estimated_xyz_.z() != -DBL_MAX)
			{
				return true;
			}
			return false;
		}

		void Point3D::SetColor(const Eigen::Vector3i& color)
		{
			color_ = color;
		}
		bool Point3D::IsValid()
		{
			return GetTrack().GetElements().size() >= VALIDTRIANGLENUM;
		}

		void Point3D::SetEstimatedXYZ(Eigen::Vector3d& xyz)
		{
			estimated_xyz_ = xyz;
		};
		Eigen::Vector3d& Point3D::GetEstimatedXYZMutual()
		{
			return estimated_xyz_;
		};
		const Eigen::Vector3d  Point3D::GetEstimatedXYZ()const
		{
			return estimated_xyz_;
		};
	

		const class Track& Point3D::GetTrack() const
		{
			return track_; 
		}

		const ptt_e Point3D::GetType() const
		{ 
			return type_;
		};
		ptt_e Point3D::GetTypeMutual() 
		{
			return type_; 
		};

		class Track& Point3D::GetTrackMutual()
		{
			return track_;
		}

		void Point3D::SetTrack(const class Track& track)
		{
			track_ = track;
		}
	}
} 
