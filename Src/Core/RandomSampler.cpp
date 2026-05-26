

#include "Core/RandomSampler.h"

#include <numeric>

#include "Core/Random.h"

namespace AI3D {
    namespace CORE {

        RandomSampler::RandomSampler(const size_t num_samples)
            : num_samples_(num_samples) {}

        void RandomSampler::Initialize(const size_t total_num_samples) {
            
            sample_idxs_.resize(total_num_samples);
            std::iota(sample_idxs_.begin(), sample_idxs_.end(), 0);
        }

        size_t RandomSampler::MaxNumSamples() {
            return std::numeric_limits<size_t>::max();
        }

        std::vector<size_t> RandomSampler::Sample() {
            Shuffle(static_cast<uint32_t>(num_samples_), &sample_idxs_);

            std::vector<size_t> sampled_idxs(num_samples_);
            for (size_t i = 0; i < num_samples_; ++i) {
                sampled_idxs[i] = sample_idxs_[i];
            }

            return sampled_idxs;
        }
    }

}  
