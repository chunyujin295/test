#include "Util/TaskProcess.h"
#include "Util/Settings.h"

#include <boost/algorithm/string.hpp>
#include <filesystem>
#include <QDir>

#include "Core/Types.h"
#include "Core/File.h"
#include "Util/JobMonitor.h"


void doCancelJobs(std::vector<std::pair<std::string, std::string> > jobs_to_delete)
{
#ifdef USE_OPENMP
#pragma omp parallel  for
#endif
	for (int idx = 0; idx < jobs_to_delete.size(); idx++)
	{
		int errornum;
		auto feedbackpath = jobs_to_delete[idx].first;
		auto jobname = jobs_to_delete[idx].second;
		bool flag = doCancelJob2(feedbackpath, (jobname), errornum);

		if (!flag)
		{
#ifdef USE_OPENMP
#pragma omp critical
#endif
			LOGW("Please wait for cancel!");
		}
	}
}

bool doCancelJob2(const std::string& tocancelPath, const std::string& jobName, int& errorCode)
{
	QString lsMasterJobQueue = Settings::getMasterJobQueue();

	QString lsPendingJobPath;
	QString lsRunningJobPath;
	QString lsCancelledJobPath;
	QString lsFailedJobPath;
	QString lsCompletedJobPath;
	QString lsPathSeperator = "/";

	{
		std::ostringstream oss;
		oss << " Cancel Job.";
		LOGI(oss.str());
	}

	if (jobName.empty() || tocancelPath.empty())
	{
		errorCode = -1;
		{
			std::ostringstream oss;
			oss << " Cancel Job: Job not found.";
			LOGI(oss.str());
		}

		return false;  
	}

	if (!JobMonitor::CheckJobQueuePath(lsMasterJobQueue, lsPendingJobPath, lsRunningJobPath, lsCancelledJobPath, lsFailedJobPath, lsCompletedJobPath, lsPathSeperator, errorCode))
	{
		errorCode += (-10);
		{
			std::ostringstream oss;
			oss << " Cancel Job: Job queue not found.";
			LOGI(oss.str());
		}

		return false;
	}
	QString postFix = "";
	if (JOB_INFO_USE_BIN) {
		postFix = BINFILE_POSTFIX;
	}
	else {
		postFix = JSONFILE_POSTFIX;
	}
	QString lsRunningJobFile = lsRunningJobPath + lsPathSeperator + str2qstr(jobName) + postFix;
	QString lsCancelledJobFile = lsCancelledJobPath + lsPathSeperator + str2qstr(jobName) + postFix;
	QString lsFailedJobFile = lsFailedJobPath + lsPathSeperator + str2qstr(jobName) + postFix;
	QString lsCompletedJobFile = lsCompletedJobPath + lsPathSeperator + str2qstr(jobName) + postFix;

	
	
	
	QString lsPendingJobFile,highjobfile,normaljobfile,lowjobfile;
	 highjobfile = lsPendingJobPath + lsPathSeperator + HIGHLEVEL + lsPathSeperator +str2qstr(jobName) + postFix;
	 normaljobfile = lsPendingJobPath + lsPathSeperator + NORMALLEVEL + lsPathSeperator + str2qstr(jobName) + postFix;
	 lowjobfile = lsPendingJobPath + lsPathSeperator + LOWLEVEL + lsPathSeperator + str2qstr(jobName) + postFix;
	if (QFile::exists(highjobfile))
	{
		lsPendingJobFile = highjobfile;
	}
	else if (QFile::exists(normaljobfile))
	{
		lsPendingJobFile = normaljobfile;
	}
	else if (QFile::exists(lowjobfile))
	{
		lsPendingJobFile = lowjobfile;
	}
	else
	{
		lsPendingJobFile = "";
	}
	
	int retryTimes = 0;
	
	if (lsPendingJobFile != "" && QFile::exists(lsPendingJobFile))
	{

		{
			std::ostringstream oss;
			oss << " Cancel Job: get pending Job.";
			LOGI(oss.str());
		}


			{
				JobFeedBack_s feedback;
				
				std::string feedbackfile = "";
				if (JOB_FEEDBACK_USE_BIN) {
					feedbackfile = MAKE_FEEDBAK_BIN_FILE(tocancelPath, jobName);
				}
				else {
					feedbackfile = MAKE_FEEDBAK_JSON_FILE(tocancelPath, jobName);
				}

				if (!feedback.load_with_retry(feedbackfile))
				{
					feedback.Percent = 0.0;
					feedback.TaskRetVal = 0;
				}

				feedback.Status = jobsta_e::STATUS_CANCLE;
				feedback.Msg = "";
				feedback.save_with_retry(feedbackfile);

				JobFullInfo_s jobinfo(qstr2str(lsPendingJobFile));


				jobinfo.tg.feedback = feedback;			

				QString datatime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
				
				
				jobinfo.tg.runinfo.runninginfo.EndTime = datatime.toStdString();


				if (!jobinfo.save(qstr2str(lsCancelledJobFile)))
				{
					LOGE("Move pending job file to cancelled Failed!");
				}
				else
				{
					
					std::this_thread::sleep_for(std::chrono::milliseconds(200));

					QFile lockFile(lsPendingJobFile);
					lockFile.remove();

					
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				
					
				}


			} 

	}
	else if(QFile::exists(lsRunningJobFile))
	{
		
		
		JobFeedBack_s feedback;
		
		std::string feedbackfile = "";
		if (JOB_FEEDBACK_USE_BIN) {
			feedbackfile = MAKE_FEEDBAK_BIN_FILE(tocancelPath, jobName);
		}
		else {
			feedbackfile = MAKE_FEEDBAK_JSON_FILE(tocancelPath, jobName);
		}


			{

				if (!feedback.load_with_retry(feedbackfile))
				{
					feedback.Percent = 0.0;
					feedback.TaskRetVal = 0;
					feedback.Msg = "";
				}

				feedback.Status = jobsta_e::STATUS_CANCLE;
				feedback.save_with_retry(feedbackfile);


				JobFullInfo_s jobinfo(qstr2str(lsRunningJobFile));


				int taskid = jobinfo.tg.GetLastRunningTaskId();
				if (taskid == -1)
					taskid = jobinfo.tg.GetFirstPendingTaskId();

				jobinfo.tg.feedback = feedback;
				QString datatime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
				if(taskid >= 0)
					jobinfo.tg.tasksmap.at(taskid).Status = int(feedback.Status);
				jobinfo.tg.runinfo.runninginfo.EndTime = datatime.toStdString();

				

				if (!jobinfo.save(qstr2str(lsCancelledJobFile)))
				{
					LOGE("Move running job file to cancelled job queue Failed!");
				}
				else
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(200));

					QFile lockFile(lsRunningJobFile);
					lockFile.remove();

					std::this_thread::sleep_for(std::chrono::milliseconds(100));

					
				}

				
			}

	}
	else
	{
		LOGI("Cancel Failed !");
		return false;
	}

	errorCode = 0;
	return true;
}

