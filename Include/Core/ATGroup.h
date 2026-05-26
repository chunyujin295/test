#ifndef _AI3D_CORE_AT_H_
#define _AI3D_CORE_AT_H_
#include <Constants.h>
#include "Core/ATData.h"
#include "Core/Alignment.h"
#include "Core/Types.h"

namespace AI3D 
{
    namespace CORE
    {
        
        class AI3D_API ATGroup
        {
        public:
            ATGroup();
            ATGroup(const ATGroup& Atgroup);
            ATGroup& operator=(const ATGroup& Atgroup);
            void SetName(std::string& name);
            const std::string GetName() const;
            std::string& GetNameMutual();
            
            void SetATOptions(ATOptions options);
            const ATOptions& GetATOptions() const;
            ATOptions& GetATOptionsMutual();

            const std::shared_ptr<ATData>& GetATData() const;
            std::shared_ptr<ATData>& GetATDataMutual();
            void SetATData(std::shared_ptr<ATData>Atdata);
        private:
            std::string name_ = "";
            ATOptions options_;
            std::shared_ptr<ATData> ATData_;
 
        };


       


    }
}
#endif