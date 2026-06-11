#include "GenHttpClient.h"
#include "Util/HttpClient.h"       // 复用现有 HttpClient::post/get
#include "Util/constant.h"         // SETTINGS_PREFIX / CURRENT_PREFIX / TOKEN_PREFIX
#include "Core/Logging.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHttpMultiPart>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QSettings>
#include <QDateTime>
#include <QCryptographicHash>
#include <thread>
#include <chrono>

using namespace AI3D::CORE;

QString GenHttpClient::LoadAccessToken()
{
    QSettings settings(SETTINGS_PREFIX + "\\User", QSettings::NativeFormat);
    QString currentUser = settings.value(CURRENT_PREFIX, "").toString();
    if (currentUser.isEmpty())
        return "";
    QString tokenKey = TOKEN_PREFIX + currentUser;
    return settings.value(tokenKey, "").toString();
}

QString GenHttpClient::BuildAuthHeader(const QString& url,
                                       const QString& dataJson)
{
    QString path = url;
    path.remove(GEN_SERVER_URL);

    QString timestampStr = QString::number(QDateTime::currentDateTime().toSecsSinceEpoch());
    QString accessToken = LoadAccessToken();

    QString finalStr = "moldai:" + path + ":" + timestampStr;
    if (!dataJson.isEmpty())
        finalStr += ":" + dataJson;
    if (!accessToken.isEmpty())
        finalStr += ":" + accessToken;

    QByteArray signBase64 = QCryptographicHash::hash(
        finalStr.toUtf8(), QCryptographicHash::Md5).toBase64();

    QString authHeader = "timestamp:" + timestampStr + ",sign:" + signBase64;
    if (!accessToken.isEmpty())
        authHeader += ",accessToken:" + accessToken;

    return authHeader;
}

#define MOCK_GEN_HTTP // TODO CYJ
// GenHttpClient.cpp — 在 SubmitTask / QueryTaskStatus / CancelTask / UploadFile 函数体开头:

#ifdef MOCK_GEN_HTTP // TODO CYJ
// ===== SubmitTask mock =====
GenTaskResponse GenHttpClient::SubmitTask(const std::string& task_uuid,
                                          const GenTaskParams& genParams,
                                          int, int)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    GenTaskResponse resp;
    resp.task_uuid = task_uuid;
    resp.status = GenTaskStatus::IN_PROGRESS;
    resp.progress = 0;
    resp.freeze_no = "123123-test-test";
    return resp;
}

// ===== QueryTaskStatus mock (模拟 3 次进度 → 完成) =====
GenTaskResponse GenHttpClient::QueryTaskStatus(const std::string& freeze_no,
                                int provider_id,
                                int timeout_ms,
                                int max_retries)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    GenTaskResponse resp;
    resp.status = GenTaskStatus::IN_PROGRESS;

    // mock 计数器: 调用 3 次后标记完成
    static std::map<std::string, int> pollCount;
    int& count = pollCount[freeze_no];
    count++;
    resp.progress = count * 33;

    if (count >= 3)
    {
        resp.status = GenTaskStatus::COMPLETED;
        resp.progress = 100;
        resp.result_url = "https://mock-cdn.example.com/" + freeze_no + "/result.glb";
    }
    resp.freeze_no = freeze_no;
    return resp;
}

// ===== CancelTask mock =====
// bool GenHttpClient::CancelTask(const std::string&, int, int) { return true; }

// ===== UploadFile mock =====
// std::string GenHttpClient::UploadFile(const std::string&, int, int) { return "mock-fk-12345"; }

#else// MOCK_GEN_HTTP

// ============================================================================
// SubmitTask — POST /api/v1/tasks/<type> — URL 由 genParams.sub_type 经 ToString() 生成
// ============================================================================

