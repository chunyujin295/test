#include "User.h"


bool isLogin() {
    bool result = false;
	
    QSettings* pSettings = new QSettings(SETTINGS_PREFIX + "\\User", QSettings::NativeFormat);
    
    QString currentUser = pSettings->value(CURRENT_PREFIX, "").toString();
    if (currentUser.isEmpty()) {
        
        result = false;
        return result;
    }
    
    QString tokenKey = TOKEN_PREFIX + currentUser;
    QString currentToken = pSettings->value(tokenKey, "").toString();
    if (currentToken.isEmpty()) {
        
        result = false;
        return result;
    }
    
    QString expireKey = EXPIRE_PREFIX + currentUser;
    QString expiryDateString = pSettings->value(expireKey, "").toString();
    if (expiryDateString.isEmpty()) {
        
        result = false;
        pSettings->remove(tokenKey);
        return result;
    }else{
        QDateTime expiryDate = QDateTime::fromString(expiryDateString, Qt::ISODate);
        if (expiryDate < QDateTime::currentDateTime()) {
            
            pSettings->remove(tokenKey);
            pSettings->remove(expireKey);
            pSettings->remove(CURRENT_PREFIX);
            result = false;
            return result;
        }
        else {
            
            result = true;
            return result;
        }
    }
    return result;
}

QString getToken() {
    QSettings *pSettings = new QSettings(SETTINGS_PREFIX + "\\User", QSettings::NativeFormat);
    QString currentUserKey = CURRENT_PREFIX;
    QString currentUser = pSettings->value(currentUserKey, "").toString();
    if (currentUser.isEmpty()) {
        
        return "";
    }
    
    QString tokenKey = TOKEN_PREFIX + currentUser;
    QString currentToken = pSettings->value(tokenKey, "").toString();
    return currentToken;
}


HttpReply userLogin(UserLoginInfo userInfo) {
	QString username = userInfo.account;
	QString passwd = userInfo.password;
    QString encryPSW = encryptPassword(passwd);
    DeviceID deviceId = getDeviceID();

	
    HttpClient clent = new HttpClient(nullptr);

    QString url = SERVER_HOST + "/account/login";
    QMap<QString, QString> data;
    data["username"] = username;
    data["password"] = encryPSW;
    data["mid"] = deviceId.mid;
    QMap<QString, QString> headMap;
    headMap["Content-Type"] = "application/json";

    HttpReply reply;

    clent.post(url, data, headMap,
        [&](int statusCode, int code, QString msg, QJsonObject result) {          
            
            if (code == 0) {
                
                QJsonObject mData = result.value("data").toObject();
                QString token = mData.value("AccessToken").toString();
                qDebug() << "token:" << token << "\n";
                QSettings *pSettings = new QSettings(SETTINGS_PREFIX + "\\User", QSettings::NativeFormat);
                
                QString userKey = TOKEN_PREFIX + username;
                pSettings->setValue(userKey, token);
                
                QDateTime now = QDateTime::currentDateTime();
                QDateTime expiryDate = now.addDays(TOKEN_EXPIRE_TIME_DAY); 
                pSettings->setValue(EXPIRE_PREFIX + username, expiryDate.toString(Qt::ISODate));
                
                QString currentUserKey = CURRENT_PREFIX;
                pSettings->setValue(currentUserKey, username);
            }
            else {
                qDebug() << "login error:" << "\n";
            }
            reply.code = code;
            reply.message = msg;
            reply.data = result;

        });
    return reply;
}