bool CheckJobQueuePath4OneJob(const std::string& jobName, int& jobStatus)
{
	QString lsMasterJobQueue = Settings::getMasterJobQueue();

	QString lsPendingJobPath;
	QString lsRunningJobPath;
	QString lsCancelledJobPath;
	QString lsFailedJobPath;
	QString lsCompletedJobPath;
	QString lsPathSeperator = "/";
	int errorCode = 0;
	
	jobStatus = -1;
	if (jobName.empty())
	{
		return false;
	}

	if (!JobMonitor::CheckJobQueuePath(lsMasterJobQueue, lsPendingJobPath, lsRunningJobPath, lsCancelledJobPath, lsFailedJobPath, lsCompletedJobPath, lsPathSeperator, errorCode))
	{
		return false;
	}
	QString postFix = "";
	if (JOB_INFO_USE_BIN) {
		postFix = BINFILE_POSTFIX;
	}
	else {
		postFix = JSONFILE_POSTFIX;
	}
	QString lsRunningJobFile = lsRunningJobPath + lsPathSeperator + str2qstr(jobName) + postFix;
	QString lsCancelledJobFile = lsCancelledJobPath + lsPathSeperator + str2qstr(jobName) + postFix;
	QString lsFailedJobFile = lsFailedJobPath + lsPathSeperator + str2qstr(jobName) + postFix;
	QString lsCompletedJobFile = lsCompletedJobPath + lsPathSeperator + str2qstr(jobName) + postFix;

	QString lsPendingJobFile, highjobfile, normaljobfile, lowjobfile;
	highjobfile = lsPendingJobPath + lsPathSeperator + HIGHLEVEL + lsPathSeperator + str2qstr(jobName) + postFix;
	normaljobfile = lsPendingJobPath + lsPathSeperator + NORMALLEVEL + lsPathSeperator + str2qstr(jobName) + postFix;
	lowjobfile = lsPendingJobPath + lsPathSeperator + LOWLEVEL + lsPathSeperator + str2qstr(jobName) + postFix;
	if (QFile::exists(highjobfile))
	{
		lsPendingJobFile = highjobfile;
	}
	else if (QFile::exists(normaljobfile))
	{
		lsPendingJobFile = normaljobfile;
	}
	else if (QFile::exists(lowjobfile))
	{
		lsPendingJobFile = lowjobfile;
	}
	else
	{
		lsPendingJobFile = "";
	}

	if (lsPendingJobFile != "" && QFile::exists(lsPendingJobFile))
	{
		jobStatus = 0; 
	}
	else if (QFile::exists(lsRunningJobFile))
	{
		jobStatus = 1; 
	}
	else if (QFile::exists(lsCompletedJobFile))
	{
		jobStatus = 2; 
	}
	else if (QFile::exists(lsCancelledJobFile))
	{
		jobStatus = 3; 
	}
	else if (QFile::exists(lsFailedJobFile))
	{
		jobStatus = 4; 
	}
	else
	{
		return false;
	}

	
	return true;
}


