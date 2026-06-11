#include "GenTaskThread.h"
#include "GenHttpClient.h"
#include "Util/GenTaskProcess.h"
#include "Util/TaskProcess.h"
#include "Util/Settings.h"
#include "Core/Types.h"
#include "Core/File.h"             // File::FopenDenyWriteLockUtf8 / BoostPathFromUtf8
#include "Core/Logging.h"          // LOGI / LOGE / LOGW
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFile>
#include <QDateTime>               // QDateTime::currentDateTime (SearchUnnormalRunningJob)
#include <QHostInfo>
#include <thread>
#include <chrono>

#include "Core/BlockObject.h"
using namespace AI3D::CORE;

// extern 声明
extern bool bQuitingApplication; // 由 CallEngine.cpp 定义, Node 关闭时置 true
extern QString genPendingJobPath; // 由 CallEngine::MakePath() 初始化
extern QString genRunningJobPath;
extern QString genCompletedJobPath;
extern QString genFailedJobPath;
extern QString genCancelledJobPath;

static bool MoveJobFile(const std::string& src, const std::string& dstDir)
{
    // 0. 先删残留 .lock (对标 DoCleanupJobLockOnceWhileEngineStart)
    QString srcLock = QString::fromStdString(src) + ".lock";
    if (QFileInfo::exists(srcLock))
        QFile::remove(srcLock);

    QFileInfo fi(QString::fromStdString(src));
    QString dstPath = QString::fromStdString(dstDir) + "/" + fi.fileName();
    QString dstLock = dstPath + ".lock";
    if (QFileInfo::exists(dstLock))
        QFile::remove(dstLock);

    // 1. 尝试直接 rename (原子操作, 同盘高效)
    if (QFile::rename(QString::fromStdString(src), dstPath))
    {
        return true;
    }

    // 2. rename 失败 (可能跨盘) → copy + remove 兜底
    LOGW("MoveJobFile: rename failed, trying copy+remove: " + src + " -> " + dstDir);
    if (QFile::copy(QString::fromStdString(src), dstPath))
    {
        QFile::remove(QString::fromStdString(src));
        return true;
    }

    LOGE("MoveJobFile: both rename and copy+remove failed: " + src + " -> " + dstDir);
    return false;
}

static std::string BuildFeedbackPath(const GenJobFullInfo_s& info)
{
    // feedback 放在结果目录下: Generations/Generation_<id>/JF_<job_name>.bin
    //   对标重建式: Productions/Production_<id>/JF_<job_name>.bin
    std::string base = info.job.result_dir + "/JF_" + info.job_name;
    if (JOB_FEEDBACK_USE_BIN)
    {
        return base + BINFILE_POSTFIX; // ".bin"
    }
    else
    {
        return base + JSONFILE_POSTFIX; // ".json"
    }
}

/// @brief 更新内存中的 JobFeedBack_s — 仅修改 Status/Percent/Msg/TaskRetVal
///        结果数据 (result_url 等) 存在 GenJobInfo_s 自身中, 不写入 feedback
///        调用者负责在 UpdateFeedback 之后 save feedback 到独立文件
static void UpdateFeedback(GenJobFullInfo_s& info)
{
    GenJobInfo_s& job = info.job;
    JobFeedBack_s& fb = info.feedback;

    // GenTaskStatus → jobsta_e 映射 (覆盖已有值, 对标 TaskGraph_s 的同步更新)
    switch (job.status)
    {
    case GenTaskStatus::IDLE:
    case GenTaskStatus::PENDING:
        fb.Status = jobsta_e::STATUS_PENDDING;
        fb.Percent = 0.0f;
        break;
    case GenTaskStatus::IN_PROGRESS:
        fb.Status = jobsta_e::STATUS_RUNNING;
        fb.Percent = job.progress; // 由 ApplyResponse 从 resp.progress 回填
        break;
    case GenTaskStatus::COMPLETED:
        fb.Status = jobsta_e::STATUS_COMPLETE;
        fb.Percent = 100.0f;
        break;
    case GenTaskStatus::FAILED:
        fb.Status = jobsta_e::STATUS_FAILURE;
        fb.Msg = "generation task failed";
        break;
    case GenTaskStatus::CANCELLED:
        fb.Status = jobsta_e::STATUS_COMPLETE;
        break;
    }
}

// ============================================================================
// 主循环 (在独立 std::thread 中运行, 对标 searchPendingJobThread2)
// ============================================================================

void GenTaskThread::Run()
{
    LOGI("GenTaskThread started");

    while (true)
    {
        if (bQuitingApplication)
            break;

        ProcessPendingJobs(); // 1. 扫描 Pending → submit → 移到 Running
        ProcessRunningJobs(); // 2. 扫描 Running → 轮询状态 → 移到 Completed/Failed
        SleepMs(2000); // 2 秒轮询间隔
    }

    LOGI("GenTaskThread stopped");
}

