// ==================== Src/Core/PointManager.cpp ====================
#include "Core/PointManager.h"
#include "Util/HttpClient.h"
#include "Util/constant.h"         // SETTINGS_PREFIX / CURRENT_PREFIX / TOKEN_PREFIX
#include "Core/Types.h"            // GEN_SERVER_URL / GEN_API_PREFIX
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
            path.remove(GEN_SERVER_URL);

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
                    result.errorCode = errorCode;
                    if (errorCode == 0 && doc.value("errorCode").toInt() == 0)
                    {
                        QJsonObject data = doc["data"].toObject();
                        result.total_balance = data.value("total_balance").toInt();
                        result.frozen_points = data.value("frozen_points").toInt();
                        result.available_points = data.value("available_points").toInt();
                        ok = true;
                        result.requestSucceeded = true;
                        result.errorCode = 0;
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
                    result.errorCode = errorCode;
                    if (errorCode == 0 && doc.value("errorCode").toInt() == 0)
                    {
                        QJsonObject data = doc["data"].toObject();
                        result.freeze_no = data.value("freeze_no").toString().toStdString();
                        result.total_balance = data.value("total_balance").toInt();
                        result.frozen_points = data.value("frozen_points").toInt();
                        result.available_points = data.value("available_points").toInt();
                        ok = true;
                        result.requestSucceeded = true;
                        result.errorCode = 0;
                    }
                });

                if (ok && !result.freeze_no.empty()) return result;
                if (attempt < max_retries)
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            return PointFreezeInfo{};
        }

        // ==================== /generation/getQuote (生成式) =====================

        PointFreezeInfo PointManager::postGenerationGetQuote(const std::string& task_param_json,
                                                             int timeout_ms, int max_retries)
        {
            QString url = QString(GEN_SERVER_URL) + QString(GEN_API_PREFIX) + "/generation/getQuote";
            QString qTaskParam = QString::fromStdString(task_param_json);

            QMap<QString, QString> params;
            params["param"] = qTaskParam;

            QMap<QString, QString> headers;
            headers["Authorization"] = buildAuthHeader(url, qTaskParam);

            for (int attempt = 0; attempt <= max_retries; attempt++)
            {
                PointFreezeInfo result;
                bool ok = false;
                HttpClient client(nullptr);
                client.post(url, params, headers, [&](int, int errorCode, QString, QJsonObject doc)
                {
                    result.errorCode = errorCode;
                    if (errorCode == 0 && doc.value("errorCode").toInt() == 0)
                    {
                        QJsonObject data = doc["data"].toObject();
                        result.estimate_points = data.value("estimated_points").toInt();
                        ok = true;
                        result.requestSucceeded = true;
                        result.errorCode = 0;
                    }
                });
                if (ok) return result;
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
                    result.errorCode = errorCode;
                    if (errorCode == 0 && doc.value("errorCode").toInt() == 0)
                    {
                        QJsonObject data = doc["data"].toObject();
                        result.estimate_points = data.value("estimate_points").toInt();
                        result.total_balance = data.value("total_balance").toInt();
                        result.frozen_points = data.value("frozen_points").toInt();
                        result.available_points = data.value("available_points").toInt();
                        ok = true;
                        result.requestSucceeded = true;
                        result.errorCode = 0;
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
                    result.errorCode = errorCode;
                    if (errorCode == 0 && doc.value("errorCode").toInt() == 0)
                    {
                        QJsonObject data = doc["data"].toObject();
                        result.consumed = data.value("consumed").toInt();
                        result.refunded = data.value("refunded").toInt();
                        result.total_balance = data.value("total_balance").toInt();
                        result.frozen_points = data.value("frozen_points").toInt();
                        result.available_points = data.value("available_points").toInt();
                        ok = true;
                        result.requestSucceeded = true;
                        result.errorCode = 0;
                    }
                });

                if (ok && (result.consumed != 0 || result.refunded != 0)) return result;
                if (attempt < max_retries)
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            return PointSettleInfo{};
        }