int getTotalTimeinSec(const QDateTime& firstDateTime, const QDateTime& lastDateTime)
{
	qint64 diffs = firstDateTime.secsTo(lastDateTime);
	if (diffs < 0)
		diffs = 0;

	return diffs;
}

QString getTotalTime(QDateTime& firstDateTime, const QDateTime& lastDateTime)
{
	qint64 diffs = firstDateTime.secsTo(lastDateTime);
	if (diffs < 0)
		diffs = 0;

	int hours = diffs / 3600;
	int left_secs = diffs - hours * 3600;
	int mins = left_secs / 60;
	int secs = left_secs % 60;

	QString timestr = QString("%1:%2:%3").arg((hours < 10) ? ("0" + QString::number(hours)) : QString::number(hours))
		.arg((mins < 10) ? ("0" + QString::number(mins)) : QString::number(mins)).arg((secs < 10) ? "0" + QString::number(secs) : QString::number(secs));


	return timestr;
}


int getDiffTime(QDateTime& firstDateTime, QDateTime& lastDateTime)
{
	qint64 diffs = firstDateTime.secsTo(lastDateTime);
	if (diffs < 0)
		diffs = 0;

	return diffs;
}


QDateTime getEarlyDateTime(QVector<QDateTime>& stageDateTimeVector)
{
	int count = stageDateTimeVector.size();

	if (count <= 0)
		return QDateTime();

	if (count == 1)
		return stageDateTimeVector.at(0);


	QDateTime dtEarly = stageDateTimeVector.at(0);

	for (int i = 1; i < count; i++)
	{
		QDateTime dtCurrent = stageDateTimeVector.at(i);
		qint64 diffs = dtEarly.secsTo(dtCurrent);
		if (diffs < 0)
			dtEarly = dtCurrent;
	}

	return dtEarly;
}

QDateTime getLateDateTime(QVector<QDateTime>& stageDateTimeVector)
{
	int count = stageDateTimeVector.size();

	if (count <= 0)
		return QDateTime();

	if (count == 1)
		return stageDateTimeVector.at(0);

	QDateTime dtLate = stageDateTimeVector.at(0);

	for (int i = 1; i < count; i++)
	{
		QDateTime dtCurrent = stageDateTimeVector.at(i);
		qint64 diffs = dtLate.secsTo(dtCurrent);
		if (diffs > 0)
			dtLate = dtCurrent;
	}

	return dtLate;
}

