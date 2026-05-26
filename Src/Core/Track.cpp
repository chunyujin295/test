















#include "Core/Track.h"

namespace AI3D
{
    namespace CORE
    {
        Track::Track() 
		{

		}

        TrackElement::TrackElement()
             
		{

		}

        TrackElement::TrackElement(const image_t image_id, const point2D_t point2D_idx)
            : image_id(image_id), point2D_idx(point2D_idx) 
		{

		}

		TrackElement::TrackElement(const image_t image_id, const Eigen::Vector2d xy):image_id(image_id),xy(xy)
		{

		}

        void Track::DeleteElement(const image_t image_id, const point2D_t point2D_idx) 
        {
            elements_.erase(
                std::remove_if(elements_.begin(), elements_.end(),
                    [image_id, point2D_idx](const TrackElement& element)
                    {
                        return element.image_id == image_id &&
                            element.point2D_idx == point2D_idx;
                    }),
                elements_.end());
        }

		




		size_t Track::Length() const { return elements_.size(); }

		
		
		const std::vector<TrackElement>& Track::GetElements() const 
		{ 
			return elements_; 
		}
		 std::vector<TrackElement>& Track::GetElementsMutual() 
		{
			return elements_;
		}

		void Track::SetElements(const std::vector<TrackElement>& elements) 
		{
			elements_ = elements;
		}

		TrackElement* Track::FindElementByImageIdMutual(const image_t imageid)
		{
			for (auto& ele : elements_)
			{
				if (ele.image_id == imageid)
				{
					return &ele;
				}
			}
			return nullptr;
		}

		const TrackElement* Track::FindElementByImageId(const image_t imageid) const
		{
			for (const auto& ele : elements_)
			{
				if (ele.image_id == imageid)
				{
					return &ele;
				}
			}
			return nullptr;
		}

		
		const TrackElement& Track::GetElement(const size_t idx) const 
		{
			return elements_.at(idx);
		}

		TrackElement& Track::GetElementMutual(const size_t idx) 
		{ 
			return elements_.at(idx);		
		}
		void Track::SetElement(const size_t idx, const TrackElement& element) 
		{
			elements_.at(idx) = element;
		}

		
		void Track::AddElement(const TrackElement& element) 
		{
			elements_.push_back(element);
		}

		void Track::AddElement(const image_t image_id, const point2D_t point2D_idx) 
		{
			elements_.emplace_back(image_id, point2D_idx);
		}

		void Track::AddElements(const std::vector<TrackElement>& elements) 
		{
			elements_.insert(elements_.end(), elements.begin(), elements.end());
		}

		
		void Track::DeleteElement(const size_t idx) 
		{

			
			auto pos = std::find_if(elements_.begin(), elements_.end(), [idx](const TrackElement& ele) {return ele.image_id == idx; });
			CHECK_OPTION(pos != elements_.end());
			elements_.erase(pos);

		}

		inline void Track::DeleteElementByImageId(image_t id)
		{

			for (auto it = elements_.begin(); it != elements_.end();)
			{
				if (it->image_id == id)
				{
					it = elements_.erase(it++);
					break;
				}
				else
				{
					it++;
				}
				
					
			}
		}

		void Track::Reserve(const size_t num_elements) 
		{
			elements_.reserve(num_elements);
		}

		void Track::Compress() 
		{ 
			elements_.shrink_to_fit(); 
		}
    }
} 
