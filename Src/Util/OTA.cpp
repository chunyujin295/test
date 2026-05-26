#include <iostream>
#include <evhttp.h>
#include <curl/curl.h>
#include <atomic>
#include <list>
#include <map>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <QProcess>
#include <QDebug>
#include <QDateTime>
#include <sstream>
#include <QDir>
#include <QFileInfoList>
#include <QStringList>
#include <QTime>
#include <QtGlobal>

#include <QCryptographicHash>
#include <QSet>
#include <QHash>
#include <QMutex>
#include "Core/File.h"
#include "Util/OTA.h"
#include <curl/curl.h>
#include "Util/TaskProcess.h"
#include <QElapsedTimer>

bool bMainWindowDestroyed = false;


#define LOCAL_STR(x)      QString::fromUtf8(x)
#define LOCAL_STR8(x)  QString::fromUtf8(x).toUtf8().data()

int currentVersionCode;
QString currentVersionName;


bool bForceCheckLocal = false;
#define EXTRA_FILES_CONFIG "extra_files.json"
#define NO_CHINESE_PATH_CONFIG "nochinesepath.json"



#if 0
bool UsingNoChinesePathVersion()
{
	
	bUsingNoChinesePath = true;
	return true;

	bUsingNoChinesePath = false;

	if (!QDir(sOTASecondPath).exists() || !QDir(sOTASecondBinPath).exists())
	{
		if (bForceCheckLocal)
		{
			if (QFileInfo::exists(NO_CHINESE_PATH_CONFIG))
			{
				bUsingNoChinesePath = true;
				return true;
			}
		}

		return false;
	}

	if (!QFileInfo::exists(sRemoteNoChinesePathConfig))
	{
		if (bForceCheckLocal)
		{
			if (QFileInfo::exists(NO_CHINESE_PATH_CONFIG))
			{
				bUsingNoChinesePath = true;
				return true;
			}
		}

		return false;
	}

	bUsingNoChinesePath = true;

	return true;
}
#endif

QByteArray getFileMd5(QString filePath)
{
	QFile localFile(filePath);

	if (!localFile.open(QFile::ReadOnly))
	{	
		return 0;
	}

	QCryptographicHash ch(QCryptographicHash::Md5);

	quint64 totalBytes = 0;
	quint64 bytesWritten = 0;
	quint64 bytesToWrite = 0;
	quint64 loadSize = 1024 * 4;
	QByteArray buf;

	totalBytes = localFile.size();
	bytesToWrite = totalBytes;

	while (1)
	{
		if (bytesToWrite > 0)
		{
			buf = localFile.read(qMin(bytesToWrite, loadSize));
			ch.addData(buf);
			bytesWritten += buf.length();
			bytesToWrite -= buf.length();
			buf.resize(0);
		}
		else
		{
			break;
		}

		if (bytesWritten == totalBytes)
		{
			break;
		}
	}

	localFile.close();
	QByteArray md5 = ch.result();

	return md5;
}



int iUploadTotal = 0;
int iUploadNow = 0;
QElapsedTimer  calcRunTime;


struct CustomProgress
{
	curl_off_t lastruntime;
	CURL* curl;
};

bool bPreviousProgressCompleted = false;

int progressCallback(void* p,
	curl_off_t dltotal,
	curl_off_t dlnow,
	curl_off_t ultotal,
	curl_off_t ulnow)
{
	struct CustomProgress* progress = (struct CustomProgress*)p;
	CURL* curl = progress->curl;
	curl_off_t curtime = 0;

	if (bMainWindowDestroyed)
		return 1;

	if (ulnow == 0 && dlnow == 0 || dltotal == 0 && ultotal == 0)
		return 0;

	

	
	int timeConsumed = calcRunTime.elapsed();
	if (ultotal)
	{
		

		
		{
			printf("UP: %ld / %ld bytes  Time: %d secs %d%%\r", ulnow, ultotal, timeConsumed / 1000, int(100.0 * ulnow / ultotal));
			if (ulnow == ultotal)
			{
				printf("\n");
				bPreviousProgressCompleted = true;
			}
			else if (ulnow > 0)
			{
				bPreviousProgressCompleted = false;
			}

			
		}
	}
	else if (dltotal)
	{
		iUploadTotal = dltotal;

		if (iUploadNow != iUploadTotal)
		{
			printf("DOWN: %ld / %ld bytes Time:  %d secs %d%%\r", dlnow, dltotal, timeConsumed / 1000, int(100.0 * dlnow / dltotal));
			if (dlnow == dltotal)
			{
				printf("\n");
				bPreviousProgressCompleted = true;
			}
			else if (dlnow > 0)
			{
				bPreviousProgressCompleted = false;
			}

			iUploadNow = dlnow;
		}
	}

	if (bMainWindowDestroyed)
		return 1;

	
	return 0;
}

