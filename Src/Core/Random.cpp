






























#include "Core/Random.h"

#include <mutex>

namespace AI3D
{
    namespace CORE
    {

thread_local std::unique_ptr<std::mt19937> PRNG;

void SetPRNGSeed(unsigned seed) {
  PRNG.reset(new std::mt19937(seed));
  
  static std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);
  srand(seed);
}
	}

}  
