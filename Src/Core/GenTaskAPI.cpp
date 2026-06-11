// Src/Core/GenTaskAPI.cpp
#include "Core/GenTaskAPI.h"
#include "Core/BlockObject.h"
#include "Util/GenTaskProcess.h"
#include "Util/TaskProcess.h"
#include "Core/Types.h"
#include "Core/Logging.h"
#include <QUuid>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QHostInfo>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <fstream>

namespace AI3D
{
    namespace CORE
    {
        // ============================================================================
        // SubmitGenTask
        // ============================================================================
        SubmitResult GenTaskAPI::SubmitGenTask(
            BlockObject::Task_Info& blockInfo, // 非 const: 内部自增 next_generation_id
            const std::string& user_account,
            const std::string& pendingJobPath)
        {
            SubmitResult result;

            if (blockInfo.block_task_category != 1)
            {
                result.success = false;
                result.result_code = AI3D_FAILURE;
                result.error_msg = "Block does not support generative tasks";
                return result;
            }

            // ================================================================
            // ★ 积分预检 (提交前必须通过)
            // ================================================================
            {
                std::string businessType = ToString(blockInfo.gen_options.gen_params.sub_type);
                std::string taskParamJson = blockInfo.gen_options.gen_params.ToJsonString();

                PointFreezeInfo estimate = PointManager::EstimateTaskPoints(businessType, taskParamJson);
                if (!estimate.requestSucceeded)
                {
                    result.success = false;
                    result.error_msg = "http request error";
                    return result;
                }
                result.estimate_points = estimate.estimate_points;

                if (result.estimate_points < 0)
                {
                    result.success = false;
                    result.result_code = AI3D_FAILURE;
                    result.error_msg = "Failed to estimate task points";
                    return result;
                }

                PointFreezeInfo balance = PointManager::QueryUserPoints();
                if (!balance.requestSucceeded)
                {
                    result.success = false;
                    result.point_check_passed = false;
                    result.result_code = balance.errorCode;
                    result.error_msg = "http request error";
                    return result;
                }
                result.available_points = balance.available_points;
                result.total_balance = balance.total_balance;

                if (result.estimate_points > result.available_points)
                {
                    result.point_check_passed = false;
                    result.success = false;
                    result.result_code = AI3D_INSUFFICIENT_POINTS;
                    result.error_msg = "Insufficient points: need " + std::to_string(result.estimate_points)
                        + ", have " + std::to_string(result.available_points);
                    return result;
                }
                result.point_check_passed = true;
            }

            // 从持久化计数器取 generation_id, 取后自增 (只增不复用)
            int generation_id = blockInfo.next_generation_id++;

            GenJobFullInfo_s fullInfo;
            fullInfo.job_name = "J_" + blockInfo.blockName + "_"
                + QDateTime::currentDateTime().toString("yyyyMMddhhmmss").toStdString();

            GenJobInfo_s& job = fullInfo.job;
            job.task_uuid = user_account + "_"
                + QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz").toStdString()
                + "_" + QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString().substr(0, 4);
            job.engine_id = QHostInfo::localHostName().toStdString();
            job.user_account = user_account;
            job.project_path = blockInfo.projectfile_;
            job.block_item = blockInfo.blockName;
            job.params = blockInfo.gen_options.gen_params;
            job.status = GenTaskStatus::IDLE;

            // 结果目录: Generations/Generation_<id>/
            std::string generationDir = std::string(GENERATION_DIR) + "/" + GENERATION_PREFIX + std::to_string(
                generation_id);
            job.result_dir = blockInfo.projectfile_ + "/" + blockInfo.blockName + "/" + generationDir;
            QDir().mkpath(QString::fromStdString(job.result_dir));

            QDir().mkpath(QString::fromStdString(pendingJobPath));
            std::string jobFilePath = pendingJobPath + fullInfo.job_name + ".bin";
            if (!fullInfo.save_with_retry(jobFilePath))
            {
                result.success = false;
                result.result_code = AI3D_INSUFFICIENT_POINTS;
                result.error_msg = "Failed to write job file: " + jobFilePath;
                return result;
            }

            std::string feedbackPath = job.result_dir + "/JF_" + fullInfo.job_name
                + (JOB_FEEDBACK_USE_BIN ? BINFILE_POSTFIX : JSONFILE_POSTFIX);
            fullInfo.feedback.Status = jobsta_e::STATUS_PENDDING;
            fullInfo.feedback.Percent = 0.0f;
            fullInfo.feedback.save_with_retry(feedbackPath, false);

            job.generation_id = generation_id; // 存入 job, 引擎后续使用

            result.success = true;
            result.result_code = AI3D_SUCCESS;
            result.task_uuid = job.task_uuid;
            result.job_name = fullInfo.job_name;
            result.generation_id = generation_id;
            return result;
        }