GenTaskResponse GenHttpClient::SubmitTask(const std::string& task_uuid,
                                          const GenTaskParams& genParams,
                                          int timeout_ms,
                                          int max_retries)
{
    QString url = GEN_SERVER_URL + GEN_API_PREFIX + "/generation/create3DTask" + QString::fromUtf8(
        ToString(genParams.sub_type));

    // 构造扁平 QMap — GenTaskParams 通过 ToJsonString() 序列化后作为字符串值
    QMap<QString, QString> params;
    params["task_uuid"] = QString::fromStdString(task_uuid);
    params["param"] = QString::fromStdString(genParams.ToJsonString());

    QMap<QString, QString> headers;
    headers["Authorization"] = BuildAuthHeader(url, params["param"]);

    for (int attempt = 0; attempt <= max_retries; attempt++)
    {
        GenTaskResponse response;
        bool ok = false;

        HttpClient client(nullptr);
        client.post(url, params, headers, [&](int, int errorCode, QString errorMsg, QJsonObject doc)
        {
            if (errorCode == 0 && doc.contains("data"))
            {
                QJsonObject data = doc["data"].toObject();
                response.task_uuid = task_uuid;
                if (data.contains("freeze_no"))
                    response.freeze_no = data["freeze_no"].toString().toStdString();
                response.status = static_cast<GenTaskStatus>(data.value("status").toInt());
                response.progress = data.value("progress").toInt();
                response.available_points = data.value("available_points").toInt();
                response.frozen_points = data.value("frozen_points").toInt();
                response.total_balance = data.value("total_balance").toInt();
                // output 仅 QueryTaskStatus 返回, SubmitTask 为 null
                if (data.contains("output") && !data["output"].isNull())
                {
                    QJsonObject output = data["output"].toObject();
                    if (output.contains("preview_url"))
                        response.preview_url = output["preview_url"].toString().toStdString();
                    // result_url: 优先 textured_model_url[0], 为空则取 geometry_model_url[0]
                    if (output.contains("textured_model_url"))
                    {
                        QJsonArray arr = output["textured_model_url"].toArray();
                        if (arr.size() > 0 && !arr[0].toString().isEmpty())
                            response.result_url = arr[0].toString().toStdString();
                    }
                    if (response.result_url.empty() && output.contains("geometry_model_url"))
                    {
                        QJsonArray arr = output["geometry_model_url"].toArray();
                        if (arr.size() > 0 && !arr[0].toString().isEmpty())
                            response.result_url = arr[0].toString().toStdString();
                    }
                }
                if (data.contains("error_message"))
                    response.error_message = data["error_message"].toString().toStdString();
                ok = true;
            }
            else
            {
                response.task_uuid = task_uuid;
                response.status = GenTaskStatus::IDLE;
                response.error_message = errorMsg.toStdString();
            }
        });

        if (ok && response.status != GenTaskStatus::IDLE)
            return response;

        if (attempt < max_retries)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    GenTaskResponse failResp;
    failResp.task_uuid = task_uuid;
    failResp.status = GenTaskStatus::IDLE;
    failResp.error_message = "submit timeout after " + std::to_string(max_retries + 1) + " attempts";
    return failResp;
}

// ============================================================================
// QueryTaskStatus — GET /api/v1/task/status (委托 HttpClient::get)
//                   手动计算 Authorization 头 (HttpClient::get 不带鉴权)
// ============================================================================

GenTaskResponse GenHttpClient::QueryTaskStatus(const std::string& freeze_no,
                                               int provider_id,
                                               int timeout_ms,
                                               int max_retries)
{
    QString url = GEN_SERVER_URL + GEN_API_PREFIX + "/generation/getTaskStatus";

    QMap<QString, QString> params;
    params["freeze_no"] = QString::fromStdString(freeze_no);
    params["provider_id"] = QString::number(provider_id);

    QMap<QString, QString> headers;
    headers["Authorization"] = BuildAuthHeader(url, "");

    for (int attempt = 0; attempt <= max_retries; attempt++)
    {
        GenTaskResponse response;
        bool ok = false;

        HttpClient client(nullptr);
        client.get(url, headers, [&](int, int errorCode, QString errorMsg, QJsonObject doc)
        {
            if (errorCode == 0)
            {
                response.task_uuid = doc.value("task_uuid").toString().toStdString();
                if (doc.contains("triverse_task_uuid"))
                    response.freeze_no = doc["triverse_task_uuid"].toString().toStdString();
                response.status = static_cast<GenTaskStatus>(doc.value("status").toInt());
                response.progress = doc.value("progress").toInt();
                if (doc.contains("result_url"))
                    response.result_url = doc["result_url"].toString().toStdString();
                if (doc.contains("error_message"))
                    response.error_message = doc["error_message"].toString().toStdString();
                // 积分字段已迁移到 PointInfoBase, 不再从 Task API 响应中解析
                ok = true;
            }
            else
            {
                response.status = GenTaskStatus::IDLE;
                response.error_message = errorMsg.toStdString();
            }
        });

        if (ok && response.status != GenTaskStatus::IDLE)
            return response;

        if (attempt < max_retries)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    GenTaskResponse failResp;
    failResp.status = GenTaskStatus::IDLE;
    failResp.error_message = "query timeout after retries";
    return failResp;
}

// ============================================================================
// CancelTask — POST /api/v1/task/cancel (委托 HttpClient::post)
// ============================================================================

// bool GenHttpClient::CancelTask(const std::string& server_task_id,
//                                int timeout_ms,
//                                int max_retries)
// {
    // QString url = GEN_SERVER_URL + GEN_API_PREFIX + "/task/cancel";
    //
    // QMap<QString, QString> params;
    // params["task_uuid"] = QString::fromStdString(server_task_id);
    //
    // QMap<QString, QString> headers;
    //
    // for (int attempt = 0; attempt <= max_retries; attempt++)
    // {
    //     bool success = false;
    //
    //     HttpClient client(nullptr);
    //     client.post(url, params, headers, [&](int, int errorCode, QString, QJsonObject doc)
    //     {
    //         success = (errorCode == 0 && doc.value("success").toBool(false));
    //     });
    //
    //     if (success)
    //         return true;
    //
    //     if (attempt < max_retries)
    //     {
    //         std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //     }
    // }
//
//     return false;
// }

// ============================================================================
// SyncPostMultipart — multipart 文件上传 (HttpClient 不支持, 自建实现)
// ============================================================================

// QByteArray GenHttpClient::SyncPostMultipart(const QString& url,
//                                             const QString& filePath,
//                                             int timeout_ms)
// {
//     QFile file(filePath);
//     if (!file.open(QIODevice::ReadOnly))
//     {
//         LOGE("UploadFile: cannot open " + filePath.toStdString());
//         return {};
//     }
//
//     QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
//
//     QHttpPart filePart;
//     QFileInfo fi(filePath);
//     QMimeDatabase mimeDb;
//     filePart.setHeader(QNetworkRequest::ContentTypeHeader, mimeDb.mimeTypeForFile(fi).name());
//     filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
//                        QString("form-data; name=\"file\"; filename=\"%1\"").arg(fi.fileName()));
//     filePart.setBodyDevice(&file);
//     file.setParent(multiPart);
//     multiPart->append(filePart);
//
//     QNetworkAccessManager manager;
//     QNetworkRequest request(QUrl (url));
//     request.setTransferTimeout(timeout_ms);
//     request.setRawHeader("Authorization", BuildAuthHeader(url, "").toUtf8());
//
//     QNetworkReply* reply = manager.post(request, multiPart);
//     multiPart->setParent(reply);
//
//     QEventLoop loop;
//     QTimer timer;
//     timer.setSingleShot(true);
//     QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
//     QObject::connect(&timer, &QTimer::timeout, [&]()
//     {
//         reply->abort();
//         loop.quit();
//     });
//     timer.start(timeout_ms);
//     loop.exec();
//
//     QByteArray result;
//     if (reply->error() == QNetworkReply::NoError)
//     {
//         result = reply->readAll();
//     }
//     else
//     {
//         LOGW("SyncPostMultipart failed: " + reply->errorString().toStdString());
//     }
//
//     reply->deleteLater();
//     return result;
// }

// ============================================================================
// UploadFile — POST /api/v1/upload (multipart, HttpClient 不支持)
// ============================================================================

// std::string GenHttpClient::UploadFile(const std::string& local_path,
//                                       int timeout_ms,
//                                       int max_retries)
// {
//     QString url = GEN_SERVER_URL + GEN_API_PREFIX + "/upload";
//
//     for (int attempt = 0; attempt <= max_retries; attempt++)
//     {
//         QByteArray raw = SyncPostMultipart(url, QString::fromStdString(local_path), timeout_ms);
//
//         if (!raw.isEmpty())
//         {
//             QJsonObject doc = QJsonDocument::fromJson(raw).object();
//             if (doc.value("errorCode").toInt() == 0)
//             {
//                 return doc.value("upload_file_key").toString().toStdString();
//             }
//         }
//
//         if (attempt < max_retries)
//         {
//             std::this_thread::sleep_for(std::chrono::milliseconds(500));
//         }
//     }
//
//     return "";
// }

#endif
