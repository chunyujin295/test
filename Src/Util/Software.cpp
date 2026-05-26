#include "Util/Software.h"

bool getVersionFromFile(QString& fileName, int& _versionCode, QString& _versionName)
{
    if (!QFile(fileName).exists())
        return false;

    QFile fileVersion(fileName);
    if (!fileVersion.open(QIODevice::ReadWrite))
    {
        qDebug() << fileName << " open failed." << endl;
        return false;
    }

    QByteArray baVersion = fileVersion.readAll();
    QJsonParseError errVersion;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(baVersion, &errVersion);

    fileVersion.close();

    if (errVersion.error != QJsonParseError::NoError || jsonDoc.isNull())
    {
        qDebug() << fileName << " read failed." << endl;
        return false;
    }

    QJsonObject jsonObj = jsonDoc.object();

    int versionCode = jsonObj.value("vi").toInt();
    QString versionName = jsonObj.value("vs").toString();

    

    if (versionName.isEmpty() || versionName.length() < 8 || versionCode <= 0 || versionCode >= 9999999)
    {
        qDebug() << fileName << " read error." << endl;
        return false;
    }

    

    _versionCode = versionCode;
    _versionName = versionName;

    return true;
}

VersionInfo checkSoftWareVersion() {
    VersionInfo versionInfo;
    QString sCurrentVersionFile = "version.json";
    int nowVersionCode;
    QString nowVersionName;
    QString version;
    bool state = getVersionFromFile(sCurrentVersionFile, nowVersionCode, nowVersionName);
    if (!state)
    {
        versionInfo.checkReturn = false;
        versionInfo.isValid = false;
        
        
        version = "0.0.0";
    }
    version = nowVersionName;
    qDebug() << "check version :" << version << "\n";
    DeviceID deviceId = getDeviceID();
    QString mac = deviceId.mid;
    QString machine = deviceId.mname;

    
    HttpClient clent = new HttpClient(nullptr);

    QString url = SERVER_HOST + "/base/checkupdate";
    QMap<QString, QString> data;
    data["version"] = version;
    data["mac"] = mac;
    data["machine"] = machine;
    QMap<QString, QString> headMap;
    headMap["Content-Type"] = "application/json";
    

    clent.post(url, data, headMap,
        [&](int statusCode, int code, QString msg, QJsonObject result) {
            
            if (code == 0) {
                
                versionInfo.checkReturn = true;
                QJsonObject mData = result.value("data").toObject();
                bool valid = mData.value("valid").toBool();
                bool canUpdate = mData.value("canUpdate").toBool();
                versionInfo.isValid = valid;
                versionInfo.canUpdate = canUpdate;
                if (canUpdate) {
                    
                    QJsonObject updateVersion = mData.value("updateVersion").toObject();
                    QString newVersion = updateVersion.value("version").toString();
                    QString md5 = updateVersion.value("md5").toString();
                    QString url = updateVersion.value("url").toString();
                    QString info = updateVersion.value("info").toString();
                    versionInfo.updateVersion = newVersion;
                    versionInfo.md5 = md5;
                    versionInfo.url = url;
                    versionInfo.info = info;
                }
                
            }
            else {
                qDebug() << "check version error:" << "\n";
                versionInfo.checkReturn = false;
            }

        });
    return versionInfo;
}