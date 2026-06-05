#include "Util/JobMonitor.h"
#include "Util/Settings.h"
#include "Core/File.h"
#include <boost/algorithm/string.hpp>
#include <filesystem>
#include <QDir>

#include "Core/Types.h"
#include <QApplication>






bool JobMonitor::SplitJobFilename(const QString& jobBaseFile, JobFileTimeInfo& jobinfo)
{
	QStringList parts = jobBaseFile.split('_');
	if (parts.size() < 3)
		return false;

	QString ymd_hms = parts.at(1);
	if (ymd_hms.isEmpty() || ymd_hms.length() != 14)
		return false;

	QString ymdStr = ymd_hms.left(8);
	QString hmsStr = ymd_hms.right(6);

	bool bOk = false;

	auto ymd = ymdStr.toInt(&bOk);
	if (!bOk)
		return false;

	auto hms = hmsStr.toInt(&bOk);
	if (!bOk)
		return false;

	jobinfo.hms = hms;
	jobinfo.ymd = ymd;
	QString type = parts.at(2);
	if (parts.at(2).contains("."))
	{
		QStringList partparts = type.split('.');
		type = partparts.at(0);
		
	}
	
	
	if (type.isEmpty() )
		return false;

	

	std::string filename = qstr2str(jobBaseFile);

	filename = AI3D::CORE::File::GetFileNameWithoutExtension(filename);
	jobinfo.type =GetJobType((filename));
	
	if (jobinfo.type == jobtype_e::JOB_TILE && parts.size()>6)
	{
		QString indexstr = parts.at(3);
		jobinfo.index =std::atoi( qstr2str(indexstr).c_str());
		QString qprjname = (parts.at(4)); 
		std::string prjname = qstr2str(qprjname);
		jobinfo.prjname = prjname;
		QString qrptid = (parts.at(5)); 
		std::string rptid = qstr2str(qrptid);
		jobinfo.rptid = rptid;

	}

	return true;
}

bool JobMonitor::CreateJobQueueDir(QString path)
{
	QFileInfo file(path);
	QDir dir(path);
	if (file.exists())
	{
		QFileInfoList folder_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
		QString lsPendingJobPath, lsRunningJobPath, lsCancelledJobPath, lsFailedJobPath, lsCompletedJobPath, lsPathSeperator;

		int errorCode;
		if(!CheckJobQueuePath(path, lsPendingJobPath, lsRunningJobPath, lsCancelledJobPath, lsFailedJobPath, lsCompletedJobPath, lsPathSeperator, errorCode))
		
		{
			
			dir.mkdir(JOBCANCELLEDSTR);
			dir.mkdir(JOBCOMPLETEDSTR);
			dir.mkdir(JOBENGINESSTR);
			dir.mkdir(JOBFAILEDSTR);
			dir.mkdir(JOBPENDINGSTR);
			dir.mkdir(JOBRUNNINGSTR);
		}
		
		
		QDir dir(path + "/"+JOBPENDINGSTR);
		folder_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
		if (folder_list.size() < 3)
		{
			dir.mkdir(HIGHLEVEL);
			dir.mkdir(LOWLEVEL);
			dir.mkdir(NORMALLEVEL);
		}

		if (!CheckJobQueuePath(path, lsPendingJobPath, lsRunningJobPath, lsCancelledJobPath, lsFailedJobPath, lsCompletedJobPath, lsPathSeperator, errorCode))
			return false;

		return true;
	}
	else
	{
		
		
	}

	return false;
}

bool JobMonitor::CreateGenJobQueueDir(QString path)
{
	QFileInfo file(path);
	QDir dir(path);
	if (file.exists())
	{
		QFileInfoList folder_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
		QString lsPendingJobPath, lsRunningJobPath, lsCancelledJobPath, lsFailedJobPath, lsCompletedJobPath, lsPathSeperator;

		int errorCode;
		if(!CheckJobQueuePath(path, lsPendingJobPath, lsRunningJobPath, lsCancelledJobPath, lsFailedJobPath, lsCompletedJobPath, lsPathSeperator, errorCode))
		{

			dir.mkdir(JOBCANCELLEDSTR);
			dir.mkdir(JOBCOMPLETEDSTR);
			dir.mkdir(JOBFAILEDSTR);
			dir.mkdir(JOBPENDINGSTR);
			dir.mkdir(JOBRUNNINGSTR);
		}

		if (!CheckJobQueuePath(path, lsPendingJobPath, lsRunningJobPath, lsCancelledJobPath, lsFailedJobPath, lsCompletedJobPath, lsPathSeperator, errorCode))
			return false;

		return true;
	}
	else
	{


	}

	return false;
}

