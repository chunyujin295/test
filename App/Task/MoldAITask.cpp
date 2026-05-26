
#include "Util/CatchProcess.h"
#include <atomic>
#include <list>
#include <map>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "Core/ReturnCode.h"
#include "Core/ReturnState.h"
#include "Core/TaskDef.h"
#include "Core/Application.h"


#include <QList>
#include <QSettings>

#include <QDir>
#include <QFileInfoList>
#include <QStringList>
#include <QTime>
#include <QtGlobal>

#include <stdio.h>
#include <QHostInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVector>
#include <QMap>
#include <QJsonArray>
#include <QApplication>
#include <QCoreApplication>
#include <QProcess>
#include <QDateTime>
#include <boost/algorithm/string.hpp>
#include <filesystem>
#include "time.h"
#include <stdlib.h>
#include "Core/String.h"
#include <Reconstruction/Reconstruct.h>
#include "Core/TaskDef.h"
#include <sstream>
#include "Core/File.h"
#include "Core/ATData.h"
#include "Core/CoordinateSystem.h"
#include "Core/BlockObject.h"
#include "Util/Settings.h"
#include "Util/TaskProcess.h"
#include "ATPreprocessTask.h"
#ifdef _MSC_VER
#include "Windows.h"
#include "DbgHelp.h"
#endif 

#include "Util/Statistic.h"
#include "Util/Settings.h"
#include <signal.h>

int resultCode = -1;
bool isChinese = false;


void GetLanguageVersion()
{
	AI3D::CORE::Application::Getinstance().ParseConfig();
	isChinese =  AI3D::CORE::BlockObject::isChineseVersion();
}

class XLog
{
public:
	static std::ostream& stream() {
		return *os;
	}

	static std::ostream& showMsg(std::string& msg, const char* file, char* func, int line)
	{
		*os << file << " " << func << " " << line << " :" << msg << std::endl;
		return *os;
	}

	static std::string& std_string_format(std::string& _str, const char* _Format, ...) {
		std::string tmp;

		va_list marker = NULL;
		va_start(marker, _Format);

		size_t num_of_chars = _vscprintf(_Format, marker);

		if (num_of_chars > tmp.capacity()) {
			tmp.resize(num_of_chars + 1);
		}

		vsprintf_s((char*)tmp.data(), tmp.capacity(), _Format, marker);

		va_end(marker);

		_str = tmp.c_str();
		return _str;
	}

	static std::ostream& log(const char* file, char* func, int line, const char* format, ...)
	{
		std::string msg;

		va_list va = NULL;
		va_start(va, format);

		size_t num_of_chars = _vscprintf(format, va);

		if (num_of_chars > msg.capacity()) {
			msg.resize(num_of_chars + 1);
		}

		vsprintf_s((char*)msg.data(), msg.capacity(), format, va);

		va_end(va);

		*os << file << " " << func << " " << line << " :" << msg << std::endl;

		return *os;
	}

public:
	static std::ostream* os;
};

std::ostream* XLog::os = &std::cout;

#define XLOGI(msg) XLog::stream() << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << ":" << msg << std::endl;
#define XLOGV(msg) XLog::showMsg(msg,__FILE__,__FUNCTION__,__LINE__);


int GenerateMiniDump(PEXCEPTION_POINTERS pExceptionPointers)
{
	
	typedef BOOL(WINAPI* MiniDumpWriteDumpT)(
		HANDLE,
		DWORD,
		HANDLE,
		MINIDUMP_TYPE,
		PMINIDUMP_EXCEPTION_INFORMATION,
		PMINIDUMP_USER_STREAM_INFORMATION,
		PMINIDUMP_CALLBACK_INFORMATION
		);
	
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
	
	char szFileName[MAX_PATH] = { 0 };
	const char* szVersion = "Dump4MoldAITask";
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
	
	MINIDUMP_EXCEPTION_INFORMATION expParam;
	expParam.ThreadId = GetCurrentThreadId();
	expParam.ExceptionPointers = pExceptionPointers;
	expParam.ClientPointers = FALSE;
	pfnMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
		hDumpFile, MiniDumpWithDataSegs, (pExceptionPointers ? &expParam : NULL), NULL, NULL);
	
	CloseHandle(hDumpFile);
	FreeLibrary(hDbgHelp);

	exit(1099);

	return EXCEPTION_EXECUTE_HANDLER;
	
}

LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS lpExceptionInfo)
{
	
	qDebug() << "has come to exception filter now.";
	
	if (IsDebuggerPresent())
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}
	return GenerateMiniDump(lpExceptionInfo);
}

