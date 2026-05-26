
#ifndef _AI3D_CORE_POINT3D_H_
#define _AI3D_CORE_POINT3D_H_

#include <vector>

#include <Eigen/Core>

#include "Core/Track.h"
#include "Core/Logging.h"
#include "Core/Types.h"
#include<Constants.h>
namespace AI3D
{
	namespace CORE
	{
		
		class AI3D_API Point3D 
		{
		public:
			EIGEN_MAKE_ALIGNED_OPERATOR_NEW

				Point3D();
			Point3D(const Eigen::Vector3d& xyz, const Eigen::Vector3i color);

			
			inline const Eigen::Vector3d& GetXYZ() const;
			inline Eigen::Vector3d& GetXYZMutual();
			inline double GetXYZ(const size_t idx) const;
			inline double& GetXYZMutual(const size_t idx);
			inline double GetX() const;
			inline double GetY() const;
			inline double GetZ() const;
			inline void SetXYZ(const Eigen::Vector3d& xyz);
			bool HasXYZ()const;

			
			inline const Eigen::Vector3i& GetColor() const;
			inline Eigen::Vector3i& GetColorMutual();
			inline int GetColor(const size_t idx) const;
			inline int& GetColorMutual(const size_t idx);
			inline void SetColor(const Eigen::Vector3i& color);
			bool HasEstimatedXYZ();
			bool HasElement();
			bool IsValid();

			inline const class Track& GetTrack() const;
			inline class Track& GetTrackMutual();
			inline void SetTrack(const class Track& track);

			
			void SetName(std::string name);
			const std::string GetName() const;
			std::string GetNameMutual();
			
			void SetId(point3D_t id);
			point3D_t GetId() const;
			point3D_t GetIdMutual();
			
			void SetType(ptt_e type);
			const ptt_e GetType() const ;
			ptt_e GetTypeMutual() ;
			
			
			void SetEstimatedXYZ(Eigen::Vector3d& xyz);
			Eigen::Vector3d& GetEstimatedXYZMutual();
			const Eigen::Vector3d  GetEstimatedXYZ() const;

			double GetPixelRMS() const; 
			void SetPixelRMS(const double rms);
			inline bool HasPixelRMS() const;
			double GetDistRMS() const;
			inline bool HasDistRMS() const;
			void SetDistRMS(const double error);

			
			bool GetStatus()const;
			bool& GetStatusMutual();
			void SetStatus(bool ischanged);
			image_t image_for_userptguide_= kInvalidImageId;
		private:
			bool ischanged_ = false;
			
			Eigen::Vector3d xyz_{ -DBL_MAX,-DBL_MAX,-DBL_MAX };

			
			Eigen::Vector3i color_;

		
			Eigen::Vector3d estimated_xyz_{ -DBL_MAX,-DBL_MAX,-DBL_MAX };						

			
			class Track track_;
			
			point3D_t id_;
			std::string name_;

			
			double pixel_rms_; 
			
			double dist_rms_;
			ptt_e type_ = ptt_e::PT_NONE;

		};

	}
} 

EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION_CUSTOM(AI3D::CORE::Point3D);

#endif  