bool userLogout() {
    bool result = false;
    if (isLogin()) {
        
        
        QSettings *pSettings = new QSettings(SETTINGS_PREFIX + "\\User", QSettings::NativeFormat);
        QString currentUser = pSettings->value(CURRENT_PREFIX, "").toString();
        QString tokenKey = TOKEN_PREFIX + currentUser;
        QString currentToken = pSettings->value(tokenKey, "").toString();

        
        HttpClient clent = new HttpClient(nullptr);

        QString url = SERVER_HOST + "/account/logout";
        QMap<QString, QString> data;
        data["accesstoken"] = currentToken;
        QMap<QString, QString> headMap;
        headMap["Content-Type"] = "application/json";

        HttpReply reply;

        clent.post(url, data, headMap,
            [&](int statusCode, int code, QString msg, QJsonObject objResult) {            
                result = true;
            });

    }
    
    QSettings *pSettings = new QSettings(SETTINGS_PREFIX + "\\User", QSettings::NativeFormat);
    
    QString currentUserKey = CURRENT_PREFIX;
    QString currentUser = pSettings->value(currentUserKey, "").toString();
    if (!currentUser.isEmpty()) {
        
         
        QString tokenKey = TOKEN_PREFIX + currentUser;
        QString currentToken = pSettings->value(tokenKey, "").toString();
        
        QString expireKey = EXPIRE_PREFIX + currentUser;
        QString expiryDateString = pSettings->value(expireKey, "").toString();
        
        pSettings->remove(tokenKey);
        pSettings->remove(expireKey);
        pSettings->remove(currentUserKey);
    }
    return true;
}


HttpReply getLicense() {
    
    bool result = false;

    QString accessToken = getToken();

    HttpClient clent = new HttpClient(nullptr);

    QString url = SERVER_HOST + "/auth/getauth";
    QMap<QString, QString> data;
    data["accesstoken"] = accessToken;
    QMap<QString, QString> headMap;
    headMap["Content-Type"] = "application/json";

    HttpReply reply;

    clent.post(url, data, headMap,
        [&](int statusCode, int code, QString msg, QJsonObject result) {
            
            if (code == 0) {
                
                QString license = result.value("data").toString();
                QSettings * pSettings = new QSettings(SETTINGS_PREFIX + "\\User", QSettings::NativeFormat);
                QString currentUser = pSettings->value(CURRENT_PREFIX, "").toString();
                
                QString licenseKey = LICENSE_PREFIX + currentUser;
                pSettings->setValue(licenseKey, license);
            }
            reply.code = code;
            reply.message = msg;
            reply.data = result;

        });

    return reply;
}


HttpReply getDefaultLicense() {
    
    bool result = false;

    QString accessToken = getToken();

    HttpClient clent = new HttpClient(nullptr);

    QString url = SERVER_HOST + "/auth/defaultauth";
    QMap<QString, QString> data;
    data["accesstoken"] = accessToken;
    QMap<QString, QString> headMap;
    headMap["Content-Type"] = "application/json";

    HttpReply reply;

    clent.post(url, data, headMap,
        [&](int statusCode, int code, QString msg, QJsonObject result) {
            
            if (code == 0) {
                
                QString license = result.value("data").toString();
                QSettings *pSettings = new QSettings(SETTINGS_PREFIX + "\\User", QSettings::NativeFormat);
                QString currentUser = pSettings->value(CURRENT_PREFIX, "").toString();
                
                QString licenseKey = LICENSE_PREFIX + currentUser;
                pSettings->setValue(licenseKey, license);
            }
            reply.code = code;
            reply.message = msg;
            reply.data = result;

        });

    return reply;
}


HttpReply getAccount() {
	
    
    bool result = false;

    QString accessToken = getToken();

    HttpClient clent = new HttpClient(nullptr);

    QString url = SERVER_HOST + "/account/user";
    QMap<QString, QString> data;
    data["accesstoken"] = accessToken;
    QMap<QString, QString> headMap;
    headMap["Content-Type"] = "application/json";

    HttpReply reply;

    clent.post(url, data, headMap,
        [&](int statusCode, int code, QString msg, QJsonObject result) {
            
            if (code == 0) {
                
                QJsonObject mData = result.value("data").toObject();
                QString username = mData.value("username").toString();
                int level = mData.value("level").toInt();

                QSettings *pSettings = new QSettings(SETTINGS_PREFIX + "\\User", QSettings::NativeFormat);
                pSettings->setValue(CURRENT_PREFIX, username);
                
                QString levelKey = LEVEL_PREFIX + username;
                pSettings->setValue(levelKey, level);
            }
            reply.code = code;
            reply.message = msg;
            reply.data = result;

        });

    return reply;
}