bool JobMonitor::CreateDirs()
{
	if (!JobMonitor::CreateJobQueueDir(Settings::getMasterJobQueue()))
	{
		LOGE("Invalid job path provided! Default job path will be used.");
		JobMonitor::CreateLocalJobQueueDir();
		if (!JobMonitor::CreateJobQueueDir(Settings::getMasterJobQueue()))
		{
			LOGE("Invalid job path provided! Default job path also invalid!");
			return false;
		}


	}
	return true;
}

bool JobMonitor::CreateLocalJobQueueDir()
{
	QString path = QCoreApplication::applicationDirPath();
	path.append("/jobs");
	
	QSettings* pSettings = new QSettings("HKEY_CURRENT_USER\\Software\\MoldAI\\JobQueues", QSettings::NativeFormat);
	
	pSettings->setValue("master", path);
	pSettings->setValue("engine", path);

	QDir dir;
	dir.mkpath(path);

	return true;
}

bool JobMonitor::CreateLocalGenJobQueueDir()
{
	QString path = QCoreApplication::applicationDirPath();
	path.append("/jobs_gen");
	QDir dir;
	bool ok = dir.mkpath(path);

	return ok;
}


bool JobMonitor::GetJobListsInfo(std::map<std::string, EngineInfo_s>& info, std::map<job_status_e, std::vector<std::string>>& joblistsmap)
{

	std::string jobpath_ = qstr2str(Settings::getMasterJobQueue());

	std::string enginesjobpath = jobpath_ + "/" + JOBENGINESSTR + "/";

	try
	{
		const std::filesystem::path enginesRoot = AI3D::CORE::File::BoostPathFromUtf8(enginesjobpath);
		if (!std::filesystem::exists(enginesRoot))
		{
			return false;
		}
		for (const auto& entry : std::filesystem::directory_iterator(enginesRoot))
		{
			if (!entry.is_regular_file())
				continue;
			std::string filepath = AI3D::CORE::File::BoostPathToUtf8String(entry.path());
			if (AI3D::CORE::File::BoostPathToUtf8String(entry.path().extension()) != JSONFILE_POSTFIX)
				continue;

			EngineInfo_s engineinfo;
			if (ENGINE_USE_BIN) {
				if (!engineinfo.loadbin(filepath))
				{
					continue;
				}
			}
			else {
				if (!engineinfo.load(filepath))
				{
					continue;
				}
			}
			
			info[engineinfo.HostName] = engineinfo;
		}

		
		std::string failedjobpath = jobpath_ + "/" + JOBFAILEDSTR + "/";

		

		const std::filesystem::path failedRoot = AI3D::CORE::File::BoostPathFromUtf8(failedjobpath);
		if (std::filesystem::exists(failedRoot))
		{
			for (const auto& entry : std::filesystem::directory_iterator(failedRoot))
			{
				if (!entry.is_regular_file())
					continue;
				std::string filepath = AI3D::CORE::File::BoostPathToUtf8String(entry.path().filename());
				if (strlen(filepath.c_str()) == JOBNAMELENGTH_AT && (filepath.find(JOB_PREFIX) != std::string::npos) && (filepath.find(SC_POSTFIX) != std::string::npos))
				{
					joblistsmap[STATUS_FAILURE].push_back(AI3D::CORE::File::BoostPathToUtf8String(entry.path()));
				}
			}
		}
		std::string runningingjobpath = jobpath_ + "/" + JOBRUNNINGSTR + "/";
		const std::filesystem::path runningRoot = AI3D::CORE::File::BoostPathFromUtf8(runningingjobpath);
		if (std::filesystem::exists(runningRoot))
		{
			for (const auto& entry : std::filesystem::directory_iterator(runningRoot))
			{
				if (!entry.is_regular_file())
					continue;
				std::string filepath = AI3D::CORE::File::BoostPathToUtf8String(entry.path().filename());
				if (strlen(filepath.c_str()) == JOBNAMELENGTH_AT && (filepath.find(JOB_PREFIX) != std::string::npos) && (filepath.find(SC_POSTFIX) != std::string::npos))
				{
					joblistsmap[STATUS_RUNNING].push_back(AI3D::CORE::File::BoostPathToUtf8String(entry.path()));
				}
			}
		}
		std::string pendingjobpath = jobpath_ + "/" + JOBPENDINGSTR + "/";
		const std::filesystem::path pendingRoot = AI3D::CORE::File::BoostPathFromUtf8(pendingjobpath);
		if (std::filesystem::exists(pendingRoot))
		{
			for (const auto& entry : std::filesystem::recursive_directory_iterator(pendingRoot))
			{
				if (!entry.is_regular_file())
					continue;
				std::string filepath = AI3D::CORE::File::BoostPathToUtf8String(entry.path().filename());
				if (strlen(filepath.c_str()) == JOBNAMELENGTH_AT && (filepath.find(JOB_PREFIX) != std::string::npos) && (filepath.find(SC_POSTFIX) != std::string::npos))
				{
					joblistsmap[STATUS_PENDDING].push_back(AI3D::CORE::File::BoostPathToUtf8String(entry.path()));
				}
			}
		}
	}
	catch (const std::filesystem::filesystem_error& fse)
	{
		std::ostringstream oss;
		oss << "filesystem error:" << fse.code() << " " << fse.what() << " " << fse.path1().string() << " " << fse.path2().string();
		LOGI(oss.str());
		return false;
	}
	catch (std::exception& ex)
	{
		std::ostringstream oss;
		oss << "exception:" << ex.what();
		LOGI(oss.str());
		return false;
	}

	return true;
}

