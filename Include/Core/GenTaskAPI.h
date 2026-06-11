// Include/Core/GenTaskAPI.h
#pragma once

#include "Core/GenTaskOptions.h"   // GenTaskStatus / GenTaskSubType / GenTaskParams
#include "Core/BlockObject.h"       // BlockObject::Task_Info
#include <string>
#include <Core/Types.h>

namespace AI3D {
    namespace CORE {

        class AI3D_API GenTaskAPI
        {
        public:
            /// @brief 提交生成式任务 (含积分预检)
            ///        内部: EstimateTaskPoints → 余额检查 → 不够则返回 success=false
            static SubmitResult SubmitGenTask(
                BlockObject::Task_Info& blockInfo,       // 非 const: 内部递增 next_generation_id
                const std::string& user_account,
                const std::string& pendingJobPath);

            /// @brief 通过 task_uuid 获取结果目录 (从 Block 的 generations_info_ 中查找)
            /// @return result_dir, 未找到返回空字符串
            static std::string GetResultDir(
                const std::string& task_uuid,
                const BlockObject::Task_Info& blockInfo);

            /// @brief 通过 task_uuid 下载结果到对应的 Generations/Generation_<id>/ 目录
            ///        自动从 generations_info_ 中查找 result_url 和 result_dir
            /// @param progressCb  进度回调 (bytesReceived, bytesTotal), 可选
            /// @return true=成功
            // static bool DownloadResultByTaskUuid(
            //     const std::string& task_uuid,
            //     const BlockObject::Task_Info& blockInfo,
            //     std::function<void(qint64 bytesReceived, qint64 bytesTotal)> progressCb = nullptr);

            /// @brief 直接下载 (保留, 供特殊场景指定自定义路径)
            // static bool DownloadResult(const std::string& result_url,
            //                             const std::string& save_path);

            /// @brief 请求取消任务
            // static bool RequestCancel(const std::string& task_uuid);

            // static int QueryCredits(const std::string& user_account);

            // Block 结果注册/更新由 GenTaskThread 直接操作 .blk (EXE 侧, 见 Phase 4.1)
        };

    }} // namespace AI3D::CORE