QString getLocalLicense() {
    QSettings *pSettings = new QSettings(SETTINGS_PREFIX + "\\User", QSettings::NativeFormat);
    QString currentUser = pSettings->value(CURRENT_PREFIX, "").toString();
    QString licenseKey = LICENSE_PREFIX + currentUser;
    QString license = pSettings->value(licenseKey, "").toString();
    return license;
}


bool checkLicenseAvaliable(QString liscense) {
    if (liscense == "") {
        return false;
    }
    bool result = false;
    qDebug() << liscense << "\n";
    
    QString strPriKey = "";
    QString strPubKey = "";
    QString strPlainData = "";

    CryptoUtil* rsa = new CryptoUtil();

    QFile customFile(CLIENT_PRIVATE_KEY);
    customFile.open(QIODevice::ReadOnly);
    strPriKey = customFile.readAll();

    QFile publicFile(SERVER_PUBLIC_KEY);
    publicFile.open(QIODevice::ReadOnly);
    strPubKey = publicFile.readAll();

    customFile.close();
    publicFile.close();

    strPlainData = rsa->rsaPriDecrypt(liscense, strPriKey);
    qDebug() << ("strPlainData:" + strPlainData);
    QStringList  dataArr = strPlainData.split(LICENSE_DIVIDER);
    if (dataArr.length() < 2) {
        qDebug() << "Error license";
        result = false;
    }
    else {
        QString headPart = dataArr[0];
        qDebug() << "dataArr[1]: " << dataArr[1] << endl;
        QString authPart = dataArr[1];
        QString signPart = dataArr[2];
        QString signSource = dataArr[0] + LICENSE_DIVIDER + dataArr[1];
        qDebug() << "signSource: " << signSource << endl;

        bool verify = rsa->rsaVerifySign(signSource, signPart, strPubKey);
        if (verify) {
            qDebug() << "verify ok" << "\n";
            qDebug() << "now verify param" << "\n";
            QJsonDocument jsonDocument = QJsonDocument::fromJson(authPart.toUtf8());
            if (!jsonDocument.isNull() && jsonDocument.isObject())
            {
                QJsonObject jsonObj = jsonDocument.object();
                QString username = jsonObj["username"].toString();
                QString mid = jsonObj["mid"].toString();
                QString endtime = jsonObj["endtime"].toString();
                QString level = jsonObj["level"].toInt();
                QSettings* pSettings = new QSettings(SETTINGS_PREFIX + "\\User", QSettings::NativeFormat);
                
                QString currentUser = pSettings->value(CURRENT_PREFIX, "").toString();
                DeviceID deviceId = getDeviceID();
                QDateTime expiryDate = QDateTime::fromString(endtime, Qt::ISODate);
                if (expiryDate < QDateTime::currentDateTime()) {
                    
                    qDebug() << "license expire:" << endtime << "\n";
                    result = false;
                }else if (currentUser.isEmpty() || currentUser != username) {
                    
                    qDebug() << "user unmatch" << currentUser << "\n";
                    result = false;
                }else if (mid != deviceId.mid) {
                        
                    qDebug() << "auth machine unmatch:" << mid << "\n";
                    result = false;

                }
                
                
                result = true;
            }
            else {
                qDebug() << "decode license error" << "\n";
                result = false;
            }
        }
        else {
            qDebug() << "verify err" << "\n";
            result = false;
        }


    }
    if (!result) {
        
        QSettings* pSettings = new QSettings(SETTINGS_PREFIX + "\\User", QSettings::NativeFormat);
        QString currentUser = pSettings->value(CURRENT_PREFIX, "").toString();
        
        QString licenseKey = LICENSE_PREFIX + currentUser;
        pSettings->remove(licenseKey);
        
        userLogout();
    }

    
    return result;
}


int getUserLevel() {
    QSettings *pSettings = new QSettings(SETTINGS_PREFIX + "\\User", QSettings::NativeFormat);
    QString currentUser = pSettings->value(CURRENT_PREFIX, "").toString();
    QString levelKey = LEVEL_PREFIX + currentUser;
    int level = pSettings->value(levelKey, "").toInt();
    return level;
}

QString encryptPassword(const QString& password)
{
    QByteArray passwordData = password.toUtf8();
    
    QByteArray hash = QCryptographicHash::hash(passwordData, QCryptographicHash::Sha256);
    
    return QString(hash.toBase64());
}