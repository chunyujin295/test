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

// ============================================================================
// LoadAccessToken — 从注册表读取 token (复制自 HttpClient::post())
// ============================================================================

#define MOCK_GEN_HTTP // TODO CYJ
QString GenHttpClient::LoadAccessToken()
{
    QSettings settings(SETTINGS_PREFIX + "\\User", QSettings::NativeFormat);
    QString currentUser = settings.value(CURRENT_PREFIX, "").toString();
    if (currentUser.isEmpty())
        return "";
    QString tokenKey = TOKEN_PREFIX + currentUser;
    return settings.value(tokenKey, "").toString();
}

// ============================================================================
// BuildAuthHeader — 计算签名 (算法与 HttpClient::calSign() 完全一致)
// ============================================================================

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


// GenHttpClient.cpp — 在 SubmitTask / QueryTaskStatus / CancelTask / UploadFile 函数体开头:

#ifdef MOCK_GEN_HTTP // TODO CYJ
// ===== SubmitTask mock =====
GenTaskResponse GenHttpClient::SubmitTask(const std::string& task_uuid,
                                          const std::string& user_account,
                                          const GenTaskParams& genParams,
                                          int, int)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    GenTaskResponse resp;
    resp.task_id = task_uuid;
    resp.server_task_id = "mock-trv-" + task_uuid.substr(0, 8);
    resp.status = GenTaskStatus::IN_PROGRESS;
    resp.progress = 0;
    return resp;
}

// ===== QueryTaskStatus mock (模拟 3 次进度 → 完成) =====
GenTaskResponse GenHttpClient::QueryTaskStatus(const std::string& server_task_id,
                                               int, int)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    GenTaskResponse resp;
    resp.server_task_id = server_task_id;
    resp.status = GenTaskStatus::IN_PROGRESS;

    // mock 计数器: 调用 3 次后标记完成
    static std::map<std::string, int> pollCount;
    int& count = pollCount[server_task_id];
    count++;
    resp.progress = count * 33;

    if (count >= 3)
    {
        resp.status = GenTaskStatus::COMPLETED;
        resp.progress = 100;
        resp.result_url = "https://mock-cdn.example.com/" + server_task_id + "/result.glb";
        resp.preview_url = "https://mock-cdn.example.com/" + server_task_id + "/preview.png";
    }
    return resp;
}

// ===== CancelTask mock =====
bool GenHttpClient::CancelTask(const std::string&, int, int) { return true; }

// ===== UploadFile mock =====
// std::string GenHttpClient::UploadFile(const std::string&, int, int) { return "mock-fk-12345"; }

#else// MOCK_GEN_HTTP

// ============================================================================
// SubmitTask — POST /api/v1/tasks/<type> — URL 由 genParams.sub_type 经 ToString() 生成
// ============================================================================

GenTaskResponse GenHttpClient::SubmitTask(const std::string& task_uuid,
                                          const std::string& user_account,
                                          const GenTaskParams& genParams,
                                          int timeout_ms,
                                          int max_retries)
{
    QString url = GEN_SERVER_URL + GEN_API_PREFIX + "/tasks/" + QString::fromUtf8(ToString(genParams.sub_type));

    // 构造扁平 QMap — GenTaskParams 通过 ToJsonString() 序列化后作为字符串值
    QMap<QString, QString> params;
    params["task_id"] = QString::fromStdString(task_uuid);
    params["user_account"] = QString::fromStdString(user_account);
    params["params"] = QString::fromStdString(genParams.ToJsonString());

    QMap<QString, QString> headers;

    for (int attempt = 0; attempt <= max_retries; attempt++)
    {
        GenTaskResponse response;
        bool ok = false;

        HttpClient client(nullptr);
        client.post(url, params, headers, [&](int, int errorCode, QString errorMsg, QJsonObject doc)
        {
            if (errorCode == 0)
            {
                response.task_id = task_uuid;
                if (doc.contains("server_task_id"))
                    response.server_task_id = doc["server_task_id"].toString().toStdString();
                response.status = static_cast<GenTaskStatus>(doc.value("status").toInt());
                response.progress = doc.value("progress").toInt();
                if (doc.contains("result_url"))
                    response.result_url = doc["result_url"].toString().toStdString();
                if (doc.contains("preview_url"))
                    response.preview_url = doc["preview_url"].toString().toStdString();
                if (doc.contains("error_message"))
                    response.error_message = doc["error_message"].toString().toStdString();
                ok = true;
            }
            else
            {
                response.task_id = task_uuid;
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
    failResp.task_id = task_uuid;
    failResp.status = GenTaskStatus::IDLE;
    failResp.error_message = "submit timeout after " + std::to_string(max_retries + 1) + " attempts";
    return failResp;
}

// ============================================================================
// QueryTaskStatus — GET /api/v1/task/status (委托 HttpClient::get)
//                   手动计算 Authorization 头 (HttpClient::get 不带鉴权)
// ============================================================================

GenTaskResponse GenHttpClient::QueryTaskStatus(const std::string& server_task_id,
                                               int timeout_ms,
                                               int max_retries)
{
    QString url = GEN_SERVER_URL + GEN_API_PREFIX + "/task/status?task_id="
        + QString::fromStdString(server_task_id);

    // HttpClient::get 不带鉴权, 手动计算并通过 headers 传入
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
                response.task_id = doc.value("task_id").toString().toStdString();
                if (doc.contains("server_task_id"))
                    response.server_task_id = doc["server_task_id"].toString().toStdString();
                response.status = static_cast<GenTaskStatus>(doc.value("status").toInt());
                response.progress = doc.value("progress").toInt();
                if (doc.contains("result_url"))
                    response.result_url = doc["result_url"].toString().toStdString();
                if (doc.contains("preview_url"))
                    response.preview_url = doc["preview_url"].toString().toStdString();
                if (doc.contains("error_message"))
                    response.error_message = doc["error_message"].toString().toStdString();
                ok = true;
            }
            else
            {
                response.server_task_id = server_task_id;
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

bool GenHttpClient::CancelTask(const std::string& server_task_id,
                               int timeout_ms,
                               int max_retries)
{
    // QString url = GEN_SERVER_URL + GEN_API_PREFIX + "/task/cancel";
    //
    // QMap<QString, QString> params;
    // params["task_id"] = QString::fromStdString(server_task_id);
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

    return false;
}

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
//                 return doc.value("file_key").toString().toStdString();
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
