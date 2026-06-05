#include <conio.h>

#include "Util/CatchProcess.h"
#include <atomic>
#include <list>
#include <map>
#include <vector>
#include <mutex>
#include <sstream>
#include <iostream>
#include <filesystem>

#include <condition_variable>
#include "Core/ReturnCode.h"
#include "Core/ReturnState.h"
#include "Core/TaskDef.h"



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
#include <QMessageBox>
#include <QThread>
#include <boost/algorithm/string.hpp>
#include "time.h"
#include <stdlib.h>
#include <QMutex>
#include <signal.h>

#include "Core/File.h"
#include "Core/Logging.h"
#include "Util/Settings.h"
#include "Core/WorkPath.h"
#include "Util/TaskProcess.h"
#include "Util/JobMonitor.h"
#include "Reconstruction/Reconstruct.h"
#ifdef _MSC_VER
#include "Windows.h"
#include "DbgHelp.h"
#endif

#include "Util/Statistic.h"

#include "GenTaskThread.h"

#include <Windows.h>

#include "GenHttpClient.h"


#define ENGINEJOBPATH Settings::getEngineJobQueue()
#define APPPATH QCoreApplication::applicationDirPath()
namespace AICORE = AI3D::CORE;










std::map<int, std::string> maptaskfunction;
int ExecTaskFile();
int ExecTaskFileV2();
int ExecTaskPostHandle();

bool gotNewPendingJobFile = false;

std::string projectfilefullpath = "";
QString NewFileForRun;
bool taskrunning = false;
std::string engineinfofile;
QString cancelledJobFile;
QString failedJobFile;
QString pathSeperator;

QString enginestarttime = "";
std::string versionName;

bool bTestTaskKill4RunningTooLong = false;
QDateTime dtTestTaskKillStart;

bool bNeedRewriteFeedbackFile = false;

FILE* fpTaskLock = NULL;
bool bQuitingApplication = false;
bool bWorkingOnEngineFile = false;
qint64 taskPid = -1;
QString taskPidFile;

QSet<QString> toCheckNetworkPathSet;
bool bNetworkPathAlreadyInvalid = false;

MsgBoxThread* msgBoxThread = nullptr;



bool checkTaskInstanceStatus(QString& sTmpNewFileForRun);


void PostQuitProcess();



void init();
void init()
{
	projectfilefullpath = "";
	taskrunning = false;
	gotNewPendingJobFile = false;
	NewFileForRun = "";
	// Each task can be handled independently (distributed); mapping should be rebuilt when needed.
	maptaskfunction.clear();

	if (fpTaskLock != NULL)
	{
		
		fclose(fpTaskLock);
		fpTaskLock = NULL;
	}
}

std::string getATBlockJobPath(std::string& jobFilename);
QString getATBlockJobPath(QString jobFilename);

// Forward declarations
void getTaskList(const std::string& rootPath, std::vector<std::string>& filenames);

QString progName;

QString userName;
QString hostName;
QString ipAddr;


QString pendingJobPath;
QString runningJobPath;
QString completedJobPath;
QString failedJobPath;
QString cancelledJobPath;

CatchProcess catchProcess;

QString genPendingJobPath;
QString genRunningJobPath;
QString genCompletedJobPath;
QString genFailedJobPath;
QString genCancelledJobPath;




int previousSfmRetryTimes = 0;

int enginePhysicalMemory = 0;

extern bool bSpecialLog;
bool bSpecialLog = false;


QStringList SortJobFile(const QString& runningJobPath, bool bRunning = false);
bool continueCurrentTask(QString &);


QMap<QString, QString> toBeCleanedJobMap;

QMap<QString,int> hasFinishedJobMap;

void DoCleanupJobLockOnceWhileEngineStart()
{
	

	QString lsPendingHighJobPath = pendingJobPath + pathSeperator + "High"; 
	QString lsRunningJobPath = runningJobPath; 

	QDir pendingHighDir(lsPendingHighJobPath);
	QDir runningDir(lsRunningJobPath);

	QStringList toBeRemovedFileList;

	if(pendingHighDir.exists())
	{
		QFileInfoList fileInfoList = pendingHighDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);
		foreach(QFileInfo fileInfo, fileInfoList)
		{
			if (!fileInfo.suffix().compare("lock", Qt::CaseInsensitive))
			{
				QString postFix = "";
				if (JOB_INFO_USE_BIN) {
					postFix = BINFILE_POSTFIX;
				}
				else {
					postFix = JSONFILE_POSTFIX;
				}
				QString jobJson = fileInfo.absolutePath() + pathSeperator + fileInfo.baseName() + postFix;
				if (!QFileInfo(jobJson).exists())
				{
					toBeRemovedFileList.append(fileInfo.absoluteFilePath());
				}
			}
		}
	}

	if (runningDir.exists())
	{
		QFileInfoList fileInfoList = runningDir.entryInfoList(QDir::NoDotAndDotDot|QDir::Files);
		foreach(QFileInfo fileInfo, fileInfoList)
		{
			if (!fileInfo.suffix().compare("lock", Qt::CaseInsensitive))
			{
				QString postFix = "";
				if (JOB_INFO_USE_BIN) {
					postFix = BINFILE_POSTFIX;
				}
				else {
					postFix = JSONFILE_POSTFIX;
				}
				QString jobJson = fileInfo.absolutePath() + pathSeperator + fileInfo.baseName() + postFix;
				if (!QFileInfo(jobJson).exists())
				{
					toBeRemovedFileList.append(fileInfo.absoluteFilePath());
				}
			}
		}
	}

	QString genPendingDir(genPendingJobPath);
	QString genRunningDir(genRunningJobPath);

	QDir pendingGenDir(genPendingDir);
	if (pendingGenDir.exists())
	{
		QFileInfoList fileInfoList = pendingGenDir.entryInfoList((QDir::NoDotAndDotDot | QDir::Files));
		foreach(QFileInfo fileInfo, fileInfoList)
		{
			if (!fileInfo.suffix().compare("lock", Qt::CaseInsensitive))
			{
				QString postFix = JOB_INFO_USE_BIN ? BINFILE_POSTFIX : JSONFILE_POSTFIX;
				QString jobFile = fileInfo.absoluteFilePath() + pathSeperator + fileInfo.baseName() + postFix;
				if (!QFileInfo(jobFile).exists())
				{
					toBeRemovedFileList.append(fileInfo.absoluteFilePath());
				}
			}
		}
	}

	QDir runningGenDir(genRunningDir);
	if (runningGenDir.exists())
	{
		QFileInfoList fileInfoList = runningGenDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);
		foreach(QFileInfo fileInfo, fileInfoList)
		{
			QString postFix = JOB_INFO_USE_BIN ? BINFILE_POSTFIX : JSONFILE_POSTFIX;
			QString jobFile = fileInfo.absolutePath() + pathSeperator + fileInfo.baseName() + postFix;
			if (!QFileInfo(jobFile).exists())
			{
				toBeRemovedFileList.append(fileInfo.absoluteFilePath());
			}
		}
	}

	foreach(QString filePath, toBeRemovedFileList)
	{
		QFile(filePath).remove();
	}
}

void DoCleanupLockFiles()
{
	while (true)
	{
		if (bQuitingApplication)
			return;

		QStringList deleteJobs;

		for (auto it = toBeCleanedJobMap.constBegin(); it != toBeCleanedJobMap.constEnd(); ++it)
		{
			QString jobName = it.key();
			QString jobProjectBlockPath = it.value();

			if (hasFinishedJobMap.contains(jobName))
			{
				int jobState = hasFinishedJobMap[jobName];
				
				QString postFix = "";
				if (JOB_INFO_USE_BIN) {
					postFix = BINFILE_POSTFIX;
				}
				else {
					postFix = JSONFILE_POSTFIX;
				}
				QString lsRunningJobFileName = jobName + postFix;
				QString lsPendingJobFile = pendingJobPath + pathSeperator + "High" + pathSeperator + lsRunningJobFileName;
				QString lsRunningJobFile = runningJobPath + pathSeperator + lsRunningJobFileName;
				QString lsCompletedJobFile = completedJobPath + pathSeperator + lsRunningJobFileName;;
				QString lsCancelledJobFile = cancelledJobPath + pathSeperator + lsRunningJobFileName;
				QString lsFailedJobFile = failedJobPath + pathSeperator + lsRunningJobFileName;

				if (jobState >= 2)
				{
					
					
					






					QString lsPendingJobFileLock = lsPendingJobFile + ".lock";
					QString lsRunningJobFileLock = lsRunningJobFile + ".lock";

					if (QFileInfo(lsPendingJobFile).exists() || QFileInfo(lsRunningJobFile).exists())
					{
						
					}
					else if(QFileInfo(lsCompletedJobFile).exists() || QFileInfo(lsCancelledJobFile).exists() || QFileInfo(lsFailedJobFile).exists())
					{
						if (QFileInfo(lsPendingJobFileLock).exists())
						{
							QFile(lsPendingJobFileLock).remove();
						}

						if (QFileInfo(lsRunningJobFileLock).exists())
						{
							QFile(lsRunningJobFileLock).remove();
						}

						

						QDir dir(jobProjectBlockPath);
						if (dir.exists())
						{
							QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot|
							QDir::Files);

							foreach(QFileInfo entryInfo, entries)
							{

								if (!entryInfo.isDir())
								{
									QString taskPath = entryInfo.absoluteFilePath();
									if (taskPath.endsWith(".pid") || taskPath.endsWith(".lock"))
									{
										QFile(taskPath).remove();
									}
								}
							}
						}

						deleteJobs.append(jobName);
					}
				}
				else
				{
					
					if (!QFileInfo(lsPendingJobFile).exists() &&
						!QFileInfo(lsRunningJobFile).exists() &&
						!QFileInfo(lsCompletedJobFile).exists() &&
						!QFileInfo(lsCancelledJobFile).exists() &&
						!QFileInfo(lsFailedJobFile).exists())
					{
						deleteJobs.append(jobName);
					}
				}
			}
		}

		for (auto& job : deleteJobs)
		{
			hasFinishedJobMap.remove(job);
			toBeCleanedJobMap.remove(job);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

void DoCleaningUpLocks4OneJobFile(QString oldJobFile,QString newJobFile,jobsta_e newJobStatus)
{

	if (newJobStatus == jobsta_e::STATUS_RUNNING)
	{
		
	}
	else if (newJobStatus == jobsta_e::STATUS_CANCLE)
	{
		
	}
	else if (newJobStatus == jobsta_e::STATUS_FAILURE)
	{
		
	}
	else if (newJobStatus == jobsta_e::STATUS_COMPLETE)
	{
		
	}
	else
	{
		if (bSpecialLog)
		{
			std::ostringstream oss;
			oss.clear();
			oss <<  " Unknown job status error!";
			LOGI(oss.str());
		}
	}
}


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
	const char* szVersion = "Dump2Engine";
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
	return EXCEPTION_EXECUTE_HANDLER;
}

LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS lpExceptionInfo)
{
	
	LOGI("Exception Filter");

	bQuitingApplication = true;

	if (IsDebuggerPresent())
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}
	return GenerateMiniDump(lpExceptionInfo);
}

void PrintTimeSum();
void PrintTimeSum(std::string nowstep)
{
	std::ostringstream oss;
	oss.clear();
	oss << nowstep<< "task num: " << maptaskfunction.size() << " ";
	for (const auto& iter : maptaskfunction)
	{
		oss << iter.first << " " << (iter.second) << " ";
	}
	
	LOGI(oss.str());
}

void SigInt_Handler(int nSignal)
{
	LOGI("get signal:" + std::to_string(nSignal));

	PostQuitProcess();
	
}

void SigBreak_Handler(int nSignal)
{
	LOGI("get break signal:" + std::to_string(nSignal));

	PostQuitProcess();
	
}



void ExportTimeSum(JobFullInfo_s jobinfo, int type=0);

void ExportTimeSum(JobFullInfo_s jobinfo,int type)
{
	if (jobinfo.tg.tasksmap.empty())
		return;
	if (GetJobType(jobinfo.JobName) == JOB_BATCH)
		return;

	// Reconstruction/tile jobs use the legacy export path (always write JT_ run times).
	// AT jobs keep the newer guard that skips incomplete task->function mappings.
	const bool isReconstructionJob = (GetJobType(jobinfo.JobName) == JOB_TILE);

	ATTimeSummary_s attimesum;
	attimesum.runinfo = jobinfo.tg.runinfo;
	int missingFunc = 0;

	// AT-only: rebuild task->function mapping from TI_ files when empty.
	if (!isReconstructionJob && maptaskfunction.empty()) {
		try {
			std::string baseDir = AI3D::CORE::File::GetParentDir(jobinfo.tg.job.ProjectPath) + "/" + jobinfo.tg.job.ItemPath;
			baseDir = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(baseDir));
			std::string jobDir = baseDir + jobinfo.JobName + "/";
			std::vector<std::string> taskList;
			getTaskList(jobDir, taskList);
			if (!taskList.empty()) {
				maptaskfunction.clear();
				maptaskfunction[0] = StepAT_function.at(GenTasks);
				for (auto taskfile : taskList) {
					ATTaskInfo task;
					task.load(taskfile);
					maptaskfunction[task.task_.id_] = task.task_.fun_name_;
				}
			}
		} catch (...) {
		}
	}

	for (auto iter : jobinfo.tg.tasksmap)
	{
		if (iter.second.Type == ATLASTTASKTYPE || iter.second.Type == ATCOMPLETETYPE)
			continue;
		attimesum.tasksmap[iter.first].Id = iter.second.Id;
		attimesum.tasksmap[iter.first].StartTime = iter.second.runinfo.StartTime;
		attimesum.tasksmap[iter.first].EndTime = iter.second.runinfo.EndTime;

		attimesum.tasksmap[iter.first].Status = iter.second.Status;

		if (maptaskfunction.count(iter.first))
		{
			attimesum.tasksmap[iter.first].FunctionName = maptaskfunction.at(iter.first);
		}
		else if (!isReconstructionJob)
		{
			missingFunc++;
		}
		attimesum.tasksmap[iter.first].Type = iter.second.Type;
	}

	std::string path = AI3D::CORE::File::GetParentDir(jobinfo.tg.job.ProjectPath) + "/"+ jobinfo.tg.job.ItemPath;

	path = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(path)));

	if (JOB_FEEDBACK_USE_BIN) {
		path = MAKE_TIMESUM_BIN_FILE(path, jobinfo.JobName);
	}
	else {
		path = MAKE_TIMESUM_JSON_FILE(path, jobinfo.JobName);
	}
	// AT-only: if mapping is incomplete, do not overwrite an existing JT_ file.
	if (!isReconstructionJob && missingFunc > 0) {
		try {
			if (std::filesystem::exists(AI3D::CORE::File::BoostPathFromUtf8(path))) {
				return;
			}
			return;
		} catch (...) {
			return;
		}
	}
	attimesum.save(path);
}

bool killTaskProcess()
{
	DWORD processId = 0;
	if (catchProcess.IsProgramRunning("MoldAITask.exe",processId))
	{
		if (bSpecialLog)
		{
			std::ostringstream oss;
			oss.clear();
			oss << "Kill MoldAITask.exe(" << processId << ").";
			LOGI(oss.str());
		
		}


		QProcess process;
		process.start("cmd", QStringList() << "/c" << "taskkill /f /t /im MoldAITask.exe");

		process.waitForStarted();
		process.waitForFinished();
		
		return true;
	}
	else
	{
		if (bSpecialLog)
		{
			std::ostringstream oss;
			oss.clear();
			oss << " MoldAITask.exe is not found ";
			LOGI(oss.str());
		}

		return false;
	}
}

void getTaskList(const std::string& rootPath, std::vector<std::string>& filenames)
{
	try
	{
		const std::filesystem::path fullpath = AICORE::File::BoostPathFromUtf8(rootPath);
		if (!std::filesystem::exists(fullpath))
		{
			return;
		}
	}
	catch (const std::filesystem::filesystem_error& fse)
	{
		std::ostringstream oss;
		oss << "filesystem error:" << fse.code() << " " << fse.what() << " "
			<< AICORE::File::BoostPathToUtf8String(fse.path1()) << " "
			<< AICORE::File::BoostPathToUtf8String(fse.path2());
		LOGI(oss.str());
		return;
	}
	catch (std::exception& ex)
	{
		std::ostringstream oss;
		oss << "exception:" << ex.what();
		LOGI(oss.str());
		return;
	}

	std::vector<std::string> tmp_filenames;
	std::string postFix = "";
	std::string find_str = "";
	std::string firstId = "0";
	if (TASK_USE_BIN) {
		postFix = BINFILE_POSTFIX;
		
		find_str = TASK_DEF_ZERO_BIN_FILE;
	}
	else {
		postFix = JSONFILE_POSTFIX;
		find_str = TASK_DEF_ZERO_JSON_FILE;
		
	}
	tmp_filenames = AI3D::CORE::File::GetFileList(rootPath, postFix);

	tmp_filenames.erase(remove_if(tmp_filenames.begin(), tmp_filenames.end(),
		[find_str](std::string n) { return n == find_str; }),
		tmp_filenames.end());


	int task_count = tmp_filenames.size();
	for (int i = 1; i <= task_count; i++) {
		std::stringstream ss;
		std::string postFix = "";
		if (TASK_USE_BIN) {
			postFix = BINFILE_POSTFIX;
		}
		else {
			postFix = JSONFILE_POSTFIX;
		}
		ss << TASK_DEF_BIN_PREFIX << i << postFix;
		
		for (auto iter2 : tmp_filenames)
		{
			if (iter2.find(ss.str()) != std::string::npos)
			{
				filenames.push_back(iter2);
				break;
			}
		}
	}
}