void SigInt_Handler(int nSignal)
{
	qDebug() << "catch int signal:" << QString::number(nSignal);
	

	exit(0);
}

void SigBreak_Handler(int nSignal)
{
	qDebug() << "catch break signal:" << QString::number(nSignal);
	

	exit(1099);
	
}

void customized_exit()
{
	
	qDebug() << "exit task.";
}


#define EXTRA_FILES_CONFIG "extra_files.json"
#define NO_CHINESE_PATH_CONFIG "nochinesepath.json"

extern bool UsingNoChinesePathVersion();
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

float lastprogress = 0.0;
std::string lastMsg = "";
std::string resultMsg = "";
int currenttaskid = -1;
std::string currentblockpath = "";
std::string currentblock = "";
std::string jobname = "";
bool GShoudStop = false;
std::string premsg = "";
std::string current_job_file;
int current_task_id;
std::string current_feedback_file;

#ifdef WIN32


BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
	
	qDebug() << "has come to handler routine now:dwCtrlType" << dwCtrlType;
	

	if (CTRL_CLOSE_EVENT == dwCtrlType || CTRL_C_EVENT == dwCtrlType || CTRL_BREAK_EVENT == dwCtrlType \
		|| CTRL_LOGOFF_EVENT == dwCtrlType || CTRL_SHUTDOWN_EVENT == dwCtrlType)
	{

	}
	return true;
}
#endif 

jobtype_e jobtype;

void cbProgress(float fvalue)
{
	JobFeedBack_s feedback;
	JobFullInfo_s curjob;
	int retryTimes = 0;

	try
	{

		{
			
		
			if (!std::filesystem::exists(AI3D::CORE::File::BoostPathFromUtf8(current_job_file)))
			{
				
				
				return;
			}

			if (!curjob.load(current_job_file))
			{
				
				
				return;
			}

			if (premsg != resultMsg)
			{
				
				feedback.Msg = resultMsg;

				premsg = resultMsg;
			}

			bool bLoadFeedbackFileError = false;
			float progreestemp = 0.0;
			
			if (!feedback.load(current_feedback_file))
			{
				bLoadFeedbackFileError = true;
				
			}

			if (!bLoadFeedbackFileError && (feedback.Status == jobsta_e::STATUS_CANCLE || feedback.Status == jobsta_e::STATUS_FAILURE))
			{
				return;
			}

			progreestemp = feedback.Percent;
			
			
		
			feedback.Msg = resultMsg;

			float currentpercent = 0;
			if (jobtype == JOB_AT )
			{
				currentpercent = curjob.tg.GetProgress((double)fvalue, current_task_id);;
				qDebug() << " progress input"<< fvalue << " progress out " << currentpercent << " taskid "<< current_task_id;
			}
			else if (jobtype == JOB_BATCH)
			{
				currentpercent = fvalue;;
			}
			else if (jobtype == JOB_TILE)
			{
				currentpercent = fvalue*100.;;
			}														 	
			if (progreestemp < currentpercent)
			{
				feedback.Percent = currentpercent;
			}
			else
			{
				
				feedback.Percent = progreestemp;
			}

			lastprogress = feedback.Percent;
			
			feedback.TaskRetVal = -2;
			if (feedback.Status == jobsta_e::STATUS_PENDDING)
				feedback.Status = jobsta_e::STATUS_RUNNING;

			
			bool a = feedback.save_with_retry(current_feedback_file, false);
		}
	}
	catch (const std::filesystem::filesystem_error& fse)
	{
		std::ostringstream oss;
		oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
		
	
		qDebug() << str2qstr(oss.str());
		std::cout << oss.str() << std::endl;
	}
	catch (std::exception ex)
	{
		qDebug() << ex.what();
		

		return;
	}
}
void cbMsg(std::string msg)
{
	
	
	
	lastMsg = AI3D::CORE::String::LocaleToUtf8(msg);
}

static ReconstructCallBack cb([](int finish) {std::cout << "[FINISH]" << std::endl; },
	cbProgress, cbMsg, & GShoudStop);


int execCancelCommandOnly(std::string fileName);

