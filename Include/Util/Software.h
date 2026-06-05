#pragma once
#include <QString>
#include "HttpClient.h"
#include "CryptoUtil.h"
#include <QJsonDocument>
#include <QJsonObject>
#include "constant.h"
#include "User.h"

struct VersionInfo
{
	bool checkReturn;
	bool isValid;
	bool canUpdate;
	QString updateVersion;
	QString md5;
	QString url;
	QString info;
};

extern bool getVersionFromFile(QString& fileName, int& _versionCode, QString& _versionName);
extern VersionInfo checkSoftWareVersion();