void SimpleWriteLog(std::string str)
{
	std::ostringstream oss;
	oss.clear();
	oss << str;
	LOGI(oss.str());
}

void SimpleWriteLog(QString str)
{
	std::ostringstream oss;
	oss.clear();
	oss << qstr2str(str);
	LOGI(oss.str());
}

bool LoadFeedbackFile(std::string& feedback_file, JobFeedBack_s& job_feedback, bool retry_more_times)
{
	int retryTimes = 0;
	bool bSuccessful = false;

Retry_It:

	do
	{
		retryTimes++;
		if (TASK_USE_BIN) {
			std::ifstream in = AICORE::File::OpenIfstreamUtf8(feedback_file, std::ios::binary);
			if (!in.is_open())
				break;

			FeedBackFile feedBackFile;
			feedBackFile.Deserialize(in);

			job_feedback.Status = (jobsta_e)feedBackFile.feedBackData.status;
			job_feedback.Percent = feedBackFile.feedBackData.percent;
			std::string msg = feedBackFile.feedBackData.msg;
			// WIN32: msg = UTF82GBK(msg);
			job_feedback.Msg = msg;

			in.close();
			bSuccessful = true;
		}
		else {
			QString feedbackFilename = str2qstr(feedback_file);
			if (feedbackFilename.isEmpty())
			{
				break;
			}
			QFile fileFeedback(feedbackFilename);
			if (!fileFeedback.open(QIODevice::ReadOnly))
			{
				fileFeedback.close();
				break;
			}

			QByteArray ba = fileFeedback.readAll();
			QJsonParseError err;
			QJsonDocument jsonDoc = QJsonDocument::fromJson(ba, &err);

			if (jsonDoc.isNull() || err.error != QJsonParseError::NoError)
			{
				fileFeedback.close();
				break;
			}

			fileFeedback.close();
			QJsonObject jsonObj = jsonDoc.object();
			if (!jsonObj.contains("Status") || !jsonObj.contains("Percent") || !jsonObj.contains("Msg"))
			{
				fileFeedback.close();
				break;
			}

			int  status = jsonObj.value("Status").toInt();
			double percent = jsonObj.value("Percent").toDouble();
			QString msg = jsonObj.value("Msg").toString();

			job_feedback.Status = (jobsta_e)status;
			job_feedback.Percent = (float)percent;
			job_feedback.Msg = qstr2str(msg);

			bSuccessful = true;
		}

		
	} while (false);

	if (!bSuccessful && retry_more_times && retryTimes < 3)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));

		goto Retry_It;
	}

	if (bSuccessful)
	{
		
		SimpleWriteLog("parse feedback file successfully by qjson:" + feedback_file + " retry:" + std::to_string(retryTimes));
	}
	else
	{
		
		SimpleWriteLog("failed to parse feedback file by qjson:" + feedback_file + " retry:" + std::to_string(retryTimes));
	}

	return bSuccessful;
}

void ProcessUnnormaldRunningJobV2()
{
	ATTaskInfo attask;
	try
	{
		if (!attask.load(qstr2str(NewFileForRun)))
			return;
		if (attask.job_ == "")
			return;
	}
	catch (std::exception &ex)
	{
		return;
	}

	std::string postFix = "";
	if (JOB_INFO_USE_BIN) {
		postFix = BINFILE_POSTFIX;
	}
	else {
		postFix = JSONFILE_POSTFIX;
	}
	std::string job_file = qstr2str(Settings::getEngineJobQueue()) + "/Running/" + attask.job_ + postFix;	

	if (!gotNewPendingJobFile)
	{

		return;
	}



	if (!AICORE::File::ExistsFile(job_file))
	{

		if (bSpecialLog)
		{
			LOGI("jobfile:" + job_file + " doesn't exist,and engine is not in initial state." + qstr2str(NewFileForRun) + " " + std::to_string(gotNewPendingJobFile));
		}


		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		if (!gotNewPendingJobFile || NewFileForRun.isEmpty())
		{
			if (bSpecialLog)
			{

				LOGI("jobfile:" + job_file + " doesn't exist,now the engine is reset to initial state.");
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			return;
		}

		if (bSpecialLog)
		{
			
			LOGI("jobfile2:" + job_file + " doesn't exist." + qstr2str(NewFileForRun) + " " + std::to_string(gotNewPendingJobFile));
		}
		if (killTaskProcess())
		{
			if (bSpecialLog)
			{
				LOGI("killed job task:" + job_file + " successfully.");
			}
		}
		else
		{
			if (bSpecialLog)
			{
				
				LOGI("job task has already disappeared:" + job_file);
			}
		}



		return;
	}
	else
	{
		
		return;
	}
}

void ProcessUnnormaldRunningJob()
{
	return;
	Sleep(500);

	ATTaskInfo attask;
	if (!attask.load(NewFileForRun.toStdString()))
		return ;
	if (attask.job_ == "")
		return;
	std::string postFix = "";
	if (JOB_INFO_USE_BIN) {
		postFix = BINFILE_POSTFIX;
	}
	else {
		postFix = JSONFILE_POSTFIX;
	}
	std::string job_file = Settings::getEngineJobQueue().toStdString() + "/Running/" + attask.job_ + postFix;
	std::string feedbakpath = AI3D::CORE::File::GetParentDir(attask.projectFile_) + "/" + attask.blockItem_ + "/";
	std::string current_feedback_file = "";
	if (JOB_FEEDBACK_USE_BIN) {
		current_feedback_file = MAKE_FEEDBAK_BIN_FILE(feedbakpath, attask.job_);
	}
	else {
		current_feedback_file = MAKE_FEEDBAK_JSON_FILE(feedbakpath, attask.job_);
		
	}
	
	
	QString jobFileName = str2qstr(job_file);
	QString lsRunningJobFileName = QFileInfo(jobFileName).fileName();
	QString lsPendingJobFile = pendingJobPath + pathSeperator + lsRunningJobFileName;
	QString lsRunningJobFile = runningJobPath + pathSeperator + lsRunningJobFileName;

	QString lsCompletedJobFile = completedJobPath + pathSeperator + lsRunningJobFileName;;
	QString cancelledJobFile = cancelledJobPath + pathSeperator + lsRunningJobFileName;
	QString failedJobFile = failedJobPath + pathSeperator + lsRunningJobFileName;

	QString jobFilePathLock = str2qstr(job_file) + ".lock";
	QString taskLock = NewFileForRun + ".lock";
	std::string feedLock = current_feedback_file + ".lock";
	QFile lockFilejob(jobFilePathLock);
	QFile lockFilefeed(str2qstr(feedLock));
	QFile lockFiletask(taskLock);

	if (!gotNewPendingJobFile)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		return;
	}

	
	{
		if (!std::filesystem::exists(AICORE::File::BoostPathFromUtf8(job_file)))
		{
			if (AICORE::File::ExistsFile(job_file))
			{
				SimpleWriteLog(job_file + " (job_file) exists.");
			}
			else
			{
				SimpleWriteLog(job_file + " (job_file) doesn't exists.");
			}

			if (QFileInfo::exists(lsPendingJobFile))
				SimpleWriteLog(lsPendingJobFile + " (lsPendingJobFile) exists.");
			else
				SimpleWriteLog(lsPendingJobFile + " (lsPendingJobFile) doesn't exists.");

			if (QFileInfo::exists(lsRunningJobFile))
				SimpleWriteLog(lsRunningJobFile + " (lsRunningJobFile) exists.");
			else
				SimpleWriteLog(lsRunningJobFile + " (lsRunningJobFile) doesn't exists.");

			if (QFileInfo::exists(lsCompletedJobFile))
				SimpleWriteLog(lsCompletedJobFile + " (lsCompletedJobFile) exists.");
			else
				SimpleWriteLog(lsCompletedJobFile + " (lsCompletedJobFile) doesn't exists.");

			if (QFileInfo::exists(cancelledJobFile))
				SimpleWriteLog(cancelledJobFile + " (cancelledJobFile) exists.");
			else
				SimpleWriteLog(cancelledJobFile + " (cancelledJobFile) doesn't exists.");

			if (QFileInfo::exists(failedJobFile))
				SimpleWriteLog(failedJobFile + " (failedJobFile) exists.");
			else
				SimpleWriteLog(failedJobFile + " (failedJobFile) doesn't exists.");

			if (!gotNewPendingJobFile)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				return;
			}

			if (bSpecialLog)
			{
				std::ostringstream oss;
				oss.clear();
				oss << __FUNCTION__ << " LINE " << __LINE__ << " to kill 1";
				LOGI(oss.str());
			}
			killTaskProcess();


			JobFeedBack_s feadback;

			bool bLoadFeedbackFileError = false;
			if (!feadback.load(current_feedback_file))
			{
				bLoadFeedbackFileError = true;
			}
			else
			{
			}

			

			if (bLoadFeedbackFileError)
			{
				bLoadFeedbackFileError = LoadFeedbackFile(current_feedback_file, feadback, true);
				if (bLoadFeedbackFileError)
				{
					LOGI("loading feedback failed : " + current_feedback_file);
				}
				else
				{
					LOGI("loading feedback sucess : " + current_feedback_file);
				}

			}

			if (!bLoadFeedbackFileError && feadback.Status == jobsta_e::STATUS_CANCLE)
			{

				if (QFile::exists(cancelledJobFile) || QFileInfo(cancelledJobFile).exists())
				{

					
				}
				else
				{
					LOGE("cannot find " + qstr2str(lsRunningJobFileName));					
					feadback.Status = jobsta_e::STATUS_UNKNOWN;
					feadback.Percent = 0;
					feadback.save_with_retry(current_feedback_file);

					
				}
			}
			else if (bLoadFeedbackFileError || feadback.Status == jobsta_e::STATUS_RUNNING)
			{
				feadback.Status = jobsta_e::STATUS_CANCLE;
				feadback.save_with_retry(current_feedback_file);
				
			}
			else if (feadback.Status == jobsta_e::STATUS_FAILURE)
			{

				if (QFile::exists(failedJobFile) || QFileInfo(failedJobFile).exists())
				{

					
				}
				else
				{
					feadback.Status = jobsta_e::STATUS_FAILURE;
					feadback.Percent = 0;
					feadback.save_with_retry(current_feedback_file);
					LOGE("cannot find " + qstr2str(lsRunningJobFileName));
					
					
					
				}
			}

			
			init();
			Sleep(100);


		}
		else 
		{
			JobFeedBack_s feadback;
			bool bLoadFeedbackFileError = false;

			if (!feadback.load(current_feedback_file))
			{
				bLoadFeedbackFileError = true;
			}		

			if (bLoadFeedbackFileError)
			{
				bLoadFeedbackFileError = LoadFeedbackFile(current_feedback_file, feadback, true);
				if (bLoadFeedbackFileError)
				{
					LOGI("loading feedback failed : " + current_feedback_file);
				}
				else
				{
					LOGI("loading feedback success : " + current_feedback_file);
				}

			}

			jobsta_e status = feadback.Status;
			if (bLoadFeedbackFileError || feadback.Status == jobsta_e::STATUS_CANCLE || feadback.Status == jobsta_e::STATUS_FAILURE)
			{
				
				if (!gotNewPendingJobFile )
				{

					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
					return;
				}
				if (bSpecialLog)
				{
					std::ostringstream oss;
					oss.clear();
					std::string msg = "";

					if(bLoadFeedbackFileError)
						msg = current_feedback_file + " corrupted.";
					else if (feadback.Status == jobsta_e::STATUS_CANCLE)
						msg = current_feedback_file + " cancle.";
					if (feadback.Status == jobsta_e::STATUS_FAILURE)
						msg = current_feedback_file + " failure.";
					oss << " need kill " << msg;
					LOGI(oss.str());

				}
				killTaskProcess();
				Sleep(100);


				JobFullInfo_s jobinfo(job_file);

				{


					int taskid = jobinfo.tg.GetLastRunningTaskId();
					if (taskid == -1)
					{
						taskid = jobinfo.tg.GetFirstPendingTaskId();

					}

					if (taskid == -1)
					{
						
						return;
					}
					std::string joboutputfile = cancelledJobFile.toStdString();

					if (bLoadFeedbackFileError || feadback.Status == jobsta_e::STATUS_CANCLE)
					{
						feadback.Msg = jobinfo.tg.tasksmap.at(taskid).Msg + " ";
						feadback.Msg += blk_status_str.at(status);
						feadback.save_with_retry(current_feedback_file);
						
						
					}
					else if (feadback.Status == jobsta_e::STATUS_FAILURE)
					{
						std::cout << feadback.Msg << std::endl;
						joboutputfile = failedJobFile.toStdString();
						
					}
					jobinfo.tg.feedback = feadback;
					QString datatime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");			
					jobinfo.tg.tasksmap.at(taskid).Status = int(status);					
					jobinfo.tg.runinfo.runninginfo.EndTime = datatime.toStdString();
					
					SimpleWriteLog("saving job file:" + joboutputfile + " after killing 2.");
					bool bsave = jobinfo.save(joboutputfile);

					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
					std::ostringstream strLine;
					strLine.clear();
					strLine << "print time sum now " << __FUNCTION__ << " LINE " << __LINE__;
					
					PrintTimeSum(strLine.str());
					ExportTimeSum(jobinfo);
					if (std::filesystem::exists(AICORE::File::BoostPathFromUtf8(qstr2str(jobFileName))))
					{
						SimpleWriteLog(QString("inside ProcessUnnormalRunningJob %1 %2 %3 %4").arg(__FILE__).arg(__FUNCTION__).arg(__LINE__).arg(jobFileName));

						QFileInfo finfo(str2qstr(joboutputfile));
						if (finfo.exists())
						{
							SimpleWriteLog(QString("inside ProcessUnnormalRunningJob %1 %2 %3 %4 exists,remove %5").arg(__FILE__).arg(__FUNCTION__).arg(__LINE__)
								.arg(str2qstr(joboutputfile)).arg(jobFileName));
							QFile(jobFileName).remove();
						}
						else
						{
							SimpleWriteLog(QString("inside ProcessUnnormalRunningJob %1 %2 %3 %4 doesn't exists,cannot remove %5").arg(__FILE__).arg(__FUNCTION__).arg(__LINE__)
								.arg(str2qstr(joboutputfile)).arg(jobFileName));

							
						}
					}



					if (std::filesystem::exists(AICORE::File::BoostPathFromUtf8(qstr2str(jobFilePathLock))))
						lockFilejob.remove();
					if (std::filesystem::exists(AICORE::File::BoostPathFromUtf8(feedLock)))
						lockFilefeed.remove();
					if (std::filesystem::exists(AICORE::File::BoostPathFromUtf8(qstr2str(taskLock))))
						lockFiletask.remove();

				}
				init();
				
				Sleep(100);

			}

			

		}
	}
}





void doFeedbackRewriteThread()
{
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
}







void searchUnnormaldRunningJobThread()
{
	while(true)
	{
		if (bQuitingApplication)
			break;

		if (!gotNewPendingJobFile )
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			continue;
		}

		if (NewFileForRun.isEmpty())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			continue;
		}

		if (bQuitingApplication)
			break;

		if (bNetworkPathAlreadyInvalid)
			break;

		ProcessUnnormaldRunningJobV2();
		

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}

