
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
#include "Core/ATGroup.h"
#include "Core/ATData.h"
#include "Core/Image.h"
#include "Core/Point2d.h"
#include "Core/Point3d.h"
#include "Core/Track.h"
#include "Core/alignment.h"
#include "Core/Types.h"


#include <glog/logging.h>


namespace AI3D
{
    namespace CORE
    {
        ATGroup::ATGroup()
        {

        };

        ATGroup::ATGroup(const ATGroup& Atgroup)
        {
            auto AT_tmp = std::make_shared<ATData>(*Atgroup.ATData_);
            if (ATData_ != nullptr)
            {
                ATData_.reset();
            }
            name_ = Atgroup.name_;
            options_ = Atgroup.options_;
            ATData_ = AT_tmp;
        }

		ATGroup& ATGroup::operator=(const ATGroup& Atgroup)
		{
			if (this != &Atgroup)
			{
                auto AT_tmp = std::make_shared<ATData>(*Atgroup.ATData_);
                if (ATData_ != nullptr)
                {
                    ATData_.reset();
                }
				name_ = Atgroup.name_;
				options_ = Atgroup.options_; 
				ATData_ = AT_tmp;
			}
			return *this;
		}

        void ATGroup::SetName(std::string& name)
        { 
            name_ = name; 
        };
        const std::string ATGroup::GetName() const
        { 
            return name_; 
        };
        std::string& ATGroup::GetNameMutual()
        { 
            return name_; 
        };
        
        void ATGroup::SetATOptions(ATOptions options) 
        { 
            options_ = options; 
        };
        const ATOptions& ATGroup::GetATOptions() const
        {
            return options_; 
        };
        ATOptions& ATGroup::GetATOptionsMutual()
        { 
            return options_; 
        };
        const std::shared_ptr<ATData>& ATGroup::GetATData() const
        {
            return ATData_;
        };
        std::shared_ptr<ATData>& ATGroup::GetATDataMutual()
        {
            return ATData_;
        };
        void ATGroup::SetATData(std::shared_ptr<ATData> Atdata)
        {
            ATData_ = Atdata;
        }
    }
}