size_t getContentLengthFunc(void* ptr, size_t size, size_t nmemb, void* stream)
{
	int r;
	long len = 0;

	r = sscanf((const char*)ptr, "Content-Length: %ld\n", &len);
	if (r)
		*((long*)stream) = len;
	return size * nmemb;
}

size_t discardFunc(void* ptr, size_t size, size_t nmemb, void* stream)
{
	return size * nmemb;
}


bool writeVersionFile(QString& updateJson, int versionCode, QString& versionName)
{
	QJsonObject updateObj;

	updateObj.insert("vi", versionCode);
	updateObj.insert("vs", versionName);

	QJsonDocument updateDoc;
	updateDoc.setObject(updateObj);

	QByteArray updateData = updateDoc.toJson();
	QFile fileUpdate(updateJson);

	fileUpdate.open(QIODevice::WriteOnly);
	fileUpdate.write(updateData);
	fileUpdate.flush();
	fileUpdate.close();

	QFileInfo updateJsonFileInfo(updateJson);

	if (updateJsonFileInfo.exists() && updateJsonFileInfo.size() > 0)
	{
		LOGI(" gen json succ: " + qstr2str(updateJson) + " size:" + std::to_string(updateJsonFileInfo.size()));
		return true;
	}
	else
	{
		LOGI(" gen json failed: " + qstr2str(updateJson) + " size:" + std::to_string(updateJsonFileInfo.size()));
		return false;
	}
}

bool checkVersionFromFile(QString& fileName, int& _versionCode, QString& _versionName)
{
	if (!QFile(fileName).exists())
		return false;

	QFile fileVersion(fileName);
	if (!fileVersion.open(QIODevice::ReadWrite))
	{
		LOGE(qstr2str(fileName) + "open error.");
		return false;
	}

	QByteArray baVersion = fileVersion.readAll();
	QJsonParseError errVersion;
	QJsonDocument jsonDoc = QJsonDocument::fromJson(baVersion, &errVersion);

	fileVersion.close();

	if (errVersion.error != QJsonParseError::NoError || jsonDoc.isNull())
	{
		LOGE(qstr2str(fileName) + "read error.");
		return false;
	}

	QJsonObject jsonObj = jsonDoc.object();

	int versionCode = jsonObj.value("vi").toInt();
	QString versionName = jsonObj.value("vs").toString();

	

	if (versionName.isEmpty() || versionName.length() < 8 || versionCode <= 0 || versionCode >= 9999999)
	{
		LOGE(qstr2str(fileName) + "decode error.");
		return false;
	}

	

	_versionCode = versionCode;
	_versionName = versionName;

	return true;
}


bool isCurrentAppPackageHasProcessRunning(QStringList appList)
{
	if (appList.size() <= 0)
		return false;

	for (int i = 0; i < appList.size(); i++)
	{
		QProcess* process = new QProcess();

		QString app = appList.at(i);

		QString compareCondition = "imagename eq " + app;
		QStringList arguments;
		arguments << "-fi" << compareCondition;

		process->execute("tasklist", arguments);

		QString processOut = QString::fromUtf8(process->readAllStandardOutput());
		if (processOut.contains(app, Qt::CaseInsensitive))
		{
			LOGI(qstr2str(app) + " is Run.");
			return true;
		}
		else
		{
			LOGI(qstr2str(app) + " is Stop.");
		}

		process->close();
	}

	return false;
}


bool bNeed2CheckFileRealGotStateWhileDownloadFailed = true;


#if 0
std::string GBKToUTF8(const std::string& strGBK)
{
	WCHAR* wszSrcString;
	int n = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
	wszSrcString = new WCHAR[n];
	MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, wszSrcString, n);
	n = WideCharToMultiByte(CP_UTF8, 0, wszSrcString, -1, NULL, 0, NULL, NULL);
	char* szDestString = new char[n];
	WideCharToMultiByte(CP_UTF8, 0, wszSrcString, -1, szDestString, n, NULL, NULL);
	std::string strDestUTF8 = szDestString;
	delete[]wszSrcString;
	wszSrcString = NULL;
	delete[]szDestString;
	szDestString = NULL;
	return strDestUTF8;
}
std::string UTF8ToGBK(const std::string & strUTF8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* pszStr = new char[len + 1];
	memset(pszStr, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, pszStr, len, NULL, NULL);
	if (wstr) delete[] wstr;
	std::string str = pszStr;
	delete[] pszStr;
	return str;
}
#endif