std::vector<std::string> JobMonitor::SortJobFile(const std::string& JobPath)
{
	std::map<std::string,JobFileTimeInfo> jobsmap;
    
	std::vector<std::string> finaljobfilepaths, finaltilejobfilepaths;
	std::vector<std::string> filepaths; 
	std::vector<std::string> filepathlist;
	std::vector<std::string> subtaskfilepathlist;
	std::vector<std::string> singletaskfilepathlist;


	try
	{
		
		const std::filesystem::path jobRoot = AI3D::CORE::File::BoostPathFromUtf8(JobPath);
		for (const auto& entry : std::filesystem::directory_iterator(jobRoot))
		{
			if (!entry.is_regular_file())
				continue;
			std::string filepath = AI3D::CORE::File::BoostPathToUtf8String(entry.path().filename());
			std::string postFix = "";
			if (JOB_INFO_USE_BIN) {
				postFix = BINFILE_POSTFIX;
			}
			else {
				postFix = JSONFILE_POSTFIX;
			}
			if ((filepath.find(JOB_PREFIX) != std::string::npos) && (filepath.find(postFix) != std::string::npos)
				&& (filepath.find(".lock") == std::string::npos))
			{
				filepaths.push_back(AI3D::CORE::File::BoostPathToUtf8String(entry.path()));
			}
		}
	}
	catch (const std::filesystem::filesystem_error& fse)
	{
		std::ostringstream oss;
		oss << "filesystem error:" << fse.code() << " " << fse.what() << " " << fse.path1().string() << " " << fse.path2().string();
		LOGI(oss.str());
	}
	catch (std::exception& ex)
	{
		std::ostringstream oss;
		oss << "exception:" << ex.what();
		LOGI(oss.str());
	}

	
	std::map<std::string, std::vector<std::string>> prjname_jobfile;
	for (auto& file : filepaths)
	{
		if (file.empty())
			continue;
		JobFileTimeInfo jobFileTimeInfo;
		std::string name;
		
		try
		{
			name = AI3D::CORE::File::BoostPathToUtf8String(AI3D::CORE::File::BoostPathFromUtf8(file).filename());
		}
		catch (const std::filesystem::filesystem_error& fse)
		{
			std::ostringstream oss;
			oss << "filesystem error:" << fse.code() << " " << fse.what() << " " << fse.path1().string() << " " << fse.path2().string();
			LOGI(oss.str());
			continue;
		}
		catch (std::exception& ex)
		{
			std::ostringstream oss;
			oss << "exception:" << ex.what();
			LOGI(oss.str());
			continue;
		}

		int ymd = 0;
		int hms = 0;

		if (SplitJobFilename(str2qstr(name), jobFileTimeInfo))
		{
			
				if (jobFileTimeInfo.ymd > 0 && jobFileTimeInfo.hms > 0)
				{
					jobFileTimeInfo.fullPathFile = file;
					jobFileTimeInfo.jobFileName = name;

					jobsmap[name] = jobFileTimeInfo;
					filepathlist.push_back(file);
					if (jobFileTimeInfo.type == jobtype_e::JOB_AT)
					{
						subtaskfilepathlist.push_back(file);
					}
					else if (jobFileTimeInfo.type == jobtype_e::JOB_TILE && 
						jobFileTimeInfo.prjname!="" && jobFileTimeInfo.rptid!="")
					{

						
						std::string rptid = jobFileTimeInfo.prjname + jobFileTimeInfo.rptid;
						prjname_jobfile[rptid].push_back( file);

					}
				}
			
			
		}
	}
	
	do {
		int num = subtaskfilepathlist.size();
		if (num <= 0)
			break;

		std::string file = subtaskfilepathlist.at(0);
		std::string jobFileName;
		
		try
		{
			jobFileName = AI3D::CORE::File::BoostPathToUtf8String(AI3D::CORE::File::BoostPathFromUtf8(file).filename());
		}
		catch (const std::filesystem::filesystem_error& fse)
		{
			std::ostringstream oss;
			oss << "filesystem error:" << fse.code() << " " << fse.what() << " " << fse.path1().string() << " " << fse.path2().string();
			LOGI(oss.str());
			continue;
		}
		catch (std::exception& ex)
		{
			std::ostringstream oss;
			oss << "exception:" << ex.what();
			LOGI(oss.str());
			continue;
		}

		JobFileTimeInfo jobFileTimeInfo;

		if (jobsmap.count(jobFileName))
		{
			jobFileTimeInfo = jobsmap.at(jobFileName);
		}
		else
		{
			continue;
		}


		int minIndex = 0;
		int minYmd = jobFileTimeInfo.ymd;
		int minHms = jobFileTimeInfo.hms;
		int minJobindex = -1;
		for (int i = 1; i < num; i++)
		{
			std::string currFileFullPath = subtaskfilepathlist.at(i);
			std::string currJobFileName;
			
			try
			{
				currJobFileName = AI3D::CORE::File::BoostPathToUtf8String(AI3D::CORE::File::BoostPathFromUtf8(currFileFullPath).filename());
			}
			catch (const std::filesystem::filesystem_error& fse)
			{
				std::ostringstream oss;
				oss << "filesystem error:" << fse.code() << " " << fse.what() << " " << fse.path1().string() << " " << fse.path2().string();
				LOGI(oss.str());
				continue;
			}
			catch (std::exception& ex)
			{
				std::ostringstream oss;
				oss << "exception:" << ex.what();
				LOGI(oss.str());
				continue;
			}

			JobFileTimeInfo currJobFileTimeInfo;


			currJobFileTimeInfo = jobsmap.at(currJobFileName);
			

			
			int currIndex = i;
			int currYmd = currJobFileTimeInfo.ymd;
			int currHms = currJobFileTimeInfo.hms;

			if (currYmd < minYmd || currYmd == minYmd && currHms < minHms)
			{
				minIndex = currIndex;
				minYmd = currYmd;
				minHms = currHms;

				
			}
			
		}

		finaljobfilepaths.push_back(subtaskfilepathlist.at(minIndex));
		subtaskfilepathlist.erase(subtaskfilepathlist.begin() +minIndex);

		
	} while (subtaskfilepathlist.size() > 0);


	if (prjname_jobfile.empty())
	{

	}
	else
	{
		for (auto iter : prjname_jobfile)
		{
			if (!iter.second.empty())
			{
				singletaskfilepathlist.push_back(iter.second.front());
			}
		}
	}

	do {
		int num = singletaskfilepathlist.size();
		if (num <= 0)
			break;

		std::string file = singletaskfilepathlist.at(0);
		std::string jobFileName;

		try
		{
			jobFileName = AI3D::CORE::File::BoostPathToUtf8String(AI3D::CORE::File::BoostPathFromUtf8(file).filename());
		}
		catch (const std::filesystem::filesystem_error& fse)
		{
			std::ostringstream oss;
			oss << "filesystem error:" << fse.code() << " " << fse.what() << " " << fse.path1().string() << " " << fse.path2().string();
			LOGI(oss.str());
			continue;
		}
		catch (std::exception& ex)
		{
			std::ostringstream oss;
			oss << "exception:" << ex.what();
			LOGI(oss.str());
			continue;
		}

		JobFileTimeInfo jobFileTimeInfo;

		if (jobsmap.count(jobFileName))
		{
			jobFileTimeInfo = jobsmap.at(jobFileName);
		}
		else
		{
			continue;
		}


		int minIndex = 0;
		int minYmd = jobFileTimeInfo.ymd;
		int minHms = jobFileTimeInfo.hms;
		int minJobindex = -1;
		for (int i = 1; i < num; i++)
		{
			std::string currFileFullPath = singletaskfilepathlist.at(i);
			std::string currJobFileName;

			try
			{
				currJobFileName = AI3D::CORE::File::BoostPathToUtf8String(AI3D::CORE::File::BoostPathFromUtf8(currFileFullPath).filename());
			}
			catch (const std::filesystem::filesystem_error& fse)
			{
				std::ostringstream oss;
				oss << "filesystem error:" << fse.code() << " " << fse.what() << " " << fse.path1().string() << " " << fse.path2().string();
				LOGI(oss.str());
				continue;
			}
			catch (std::exception& ex)
			{
				std::ostringstream oss;
				oss << "exception:" << ex.what();
				LOGI(oss.str());
				continue;
			}

			JobFileTimeInfo currJobFileTimeInfo;


			currJobFileTimeInfo = jobsmap.at(currJobFileName);
			


			int currIndex = i;
			int currYmd = currJobFileTimeInfo.ymd;
			int currHms = currJobFileTimeInfo.hms;
			
			
			if (currYmd < minYmd || currYmd == minYmd && currHms < minHms)
			{
				minIndex = currIndex;
				minYmd = currYmd;
				minHms = currHms;


			}
			
		}

		finaltilejobfilepaths.push_back(singletaskfilepathlist.at(minIndex));
		singletaskfilepathlist.erase(singletaskfilepathlist.begin() + minIndex);

		
	} while ( singletaskfilepathlist.size() > 0);

	
	std::vector<std::vector<std::string>> rptfiles;
	for (auto iter : finaltilejobfilepaths)
	{
		std::vector<std::string> tempepathlist, finaltilejobfilepathstemp;

		
		
		auto name = AI3D::CORE::File::BoostPathToUtf8String(AI3D::CORE::File::BoostPathFromUtf8(iter).filename());
		if (!jobsmap.count(name))
		{
			continue;
		}
		
		auto jobtemp = jobsmap.at(name);
		auto rpttemp = jobtemp.prjname + jobtemp.rptid;
		tempepathlist.assign(prjname_jobfile.at(rpttemp).begin(), prjname_jobfile.at(rpttemp).end());
		do {
			int num = tempepathlist.size();
			if (num <= 0)
				break;

			std::string file = tempepathlist.at(0);
			std::string jobFileName;

			try
			{
				jobFileName = AI3D::CORE::File::BoostPathToUtf8String(AI3D::CORE::File::BoostPathFromUtf8(file).filename());
			}
			catch (const std::filesystem::filesystem_error& fse)
			{
				std::ostringstream oss;
				oss << "filesystem error:" << fse.code() << " " << fse.what() << " " << fse.path1().string() << " " << fse.path2().string();
				LOGI(oss.str());
				continue;
			}
			catch (std::exception& ex)
			{
				std::ostringstream oss;
				oss << "exception:" << ex.what();
				LOGI(oss.str());
				continue;
			}

			JobFileTimeInfo jobFileTimeInfo;

			if (jobsmap.count(jobFileName))
			{
				jobFileTimeInfo = jobsmap.at(jobFileName);
			}
			else
			{
				continue;
			}

			int minIndex = 0;
			int minYmd = jobFileTimeInfo.ymd;
			int minHms = jobFileTimeInfo.hms;
			int minJobindex = jobFileTimeInfo.index;
			for (int i = 1; i < num; i++)
			{
				std::string currFileFullPath = tempepathlist.at(i);
				std::string currJobFileName;

				try
				{
					currJobFileName = AI3D::CORE::File::BoostPathToUtf8String(AI3D::CORE::File::BoostPathFromUtf8(currFileFullPath).filename());
				}
				catch (const std::filesystem::filesystem_error& fse)
				{
					std::ostringstream oss;
					oss << "filesystem error:" << fse.code() << " " << fse.what() << " " << fse.path1().string() << " " << fse.path2().string();
					LOGI(oss.str());
					continue;
				}
				catch (std::exception& ex)
				{
					std::ostringstream oss;
					oss << "exception:" << ex.what();
					LOGI(oss.str());
					continue;
				}

				JobFileTimeInfo currJobFileTimeInfo;


				currJobFileTimeInfo = jobsmap.at(currJobFileName);
				


				int currIndex = i;
				int currYmd = currJobFileTimeInfo.ymd;
				int currHms = currJobFileTimeInfo.hms;
				int curJobIndex = currJobFileTimeInfo.index;
				if (minJobindex == -1 && curJobIndex >= 0)
				{
					minJobindex = curJobIndex;
				}
				

				
						if (curJobIndex >=0 &&curJobIndex < minJobindex)
						{
							minIndex = currIndex;
							minJobindex = curJobIndex;
						}
					

				
				
			}

			finaltilejobfilepathstemp.push_back(tempepathlist.at(minIndex));
			tempepathlist.erase(tempepathlist.begin() + minIndex);

			
		} while (tempepathlist.size() > 0);
		rptfiles.push_back(finaltilejobfilepathstemp);
	}

	for (auto iter : rptfiles)
	{
		finaljobfilepaths.insert(finaljobfilepaths.end(), iter.begin(),iter.end());
	}
	

	return finaljobfilepaths;
}

