
#include "Core/TaskCommandSet.h"
#include "Core/File.h"
#include "Core/PointManager.h"
#include "Core/TaskDef.h"
#include "Core/Types.h"
#include "Util/TaskProcess.h"


namespace AI3D
{
    namespace CORE
    {
        TaskCommandSet::TaskCommandSet()
        {

        };


        bool TaskCommandSet::CreateJobAndFeedbackFiles(std::string jobpath, std::string projectpath, std::string itempath,
        std::string hostname, std::string datetime, std::string dir, std::string job, PointFreezeInfo freezeResult, bool bUseTimeSummary)
        {
            File::CreateDirIfNotExists(jobpath,true);
            JobFeedBack_s feedback;
            feedback.Msg = GetTaskStartingString(job);

            feedback.Percent = 0;
            feedback.Status = jobsta_e::STATUS_PENDDING;

            
            std::string feedbackfile = "";
            if (JOB_FEEDBACK_USE_BIN) {
                feedbackfile = MAKE_FEEDBAK_BIN_FILE(dir, job);
            }
            else {
                feedbackfile = MAKE_FEEDBAK_JSON_FILE(dir, job);
            }

            JobFullInfo_s jobinfo;
            std::string userName = getenv("USERNAME");

            WCHAR PCName[255];
            WCHAR UserName[255];
            unsigned long size = 255;
            GetComputerName(PCName, &size);
            GetUserName(UserName, &size);


            

            jobinfo.tg.feedback = feedback;

            jobinfo.SetPendingInfo(GetTypeId(job), job, projectpath, itempath,
                RunInfo_s(hostname, userName, datetime));

            jobinfo.tg.job.point_info.freeze_no = freezeResult.freeze_no;
            jobinfo.tg.job.point_info.frozen_points = freezeResult.frozen_points;
            jobinfo.tg.job.point_info.total_balance = freezeResult.total_balance;
            jobinfo.tg.job.point_info.available_points = freezeResult.available_points;

            std::string postFix = "";
            if (JOB_INFO_USE_BIN) {
                postFix = BINFILE_POSTFIX;
            }
            else {
                postFix = JSONFILE_POSTFIX;
            }
            LOGI("=================save jobpath:" + jobpath);
            jobinfo.save(jobpath + "/" + job + postFix);
            feedback.save_with_retry(feedbackfile);

            if (bUseTimeSummary)
            {
                
                std::string timesumfile = "";
                if (JOB_FEEDBACK_USE_BIN) {
                    timesumfile = MAKE_TIMESUM_BIN_FILE(dir, job);
                }
                else {
                    timesumfile = MAKE_TIMESUM_BIN_FILE(dir, job);
                }
                timesumfile = File::EnsureUnifySlash(timesumfile);
                ATTimeSummary_s attimesum;

                attimesum.runinfo = jobinfo.tg.runinfo;
                attimesum.save(timesumfile);
            }

            return true;
        }

        bool TaskCommandSet::CheckJobQueuePath(const std::string& lsMasterJobQueue, std::string& lsPendingJobPath,
            std::string& lsRunningJobPath, std::string& lsCancelledJobPath,
            std::string& lsFailedJobPath, std::string& lsCompletedJobPath, std::string& lsPathSeperator, int& errorCode)
        {


            if (lsMasterJobQueue.empty())
            {
                errorCode = -1;
                return false;
            }

            {

                if (!File::ExistsPath(lsMasterJobQueue) || !File::ExistsDir(lsMasterJobQueue))
                {
                    errorCode = -2;
                    return false;
                }
            }

            if (lsPathSeperator.empty())
                lsPathSeperator = "/";

            {
                lsPendingJobPath = lsMasterJobQueue + lsPathSeperator + JOBPENDINGSTR;

                if (!File::ExistsPath(lsPendingJobPath) || !File::ExistsDir(lsPendingJobPath))
                {
                    lsPathSeperator = "\\";
                    lsPendingJobPath = lsMasterJobQueue + lsPathSeperator + JOBPENDINGSTR;


                    if (!File::ExistsPath(lsPendingJobPath) || !File::ExistsDir(lsPendingJobPath))
                    {
                        errorCode = -3;
                        return false;
                    }
                }
            }

            {
                lsRunningJobPath = lsMasterJobQueue + lsPathSeperator + JOBRUNNINGSTR;

                if (!File::ExistsPath(lsRunningJobPath) || !File::ExistsDir(lsRunningJobPath))
                {
                    errorCode = -4;
                    return false;
                }
            }

            {
                lsCancelledJobPath = lsMasterJobQueue + lsPathSeperator + JOBCANCELLEDSTR;

                if (!File::ExistsPath(lsCancelledJobPath) || !File::ExistsDir(lsCancelledJobPath))
                {
                    errorCode = -5;
                    return false;
                }
            }

            {
                lsFailedJobPath = lsMasterJobQueue + lsPathSeperator + JOBFAILEDSTR;

                if (!File::ExistsPath(lsFailedJobPath) || !File::ExistsDir(lsFailedJobPath))
                {
                    errorCode = -6;
                    return false;
                }
            }

            {
                lsCompletedJobPath = lsMasterJobQueue + lsPathSeperator + JOBCOMPLETEDSTR;

                if (!File::ExistsPath(lsCompletedJobPath) || !File::ExistsDir(lsCompletedJobPath))
                {
                    errorCode = -7;
                    return false;
                }
            }

            errorCode = 0;
            return true;
        }

