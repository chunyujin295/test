

#ifndef COLMAP_SRC_OPTIM_RANDOM_SAMPLER_H_
#define COLMAP_SRC_OPTIM_RANDOM_SAMPLER_H_

#include "Core/Sampler.h"

namespace AI3D {
    namespace CORE {

        
        
        
        class RandomSampler : public Sampler {
        public:
            explicit RandomSampler(const size_t num_samples);

            void Initialize(const size_t total_num_samples) override;

            size_t MaxNumSamples() override;

            std::vector<size_t> Sample() override;

        private:
            const size_t num_samples_;
            std::vector<size_t> sample_idxs_;
        };

    }  
}
#endif  
