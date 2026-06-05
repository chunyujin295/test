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
        GenTaskAPI::SubmitResult GenTaskAPI::SubmitGenTask(
            const BlockObject::Task_Info& blockInfo,
            const std::string& user_account,
            const std::string& pendingJobPath)
        {
            SubmitResult result;

            if (blockInfo.block_task_category != 1)
            {
                result.success = false;
                result.error_msg = "Block does not support generative tasks";
                return result;
            }

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

            QString resultDir = QString::fromStdString(blockInfo.projectfile_)
                + "/" + QString::fromStdString(blockInfo.blockName)
                + "/" + QString::fromStdString(fullInfo.job_name);
            QDir().mkpath(resultDir);

            QDir().mkpath(QString::fromStdString(pendingJobPath));
            std::string jobFilePath = pendingJobPath + fullInfo.job_name + ".bin";
            if (!fullInfo.save_with_retry(jobFilePath))
            {
                result.success = false;
                result.error_msg = "Failed to write job file: " + jobFilePath;
                return result;
            }

            std::string feedbackPath = blockInfo.projectfile_ + "/"
                + blockInfo.blockName + "/JF_" + fullInfo.job_name
                + (JOB_FEEDBACK_USE_BIN ? BINFILE_POSTFIX : JSONFILE_POSTFIX);
            fullInfo.feedback.Status = jobsta_e::STATUS_PENDDING;
            fullInfo.feedback.Percent = 0.0f;
            fullInfo.feedback.save_with_retry(feedbackPath, false);

            result.success = true;
            result.task_uuid = job.task_uuid;
            result.job_name = fullInfo.job_name;
            return result;
        }

        bool GenTaskAPI::DownloadResult(const std::string& result_url,
                                        const std::string& save_path)
        {
            if (result_url.empty())
            {
                LOGE("DownloadResult: result_url is empty");
                return false;
            }

            QNetworkAccessManager manager;
            QNetworkRequest request(QUrl(QString::fromStdString(result_url)));
            request.setTransferTimeout(30000);
            QNetworkReply* reply = manager.get(request);
            QEventLoop loop;
            QTimer timer;
            timer.setSingleShot(true);
            QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
            QObject::connect(&timer, &QTimer::timeout, [&]()
            {
                reply->abort();
                loop.quit();
            });
            timer.start(30000);
            loop.exec();
            if (reply->error() != QNetworkReply::NoError)
            {
                LOGE("DownloadResult: " + reply->errorString().toStdString());
                reply->deleteLater();
                return false;
            }
            QByteArray data = reply->readAll();
            reply->deleteLater();
            QFile file(QString::fromStdString(save_path));
            if (!file.open(QIODevice::WriteOnly))
            {
                LOGE("DownloadResult: cannot write");
                return false;
            }
            file.write(data);
            file.close();
            return true;
        }

        bool GenTaskAPI::RequestCancel(const std::string& task_uuid)
        {
            // TODO
            return false;
        }

        // ============================================================================
        // QueryCredits — TODO: 对接 Triverse 积分查询 API
        // ============================================================================
        int GenTaskAPI::QueryCredits(const std::string& user_account)
        {
            // TODO:
            return -1;
        }
    }
} // namespace AI3D::CORE
