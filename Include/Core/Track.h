#ifndef _AI3D_CORE_TRACK_H_
#define _AI3D_CORE_TRACK_H_

#include <vector>

#include "Core/Logging.h"
#include "Core/Types.h"
#include<Constants.h> 

namespace AI3D
{
	namespace CORE
	{
		
		struct AI3D_API TrackElement 
		{
			TrackElement();
			TrackElement(const image_t image_id, const point2D_t point2D_idx);
			TrackElement(const image_t image_id, const Eigen::Vector2d xy);
			Eigen::Vector2d xy{-DBL_MAX, -DBL_MAX};
			
			image_t image_id = kInvalidImageId;
			
			point2D_t point2D_idx = (point2D_t)kInvalidPoint3DId;
			
		};

		class AI3D_API Track
		{
		public:
			Track();

			
			inline size_t Length() const;
			
			
			inline const std::vector<TrackElement>& GetElements() const;
			std::vector<TrackElement>& GetElementsMutual();
			inline void SetElements(const std::vector<TrackElement>& elements);

			
			TrackElement* FindElementByImageIdMutual(image_t imageid);
			const TrackElement* FindElementByImageId(image_t imageid) const;
			
			inline const TrackElement& GetElement(const size_t idx) const;
			inline TrackElement& GetElementMutual(const size_t idx);
			inline void SetElement(const size_t idx, const TrackElement& element);

			
			inline void AddElement(const TrackElement& element);
			inline void AddElement(const image_t image_id, const point2D_t point2D_idx);
			inline void AddElements(const std::vector<TrackElement>& elements);

			
			inline void DeleteElement(const size_t idx);
			inline void DeleteElementByImageId(image_t id);
			void DeleteElement(const image_t image_id, const point2D_t point2D_idx);

			
			
			inline void Reserve(const size_t num_elements);

			
			inline void Compress();

		private:
			std::vector<TrackElement> elements_;
		};
	}
} 

#endif 

