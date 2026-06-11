#pragma once

#include <string>
#include <optional>
#include <thread>
#include <chrono>

#include "Core/GenTaskOptions.h" // GenTaskParams (JSON 序列化) + GenTaskOptions
#include "Core/File.h"            // FopenDenyWriteLockUtf8 / OpenOfstreamUtf8
#include "Core/Logging.h"         // LOGE / LOGI / LOGW
#include "Core/DataStruct.h"      // GenJobInfoData / GenJobFile (Phase 1.2)
#include "Core/PointManager.h"
#include "Util/TaskProcess.h"     // JobFeedBack_s (GenJobFullInfo_s 持有, 对标 TaskGraph_s)
using namespace AI3D::CORE;

// GenTaskStatus 枚举 — 定义在 Core/GenTaskOptions.h, 供 DLL 和 EXE 共用
// (GenTaskProcess.h 已 include Core/GenTaskOptions.h, 同名命名空间内可直接使用)

// ============================================================================
// GenTaskResponse — 服务端返回的 HTTP 响应体 (DTO, 与服务端 API 契约一一对应)
//
// 由 GenHttpClient 回调 lambda 直接从 QJsonObject 解析, 不需要 JSON 序列化方法.
// optional 字段仅在服务端提供对应值时有值.
// ============================================================================
struct GenTaskResponse
{
    std::string task_uuid; // 回显客户端的 task_uuid
    std::optional<std::string> freeze_no; // 冻结单号 (/generation/create3DTask 返回, 同时也是任务查询标识)
    GenTaskStatus status = GenTaskStatus::IDLE; // 任务状态
    int progress = 0; // 进度百分比 0-100
    std::optional<std::string> result_url; // 结果文件 URL (COMPLETED 时服务端返回, 纹理模型)
    std::optional<std::string> preview_url; // 预览图 URL (COMPLETED 时服务端返回)
    std::optional<std::string> error_message; // 错误详情 (FAILED 时有值)
    int available_points = 0; // 可用积分 (submit/query 均返回)
    int frozen_points = 0; // 冻结积分 (submit/query 均返回)
    int total_balance = 0; // 总积分 (submit/query 均返回)
};

// ============================================================================
// GenJobInfo_s — job 文件顶层结构, 对标 TaskProcess.h 的 JobInfo_s
//
// 生命周期: GenTaskAPI::SubmitGenTask 创建 (params 由前端填入) 并写入 Pending/
//          → GenTaskThread::ProcessPendingJobs 透传 params 给服务端
//          → ProcessRunningJobs 轮询并更新状态
//          → 完成后移到 Completed/ 或 Failed/
//
// 序列化策略: 文件 I/O 始终走 BIN 加密, 不需要 JSON 序列化.
//   GenJobInfo_s     — 纯数据 (对标 JobInfo_s), 无序列化方法
//   GenJobFullInfo_s — 文件级结构体 (对标 JobFullInfo_s), save/load → WriteToBin/LoadFromBin
//   GenJobFile(GenJobInfoData + GenJobTaskData + FeedBackData) — DataStruct.h BIN 序列化 (对标 JobListFile)
// ============================================================================

/// @brief 单个生成式任务的纯数据结构 (对标 JobInfo_s)
struct GenJobInfo_s
{
    // --- 客户端标识 ---
    std::string task_uuid; // 客户端生成的 UUID, 全局唯一, 前端通过此 ID 查询
    std::string engine_id; // 发起 Engine 的主机名
    std::string user_account; // 用户账号 (submit 时传给服务端, 积分计费用)

    // --- Block 关联 (定位 feedback 文件用) ---
    std::string project_path; // 所属项目目录
    std::string block_item; // 所属 Block 名称 (feedback 路径 = result_dir/JF_job_name)

    // --- 生成参数 (前端填入 → WriteToJson()/ToJsonString() → HTTP/文件) ---
    GenTaskParams params; // 生成参数结构体, 自带 JSON 序列化方法

    // --- 运行时状态 (由 GenTaskThread 从 HTTP 响应回填) ---
    int generation_id = 0; // Generation_<id>, SubmitGenTask 推算并写入
    GenTaskStatus status = GenTaskStatus::IDLE;
    std::string result_url; // 结果文件 URL (COMPLETED 时服务端返回)
    std::string preview_url; // 预览图 URL (COMPLETED 时服务端返回)
    std::string result_dir; // 下载目标目录: Generations/Generation_<id>/ (结果和预览共用)
    std::string result_path; // 结果完整路径 (前端下载完成后回填)
    std::string preview_path; // 预览完整路径 (前端下载完成后回填)
    std::string error_message; // 详细错误信息 (FAILED 时填充)
    int query_retry_count = 0; // 连续轮询失败次数 (>= 5 则标记失败)
    int progress = 0; // 进度百分比 0-100 (IN_PROGRESS 时由 resp.progress 回填)

