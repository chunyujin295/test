#include "CryptoUtil.h"


CryptoUtil::CryptoUtil()
{

}

CryptoUtil::~CryptoUtil()
{

}


QString CryptoUtil::rsaPubEncrypt(const QString& strPlainData, const QString& strPubKey)
{
    QByteArray pubKeyArry = strPubKey.toUtf8();
    uchar* pPubKey = (uchar*)pubKeyArry.data();
    BIO* pKeyBio = BIO_new_mem_buf(pPubKey, pubKeyArry.length());
    if (pKeyBio == NULL) {
        qDebug() << "key memory err" << endl;
        return "";
    }


    RSA* pRsa = RSA_new();
    if (strPubKey.contains(BEGIN_RSA_PUBLIC_KEY)) {
        pRsa = PEM_read_bio_RSAPublicKey(pKeyBio, &pRsa, NULL, NULL);

    }
    else {
        pRsa = PEM_read_bio_RSA_PUBKEY(pKeyBio, &pRsa, NULL, NULL);

    }


    if (pRsa == NULL) {
        BIO_free_all(pKeyBio);
        qDebug() << "key read err" << endl;
        return "";
    }

    int nLen = RSA_size(pRsa);
    char* pEncryptBuf = new char[nLen];
    qDebug() << "nLen:" << nLen << "\n";

    
    QByteArray plainDataArry = strPlainData.toUtf8();
    int nPlainDataLen = plainDataArry.length();
    qDebug() << "nPlainDataLen:" << nPlainDataLen << "\n";

    int exppadding = nLen;
    if (nPlainDataLen > exppadding - 11)
        exppadding = exppadding - 11;
    int slice = nPlainDataLen / exppadding;
    if (nPlainDataLen % (exppadding))
        slice++;
    qDebug() << "slice:" << slice << "\n";


    QString strEncryptData = "";
    QByteArray arry;
    for (int i = 0; i < slice; i++)
    {
        QByteArray baData = plainDataArry.mid(i * exppadding, exppadding);
        nPlainDataLen = baData.length();
        memset(pEncryptBuf, 0, nLen);
        uchar* pPlainData = (uchar*)baData.data();
        int nSize = RSA_public_encrypt(nPlainDataLen,
            pPlainData,
            (uchar*)pEncryptBuf,
            pRsa,
            RSA_PKCS1_PADDING);
        if (nSize >= 0)
        {
            arry.append(QByteArray(pEncryptBuf, nSize));
        }
    }

    strEncryptData += arry.toBase64();
    
    delete pEncryptBuf;
    BIO_free_all(pKeyBio);
    RSA_free(pRsa);

    return strEncryptData;
}


QString CryptoUtil::rsaPriDecrypt(const QString& strDecryptData, const QString& strPriKey)
{
    QByteArray priKeyArry = strPriKey.toUtf8();
    uchar* pPriKey = (uchar*)priKeyArry.data();
    BIO* pKeyBio = BIO_new_mem_buf(pPriKey, priKeyArry.length());
    if (pKeyBio == NULL) {
        qDebug() << "key memory err" << endl;
        return "";
    }

    RSA* pRsa = RSA_new();
    pRsa = PEM_read_bio_RSAPrivateKey(pKeyBio, &pRsa, NULL, NULL);
    if (pRsa == NULL) {
        BIO_free_all(pKeyBio);
        qDebug() << "key read err" << endl;
        return "";
    }

    int nLen = RSA_size(pRsa);
    char* pPlainBuf = new char[nLen];

    
    QByteArray decryptDataArry = strDecryptData.toUtf8();
    decryptDataArry = QByteArray::fromBase64(decryptDataArry);
    int nDecryptDataLen = decryptDataArry.length();
    qDebug() << "decryptDataArry:" << decryptDataArry << "\n";
    qDebug() << "nDecryptDataLen:" << nDecryptDataLen << "\n";

    int rsasize = nLen;
    int slice = nDecryptDataLen / rsasize;
    if (nDecryptDataLen % (rsasize))
        slice++;
    qDebug() << "slice:" << slice << "\n";

    QString strPlainData = "";
    for (int i = 0; i < slice; i++)
    {
        QByteArray baData = decryptDataArry.mid(i * rsasize, rsasize);
        nDecryptDataLen = baData.length();
        memset(pPlainBuf, 0, nLen);
        uchar* pDecryptData = (uchar*)baData.data();
        int nSize = RSA_private_decrypt(nDecryptDataLen,
            pDecryptData,
            (uchar*)pPlainBuf,
            pRsa,
            RSA_PKCS1_PADDING);
        if (nSize >= 0) {
            strPlainData += QByteArray(pPlainBuf, nSize);
        }
    }
    qDebug() << "strPlainData:" << strPlainData << "\n";

    
    delete pPlainBuf;
    BIO_free_all(pKeyBio);
    RSA_free(pRsa);

    return strPlainData;
}


