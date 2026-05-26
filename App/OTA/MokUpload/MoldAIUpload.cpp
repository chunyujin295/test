
#include <iostream>
#include<evhttp.h>
#include<curl/curl.h>
#include <atomic>
#include <list>
#include <map>
#include <vector>
#include <mutex>
#include <condition_variable>

#include "time.h"
#include <stdlib.h>

#include <sstream>
#include <QApplication>
#include <QWidget>
#include <QRect>
#include <QDesktopWidget>

#include <QList>
#include <QSettings>
#include <QMessageBox>
#include <QDir>
#include <QFileInfoList>
#include <QStringList>
#include <QTime>
#include <QtGlobal>
#include <QTextCodec>
#include <stdio.h>
#include <share.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QVector>
#include <QMap>
#include <QJsonArray>

#ifdef _MSC_VER
#include "Windows.h"
#include "DbgHelp.h"
#endif // _MSC_VER
#include <QStyleFactory>
#include <QProcess>
#include <QCryptographicHash>
#include "Util/OTA.h"
extern bool bSpecialLog;



//?chy
// write json file / truncate it when open a existing file.
// two way to handle lock file. even for job file.

//QSettings *sets = nullptr;
int GenerateMiniDump(PEXCEPTION_POINTERS pExceptionPointers)
{
	// 定义函数指针
	typedef BOOL(WINAPI* MiniDumpWriteDumpT)(
		HANDLE,
		DWORD,
		HANDLE,
		MINIDUMP_TYPE,
		PMINIDUMP_EXCEPTION_INFORMATION,
		PMINIDUMP_USER_STREAM_INFORMATION,
		PMINIDUMP_CALLBACK_INFORMATION
		);
	// 从 "DbgHelp.dll" 库中获取 "MiniDumpWriteDump" 函数
	MiniDumpWriteDumpT pfnMiniDumpWriteDump = NULL;
	HMODULE hDbgHelp = LoadLibraryA("DbgHelp.dll");
	if (NULL == hDbgHelp)
	{
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	pfnMiniDumpWriteDump = (MiniDumpWriteDumpT)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");

	if (NULL == pfnMiniDumpWriteDump)
	{
		FreeLibrary(hDbgHelp);
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	// 创建 dmp 文件件
	char szFileName[MAX_PATH] = { 0 };
	char* szVersion = "Dump";
	SYSTEMTIME stLocalTime;
	GetLocalTime(&stLocalTime);
	sprintf(szFileName, "%s-%04d%02d%02d-%02d%02d%02d.dmp",
		szVersion, stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay,
		stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond);
	HANDLE hDumpFile = CreateFileA(szFileName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	if (INVALID_HANDLE_VALUE == hDumpFile)
	{
		FreeLibrary(hDbgHelp);
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	// 写入 dmp 文件
	MINIDUMP_EXCEPTION_INFORMATION expParam;
	expParam.ThreadId = GetCurrentThreadId();
	expParam.ExceptionPointers = pExceptionPointers;
	expParam.ClientPointers = FALSE;
	pfnMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
		hDumpFile, MiniDumpWithDataSegs, (pExceptionPointers ? &expParam : NULL), NULL, NULL);
	// 释放文件
	CloseHandle(hDumpFile);
	FreeLibrary(hDbgHelp);
	return EXCEPTION_EXECUTE_HANDLER;
}

LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS lpExceptionInfo)
{
	// 这里做一些异常的过滤或提示
	if (IsDebuggerPresent())
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}
	return GenerateMiniDump(lpExceptionInfo);
}

QString progBaseName;

extern int SyncOTADownloadedPackageInsideIntranet(int& newVer, QString& newVerName, int &oldVer, QString& oldVerName);

int main(int argc,char **argv) {
#ifdef WIN32
	SetUnhandledExceptionFilter(ExceptionFilter);
	//SetConsoleCtrlHandler(HandlerRoutine, TRUE);//注册监听程序退出事件
#endif // WIN32
	
	QFileInfo finfoProgFile(argv[0]);

	progBaseName = finfoProgFile.baseName();

	if (!progBaseName.compare("MokCopy", Qt::CaseInsensitive))
	{
		int result = 0;
		int oldVer, newVer;
		QString oldVerName, newVerName;

		result = SyncOTADownloadedPackageInsideIntranet(newVer,newVerName,oldVer,oldVerName);
		if(result == 0)
		{
			std::cout << "ota package has updated successfully(from " <<  std::to_string(oldVer) << "(" + oldVerName.toStdString()
				<< ") to " << std::to_string(newVer) << "(" << newVerName.toStdString() << ")." << std::endl;
		}
		else
		{
			std::cout << "ota package failed to update(error code:" << std::to_string(result) << ")!" << std::endl;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(5000));

		return 0;
	}

		// 打包最新的程序包并上传.
	DoArchiveAndUpload();
	return 0;

}