        bool TaskCommandSet::CheckJobStatusInsideJobQueuePath(const std::string& lsMasterJobQueue,std::string& jobName,std::string &fullPathJobName,int& jobStatus)
        {
            std::string lsPendingJobPath;
            std::string lsRunningJobPath;
            std::string lsCancelledJobPath;
            std::string lsFailedJobPath;
            std::string lsCompletedJobPath;
            std::string lsPathSeperator;
            int errorCode = -1;

            jobStatus = -1;
            fullPathJobName = jobName;

            if (lsMasterJobQueue.empty() || jobName.empty())
            {
                return false;
            }

            if (!TaskCommandSet::CheckJobQueuePath(lsMasterJobQueue, lsPendingJobPath, lsRunningJobPath, lsCancelledJobPath,
                lsFailedJobPath, lsCompletedJobPath, lsPathSeperator, errorCode))
                return false;

            errorCode = 0;
            std::string postFix = "";
            if (JOB_INFO_USE_BIN) {
                postFix = BINFILE_POSTFIX;
            }
            else {
                postFix = JSONFILE_POSTFIX;
            }
            std::string lsRunningJobFile = lsRunningJobPath + lsPathSeperator + jobName + postFix;
            std::string lsCancelledJobFile = lsCancelledJobPath + lsPathSeperator + jobName + postFix;
            std::string lsFailedJobFile = lsFailedJobPath + lsPathSeperator + jobName + postFix;
            std::string lsCompletedJobFile = lsCompletedJobPath + lsPathSeperator + jobName + postFix;
            std::string lsPendingJobFile, highjobfile, normaljobfile, lowjobfile;

            highjobfile = lsPendingJobPath + lsPathSeperator + HIGHLEVEL + lsPathSeperator + jobName + postFix;
            normaljobfile = lsPendingJobPath + lsPathSeperator + NORMALLEVEL + lsPathSeperator + jobName + postFix;
            lowjobfile = lsPendingJobPath + lsPathSeperator + LOWLEVEL + lsPathSeperator + jobName + postFix;

            
            
            
            
            

            if (File::ExistsFile(highjobfile))
            {
                lsPendingJobFile = highjobfile;
                fullPathJobName = lsPendingJobFile;
                jobStatus = 0;
            }
            else if (File::ExistsFile(normaljobfile))
            {
                lsPendingJobFile = normaljobfile;
                fullPathJobName = lsPendingJobFile;
                jobStatus = 0;
            }
            else if (File::ExistsFile(lowjobfile))
            {
                lsPendingJobFile = lowjobfile;
                fullPathJobName = lsPendingJobFile;
                jobStatus = 0;
            }
            else if (File::ExistsFile(lsRunningJobFile))
            {
                fullPathJobName = lsRunningJobFile;
                jobStatus = 1; 
            }
            else if (File::ExistsFile(lsCompletedJobFile))
            {
                fullPathJobName = lsCompletedJobFile;
                jobStatus = 2; 
            }
            else if (File::ExistsFile(lsCancelledJobFile))
            {
                fullPathJobName = lsCancelledJobFile;
                jobStatus = 3; 
            }
            else if (File::ExistsFile(lsFailedJobFile))
            {
                fullPathJobName = lsFailedJobFile;
                jobStatus = 4; 
            }
            else
            {
                return false;
            }

            return true;
        }

