// ==================== Include/Core/PointManager.h ====================
#pragma once
#include <QString>
#include <string>
#include <Constants.h>

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
            std::string freeze_no = ""; // 冻结单号 (仅 /point/freeze 有值)
            int estimate_points = 0; // 预估消耗 (仅 /point/estimate 有值)
            int total_balance = 0; // 总积分
            int frozen_points = 0; // 冻结积分总额
            int available_points = 0; // 可用积分
            bool requestSucceeded = false;
            int errorCode = 20001;
        };

        /// @brief 积分结算返回结构
        struct PointSettleInfo
        {
            int consumed = 0; // 本次实际消耗
            int refunded = 0; // 本次返还
            int total_balance = 0; // 结算后总积分
            int frozen_points = 0; // 结算后冻结积分
            int available_points = 0; // 结算后可用积分
            bool requestSucceeded = false;
            int errorCode = 20001;
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
            static PointFreezeInfo EstimateTaskPoints(const std::string& business_type,
                                                      const std::string& task_param_json);


            static PointFreezeInfo CreatePointTask(const std::string& business_type,
                                                   const std::string& task_param_json);

            static PointFreezeInfo QueryUserPoints();


            static PointSettleInfo SettlePoints(const std::string& freeze_no,
                                                const std::string& settle_status,
                                                const std::string& task_param_json);

        private:
            static PointFreezeInfo postPointsOutline(int timeout_ms = 3000, int max_retries = 2);
            static PointFreezeInfo postPointsFreeze(const std::string& business_type,
                                                    const std::string& task_param_json,
                                                    int timeout_ms = 3000, int max_retries = 3);
            static PointFreezeInfo postPointsEstimate(const std::string& business_type,
                                                      const std::string& task_param_json,
                                                      int timeout_ms = 3000, int max_retries = 2);
            static PointFreezeInfo postGenerationGetQuote(const std::string& task_param_json,
                                                          int timeout_ms = 3000, int max_retries = 2);
            static PointSettleInfo postPointsSettle(const std::string& freeze_no,
                                                    const std::string& settle_status,
                                                    const std::string& task_param_json,
                                                    int timeout_ms = 3000, int max_retries = 3);

            static QString buildAuthHeader(const QString& url, const QString& dataJson = "");
            static QString loadAccessToken();
        };
    } // namespace CORE
} // namespace AI3D
