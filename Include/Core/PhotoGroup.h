

#ifndef _AI3D_CORE_PHOTOGROUP_H_
#define _AI3D_CORE_PHOTOGROUP_H_
#include <Constants.h>
#include "Core/Alignment.h"
#include "Core/Types.h"




namespace AI3D
{
	namespace CORE
	{
		
		class AI3D_API PhotoGroup
		{
		public:

			PhotoGroup() ;
			
			const std::string GetGroupPath() const;
			 std::string GetGroupPathMutual() ;
			void SetGroupPath(const std::string path);
			void SetId(group_t id);
			const group_t GetId() const;
			group_t& GetIdMutual();

			void SetName(std::string name);
			const std::string GetName() const;
			std::string& GetNameMutual();
			void SetCamera(const class Camera& camera);
			const class Camera& GetCamera() const;
			Camera& GetCameraMutual();
			bool IsSame(const PhotoGroup& group);
			bool IsSame_Beta(const PhotoGroup& group);
			
			
			
			size_t GetNumImagesMutual();
			size_t GetNumImages() const;
			std::string GetExtension();
			std::string GetExtension()const;
			void SetExtension(const std::string& extension);
			
			

			

			
			
			 
			 
			
			
			void SetGroupImage(const std::set<image_t>& group_image);
			std::set<image_t> GetGroupImageIds()const;
			void AddImageId(image_t id);
			void ClearImage();
			
			
			bool PhotoGroupContain(const PhotoGroup& photogroup);
		private:
			std::string name_ = "";
			class Camera camera_;
			group_t id_ = kInvalidCameraId;
			std::string path_;
			std::string image_extension_ = "";
			
			
			
		   std::set<image_t> include_images_;
		};

	}
}
#endif