        // ============================================================================
        // GetResultDir — 通过 task_uuid 查找结果目录
        // ============================================================================
        std::string GenTaskAPI::GetResultDir(const std::string& task_uuid,
                                             const BlockObject::Task_Info& blockInfo)
        {
            for (auto& gen : blockInfo.generations_info_)
            {
                if (gen.task_uuid == task_uuid)
                    return gen.result_dir;
            }
            return "";
        }

        // ============================================================================
        // DownloadResultByTaskUuid — 自动定位 result_dir 下载, 支持进度回调
        // ============================================================================
        // bool GenTaskAPI::DownloadResultByTaskUuid(
        //     const std::string& task_uuid,
        //     const BlockObject::Task_Info& blockInfo,
        //     std::function<void(qint64, qint64)> progressCb)
        // {
        //     // 1. 从 generations_info_ 中查找 result_url 和 result_dir
        //     std::string resultUrl;
        //     std::string resultDir;
        //     for (auto& gen : blockInfo.generations_info_) {
        //         if (gen.task_uuid == task_uuid) {
        //             resultUrl = gen.result_url;
        //             resultDir = gen.result_dir;
        //             break;
        //         }
        //     }
        //     if (resultUrl.empty()) {
        //         LOGE("DownloadResultByTaskUuid: task_uuid not found or result_url empty");
        //         return false;
        //     }
        //     if (resultDir.empty()) {
        //         LOGE("DownloadResultByTaskUuid: result_dir is empty");
        //         return false;
        //     }
        //
        //     // 2. 确定保存路径: result_dir/result.glb (后续可按 sub_type 选择扩展名)
        //     std::string savePath = resultDir + "/result.glb";
        //
        //     // 3. 下载
        //     QNetworkAccessManager manager;
        //     QUrl url(QString::fromStdString(resultUrl));
        //     QNetworkRequest request(url);
        //     request.setTransferTimeout(60000);
        //     QNetworkReply* reply = manager.get(request);
        //
        //     // 进度回调
        //     if (progressCb) {
        //         QObject::connect(reply, &QNetworkReply::downloadProgress,
        //                          [&](qint64 received, qint64 total) {
        //                              progressCb(received, total);
        //                          });
        //     }
        //
        //     QEventLoop loop;
        //     QTimer timer;
        //     timer.setSingleShot(true);
        //     QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        //     QObject::connect(&timer, &QTimer::timeout, [&]() { reply->abort(); loop.quit(); });
        //     timer.start(60000);
        //     loop.exec();
        //
        //     if (reply->error() != QNetworkReply::NoError) {
        //         LOGE("DownloadResultByTaskUuid: " + reply->errorString().toStdString());
        //         reply->deleteLater();
        //         return false;
        //     }
        //     QByteArray data = reply->readAll();
        //     reply->deleteLater();
        //
        //     QFile file(QString::fromStdString(savePath));
        //     if (!file.open(QIODevice::WriteOnly)) {
        //         LOGE("DownloadResultByTaskUuid: cannot write to " + savePath);
        //         return false;
        //     }
        //     file.write(data);
        //     file.close();
        //     return true;
        // }

        // ============================================================================
        // DownloadResult — 指定 URL 和路径直接下载 (保留, 供特殊场景)
        // ============================================================================
        // bool GenTaskAPI::DownloadResult(const std::string& result_url,
        //                                  const std::string& save_path)
        // {
        //     if (result_url.empty()) { LOGE("DownloadResult: result_url is empty"); return false; }
        //
        //     QNetworkAccessManager manager;
        //     QUrl url(QString::fromStdString(result_url));
        //     QNetworkRequest request(url);
        //     request.setTransferTimeout(30000);
        //     QNetworkReply* reply = manager.get(request);
        //     QEventLoop loop;
        //     QTimer timer;
        //     timer.setSingleShot(true);
        //     QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        //     QObject::connect(&timer, &QTimer::timeout, [&]() { reply->abort(); loop.quit(); });
        //     timer.start(30000);
        //     loop.exec();
        //     if (reply->error() != QNetworkReply::NoError) {
        //         LOGE("DownloadResult: " + reply->errorString().toStdString());
        //         reply->deleteLater();
        //         return false;
        //     }
        //     QByteArray data = reply->readAll();
        //     reply->deleteLater();
        //     QFile file(QString::fromStdString(save_path));
        //     if (!file.open(QIODevice::WriteOnly)) { LOGE("DownloadResult: cannot write"); return false; }
        //     file.write(data);
        //     file.close();
        //     return true;
        // }

        // ============================================================================
        // RequestCancel — TODO: 取消任务
        // ============================================================================
        // bool GenTaskAPI::RequestCancel(const std::string& task_uuid)
        // {
        //     // TODO
        //     return false;
        // }

        // ============================================================================
        // QueryCredits — TODO: 对接 Triverse 积分查询 API
        // ============================================================================
        // int GenTaskAPI::QueryCredits(const std::string& user_account)
        // {
        //     // TODO: GenHttpClient 发 GET /api/v1/credits?user=xxx → 解析 points_balance
        //     return -1;
    }
}