void GetPendingJob()
{
	QStringList jobNameList;
	QStringList jobFullPathList;

	std::vector<std::string> jobliststring = JobMonitor::SortPendingJobFile(qstr2str(pendingJobPath));
	for (auto iter : jobliststring)
	{
		jobFullPathList.append(str2qstr(iter));
	}
		
	for (int i = 0; i < jobFullPathList.size(); i++)
	{
		QFileInfo finfo(jobFullPathList.at(i));
		jobNameList.append(finfo.fileName());
	}

	for (int i = 0; i < jobNameList.size(); i++)
	{
		int jobPos = i;

		QString jobFilePath = jobFullPathList.at(jobPos);
		QString jobFileName = jobNameList.at(jobPos);

		if (bQuitingApplication)
			break;

		if (!QFile(jobFilePath).exists())
			continue;

 		std::string jobFileNameString = qstr2str(jobFileName);
		std::string jobBaseNameString = qstr2str(QFileInfo(jobFilePath).baseName());
		std::string jobFilePathString = qstr2str(jobFilePath);

		QString jobFilePathLock = jobFilePath + LOCKFILE_POSTFIX;

		JobFullInfo_s jobpending(jobFilePathString);

		
		
		std::string blockpath = jobpending.tg.job.ProjectPath;
		std::string block = jobpending.tg.job.ItemPath;





		if (jobpending.JobName != jobBaseNameString)
		{
			continue;
		}
		
		FILE* fp = AICORE::File::FopenDenyWriteLockUtf8(qstr2str(jobFilePathLock));
		if (fp != NULL)
		{

	

			
			projectfilefullpath = jobpending.tg.job.ProjectPath;

			JobFeedBack_s feadback;
			std::string nameWithNoext = AI3D::CORE::File::GetFileNameWithoutExtension(jobFileNameString);
			std::string feedback_file = "";
			if (JOB_FEEDBACK_USE_BIN) {
				feedback_file = MAKE_FEEDBAK_BIN_FILE(AICORE::File::GetParentDir(blockpath) + "/" + block, nameWithNoext);
			}
			else {
				feedback_file = MAKE_FEEDBAK_JSON_FILE(AICORE::File::GetParentDir(blockpath) + "/" + block, nameWithNoext);
			}
			
					

			std::string jobdir = qstr2str(ENGINEJOBPATH);
			std::string postFix = "";
			if (JOB_INFO_USE_BIN) {
				postFix = BINFILE_POSTFIX;
			}
			else {
				postFix = JSONFILE_POSTFIX;
			}
			std::string job_file = jobdir + PATH_SEPARATOR_STR + blk_status_str.at(job_status_e::STATUS_RUNNING) + PATH_SEPARATOR_STR + jobBaseNameString + postFix;
			QString cancelledJobFile = cancelledJobPath + PATH_SEPARATOR_STR + jobFileName;
			
			
			{

				JobFullInfo_s jobpending_new;
				
				if(jobpending.tg.tasksmap.size() > 0)
				{
					
					jobpending_new.SetPendingInfo(jobpending.tg.tasksmap.at(0).Type,jobBaseNameString, blockpath, block,
						RunInfo_s(jobpending.tg.runinfo.SubmitHostName, jobpending.tg.runinfo.SubmitUser, jobpending.tg.runinfo.SubmitTime));
				}
				else
				{
					
					jobpending_new.SetPendingInfo(1, jobBaseNameString, blockpath, block,
						RunInfo_s(jobpending.tg.runinfo.SubmitHostName, jobpending.tg.runinfo.SubmitUser, jobpending.tg.runinfo.SubmitTime));

				}
				Run_s runinfo;
				runinfo.RunHostName = getenv("USERNAME");
				runinfo.RunUserName = QHostInfo::localHostName().toStdString();
				QString datatime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
				runinfo.StartTime = datatime.toStdString();
				jobpending_new.tg.runinfo.runninginfo = runinfo;
				jobpending_new.tg.feedback.Status = jobsta_e::STATUS_RUNNING;
				
				feadback = jobpending_new.tg.feedback;
				if (jobpending.tg.tasksmap.size() > 0)
				{
					jobpending_new.tg.tasksmap.at(0).SetRunningInfo(runinfo);
				}
				QString toRunningJobFile = runningJobPath + PATH_SEPARATOR_STR + jobFileName;
				
				std::string basepathstr = AICORE::File::GetParentDir(blockpath);
				basepathstr = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(basepathstr)));
				std::string taskpath = AI3D::CORE::File::JoinPaths(basepathstr, block, jobBaseNameString);
				taskpath = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(taskpath)));
				std::string taskfile = "";
				int firstId = 0;
				if (TASK_USE_BIN) {
					taskfile = MAKE_TASK_BIN_FILE(taskpath, std::to_string(firstId));
				}
				else {
					taskfile = MAKE_TASK_JSON_FILE(taskpath, std::to_string(firstId));
				}

				if (!AICORE::File::ExistsFile(taskfile))
				{
					LOGI("task file not exist (UTF-8 path check): " + taskfile);
					projectfilefullpath = "";
					

					fclose(fp);

					Sleep(100);
					continue;
				}
				if (jobpending_new.save(qstr2str(toRunningJobFile)))
				{
					std::ostringstream strLine;
					strLine.clear();
					strLine << "print time sum now " << __FUNCTION__ << " LINE " << __LINE__;
					PrintTimeSum(strLine.str());
					ExportTimeSum(jobpending_new);

					std::this_thread::sleep_for(std::chrono::milliseconds(1000));

					QFileInfo finfo(toRunningJobFile);
					if (finfo.exists())
					{
						LOGI(qstr2str(QString("save " + jobFilePath + " to " + toRunningJobFile + " ok(remove jobfile.).")));
						QFile(jobFilePath).remove();
					}
					else
					{
						LOGI(qstr2str(QString("save " + jobFilePath + " to " + toRunningJobFile + "ok,but the latter doesn't exist even after waiting for 1 second,try to find out the the cause with great care though deleting the pending job file temporarily(remove jobfile)!!!")));
						QFile(jobFilePath).remove();
					}
					if (!feadback.save_with_retry(feedback_file))
					{
						LOGI("failed to save feedback file inside GetPendingJob!!!");
					}

					if (fpTaskLock != NULL)
					{
						
						fclose(fpTaskLock);
						fpTaskLock = NULL;
					}

					

					QString taskFilePathLock = str2qstr(taskfile) + ".lock";

					std::string taskfile_lock = taskfile + ".lock";
					fpTaskLock = AICORE::File::FopenDenyWriteLockUtf8(taskfile_lock);

					if (fpTaskLock == NULL)
					{
						projectfilefullpath = "";
						fclose(fp);
						break;
					}

					
					projectfilefullpath = jobpending_new.tg.job.ProjectPath;

					
					gotNewPendingJobFile = true; 		

					NewFileForRun = str2qstr(taskfile);

					

					fclose(fp);

					
							
					break;
				}
				else
				{
					LOGI("failed to move pending job file into running job queue,check it to find out what had happened.");
							
					if (fpTaskLock != NULL)
					{
						fclose(fpTaskLock);
						fpTaskLock = NULL;
					}

					
					projectfilefullpath = "";

					

					fclose(fp);
					std::this_thread::sleep_for(std::chrono::milliseconds(500));
					continue;
				}

			}

		}
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(500));			
}

int GetRunningTaskInRunningJob()
{
	QStringList jobNameList;
	QStringList jobFullPathList;

	std::vector<std::string> jobliststring = JobMonitor::SortJobFile(qstr2str(runningJobPath));
	for (auto iter : jobliststring)
	{
		jobFullPathList.append(str2qstr(iter));
	}

	if (bSpecialLog)
	{
		std::ostringstream oss;
		oss.clear();
		oss << __FUNCTION__ << " LINE " << __LINE__ << " running jobs lists " << jobliststring.size();

		
	}

	for (int i = 0; i < jobFullPathList.size(); i++)
	{
		QFileInfo finfo(jobFullPathList.at(i));
		jobNameList.append(finfo.fileName());
	}

	for (int i = 0; i < jobNameList.size(); i++)
	{
		int jobPos = i;

		if (bQuitingApplication)
			break;

		QString jobFilePath = jobFullPathList.at(jobPos);
		QString jobFileName = jobNameList.at(jobPos);

		if (jobFilePath.endsWith(".lock"))
			continue;

		if (!QFile(jobFilePath).exists())
		{
			std::cout << __FUNCTION__ << " LINE " << __LINE__ << qstr2str(jobFilePath) << std::endl;
			continue;
		}

		if (bSpecialLog)
		{
			std::ostringstream oss;
			oss.clear();
			oss << __FUNCTION__ << " LINE " << __LINE__ << " exists " << qstr2str(jobFilePath);
			
		}


		std::string jobFileNameString = qstr2str(jobFileName);
		std::string jobBaseNameString = qstr2str(QFileInfo(jobFilePath).baseName());
		std::string jobFilePathString = qstr2str(jobFilePath);

		QString jobFilePathLock = jobFilePath + LOCKFILE_POSTFIX;
		JobFullInfo_s jobinfo(jobFilePathString);

		if (bSpecialLog)
		{
			std::ostringstream oss;
			oss.clear();
			oss << __FUNCTION__ << " LINE " << __LINE__ << " exists " << qstr2str(jobFilePath) << " taskmaps size " << (std::to_string(jobinfo.tg.tasksmap.size()));

			
		}

		if (jobinfo.tg.tasksmap.empty())
		{
			if (bSpecialLog)
			{
				std::ostringstream oss;
				oss.clear();
				oss << __FUNCTION__ << " LINE " << __LINE__ << " exists " << qstr2str(jobFilePath) << " taskmaps size empty ";


			}

			std::cout << __FUNCTION__ << " LINE " << __LINE__ << " exists " << qstr2str(jobFilePath) << " taskmaps size empty " << std::endl;
			continue;
		}

		if (jobinfo.JobName != jobBaseNameString)
		{
			if (bSpecialLog)
			{
				std::ostringstream oss;
				oss.clear();
				oss << __FUNCTION__ << " LINE " << __LINE__ << " exists " << qstr2str(jobFilePath) << " jobinfo.JobName is not equal jobBaseNameString";
				LOGI(oss.str());

			}

			
			continue;
		}

		
		
		std::string blockpath = jobinfo.tg.job.ProjectPath;
		std::string block = jobinfo.tg.job.ItemPath;

		
		
		if (jobinfo.tg.HasTaskDef0())
			{
				if (bSpecialLog)
				{
					std::ostringstream oss;
					oss.clear();
					oss << __FUNCTION__ << " LINE " << __LINE__ << " exists " << qstr2str(jobFilePath) << " has taskdef0 ";
					
					LOGI(oss.str());

				}
		

				FILE* fpjob = AICORE::File::FopenDenyWriteLockUtf8(qstr2str(jobFilePathLock));

				if (fpjob == NULL)
				{
					if (bSpecialLog)
					{
						std::ostringstream oss;
						oss.clear();
						oss << __FUNCTION__ << " LINE " << __LINE__ << " file " << qstr2str(jobFilePathLock) << " locked failed,continue ";
						LOGI(oss.str());
					}

					
					continue;
				}

				int id = -1;
	

				if (jobinfo.tg.tasksmap.at(0).Status != jobsta_e::STATUS_COMPLETE)
				{
					if (bSpecialLog)
					{
						std::ostringstream oss;
						oss.clear();
						oss << __FUNCTION__ << " LINE " << __LINE__ << " exists " << qstr2str(jobFilePath) << " taskdef0 is not complete.";
						LOGI(oss.str());
					}


					
					id = 0;
					std::string taskfile = "";
					std::string taskBasePath = AICORE::File::GetParentDir(blockpath) + "/" + block + "/" + jobBaseNameString + "/";
					if (TASK_USE_BIN) {
						taskfile = MAKE_TASK_BIN_FILE(taskBasePath, std::to_string(id));
					}
					else {
						taskfile = MAKE_TASK_JSON_FILE(taskBasePath, std::to_string(id));
					}



					std::string taskfile_lock = taskfile + ".lock";
					FILE* fptask = AICORE::File::FopenDenyWriteLockUtf8(taskfile_lock);
					if (fptask == NULL)
					{
						fclose(fpjob);
						continue;
					}
					else
					{
						fpTaskLock = fptask;
						LOGI("locked succ for : " + taskfile);
					}
				}

				if (id == -1)
				{
					id = jobinfo.tg.GetFirstPendingTaskId();
					if (id >= 0)
					{
						if (bSpecialLog)
						{
							std::ostringstream oss;
							oss.clear();
							oss << " exists " << qstr2str(jobFilePath) << " first pending task id " << std::to_string(id);
							LOGI(oss.str());
						}

				
				
					}

					
				}

				int id2 = -1;

				if (id == -1)
				{
					if (bSpecialLog)
					{
						std::ostringstream oss;
						oss.clear();
						oss << " exists " << qstr2str(jobFilePath) << " first pending task id is not good and continue ";

					}


					
				
				
					for (auto& iter : jobinfo.tg.tasksmap)
					{
						id2 = iter.first;
						if (iter.second.Status != int(jobsta_e::STATUS_RUNNING))
						{
							id2 = -1;
							continue;
						}

						if (!jobinfo.tg.IsTaskComplete(id2))
						{
							
							id2 = -1;
							break;
						}
						std::string taskfile = "";
						std::string taskBasePath = AICORE::File::GetParentDir(blockpath) + "/" + block + "/" + jobBaseNameString + "/";
						if (TASK_USE_BIN) {
							taskfile = MAKE_TASK_BIN_FILE(taskBasePath, std::to_string(id2));
						}
						else {
							taskfile = MAKE_TASK_JSON_FILE(taskBasePath, std::to_string(id2));
						}



						std::string taskfile_lock = taskfile + ".lock";
						FILE* fptask = AICORE::File::FopenDenyWriteLockUtf8(taskfile_lock);
						if (fptask == NULL)
						{
							id2 = -1;
							continue;
						}
						else
						{
							fpTaskLock = fptask;
							LOGI("lock task file:" + taskfile);
							break;
						}
					}

					if (id2 == -1)
					{
						fclose(fpjob);
						continue;
					}

					id = id2;

				
				}



				std::string feedback_file = "";
				if (JOB_FEEDBACK_USE_BIN) {
					feedback_file = MAKE_FEEDBAK_BIN_FILE(AICORE::File::GetParentDir(blockpath) + block + "/", jobBaseNameString);
				}
				else {
					feedback_file = MAKE_FEEDBAK_JSON_FILE(AICORE::File::GetParentDir(blockpath) + block + "/", jobBaseNameString);
				}

				std::string taskfile = "";
				std::string taskBasePath = AICORE::File::GetParentDir(blockpath) + block + "/" + jobBaseNameString + "/";
				if (TASK_USE_BIN) {
					taskfile = MAKE_TASK_BIN_FILE(taskBasePath, std::to_string(id));
				}
				else {
					taskfile = MAKE_TASK_JSON_FILE(taskBasePath, std::to_string(id));
				}

				if (bSpecialLog)
				{
					std::ostringstream oss;
					oss.clear();
					oss << " jobfile " << qstr2str(jobFilePath) << " taskfile " << taskfile << " id:" << std::to_string(id) << " id2:" << std::to_string(id2);
					LOGI(oss.str());
				}


				
				if (!AICORE::File::ExistsFile(taskfile))
				{
					
					if (bSpecialLog)
					{
						std::ostringstream oss;
						oss.clear();
						oss << " jobfile " << qstr2str(jobFilePath) << " taskfile is not exist: " << taskfile;
						LOGI(oss.str());
					}




					fclose(fpjob);
					
					if (fpTaskLock != NULL)
					{
						fclose(fpTaskLock);
						fpTaskLock = NULL;
						LOGI("unlocked lock for task file:" + taskfile);
					}

					Sleep(50);
					continue;
				}

	
				std::string taskfile_lock = taskfile + ".lock";
				

				if (fpTaskLock == NULL)
				{
					
					LOGI("preparing to lock taskfile:" + taskfile);
					fpTaskLock = AICORE::File::FopenDenyWriteLockUtf8(taskfile_lock);
				}

				if (bSpecialLog)
				{
					std::ostringstream oss;
					oss.clear();
					oss << __FUNCTION__ << " LINE " << __LINE__ << " jobfile " << qstr2str(jobFilePath) << " taskfile_lock " << (taskfile_lock);

				}

				if (fpTaskLock == NULL)
				{
					if (bSpecialLog)
					{
						std::ostringstream oss;
						oss.clear();
						oss << " jobfile " << qstr2str(jobFilePath) << " taskfile lock failed " << taskfile;
						LOGI(oss.str());

					}

					
					fclose(fpjob);


					Sleep(50);
					continue;
				}

				LOGI("locked taskfile succ:" + taskfile);

				JobFeedBack_s feadback;

				bool bLoadFeedbackFileError = false;
				if (!feadback.load_with_retry(feedback_file))
				{
					if (bSpecialLog)
					{
						std::ostringstream oss;
						oss.clear();
						oss << " jobfile " << qstr2str(jobFilePath) << " load feedback_file failed,feedback status " << feadback.Status;
						LOGI(oss.str());
					}

					bLoadFeedbackFileError = true;




					fclose(fpjob);
					fclose(fpTaskLock);
					fpTaskLock = NULL;

					Sleep(50);
					continue;
				}
				if(feadback.Status != jobsta_e::STATUS_RUNNING)
				{
					
					if (bSpecialLog)
					{
						std::ostringstream oss;
						oss.clear();
						oss << " jobfile " << qstr2str(jobFilePath) << " feedback status is not running " << feadback.Status;
						LOGI(oss.str());

					}



					projectfilefullpath = "";
					
					gotNewPendingJobFile = false;
					NewFileForRun = "";

					fclose(fpjob);
					fclose(fpTaskLock);
					fpTaskLock = NULL;

					LOGI("unlocked job:" + qstr2str(jobFilePath) + " with taskfile:" +taskfile);

					Sleep(50);
					continue;
				}


				
				projectfilefullpath = jobinfo.tg.job.ProjectPath;


				Run_s runinfo;
				runinfo.RunHostName = getenv("USERNAME");
				runinfo.RunUserName = QHostInfo::localHostName().toStdString();
				QString datatime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");

				runinfo.StartTime = datatime.toStdString();

				jobinfo.tg.tasksmap.at(id).runinfo = runinfo;
				jobinfo.tg.tasksmap.at(id).Status = (int)jobsta_e::STATUS_RUNNING;
				
				feadback.Status = jobsta_e::STATUS_RUNNING;
				jobinfo.tg.feedback = feadback;


				if (bSpecialLog)
				{
					std::ostringstream oss;
					oss.clear();
					oss << "save jobfile: " << qstr2str(jobFilePath);
					LOGI(oss.str());
				}
				
				if (jobinfo.save(jobFilePathString))
				{
					if (bSpecialLog)
					{
						std::ostringstream oss;
						oss.clear();
						oss << " jobfile " << qstr2str(jobFilePath) << " to save jobfile success ,export timesum " << (jobFilePathString);
						LOGI(oss.str());

					}
					std::ostringstream strLine;
					strLine.clear();
					strLine << "print time sum now " << __FUNCTION__ << " LINE " << __LINE__;
					PrintTimeSum(strLine.str());
					ExportTimeSum(jobinfo);
						
					projectfilefullpath = jobinfo.tg.job.ProjectPath;

					QString  sTmpNewFileForRun = str2qstr(taskfile);

					if (!continueCurrentTask(sTmpNewFileForRun))
					{
						
						LOGI("never to continue with the task, and unlock it:" + qstr2str(sTmpNewFileForRun));
						fclose(fpTaskLock);
						fpTaskLock = NULL;

						projectfilefullpath = "";
						gotNewPendingJobFile = false;
						NewFileForRun = "";

						fclose(fpjob);
						std::this_thread::sleep_for(std::chrono::milliseconds(500));
						continue;
					}
					else
					{
						
						LOGI("to continue with task:" + qstr2str(sTmpNewFileForRun));
					}


					gotNewPendingJobFile = true;
					NewFileForRun = str2qstr(taskfile);

					if (bSpecialLog)
					{
						std::ostringstream oss;
						oss.clear();
						oss << "getrunningtaskinrunningjob succ:jobfile " << qstr2str(jobFilePath) << " gotNewPendingJobFile " << gotNewPendingJobFile << " NewFileForRun " << qstr2str(NewFileForRun);
						LOGI(oss.str());
					}



					fclose(fpjob);

					Sleep(50);
					return AI3D_SUCCESS;
				}
				else
				{
					if (bSpecialLog)
					{
						std::ostringstream oss;
						oss.clear();
						oss << " to save jobfile failed with unlocking related lock:" << qstr2str(jobFilePath) << " " << taskfile;
						LOGI(oss.str());

					}

					

					fclose(fpTaskLock);
					fpTaskLock = NULL;

					projectfilefullpath = "";
					gotNewPendingJobFile = false;
					NewFileForRun = "";

					fclose(fpjob);
					std::this_thread::sleep_for(std::chrono::milliseconds(500));
					continue;
				}
			}
		else             
		{
			if (jobinfo.tg.tasksmap.at(0).Status != jobsta_e::STATUS_COMPLETE)
			{
				if (bSpecialLog)
				{
					std::ostringstream oss;
					oss.clear();
					oss << __FUNCTION__ << " LINE " << __LINE__ << " exists/prod " << qstr2str(jobFilePath) << " taskdef0 is not complete.status"
						<< jobinfo.tg.tasksmap.at(0).Status;
					LOGI(oss.str());
					std::cout << oss.str() << std::endl;
				}

				std::string feedback_file = "";
				if (JOB_FEEDBACK_USE_BIN) {
					feedback_file = MAKE_FEEDBAK_BIN_FILE(AICORE::File::GetParentDir(blockpath) + "/" + block, jobBaseNameString);
				}
				else {
					feedback_file = MAKE_FEEDBAK_JSON_FILE(AICORE::File::GetParentDir(blockpath) + "/" + block, jobBaseNameString);
				}

				if (jobinfo.tg.tasksmap.at(0).Status == jobsta_e::STATUS_CANCLE
					|| jobinfo.tg.tasksmap.at(0).Status == jobsta_e::STATUS_FAILURE)
				{
					if (jobinfo.tg.tasksmap.at(0).Status == jobsta_e::STATUS_CANCLE)
					{
						LOGI("task running/prod cancelled for : " + jobFilePath.toStdString());
						
					}
					else if (jobinfo.tg.tasksmap.at(0).Status == jobsta_e::STATUS_FAILURE)
					{
						LOGI("task running/prod failed for : " + jobFilePath.toStdString());
						
					}
					 
					continue;
				}

				if (jobinfo.tg.tasksmap.at(0).Status == jobsta_e::STATUS_PENDDING)
				{
					LOGI("task running/prod pending for : " + jobFilePath.toStdString());
					

				}
				else if (jobinfo.tg.tasksmap.at(0).Status == jobsta_e::STATUS_RUNNING)
				{
					LOGI("task running/prod running for : " + jobFilePath.toStdString());
					
				}
				else
				{
					LOGI("task running/prod unknown for : " + jobFilePath.toStdString());
					
				}


				{
					FILE* fpjob = AICORE::File::FopenDenyWriteLockUtf8(qstr2str(jobFilePathLock));
					if (fpjob == NULL)
					{
						LOGI("locked failed/prod for : " + jobFilePath.toStdString());
						

						continue;
					}

					int id = 0;

					std::string taskfile = "";
					std::string taskBasePath = AICORE::File::GetParentDir(blockpath) + "/" + block + "/" + jobBaseNameString + "/";
					if (TASK_USE_BIN) {
						taskfile = MAKE_TASK_BIN_FILE(taskBasePath, std::to_string(id));
					}
					else {
						taskfile = MAKE_TASK_JSON_FILE(taskBasePath, std::to_string(id));
					}

					std::string taskfile_lock = taskfile + ".lock";
					FILE* fptask = AICORE::File::FopenDenyWriteLockUtf8(taskfile_lock);
					if (fptask == NULL)
					{
						fclose(fpjob);
						continue;
					}

					fpTaskLock = fptask;
					LOGI("locked succ/prod for : " + taskfile);
					

					QString NewFileForRunTemp = str2qstr(taskfile);
					
					fclose(fpjob);

					if (!checkTaskInstanceStatus(NewFileForRunTemp))
					{
						fclose(fpTaskLock);
						fpTaskLock = nullptr;

						LOGI("task is unavailable/prod : " + NewFileForRunTemp.toStdString());
						

						std::this_thread::sleep_for(std::chrono::milliseconds(150));
						continue;
					}

					gotNewPendingJobFile = true;
					
					NewFileForRun = str2qstr(taskfile);
					
					projectfilefullpath = jobinfo.tg.job.ProjectPath;

					if (bSpecialLog)
					{
						std::ostringstream oss;
						oss.clear();
						oss << "get succ:jobfile/prod " << taskfile << " gotNewPendingJobFile " << gotNewPendingJobFile << " NewFileForRun " << qstr2str(NewFileForRun);
						LOGI(oss.str());
						std::cout << oss.str() << std::endl;
					}

					fclose(fpTaskLock);
					fpTaskLock = nullptr;
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
					return AI3D_SUCCESS;
				}

				LOGI("task completed/prod for : " + jobFilePath.toStdString());
				

				int id = -1;
				if (jobinfo.tg.tasksmap.at(0).Status == jobsta_e::STATUS_RUNNING)
				{
					
					continue;
					if (1)
					{
						FILE* fpjob = AICORE::File::FopenDenyWriteLockUtf8(qstr2str(jobFilePathLock));
						if (fpjob == NULL)
						{
							if (bSpecialLog)
							{
								std::ostringstream oss;
								oss.clear();
								oss << __FUNCTION__ << " LINE " << __LINE__ << " file " << qstr2str(jobFilePathLock) << " locked failed,continue ";
								LOGI(oss.str());
								
							}
							continue;
						}

						id = 0;
						std::string taskfile = "";
						std::string taskBasePath = AICORE::File::GetParentDir(blockpath) + "/" + block + "/" + jobBaseNameString + "/";
						if (TASK_USE_BIN) {
							taskfile = MAKE_TASK_BIN_FILE(taskBasePath, std::to_string(id));
						}
						else {
							taskfile = MAKE_TASK_JSON_FILE(taskBasePath, std::to_string(id));
						}

						std::string taskfile_lock = taskfile + ".lock";
						FILE* fptask = AICORE::File::FopenDenyWriteLockUtf8(taskfile_lock);
						if (fptask == NULL)
						{
							fclose(fpjob);
							continue;
						}
						else
						{
							fpTaskLock = fptask;
							LOGI("locked succ for : " + taskfile);
							
						}

						
						projectfilefullpath = jobinfo.tg.job.ProjectPath;
						QString  sTmpNewFileForRun = str2qstr(taskfile);

						if (!continueCurrentTask(sTmpNewFileForRun))
						{
							LOGI("never to continue with the task, and unlock it:" + qstr2str(sTmpNewFileForRun));
							
							fclose(fpTaskLock);
							fpTaskLock = NULL;
							projectfilefullpath = "";
							gotNewPendingJobFile = false;
							NewFileForRun = "";
							fclose(fpjob);
							std::this_thread::sleep_for(std::chrono::milliseconds(500));
							continue;
						}
						else
						{
							LOGI("to continue with task:" + qstr2str(sTmpNewFileForRun));
							
						}

						gotNewPendingJobFile = true;
						NewFileForRun = str2qstr(taskfile);

						if (bSpecialLog)
						{
							std::ostringstream oss;
							oss.clear();
							oss << "getrunningtaskinrunningjob succ:jobfile " << qstr2str(jobFilePath) << " gotNewPendingJobFile " << gotNewPendingJobFile << " NewFileForRun " << qstr2str(NewFileForRun);
							LOGI(oss.str());
							
						}

						fclose(fpjob);
						Sleep(50);
						return AI3D_SUCCESS;
					}
				}

				
				continue;
			}

			
		}
	}

	
	return AI3D_FAILURE;
}