// ============================================================================
// ProcessPendingJobs — 处理 Pending 目录中的新任务
//
// 流程:
//   1. 遍历 jobs_gen/Pending/*.json
//   2. deny-write 锁 → 加载 job → 检查崩溃恢复
//   3. 上传本地文件 (FILE_PATH → FILE_KEY)
//   4. HTTP POST submit → 回填 freeze_no
//   5. HTTP POST /point/freeze → 回填 freeze_no         ← ★ 积分冻结 (详见 积分接口集成方案.md 第四章)
//      失败不移文件, 下轮重试
//   6. 保存 → 移到 Running/
//   7. submit 失败不移动文件, 下轮重试
// ============================================================================

void GenTaskThread::ProcessPendingJobs()
{
    QString pendingDir = genPendingJobPath;
    QDirIterator it(pendingDir, {"J_*.bin"}, QDir::Files);

    while (it.hasNext())
    {
        it.next();
        QString filePath = it.filePath();
        std::string filePathStr = filePath.toStdString();

        // 1. 加载 job 文件 (load_with_retry 内部已处理 deny-write 锁)
        GenJobFullInfo_s info;
        if (!info.load_with_retry(filePathStr))
        {
            LOGE("ProcessPendingJobs: failed to load: " + filePathStr);
            continue;
        }
        GenJobInfo_s& job = info.job;

        // 1.5. 加载 feedback 到内存 (独立 JF_* 文件, 对标 TaskGraph_s 持有 JobFeedBack_s)
        std::string fbPath = BuildFeedbackPath(info);
        info.feedback.load_with_retry(fbPath, false);

        // 2. 崩溃恢复: 已有 freeze_no 则直接移到 Running
        if (!job.point_info.freeze_no.empty())
        {
            LOGI("Crash recovery: " + job.task_uuid + " already submitted, moving to Running");
            MoveJobFile(filePathStr, qstr2str(genRunningJobPath));
            continue;
        }

        // 3. HTTP POST submit — GenTaskParams 自动序列化为 JSON
        //    前端已处理文件上传 (如有), engine 只负责透传参数
        GenTaskResponse resp = GenHttpClient::SubmitTask(
            job.task_uuid, job.params);

        // 4. 处理响应
        if (resp.status == GenTaskStatus::IDLE && resp.error_message.has_value())
        {
            // 网络超时 → 不移动文件, 下轮重试
            LOGW("SubmitTask network timeout for: " + job.task_uuid);
            continue;
        }

        if (resp.status == GenTaskStatus::FAILED || resp.status == GenTaskStatus::CANCELLED)
        {
            // 服务端拒绝
            job.ApplyResponse(resp);
            UpdateFeedback(info);
            info.feedback.save_with_retry(fbPath, false);
            info.save_with_retry(filePathStr);
            MoveJobFile(filePathStr, qstr2str(genFailedJobPath));
            continue;
        }

        // 5. 提交成功 → 回填 freeze_no → 注册到 Block → 移到 Running
        job.ApplyResponse(resp);
        job.status = GenTaskStatus::PENDING;
        info.save_with_retry(filePathStr);
        UpdateFeedback(info);
        info.feedback.save_with_retry(fbPath, false);

        // 注册到 Block: 读 .blk → generations_info_.push_back → 写回
        // (job.project_path + job.block_item 定位 Block)
        {
            std::string blkPath = job.project_path + "/" + job.block_item + ".blk";
            BlockObject::Task_Info blkInfo;
            if (blkInfo.ReadBlockInfoBin(blkPath))
            {
                blk_generation_info_s genInfo;
                genInfo.generation_id = job.generation_id;
                genInfo.task_uuid = job.task_uuid;
                genInfo.job_name = info.job_name;
                genInfo.sub_type = static_cast<int>(job.params.sub_type);
                genInfo.status = static_cast<int>(GenTaskStatus::PENDING);
                genInfo.result_dir = job.result_dir;
                genInfo.created_time = QDateTime::currentDateTime().toString("yyyyMMddhhmmss").toStdString();
                blkInfo.generations_info_.push_back(genInfo);
                blkInfo.generationjobs_[job.task_uuid] = info.job_name;
                blkInfo.WriteBlockInfoToBin(blkPath, false);
            }
        }

        MoveJobFile(filePathStr, qstr2str(genRunningJobPath));

        LOGI("Submitted: " + job.task_uuid + " freeze_no=" + job.point_info.freeze_no);
    }
}

