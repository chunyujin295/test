#pragma once

#include "Util/GenTaskProcess.h"
#include <QString>
#include <QMap>
using namespace AI3D::CORE;

class GenHttpClient
{
public:
    static GenTaskResponse SubmitTask(const std::string& task_uuid,
                                      const std::string& user_account,
                                      const GenTaskParams& genParams,
                                      int timeout_ms = 5000,
                                      int max_retries = 3);

    static GenTaskResponse QueryTaskStatus(const std::string& server_task_id,
                                           int timeout_ms = 3000,
                                           int max_retries = 3);

    static bool CancelTask(const std::string& server_task_id,
                           int timeout_ms = 3000,
                           int max_retries = 3);

private:
    static QString BuildAuthHeader(const QString& url,
                                   const QString& dataJson = "");

    static QString LoadAccessToken();

    ///  POST multipart/form-data
    // static QByteArray SyncPostMultipart(const QString& url,
    //                                     const QString& filePath,
    //                                     int timeout_ms)
};
