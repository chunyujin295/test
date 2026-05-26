#pragma once
#include <QString>
#include "HttpClient.h"
#include "DeviceInfo.h"
#include <QSettings>
#include <QFile>
#include "CryptoUtil.h"
#include <QJsonDocument>
#include <QJsonObject>
#include "constant.h"

extern struct UserLoginInfo {
	QString account;
	QString password;
	
	
};

extern struct HttpReply {
	int code;                   
	QString message;            
	QJsonObject data;               
};


extern struct SoftwareID {
	QString account;    
	QString deviceId;   
	
};

extern struct UserInfo {
	QString account;    
	QString icon;       
	int level;          
	QString time;       
};




extern bool isLogin();

extern HttpReply userLogin(UserLoginInfo userInfo);

extern bool userLogout();

extern HttpReply getLicense();

extern HttpReply getDefaultLicense();

extern QString getToken();

extern HttpReply getAccount();

extern QString getLocalLicense();


extern bool checkLicenseAvaliable(QString liscense);

int getUserLevel();


extern QString encryptPassword(const QString& password);