int execCancelCommandOnly(std::string fileName) {
	
	

	try
	{
		std::string taskPostFix = "";
		if (TASK_USE_BIN) {
			taskPostFix = BINFILE_POSTFIX;
		}
		else {
			taskPostFix = JSONFILE_POSTFIX;
		}
		int lastDot = fileName.rfind(taskPostFix);
		int fileNamePos = fileName.rfind(TASK_DEF_BIN_PREFIX);

		if (lastDot == std::string::npos || fileNamePos == std::string::npos || fileNamePos >= lastDot)
		{
			return resultCode;
		}

		std::string taskNumStr = fileName.substr(fileNamePos + strlen(TASK_DEF_BIN_PREFIX), lastDot - fileNamePos);

		int taskdefNum = atoi(taskNumStr.c_str());
		if (taskdefNum < 0)
		{
			return resultCode;
		}

		ATTaskInfo atparam;
		

		if (!std::filesystem::exists(AI3D::CORE::File::BoostPathFromUtf8(fileName)))
		{
			return resultCode;
		}
		atparam.load(fileName);

		std::string function = atparam.task_.fun_name_;
		std::ifstream ifs = AI3D::CORE::File::OpenIfstreamUtf8(fileName, std::ios::in);
		if (!ifs.is_open())
		{
			
			

			std::cout << "open " + fileName + " failed" << std::endl;
			return -1;
		}
		std::string strall((std::istreambuf_iterator<char>(ifs)),
			std::istreambuf_iterator<char>());
		ifs.close();
		
		currentblockpath = atparam.projectFile_;
		currentblock = atparam.blockItem_;
		jobname = atparam.job_;

		jobtype = GetJobType(jobname);

		current_task_id = atparam.task_.id_;
		resultMsg = StepATFromfunctionToshow.at(function);
		if (isChinese)
		{

			resultMsg = StepATFromfunctionToshow_chinese.at(function);


		}

		QString timenow = (QDateTime::currentDateTime()).toString("yyyy-MM-dd hh:mm:ss.zzz");

		
		int type = atparam.task_.type_;
		if (type == 0)
		{
			qDebug() << "type is 0";
			return -1;
		}

		{
			if (function == "RunReconstruction")
			{
				resultCode = RunCancel(strall, cb);
			}
			else
			{
				qDebug() << "no function specified." ;
				return -1;
			}

		}

		return resultCode;
	}
	catch (const std::filesystem::filesystem_error& fse)
	{
		std::ostringstream oss;
		oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
		

		qDebug() << str2qstr(oss.str());
		
		return -1;
	}
	catch (std::exception ex)
	{
		qDebug() << "Exception:" << ex.what();
		return 1001;
	}
	return 0;
}


int execTaskFile2Only(std::string fileName);