int UpdateEngineStatus()
{
	std::vector<EngineInfo_s> engineinfos;

	try
	{
		QString  Qenginedir = Settings::getEngineJobQueue() + "/" + JOBENGINESSTR;

		std::string enginedir = qstr2str(Qenginedir);
		const std::filesystem::path engPath = AI3D::CORE::File::BoostPathFromUtf8(enginedir);

		if (!std::filesystem::exists(engPath))
		{
			LOGI("enginedir " + enginedir + " not found.");
			return -2;
		}
		std::vector<std::string> enginefiles;
		for (const auto& entry : std::filesystem::directory_iterator(engPath))
		{

			if (entry.is_regular_file())
			{
				std::string filepath = AI3D::CORE::File::BoostPathToUtf8String(entry.path());

				enginefiles.push_back(filepath);

			}
		}


		if (enginefiles.empty())
		{
			
			return -1;
		}

		engineinfos.clear();
		for (auto finfo : enginefiles)
		{

			EngineInfo_s run;
			if (std::filesystem::exists(AI3D::CORE::File::BoostPathFromUtf8(finfo)))
			{
				if (ENGINE_USE_BIN) {
					if (run.loadbin(finfo))
					{
						engineinfos.push_back(run);
					}
				}
				else {
					if (run.load(finfo))
					{
						engineinfos.push_back(run);
					}
				}
				
			}
		}
	}
	catch (const std::filesystem::filesystem_error& fse)
	{
		std::ostringstream oss;
		oss << "filesystem error:" << fse.code() << " " << fse.what() << " " << fse.path1().string() << " " << fse.path2().string();

		LOGI(oss.str());
		
		return -1;
	}
	catch (std::exception& err)
	{
		std::ostringstream oss;
		oss << "exception:" << err.what();
		LOGI(oss.str());
		return -1;
	}

	if (std::all_of(engineinfos.begin(), engineinfos.end(), [](EngineInfo_s& run) {return run.Status == 1; }))
		return 0;
	return 1;
}
bool cmp_val(const std::pair<std::string, int>& left, const std::pair<std::string, int>& right)
{
	return left.second < right.second;
}
int GetJobStagesStatus(JobFeedBack_s& feadback, ATTimeSummary_s& attimesum, QVector<JobStage>& jobStages)
{
	std::map<std::string, std::set<int>> stagemap_ids;
	
	for (auto iter : attimesum.tasksmap)
	{
		if (iter.second.FunctionName != "")
		{
			stagemap_ids[iter.second.FunctionName].insert(iter.first);
		}
		else
		{
			return 0;
		}
	}

	std::map<std::string, int> stagemap;
	for (auto iter : attimesum.tasksmap)
	{
		stagemap[iter.second.FunctionName] = iter.first;
	}

	int cancledid = -1; int failureid = -1;
	if (feadback.Status == jobsta_e::STATUS_CANCLE)
	{
		for (auto iter : attimesum.tasksmap)
		{
			if (iter.second.Status == jobsta_e::STATUS_CANCLE)
			{
				cancledid = iter.first;
			}
		}

		if (cancledid < 0)
		{
			int taskid = attimesum.GetLastRunningTaskId();
			if (taskid == -1)
			{
				taskid = attimesum.GetFirstPendingTaskId();
				if (taskid >= 0)
					cancledid = taskid;
			}
		}

		if (cancledid >= 0)
		{

			for (auto iter : stagemap_ids[attimesum.tasksmap.at(cancledid).FunctionName])
			{

				attimesum.tasksmap.at(iter).Status = jobsta_e::STATUS_CANCLE;
			}

			if (stagemap_ids.count(attimesum.tasksmap.at(cancledid).FunctionName))
			{


				int latsid = *stagemap_ids[attimesum.tasksmap.at(cancledid).FunctionName].crbegin();

				for (auto& iter : attimesum.tasksmap)
				{
					if (iter.first > latsid)
					{
						iter.second.Status = jobsta_e::STATUS_UNKNOWN;
					}
				}

			}
		}
		else
		{
			
		}
	}
	else if (feadback.Status == jobsta_e::STATUS_FAILURE)
	{
		for (auto iter : attimesum.tasksmap)
		{
			if (iter.second.Status == jobsta_e::STATUS_FAILURE)
			{
				failureid = iter.first;
			}
		}
		if (failureid >= 0)
		{
			for (auto iter : stagemap_ids[attimesum.tasksmap.at(failureid).FunctionName])
			{

				attimesum.tasksmap.at(iter).Status = jobsta_e::STATUS_FAILURE;
			}

			if (stagemap_ids.count(attimesum.tasksmap.at(failureid).FunctionName))
			{
				int latsid = *stagemap_ids[attimesum.tasksmap.at(failureid).FunctionName].crbegin();
				for (auto& iter : attimesum.tasksmap)
				{
					if (iter.first > latsid)
					{
						iter.second.Status = jobsta_e::STATUS_UNKNOWN;
					}
				}
			}
		}
	}


	std::vector<std::pair<std::string, int>> stagevec(stagemap.begin(), stagemap.end());


	sort(stagevec.begin(), stagevec.end(), cmp_val);

	for (auto iter : stagevec)
	{
		JobStage tasktemp;
		tasktemp.functionName = QString::fromStdString(iter.first);

		tasktemp.stageTotalTime = attimesum.GetTimeSummaryBetweenStages(iter.first);
		attimesum.GetTaskFinishedNum(iter.first, tasktemp.stagedTotalNum, tasktemp.completedNum);


		if (tasktemp.stagedTotalNum > 0)
		{
			if (tasktemp.stagedTotalNum == tasktemp.completedNum)
			{
				tasktemp.status = int(jobsta_e::STATUS_COMPLETE);
				QDateTime dateEarly, dateLate;
				attimesum.GetStageStartAndEndTime(iter.first, dateEarly, dateLate);
			}
			if (tasktemp.completedNum > 0)
			{

				if (tasktemp.stagedTotalNum > tasktemp.completedNum)
				{
					tasktemp.status = int(feadback.Status);
					tasktemp.stageTotalTime = attimesum.GetTimeSummaryBetweenStagesToCurrenttime(iter.first);
				}
			}
			else if (tasktemp.completedNum == 0)
			{
				if (feadback.Status == jobsta_e::STATUS_RUNNING)
				{


					QDateTime dateEarly, dateLate;
					attimesum.GetStageStartAndEndTime(iter.first, dateEarly, dateLate);
					if (dateEarly == QDateTime())
					{
						tasktemp.status = jobsta_e::STATUS_PENDDING;
						tasktemp.stageTotalTime = "00:00:00";
					}
					else
					{
						tasktemp.status = jobsta_e::STATUS_RUNNING;
						tasktemp.stageTotalTime = attimesum.GetTimeSummaryBetweenStagesToCurrenttime(iter.first);
					}
				}
				else
				{
					tasktemp.status = attimesum.tasksmap.at(iter.second).Status;
				}
			}
		}

		jobStages.push_back(tasktemp);
	}


	return jobStages.size();
}