void searchPendingJobThread2()
{

	while (true)
	{
		if (bQuitingApplication)
			break;

		if (bNetworkPathAlreadyInvalid)
			break;

		QStringList checkPathList;
		QStringList errPathList;

		
		checkPathList.append(ENGINEJOBPATH);


		if (gotNewPendingJobFile || !NewFileForRun.isEmpty())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		
			continue;
		}

		if(taskrunning)
		{


			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			continue;
		}

		
		if (fpTaskLock != NULL)
		{
			fclose(fpTaskLock);
			fpTaskLock = NULL;
			
			if (bSpecialLog)
			{
				std::ostringstream oss;
				oss.clear();
				oss << " no taskfile should have been locking here,why?" << qstr2str(NewFileForRun);
				LOGI(oss.str());
			}
		}

		
		if (GetRunningTaskInRunningJob() != AI3D_SUCCESS)
		{
	
			if (bSpecialLog)
			{
				std::ostringstream oss;
				oss.clear();
				oss << "no running job ,to get pending job";
				
				
			}

			GetPendingJob();

		}
#if 1
		
		
		if (bSpecialLog)
		{

			
		}

		if (gotNewPendingJobFile && !NewFileForRun.isEmpty())
		{		

			
			if (fpTaskLock != NULL)
			{
				LOGI("before execing task file:" + qstr2str(NewFileForRun));
				std::string msg = "Execing task :" + qstr2str(NewFileForRun);
				LogConsole(msg);
			}
			else
			{
				LOGI("before execing task file2:" + qstr2str(NewFileForRun));
				std::string msg = "Execing task :" + qstr2str(NewFileForRun);
				LogConsole(msg);
			}

			taskPid = qApp->applicationPid();
			taskPidFile = NewFileForRun + ".pid";
			QString currentFile = NewFileForRun;
			
			
			int retCode = ExecTaskFileV2();
			if(retCode == -1)
			{
			
				LOGI("execing task file failed:" + qstr2str(currentFile));
				
				std::string msg = "Execing task file failed :" + qstr2str(currentFile);
				LogConsole(msg);
				init();
			}

			if (fpTaskLock != NULL) {
				LOGI("after execing task file:gotNewPendingJobFile:" + qstr2str(currentFile));
				std::string msg = "Task:" + qstr2str(currentFile) + " Finished !";
				LogConsole(msg);
			}
			else {
				LOGI("after execing task file:gotNewPendingJobFile2:" + qstr2str(currentFile));
				std::string msg = "Task:" + qstr2str(currentFile) + " Finished !";
				LogConsole(msg);
			}

			taskPid = -1;
			NewFileForRun = "";

		}
#endif
		if(!bNetworkPathAlreadyInvalid)
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

}


int RunAlgorithmCommonDummy2(const std::string function_name, const std::string& task_json_file, int taskIndex)
{
	QString path = QCoreApplication::applicationDirPath();
	QString workingDirectory = path;

	if (std::filesystem::exists(AICORE::File::BoostPathFromUtf8(task_json_file + ".lock")))
	{
		return -1;
	}

	if (!gotNewPendingJobFile && NewFileForRun.isEmpty())
	{

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		init();
		return -1;
	}
	if (function_name=="")
	{
		return -1;
	}
	if(taskrunning)
	{
		return -1;
	}
	
	if (bSpecialLog)
	{
		std::ostringstream oss;
		oss.clear();
		oss << __FUNCTION__ << " LINE " << __LINE__ << " to kill 3 ";
		LOGI(oss.str());
		
	}
	killTaskProcess();
	QString timenow = (QDateTime::currentDateTime()).toString("yyyy-MM-dd hh:mm:ss.zzz");
	std::cout << "[" << timenow.toStdString() << "] Engine: Run Task " << std::endl;

	path.append("/MoldAITask.exe");

	QStringList argumentList;
	argumentList << str2qstr(const_cast<std::string &>(task_json_file));

	qint64 pid = -1;

	QProcess process;
	bool started = process.startDetached(path, argumentList, workingDirectory, &pid);
	if (!started || pid <= 0)
	{
		LOGE("failed to start MoldAITask.exe, task file: " + task_json_file);
		return -1;
	}
	if (bSpecialLog)
	{
		std::ostringstream oss;
		oss.clear();
		oss << __FUNCTION__ << " LINE " << __LINE__;
		LOGI(oss.str());
		
	}
	int state = 0;
	
	taskrunning = true;
	if (bSpecialLog)
	{
		std::ostringstream oss;
		oss.clear();
		oss << __FUNCTION__ << " LINE " << __LINE__;
		LOGI(oss.str());
		
	}
	{
		while (true)
		{
			bool running = false;
#ifdef WIN32
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, static_cast<DWORD>(pid));
			if (hProcess != NULL)
			{
				DWORD exitCode = 0;
				if (GetExitCodeProcess(hProcess, &exitCode))
				{
					running = (exitCode == STILL_ACTIVE);
				}
				CloseHandle(hProcess);
			}
#else
			running = catchProcess.IsProgramRunning("MoldAITask.exe");
#endif
			if (!running)
			{
				break;
			}
			if (bSpecialLog)
			{
				std::ostringstream oss;
				oss.clear();
				oss << __FUNCTION__ << " LINE " << __LINE__;
				LOGI(oss.str());
				
			}
			state++;
			Sleep(100);
		}
	}
	

	return state;
}

int ExecTaskPostHandle(QString jobFile, QString path) {

  QProcess finishProcess;
  QStringList finishArgumentList;
  
  QString commandType = "cancel";
  finishArgumentList << commandType;
  finishArgumentList << jobFile;
  QString exceptionMsg;

  QObject::connect(&finishProcess, &QProcess::readyRead, [&] {
	  while (finishProcess.canReadLine())
	  {
		  QString strTaskOutput = finishProcess.readLine();
		  std::cout << "[MoldAITask]" << qstr2str(strTaskOutput);

		  if (strTaskOutput.contains("Exception:", Qt::CaseSensitive))
		  {
			  exceptionMsg = strTaskOutput;
		  }
	  }
	  });

  finishProcess.start(path, finishArgumentList);

  int finishState = 0;

  bool finSuce = finishProcess.waitForStarted(-1);
  if (!finSuce) {
	  
  }
  finishProcess.waitForFinished(-1);
  

  finishProcess.close();
  int iTaskRetVal = finishProcess.exitCode();
  return iTaskRetVal;
}

