// ==================== Include/Core/PointManager.h ====================
#pragma once
#include <string>
#include <Constants.h>
#include <Util/HttpClient.h>

namespace AI3D
{
    namespace CORE
    {
        // ============================================================================
        // 积分数据结构 — 生成式和重建式共用
        // ============================================================================

        /// @brief 积分冻结/查询/估算共用返回结构
        struct PointFreezeInfo
        {
            std::string freeze_no; // 冻结单号 (仅 /point/freeze 有值)
            int estimate_points = 0; // 预估消耗 (仅 /point/estimate 有值)
            int total_balance = 0; // 总积分
            int frozen_points = 0; // 冻结积分总额
            int available_points = 0; // 可用积分
        };

        /// @brief 积分结算返回结构
        struct PointSettleInfo
        {
            int consumed = 0; // 本次实际消耗
            int refunded = 0; // 本次返还
            int total_balance = 0; // 结算后总积分
            int frozen_points = 0; // 结算后冻结积分
            int available_points = 0; // 结算后可用积分
        };

        /// @brief 嵌入任务结构体的积分元信息
        ///        生成式: GenJobInfo_s::point_info
        ///        重建式: JobInfo_s::point_info
        struct PointInfoBase
        {
            std::string freeze_no; // 冻结单号
            int frozen_points = 0; // 冻结积分总额
            int consumed = 0; // 实际消耗 (结算后)
            int refunded = 0; // 返还积分 (结算后)
            bool points_settled = false; // 是否已结算
            int total_balance = 0; // 总积分
            int available_points = 0; // 可用积分
        };

        // ============================================================================
        // PointManager
        // ============================================================================

        class AI3D_API PointManager
        {
        public:
            /// @brief 任务积分消耗预估 — POST /point/estimate
            /// @return estimate_points; 失败返回 -1
            static int EstimateTaskPoints(const std::string& business_type,
                                          const std::string& task_param_json);

            /// @brief 积分任务创建 (冻结) — POST /point/freeze
            /// @return freeze_no + 余额快照; freeze_no 为空表示失败
            static PointFreezeInfo CreatePointTask(const std::string& business_type,
                                                   const std::string& task_param_json);

            /// @brief 用户积分查询 — POST /point/outline
            static PointFreezeInfo QueryUserPoints();

            /// @brief 积分结算 — POST /point/settle
            /// @return consumed+refunded==0 表示失败
            static PointSettleInfo SettlePoints(const std::string& freeze_no,
                                                const std::string& settle_status,
                                                const std::string& task_param_json);

        private:
            // ---- HTTP 实现 (复用 HttpClient::post, 不暴露给外部) ----
            static PointFreezeInfo postPointsOutline(int timeout_ms = 3000, int max_retries = 2);
            static PointFreezeInfo postPointsFreeze(const std::string& business_type,
                                                    const std::string& task_param_json,
                                                    int timeout_ms = 3000, int max_retries = 3);
            static PointFreezeInfo postPointsEstimate(const std::string& business_type,
                                                      const std::string& task_param_json,
                                                      int timeout_ms = 3000, int max_retries = 2);
            static PointSettleInfo postPointsSettle(const std::string& freeze_no,
                                                    const std::string& settle_status,
                                                    const std::string& task_param_json,
                                                    int timeout_ms = 3000, int max_retries = 3);

            // ---- 鉴权工具 (与 GenHttpClient::BuildAuthHeader 逻辑一致) ----
            static QString buildAuthHeader(const QString& url, const QString& dataJson = "");
            static QString loadAccessToken();
        };
    } // namespace CORE
} // namespace AI3D