// ============================================================================
// ProcessRunningJobs — 轮询运行中的任务
//
// 流程:
//   1. 遍历 jobs_gen/Running/J_*
//   2. deny-write 锁 → 加载 job
//   3. 兜底: 无 freeze_no → 补 FreezeCredits                  ← ★ 积分冻结兜底
//   4. HTTP GET query 服务端状态
//   5. 根据返回处理 — 生成式后端 query 直接返回积分, 无需单独 settle:
//      COMPLETED/FAILED → ApplyResponse (含积分) → 更新 feedback/Block → 移目录
//      IN_PROGRESS → 更新 feedback (含积分余额)
//      重建式不同: 需调 SettlePoints 再更新状态 (见 积分接口集成方案.md)
//   6. 连续 5 次网络超时 → 标记 FAILED
// ============================================================================

void GenTaskThread::ProcessRunningJobs()
{
    QString runningDir = genRunningJobPath;
    QDirIterator it(runningDir, {"J_*.bin"}, QDir::Files);

    while (it.hasNext())
    {
        it.next();
        QString filePath = it.filePath();
        std::string filePathStr = filePath.toStdString();

        // 1. 加载 job 文件 (load_with_retry 内部已处理 deny-write 锁)
        GenJobFullInfo_s info;
        if (!info.load_with_retry(filePathStr))
            continue;
        GenJobInfo_s& job = info.job;
        if (job.point_info.freeze_no.empty())
        {
            continue;
        }

        // 1.5. 加载 feedback 到内存
        std::string fbPath = BuildFeedbackPath(info);
        info.feedback.load_with_retry(fbPath, false);
        // std::cout << "  [" << fbPath << "] status=" << (int)info.feedback.Status
        //    << " percent=" << info.feedback.Percent << "%" << std::endl;

        // 2. 查询服务端状态
        GenTaskResponse resp = GenHttpClient::QueryTaskStatus(job.point_info.freeze_no, job.params.provider_id);

        // 3. 网络超时 → 递增重试计数
        if (resp.status == GenTaskStatus::IDLE && resp.error_message.has_value())
        {
            job.query_retry_count++;
            if (job.query_retry_count >= 5)
            {
                LOGE("Task " + job.task_uuid + " query failed 5 times, moving to Failed");
                job.status = GenTaskStatus::FAILED;
                info.save_with_retry(filePathStr);
                UpdateFeedback(info);
                info.feedback.save_with_retry(fbPath, false);
                MoveJobFile(filePathStr, qstr2str(genFailedJobPath));
            }
            else
            {
                info.save_with_retry(filePathStr);
            }
            continue;
        }

        // 4. 重置重试计数 (成功获取到响应)
        job.query_retry_count = 0;

        switch (resp.status)
        {
        case GenTaskStatus::COMPLETED:
            {
                job.ApplyResponse(resp);
                info.save_with_retry(filePathStr);
                UpdateFeedback(info);
                info.feedback.save_with_retry(fbPath, false);
                // 更新 Block 中的状态和 result_url
                {
                    std::string blkPath = job.project_path + "/" + job.block_item + ".blk";
                    BlockObject::Task_Info blkInfo;
                    if (blkInfo.ReadBlockInfoBin(blkPath))
                    {
                        for (auto& gen : blkInfo.generations_info_)
                        {
                            if (gen.task_uuid == job.task_uuid)
                            {
                                gen.status = static_cast<int>(GenTaskStatus::COMPLETED);
                                if (!job.result_url.empty()) gen.result_url = job.result_url;
                                if (!job.preview_url.empty()) gen.preview_url = job.preview_url;
                                gen.consumed = job.point_info.consumed;
                                gen.refunded = job.point_info.refunded;
                                gen.total_balance = job.point_info.total_balance;
                                gen.available_points = job.point_info.available_points;
                                break;
                            }
                        }
                        blkInfo.WriteBlockInfoToBin(blkPath, false);
                    }
                }
                MoveJobFile(filePathStr, qstr2str(genCompletedJobPath));
                LOGI("Completed: " + job.task_uuid);
                break;
            }

        case GenTaskStatus::FAILED:
            {
                job.ApplyResponse(resp);
                info.save_with_retry(filePathStr);
                UpdateFeedback(info);
                info.feedback.save_with_retry(fbPath, false);
                // 更新 Block 中的状态
                {
                    std::string blkPath = job.project_path + "/" + job.block_item + ".blk";
                    BlockObject::Task_Info blkInfo;
                    if (blkInfo.ReadBlockInfoBin(blkPath))
                    {
                        for (auto& gen : blkInfo.generations_info_)
                        {
                            if (gen.task_uuid == job.task_uuid)
                            {
                                gen.status = static_cast<int>(GenTaskStatus::FAILED);
                                gen.consumed = job.point_info.consumed;
                                gen.refunded = job.point_info.refunded;
                                gen.total_balance = job.point_info.total_balance;
                                gen.available_points = job.point_info.available_points;
                                break;
                            }
                        }
                        blkInfo.WriteBlockInfoToBin(blkPath, false);
                    }
                }
                MoveJobFile(filePathStr, qstr2str(genFailedJobPath));
                LOGE("Failed: " + job.task_uuid);
                break;
            }

        case GenTaskStatus::IN_PROGRESS:
            {
                // 进度更新: ApplyResponse 回填 progress/result_url/preview_url 等
                // 再 UpdateFeedback 写到 feedback 文件供前端轮询
                job.ApplyResponse(resp);
                info.save_with_retry(filePathStr);
                UpdateFeedback(info);
                info.feedback.save_with_retry(fbPath, false);
                break;
            }

        case GenTaskStatus::CANCELLED:
            {
                // 服务端返回取消 (用户可能通过其他渠道取消)
                job.status = GenTaskStatus::CANCELLED;
                if (resp.error_message.has_value())
                    job.error_message = resp.error_message.value();
                info.save_with_retry(filePathStr);
                UpdateFeedback(info);
                info.feedback.save_with_retry(fbPath, false);
                // 更新 Block 中的状态
                {
                    std::string blkPath = job.project_path + "/" + job.block_item + ".blk";
                    BlockObject::Task_Info blkInfo;
                    if (blkInfo.ReadBlockInfoBin(blkPath))
                    {
                        for (auto& gen : blkInfo.generations_info_)
                        {
                            if (gen.task_uuid == job.task_uuid)
                            {
                                gen.status = static_cast<int>(GenTaskStatus::CANCELLED);
                                break;
                            }
                        }
                        blkInfo.WriteBlockInfoToBin(blkPath, false);
                    }
                }
                MoveJobFile(filePathStr, qstr2str(genCancelledJobPath));
                LOGI("Cancelled by server: " + job.task_uuid);
                break;
            }

        default:
            break;
        }
    }
}

