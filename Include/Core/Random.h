

#ifndef COLMAP_SRC_UTIL_RANDOM_H_
#define COLMAP_SRC_UTIL_RANDOM_H_

#include <chrono>
#include <memory>
#include <random>
#include <thread>

namespace AI3D {
    namespace CORE {

        extern thread_local std::unique_ptr<std::mt19937> PRNG;

        static int kDefaultPRNGSeed = 0;

        
        
        
        
        void SetPRNGSeed(unsigned seed = kDefaultPRNGSeed);

        
        
        
        template <typename T>
        T RandomInteger(const T min, const T max);

        
        
        
        template <typename T>
        T RandomReal(const T min, const T max);

        
        
        
        template <typename T>
        T RandomGaussian(const T mean, const T stddev);

        
        
        
        
        
        
        
        
        
        template <typename T>
        void Shuffle(const uint32_t num_to_shuffle, std::vector<T>* elems);

        
        
        

        template <typename T>
        T RandomInteger(const T min, const T max) {
            if (PRNG == nullptr) {
                SetPRNGSeed();
            }

            std::uniform_int_distribution<T> distribution(min, max);

            return distribution(*PRNG);
        }

        template <typename T>
        T RandomReal(const T min, const T max) {
            if (PRNG == nullptr) {
                SetPRNGSeed();
            }

            std::uniform_real_distribution<T> distribution(min, max);

            return distribution(*PRNG);
        }

        template <typename T>
        T RandomGaussian(const T mean, const T stddev) {
            if (PRNG == nullptr) {
                SetPRNGSeed();
            }

            std::normal_distribution<T> distribution(mean, stddev);
            return distribution(*PRNG);
        }

        template <typename T>
        void Shuffle(const uint32_t num_to_shuffle, std::vector<T>* elems) {
            
            const uint32_t last_idx = static_cast<uint32_t>(elems->size() - 1);
            for (uint32_t i = 0; i < num_to_shuffle; ++i) {
                const auto j = RandomInteger<uint32_t>(i, last_idx);
                std::swap((*elems)[i], (*elems)[j]);
            }
        }

    }  
}
#endif  
