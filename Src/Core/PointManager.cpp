// ==================== Src/Core/PointManager.cpp ====================
#include "Core/PointManager.h"
#include "Util/HttpClient.h"
#include "Util/constant.h"         // SETTINGS_PREFIX / CURRENT_PREFIX / TOKEN_PREFIX
#include "Core/Types.h"            // QString(GEN_SERVER_URL) / QString(GEN_API_PREFIX)
#include "Core/Logging.h"
#include <QSettings>
#include <QDateTime>
#include <QCryptographicHash>
#include <thread>
#include <chrono>

namespace AI3D
{
    namespace CORE
    {
        // ============================== 鉴权工具 ==============================

        QString PointManager::loadAccessToken()
        {
            QSettings settings(SETTINGS_PREFIX + "\\User", QSettings::NativeFormat);
            QString currentUser = settings.value(CURRENT_PREFIX, "").toString();
            if (currentUser.isEmpty())
                return "";
            QString tokenKey = TOKEN_PREFIX + currentUser;
            return settings.value(tokenKey, "").toString();
        }

        QString PointManager::buildAuthHeader(const QString& url, const QString& dataJson)
        {
            QString path = url;
            path.remove(QString(GEN_SERVER_URL));

            QString timestampStr = QString::number(QDateTime::currentDateTime().toSecsSinceEpoch());
            QString accessToken = loadAccessToken();

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

        // =========================== /point/outline ==========================

        PointFreezeInfo PointManager::postPointsOutline(int timeout_ms, int max_retries)
        {
            QString url = QString(GEN_SERVER_URL) + QString(GEN_API_PREFIX) + "/point/outline";
            QMap<QString, QString> headers;
            headers["Authorization"] = buildAuthHeader(url, "");

            for (int attempt = 0; attempt <= max_retries; attempt++)
            {
                PointFreezeInfo result;
                bool ok = false;

                HttpClient client(nullptr);
                client.post(url, {}, headers, [&](int, int errorCode, QString, QJsonObject doc)
                {
                    if (errorCode == 0 && doc.value("errorCode").toInt() == 0)
                    {
                        QJsonObject data = doc["data"].toObject();
                        result.total_balance = data.value("total_balance").toInt();
                        result.frozen_points = data.value("frozen_points").toInt();
                        result.available_points = data.value("available_points").toInt();
                        ok = true;
                    }
                });

                if (ok) return result;
                if (attempt < max_retries)
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            return PointFreezeInfo{};
        }

        // =========================== /point/freeze ===========================

        PointFreezeInfo PointManager::postPointsFreeze(const std::string& business_type,
                                                       const std::string& task_param_json,
                                                       int timeout_ms, int max_retries)
        {
            QString url = QString(GEN_SERVER_URL) + QString(GEN_API_PREFIX) + "/point/freeze";
            QString qTaskParam = QString::fromStdString(task_param_json);

            QMap<QString, QString> params;
            params["business_type"] = QString::fromUtf8(business_type.c_str());
            params["task_param"] = qTaskParam;

            QMap<QString, QString> headers;
            headers["Authorization"] = buildAuthHeader(url, qTaskParam);

            for (int attempt = 0; attempt <= max_retries; attempt++)
            {
                PointFreezeInfo result;
                bool ok = false;

                HttpClient client(nullptr);
                client.post(url, params, headers, [&](int, int errorCode, QString, QJsonObject doc)
                {
                    if (errorCode == 0 && doc.value("errorCode").toInt() == 0)
                    {
                        QJsonObject data = doc["data"].toObject();
                        result.freeze_no = data.value("freeze_no").toString().toStdString();
                        result.total_balance = data.value("total_balance").toInt();
                        result.frozen_points = data.value("frozen_points").toInt();
                        result.available_points = data.value("available_points").toInt();
                        ok = true;
                    }
                });

                if (ok && !result.freeze_no.empty()) return result;
                if (attempt < max_retries)
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            return PointFreezeInfo{};
        }

        // ========================= /point/estimate ===========================

        PointFreezeInfo PointManager::postPointsEstimate(const std::string& business_type,
                                                         const std::string& task_param_json,
                                                         int timeout_ms, int max_retries)
        {
            QString url = QString(GEN_SERVER_URL) + QString(GEN_API_PREFIX) + "/point/estimate";
            QString qTaskParam = QString::fromStdString(task_param_json);

            QMap<QString, QString> params;
            params["business_type"] = QString::fromUtf8(business_type.c_str());
            params["task_param"] = qTaskParam;

            QMap<QString, QString> headers;
            headers["Authorization"] = buildAuthHeader(url, qTaskParam);

            for (int attempt = 0; attempt <= max_retries; attempt++)
            {
                PointFreezeInfo result;
                bool ok = false;

                HttpClient client(nullptr);
                client.post(url, params, headers, [&](int, int errorCode, QString, QJsonObject doc)
                {
                    if (errorCode == 0 && doc.value("errorCode").toInt() == 0)
                    {
                        QJsonObject data = doc["data"].toObject();
                        result.estimate_points = data.value("estimate_points").toInt();
                        result.total_balance = data.value("total_balance").toInt();
                        result.frozen_points = data.value("frozen_points").toInt();
                        result.available_points = data.value("available_points").toInt();
                        ok = true;
                    }
                });

                if (ok) return result;
                if (attempt < max_retries)
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            return PointFreezeInfo{};
        }

        // ========================== /point/settle ============================

        PointSettleInfo PointManager::postPointsSettle(const std::string& freeze_no,
                                                       const std::string& settle_status,
                                                       const std::string& task_param_json,
                                                       int timeout_ms, int max_retries)
        {
            QString url = QString(GEN_SERVER_URL) + QString(GEN_API_PREFIX) + "/point/settle";
            QString qTaskParam = QString::fromStdString(task_param_json);

            QMap<QString, QString> params;
            params["freeze_no"] = QString::fromStdString(freeze_no);
            params["status"] = QString::fromStdString(settle_status);
            params["task_param"] = qTaskParam;

            QMap<QString, QString> headers;
            headers["Authorization"] = buildAuthHeader(url, qTaskParam);

            for (int attempt = 0; attempt <= max_retries; attempt++)
            {
                PointSettleInfo result;
                bool ok = false;

                HttpClient client(nullptr);
                client.post(url, params, headers, [&](int, int errorCode, QString, QJsonObject doc)
                {
                    if (errorCode == 0 && doc.value("errorCode").toInt() == 0)
                    {
                        QJsonObject data = doc["data"].toObject();
                        result.consumed = data.value("consumed").toInt();
                        result.refunded = data.value("refunded").toInt();
                        result.total_balance = data.value("total_balance").toInt();
                        result.frozen_points = data.value("frozen_points").toInt();
                        result.available_points = data.value("available_points").toInt();
                        ok = true;
                    }
                });

                if (ok && (result.consumed != 0 || result.refunded != 0)) return result;
                if (attempt < max_retries)
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            return PointSettleInfo{};
        }

        // ============================== 公开接口 ===============================

        int PointManager::EstimateTaskPoints(const std::string& business_type,
                                             const std::string& task_param_json)
        {
            PointFreezeInfo result = postPointsEstimate(business_type, task_param_json);
            return result.estimate_points > 0 ? result.estimate_points : -1;
        }

        PointFreezeInfo PointManager::CreatePointTask(const std::string& business_type,
                                                      const std::string& task_param_json)
        {
            return postPointsFreeze(business_type, task_param_json);
        }

        PointFreezeInfo PointManager::QueryUserPoints()
        {
            return postPointsOutline();
        }

        PointSettleInfo PointManager::SettlePoints(const std::string& freeze_no,
                                                   const std::string& settle_status,
                                                   const std::string& task_param_json)
        {
            return postPointsSettle(freeze_no, settle_status, task_param_json);
        }
    } // namespace CORE
} // namespace AI3D
