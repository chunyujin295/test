#include "dialog.h"
#include "./ui_dialog.h"
#include <QtWidgets/QLabel>
#include <Util/DeviceInfo.h>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCryptographicHash>

QString Dialog::calSign(QString url, QString accessToken, QMap<QString, QString> param) {
    QString path = "/user/login";
    QString finalStr = "moldai:" + path;
    // 获取当前时间
    QDateTime currentDateTime = QDateTime::currentDateTime();
    // 获取自Unix纪元以来的秒数
    QString timestampStr = QString::number(currentDateTime.toSecsSinceEpoch());
    finalStr += ":" + timestampStr;
    if (param.size()==0) {
        // continue;
    }
    else {
        QList<QString> sortedKeys = param.keys();
        QJsonObject jsonObject;
        for (const QString& key : sortedKeys) {  // 使用sortedKeys进行迭代
            jsonObject[key] = param[key];  // 直接从原始map中取值
        }
        QJsonDocument jsonDoc(jsonObject);
        QString dataStr = QString(jsonDoc.toJson(QJsonDocument::Compact));
        finalStr += ":" + dataStr;
    }
    if (accessToken == "") {
        // continue;
    }
    else {
        finalStr += ":" + accessToken;
    }
    qDebug() << "finalStr:" << finalStr << endl;
    QByteArray byteArray = finalStr.toUtf8();
    QByteArray md5 = QCryptographicHash::hash(byteArray, QCryptographicHash::Md5);
    QByteArray hashInBase64 = md5.toBase64();
    qDebug() << "hashInBase64:" << hashInBase64 << endl;
    QString authorizationStr = "timestamp:" + timestampStr;
    authorizationStr += ",sign:" + hashInBase64;
    if (accessToken == "") {
        // continue;
    }
    else {
        authorizationStr += ",accessToken:" + accessToken;
    }
    qDebug() << "authorizationStr:" << authorizationStr << endl;
    return authorizationStr;
    //return finalStr;
}

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    QLabel* label = new QLabel(this);
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    DeviceDetail detail = checkDeviceAvailable();
    QString systemInfo = "";
    /*if (detail.unMatch){
        systemInfo = detail.systemInfo + detail.errorInfo;
    }else{
        systemInfo = detail.systemInfo;
    }*/
    QString path = "";
    QString accessToken = "";
    QMap<QString, QString> param;
    param["username"] = "12345678910";
    param["password"] = "abcderg";
    param["company"] = "";
    param["email"] = "";
    param["verify_code"] = "1234";
    param["version"] = "1.00.009";
    systemInfo = calSign(path, accessToken, param);
    label->setText(systemInfo);
}

Dialog::~Dialog()
{
    delete ui;
}

