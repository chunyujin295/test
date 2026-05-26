#ifndef _AI3D_CORE_ATDEFINITION_H_
#define _AI3D_CORE_ATDEFINITION_H_
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <mutex>
#include <Eigen/Core>
#include <fstream>
#include <Constants.h>
#include <omp.h>

#include "Core/ATData.h"
#include  "Core/ATOptions.h"
#include "Core/ReturnCode.h"
#include "Core/Application.h"
namespace AI3D
{
    namespace CORE
    {
        

        class AI3D_API ATDefinition
        {
        public:

            ATDefinition(const ATData& Atdata, bool hastiepoints);

            int GetPolicyDim(policies_e object);
            
            
            std::tuple<policies_e, std::vector<std::pair<policies_e, bool>>> GetPolicy(policies_object_e object);
            
        private:
            ATData atdata_;
            bool hastiepoints_;

        };
    }
}
#endif