void GenTaskThread::SleepMs(int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// ============================================================================
// CancelGenTask — TODO: 取消生成式任务
// 流程: 遍历 jobs_gen/ → 找 task_uuid → HTTP cancel → 更新 feedback + Block → 移 Cancelled/
bool GenTaskThread::CancelGenTask(const std::string& task_uuid)
{
    // TODO
    return false;
}


void GenTaskThread::SearchUnnormalRunningJob()
{
    while (true) {
        if (bQuitingApplication)
            break;

        QString runningDir = genRunningJobPath;
        QDirIterator it(runningDir, {"J_*.bin"}, QDir::Files);
        QDateTime now = QDateTime::currentDateTime();

        while (it.hasNext()) {
            it.next();
            QFileInfo fi(it.filePath());
            std::string filePathStr = fi.absoluteFilePath().toStdString();

            qint64 secsSinceMod = fi.lastModified().secsTo(now);

            // 1. 超过 24h 未更新 → 标记失败
            if (secsSinceMod > 86400) {  // 24 * 3600
                GenJobFullInfo_s info;
                if (!info.load_with_retry(filePathStr)) {
                    // 损坏文件, 直接删除
                    QFile::remove(fi.absoluteFilePath());
                    continue;
                }

                LOGE("UnnormalRunning: " + info.job.task_uuid
                     + " stuck for " + std::to_string(secsSinceMod / 3600) + "h, moving to Failed");

                info.job.status = GenTaskStatus::FAILED;
                info.job.error_message = "task stuck in Running for over 24h";
                std::string fbPath = BuildFeedbackPath(info);
                info.feedback.load_with_retry(fbPath, false);
                info.save_with_retry(filePathStr);
                UpdateFeedback(info);
                info.feedback.save_with_retry(fbPath, false);
                MoveJobFile(filePathStr, qstr2str(genFailedJobPath));
                continue;
            }

            // 2. 超过 1h 无更新 且无 freeze_no → submit 阶段残留, 移回 Pending 重试
            if (secsSinceMod > 3600) {
                GenJobFullInfo_s info;
                if (!info.load_with_retry(filePathStr))
                    continue;
                if (info.job.point_info.freeze_no.empty()) {
                    LOGW("UnnormalRunning: " + info.job.task_uuid
                         + " no freeze_no for 1h, moving back to Pending");
                    MoveJobFile(filePathStr, qstr2str(genPendingJobPath));
                }
            }
        }

        SleepMs(30000);  // 30s 扫描间隔
    }
}
