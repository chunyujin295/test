#pragma once
#include <QVector>
#include <QDateTime>
#include <iostream>

#include <fstream>  
#include <streambuf>
#include "Settings.h"
#include "Core/Types.h"
#include "Core/json.h"
#include "Core/String.h"
#include <set>
#include "Util/TaskProcess.h"






class JobMonitor
{
public:
	JobMonitor() {};
	static bool CreateJobQueueDir(QString path);
	static bool CreateGenJobQueueDir(const QString& path);
	static bool CreateLocalJobQueueDir();
	static bool CreateLocalGenJobQueueDir();
	static bool CreateDirs();
	struct JobFileTimeInfo
	{
		std::string fullPathFile;  
		std::string jobFileName;  
		int ymd;
		int hms;
		jobtype_e type;
		int index = -1;
		std::string prjname = "";
		std::string rptid = "";
	};
	bool static SplitJobFilename(const QString& jobBaseFile, JobFileTimeInfo& jobinfo);

	static std::vector<std::string> SortPendingJobFile(const std::string& JobPath);
	static std::vector<std::string> SortJobFile(const  std::string& JobPath);

	
	static bool GetJobListsInfo(std::map<std::string, EngineInfo_s>& enginlistsinfo,std::map<job_status_e,std::vector<std::string>>& joblistsmap );
	static bool CheckJobQueuePath(QString& lsMasterJobQueue, QString& lsPendingJobPath,
		QString& lsRunningJobPath, QString& lsCancelledJobPath, QString& lsFailedJobPath, QString& lsCompletedJobPath, QString& lsPathSeperator, int& errorCode);
private:

	

};
