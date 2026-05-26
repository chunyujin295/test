#pragma once

#include <QString>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <QDebug>
#define BEGIN_RSA_PUBLIC_KEY    "BEGIN RSA PUBLIC KEY"
#define BEGIN_RSA_PRIVATE_KEY   "BEGIN RSA PRIVATE KEY"
#define BEGIN_PUBLIC_KEY        "BEGIN PUBLIC KEY"
#define BEGIN_PRIVATE_KEY       "BEGIN PRIVATE KEY"
#define KEY_LENGTH              1024

class CryptoUtil
{
public:
    CryptoUtil();
    ~CryptoUtil();

    QString rsaPubEncrypt(const QString& strPlainData, const QString& strPubKey);

    QString rsaPriDecrypt(const QString& strDecryptData, const QString& strPriKey);

    QString rsaPriEncrypt(const QString& strPlainData, const QString& strPriKey);

    QString rsaPubDecrypt(const QString& strDecryptData, const QString& strPubKey);

    QString rsaSign(const QString& strPlainData, const QString& strPriKey);

    bool rsaVerifySign(const QString& strPlainData, const QString& strSignData, const QString& strPubKey);
};