    // --- 积分相关 (详见 App/doc/积分接口集成方案.md) ---
    PointInfoBase point_info; // 替代旧 cost_credits/points_balance, 统一管理所有积分数据

    /// @brief 将 HTTP 响应回填到自身
    ///        optional 字段只在有值时覆盖; 积分字段直接覆盖 (后端返回值更准确)
    void ApplyResponse(const GenTaskResponse& resp)
    {
        if (resp.result_url) result_url = *resp.result_url;
        if (resp.preview_url) preview_url = *resp.preview_url;
        if (resp.error_message) error_message = *resp.error_message;
        status = resp.status;
        progress = resp.progress;
        if (resp.freeze_no) point_info.freeze_no = *resp.freeze_no;
        point_info.frozen_points = resp.frozen_points;
        point_info.total_balance = resp.total_balance;
        point_info.available_points = resp.available_points;
    }
};

// ============================================================================
// GenJobFullInfo_s — 文件级结构体 (对标 JobFullInfo_s)
//
// 与 JobFullInfo_s 的对应关系:
//   JobFullInfo_s { JobName, TaskGraph_s { JobInfo_s, RunInfo_s, JobFeedBack_s, tasksmap } }
//   GenJobFullInfo_s { JobName, GenJobInfo_s }
//
// GenJobFullInfo_s 比 JobFullInfo_s 简单: 生成式任务没有子任务拆分 (tasksmap),
// 没有 RunInfo_s, 执行方式为 HTTP 调用而非 spawn 子进程。
//
// JobFeedBack_s feedback — 对标 TaskGraph_s 持有的 JobFeedBack_s, 内存中持有避免
// UpdateFeedback 每次 load/save 独立文件, 减少不必要的 I/O。
// feedback 同时序列化到 GenJobFile BIN (对标 JobListFile::feedBackData), 也独立持久化到 JF_* 文件。
//
// save/load — 始终走 BIN 加密 (XOR 0xAB), 无 JSON 分支。
//   JOB_INFO_USE_BIN 当前恒为 true, JSON 调试路径已废弃。
// ============================================================================

struct GenJobFullInfo_s
{
    std::string job_name; // J_<BlockName>_<timestamp>, 用作文件名, 对标 JobFullInfo_s::JobName
    GenJobInfo_s job; // 任务数据, 对标 JobFullInfo_s::tg.job (JobInfo_s)
    JobFeedBack_s feedback; // 进度反馈, 对标 TaskGraph_s 持有的 JobFeedBack_s, 会序列化到 BIN

    GenJobFullInfo_s()
    {
    }

    GenJobFullInfo_s(const std::string& file)
    {
        load(file);
    }

    // ========================================================================
    // WriteToBin / LoadFromBin — 通过 GenJobFile 序列化
    // 对标 JobFullInfo_s::WriteToBin / LoadFromBin
    // ========================================================================

    bool WriteToBin(const std::string& filePath) const
    {
        std::ofstream out = File::OpenOfstreamUtf8(filePath, std::ios::binary);
        if (!out.is_open())
        {
            LOGE("GenJobFullInfo_s::WriteToBin: failed to open: " + filePath);
            return false;
        }

        GenJobFile f;
        f.jobName = job_name;

        // 指针节
        f.genJobInfoData.project_path = job.project_path;
        f.genJobInfoData.block_item = job.block_item;

        // 任务数据节
        GenJobTaskData& t = f.genJobTaskData;
        t.generation_id = job.generation_id;
        t.task_uuid = job.task_uuid;
        t.engine_id = job.engine_id;
        t.user_account = job.user_account;
        t.params_json = job.params.ToJsonString();
        t.status = static_cast<int>(job.status);
        t.result_url = job.result_url;
        t.preview_url = job.preview_url;
        t.result_path = job.result_path;
        t.preview_path = job.preview_path;
        t.result_dir = job.result_dir;
        t.error_message = job.error_message;
        t.query_retry_count = job.query_retry_count;
        t.progress = job.progress;

        // 积分 (PointInfoBase → GenJobTaskData 平铺)
        t.freeze_no = job.point_info.freeze_no;
        t.frozen_points = job.point_info.frozen_points;
        t.consumed = job.point_info.consumed;
        t.refunded = job.point_info.refunded;
        t.total_balance = job.point_info.total_balance;
        t.available_points = job.point_info.available_points;
        t.points_settled = job.point_info.points_settled;

        // 反馈节
        f.feedBackData.status = static_cast<int>(feedback.Status);
        f.feedBackData.percent = feedback.Percent;
        f.feedBackData.taskRetVal = feedback.TaskRetVal;
        f.feedBackData.msg = feedback.Msg;

        f.Serialize(out);
        out.close();
        return true;
    }

