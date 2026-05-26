#pragma once

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>
#include "constant.h"
#include <QDateTime>
#include <QCryptographicHash>
#include <QSettings>


class HttpClient:public QObject
{
	Q_OBJECT

public:
	HttpClient(QObject* parent);
	~HttpClient();

public:
	QString calSign(QString url, QString accessToken, QMap<QString, QString> param);
	void post(QString url, QMap<QString, QString> param, QMap<QString, QString> headers, std::function<void(int, int, QString, QJsonObject)> onResponseResult);
	void get(QString url, QMap<QString, QString> headers, std::function<void(int, int, QString, QJsonObject)> onResponseResult);
};