std::vector<std::string> JobMonitor::SortPendingJobFile(const std::string& JobPath)
{

	std::string path = JobPath;
	  path = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(path)));

	std::vector<std::string > dirs = AI3D::CORE::File::GetDirList(path);
	if (dirs.empty())
	{
		return std::vector<std::string>();
	}
	std::vector<std::string> jobdirs;

	for (auto iter : proritydir_str)
	{
		AI3D::CORE::String::StringToLower(&iter.second);
		for (auto iterdir : dirs)
		{
			iterdir = AI3D::CORE::File::GetDirName(iterdir);
			AI3D::CORE::String::StringToLower(&iterdir);

			if (iterdir == iter.second)
			{
				std::string jobdir = AI3D::CORE::File::JoinPaths(path, iterdir);
				jobdir = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(std::string(jobdir)));

				jobdirs.push_back(jobdir);
			}
		}
	}
	

	std::vector<std::string> joblist;
	for (int i = 0; i < jobdirs.size(); i++)
	{
		std::vector<std::string> list;
		list  = (SortJobFile(jobdirs[i]));
		joblist.insert(joblist.end(), list.begin(), list.end());
	}
	return joblist;
		
}



bool JobMonitor::CheckJobQueuePath(QString & lsMasterJobQueue, QString & lsPendingJobPath, QString & lsRunningJobPath, QString & lsCancelledJobPath, QString & lsFailedJobPath, QString & lsCompletedJobPath, QString & lsPathSeperator, int& errorCode)
{
	if (lsMasterJobQueue.isEmpty())
	{
		errorCode = -1;
		return false;
	}

	{
		QDir dir(lsMasterJobQueue);
		if (!dir.exists())
		{
			errorCode = -2;
			return false;
		}
	}

	if (lsPathSeperator.isEmpty())
		lsPathSeperator = "/";

	{
		lsPendingJobPath = lsMasterJobQueue + lsPathSeperator + JOBPENDINGSTR;
		QDir dir(lsPendingJobPath);
		if (!dir.exists())
		{
			lsPathSeperator = "\\";
			lsPendingJobPath = lsMasterJobQueue + lsPathSeperator + JOBPENDINGSTR;

			QDir dir(lsPendingJobPath);
			if (!dir.exists(lsPendingJobPath))
			{
				errorCode = -3;
				return false;
			}
		}
	}

	{
		lsRunningJobPath = lsMasterJobQueue + lsPathSeperator + JOBRUNNINGSTR;
		QDir dir(lsRunningJobPath);
		if (!dir.exists(lsRunningJobPath))
		{
			errorCode = -4;
			return false;
		}
	}

	{
		lsCancelledJobPath = lsMasterJobQueue + lsPathSeperator + JOBCANCELLEDSTR;
		QDir dir(lsCancelledJobPath);
		if (!dir.exists(lsCancelledJobPath))
		{
			errorCode = -5;
			return false;
		}
	}

	{
		lsFailedJobPath = lsMasterJobQueue + lsPathSeperator + JOBFAILEDSTR;
		QDir dir(lsFailedJobPath);
		if (!dir.exists(lsFailedJobPath))
		{
			errorCode = -6;
			return false;
		}
	}

	{
		lsCompletedJobPath = lsMasterJobQueue + lsPathSeperator + JOBCOMPLETEDSTR;
		QDir dir(lsCompletedJobPath);
		if (!dir.exists(lsCompletedJobPath))
		{
			errorCode = -7;
			return false;
		}
	}

	errorCode = 0;
	return true;
}
