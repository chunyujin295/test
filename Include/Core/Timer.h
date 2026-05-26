#ifndef _AI3D_CORE_TIMER_H_
#define _AI3D_CORE_TIMER_H_

#include <Constants.h>

#include <chrono>

namespace AI3D 
{
	namespace CORE 
	{
		class AI3D_API Timer
		{
		public:
			Timer();


			void Start();
			void Restart();
			void Pause();
			void Resume();
			void Reset();

			double ElapsedMicroSeconds() const;
			double ElapsedSeconds() const;
			double ElapsedMinutes() const;
			double ElapsedHours() const;
			void PrintSeconds() const;
			void PrintMinutes() const;
			void PrintHours() const;

			static char* TimeNow();
			

		private:
			bool started_;
			bool paused_;
			std::chrono::high_resolution_clock::time_point start_time_;
			std::chrono::high_resolution_clock::time_point pause_time_;
		};
	}
}  

#endif  
