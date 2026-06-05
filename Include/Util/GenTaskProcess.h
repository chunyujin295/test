#pragma once

#include <string>
#include <vector>
#include <optional>
#include <thread>
#include <chrono>

#include "Core/json.h"
#include "Core/File.h"
#include "Core/Logging.h"
#include "Core/DataStruct.h"
#include "Core/GenTaskOptions.h"
#include "Core/PointManager.h"
#include "Core/Types.h"
#include "Util/TaskProcess.h"

namespace AI3D
{
    namespace CORE
    {
        // ========= enum =========

        using AI3D::CORE::GenTaskStatus;

        struct GenTaskResponse
        {
            std::string task_id;
            std::optional<std::string> server_task_id;
            GenTaskStatus status = GenTaskStatus::IDLE;
            int progress = 0;
            std::optional<std::string> result_url;
            std::optional<std::string> preview_url; // image url for preview
            std::optional<std::string> error_message;
        };

        struct GenJobInfo_s
        {
            std::string task_uuid;
            std::string engine_id;
            std::string user_account;

            std::string project_path;
            std::string block_item;

            GenTaskParams params;

            std::string server_task_id;
            GenTaskStatus status = GenTaskStatus::IDLE;
            int progress = 0;
            std::string result_url;
            std::string preview_url; // image url for preview
            std::string result_path;
            std::string preview_path;
            std::string error_message;
            int query_retry_count = 0;

            PointInfoBase point_info;

            void ApplyResponse(const GenTaskResponse& resp)
            {
                if (resp.server_task_id)
                {
                    this->server_task_id = *resp.server_task_id;
                }

                if (resp.result_url)
                {
                    this->result_url = *resp.result_url;
                }

                if (resp.preview_url)
                {
                    this->preview_url = *resp.preview_url;
                }

                if (resp.error_message)
                {
                    this->error_message = *resp.error_message;
                }

                this->status = resp.status;
                progress = resp.progress;
            }
        };

        struct GenJobFullInfo_s
        {
            std::string job_name;
            GenJobInfo_s job;
            JobFeedBack_s feedback;

            GenJobFullInfo_s()
            {
            };

            GenJobFullInfo_s(const std::string& file)
            {
                load(file);
            }

            bool WriteToBin(const std::string& filePath) const
            {
                std::ofstream out = File::OpenOfstreamUtf8(filePath, std::ios::binary);
                if (!out.is_open())
                {
                    LOGE("GenJobFullInfo)s::WriteToBin: failed to open: " + filePath);
                    return false;
                }

                GenJobFile genJobFile;
                genJobFile.jobName = job_name;

                GenJobInfoData& d = genJobFile.genJobInfoData;

                d.task_uuid = job.task_uuid;
                d.job_name = job_name;
                d.engine_id = job.engine_id;
                d.user_account = job.user_account;
                d.project_path = job.project_path;
                d.block_item = job.block_item;
                d.params_json = job.params.ToJsonString();
                d.status = static_cast<int>(job.status);
                d.server_task_id = job.server_task_id;
                d.result_url = job.result_url;
                d.preview_url = job.preview_url;
                d.result_path = job.result_path;
                d.preview_path = job.preview_path;
                d.error_message = job.error_message;
                d.query_retry_count = job.query_retry_count;

                genJobFile.feedBackData.status = static_cast<int>(feedback.Status);
                genJobFile.feedBackData.percent = feedback.Percent;
                genJobFile.feedBackData.taskRetVal = feedback.TaskRetVal;
                genJobFile.feedBackData.msg = feedback.Msg;

                genJobFile.Serialize(out);
                out.close();
                return true;
            }

            bool LoadFromBin(const std::string& filePath)
            {
                std::ifstream in = File::OpenIfstreamUtf8(filePath, std::ios::binary);
                if (!in.is_open())
                {
                    return false;
                }

                GenJobFile genJobFile;
                if (!genJobFile.Deserialize(in))
                {
                    in.close();
                    return false;
                }
                in.close();

                job_name = genJobFile.jobName;

                GenJobInfoData& d = genJobFile.genJobInfoData;
                job.task_uuid = d.task_uuid;
                job.engine_id = d.engine_id;
                job.user_account = d.user_account;
                job.project_path = d.project_path;
                job.block_item = d.block_item;
                job.params = GenTaskParams::CreateFromJsonString(d.params_json);
                job.status = static_cast<GenTaskStatus>(d.status);
                job.server_task_id = d.server_task_id;
                job.result_url = d.result_url;
                job.preview_url = d.preview_url;
                job.result_path = d.result_path;
                job.preview_path = d.preview_path;
                job.error_message = d.error_message;
                job.query_retry_count = d.query_retry_count;

                feedback.Status = static_cast<jobsta_e>(genJobFile.feedBackData.status);
                feedback.Percent = genJobFile.feedBackData.percent;
                feedback.TaskRetVal = genJobFile.feedBackData.taskRetVal;
                feedback.Msg = genJobFile.feedBackData.msg;

                return true;
            }

            bool save(const std::string& filePath) const
            {
                bool result = WriteToBin(filePath);
                if (!result)
                {
                    LOGE("GenJobFullInfo_s::save: writeToBin failed: " + filePath);
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

            bool save_with_retry(const std::string& filePath) const
            {
                int retryTimes = 0;
                do
                {
                    FILE* fpLock = File::FopenDenyWriteLockUtf8(filePath + ".lock");
                    if (fpLock == nullptr)
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
                    if (result)
                    {
                        return true;
                    }

                    retryTimes++;
                    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                }
                while (retryTimes < 3);

                LOGE(std::string("GenJobFullInfo_s::load_with_retry failed after 3 retries: ") + filePath);
                return false;
            }
        };
    }
}
