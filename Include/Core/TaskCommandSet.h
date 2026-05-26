#ifndef _AI3D_TASKCOMMANDSET_H_
#define _AI3D_TASKCOMMANDSET_H_
#include <Constants.h>
#include <glog/logging.h>



namespace AI3D
{
    namespace CORE
    {
        class AI3D_API TaskCommandSet
        {
        public:
            TaskCommandSet();
            static bool CheckJobQueuePath(const std::string& lsMasterJobQueue, std::string& lsPendingJobPath,
                std::string& lsRunningJobPath, std::string& lsCancelledJobPath,
                std::string& lsFailedJobPath, std::string& lsCompletedJobPath, std::string& lsPathSeperator, int& errorCode);
            static bool CheckJobStatusInsideJobQueuePath(const std::string& lsMasterJobQueue, std::string& jobName, std::string& fullPathJobName,int& jobStatus);
            static bool DoCancelJob(const std::string& lsMasterJobQueue, const std::string& tocancelPath, const std::string& jobName, int& errorCode);
            static bool CreateJobAndFeedbackFiles(std::string jobpath, std::string projectpath, std::string itempath,
                std::string hostname, std::string datetime, std::string dir, std::string job,bool bUseTimeSummary =false );
        };
    }
}
#endif