int execTaskFile2Only(std::string fileName)
{


	try
	{ 
		int retryTimes = 0;
		std::string taskPostFix = "";
		if (TASK_USE_BIN) {
			taskPostFix = BINFILE_POSTFIX;
		}
		else {
			taskPostFix = JSONFILE_POSTFIX;
		}
		int lastDot = fileName.rfind(taskPostFix);
		
		int fileNamePos = fileName.rfind(TASK_DEF_BIN_PREFIX);
		if (lastDot == std::string::npos || fileNamePos == std::string::npos || fileNamePos >= lastDot)
		{
			
			return resultCode;
		}

		std::string taskNumStr = fileName.substr(fileNamePos + strlen(TASK_DEF_BIN_PREFIX), lastDot - fileNamePos);

		int taskdefNum = atoi(taskNumStr.c_str());
		if (taskdefNum < 0)
		{
			qDebug() << "taskdefNum is 0";
			return resultCode;
		}

		ATTaskInfo atparam;
		

		if (!std::filesystem::exists(AI3D::CORE::File::BoostPathFromUtf8(fileName)))
		{
			qDebug() << str2qstr(fileName) << "not exist";

			return resultCode;
		}
		atparam.load(fileName);

		std::string function = atparam.task_.fun_name_;
		std::string strall = "";
		if (TASK_USE_BIN) {
			strall = fileName;
		}
		else {
			std::ifstream ifs = AI3D::CORE::File::OpenIfstreamUtf8(fileName, std::ios::in);
			if (!ifs.is_open())
			{
				qDebug() << "open " << str2qstr(fileName) << " failed";
				return -1;
			}

			strall = std::string((std::istreambuf_iterator<char>(ifs)),
				std::istreambuf_iterator<char>());
			ifs.close();
		}


		currentblockpath = atparam.projectFile_;
		currentblock = atparam.blockItem_;
		jobname = atparam.job_;

		jobtype = GetJobType(jobname);

		current_task_id = atparam.task_.id_;
		resultMsg = StepATFromfunctionToshow.at(function);
		if (isChinese)
		{

			resultMsg = StepATFromfunctionToshow_chinese.at(function);

			
		}

		std::string path = AI3D::CORE::File::GetParentDir(currentblockpath) + "/" + currentblock;
		if (JOB_FEEDBACK_USE_BIN) {
			current_feedback_file = MAKE_FEEDBAK_BIN_FILE(path, jobname);
		}
		else {
			current_feedback_file = MAKE_FEEDBAK_JSON_FILE(path, jobname);
		}
		



		std::string jobdir = qstr2str(Settings::getEngineJobQueue());
		std::string postFix = "";
		if (JOB_INFO_USE_BIN) {
			postFix = BINFILE_POSTFIX;
		}
		else {
			postFix = JSONFILE_POSTFIX;
		}
		current_job_file = jobdir + "/Running/" + jobname + postFix;


		int type = atparam.task_.type_;
		if (type == 0)
		{
			qDebug() << "type is 0 ";
			return -1;
		}
	
		{

		
			JobFeedBack_s feedback;
			bool bLoadFeedbackFileError = false;

			if (!feedback.load(current_feedback_file))
			{
				
				qDebug() << "load " << str2qstr(current_feedback_file) << " failed";
				bLoadFeedbackFileError = true;
				
				
			}

			if (!bLoadFeedbackFileError && (feedback.Status == jobsta_e::STATUS_CANCLE || feedback.Status == jobsta_e::STATUS_FAILURE))
			{
				qDebug() << "feedback state error ";
				return -1;
			}

			if (bLoadFeedbackFileError)
			{
				feedback.Status = jobsta_e::STATUS_RUNNING;
				feedback.Percent = lastprogress;
			}

			feedback.TaskRetVal = 0;
			
			feedback.Msg = function;
			feedback.save_with_retry(current_feedback_file,false);

	#if 0
			qDebug() << "inside MoldAITask,start testing sleep before actual algorithm function.";

			Sleep(5000);

			qDebug() << "inside MoldAITask,in the middle of testing sleep before actual algorithm function.";
		
			
			
			

			Sleep(35000);

			qDebug() << "inside MoldAITask,end testing sleep before actual algorithm function.";
	#endif

			qDebug() << "before algorithm function:" << QString::fromStdString(function) << " " << QString::fromStdString(fileName);

			 if (function == "RunGenTasks" || function == "GenTasks")
			{
			
				resultCode = RunGenTasks(strall, cb);

			}

			else if (function == "RunFeatureDetection")
			{
				
				resultCode = RunFeatureDetection(strall, cb);

			}

			else if (function == "RunMatchPairs")
			{

				
				resultCode = RunMatchPairs(strall, cb);

			}
			else if (function == "RunPairSelection")
			{
				
				resultCode = RunPairSelection(strall, cb);
			}
			
			
			
			
			
			else if (function == "RunSfM")
			{
				
				resultCode = RunSfM(strall, cb);

			}
			else if (function == "RunReconstruction")
			{
				resultCode = RunReconstruction(strall, cb);
			}
			else if (function == "RunBatchPrepare")
			{
			
				resultCode = RunBatchPrePare(strall, cb);

			}
			else
			{
				
				std::cout << "no function specified." << std::endl;
				return -1;
			}

			qDebug() << "after algorithm function:" << QString::fromStdString(function) << " " << QString::fromStdString(fileName);
		}

		
		
		do
		{
			FILE* fpLock = AI3D::CORE::File::FopenDenyWriteLockUtf8(current_feedback_file + ".lock");
			if (fpLock == NULL)
			{
				retryTimes++;
				std::this_thread::sleep_for(std::chrono::milliseconds(3000));
				continue;
			}

			
			JobFeedBack_s feedback;
			bool bLoadFeedbackFileError = false;

			if (!feedback.load(current_feedback_file))
			{
	
				bLoadFeedbackFileError = true;

				
				
			}

			if(!bLoadFeedbackFileError && (feedback.Status == jobsta_e::STATUS_CANCLE || feedback.Status == jobsta_e::STATUS_FAILURE))
			{
				fclose(fpLock);
				return -1;
			}	


			feedback.TaskRetVal = resultCode;
			

			
			feedback.Msg = resultMsg;

			
			if(bLoadFeedbackFileError || lastprogress > feedback.Percent)
				feedback.Percent = lastprogress;
			else
			{
				
			}

			if (resultCode == MOLDAI_SUCCESS)
			{

				if (bLoadFeedbackFileError || feedback.Status == jobsta_e::STATUS_PENDDING)
				{
					feedback.Status = jobsta_e::STATUS_RUNNING;
				}
				
				feedback.save(current_feedback_file);
			}
			else if (resultCode == MOLDAI_USER_CANCEL)
			{

				feedback.Status = jobsta_e::STATUS_CANCLE;

				feedback.Msg = lastMsg;
				feedback.save(current_feedback_file);

			}
			else
			{
				feedback.Msg = lastMsg;
				
				feedback.Status = jobsta_e::STATUS_FAILURE;
				feedback.save(current_feedback_file);


			}

			fclose(fpLock);
			break;
		}while (retryTimes < 3);

		return resultCode;
	}
	catch (const std::filesystem::filesystem_error& fse)
	{
		std::ostringstream oss;
		oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
		

		qDebug() << str2qstr(oss.str());
		
		return -1;
	}
	catch (std::exception ex)
	{
		qDebug() << "Exception:" << ex.what();

		return 1001;
	}
}

	int doTaskInProcess(int argc, char** argv)
	{
		if (argc == 2) {

			QString taskJsonName = QString::fromUtf8(argv[1]);

			QFileInfo finfo(taskJsonName);
			QString fileName = finfo.fileName();

			if (!finfo.exists() || finfo.size() <= 0)
			{
				return -1;
			}

			QString postFix = "";
			if (TASK_USE_BIN) {
				postFix = BINFILE_POSTFIX;
			}
			else {
				postFix = JSONFILE_POSTFIX;
			}
			if (!fileName.startsWith(TASK_DEF_BIN_PREFIX, Qt::CaseInsensitive) ||
				!fileName.endsWith(postFix, Qt::CaseInsensitive))
			{
				return -1;
			}

			return execTaskFile2Only(argv[1]);
		}
		else if (argc == 3) {

			std::string commandType = argv[1];
			if (commandType == "cancel") {
				
				QString taskJsonName = QString::fromUtf8(argv[2]);

				QFileInfo finfo(taskJsonName);
				QString fileName = finfo.fileName();

				if (!finfo.exists() || finfo.size() <= 0)
				{
					return -1;
				}

				QString postFix = "";
				if (TASK_USE_BIN) {
					postFix = BINFILE_POSTFIX;
				}
				else {
					postFix = JSONFILE_POSTFIX;
				}
				if (!fileName.startsWith(TASK_DEF_BIN_PREFIX, Qt::CaseInsensitive) ||
					!fileName.endsWith(postFix, Qt::CaseInsensitive))
				{
					return -1;
				}
				return execCancelCommandOnly(argv[2]);
			}
			else {
				return -1;
			}
		}
		else
		{

			return -1;
		}
		

}