int ExecTaskFileV2()
{
	QString path = APPPATH;
	QString workingDirectory = path;

	std::string fileName = qstr2str(NewFileForRun);

	ATTaskInfo atparam;;
	
	std::string msg = "load task file:" + fileName;
	
	LOGI(msg);
	if (!AICORE::File::ExistsFile(fileName))
		return -1;
	if (!atparam.load(fileName))
		return -1;
	std::string function = atparam.task_.fun_name_;
	if (atparam.projectFile_.empty())
		return -1;

	QString exceptionMsg;

	std::string blockpath1 = atparam.projectFile_;
	std::string blockpath = atparam.projectFile_;

	QString qsBlockPath1 = str2qstr(blockpath1);
	QString qsBlockPath = str2qstr(blockpath);


	std::string block = atparam.blockItem_;
	std::string jobname = atparam.job_;


	JobFeedBack_s feadback;
	std::string feedback_file = "";
	if (JOB_FEEDBACK_USE_BIN) {
		feedback_file = MAKE_FEEDBAK_BIN_FILE(AICORE::File::GetParentDir(blockpath) + "/" + block, jobname);
	}
	else {
		feedback_file = MAKE_FEEDBAK_JSON_FILE(AICORE::File::GetParentDir(blockpath) + "/" + block, jobname);
	}
	bool bLoadFeedbackFileError = false;

	std::string projectPath = AICORE::File::GetParentDir(blockpath);
	

	if (bNetworkPathAlreadyInvalid)
	{
		std::string msg = "bNetworkPathAlreadyInvalid:" + fileName;
		std::cout << msg << std::endl;
		LOGI(msg);
		return -1;
	}

	

	QStringList checkPathList;
	QStringList errPathList;

	
	checkPathList.append(str2qstr(projectPath));

	checkPathList.append(ENGINEJOBPATH);


	int type = atparam.task_.type_;
	int taskid = atparam.task_.id_;
	if(type!=4)
	{
			std::vector<std::string> taskList;
			std::string dirPath = qstr2str(getATBlockJobPath(str2qstr(fileName)));

			std::string dirnew = getATBlockJobPath(fileName);
			getTaskList(dirPath, taskList);

			toBeCleanedJobMap.insert(str2qstr(jobname), str2qstr(dirnew));
			hasFinishedJobMap.insert(str2qstr(jobname), 0); 

			int task_count = taskList.size();

			// Rebuild mapping for current job
			maptaskfunction.clear();
			maptaskfunction[0] = StepAT_function.at(GenTasks);

			for (auto taskfile : taskList)
			{
				ATTaskInfo task;
				task.load(taskfile);

				
				
				maptaskfunction[task.task_.id_] = task.task_.fun_name_;
			}
	}
	
	
	if (!AICORE::File::ExistsFile(feedback_file))
		return -1;
	if (!feadback.load_with_retry(feedback_file))
	{
		bLoadFeedbackFileError = true;
		return -1;
	}
	if (feadback.Status == jobsta_e::STATUS_PENDDING)
	{
		
		
		feadback.Percent = 0.0;
		feadback.Status = jobsta_e::STATUS_RUNNING;
		feadback.save_with_retry(feedback_file);

		
	}

	
	if (feadback.Status != jobsta_e::STATUS_RUNNING)
	{
		
		
		LOGI("current job file is not running now:" + qstr2str(NewFileForRun));
		if (fpTaskLock != NULL)
		{
			fclose(fpTaskLock);
			fpTaskLock = NULL;
			LOGI("current job file is not running now:" + qstr2str(NewFileForRun) + " ,unlock taskfile lock.");
		}

		std::string msg = fileName + "is not running."  ;
		
		LOGI(msg);
		return -1;
	}

	
	if (!gotNewPendingJobFile && NewFileForRun.isEmpty())
	{
		
		if (fpTaskLock != NULL)
		{
			fclose(fpTaskLock);
			fpTaskLock = NULL;
		}

		LOGI("current job file is not in normal state:" + qstr2str(NewFileForRun));
		return -1;
	}

	if (type == ATLASTTASKTYPE)
	{					
					int resultCode = AI3D_SUCCESS;

					std::string postFix = "";
					if (JOB_INFO_USE_BIN) {
						postFix = BINFILE_POSTFIX;
					}
					else {
						postFix = JSONFILE_POSTFIX;
					}
					std::string jobdirstr = qstr2str(runningJobPath) + "/" + jobname + postFix;


					JobFullInfo_s jobinfo(jobdirstr);
					if (jobinfo.tg.IsTaskComplete(taskid))
					{

						QString jobFilePathLock = str2qstr(jobdirstr) + ".lock";
						std::string postFix = "";
						if (JOB_INFO_USE_BIN) {
							postFix = BINFILE_POSTFIX;
						}
						else {
							postFix = JSONFILE_POSTFIX;
						}
						std::string jobdirstr = qstr2str(runningJobPath) + "/" + jobname + postFix;
						JobFullInfo_s jobinfo(jobdirstr);
						jobFilePathLock = str2qstr(jobdirstr) + ".lock";

						
						FILE* fp = AICORE::File::FopenDenyWriteLockUtf8(qstr2str(jobFilePathLock));

						if (fp != NULL)
						{

							feadback.Status = job_status_e::STATUS_COMPLETE;
							feadback.Percent = COMPLETE_PROGRESS;
							QString datatime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");

							jobinfo.tg.tasksmap.at(taskid).runinfo.EndTime = datatime.toStdString();
							jobinfo.tg.tasksmap.at(taskid).Status = int(job_status_e::STATUS_COMPLETE);
							jobinfo.tg.tasksmap.at(taskid).Percent = float(COMPLETE_PROGRESS);
							feadback.Msg = GetTaskEndingString(jobinfo.JobName);

							hasFinishedJobMap.insert(str2qstr(jobinfo.JobName), 2);

							jobinfo.tg.feedback = feadback;

							jobinfo.tg.runinfo.runninginfo.EndTime = datatime.toStdString();

							QString jobFileName = str2qstr(jobdirstr);
							QString lsRunningJobFileName = QFileInfo(jobFileName).fileName();
							QString lsCompletedJobFile = completedJobPath + pathSeperator + lsRunningJobFileName;;
							bool bsave = jobinfo.save(qstr2str(lsCompletedJobFile));

							if (!bsave)
							{
								if (bSpecialLog)
								{
									std::ostringstream oss;
									oss.clear();
									oss << __FUNCTION__ << " LINE " << __LINE__ << "  " << "failed to save jobfile into completed dir.";
									LOGI(oss.str());
								}
							}
							std::ostringstream strLine;
							strLine.clear();
							strLine << "print time sum now " << __FUNCTION__ << " LINE " << __LINE__;
							PrintTimeSum(strLine.str());
							ExportTimeSum(jobinfo);

							std::this_thread::sleep_for(std::chrono::milliseconds(500));

							if (bSpecialLog)
							{
								LOGI("remove " + qstr2str(jobFileName) + " for completion status.");
							}

							
							QFile(jobFileName).remove();

							
							if (!feadback.save_with_retry(feedback_file))
							{
								if (bSpecialLog)
								{
									std::ostringstream oss;
									oss.clear();
									oss << __FUNCTION__ << " LINE " << __LINE__ << "  " << "failed to save feedback with complete status.";
									LOGI(oss.str());
								}
							}

							LOGI("after remove jobfile:" + qstr2str(jobFileName) + " taskfile:" + qstr2str(NewFileForRun));

							init();
							fclose(fp);
							


							return resultCode;
						}
						else
						{

						}
					}
					else
					{
						LOGI("jobfile:" + jobdirstr + " taskfile:" + qstr2str(NewFileForRun));
						init();
						Sleep(30);
						return -1;
					}
	}
	else if (type == ATCOMPLETETYPE)
	{
		{
			int resultCode = AI3D_SUCCESS;

			std::string postFix = "";
			if (JOB_INFO_USE_BIN) {
				postFix = BINFILE_POSTFIX;
			}
			else {
				postFix = JSONFILE_POSTFIX;
			}
			std::string jobdirstr = qstr2str(runningJobPath) + "/" + jobname + postFix;


			JobFullInfo_s jobinfo(jobdirstr);
			if (jobinfo.tg.IsTaskComplete(taskid))
			{

				QString jobFilePathLock = str2qstr(jobdirstr) + ".lock";
				std::string postFix = "";
				if (JOB_INFO_USE_BIN) {
					postFix = BINFILE_POSTFIX;
				}
				else {
					postFix = JSONFILE_POSTFIX;
				}
				std::string jobdirstr = qstr2str(runningJobPath) + "/" + jobname + postFix;
				JobFullInfo_s jobinfo(jobdirstr);
				jobFilePathLock = str2qstr(jobdirstr) + ".lock";

				
				FILE* fp = AICORE::File::FopenDenyWriteLockUtf8(qstr2str(jobFilePathLock));

				if (fp != NULL)
				{
					
					
					{
						feadback.Msg = atparam.task_.msg_;
						feadback.save_with_retry(feedback_file);

						
						jobinfo.tg.tasksmap.at(taskid).Status = jobsta_e::STATUS_COMPLETE;
						jobinfo.tg.tasksmap.at(taskid).Percent = float(COMPLETE_PROGRESS);
						bool bsave = jobinfo.save(jobdirstr);
						std::ostringstream strLine;
						strLine.clear();
						strLine << "print time sum now " << __FUNCTION__ << " LINE " << __LINE__;
						PrintTimeSum(strLine.str());
						ExportTimeSum(jobinfo);
						init();

						LOGI("save jobfile:" + jobdirstr + " taskfile:" + qstr2str(NewFileForRun) + " resultCode:" + std::to_string(resultCode));

						fclose(fp);

					}

					return resultCode;
				}

			}
			else
			{
				LOGI("jobfile:" + jobdirstr + " taskfile:" + qstr2str(NewFileForRun));
				init();
				Sleep(30);
				return -1;
			}
		}
	}
	else 
	{
		if (bSpecialLog)
		{
			std::ostringstream oss;
			oss.clear();
			oss << " " << qstr2str(NewFileForRun) << " type " << type;;
			LOGI(oss.str());

		}
		
		
		QString timenow = (QDateTime::currentDateTime()).toString("yyyy-MM-dd hh:mm:ss.zzz");
		if (bSpecialLog)
		{
			std::ostringstream oss;
			oss.clear();
			oss << __FUNCTION__ << " LINE " << __LINE__ << " " << "[" << timenow.toStdString() << "] Engine: Run Task ";
			LOGI(oss.str());
		}


		path.append("/MoldAITask.exe");

		QStringList argumentList;

		
		argumentList << NewFileForRun;

		QString currentFileForRun = NewFileForRun;

		qint64 pid = -1;

		QProcess process;

		if (bSpecialLog)
		{
			std::ostringstream oss;
			oss.clear();
			oss << "start " << qstr2str(NewFileForRun);
			LOGI(oss.str());
			std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
		}
		QObject::connect(&process, &QProcess::readyRead, [&] {
			while (process.canReadLine())
			{
				QString strTaskOutput = process.readLine();
				

				if (strTaskOutput.contains("Exception:", Qt::CaseSensitive))
				{
					exceptionMsg = strTaskOutput;
				}
			}
			});

		hasFinishedJobMap.insert(str2qstr(jobname), 1);

		process.start(path, argumentList);

		int state = 0;
		
		bool suce = process.waitForStarted(-1);

		if (bSpecialLog)
		{
			std::ostringstream oss;
			oss.clear();
			oss << " " << qstr2str(NewFileForRun) << " succ " << suce;
			LOGI(oss.str());

		}

		if (!suce)
		{
			std::ostringstream oss;
			oss.clear();
			oss << " " << qstr2str(NewFileForRun) << " MoldAITask start failed. exe=" << qstr2str(path)
				<< " err=" << process.errorString().toUtf8().constData();
			LOGI(oss.str());
			

			

			if (fpTaskLock != NULL)
			{
				fclose(fpTaskLock);
				fpTaskLock = NULL;
				LOGI(qstr2str(NewFileForRun) + "start failed,release task file lock.");
			}

			return -1;
		}

		
		if (bSpecialLog)
		{
			std::ostringstream oss;
			oss.clear();
			
			oss << qstr2str(NewFileForRun) << " started successfully." << suce;


			LOGI(oss.str());

			
		}


		taskrunning = true;

		
		process.waitForFinished(-1);
		int iTaskRetVal = -1;

		if (process.exitStatus() == QProcess::NormalExit)
		{

			iTaskRetVal = process.exitCode();
			if (iTaskRetVal == 1099)
			{
				if (exceptionMsg.isEmpty())
				{
					exceptionMsg = "MoldAITask Crashed.";
				}
			}

			if (bSpecialLog)
			{
				std::ostringstream oss;
				oss.clear();
				
							
				oss << " task exit with code:" << process.exitCode();
				LOGI(oss.str());
			}

			
			if (iTaskRetVal < MOLDAI_SUCCESS)
			{
				
				process.close();

				
				if (bSpecialLog)
				{

					std::ostringstream oss;
					oss.clear();
					
								
					oss << " task exit with code(less than 100000):" << process.exitCode();
					LOGI(oss.str());
				}

				ATTaskInfo attask;
				attask.load(qstr2str(NewFileForRun));
				std::string postFix = "";
				if (JOB_INFO_USE_BIN) {
					postFix = BINFILE_POSTFIX;
				}
				else {
					postFix = JSONFILE_POSTFIX;
				}
				std::string job_file = qstr2str(Settings::getEngineJobQueue()) + "/Running/" + attask.job_ + postFix;

				JobFullInfo_s jobinfo(job_file);
				QString jobFilePathLock = str2qstr(job_file) + ".lock";
				FILE* fp = AICORE::File::FopenDenyWriteLockUtf8(qstr2str(jobFilePathLock));

				if (fp != NULL)
				{
					std::ostringstream strLine;
					strLine.clear();
					strLine << "print time sum now " << __FUNCTION__ << " LINE " << __LINE__;
					PrintTimeSum(strLine.str());
					
					ExportTimeSum(jobinfo);
					fclose(fp);

					
					LOGI("save job's time sum info::" + job_file);
				}


				if (fpTaskLock != NULL)
				{
					fclose(fpTaskLock);
					fpTaskLock = NULL;
					LOGI("release taskfile lock:" + qstr2str(NewFileForRun));
				}
				std::string msg = fileName + " task value is less than 100000.";
				std::cout << msg << std::endl;
				LOGI(msg);


				int postResult = ExecTaskPostHandle(currentFileForRun, path);
                if (postResult != MOLDAI_USER_CANCEL && postResult != MOLDAI_SUCCESS) {
					std::string errStr = "get post  handle error:" + std::to_string(postResult);
					LOGI(errStr);
					LogConsole(errStr);
                }
				return -1;
				
			}

					

		}
		else
		{
			
			
			std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
			if (bSpecialLog)
			{
				std::ostringstream oss;
				oss.clear();
				
				
				oss << qstr2str(NewFileForRun) << " task crashed:" << process.exitCode();
				LOGI(oss.str());
			}

			process.close();

			if (fpTaskLock != NULL)
			{
				fclose(fpTaskLock);
				fpTaskLock = NULL;
				LOGI("relase taskfile lock:" + qstr2str(NewFileForRun));
			}

			exceptionMsg = "MoldAITask Crashed.";
			
			
			iTaskRetVal = 1099;
		}
		process.close();

		bLoadFeedbackFileError = false;
		bool readrtn = feadback.load_with_retry(feedback_file);
		if (!readrtn)
		{
			bLoadFeedbackFileError = true;
		}
		if (iTaskRetVal == MOLDAI_SUCCESS && type == ATSTARTTYPE && fabs(feadback.Percent - 1.0) <= 0.0001)
		{
			
			if (bSpecialLog)
			{
				std::ostringstream oss;
				
				
				

				oss.clear();
				oss << qstr2str(NewFileForRun) << " gentask finish: " << feadback.Percent;
				LOGI(oss.str());
			}

			
		}

		
		timenow = (QDateTime::currentDateTime()).toString("yyyy-MM-dd hh:mm:ss.zzz");

		if (bSpecialLog)
		{
			std::ostringstream oss;
			oss.clear();
			oss << __FUNCTION__ << " LINE " << __LINE__ << "  " << "[" << timenow.toStdString() << "] Engine: End Task " << state;
			LOGI(oss.str());
		}

		
		Sleep(1000);

		if (bSpecialLog)
		{
			std::ostringstream oss;
			oss.clear();
			oss << "to process complete " << gotNewPendingJobFile << " " << qstr2str(NewFileForRun);
			LOGI(oss.str());
		}

		if (!gotNewPendingJobFile && NewFileForRun.isEmpty())
		{
			if (fpTaskLock != NULL)
			{
				fclose(fpTaskLock);
				fpTaskLock = NULL;
				LOGI("unlock taskfile lock:" + qstr2str(NewFileForRun));
			}
			std::string msg = std::to_string(gotNewPendingJobFile) + " " + NewFileForRun.QString::toStdString() + " !gotNewPendingJobFile and NewFileForRun.isEmpty()";
			
			LOGI(msg);
			return -1;
		}
		std::string postFix = "";
		if (JOB_INFO_USE_BIN) {
			postFix = BINFILE_POSTFIX;
		}
		else {
			postFix = JSONFILE_POSTFIX;
		}
		std::string jobdirstr = qstr2str(ENGINEJOBPATH) + "/Running/" + atparam.job_ + postFix;
		std::string jobdirstr_cancelled = qstr2str(ENGINEJOBPATH) + "/Cancelled/" + atparam.job_ + postFix;
		std::string jobdirstr_failed = qstr2str(ENGINEJOBPATH) + "/Failed/" + atparam.job_ + postFix;

		JobFullInfo_s jobinfo(jobdirstr);
		QString jobFilePathLock = str2qstr(jobdirstr) + ".lock";

		bLoadFeedbackFileError = false;
		readrtn = feadback.load_with_retry(feedback_file);
		if (!readrtn)
		{
			bLoadFeedbackFileError = true;
		}

		if (bSpecialLog)
		{
			std::ostringstream oss;
			oss.clear();
			oss << __FUNCTION__ << " LINE " << __LINE__ << "  " << "to process complete .read feadback " << (feedback_file) << " " << readrtn
				<< " " << feadback.Status << " " << feadback.TaskRetVal << " " << feadback.Percent;
			
		}

		
		
		if (bLoadFeedbackFileError)
		{
			if (bSpecialLog)
			{
				std::ostringstream oss;
				oss.clear();
				oss << "load feedback failed." << (feedback_file) << " " << readrtn
					<< " " << feadback.Status << " " << iTaskRetVal << " " << feadback.Percent;
				LOGI(oss.str());
			}
		}
		

		
		
		
		
		if (iTaskRetVal > MOLDAI_SUCCESS ||
			!bLoadFeedbackFileError && (feadback.Status == jobsta_e::STATUS_CANCLE || feadback.Status == jobsta_e::STATUS_FAILURE))
		{
			
			std::ostringstream oss;
			oss.clear();
			oss << "======" << __FUNCTION__ << " LINE " << __LINE__ << " save feedback with cancelled status failed." << " bLoadFeedbackFileError  " << bLoadFeedbackFileError <<
				" feadback status " << feadback.Status << " task value " << iTaskRetVal;
			LOGI(oss.str());
			if ((iTaskRetVal == MOLDAI_USER_CANCEL) || !bLoadFeedbackFileError && (feadback.Status == jobsta_e::STATUS_CANCLE))
			{
				
				
				
				hasFinishedJobMap.insert(str2qstr(jobname), 3);

				if (!bLoadFeedbackFileError && (feadback.Status == jobsta_e::STATUS_CANCLE))
				{
				}
				else
				{
					feadback.Status = jobsta_e::STATUS_CANCLE;
					if (!feadback.save_with_retry(feedback_file))
					{
						if (bSpecialLog)
						{
							std::ostringstream oss;
							oss.clear();
							oss << __FUNCTION__ << " LINE " << __LINE__ << "  " << "save feedback with cancelled status failed.";
							LOGI(oss.str());
						}
					}
				}

				
				QFileInfo finfoRunning(str2qstr(jobdirstr));
				QFileInfo finfoCancelled(str2qstr(jobdirstr_cancelled));

				bool bCancelledExist = finfoCancelled.exists();
				bool bRunningExist = finfoRunning.exists();

				if (bCancelledExist)
				{
					if (bSpecialLog)
					{
						std::ostringstream oss;
						oss.clear();
						oss << __FUNCTION__ << " LINE " << __LINE__ << "  " << "jobfile already in cancelled dir.";
						LOGI(oss.str());
					}

					if (fpTaskLock != NULL)
					{
						fclose(fpTaskLock);
						fpTaskLock = NULL;
					}
					std::string msg = __LINE__ + " bCancelledExist";
					
					LOGI(msg);
					return -1;
				}

				if (!bRunningExist)
				{
					if (bSpecialLog)
					{
						std::ostringstream oss;
						oss.clear();
						oss << __FUNCTION__ << " LINE " << __LINE__ << "  " << "jobfile why not exists in running dir???";
						LOGI(oss.str());
					}

					if (fpTaskLock != NULL)
					{
						fclose(fpTaskLock);
						fpTaskLock = NULL;
					}
					std::string msg = __LINE__ + " !bRunningExist";
					std::cout << msg << std::endl;
					LOGI(msg);
					return -1;
				}

				
				
				
				jobinfo.tg.feedback = feadback;
				QString datatime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
				jobinfo.tg.tasksmap.at(taskid).Status = int(feadback.Status);
				jobinfo.tg.runinfo.runninginfo.EndTime = datatime.toStdString();

				
				bool bsave = jobinfo.save(jobdirstr_cancelled);
				if (!bsave)
				{
					if (bSpecialLog)
					{
						std::ostringstream oss;
						oss.clear();
						oss << __FUNCTION__ << " LINE " << __LINE__ << "  " << "failed to save jobfile into failed dir!!!";
						LOGI(oss.str());
					}
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(200));

				if (bSpecialLog)
				{
					LOGI("remove jobfile:" + jobdirstr);
				}

				QFile(str2qstr(jobdirstr)).remove();

				std::this_thread::sleep_for(std::chrono::milliseconds(100));

				QFileInfo finfoRunning2(str2qstr(jobdirstr));
				QFileInfo finfoCancelled2(str2qstr(jobdirstr_cancelled));

				
				bCancelledExist = finfoCancelled2.exists();
				bRunningExist = finfoRunning2.exists();

				if (!bCancelledExist || bRunningExist)
				{
					if (bSpecialLog)
					{
						
						std::ostringstream oss;
						oss.clear();
						oss << __FUNCTION__ << " LINE " << __LINE__ << "  " << "failed to save jobfile into failed dir!!!" << bCancelledExist << bRunningExist;
						LOGI(oss.str());
					}

					if (fpTaskLock != NULL)
					{
						fclose(fpTaskLock);
						fpTaskLock = NULL;
					}
					std::string msg = __LINE__ + " !bCancelledExist || bRunningExist";
					
					LOGI(msg);
					return -1;
				}

				if (fpTaskLock != NULL)
				{
					fclose(fpTaskLock);
					fpTaskLock = NULL;
				}
				std::string msg = __LINE__ + " more than 100000.";
				
				LOGI(msg);
				return -1;
			}
			else if ((iTaskRetVal > MOLDAI_SUCCESS) || !bLoadFeedbackFileError && (feadback.Status == jobsta_e::STATUS_FAILURE))
			{


				hasFinishedJobMap.insert(str2qstr(jobname), 4);
				
				std::ostringstream oss;
				oss.clear();
				oss << "======" << __FUNCTION__ << " LINE " << __LINE__ << "  ";
				LOGI(oss.str());
				
				if (!bLoadFeedbackFileError && (feadback.Status == jobsta_e::STATUS_FAILURE))
				{
					std::ostringstream oss;
					oss.clear();
					oss << "======" << __FUNCTION__ << " LINE " << __LINE__ << " !bLoadFeedbackFileError failure  ";
					LOGI(oss.str());
				}
				else
				{
					if (!exceptionMsg.isEmpty())
					{
						feadback.Msg = qstr2str(exceptionMsg);
						std::ostringstream oss;
						oss.clear();
						oss << "======" << __FUNCTION__ << " LINE " << __LINE__ << "  "<< feadback.Msg;
						LOGI(oss.str());
					}

					feadback.Status = jobsta_e::STATUS_FAILURE;
					
					std::ostringstream oss;
					oss.clear();
					oss << "====+++==" << __FUNCTION__ << " LINE " << __LINE__ << "  ";
					LOGI(oss.str());
					if (!feadback.save_with_retry(feedback_file))
					{
						if (bSpecialLog)
						{
							std::ostringstream oss;
							oss.clear();
							oss << __FUNCTION__ << " LINE " << __LINE__ << "  " << "save feedback with failed status failed.";
							LOGI(oss.str());
						}
					}
				}

				QFileInfo finfoRunning(str2qstr(jobdirstr));
				QFileInfo finfoFailed(str2qstr(jobdirstr_failed));

				bool bFailedExist = finfoFailed.exists();
				bool bRunningExist = finfoRunning.exists();

				if (bFailedExist)
				{
					if (bSpecialLog)
					{
						std::ostringstream oss;
						oss.clear();
						oss << __FUNCTION__ << " LINE " << __LINE__ << "  " << "jobfile already in failed dir.";
						LOGI(oss.str());
					}

					if (fpTaskLock != NULL)
					{
						fclose(fpTaskLock);
						fpTaskLock = NULL;
					}
					std::string msg = __LINE__ + " bFailedExist.";
					
					LOGI(msg);
					return -1;
				}

				if (!bRunningExist)
				{
					if (bSpecialLog)
					{
						std::ostringstream oss;
						oss.clear();
						oss << __FUNCTION__ << " LINE " << __LINE__ << "  " << "jobfile why not exists in running dir???";
						LOGI(oss.str());
					}

					if (fpTaskLock != NULL)
					{
						fclose(fpTaskLock);
						fpTaskLock = NULL;
					}
					std::string msg = __LINE__ + " !bRunningExist.";
					
					LOGI(msg);
					return -1;
				}


				jobinfo.tg.feedback = feadback;
				QString datatime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
				jobinfo.tg.tasksmap.at(taskid).Status = int(feadback.Status);
				jobinfo.tg.runinfo.runninginfo.EndTime = datatime.toStdString();

				
				bool bsave = jobinfo.save(jobdirstr_failed);
				if (!bsave)
				{
					if (bSpecialLog)
					{
						std::ostringstream oss;
						oss.clear();
						oss << __FUNCTION__ << " LINE " << __LINE__ << "  " << "failed to save jobfile into failed dir!!!";
						LOGI(oss.str());
					}
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(200));

				if (bSpecialLog)
				{
					LOGI("remove jobfile:" + jobdirstr);
				}

				QFile(str2qstr(jobdirstr)).remove();

				std::this_thread::sleep_for(std::chrono::milliseconds(100));

				
				QFileInfo finfoRunning2(str2qstr(jobdirstr));
				QFileInfo finfoFailed2(str2qstr(jobdirstr_failed));

				bFailedExist = finfoFailed2.exists();
				bRunningExist = finfoRunning2.exists();

				
				

				if (!bFailedExist || bRunningExist)
				{
					if (bSpecialLog)
					{
						std::ostringstream oss;
						oss.clear();
						oss << __FUNCTION__ << " LINE " << __LINE__ << "  " << "failed to save jobfile into failed dir!!!" << bFailedExist << bRunningExist;
						
						LOGI(oss.str());
					}

					if (fpTaskLock != NULL)
					{
						fclose(fpTaskLock);
						fpTaskLock = NULL;
					}
					std::string msg = __LINE__ + " !bFailedExist || bRunningExist.";
					
					LOGI(msg);
					return -1;
				}

				if (fpTaskLock != NULL)
				{
					fclose(fpTaskLock);
					fpTaskLock = NULL;
				}
				std::string msg = __LINE__ + " to check.";
				
				LOGI(msg);
				return -1;
			}
		}

		if (bLoadFeedbackFileError)
		{
			bLoadFeedbackFileError = false;
			
			feadback.Status = jobsta_e::STATUS_RUNNING;
		}

		if (!bLoadFeedbackFileError && feadback.Status == jobsta_e::STATUS_RUNNING)
		{
			if (bSpecialLog)
			{
				std::ostringstream oss;
				oss.clear();
				oss << __FUNCTION__ << " LINE " << __LINE__ << "  " << "to process complete 1";
				LOGI(oss.str());
			}

			
			
			{
				if (bSpecialLog)
				{
					std::ostringstream oss;
					oss.clear();
					oss << __FUNCTION__ << " LINE " << __LINE__ << "  " << "to process complete 2";
					
				}

				{
					if (type == ATLASTTASKTYPE)
					{
						if (bSpecialLog)
						{
							std::ostringstream oss;
							oss.clear();
							oss << __FUNCTION__ << " LINE " << __LINE__ << "  " << "to process complete error";
							LOGI(oss.str());
						}

						

						if (fpTaskLock != NULL)
						{
							fclose(fpTaskLock);
							fpTaskLock = NULL;
							LOGI("unlock taskfile lock:" + qstr2str(NewFileForRun));
						}
						std::string msg = __LINE__ + " complete progress.";
						
						LOGI(msg);
						return -1;
					}

					if (bSpecialLog)
					{
						std::ostringstream oss;
						oss.clear();
						oss << __FUNCTION__ << " LINE " << __LINE__ << "  " << "to process complete 3";
						LOGI(oss.str());
					}
					
					FILE* fp = AICORE::File::FopenDenyWriteLockUtf8(qstr2str(jobFilePathLock));

					if (fp != NULL)
					{
						

						if (type == ATSTARTTYPE) 
						{
							std::vector<std::string> taskList;
							std::string dirPath = qstr2str(getATBlockJobPath(str2qstr(fileName)));

							std::string dirnew = getATBlockJobPath(fileName);
							getTaskList(dirPath, taskList);

							int task_count = taskList.size();

							QString datatime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");

							jobinfo.tg.tasksmap.at(0).runinfo.EndTime = datatime.toStdString();
							jobinfo.tg.tasksmap.at(0).Status = int(job_status_e::STATUS_COMPLETE);
							jobinfo.tg.tasksmap.at(0).Percent = 100.0;

							// Rebuild mapping for current job
							maptaskfunction.clear();
							maptaskfunction[0] = StepAT_function.at(GenTasks);
							
							for (auto taskfile : taskList)
							{
								ATTaskInfo task;
								task.load(taskfile);

								Task_s newtask;
								if (task.task_.depends_.size() > 0)
									newtask.Depends.insert(task.task_.depends_.begin(), task.task_.depends_.end());
								newtask.FatherId = task.task_.fatherId_;
								newtask.Percent = 0;
								newtask.Status = int(job_status_e::STATUS_PENDDING);
								
								newtask.Msg = task.task_.msg_;
								newtask.Type = task.task_.type_;
								newtask.Id = task.task_.id_;
								if (bSpecialLog)
								{
									std::ostringstream oss;
									oss.clear();
									oss << __FUNCTION__ << " LINE " << __LINE__ << "  " << task.task_.id_ << " " << "to process complete 24";
									LOGI(oss.str());
								}
								newtask.ItemPath = jobinfo.tg.tasksmap.at(0).ItemPath;
								newtask.ProjectPath = jobinfo.tg.tasksmap.at(0).ProjectPath;

								newtask.ItemPath2 = jobinfo.tg.tasksmap.at(0).ItemPath2;
								newtask.ProjectPath2 = jobinfo.tg.tasksmap.at(0).ProjectPath2;

								LOGI("before add task:" + jobinfo.tg.tasksmap.size());
								jobinfo.tg.tasksmap[newtask.Id] = newtask;
								LOGI("after add task:" + jobinfo.tg.tasksmap.size());
								maptaskfunction[newtask.Id] = task.task_.fun_name_;
							}
							if (bSpecialLog)
							{
								std::ostringstream oss;
								oss.clear();
								oss << __FUNCTION__ << " LINE " << __LINE__ << "  " << (jobdirstr) << " " << jobinfo.tg.tasksmap.at(taskid).Status << " "
									<< jobinfo.tg.tasksmap.at(taskid).Percent << " " << datatime.toStdString() << " " << "to process complete 4";
								LOGI(oss.str());
							}
							
							jobinfo.save(jobdirstr);

							LOGI("save jobfile:" + jobdirstr);


							std::ostringstream strLine;
							strLine.clear();
							strLine << "print time sum now " << __FUNCTION__ << " LINE " << __LINE__;
							PrintTimeSum(strLine.str());
						
							init();
							
							fclose(fp);
						}
						else
						{
							if (type == RECONSTRUCTIONSTARTTYPE) 
							{

								feadback.Status = job_status_e::STATUS_COMPLETE;
								feadback.Percent = COMPLETE_PROGRESS;
								QString datatime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");

								jobinfo.tg.tasksmap.at(taskid).runinfo.EndTime = datatime.toStdString();
								jobinfo.tg.tasksmap.at(taskid).Status = int(job_status_e::STATUS_COMPLETE);
								jobinfo.tg.tasksmap.at(taskid).Percent = float(COMPLETE_PROGRESS);
								feadback.Msg = GetTaskEndingString(jobinfo.JobName);

								hasFinishedJobMap.insert(str2qstr(jobinfo.JobName), 2);

								jobinfo.tg.feedback = feadback;

								jobinfo.tg.runinfo.runninginfo.EndTime = datatime.toStdString();

								QString jobFileName = str2qstr(jobdirstr);
								QString lsRunningJobFileName = QFileInfo(jobFileName).fileName();
								QString lsCompletedJobFile = completedJobPath + pathSeperator + lsRunningJobFileName;;
								bool bsave = jobinfo.save(qstr2str(lsCompletedJobFile));

								if (!bsave)
								{
									if (bSpecialLog)
									{
										std::ostringstream oss;
										oss.clear();
										oss << __FUNCTION__ << " LINE " << __LINE__ << "  " << "failed to save jobfile into completed dir.";
										LOGI(oss.str());
									}
								}
								std::ostringstream strLine;
								strLine.clear();
								strLine << "print time sum now " << __FUNCTION__ << " LINE " << __LINE__;
								PrintTimeSum(strLine.str());
								ExportTimeSum(jobinfo);

								std::this_thread::sleep_for(std::chrono::milliseconds(500));

								if (bSpecialLog)
								{
									LOGI("remove " + qstr2str(jobFileName) + " for completion status.");
								}

								
								QFile(jobFileName).remove();

								
								if (!feadback.save_with_retry(feedback_file))
								{
									if (bSpecialLog)
									{
										std::ostringstream oss;
										oss.clear();
										oss << __FUNCTION__ << " LINE " << __LINE__ << "  " << "failed to save feedback with complete status.";
										LOGI(oss.str());
									}
								}

								LOGI("after remove jobfile:" + qstr2str(jobFileName) + " taskfile:" + qstr2str(NewFileForRun));

								init();
								fclose(fp);


								
							}
							else
							{
								jobinfo.tg.feedback = feadback;
								Run_s runinfo;

								QString datatime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");

								jobinfo.tg.tasksmap.at(taskid).runinfo.EndTime = datatime.toStdString();
								jobinfo.tg.tasksmap.at(taskid).Status = int(job_status_e::STATUS_COMPLETE);
								jobinfo.tg.tasksmap.at(taskid).Percent = float(COMPLETE_PROGRESS);

								if (bSpecialLog)
								{
									std::ostringstream oss;
									oss.clear();
									oss << __FUNCTION__ << " LINE " << __LINE__ << "  " << (jobdirstr) << " " << jobinfo.tg.tasksmap.at(taskid).Status << " "
										<< jobinfo.tg.tasksmap.at(taskid).Percent << " " << datatime.toStdString() << " " << "to process complete 4";
									LOGI(oss.str());
								}
								
								jobinfo.save(jobdirstr);

								LOGI("save jobfile:" + jobdirstr);



								std::ostringstream strLine;
								strLine.clear();
								strLine << "print time sum now " << __FUNCTION__ << " LINE " << __LINE__;
								PrintTimeSum(strLine.str());
								ExportTimeSum(jobinfo);
								init();
								
								fclose(fp);
							}



							
						}
						return AI3D_SUCCESS;
					}
					else
					{
						if (fpTaskLock != NULL)
						{
							fclose(fpTaskLock);
							fpTaskLock = NULL;
							LOGI("release taskfile lock:" + qstr2str(NewFileForRun));
						}
						std::string msg = __LINE__ + " fopen.";
						
						LOGI(msg);
						return -1;
					}


				}
				
			}
		}


		Sleep(100);
	}
	
	

	LOGI("jobfile taskfile:" + qstr2str(NewFileForRun));

	return -1;
}

