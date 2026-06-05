#ifndef _AI3D_CORE_RECON_PERF_LOG_H_
#define _AI3D_CORE_RECON_PERF_LOG_H_

#include <Core/Logging.h>
#include <Core/String.h>
#include <Core/Timer.h>
#include <string>

namespace AI3D
{
    namespace CORE
    {
        /** Global switch: [ReconPerf] logs print only when true. Default: false. */
        AI3D_API bool IsReconPerfLogEnabled();
        AI3D_API void SetReconPerfLogEnabled(bool enabled);

        inline void ReconPerfLog(const std::string& message)
        {
            if (IsReconPerfLogEnabled()) {
                LOGI(message);
            }
        }

        inline void ReconPerfLog(const char* message)
        {
            if (IsReconPerfLogEnabled()) {
                LOGI(message);
            }
        }

        /** RAII: logs [ReconPerf] flow | stage | start and elapsed seconds on destruction. */
        class ReconPerfStage
        {
        public:
            ReconPerfStage(const char* flow, const char* stage)
                : flow_(flow), stage_(stage), enabled_(IsReconPerfLogEnabled())
            {
                if (!enabled_) {
                    return;
                }
                timer_.Start();
                ReconPerfLog(String::StringPrintf("[ReconPerf] %s | %s | start", flow_, stage_));
            }

            ~ReconPerfStage()
            {
                if (!enabled_) {
                    return;
                }
                ReconPerfLog(String::StringPrintf(
                    "[ReconPerf] %s | %s | %.3f s", flow_, stage_, timer_.ElapsedSeconds()));
            }

            ReconPerfStage(const ReconPerfStage&) = delete;
            ReconPerfStage& operator=(const ReconPerfStage&) = delete;

        private:
            const char* flow_;
            const char* stage_;
            bool enabled_ = false;
            Timer timer_;
        };
    }
}

#endif
