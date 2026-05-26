

#ifndef _AI3D_CORE_GCPATALGORITHM_H_
#define _AI3D_CORE_GCPATALGORITHM_H_

#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <mutex>
#include <Eigen/Core>
#include <fstream>
#include <Constants.h>

#include"Core/ATData.h"
#include"Core/String.h"
namespace AI3D 
{
    namespace CORE
    {
        
        class AI3D_API GCPATAlgorithm
        {
        public:
           static int AlignToATData(ATData& ATData);
           static bool IsValidForTriangulation();
           static bool IsValidForAT();
            

        };

    }
}
#endif