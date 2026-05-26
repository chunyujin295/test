
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <mutex>
#include <Eigen/Core>
#include <fstream>
#include <Constants.h>
#include "Core/Camera.h"
#include "Core/ProjectObject.h"
#include "Core/ATData.h"
#include "Core/Image.h"
#include "Core/Point2d.h"
#include "Core/Point3d.h"
#include "Core/Track.h"
#include "Core/alignment.h"
#include "Core/Types.h"

#include "Core/PhotoGroup.h"
#include <glog/logging.h>


namespace AI3D
{
    namespace CORE
    {


         PhotoGroup::PhotoGroup() 
         {

         };

         const std::string PhotoGroup::GetGroupPath() const
         {
             return path_;
         }
         std::string PhotoGroup::GetGroupPathMutual()
         {
             return path_;
         }
         void PhotoGroup::SetGroupPath(const std::string path)
         {
             path_ = path;
         }

        void PhotoGroup::SetId(group_t id) 
        { 
            id_ = id; 
        };
        const group_t PhotoGroup::GetId() const 
        { 
            return id_; 
        };

        group_t& PhotoGroup::GetIdMutual()
        {
            return id_;
        }
        void PhotoGroup::SetName(std::string name) 
        { 
            name_ = name;
        };
        const std::string PhotoGroup::GetName() const 
        { 
            return name_; 
        };
        std::string& PhotoGroup::GetNameMutual() 
        { 
            return name_; 
        };
        void PhotoGroup::SetCamera(const class Camera& camera) 
        { 
            camera_ = camera; 
        };
        const class Camera& PhotoGroup::GetCamera() const
        {
            return camera_;
        };
        Camera& PhotoGroup::GetCameraMutual() 
        { 
            return camera_; 
        };

        bool PhotoGroup::IsSame(const PhotoGroup& group)
        {
            
            if (camera_.IsSame(group.GetCamera()) && path_ == group.GetGroupPath())
            {
                return true;
            }
            return false;
        }
        bool PhotoGroup::IsSame_Beta(const PhotoGroup& group)
        {
            
			if (camera_.IsSame(group.GetCamera()) && path_ == group.GetGroupPath() && image_extension_ == group.GetExtension())
            {
                return true;
            }
            return false;
        }
        
        
        
        
        
        
        
        
        
        size_t PhotoGroup::GetNumImagesMutual() 
        {
            return include_images_.size();
        }
        size_t PhotoGroup::GetNumImages() const 
        {
            return include_images_.size(); 
        }
        std::string PhotoGroup::GetExtension()
        {
            return image_extension_;
        }
        std::string PhotoGroup::GetExtension()const
        {
            return image_extension_;
        }
        void PhotoGroup::SetExtension(const std::string& extension)
        {
            image_extension_ = extension;
        }
        
        

        

        
        
         
         

        void PhotoGroup::SetGroupImage(const std::set<image_t>& group_image)
        {
            include_images_ = group_image;
        }
        std::set<image_t> PhotoGroup::GetGroupImageIds()const
        {
            return include_images_;
        }
  
        void PhotoGroup::AddImageId(image_t id)
        {
            include_images_.insert(id);
        }
        void PhotoGroup::ClearImage()
        {
            include_images_.clear();
        }
        bool PhotoGroup::PhotoGroupContain(const PhotoGroup& photogroup)
        {
            bool IsContain = false;
            for (const auto& id : photogroup.GetGroupImageIds())
            {
                if (include_images_.find(id) == include_images_.end())
                {
                    return IsContain;
                }
            }
            IsContain = true;
            return IsContain;
        }

    }
}