bool GetRealTimeInfo(jobexchangefile_s& jobinfo, bool* bGotNewJobInfo, bool* bGettingJobInfo)
{
	if (*bGettingJobInfo==true && *bGotNewJobInfo==false)
	{
		
		LOGI("Get job info.");
		return true;
	}
	*bGettingJobInfo = true;


	jobsta_e status = jobinfo.status;

	std::string timefile = jobinfo.timesumfile;
	std::string feedbackfile = jobinfo.feadbackfile;

	
	infoforshow_s show = jobinfo.show;
	jobsta_e jobstatusnow = jobsta_e::STATUS_UNKNOWN;
	int enginstatus = UpdateEngineStatus();
	bool bstatusknown = false;
	std::string job = jobinfo.jobname;

		
		{
			{

				if (QFileInfo(str2qstr(feedbackfile)).exists() && QFileInfo(str2qstr(timefile)).exists())
				{
					JobFeedBack_s feadback; ATTimeSummary_s attimesum;
					if (feadback.load(feedbackfile) && (attimesum.load(timefile)))
					{
						jobstatusnow = feadback.Status;
						bstatusknown = (jobstatusnow == jobsta_e::STATUS_UNKNOWN) ? false : true;
						
						if (bstatusknown)
						{
							if (attimesum.runinfo.SubmitTime != "")
							{
								show.SubmitTime = QString::fromStdString(attimesum.runinfo.SubmitTime);
								show.SubmitTime = QDateTime::fromString(show.SubmitTime, "yyyyMMddhhmmss").toString("yyyy/MM/dd hh:mm");
								if (attimesum.runinfo.runninginfo.EndTime != "")
								{
									show.EndTime = QString::fromStdString(attimesum.runinfo.runninginfo.EndTime);
									show.EndTime = QDateTime::fromString(show.EndTime, "yyyyMMddhhmmss").toString("yyyy/MM/dd hh:mm");
								}
							}

							if (jobstatusnow == jobsta_e::STATUS_PENDDING)
							{

								show.ATStagetext = QString::fromStdString(blk_status_str.at(jobstatusnow));
								if (enginstatus == -1)
								{
									show.ATStatustext = "No engine found.Start an engine.";
									LOGI("No engine found.Start an engine inside GetRealTimeInfo.");
									
									show.ATStatustextstylestr = "color: #c80000;";
								}
								else if (enginstatus == 0)
								{
									show.ATStatustext = "Waiting for handle.";
									LOGI("Waiting for handle.");
									
								}
								show.progreesvalue = 0;
								
								{
									
								}
								

							}

							else
							{
								
								show.progreesvalue = feadback.Percent;

								
								if (GetJobStagesStatus(feadback, attimesum, jobinfo.vec_job) > 0)
								{



								}

								if (jobstatusnow == jobsta_e::STATUS_RUNNING)
								{
									show.ATStagetext = QString::fromStdString(feadback.Msg);
									show.progressstylestr = RUNNINGSTYLE;
									
									if (enginstatus == -1)
									{
										show.ATStatustext = "No engine found.Start an engine.";
										show.ATStatustextstylestr = "color: #c80000;";
										jobstatusnow = jobsta_e::STATUS_CANCLE;

										LOGI("No engine found.Start an engine.");

										feadback.Status = jobstatusnow;
										feadback.save_with_retry(feedbackfile);

										int taskid = attimesum.GetLastRunningTaskId();
										if (taskid != -1)
										{
											attimesum.tasksmap.at(taskid).Status = jobstatusnow;
											attimesum.save(timefile);
										}

										QString postFix = "";
										if (JOB_INFO_USE_BIN) {
											postFix = BINFILE_POSTFIX;
										}
										else {
											postFix = JSONFILE_POSTFIX;
										}
										QString jobfile = Settings::getMasterJobQueue() + "/" + JOBRUNNINGSTR + "/" + str2qstr(job) + postFix;
										
										if (QFileInfo::exists(jobfile))
										{
											
											QFile(jobfile).remove();
										}
										QString lockfile = jobfile + LOCKFILE_POSTFIX;
										
										if (QFileInfo::exists(lockfile))
										{
											
											QFile(lockfile).remove();
										}
										show.ATStatustext = QString::fromStdString(feadback.Msg + " " + blk_status_str.at(jobstatusnow));

									}

								}
								else if (jobstatusnow == jobsta_e::STATUS_FAILURE)
								{
									show.progressstylestr = FAILURESTYLE;
									show.ATStagetext = QString::fromStdString(blk_status_str.at(jobstatusnow));
									show.ATStatustext = QString::fromStdString(feadback.Msg);
									show.ATStatustextstylestr = "color: #c80000;";

								}
								else if (jobstatusnow == jobsta_e::STATUS_CANCLE)
								{
									show.progressstylestr = CANCELSTYLE;
									show.ATStagetext = QString::fromStdString(blk_status_str.at(jobstatusnow));
									show.ATStatustext = QString::fromStdString(feadback.Msg);
									show.ATStatustextstylestr = "color: #F7BA0B;";

								}
								
								else if ((jobstatusnow == jobsta_e::STATUS_COMPLETE))
								{
										show.progressstylestr = RUNNINGSTYLE;
										show.ATStagetext = QString::fromStdString(blk_status_str.at(jobstatusnow));
									
									if ((GetJobType(job) == JOB_AT))
									{



									}
									else if ((GetJobType(job) == JOB_BATCH))
									{



									}

								}
							}
						}
					}
				}


				show.status = jobstatusnow;
				
			}
	
		}
		return true;
}