        bool TaskCommandSet::DoCancelJob(const std::string& lsMasterJobQueue, const std::string& feedbackfile, const std::string& jobName, int& errorCode)
        {


            std::string lsPendingJobPath;
            std::string lsRunningJobPath;
            std::string lsCancelledJobPath;
            std::string lsFailedJobPath;
            std::string lsCompletedJobPath;
            std::string lsPathSeperator = "/";



            if (jobName.empty() || feedbackfile.empty())
            {
                errorCode = -1;
                {
                    std::ostringstream oss;
                    oss << __FUNCTION__ << " LINE " << __LINE__ << " doCancel.";
                    LOGI(oss.str());
                }

                return false;
            }

            if (!CheckJobQueuePath(lsMasterJobQueue, lsPendingJobPath, lsRunningJobPath, lsCancelledJobPath, lsFailedJobPath, lsCompletedJobPath, lsPathSeperator, errorCode))
            {
                errorCode += (-10);
                {
                    std::ostringstream oss;
                    oss << __FUNCTION__ << " LINE " << __LINE__ << " doCancel.";
                    LOGI(oss.str());
                }

                return false;
            }
            std::string postFix = "";
            if (JOB_INFO_USE_BIN) {
                postFix = BINFILE_POSTFIX;
            }
            else {
                postFix = JSONFILE_POSTFIX;
            }

            std::string lsRunningJobFile = lsRunningJobPath + lsPathSeperator + (jobName)+ postFix;
            std::string lsCancelledJobFile = lsCancelledJobPath + lsPathSeperator + (jobName)+postFix;
            std::string lsFailedJobFile = lsFailedJobPath + lsPathSeperator + (jobName)+postFix;
            std::string lsCompletedJobFile = lsCompletedJobPath + lsPathSeperator + (jobName)+postFix;

            
            
            
            std::string lsPendingJobFile, highjobfile, normaljobfile, lowjobfile;
            highjobfile = lsPendingJobPath + lsPathSeperator + HIGHLEVEL + lsPathSeperator + (jobName)+postFix;
            normaljobfile = lsPendingJobPath + lsPathSeperator + NORMALLEVEL + lsPathSeperator + (jobName)+postFix;
            lowjobfile = lsPendingJobPath + lsPathSeperator + LOWLEVEL + lsPathSeperator + (jobName)+postFix;
            
            if (File::ExistsFile(highjobfile))
            {
                lsPendingJobFile = highjobfile;
            }
            else if (File::ExistsFile(normaljobfile))
            {
                lsPendingJobFile = normaljobfile;
            }
            else if (File::ExistsFile(lowjobfile))
            {
                lsPendingJobFile = lowjobfile;
            }
            else
            {
                lsPendingJobFile = "";
            }

            int retryTimes = 0;

            if (lsPendingJobFile != "" && File::ExistsFile(lsPendingJobFile))
            {

                {
                    std::ostringstream oss;
                    oss << __FUNCTION__ << " LINE " << __LINE__ << " doCancel.";
                    LOGI(oss.str());
                }


                {
                    JobFeedBack_s feedback;


                    if (!feedback.load_with_retry(feedbackfile))
                    {
                        feedback.Percent = 0.0;
                        feedback.TaskRetVal = 0;
                    }

                    feedback.Status = jobsta_e::STATUS_CANCLE;
                    feedback.Msg = "";
                    feedback.save_with_retry(feedbackfile);

                    JobFullInfo_s jobinfo((lsPendingJobFile));


                    jobinfo.tg.feedback = feedback;

                    std::string  datatime = GetCurrentTimeStr();
                    
                    
                    jobinfo.tg.runinfo.runninginfo.EndTime = datatime;


                    if (!jobinfo.save((lsCancelledJobFile)))
                    {
                        LOGI("failed to move pending job file to cancelled job queue directory inside doCancelJob!!!");
                        return false;
                    }
                    else
                    {
                        
                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                        bool removeResult = File::RemoveFile(lsPendingJobFile);

                        
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));

                        {
                            std::ostringstream oss;
                            oss << "cancelled pending job file:" << lsCancelledJobFile << " " <<  lsPendingJobFile << " " << feedbackfile;
                            LOGI(oss.str());
                        }

                        return removeResult;
                    }


                } 

            }
            else if (File::ExistsFile(lsRunningJobFile))
            {
                
                
                JobFeedBack_s feedback;


                {
                    std::ostringstream oss;
                    oss << __FUNCTION__ << " LINE " << __LINE__ << " doCancel.";
                    LOGI(oss.str());
                }

                std::ostringstream oss;
                oss << "======================Cancel running job:" << lsRunningJobFile << std::endl;
                LOGI(oss.str());
                {

                    if (!feedback.load_with_retry(feedbackfile))
                    {
                        feedback.Percent = 0.0;
                        feedback.TaskRetVal = 0;
                        feedback.Msg = "";
                    }

                    feedback.Status = jobsta_e::STATUS_CANCLE;
                    feedback.save_with_retry(feedbackfile);


                    JobFullInfo_s jobinfo((lsRunningJobFile));


                    int taskid = jobinfo.tg.GetLastRunningTaskId();
                    if (taskid == -1)
                        taskid = jobinfo.tg.GetFirstPendingTaskId();

                    jobinfo.tg.feedback = feedback;
                    std::string datetime = GetCurrentTimeStr();
                    if (taskid >= 0)
                        jobinfo.tg.tasksmap.at(taskid).Status = int(feedback.Status);
                    jobinfo.tg.runinfo.runninginfo.EndTime = datetime;

                    if (!jobinfo.save((lsCancelledJobFile)))
                    {
                        LOGI("failed to move running job file to cancelled job queue directory inside doCancelJob!!!");
                        return false;
                    }
                    else
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(200));

                        bool removeResult = File::RemoveFile(lsRunningJobFile);


                        std::this_thread::sleep_for(std::chrono::milliseconds(100));

                        {
                            std::ostringstream oss;
                            oss << "cancelled running job file:" << lsRunningJobFile << " " <<  lsCancelledJobFile << " " << feedbackfile;
                            LOGI(oss.str());
                        }

                        return removeResult;
                    }


                }

            }
            else
            {
                LOGI("no file has been cancelled,what is up inside doCancelJob?");
                return false;
            }

            errorCode = 0;
            return true;
        }

    }
}






       