QString CryptoUtil::rsaPriEncrypt(const QString& strPlainData, const QString& strPriKey)
{
    QByteArray pubKeyArry = strPriKey.toUtf8();
    uchar* pPubKey = (uchar*)pubKeyArry.data();
    BIO* pKeyBio = BIO_new_mem_buf(pPubKey, pubKeyArry.length());
    if (pKeyBio == NULL) {
        qDebug() << "key memory err" << endl;
        return "";
    }


    RSA* pRsa = RSA_new();
    pRsa = PEM_read_bio_RSAPrivateKey(pKeyBio, &pRsa, NULL, NULL);

    if (pRsa == NULL) {
        BIO_free_all(pKeyBio);
        qDebug() << "key read err" << endl;
        return "";
    }

    int nLen = RSA_size(pRsa);
    char* pEncryptBuf = new char[nLen];
    qDebug() << "nLen:" << nLen << "\n";

    
    QByteArray plainDataArry = strPlainData.toUtf8();
    int nPlainDataLen = plainDataArry.length();
    qDebug() << "nPlainDataLen:" << nPlainDataLen << "\n";

    int exppadding = nLen;
    if (nPlainDataLen > exppadding - 11)
        exppadding = exppadding - 11;
    int slice = nPlainDataLen / exppadding;
    if (nPlainDataLen % (exppadding))
        slice++;
    qDebug() << "slice:" << slice << "\n";


    QString strEncryptData = "";
    QByteArray arry;
    for (int i = 0; i < slice; i++)
    {
        QByteArray baData = plainDataArry.mid(i * exppadding, exppadding);
        nPlainDataLen = baData.length();
        memset(pEncryptBuf, 0, nLen);
        uchar* pPlainData = (uchar*)baData.data();
        int nSize = RSA_private_encrypt(nPlainDataLen,
            pPlainData,
            (uchar*)pEncryptBuf,
            pRsa,
            RSA_PKCS1_PADDING);
        if (nSize >= 0)
        {
            arry.append(QByteArray(pEncryptBuf, nSize));
        }
    }

    strEncryptData += arry.toBase64();
    
    delete pEncryptBuf;
    BIO_free_all(pKeyBio);
    RSA_free(pRsa);

    return strEncryptData;
}


QString CryptoUtil::rsaPubDecrypt(const QString& strDecryptData, const QString& strPubKey)
{
    QByteArray priKeyArry = strPubKey.toUtf8();
    uchar* pPriKey = (uchar*)priKeyArry.data();
    BIO* pKeyBio = BIO_new_mem_buf(pPriKey, priKeyArry.length());
    if (pKeyBio == NULL) {
        qDebug() << "key memory err" << endl;
        return "";
    }

    RSA* pRsa = RSA_new();

    if (strPubKey.contains(BEGIN_RSA_PUBLIC_KEY)) {
        pRsa = PEM_read_bio_RSAPublicKey(pKeyBio, &pRsa, NULL, NULL);

    }
    else {
        pRsa = PEM_read_bio_RSA_PUBKEY(pKeyBio, &pRsa, NULL, NULL);

    }

    if (pRsa == NULL) {
        BIO_free_all(pKeyBio);
        qDebug() << "key read err" << endl;
        return "";
    }

    int nLen = RSA_size(pRsa);
    char* pPlainBuf = new char[nLen];

    
    QByteArray decryptDataArry = strDecryptData.toUtf8();
    decryptDataArry = QByteArray::fromBase64(decryptDataArry);
    int nDecryptDataLen = decryptDataArry.length();
    qDebug() << "decryptDataArry:" << decryptDataArry << "\n";
    qDebug() << "nDecryptDataLen:" << nDecryptDataLen << "\n";

    int rsasize = nLen;
    int slice = nDecryptDataLen / rsasize;
    if (nDecryptDataLen % (rsasize))
        slice++;
    qDebug() << "slice:" << slice << "\n";

    QString strPlainData = "";
    for (int i = 0; i < slice; i++)
    {
        QByteArray baData = decryptDataArry.mid(i * rsasize, rsasize);
        nDecryptDataLen = baData.length();
        memset(pPlainBuf, 0, nLen);
        uchar* pDecryptData = (uchar*)baData.data();
        int nSize = RSA_public_decrypt(nDecryptDataLen,
            pDecryptData,
            (uchar*)pPlainBuf,
            pRsa,
            RSA_PKCS1_PADDING);
        if (nSize >= 0) {
            strPlainData += QByteArray(pPlainBuf, nSize);
        }
    }
    qDebug() << "strPlainData:" << strPlainData << "\n";

    
    delete pPlainBuf;
    BIO_free_all(pKeyBio);
    RSA_free(pRsa);

    return strPlainData;
}


