#include "Util/HttpClient.h"
#include <QNetworkRequest>


#pragma execution_character_set("utf-8")

HttpClient::HttpClient(QObject* parent)
    : QObject(parent)
{
};

HttpClient::~HttpClient()
{
}

QString HttpClient::calSign(QString url, QString accessToken, QMap<QString, QString> param)
{
    QString path = url;
    path.remove(SERVER_HOST);
    QString finalStr = "moldai:" + path;

    QDateTime currentDateTime = QDateTime::currentDateTime();

    QString timestampStr = QString::number(currentDateTime.toSecsSinceEpoch());
    finalStr += ":" + timestampStr;
    if (param.size() == 0)
    {
    }
    else
    {
        QList<QString> sortedKeys = param.keys();
        QJsonObject jsonObject;
        for (const QString& key : sortedKeys)
        {
            jsonObject[key] = param[key];
        }
        QJsonDocument jsonDoc(jsonObject);
        QString dataStr = QString(jsonDoc.toJson(QJsonDocument::Compact));
        finalStr += ":" + dataStr;
    }
    if (accessToken == "")
    {
    }
    else
    {
        finalStr += ":" + accessToken;
    }

    QByteArray byteArray = finalStr.toUtf8();
    QByteArray md5 = QCryptographicHash::hash(byteArray, QCryptographicHash::Md5);
    QByteArray hashInBase64 = md5.toBase64();

    QString authorizationStr = "timestamp:" + timestampStr;
    authorizationStr += ",sign:" + hashInBase64;
    if (accessToken == "")
    {
    }
    else
    {
        authorizationStr += ",accessToken:" + accessToken;
    }

    return authorizationStr;
}

void HttpClient::get(QString url, QMap<QString, QString> headers,
                     std::function<void(int, int, QString, QJsonObject)> onResponseResult)
{
    QNetworkRequest request;
    QNetworkAccessManager nam;

    nam.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setUrl(url);
    connect(&nam, &QNetworkAccessManager::finished, this, [=](QNetworkReply* reply)
    {
        QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (statusCode.isValid())
        {
            qDebug() << "Http request code = " << statusCode.toInt();
        }

        QVariant reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        if (reason.isValid())
        {
            qDebug() << "Http reply =" << reason.toString();
        }


        QNetworkReply::NetworkError err = reply->error();
        QByteArray res = reply->readAll();
        QJsonObject doc = QJsonDocument::fromJson(res).object();
        if (err == QNetworkReply::NoError)
        {
            int code = doc.value("errorCode").toInt();
            QString message = doc.value("errorMsg").toString();


            onResponseResult(statusCode.toInt(), code, message, doc);
        }
        else
        {
            qDebug() << "Request error";
            onResponseResult(statusCode.toInt(), -1, reason.toString(), doc);
        }


        reply->deleteLater();
    });

    request.setUrl(QUrl(url));


    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json; charset=UTF-8"));
    for (auto& pair : headers.toStdMap())
    {
        request.setRawHeader(pair.first.toStdString().c_str(), pair.second.toStdString().c_str());
    }

    QNetworkReply* reply = nam.get(request);


    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
}

void HttpClient::post(QString url, QMap<QString, QString> param, QMap<QString, QString> headers,
                      std::function<void(int, int, QString, QJsonObject)> onResponseResult)
{
    QNetworkRequest request;
    QNetworkAccessManager nam;

    QJsonObject dataObj;
    for (auto& pair : param.toStdMap())
    {
        dataObj.insert(pair.first, pair.second);
    }


    QJsonDocument document;
    document.setObject(dataObj);

    QByteArray byteArray = document.toJson(QJsonDocument::Compact);

    connect(&nam, &QNetworkAccessManager::finished, this, [=](QNetworkReply* reply)
    {
        QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (statusCode.isValid())
        {
            qDebug() << "Http request code = " << statusCode.toInt();
        }

        QVariant reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        if (reason.isValid())
        {
            qDebug() << "Http reply =" << reason.toString();
        }


        QNetworkReply::NetworkError err = reply->error();
        QByteArray res = reply->readAll();
        QJsonObject doc = QJsonDocument::fromJson(res).object();
        if (err == QNetworkReply::NoError)
        {
            int code = doc.value("errorCode").toInt();
            QString message = doc.value("errorMsg").toString();


            onResponseResult(statusCode.toInt(), code, message, doc);

            qDebug() << "result: " << res;
        }
        else
        {
            qDebug() << "Request Error";
            onResponseResult(statusCode.toInt(), -1, reason.toString(), doc);
        }


        reply->deleteLater();
    });

    request.setUrl(QUrl(url));


    QString accessToken = "";

    QSettings* pSettings = new QSettings(SETTINGS_PREFIX + "\\User", QSettings::NativeFormat);

    QString currentUser = pSettings->value(CURRENT_PREFIX, "").toString();
    if (currentUser.isEmpty())
    {
        accessToken = "";
    }
    else
    {
        QString tokenKey = TOKEN_PREFIX + currentUser;
        QString currentToken = pSettings->value(tokenKey, "").toString();
        if (currentToken.isEmpty())
        {
            accessToken = "";
        }
        else
        {
            accessToken = currentToken;
        }
    }

    QString auth = calSign(url, accessToken, param);

    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json; charset=UTF-8"));
    request.setRawHeader("Authorization", auth.toUtf8());
    for (auto& pair : headers.toStdMap())
    {
        request.setRawHeader(pair.first.toStdString().c_str(), pair.second.toStdString().c_str());
    }

    QNetworkReply* reply = nam.post(request, byteArray);


    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
}