#define MOCK_POINT_MANAGER  // TODO CYJ: 取消注释启用 mock, 正式上线前移除

#ifdef MOCK_POINT_MANAGER

        // ========================= Mock 实现 (无 HTTP 依赖) =========================

        PointFreezeInfo PointManager::EstimateTaskPoints(const std::string& business_type,
                                                         const std::string& task_param_json)
        {
            PointFreezeInfo info;
            (void)business_type;
            (void)task_param_json;
            // mock: mesh=50, dsm_dom=30, 其余=100
            if (business_type == BusinessType::MESH)
            {
                info.requestSucceeded = true;
                info.estimate_points = 50;
                info.errorCode = 0;
                return info;
            };
            if (business_type == BusinessType::DSM_DOM)
            {
                info.requestSucceeded = true;
                info.estimate_points = 30;
                info.errorCode = 0;
                return info;
            }
            info.requestSucceeded = true;
            info.estimate_points = 100;
            info.errorCode = 0;
            return info;
        }

        PointFreezeInfo PointManager::CreatePointTask(const std::string& business_type,
                                                      const std::string& task_param_json)
        {
            (void)business_type;
            (void)task_param_json;
            PointFreezeInfo info;
            info.freeze_no = "mock-freeze-" + std::to_string(std::rand());
            info.total_balance = 500;
            info.frozen_points = 100;
            info.available_points = 400;
            return info;
        }

        PointFreezeInfo PointManager::QueryUserPoints()
        {
            PointFreezeInfo info;
            info.total_balance = 500;
            info.frozen_points = 100;
            info.available_points = 400;
            info.requestSucceeded = true;
            info.errorCode = 0;
            return info;
        }

        PointSettleInfo PointManager::SettlePoints(const std::string& freeze_no,
                                                   const std::string& settle_status,
                                                   const std::string& task_param_json)
        {
            (void)task_param_json;
            PointSettleInfo info;
            if (settle_status == SettleStatus::SUCCESS)
            {
                info.consumed = 50; // 全部成功: 扣 50
                info.refunded = 0;
            }
            else if (settle_status == SettleStatus::CANCEL)
            {
                info.consumed = 0;
                info.refunded = 50; // 取消: 返 50
            }
            else if (settle_status == SettleStatus::FAIL)
            {
                info.consumed = 10; // 失败: 扣 10, 返 40
                info.refunded = 40;
            }
            else
            {
                // partial
                info.consumed = 30;
                info.refunded = 20;
            }
            info.total_balance = 450;
            info.frozen_points = 0;
            info.available_points = 450;
            LOGI("Mock SettlePoints: freeze_no=" + freeze_no
                + " status=" + settle_status
                + " consumed=" + std::to_string(info.consumed)
                + " refunded=" + std::to_string(info.refunded));
            return info;
        }

#else // MOCK_POINT_MANAGER

        // ============================== 公开接口 ===============================
        //   启用 MOCK_POINT_MANAGER 时, 以下 4 个方法替换为 4.5 节中的 mock 实现
        //   真实版本放在 #else 分支中 (详见 4.5 节完整代码)

        PointFreezeInfo PointManager::EstimateTaskPoints(const std::string& business_type,
                                                         const std::string& task_param_json)
        {
            // 生成式走 /generation/getQuote, 重建式走 /point/estimate
            PointFreezeInfo result;
            bool isGen = (business_type == BusinessType::TEXT_TO_MODEL ||
                business_type == BusinessType::TEXT_TO_MESH ||
                business_type == BusinessType::IMAGE_TO_MODEL ||
                business_type == BusinessType::IMAGE_TO_MESH);
            if (isGen)
                result = postGenerationGetQuote(task_param_json);
            else
                result = postPointsEstimate(business_type, task_param_json);
            if (result.requestSucceeded)
            {
                if (result.estimate_points > 0)
                    return result;
                else
                {
                    result.requestSucceeded = fasle;
                    result.errorCode = 20001;
                    return result;
                }
            }
            return result;
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

#endif // MOCK_POINT_MANAGER
    } // namespace CORE
} // namespace AI3D