std::string GetAPPPath()
{
	std::string app_path;
#ifdef _MSC_VER
	TCHAR buf[MAX_PATH + 1] = { 0 };
	GetModuleFileName(NULL, buf, MAX_PATH);
	std::string path;
	try
	{
		path = AI3D::CORE::File::BoostPathToUtf8String(std::filesystem::path(buf).parent_path());
	}
	catch (const std::filesystem::filesystem_error& fse)
	{
		std::ostringstream oss;
		oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1() << " " << fse.path2();
		
		qDebug() << QString::fromStdString(oss.str());
	}
	catch (std::exception& ex)
	{
		qDebug()  << ex.what();
		
	}

	return path;
#else 
	LPTSTR home = getenv("HOME");
	if (home == NULL)
		return String();
	String name(String(home) + "/app");
	return ensureUnifySlash(name);
#endif 
	return app_path;
}

std::string GetGDALPath()
{
	std::string gdal_path;
	gdal_path = GetAPPPath() + "\\data";
	return gdal_path;
}

int main(int argc, char** argv)
{
	
#ifdef WIN32
	SetUnhandledExceptionFilter(ExceptionFilter);
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);


#endif 

	::atexit(customized_exit);

	signal(SIGINT, SigInt_Handler);
	signal(SIGBREAK, SigBreak_Handler);
	signal(SIGTERM, SigBreak_Handler);
	signal(SIGABRT, SigBreak_Handler);

	QApplication app(argc, argv);

	qInstallMessageHandler(customMessageHandler);

	

	int enginePid = qApp->applicationPid();
	std::string gdalDir = GetGDALPath();
	std::string strEnv = "PROJ_LIB=" + gdalDir;
	putenv(strEnv.c_str());
	GetLanguageVersion();

	qDebug() << "task process start.";

	int rtn = doTaskInProcess(argc, argv);
	if (rtn == -1)
	{
		qDebug() << "task process failed:return -1";
	}
	else
	{
		qDebug() << "task process return:" << QString::number(rtn);
	}

	return rtn;
}