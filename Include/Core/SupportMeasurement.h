

#ifndef COLMAP_SRC_OPTIM_SUPPORT_MEASUREMENT_H_
#define COLMAP_SRC_OPTIM_SUPPORT_MEASUREMENT_H_

#include <cstddef>
#include <limits>
#include <vector>

namespace AI3D {
    namespace CORE {
        
        
        
        struct InlierSupportMeasurer {
            struct Support {
                
                size_t num_inliers = 0;

                
                double residual_sum = std::numeric_limits<double>::max();
            };

            
            Support Evaluate(const std::vector<double>& residuals,
                const double max_residual);

            
            bool Compare(const Support& support1, const Support& support2);
        };

        
        
        struct MEstimatorSupportMeasurer {
            struct Support {
                
                size_t num_inliers = 0;

                
                double score = std::numeric_limits<double>::max();
            };

            
            Support Evaluate(const std::vector<double>& residuals,
                const double max_residual);

            
            bool Compare(const Support& support1, const Support& support2);
        };

    }  
}
#endif  