std::string getATBlockJobPath(std::string& jobFilename)
{
	ATTaskInfo task;
	if (!task.load(jobFilename))
		return "";

	
	
	
	std::string projectPath = AI3D::CORE::File::GetParentDir(task.projectFile_);
	std::string blockItem = task.blockItem_;
	std::string jobName = task.job_;

	return AI3D::CORE::File::JoinPaths(projectPath+ "/"+ blockItem, jobName);
			
}


QString getATBlockJobPath(QString jobFilename)
{
	if (TASK_USE_BIN) {
		ATTaskInfo atTaskInfo;
		atTaskInfo.LoadBin(qstr2str(jobFilename));
		QString atJson = str2qstr(atTaskInfo.ATJson_);
		QString projectFile = str2qstr(atTaskInfo.projectFile_);
		
		QString blockPath;
		QString projectPath;
		QString blockItem = str2qstr(atTaskInfo.blockItem_);
		QString jobName = str2qstr(atTaskInfo.job_);

		int blockPos = atJson.lastIndexOf("/");
		if (blockPos < 0)
		{
			blockPos = atJson.lastIndexOf("\\");
		}

		if (blockPos > 0) {
			blockPath = atJson.left(blockPos);
		}

		int projectPos = projectFile.lastIndexOf("/");
		if (projectPos < 0)
		{
			projectPos = projectFile.lastIndexOf("\\");
		}

		if (projectPos > 0) {
			projectPath = projectFile.left(projectPos);
		}
		QString path = projectPath +  "/" + blockItem + "/" + jobName;
		return path;
	}
	else {
		QFile file(jobFilename);
		if (file.open(QIODevice::ReadOnly))
		{
			QByteArray ba = file.readAll();
			QJsonParseError e;
			QJsonDocument jsonDoc = QJsonDocument::fromJson(ba, &e);

			if (e.error == QJsonParseError::NoError && !jsonDoc.isNull())
			{


				QJsonObject jsonObj = jsonDoc.object();

				QString atJson = jsonObj.value("ATJson").toString();
				QString projectFile = jsonObj.value("projectPath").toString();
				
				QString blockPath;
				QString projectPath;
				QString blockItem = jsonObj.value("blockItem").toString();
				QString jobName = jsonObj.value("job").toString();

				int blockPos = atJson.lastIndexOf("/");
				if (blockPos < 0)
				{
					blockPos = atJson.lastIndexOf("\\");
				}

				if (blockPos > 0) {
					blockPath = atJson.left(blockPos);
				}

				int projectPos = projectFile.lastIndexOf("/");
				if (projectPos < 0)
				{
					projectPos = projectFile.lastIndexOf("\\");
				}

				if (projectPos > 0) {
					projectPath = projectFile.left(projectPos);
				}

				return projectPath +  "/" + blockItem + "/" + jobName;
			}

		}
		return "";
	}
	
}