    bool LoadFromBin(const std::string& filePath)
    {
        std::ifstream in = File::OpenIfstreamUtf8(filePath, std::ios::binary);
        if (!in.is_open())
            return false;

        GenJobFile f;
        if (!f.Deserialize(in))
        {
            in.close();
            return false;
        }
        in.close();

        job_name = f.jobName;

        // 任务数据节
        GenJobTaskData& t = f.genJobTaskData;
        job.generation_id = t.generation_id;
        job.task_uuid = t.task_uuid;
        job.engine_id = t.engine_id;
        job.user_account = t.user_account;
        job.project_path = f.genJobInfoData.project_path;
        job.block_item = f.genJobInfoData.block_item;
        job.params = GenTaskParams::CreateFromJsonString(t.params_json);
        job.status = static_cast<GenTaskStatus>(t.status);
        job.result_url = t.result_url;
        job.preview_url = t.preview_url;
        job.result_path = t.result_path;
        job.preview_path = t.preview_path;
        job.result_dir = t.result_dir;
        job.error_message = t.error_message;
        job.query_retry_count = t.query_retry_count;
        job.progress = t.progress;

        // 积分
        job.point_info.freeze_no = t.freeze_no;
        job.point_info.frozen_points = t.frozen_points;
        job.point_info.consumed = t.consumed;
        job.point_info.refunded = t.refunded;
        job.point_info.total_balance = t.total_balance;
        job.point_info.available_points = t.available_points;
        job.point_info.points_settled = t.points_settled;

        // 反馈
        feedback.Status = static_cast<jobsta_e>(f.feedBackData.status);
        feedback.Percent = f.feedBackData.percent;
        feedback.TaskRetVal = f.feedBackData.taskRetVal;
        feedback.Msg = f.feedBackData.msg;

        return true;
    }

    // ========================================================================
    // save / load — BIN 加密 (对标 JobFullInfo_s::save/load, 但只保留 BIN 路径)
    // ========================================================================

    bool save(const std::string& filePath) const
    {
        bool result = WriteToBin(filePath);
        if (!result)
        {
            LOGE("GenJobFullInfo_s::save: WriteToBin failed: " + filePath);
        }
        return result;
    }

    bool load(const std::string& filePath)
    {
        bool result = LoadFromBin(filePath);
        if (!result)
        {
            LOGE("GenJobFullInfo_s::load: LoadFromBin failed: " + filePath);
        }
        return result;
    }

    // ========================================================================
    // save_with_retry / load_with_retry — deny-write 锁 + 3 次重试
    // 对标 JobFullInfo_s::save / load, 但增加了 deny-write 锁 (参照 JobFeedBack_s 的锁模式)
    // ========================================================================

    bool save_with_retry(const std::string& filePath) const
    {
        int retryTimes = 0;
        do
        {
            FILE* fpLock = File::FopenDenyWriteLockUtf8(filePath + ".lock");
            if (fpLock == NULL)
            {
                retryTimes++;
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                continue;
            }
            bool result = save(filePath);
            fclose(fpLock);
            return result;
        }
        while (retryTimes < 3);

        LOGE(std::string("GenJobFullInfo_s::save_with_retry failed after 3 retries: ") + filePath);
        return false;
    }

    bool load_with_retry(const std::string& filePath)
    {
        int retryTimes = 0;
        do
        {
            FILE* fpLock = File::FopenDenyWriteLockUtf8(filePath + ".lock");
            if (fpLock == NULL)
            {
                retryTimes++;
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                continue;
            }
            bool result = load(filePath);
            fclose(fpLock);
            if (result) return true;

            retryTimes++;
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }
        while (retryTimes < 3);

        LOGE(std::string("GenJobFullInfo_s::load_with_retry failed after 3 retries: ") + filePath);
        return false;
    }
};