QString CryptoUtil::rsaSign(const QString& strPlainData, const QString& strPriKey) {
    QByteArray priKeyArry = strPriKey.toUtf8();
    uchar* pPriKey = (uchar*)priKeyArry.data();
    BIO* pKeyBio = BIO_new_mem_buf(pPriKey, strPriKey.length());
    if (pKeyBio == NULL) {
        qDebug() << "key memory err" << endl;
        return "";
    }
    RSA* prsa = RSA_new();
    prsa = PEM_read_bio_RSAPrivateKey(pKeyBio, &prsa, NULL, NULL);
    if (prsa == NULL) {
        qDebug() << "key read err" << endl;
        BIO_free_all(pKeyBio);
        return "";
    }

    int nlen = 0;
    
    unsigned char pSrcDat[10240] = { 0 };
    unsigned char pDscDat[10240] = { 0 };
    
    int i = 0;
    int srclen = strPlainData.length();
    const char* data = strPlainData.toStdString().c_str();
    for (i = 0;i < srclen;i++)
    {
        pSrcDat[i] = data[i];
    }
    pSrcDat[i] = '\0';
    
    SHA256(pSrcDat, srclen, pDscDat);
    qDebug() << "srclen=:" << srclen << endl;

    unsigned char signature[2048]; 
    unsigned int signatureLength = RSA_size(prsa);
    nlen = RSA_sign(NID_sha256, pDscDat, SHA256_DIGEST_LENGTH, signature, &signatureLength, prsa);
    if (nlen != 1) { 
        unsigned long ulErr = ERR_get_error();
        char szErrMsg[1024] = { 0 };
        qDebug() << "error number:" << ulErr << endl;
        char* pTmp = NULL;
        pTmp = ERR_error_string(ulErr, szErrMsg); 
        qDebug() << szErrMsg << endl;
        RSA_free(prsa);
        BIO_free_all(pKeyBio);
        qDebug() << "sign err" << endl;
        return "";
    }
    QString strSigned = QByteArray(reinterpret_cast<char*>(signature), signatureLength).toBase64();
    BIO_free_all(pKeyBio);
    RSA_free(prsa);

    return strSigned;
}


bool CryptoUtil::rsaVerifySign(const QString& strPlainData, const QString& strSignData, const QString& strPubKey) {
    QByteArray pubKeyArry = strPubKey.toUtf8();
    uchar* pPriKey = (uchar*)pubKeyArry.data();
    BIO* pKeyBio = BIO_new_mem_buf(pPriKey, strPubKey.length());
    if (pKeyBio == NULL) {
        qDebug() << "key memory err" << endl;
        return false;
    }
    RSA* prsa = RSA_new();
    prsa = PEM_read_bio_RSA_PUBKEY(pKeyBio, &prsa, NULL, NULL);
    if (prsa == NULL) {
        BIO_free_all(pKeyBio);
        qDebug() << "key read error" << endl;
        return false;
    }

    
    unsigned char pSrcDat[102400] = { 0 };
    unsigned char pDscDat[102400] = { 0 };
    int i = 0;
    int srclen = strPlainData.length();
    const char* data = strPlainData.toStdString().c_str();
    for (i = 0;i < srclen;i++)
    {
        pSrcDat[i] = data[i];
    }
    pSrcDat[i] = '\0';
    SHA256(pSrcDat, srclen, pDscDat);
    
    qDebug() << "strSignData" << strSignData << endl;
    QByteArray encoded = strSignData.toUtf8();
    QByteArray decoded = QByteArray::fromBase64(encoded);
    
    const char* decodeStr = decoded.toStdString().c_str();
    qDebug() << "decoded" << decoded << endl;
    int signlen = decoded.length();
    qDebug() << "signlen:" << signlen << endl;
    int nlen = RSA_verify(NID_sha256, pDscDat, SHA256_DIGEST_LENGTH, (const unsigned char*)decodeStr, signlen, prsa);
    qDebug() << "end verify" << "endl";
    qDebug() << "nlen:" << nlen;
    if (nlen != 1) {
        qDebug() << "verify error" << endl;
        unsigned long ulErr = ERR_get_error();
        char szErrMsg[1024] = { 0 };
        qDebug() << "error number:" << ulErr << endl;
        char* pTmp = NULL;
        pTmp = ERR_error_string(ulErr, szErrMsg); 
        qDebug() << szErrMsg << endl;
        RSA_free(prsa);
        BIO_free_all(pKeyBio);
        return false;
    }
    qDebug() << "verify ok" << endl;
    BIO_free_all(pKeyBio);
    RSA_free(prsa);
    return true;

}


