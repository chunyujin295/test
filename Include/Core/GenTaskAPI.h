#pragma once
#include "Core/GenTaskOptions.h"   // GenTaskStatus / GenTaskSubType / GenTaskParams
#include "Core/BlockObject.h"       // BlockObject::Task_Info (SubmitGenTask 参数)
#include <string>
#include <functional>

namespace AI3D
{
    namespace CORE
    {
        class AI3D_API GenTaskAPI
        {
        public:
            struct SubmitResult
            {
                std::string task_uuid;
                std::string job_name;
                bool success = false;
                std::string error_msg;
            };

            /**
             *
             * @param blockInfo
             * @param user_account
             * @param pendingJobPath
             * @return
             */
            static SubmitResult SubmitGenTask(
                const AI3D::CORE::BlockObject::Task_Info& blockInfo,
                const std::string& user_account,
                const std::string& pendingJobPath);

            // static bool RequestCancel(const std::string& task_uuid);

            /**
             * downloadResult
             * @param result_url
             * @param save_path
             * @return
             */
            // static bool DownloadResult(const std::string& result_url,
            //                            const std::string& save_path);

            /**
             * Search user credits
             * @param user_account
             * @return
             */
            // static int QueryCredits(const std::string& user_account);
        };
    }
} // namespace AI3D::CORE