QString get_local_ip()
{
	QHostInfo info = QHostInfo::fromName(QHostInfo::localHostName());
	foreach(QHostAddress address, info.addresses())
	{
		if (address.protocol() == QAbstractSocket::IPv4Protocol)
		{
			return address.toString();
		}
	}
	return "0.0.0.0";
}

bool bDistEngine = false;
QString progBaseName;

void PostQuitProcess()
{
	if (bQuitingApplication)
		return;

	bQuitingApplication = true;
	int iTryTimes = 0;
	bool bNeed2WaitAgain = false;

	while (bWorkingOnEngineFile)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		iTryTimes++;
		if (iTryTimes > 10)
		{
			if (bSpecialLog)
			{
				std::ostringstream oss;
				oss.clear();
				oss << __FUNCTION__ << " LINE " << __LINE__ << "PostQuitProcess/WaitTooLong.";
				LOGI(oss.str());
			}

			bNeed2WaitAgain = true;
			break;
		}
	}

	if (bSpecialLog)
	{
		std::ostringstream oss;
		oss.clear();
		oss << __FUNCTION__ << " LINE " << __LINE__ << "PostQuitProcess";
		LOGI(oss.str());
	}

	EngineInfo::Getinstance().GetAPPUseInfosMutual().rbegin()->QuitTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss").toStdString();
	if (STAT_USE_BIN) {
		EngineInfo::Getinstance().ExportEngineInfoBin();
	}
	else {
		EngineInfo::Getinstance().ExportEngineInfoJson();
	}


	std::string postFix = "";
	if (ENGINE_USE_BIN) {
		postFix = BINFILE_POSTFIX;
	}
	else {
		postFix = JSONFILE_POSTFIX;
	}
	engineinfofile = qstr2str(ENGINEJOBPATH) + "/Engines/" + qstr2str(QHostInfo::localHostName()) + postFix;

	std::cout << engineinfofile << std::endl;
	
	try
	{
		if (std::filesystem::exists(AICORE::File::BoostPathFromUtf8(engineinfofile)))
		{
			std::string file = engineinfofile;

			engineinfofile = "";
			if (bWorkingOnEngineFile || bNeed2WaitAgain)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}

			std::filesystem::remove(AICORE::File::BoostPathFromUtf8(file));
		}
	}
	catch (const std::filesystem::filesystem_error& fse)
	{
		std::ostringstream oss;
		oss << "fse error:" << fse.code() << " " << fse.what() << " "
			<< AICORE::File::BoostPathToUtf8String(fse.path1()) << " "
			<< AICORE::File::BoostPathToUtf8String(fse.path2());
		LOGI(oss.str());
	}
	catch (std::exception& ex)
	{
		std::ostringstream oss;
		oss << "exception:" << ex.what();
		LOGI(oss.str());
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(300));

	if (gotNewPendingJobFile || !NewFileForRun.isEmpty())
	{
		ATTaskInfo attask;

		attask.load(qstr2str(NewFileForRun));
		std::string postFix = "";
		if (JOB_INFO_USE_BIN) {
			postFix = BINFILE_POSTFIX;
		}
		else {
			postFix = JSONFILE_POSTFIX;
		}
		std::string job_file = qstr2str(Settings::getEngineJobQueue()) + "/Running/" + attask.job_ + postFix;
		if (killTaskProcess())
		{
			if (bSpecialLog)
			{
				
				LOGI("killed job task:" + job_file + " successfully.");
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		else
		{
			if (bSpecialLog)
			{
				std::cout << __FILE__ << __LINE__ << "job task has already disappeared:" << job_file << "!" << std::endl;
				LOGI("job task has already disappeared:" + job_file);
			}
		}

		JobFullInfo_s jobinfo(job_file);



		std::string jobFilePathLock = job_file + ".lock";
		FILE* fp = AICORE::File::FopenDenyWriteLockUtf8(jobFilePathLock);

		if (fp != NULL)
		{
			LOGI("HandlerRoutine4");
			if (jobinfo.tg.tasksmap.count(0))
			{
				jobinfo.tg.tasksmap.at(0).Status = int(jobsta_e::STATUS_PENDDING);
			}
			jobinfo.tg.feedback.Msg = GetTaskStartingString(jobinfo.JobName2);
			jobinfo.tg.feedback.Status = jobsta_e::STATUS_PENDDING;
			jobinfo.tg.feedback.Percent = 0.;
			
			std::ostringstream strLine;
			strLine.clear();
			strLine << "print time sum now " << __FUNCTION__ << " LINE " << __LINE__;
			PrintTimeSum(strLine.str());
			ExportTimeSum(jobinfo);
			fclose(fp);

			
			LOGI("save job's time sum info::" + job_file);
			std::string postFix = "";
			if (JOB_INFO_USE_BIN) {
				postFix = BINFILE_POSTFIX;
			}
			else {
				postFix = JSONFILE_POSTFIX;
			}
			std::string jobnew_file = qstr2str(Settings::getEngineJobQueue()) + "/Pending/Normal/" + attask.job_ + postFix;
			
			bool bsave = jobinfo.save(jobnew_file);
			if (std::filesystem::exists(AICORE::File::BoostPathFromUtf8(job_file)))
			{
				AI3D::CORE::File::RemoveFile(job_file);
			}
			JobFeedBack_s feadback;
			std::string feedname = AI3D::CORE::File::GetParentDir(attask.projectFile_) + "/" + attask.blockItem_ + "/";
			std::string feedbackfile = "";
			if (JOB_FEEDBACK_USE_BIN) {
				feedbackfile = MAKE_FEEDBAK_BIN_FILE(feedname, attask.job_);
			}
			else {
				feedbackfile = MAKE_FEEDBAK_JSON_FILE(feedname, attask.job_);
				
			}

			if (std::filesystem::exists(AICORE::File::BoostPathFromUtf8(feedbackfile)))
			{
				feadback.load(feedbackfile);
				feadback = jobinfo.tg.feedback;
				FILE* fp = AICORE::File::FopenDenyWriteLockUtf8(feedbackfile + ".lock");
				if (fp != NULL)
				{
					feadback.save(feedbackfile);
					fclose(fp);
				}
			}
		}
	}
}

#ifdef WIN32


BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
	if (bSpecialLog)
	{
		std::ostringstream oss;
		oss.clear();
		oss << __FUNCTION__ << " LINE " << __LINE__ << "HandlerRoute" << dwCtrlType;
		LOGI(oss.str());
	}

	if (CTRL_CLOSE_EVENT == dwCtrlType || CTRL_C_EVENT == dwCtrlType || CTRL_BREAK_EVENT == dwCtrlType \
		|| CTRL_LOGOFF_EVENT == dwCtrlType || CTRL_SHUTDOWN_EVENT == dwCtrlType)
	{
		
		bQuitingApplication = true;

		if (bSpecialLog)
		{
			std::ostringstream oss;
			oss.clear();
			oss << __FUNCTION__ << " LINE " << __LINE__ << "HandlerRoute2" << dwCtrlType;
			LOGI(oss.str());
		}

		EngineInfo::Getinstance().GetAPPUseInfosMutual().rbegin()->QuitTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss").toStdString();
		if (STAT_USE_BIN) {
			EngineInfo::Getinstance().ExportEngineInfoBin();
		}
		else {
			EngineInfo::Getinstance().ExportEngineInfoJson();
		}
		
		
		
		std::string postFix = "";
		if (JOB_INFO_USE_BIN) {
			postFix = BINFILE_POSTFIX;
		}
		else {
			postFix = JSONFILE_POSTFIX;
		}
		engineinfofile = qstr2str(ENGINEJOBPATH) + "/Engines/" + qstr2str(QHostInfo::localHostName()) + postFix;

		std::cout << engineinfofile << std::endl;
		

		try
		{
			if (std::filesystem::exists(AICORE::File::BoostPathFromUtf8(engineinfofile)))
			{
				std::string file = engineinfofile;

				engineinfofile = "";
				std::filesystem::remove(AICORE::File::BoostPathFromUtf8(file));
			}
		}
		catch (const std::filesystem::filesystem_error& fse)
		{
			std::ostringstream oss;
			oss << "fse error:" << fse.code() << " " << fse.what() << " "
				<< AICORE::File::BoostPathToUtf8String(fse.path1()) << " "
				<< AICORE::File::BoostPathToUtf8String(fse.path2());
			LOGI(oss.str());
		}
		catch (std::exception& ex)
		{
			std::ostringstream oss;
			oss << "exception:" << ex.what();
			LOGI(oss.str());
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(300));

		if (gotNewPendingJobFile || !NewFileForRun.isEmpty())
		{
			ATTaskInfo attask;

			attask.load(qstr2str(NewFileForRun));
			std::string postFix = "";
			if (JOB_INFO_USE_BIN) {
				postFix = BINFILE_POSTFIX;
			}
			else {
				postFix = JSONFILE_POSTFIX;
			}
			std::string job_file = qstr2str(Settings::getEngineJobQueue()) + "/Running/" + attask.job_ + postFix;

			
			if (killTaskProcess())
			{
				if (bSpecialLog)
				{
					
					LOGI("killed job task:" + job_file + " successfully.");
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
			else
			{
				if (bSpecialLog)
				{
					
					LOGI("job task has already disappeared:" + job_file);
				}
			}

			JobFullInfo_s jobinfo(job_file);


			QString jobFilePathLock = str2qstr(job_file) + ".lock";
					FILE* fp = AICORE::File::FopenDenyWriteLockUtf8(qstr2str(jobFilePathLock));

			if (fp != NULL)
			{
				


				std::ostringstream strLine;
				strLine.clear();
				strLine << "print time sum now " << __FUNCTION__ << " LINE " << __LINE__;
				PrintTimeSum(strLine.str());
				ExportTimeSum(jobinfo);
				fclose(fp);

				
				LOGI("save job's time sum info::" + job_file);
				std::string postFix = "";
				if (JOB_INFO_USE_BIN) {
					postFix = BINFILE_POSTFIX;
				}
				else {
					postFix = JSONFILE_POSTFIX;
				}
				std::string jobnew_file = qstr2str(Settings::getEngineJobQueue()) + "/Pending/" + attask.job_ + postFix;
				bool bsave = jobinfo.save(jobnew_file);
				if (std::filesystem::exists(AICORE::File::BoostPathFromUtf8(job_file)))
				{
					AI3D::CORE::File::RemoveFile(job_file);
				}
				JobFeedBack_s feadback;
				std::string feedname = AI3D::CORE::File::GetParentDir(attask.projectFile_) + "/" + attask.blockItem_ + "/";
				std::string feedbackfile = "";
				if (JOB_FEEDBACK_USE_BIN) {
					feedbackfile = MAKE_FEEDBAK_BIN_FILE(feedname, attask.job_);
				}
				else {
					feedbackfile = MAKE_FEEDBAK_JSON_FILE(feedname, attask.job_);
					
				}

				if (std::filesystem::exists(AICORE::File::BoostPathFromUtf8(feedbackfile)))
				{
					feadback.load(feedbackfile);
					feadback.Status = jobsta_e::STATUS_PENDDING;
					FILE* fp = AICORE::File::FopenDenyWriteLockUtf8(feedbackfile + ".lock");
					if (fp != NULL)
					{
						feadback.save(feedbackfile);
						fclose(fp);
					}
				}

			}
		}
		else
		{
			LOGI("HandlerRoutine5");
		}

#if 0
		if (!NewFileForRun.isEmpty())
		{
			JobFeedBack_s feadback;
			ATTaskInfo attask;
			attask.load(NewFileForRun.toStdString());
			std::string job_file = Settings::getEngineJobQueue().toStdString() + "/Running/" + attask.job_ + ".json";

			std::string feedname = AI3D::CORE::File::GetParentDir(attask.projectFile_) + "/" + attask.blockItem_ + "/";
			std::string feedbackfile = MAKE_FEEDBAK_FILE(feedname, attask.job_);;
			
			if (std::filesystem::exists(AICORE::File::BoostPathFromUtf8(feedbackfile)))
			{
				feadback.load(feedbackfile);
				feadback.Status = jobsta_e::STATUS_CANCLE;
				FILE* fp = AICORE::File::FopenDenyWriteLockUtf8(feedbackfile + ".lock");
				if (fp != NULL)
				{
					feadback.save(feedbackfile);
					fclose(fp);
				}
				
				ProcessUnnormaldRunningJob();
			}
		}
#endif
	}
	return true;
}
#endif 

void customized_exit()
{
	
	LOGI("exit main to execute");

	PostQuitProcess();

}

int minSfmMemory = -1; 

std::map<std::string, int> jobSfmMemoryMap;


int TestSfmMemory(std::string taskJsonFile)
{
	int MB = 1024 * 1024;
	int GB = 1024 * 1024 * 1024;
	int HalfGB = 512 * 1024 * 1024;

	MEMORYSTATUSEX memoryStatusEx;
	memoryStatusEx.dwLength = sizeof(memoryStatusEx);

	GlobalMemoryStatusEx(&memoryStatusEx);

	int allMemoryMB = memoryStatusEx.ullTotalPhys / MB;
	int freeMemoryMB = memoryStatusEx.ullAvailPhys / MB;

	int allMemory = (memoryStatusEx.ullTotalPhys  + HalfGB) / GB;
	int freeMemory = (memoryStatusEx.ullAvailPhys + HalfGB) / GB;

	
	enginePhysicalMemory = allMemory;




	return 0;
}

void MakePath();
void MakePath()
{
	if (!JobMonitor::CreateJobQueueDir(ENGINEJOBPATH))
	{
		JobMonitor::CreateLocalJobQueueDir();
		JobMonitor::CreateJobQueueDir(ENGINEJOBPATH);
	}

	pathSeperator = "/";
	pendingJobPath = Settings::getEngineJobQueue() + pathSeperator + JOBPENDINGSTR + pathSeperator ;
	runningJobPath = Settings::getEngineJobQueue() + pathSeperator + JOBRUNNINGSTR + pathSeperator;
	completedJobPath = Settings::getEngineJobQueue() + pathSeperator + JOBCOMPLETEDSTR + pathSeperator;
	failedJobPath = Settings::getEngineJobQueue() + pathSeperator + JOBFAILEDSTR + pathSeperator;
	cancelledJobPath = Settings::getEngineJobQueue() + pathSeperator + JOBCANCELLEDSTR + pathSeperator;

	QString genEnginePath = Settings::getGenEngineJobQueue();
	if (!JobMonitor::CreateGenJobQueueDir(genEnginePath))
	{
		JobMonitor::CreateLocalGenJobQueueDir();
		JobMonitor::CreateGenJobQueueDir(genEnginePath);
	}

	genPendingJobPath = genEnginePath + pathSeperator + JOBPENDINGSTR + pathSeperator;
	genRunningJobPath = genEnginePath + pathSeperator + JOBRUNNINGSTR + pathSeperator;
	genCompletedJobPath = genEnginePath + pathSeperator + JOBCOMPLETEDSTR + pathSeperator;
	genFailedJobPath = genEnginePath + pathSeperator + JOBFAILEDSTR + pathSeperator;
	genCancelledJobPath = genEnginePath + pathSeperator + JOBCANCELLEDSTR + pathSeperator;
}



void execEngineTimeThread()
{
	while (true)
	{
		if (engineinfofile.empty())
			break;

		if (bQuitingApplication)
			break;

		bWorkingOnEngineFile = true;

		EngineInfo_s runinfo;
		runinfo.HostName = qstr2str(QHostInfo::localHostName());
		runinfo.UserName = getenv("USERNAME");
		runinfo.StartTime = enginestarttime.toStdString();
		runinfo.Status = 0;
		
		runinfo.Version = RunGetEngineVersion();
		runinfo.IPAddr = get_local_ip().toStdString();
		runinfo.EndTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss").toStdString();
		runinfo.ProcessId = qApp->applicationPid();

		runinfo.TaskFile = qstr2str(NewFileForRun);


		
		if ((gotNewPendingJobFile ) && projectfilefullpath != "")
		{
			runinfo.Status = 1;

			runinfo.ProjectName = projectfilefullpath;
			runinfo.ProjectName2 = runinfo.ProjectName;

		}

 
		if (!engineinfofile.empty() && !bQuitingApplication)
		{
			if (ENGINE_USE_BIN) {
				runinfo.savebin(engineinfofile);
			}
			else {
				runinfo.save(engineinfofile);
			}
			
		}

		bWorkingOnEngineFile = false;
		
		Sleep(1000);
	}
}

struct TaskTimeInfo
{
	QString sLastActiveTime;
	qint64 iTaskPid;
	QString sTaskFile;
};

bool checkTaskInstanceStatus(QString& sTmpNewFileForRun)
{
	qint64 iTaskPid = qApp->applicationPid();

	if (bQuitingApplication)
		return false;

	if (sTmpNewFileForRun.isEmpty())
		return false;

	if (!QFileInfo::exists(sTmpNewFileForRun))
		return false;

	QString sTaskPidFile = sTmpNewFileForRun + ".pid";

	if (!QFileInfo::exists(sTaskPidFile))
		return true;

	

	bool bSuccessfullyLoadedFromTaskTimeFile = false;

	TaskTimeInfo _taskTimeInfo;

	
	std::string file = qstr2str(sTaskPidFile);
	std::ifstream in = AICORE::File::OpenIfstreamUtf8(file, std::ios::binary);
	if (!in.is_open())
	{
		LOGE("Load pid file failed!");
		return false;
	}

	PIDFile pidFile;
	pidFile.Deserialize(in);
	_taskTimeInfo.iTaskPid = pidFile.pid;
	std::string taskFile = pidFile.taskFile;
#ifdef WIN32
	// taskFile = UTF82GBK(taskFile);
#endif 
	_taskTimeInfo.sTaskFile = QString::fromStdString(taskFile);
	std::string lastActivateTime = pidFile.lastActivateTime;
	_taskTimeInfo.sLastActiveTime = QString::fromStdString(lastActivateTime);

	bSuccessfullyLoadedFromTaskTimeFile = true;
	bool returnRes = false;
	if (bSuccessfullyLoadedFromTaskTimeFile)
	{
		
		QDateTime dtLastActiveTime = QDateTime::fromString(_taskTimeInfo.sLastActiveTime, "yyyyMMddhhmmss");
		QDateTime dtNow = QDateTime::currentDateTime();
		int diffSecs = dtLastActiveTime.secsTo(dtNow);

		if (iTaskPid == _taskTimeInfo.iTaskPid)
		{
			
			if (diffSecs > 3)
				returnRes = true;
			else
				returnRes =  false;
		}
		else
		{
			
			if (diffSecs > 10)
				returnRes = true;
			else
				returnRes = false;
		}
	}
	in.close();
	return returnRes;

}


bool continueCurrentTaskByEngineInfo(QString& sTmpNewFileForRun)
{
	return false;
}

bool continueCurrentTask(QString &sTmpNewFileForRun)
{
	if (bQuitingApplication)
		return false;

	if (sTmpNewFileForRun.isEmpty())
		return false;

	if (!QFileInfo::exists(sTmpNewFileForRun))
		return false;

	QString sTaskPidFile = sTmpNewFileForRun + ".pid";
	if (!QFileInfo::exists(sTaskPidFile))
		return true;

	bool bSuccessfullyLoadedFromTaskTimeFile = false;

	TaskTimeInfo _taskTimeInfo;

	
	std::string file = qstr2str(sTaskPidFile);
	std::ifstream in = AICORE::File::OpenIfstreamUtf8(file, std::ios::binary);
	if (!in.is_open())
	{
		LOGE("Load pid file failed!");
		return false;
	}

	PIDFile pidFile;
	pidFile.Deserialize(in);
	_taskTimeInfo.iTaskPid = pidFile.pid;
	std::string taskFile = pidFile.taskFile;
#ifdef WIN32
	// taskFile = UTF82GBK(taskFile);
#endif 
	_taskTimeInfo.sTaskFile = QString::fromStdString(taskFile);
	std::string lastActivateTime = pidFile.lastActivateTime;
	_taskTimeInfo.sLastActiveTime = QString::fromStdString(lastActivateTime);

	bSuccessfullyLoadedFromTaskTimeFile = true;
	bool returnRes = false;
	if (bSuccessfullyLoadedFromTaskTimeFile)
	{
		
		QDateTime dtLastActiveTime = QDateTime::fromString(_taskTimeInfo.sLastActiveTime, "yyyyMMddhhmmss");
		QDateTime dtNow = QDateTime::currentDateTime();

		int diffSecs = dtLastActiveTime.secsTo(dtNow);
		if (diffSecs > 5) 
			returnRes = true;
		else
			returnRes = false;
	}
	else
	{
		returnRes = true;
	}
	in.close();
	return returnRes;

}

void execTaskTimeThread()
{
	while (true)
	{
		bool bSuccessfullyLoadedFromTaskTimeFile = false;
		TaskTimeInfo taskTimeInfo;

		if (bQuitingApplication)
			break;

		if (taskPid == -1 || taskPidFile.isEmpty() || NewFileForRun.isEmpty() || !gotNewPendingJobFile)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		if (!QFileInfo::exists(NewFileForRun))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		if (QFileInfo::exists(taskPidFile))
		{
			TaskTimeInfo _taskTimeInfo;

			std::string file = qstr2str(taskPidFile);
			std::ifstream in = AICORE::File::OpenIfstreamUtf8(file, std::ios::binary);
			if (!in.is_open())
			{
				LOGE("Load pid file failed!");
				return ;
			}

			PIDFile pidFile;
			pidFile.Deserialize(in);
			_taskTimeInfo.iTaskPid = pidFile.pid;
			std::string taskFile = pidFile.taskFile;
#ifdef WIN32
			// taskFile = UTF82GBK(taskFile);
#endif 
			_taskTimeInfo.sTaskFile = QString::fromStdString(taskFile);
			std::string lastActivateTime = pidFile.lastActivateTime;
			_taskTimeInfo.sLastActiveTime = QString::fromStdString(lastActivateTime);
			in.close();
			bSuccessfullyLoadedFromTaskTimeFile = true;
			if (bSuccessfullyLoadedFromTaskTimeFile)
			{
				if (_taskTimeInfo.iTaskPid == taskPid)
				{
					
					taskTimeInfo.iTaskPid = taskPid;
					taskTimeInfo.sLastActiveTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
					taskTimeInfo.sTaskFile = NewFileForRun;
				}
				else
				{
					
					QDateTime dtLastActiveTime = QDateTime::fromString(_taskTimeInfo.sLastActiveTime, "yyyyMMddhhmmss");
					QDateTime dtNow = QDateTime::currentDateTime();

					int diffSecs = dtLastActiveTime.secsTo(dtNow);
					if (diffSecs > 5) 
					{
						
						taskTimeInfo.iTaskPid = taskPid;
						taskTimeInfo.sTaskFile = NewFileForRun;

						taskTimeInfo.sLastActiveTime = dtNow.toString("yyyyMMddhhmmss");
						
					}
					else
					{
						
						std::this_thread::sleep_for(std::chrono::milliseconds(1000));
						continue;
					}
				}
			}
			
		}
		
		if (!bSuccessfullyLoadedFromTaskTimeFile)
		{
			
			taskTimeInfo.iTaskPid = taskPid;
			taskTimeInfo.sTaskFile = NewFileForRun;
			
			taskTimeInfo.sLastActiveTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
		}
		else
		{
			
		}
	

		std::string file = qstr2str(taskPidFile);
		std::ofstream out = AICORE::File::OpenOfstreamUtf8(file, std::ios::binary);
		if (!out.is_open()) {
			LOGE("Writing PID file failed!");
			return;
		}
		PIDFile pidFile;
		pidFile.pid = taskTimeInfo.iTaskPid;
		pidFile.lastActivateTime = qstr2str(taskTimeInfo.sLastActiveTime);
		pidFile.taskFile = qstr2str2(taskTimeInfo.sTaskFile);
		pidFile.Serialize(out);
		out.close();

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

qint64 nTest1Num = 0;
qint64 nTest2Num = 0;

void execTestThread()
{
	while (true)
	{
		if (bQuitingApplication)
		{
			
			LOGI("nTest1Num:" + std::to_string(nTest1Num) + " quiting... ");
			break;
		}

		
		LOGI("nTest1Num:" + std::to_string(nTest1Num));

		nTest1Num++;

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}

void execTest2Thread()
{
	while (true)
	{
		if (bQuitingApplication)
		{
			
			LOGI("nTest2Num:" + std::to_string(nTest2Num) + " quiting... ");
			break;
		}

		
		LOGI("nTest2Num:" + std::to_string(nTest2Num));

		nTest2Num++;

		std::this_thread::sleep_for(std::chrono::milliseconds(2000));

		if (nTest2Num == 5)
		{
			int a = 30;
			int b = 0;
	

		}
	}
}


QString testTaskJsonLog = "D:/worksp/jobs/Logs";
QString testTaskJsonFile = "D:/worksp/projs/NewProject11221/Block_2/job_20230316095227_AT/task_def_10.json";

void writeCurrentLockInfo(bool bLock)
{
	qint64 qid = QApplication::applicationPid();
	QString qname = QApplication::applicationName();
	QString nowTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss.zzz");
	QString hostName = QHostInfo::localHostName();

	std::ostringstream oss;
	oss << "host:" << hostName.toStdString() << " qname:" << qname.toStdString() << " qid:" << qid << " now:" << nowTime.toStdString() << (bLock ? " lock." : " unlock.");
	std::cout << oss.str() << std::endl;
	LOGI(oss.str());
}

void execTest6Thread()
{
	while (true)
	{


		FILE* fpLock = NULL;

		fpLock = AICORE::File::FopenDenyWriteLockUtf8(qstr2str(testTaskJsonFile + ".lock"));

		if (fpLock == NULL)
		{
			
			continue;
		}

		writeCurrentLockInfo(true);

		std::this_thread::sleep_for(std::chrono::milliseconds(6000));

		fclose(fpLock);

		writeCurrentLockInfo(false);
	}
}

bool checkProcessRunning(const QString& processName, QList<quint64>& listProcessId)
{
#ifdef Q_OS_WIN
	bool res = false;
	HANDLE    hToolHelp32Snapshot;
	hToolHelp32Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32    pe = { sizeof(PROCESSENTRY32) };
	BOOL  isSuccess = Process32First(hToolHelp32Snapshot, &pe);
	while (isSuccess)
	{
		size_t len = WideCharToMultiByte(CP_ACP, 0, pe.szExeFile, wcslen(pe.szExeFile), NULL, 0, NULL, NULL);
		char* des = (char*)malloc(sizeof(char) * (len + 1));
		WideCharToMultiByte(CP_ACP, 0, pe.szExeFile, wcslen(pe.szExeFile), des, len, NULL, NULL);
		des[len] = '\0';

		if (!strcmp(des, qstr2str(const_cast<QString &>(processName)).c_str()))
		{
			listProcessId.append(pe.th32ProcessID);
			res = true;
			free(des);
			break;
		}
		free(des);
		isSuccess = Process32Next(hToolHelp32Snapshot, &pe);
	}
	CloseHandle(hToolHelp32Snapshot);
	return res;
#elif defined Q_OS_MAC
	bool res(false);
	QString strCommand = "ps -ef|grep " + processName + " |grep -v grep |awk '{print $2}'";

	const char* strFind_ComName = convertQString2char(strCommand);
	FILE* pPipe = popen(strFind_ComName, "r");
	if (pPipe)
	{
		std::string com;
		char name[512] = { 0 };
		while (fgets(name, sizeof(name), pPipe) != NULL)
		{
			int nLen = strlen(name);
			if (nLen > 0
				&& name[nLen - 1] == '\n'))
			{
				name[nLen - 1] = '\0';
				listProcessId.append(atoi(name));
				res = true;
				break;
			}
		}
		pclose(pPipe);
	}
	return res;
#endif
}


int main(int argc, char** argv)
{
	
	

	
	
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

#ifdef WIN32
	SetUnhandledExceptionFilter(ExceptionFilter);
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);
#endif 
	

	QApplication app(argc, argv);

	
	
	InitializeLogEngine(argc > 0 ? argv : nullptr);

	
	std::string version = RunGetEngineVersion();
	std::string startMsg = "USE ENGINE VESION : " + version;
	LogConsole(startMsg);

	enginestarttime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
	QFileInfo finfoProgFile(argv[0]);
	std::string workPath = GetWorkPath();
	std::string m_enginePath = workPath + "/Engine";

	std::string apppath = qstr2str(finfoProgFile.absolutePath());

	std::string machinecode = EngineInfo::Getinstance().GetMachineCode();
	std::string postFix = "";
	if (STAT_USE_BIN) {
		postFix = BINFILE_POSTFIX;
	}
	else {
		postFix = JSONFILE_POSTFIX;
	}
	
	std::string enginejsonpath = workPath + "/" + machinecode + STAT_ENGINE_POSTFIX + postFix;
	EngineInfo::Getinstance().GetEngineJsonPathMutual() = enginejsonpath;

	try
	{
		if (std::filesystem::is_regular_file(AICORE::File::BoostPathFromUtf8(enginejsonpath)))
		{
			if (STAT_USE_BIN) {
				EngineInfo::Getinstance().LoadEngineInfoBin();
			}
			else {
				EngineInfo::Getinstance().LoadEngineInfoJson();
			}
			
		}
	}
	catch (const std::filesystem::filesystem_error& fse)
	{
		std::ostringstream oss;
		oss << "fse error:" << fse.code() << " " << fse.what() << " "
			<< AICORE::File::BoostPathToUtf8String(fse.path1()) << " "
			<< AICORE::File::BoostPathToUtf8String(fse.path2());
		LOGI(oss.str());
	}
	catch (std::exception& ex)
	{
		std::ostringstream oss;
		oss << "exception:" << ex.what();
		LOGI(oss.str());
	}


	APPUseInfo appuseinfo;
	appuseinfo.StartTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss").toStdString();

	std::string versionCode;
	std::string configpath = apppath + "/" + "MoldAIConfig.ini";
	std::string language;
	EngineInfo::Getinstance().ParseConfig(configpath, versionName, versionCode, language);
	appuseinfo.VersionCode = versionCode;
	appuseinfo.VersionName = versionName;
	appuseinfo.language = language;
	EngineInfo::Getinstance().GetAPPUseInfosMutual().push_back(appuseinfo);


	if (argc > 1)
	{
		bSpecialLog = argv[1];
	}
	
	
	progBaseName = finfoProgFile.baseName();


	
	
	
	TestSfmMemory("");
	
	{



	}

	if (!progBaseName.compare("MoldAINode", Qt::CaseInsensitive))
	{
		int exenum = catchProcess.NumProgramRunning("MoldAINode.exe");
		if (bSpecialLog)
		{
			
		}

		QList<quint64> listProcessId;
		
		
		
		
		
		
		
		if (exenum>1)
		{
			std::cout << " Fatal error: Another Engine instance is running" << std::endl;
			return -1;
		}
		

	}

	
	

	app.setQuitOnLastWindowClosed(false);

	::atexit(customized_exit);
	signal(SIGINT, SigInt_Handler);
	signal(SIGBREAK, SigBreak_Handler);
	signal(SIGTERM, SigBreak_Handler);
	signal(SIGABRT, SigBreak_Handler);

	


	
	QString timenow = (QDateTime::currentDateTime()).toString("yyyy-MM-dd hh:mm:ss.zzz");
	
	std::string msg = "Starting Engine on job queue :" + qstr2str(Settings::getEngineJobQueue());
	LogConsole(msg);
	if (bSpecialLog)
	{
		std::ostringstream oss;
		oss.clear();
		oss << __FUNCTION__ << " LINE " << __LINE__ << "[" << timenow.toStdString() << "] Starting Engine on job queue " << qstr2str(Settings::getEngineJobQueue());
		LOGI(oss.str());	
	}
	
	MakePath();

	std::thread genTaskThread(GenTaskThread::Run);
	genTaskThread.detach();

	std::thread searchUnnormalGenRunningJob(GenTaskThread::SearchUnnormalRunningJob);
	searchUnnormalGenRunningJob.detach();
	
	DoCleanupJobLockOnceWhileEngineStart();

	msgBoxThread = new MsgBoxThread(nullptr);
	std::string enginepostFix = "";
	if (JOB_INFO_USE_BIN) {
		enginepostFix = BINFILE_POSTFIX;
	}
	else {
		enginepostFix = JSONFILE_POSTFIX;
	}
	engineinfofile = qstr2str(ENGINEJOBPATH) + "/Engines/" + qstr2str(QHostInfo::localHostName()) + enginepostFix;

	try
	{
		if (std::filesystem::exists(AICORE::File::BoostPathFromUtf8(engineinfofile)))
		{
			

			QFile(str2qstr(engineinfofile)).remove();

		}
	}
	catch (const std::filesystem::filesystem_error& fse)
	{
		std::ostringstream oss;
		oss << "fse error:" << fse.code() << " " << fse.what() << " "
			<< AICORE::File::BoostPathToUtf8String(fse.path1()) << " "
			<< AICORE::File::BoostPathToUtf8String(fse.path2());
		LOGI(oss.str());
	}
	catch (std::exception& ex)
	{
		std::ostringstream oss;
		oss << "exception:" << ex.what();
		LOGI(oss.str());
	}

	std::thread execEngineTimejob(execEngineTimeThread);
	execEngineTimejob.detach();

	
	std::thread searchPendingJob1(searchPendingJobThread2);
	searchPendingJob1.detach();

	std::thread searchCancelledRunningJob(searchUnnormaldRunningJobThread);
	searchCancelledRunningJob.detach();

	
	

	std::thread execTaskTime(execTaskTimeThread);
	execTaskTime.detach();

	std::thread execDoCleanup(DoCleanupLockFiles);
	execDoCleanup.detach();


	return app.exec();
}

struct JobFileTimeInfo
{
	QString fullPathFile;  
	QString jobFileName;  
	int ymd;
	int hms;
};

QMap<QString, JobFileTimeInfo> jobFileTimeInfoMap;
QMap<QString, JobFileTimeInfo> jobFileTimeInfoRunningMap;




