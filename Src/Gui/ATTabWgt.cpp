#include "Gui/BlockWgt.h"
#include <algorithm>
#include "Core/TaskDef.h"
#include <QVariant>
#include <QDateTime>
#include <QtConcurrent>
#include <QStringList>
#include <QHostInfo>
#include "Util/TaskProcess.h"
#include <QThreadPool>
#include "Gui/MohackerWin.h"
#include "Core/CoordinateSystem.h"
#include "Gui/BlockManager.h"
#include "Gui/ImportGcpDia.h"
#include "Gui/PosSigmaDia.h"
#include "Core/ControlPoint.h"
#include "Core/CoordinateSystem.h"
#include "Gui/ProjectManager.h"
#include "Gui/BatchPrepareWgt.h"
#include "Core/Timer.h"
#include "Gui/ImportPosDia.h"
//#include "Gui/Network.h"
#include "Core/Types.h"
#include "Core/ATOptions.h"
#include"Gui/AddSigGcp.h"
#include "Util/Statistic.h"
#include "Util/Settings.h"
#include "Util/TaskProcess.h"
//#include "Gui/OTA.h"
#include "Util/JobMonitor.h"
#include "Core/TaskCommandSet.h"
#include "Core/ATCommandSet.h"
#include "Util/Software.h"
#include "Core/File.h"
#include <filesystem>
//?chy InitGcpData
using namespace AI3D::CORE;

// util function,places here temporarily
int CheckEncode4StdString(std::string& stdstr)
{


    return -1;
}

int CheckEncode4QString(QString& str)
{


    return -1;
}

//该函数在于前面已经组织好task信息.然后调此函数
//提交任务时生成的任务文件，job和feedback文件，理论上time文件不需要，待测试1121
bool CreatePendingJob(std::string path, std::string jobfilename, std::string projectfile,std::string blockitem, std::string datatime)
{
    std::string job = AI3D::CORE::File::GetFileNameWithoutExtension(jobfilename);
    
    // check whether the job queue is in correct state based on the return value from the following function.
    // if (!boost::filesystem::exists(Settings::getMasterJobQueue().toStdString()) || !boost::filesystem::is_directory(Settings::getMasterJobQueue().toStdString()))
    bool ret = JobMonitor::CreateDirs();
    if (!ret)
        return false;

    // what will happen to this jobfilename with another job queue directory changed inside current function?
    // calling current function one more time using new jobfilename generated based on newer job queue directory. 

    try
    {
        if (std::filesystem::is_directory(File::BoostPathFromUtf8(File::GetParentDir(jobfilename))))
        {
            //需要创建两个文件，一个是job一个feedback
            JobFeedBack_s feedback;
            feedback.Msg = GetTaskStartingString(job);// StepATFromfunctionToshow.at(task.functionname_);

            feedback.Percent = 0;//cc为 0;
            feedback.Status = jobsta_e::STATUS_PENDDING;

            //std::string feedbackfile = MAKE_FEEDBAK_FILE(path, job);// path + PATH_SEPARATOR_STR + "feedback_" + task.job_ + ".json";
            std::string feedbackfile = "";
            if (JOB_FEEDBACK_USE_BIN) {
                feedbackfile = MAKE_FEEDBAK_BIN_FILE(path, job);
            }
            else {
                feedbackfile = MAKE_FEEDBAK_JSON_FILE(path, job);
            }
            JobFullInfo_s jobinfo;
            std::string userName = getenv("USERNAME");

            // 获取当前主机名,IP地址,以及当前Engine的进程ID.
            QString hostName = QHostInfo::localHostName();
            jobinfo.tg.feedback = feedback;
            //std::string blockidstr = BLOCK_PRE + std::to_string(task.blockId);;
            jobinfo.SetPendingInfo(GetTypeId(job), job, projectfile, blockitem,
                RunInfo_s(hostName.toStdString(), userName, datatime));

            jobinfo.save(jobfilename);
            //std::string timesumfile = path + PATH_SEPARATOR_STR + "time_" + job + ".json";
            std::string timesumfile = "";
            if (JOB_FEEDBACK_USE_BIN) {
                timesumfile = MAKE_TIMESUM_BIN_FILE(path, job);
            }
            else {
                timesumfile = MAKE_TIMESUM_BIN_FILE(path, job);
            }
            ATTimeSummary_s attimesum;

            attimesum.runinfo = jobinfo.tg.runinfo;
            attimesum.save(timesumfile);

            feedback.save_with_retry(feedbackfile);
        }
        else
        {
            // now it should not come here while running in normal state.
            // if current local directory does not exists,try to mkdir it,
            // and then do all the above works which lies inside the 'if' branch block again.
            //LOGE("Invalid job path! Using default job path instead!");
            LOGE("Invalid job path! Using default job path instead:" + jobfilename);
            QString path = QCoreApplication::applicationDirPath();
            path.append("/jobs");
            QSettings* pSettings = new QSettings("HKEY_CURRENT_USER\\Software\\MoldAI\\JobQueues", QSettings::NativeFormat);
            pSettings->setValue("master", path);
            pSettings->setValue("engine", path);

            // add the other code like above?
        }
    }
    catch (const std::filesystem::filesystem_error& fse)
    {
        std::ostringstream oss;
        oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1().string() << " " << fse.path2().string();
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
namespace AI3D
{
    namespace GUI
    {
        bool cmp_val(const std::pair<std::string, int>& left, const std::pair<std::string, int>& right)
        {
            return left.second < right.second;
        }
        //通过time_job.json文件来实时获取

        int GetJobStagesStatus( JobFeedBack_s& feadback, ATTimeSummary_s& attimesum, QVector<JobStage>& jobStages)
        {
            std::map<std::string, std::set<int>> stagemap_ids;//用于记录每个阶段有哪些task

            //LOGI(std::string("inside GetJobStageStatus:") + std::to_string(__LINE__));

            //把属于某个阶段的task归类；
            for (auto iter : attimesum.tasksmap)
            {
                if (iter.second.FunctionName != "")
                {
                    stagemap_ids[iter.second.FunctionName].insert(iter.first);
                }
                else
                {
            ///     std::cout << __FILE__ << __FUNCTION__ << __LINE__ << "return 0 inside GetJobStagesStatus" << std::endl;
                    return 0;//如果有一个没有functionname说明有问题，这个一般发生在调试时文件没有写进去
                }
            }

            std::map<std::string, int> stagemap;
            for (auto iter : attimesum.tasksmap)
            {
                stagemap[iter.second.FunctionName] = iter.first;//id值最大的一個
            }
            
            ///std::cout << __FILE__ << __FUNCTION__ << __LINE__ << "stagemap size:" << stagemap.size() << "inside GetJobStagesStatus" << std::endl;
            

            //然后对每个阶段的状态取最关键的那一步的，如在哪一步cancle了就取哪一步 
            int cancledid = -1; int failureid = -1;
            if (feadback.Status == jobsta_e::STATUS_CANCLE)
            {
                for (auto iter : attimesum.tasksmap)
                {
                    if (iter.second.Status == jobsta_e::STATUS_CANCLE)
                    {
                        cancledid = iter.first;
                        //LOGI(std::string("inside GetJobStageStatus:") + std::to_string(__LINE__) + "found cancel id:" + std::to_string(cancledid));
                    }
                }
                //这种情形发生
                if (cancledid < 0)
                {
                    int taskid = attimesum.GetLastRunningTaskId();
                    if (taskid == -1)
                    {
                        taskid = attimesum.GetFirstPendingTaskId();
                        if (taskid >= 0)
                            cancledid = taskid;
                    }

                    //LOGI(std::string("inside GetJobStageStatus:") + std::to_string(__LINE__) + "found other cancel id:" + std::to_string(cancledid));
                }
                //找到了发生终止的那一个
                if (cancledid >= 0)
                {
                    //cancleid对应的那个阶段的都设置为cancel;
                    for (auto iter : stagemap_ids[attimesum.tasksmap.at(cancledid).FunctionName])
                    {

                        attimesum.tasksmap.at(iter).Status = jobsta_e::STATUS_CANCLE;
                    //  LOGI(std::string("inside GetJobStageStatus:") + std::to_string(__LINE__) + "found set other task to cancel:" + std::to_string(iter));
                    }
                    //找到该id对应的阶段的最大id值，大于这个最大id值的task均为unkonwn
                    if (stagemap_ids.count(attimesum.tasksmap.at(cancledid).FunctionName))
                    {
                        
                        //大于该阶段的设置为unkown；

                        int latsid = *stagemap_ids[attimesum.tasksmap.at(cancledid).FunctionName].crbegin();

                        for (auto& iter : attimesum.tasksmap)
                        {
                            if (iter.first > latsid)
                            {
                                iter.second.Status = jobsta_e::STATUS_UNKNOWN;
                                LOGI(std::string("inside GetJobStageStatus:") + std::to_string(__LINE__) + "found set other task to unknown:" + std::to_string(iter.first));
                            }
                        }
                        
                    }
                }
                else
                {
                    //todo 还没遇到
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
                    //attimesum.tasksmap.at(failureid).Status = jobsta_e::STATUS_FAILURE;
                    //找到该id对应的阶段的最大id值，大于这个最大id值的task均为unkonwn
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


            //对stagemap 排序
            std::vector<std::pair<std::string, int>> stagevec(stagemap.begin(), stagemap.end());


            sort(stagevec.begin(), stagevec.end(), cmp_val);

            bool bHasGotInCompleteStage = false;

            //对于在id前的则按正常
            for (auto iter : stagevec)
            {
                JobStage tasktemp;
                tasktemp.functionName = QString::fromStdString(iter.first);
                
                tasktemp.stageTotalTime = attimesum.GetTimeSummaryBetweenStages(iter.first);
            /// std::cout << __FILE__ << __FUNCTION__ << __LINE__ << "stagedTotalNum:" << tasktemp.stagedTotalNum << " completeNum:" << tasktemp.completedNum << " totaltime:" << tasktemp.stageTotalTime.toStdString() << std::endl;
                attimesum.GetTaskFinishedNum(iter.first, tasktemp.stagedTotalNum, tasktemp.completedNum);

                //LOGI("");

                if (tasktemp.stagedTotalNum > 0)
                {
                    /// todo:check stagetotaltime for non-running state.
                    if (tasktemp.stagedTotalNum == tasktemp.completedNum)
                    {
                        tasktemp.status = int(jobsta_e::STATUS_COMPLETE);
                        QDateTime dateEarly, dateLate;
                        attimesum.GetStageStartAndEndTime(iter.first, dateEarly, dateLate);
                    }
                    
                    if (tasktemp.completedNum > 0)//说明有处理完成
                    {
                        if (tasktemp.stagedTotalNum > tasktemp.completedNum)//说明正在处理
                        {
                            tasktemp.status = int(feadback.Status);//
                            ///LOGI("status:"+ std::to_string(int(feadback.Status)));

                            if (feadback.Status == jobsta_e::STATUS_RUNNING)
                            {
                                tasktemp.stageTotalTime = attimesum.GetTimeSummaryBetweenStagesToCurrenttime(iter.first);
                                //std::cout << __FILE__ << __FUNCTION__ << __LINE__ << "stagedTotalNum:" << tasktemp.stagedTotalNum << " completeNum:" << tasktemp.completedNum << " totaltime:" << tasktemp.stageTotalTime.toStdString() << std::endl;
                            /// LOGI("status:" + std::to_string(int(feadback.Status)));
                            }
                            else
                            {
                                // todo:check it later. add the following code?
                                //tasktemp.stageTotalTime = attimesum.GetTimeSummaryBetweenStagesToCurrenttime(iter.first);
                            }
                            
                            bHasGotInCompleteStage = true;
                        }
                    }
                    else if (tasktemp.completedNum == 0)//三种情况：cancle\failure\running\以及在等待，因为前面的可能还没做完
                    {
                        if (feadback.Status == jobsta_e::STATUS_RUNNING)
                        {
                            //if (attimesum.tasksmap.at(iter.second).Status == jobsta_e::STATUS_PENDDING )//pending有两种情形一种是开始了，一种是没开始

                            QDateTime dateEarly, dateLate;
                            attimesum.GetStageStartAndEndTime(iter.first, dateEarly, dateLate);
                            if (dateEarly == QDateTime())
                            {
                                tasktemp.status = jobsta_e::STATUS_PENDDING;
                                tasktemp.stageTotalTime = "00:00:00";
                                //LOGI("status:" + std::to_string(int(tasktemp.status)));
                            }
                            else
                            {
                                tasktemp.status = jobsta_e::STATUS_RUNNING;
                                //LOGI("status:" + std::to_string(int(tasktemp.status)));
                                tasktemp.stageTotalTime = attimesum.GetTimeSummaryBetweenStagesToCurrenttime(iter.first);
                                //std::cout << __FILE__ << __FUNCTION__ << __LINE__ << "stagedTotalNum:" << tasktemp.stagedTotalNum << " completeNum:" << tasktemp.completedNum << " totaltime:" << tasktemp.stageTotalTime.toStdString() << std::endl;
                            }
                        }
                        else
                        {
                            ///tasktemp.status = attimesum.tasksmap.at(iter.second).Status;
                            ///LOGI("status:" + std::to_string(int(tasktemp.status)));

                            if (bHasGotInCompleteStage)
                            {
                                tasktemp.status = jobsta_e::STATUS_UNKNOWN;
                            }
                            else
                            {
                                tasktemp.status = int(feadback.Status);
                                
                                // check it later.add the following code?
                                //tasktemp.stageTotalTime = attimesum.GetTimeSummaryBetweenStagesToCurrenttime(iter.first);

                                bHasGotInCompleteStage = true;
                            }
                        }
                    }
                }
                
                jobStages.push_back(tasktemp);
            }


            return jobStages.size();
        }

        void BlockWgt::UpdateATTabLabel(InfoForShow_s show)
        {
            jobsta_e status = show.status;
            QTabBar* tabbar = ui->tabWidget->tabBar();
            //tabbar->setEnabled(false);
            if (status == jobsta_e::STATUS_COMPLETE || status == jobsta_e::STATUS_NEW)
                tabbar->setEnabled(true);
            //时间
            
            
            ui->label_create_time->setVisible(true);
            ui->label->setVisible(true);
            ui->label_complete_time->setVisible(true);
            

            ui->label_create_time->setVisible(true);
            ui->label_create_time->setText(show.SubmitTime);
            ui->label_complete_time->setVisible(true);
            
            //进度
            if (/*status == jobsta_e::STATUS_PENDDING || status == jobsta_e::STATUS_CANCLE ||*/ status == jobsta_e::STATUS_NEW)
            {
                ui->label_Progress->setVisible(false);
                ui->progressBar_submit->setVisible(false);
                ui->label_ProgressValue->setVisible(false);
            
            }
            else
            {
                int progress = (int)show.progreesvalue;
                ui->label_Progress->setVisible(true);

                ui->label_ProgressValue->setVisible(true);
                ui->progressBar_submit->setVisible(true);
                ui->label_ProgressValue->setText(QString::number(progress) + "%");
                /*  ui->label_ProgressValue->setAlignment(Qt::AlignVCenter |Qt::AlignHCenter);*/
                ui->progressBar_submit->setValue(progress);



                ui->progressBar_submit->setStyleSheet(QString::fromStdString(show.progressstylestr));

                ui->label_Progress->setStyleSheet(tr(show.ATStatustextstylestr.c_str()));
                ui->label_ProgressValue->setStyleSheet(tr(show.ATStatustextstylestr.c_str()));
            }
            if (status == jobsta_e::STATUS_COMPLETE )
            {
                ui->label_complete_time->setText(show.EndTime);
            }
            else
            {
                ui->label_complete_time->setText("--/--");
            }

            //status
            QString text = show.ATStagetext;
            ui->label_11->setVisible(true);
            ui->label_StatusValue->setVisible(true);

            ///ui->label_StatusValue->setText(tr(show.ATStagetext.toStdString().c_str()));

            ui->label_StatusValue->setText(show.ATStagetext);

                /*ui->label_StatusValue->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
                ui->label_cancleLog->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);*/
                if (status == jobsta_e::STATUS_CANCLE || status == jobsta_e::STATUS_NEW)
                {
                    ui->label_cancleLog->setVisible(false);
                }
                else 
                {
                    ui->label_cancleLog->setVisible(true);
                    ui->label_cancleLog->setText(show.ATStatustext);
                    ui->label_cancleLog->setStyleSheet(tr(show.ATStatustextstylestr.c_str()));
                }

        }

        void BlockWgt::QuitJobInfoTimer()
        {
            if (GetRunningInfoTime->isActive())
            {
                GetRunningInfoTime->stop();
            }
        }

        int BlockWgt::UpdateEngineStatus()
        {
            QString  Qenginedir = Settings::getEngineJobQueue() + "/"+ JOBENGINESSTR ;

            ///std::string enginedir = Qenginedir.toStdString();

            std::string enginedir = qstr2str(Qenginedir);

/*
            if (!bfs::exists(enginedir))
            {
                std::cout << "enginedir " << enginedir << " doesn't exist." << std::endl;
                LOGI("enginedir " + enginedir + " doesn't exist.");

                return -2;
            }
*/
            QDir dir;
            if (!dir.exists(Qenginedir))
            {
                std::cout << "enginedir " << enginedir << " doesn't exist." << std::endl;
                LOGI("enginedir " + enginedir + " doesn't exist.");

                return -2;
            }

            std::vector<std::string> enginefiles;

/*
            boost::filesystem::directory_iterator end_itr;
            for (boost::filesystem::directory_iterator itr(enginedir); itr != end_itr; ++itr)
            {
                if (boost::filesystem::is_regular_file(itr->path()))
                {
                    std::string filepath = itr->path().string();

                    enginefiles.push_back(filepath);
                }
                else
                {
                    LOGI("is not regular file:" + itr->path().string());
                    std::cout << "is not regular file:"  << itr->path().string() << std::endl;

                    if (!bfs::exists(itr->path().string()))
                    {
                        LOGI(" not exist:" + itr->path().string());
                        std::cout << " not exist:" << itr->path().string() << std::endl;
                    }
                    else
                    {
                        LOGI(" exist:" + itr->path().string());
                        std::cout << " exist:" << itr->path().string() << std::endl;
                    }

                    QString strFilename = QString::fromStdString(itr->path().string());
                    QFileInfo finfo(strFilename);
                    if (!finfo.exists())
                    {
                        LOGI(" not existq:" + strFilename.toStdString());
                        std::cout << " not existq:" << strFilename.toStdString() << std::endl;
                    }
                    else
                    {
                        LOGI(" existq:" + strFilename.toStdString());
                        std::cout << " existq:" << strFilename.toStdString() << std::endl;
                    }
                }
            }
*/

            //如果文件夹为空，则没有engine，-1；
            //若有，则判断是否都在busy如果是则返回 0，代表有引擎但是需要等待处理
            
            ///if (enginefiles.empty())
            {
                //LOGI("enginefiles is empty.");
                //std::cout << "enginefiles is empty." << std::endl;
                QDir tmpDir(Qenginedir);
                quint64 size = 0;

                foreach(QFileInfo fileInfo, tmpDir.entryInfoList(QDir::Files))
                {
                    if(fileInfo.exists())
                    {

                        ///enginefiles.push_back(fileInfo.absoluteFilePath().toStdString());

                        enginefiles.push_back(qstr2str(fileInfo.absoluteFilePath()));


//                      LOGI("enginefiles2 exists:" + fileInfo.absoluteFilePath().toStdString());
//                      std::cout << "enginefiles2 exists:" << fileInfo.absoluteFilePath().toStdString() << std::endl;
                    }
                    else
                    {
                        LOGI("enginefiles2 not exists:" + qstr2str(fileInfo.absoluteFilePath()));
                        std::cout << "enginefiles2 not exists:" << qstr2str(fileInfo.absoluteFilePath()) << std::endl;
                    }
                }
///             return -1;
            }

            if (enginefiles.empty())
                return -1;

            std::vector<EngineInfo_s> engineinfos;
            engineinfos.clear();
            for (auto finfo : enginefiles)
            {       
                EngineInfo_s run;
                ///if (boost::filesystem::exists(finfo))
                {
                    try
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
                    catch (std::exception ex)
                    {
                        LOGI("enginefile load error:" + finfo + " " + ex.what());
                        //std::cout << "enginefiles2 exists:" << fileInfo.absoluteFilePath().toStdString() << std::endl;
                        std::cout << "enginefile load error:" << finfo << " " << ex.what() << std::endl;
                    }
                }
            }

            if (std::all_of(engineinfos.begin(), engineinfos.end(), [](EngineInfo_s& run) {return run.Status == 1; }))
                return 0;

            return 1;
        }

        //此处最大的问题在于如果master没关闭在正常处理，但是如果是master关闭block_id.bin中没有记录提交空三前的结果则会有问题
        int BlockWgt::UpdateCompleteJobATFile()
        {   
            if (!block_data_->GetTaskInfo().isFinished)
            {
            
                int recode = block_data_->UpdateCompleteATFile();

                if (AI3D_SUCCESS != recode)
                    return recode;
                
                PopulatePhotoGroupTable();
                
                controlPoints_ui_->SetBlockdata(block_data_);
                controlPoints_ui_->InitGcpData();       

            }
            
            return AI3D_SUCCESS;
        }
        
        jobsta_e GetJobStatus(std::string projectpath, std::string blockitem, std::string job)
        {
            
            std::string basepath = projectpath + "/" + blockitem + "/";

            /*std::string timefile = basepath + TIME_PREFIX + job + JSONFILE_POSTFIX;
            std::string feedbackfile = MAKE_FEEDBAK_FILE(basepath,job); */
            std::string feedbackfile = "";
            std::string timefile = "";
            if (JOB_FEEDBACK_USE_BIN) {
                feedbackfile = MAKE_FEEDBAK_BIN_FILE(basepath, job);
                timefile = MAKE_TIMESUM_BIN_FILE(basepath, job);
            }
            else {
                feedbackfile = MAKE_FEEDBAK_JSON_FILE(basepath, job);
                timefile = MAKE_TIMESUM_BIN_FILE(basepath, job);
                //feedbackName = blockitembase_path + FEEDBACK_PREFIX + jobstring + JSONFILE_POSTFIX;
            }
            //如果是new状态的话，就不会有AT页卡所以直接退出
                
            jobsta_e jobstatusnow = jobsta_e::STATUS_UNKNOWN;
            int enginstatus = UpdateEngineStatus();
            bool bstatusknown = false;
            try
            {
                
                if (std::filesystem::exists(File::BoostPathFromUtf8(feedbackfile)) && std::filesystem::exists(File::BoostPathFromUtf8(timefile)))
                {
                    JobFeedBack_s feadback; ATTimeSummary_s attimesum;
                    if (feadback.load(feedbackfile) && (attimesum.load(timefile)))
                    {
                        QString totaltime = "00:00:00";
                        jobstatusnow = feadback.Status;
                        bstatusknown = (jobstatusnow == jobsta_e::STATUS_UNKNOWN) ? false : true;
                        //如果是pending，分两种情况1：有engine，2：无引擎，两者唯一的差别就是一个有提示一个没提示
                        if (bstatusknown)
                        {
                            if (jobstatusnow != jobsta_e::STATUS_PENDDING)
                            {
                                /*totaltime = getTotalTime(attimesum.GetStartTime(), attimesum.GetLastTime());*/
                                if (jobstatusnow == jobsta_e::STATUS_RUNNING)
                                {

                                    //引擎突然不见了，

                                    if (enginstatus < 0)
                                    {
            ///                         LOGI("enginestatus is less than zero inside GetJobStatus.");
            ///                         std::cout << "enginestatus is less than zero inside GetJobStatus." << std::endl;

                                        /*
                                                                jobstatusnow = jobsta_e::STATUS_CANCLE;

                                                                feadback.Status = jobstatusnow;

                                                                feadback.save_with_retry(feedbackfile);
                                                                int taskid = attimesum.GetLastRunningTaskId();
                                                                if (taskid != -1)
                                                                {
                                                                    attimesum.tasksmap.at(taskid).Status = jobstatusnow;
                                                                    attimesum.save(timefile);
                                                                }
                                                                auto jobfile = Settings::getMasterJobQueue().toStdString() + "/" + JOBRUNNINGSTR + "/" + job + JSONFILE_POSTFIX;
                                                                if (boost::filesystem::exists(jobfile))
                                                                {
                                                                    LOGI("remove jobfile:" + jobfile);
                                                                    QFile(QString::fromStdString(jobfile)).remove();
                                                                }
                                                                auto lockfile = jobfile + LOCKFILE_POSTFIX;
                                                                if (boost::filesystem::exists(lockfile))
                                                                {
                                                                    LOGI("remove lockfile:" + lockfile);
                                                                    QFile(QString::fromStdString(lockfile)).remove();
                                                                }
                                                                */
                                    }
                                }

                            }
                        }
                    }
                }
            }
            catch (const std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1().string() << " " << fse.path2().string();
                LOGI(oss.str());
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
            }

            return jobstatusnow;
        }
        
        //genblock没关系，所以需要单独出来
        jobsta_e BlockWgt::UpdateBlockStatusToProject(AI3D::CORE::BlockObject* block_data)
        {
            
            auto taskinfo = block_data->GetTaskInfoMutual();
            if (/*GetJobType(taskinfo.job_) == JOB_AT &&*/ block_data->GetStatusMutual() == jobsta_e::STATUS_NEW)
            {
                return block_data->GetStatusMutual();
            }
            auto jobstatusnow =GetJobStatus(block_data->GetPath(), taskinfo.blockName, taskinfo.job_);
            if (GetJobType(taskinfo.job_) == JOB_BATCH && jobstatusnow == STATUS_COMPLETE)
            {
                jobstatusnow = STATUS_NEW;
            }
            //add by chy 0707
            if (block_data->GetStatus() != STATUS_COMPLETE)
            {
                block_data->SetStatus(jobsta_e(jobstatusnow));
            }
            else
            {
                jobstatusnow = STATUS_COMPLETE;
            }

            return jobstatusnow;
            
        }
        //对于任务来讲只需获取任务名称就可以获取到所有信息，任务名称包含类型block，而任务名称只需在当前job队列running目录下找
        //任务的监测不一定要一直进行，目前只有空三任务需要，其他任务任务执行完了就可以不再监测了，当然需要测试一下cc
        void BlockWgt::GetRealTimeInfo()
        {
            
            if (bGettingJobInfo && !bGotNewJobInfo)
            {
                std::cout <<  " ------------- "<< bGotNewJobInfo << " " << __LINE__ << bGotNewJobInfo << block_data_->GetId() << std::endl;
                
                return;
            }
            bGettingJobInfo = true;
            //读取time_json
            //是否存在time
            auto taskinfo = block_data_->GetTaskInfoMutual();
            
            
            //std::string timefile = block_data_->GetPath() + "/" + TIME_PREFIX + taskinfo.job_+ JSONFILE_POSTFIX;
            //std::string feedbackfile = MAKE_FEEDBAK_FILE(block_data_->GetPath(), taskinfo.job_);// block_data_->GetPath() + "/" + FEEDBACK_PREFIX + taskinfo.job_ + JSONFILE_POSTFIX;
            std::string feedbackfile = "";
            std::string timefile = "";
            if (JOB_FEEDBACK_USE_BIN) {
                feedbackfile = MAKE_FEEDBAK_BIN_FILE(block_data_->GetPath(), taskinfo.job_);
                timefile = MAKE_TIMESUM_BIN_FILE(block_data_->GetPath(), taskinfo.job_);
            }
            else {
                feedbackfile = MAKE_FEEDBAK_JSON_FILE(block_data_->GetPath(), taskinfo.job_);
                timefile = MAKE_TIMESUM_BIN_FILE(block_data_->GetPath(), taskinfo.job_);
                //feedbackName = blockitembase_path + FEEDBACK_PREFIX + jobstring + JSONFILE_POSTFIX;
            }
            //如果是new状态的话，就不会有AT页卡所以直接退出
            

            //
            InfoForShow_s show;
            jobsta_e jobstatusnow = jobsta_e::STATUS_UNKNOWN;
            int enginstatus = UpdateEngineStatus();
            bool bstatusknown = false;

            /*if (TaskProcess::GetJobType(taskinfo.job_) == JOB_AT)*/
            {

                //如果是New的状态的话是不读feedback的，但是在complete完毕后如果做全部删除操作则会引发new状态的产生
                if (block_data_->GetStatusMutual() != jobsta_e::STATUS_NEW)
                {
            
                    {
                        try
                        {
                            if (std::filesystem::exists(File::BoostPathFromUtf8(feedbackfile)) && std::filesystem::exists(File::BoostPathFromUtf8(timefile)))
                            {
                                JobFeedBack_s feadback; ATTimeSummary_s attimesum;
                                if (feadback.load(feedbackfile) && (attimesum.load(timefile)))
                                {

                                    jobstatusnow = feadback.Status;
                                    bstatusknown = (jobstatusnow == jobsta_e::STATUS_UNKNOWN) ? false : true;
                                    //如果是pending，分两种情况1：有engine，2：无引擎，两者唯一的差别就是一个有提示一个没提示
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

                                            if (AI3D::CORE::BlockObject::isChineseVersion())
                                            {
                                                ///show.ATStagetext = QString::fromStdString(blk_status_str_chinese.at(jobstatusnow));
                                                show.ATStagetext = str2qstr(blk_status_str_chinese.at(jobstatusnow));
                                            }
                                            else
                                            {
                                                show.ATStagetext = QString::fromStdString(blk_status_str.at(jobstatusnow));
                                            }
                                            if (enginstatus == -1)
                                            {
                                                if (AI3D::CORE::BlockObject::isChineseVersion())
                                                {
                                                    show.ATStatustext = "当前无引擎运行，请启动引擎.";
                                                }
                                                else
                                                {
                                                    show.ATStatustext = "No engine running.Start an engine.";
                                                }

                                                show.ATStatustextstylestr = "color: #c80000;";
                                                LOGI("No engine running.Start an engine.");
                                                //std::cout << "No engine running.Start an engine." << std::endl;
                                            }
                                            else if (enginstatus == 0)
                                            {
                                                if (AI3D::CORE::BlockObject::isChineseVersion())
                                                {
                                                    show.ATStatustext = "等待处理中.";
                                                }
                                                else
                                                {
                                                    show.ATStatustext = "Waiting to be processed.";
                                                }
                                                //std::cout << "Waiting to be processed." << std::endl;
                                            }
                                            show.progreesvalue = 0;
                                            {
                                                ui->taskList->setShowGrid(false);
                                                ui->taskList->verticalHeader()->setHidden(true);
                                                ui->taskList->horizontalHeader()->setHidden(true);
                                                ui->taskList->setStyleSheet("QTableWidget{background:#000000;border:none;color:#FFFFFF;}");



                                                int tasklen = ui->taskList->rowCount();
                                                for (int i = tasklen - 1; i >= 0; i--)
                                                {
                                                    ui->taskList->removeRow(i);
                                                }
                                            }

                                        }
                                        //运行时 stage为feadback的msg，status不显示，cc是在status的位置显示进度条
                                        else
                                        {
                                            /*jobStats.rtnTotalTime = getTotalTime(attimesum.GetStartTime(), attimesum.GetLastTime());*/
                                            show.progreesvalue = feadback.Percent;

                                            QVector<JobStage> vec_job;
                                            if (GetJobStagesStatus(feadback, attimesum, vec_job) > 0)
                                            {
                                                ///                                         std::cout << __FILE__ << __FUNCTION__ << __LINE__ << "GetJobStagesStatus > 0"  << std::endl;
                                                UpdateJobStageLists(vec_job);

                                                ///QVector<JobStage> tempvecStates;
                                            }
                                            else
                                            {
                                                ///std::cout << __FILE__ << __FUNCTION__ << __LINE__ << "GetJobStagesStatus <= 0" << std::endl;
                                            }

                                            if (jobstatusnow == jobsta_e::STATUS_RUNNING)
                                            {
                                                std::string text = feadback.Msg;
                                                //text.erase(0, 1);
                                                show.ATStagetext = QString::fromStdString(text);
                                                show.progressstylestr = RUNNINGSTYLE;
                                                //引擎突然不见了，

                                                if (enginstatus == -1)
                                                {
                                                    if (AI3D::CORE::BlockObject::isChineseVersion())
                                                    {
                                                        show.ATStatustext = "当前无引擎运行，请启动引擎";
                                                    }
                                                    else
                                                    {
                                                        show.ATStatustext = "No engine running.Start an engine.";
                                                    }
                                                    show.ATStatustextstylestr = "color: #c80000;";

                                                    ///jobstatusnow = jobsta_e::STATUS_CANCLE;
                                                    ///feadback.Status = jobstatusnow;

                                                    LOGI("No engine running.Start an engine.");
                                                    std::cout << "No engine running.Start an engine." << std::endl;

                                                    /*
                                                    feadback.save_with_retry(feedbackfile);

                                                    int taskid = attimesum.GetLastRunningTaskId();
                                                    if (taskid != -1)
                                                    {
                                                        attimesum.tasksmap.at(taskid).Status = jobstatusnow;
                                                        attimesum.save(timefile);
                                                    }
                                                    auto jobfile = Settings::getMasterJobQueue().toStdString() + "/" + JOBRUNNINGSTR + "/" + taskinfo.job_ + JSONFILE_POSTFIX;
                                                    if (boost::filesystem::exists(jobfile))
                                                    {
                                                        LOGI("remove jobfile:" + jobfile);
                                                        QFile(QString::fromStdString(jobfile)).remove();
                                                    }
                                                    auto lockfile = jobfile + LOCKFILE_POSTFIX;
                                                    if (boost::filesystem::exists(lockfile))
                                                    {
                                                        LOGI("remove lockfile:" + lockfile);
                                                        QFile(QString::fromStdString(lockfile)).remove();
                                                    }*/

                                                    ///                 show.ATStatustext = QString::fromStdString(feadback.Msg + " " + blk_status_str.at(jobstatusnow));
                                                    std::string text = feadback.Msg;
                                                    //text.erase(0, 1);
                                                    if (AI3D::CORE::BlockObject::isChineseVersion())
                                                    {
                                                        show.ATStatustext = QString::fromStdString(text + ".当前无引擎运行，请启动引擎.");
                                                    }
                                                    else
                                                    {
                                                        show.ATStatustext = QString::fromStdString(text + ".No engine running.Start an engine.");
                                                    }
                                                }

                                            }
                                            else if (jobstatusnow == jobsta_e::STATUS_FAILURE)
                                            {
                                                show.progressstylestr = FAILURESTYLE;
                                                if (AI3D::CORE::BlockObject::isChineseVersion())
                                                {
                                                    show.ATStagetext = str2qstr(blk_status_str_chinese.at(jobstatusnow));
                                                }
                                                else
                                                {
                                                    show.ATStagetext = QString::fromStdString(blk_status_str.at(jobstatusnow));
                                                }
                                                std::string text = feadback.Msg;
                                                //text.erase(0, 1);
                                                show.ATStatustext = QString::fromStdString(text);
                                                show.ATStatustextstylestr = "color: #c80000;";

                                                MasterInfo::Getinstance().GetAPPUseInfosMutual().rbegin()->AtJobPercent[taskinfo.job_] = (int)feadback.Percent;

                                                ///LOGI("engine:job status now:failure.");
            ///                                 std::cout << "engine:job status now:failure." << std::endl;
                                            }
                                            else if (jobstatusnow == jobsta_e::STATUS_CANCLE)
                                            {
                                                show.progressstylestr = CANCELSTYLE;
                                                if (AI3D::CORE::BlockObject::isChineseVersion())
                                                {
                                                    show.ATStagetext = str2qstr(blk_status_str_chinese.at(jobstatusnow));
                                                }
                                                else
                                                {
                                                    show.ATStagetext = QString::fromStdString(blk_status_str.at(jobstatusnow));
                                                }
                                                std::string text = feadback.Msg;
                                                //text.erase(0, 1);
                                                show.ATStatustext = QString::fromStdString(text);
                                                show.ATStatustextstylestr = "color: #F7BA0B;";// "color: #aa0000;";

                                            

                                                MasterInfo::Getinstance().GetAPPUseInfosMutual().rbegin()->AtJobPercent[taskinfo.job_] = (int)feadback.Percent;

                                                ///LOGI("engine:job status now:cancel.");
                                                ///std::cout << "engine:job status now:cancel." << std::endl;

                                            }
                                            //此处需要沟通采用什么形式
                                            else if ((jobstatusnow == jobsta_e::STATUS_COMPLETE) /*&& (feadback.Msg == GetTaskEndingString(taskinfo.job_))*/)
                                            {
                                                /// LOGI("engine:job status now:complete.");
                                                ///std::cout << "engine:job status now:complete." << std::endl;

                                                MasterInfo::Getinstance().GetAPPUseInfosMutual().rbegin()->AtJobPercent[taskinfo.job_] = 100;

                                                //此处因空三需要单独处理所以需要分开
                                                if ((GetJobType(taskinfo.job_) == JOB_AT))
                                                {

                                                    block_data_->SetStatus(jobsta_e::STATUS_COMPLETE);
                                                    if (UpdateCompleteJobATFile() != AI3D_SUCCESS)
                                                    {
                                                        jobstatusnow = jobsta_e::STATUS_FAILURE;
                                                        if (AI3D::CORE::BlockObject::isChineseVersion())
                                                        {
                                                            show.ATStagetext = str2qstr(blk_status_str_chinese.at(jobstatusnow));
                                                        }
                                                        else
                                                        {
                                                            show.ATStagetext = QString::fromStdString(blk_status_str.at(jobstatusnow));
                                                        }
                                                        std::string text = feadback.Msg;
                                                        //text.erase(0, 1);
                                                        show.ATStatustext = QString::fromStdString(text);
                                                        show.ATStatustextstylestr = "color: #c80000;";
                                                        feadback.Status = jobstatusnow;

                                                        feadback.save_with_retry(feedbackfile);
                                                    }
                                                    else
                                                    {
                                                        show.progressstylestr = RUNNINGSTYLE;//因其原始颜色偏深，running的偏亮
                                                        if(AI3D::CORE::BlockObject::isChineseVersion())
                                                        { 
                                                            show.ATStagetext = str2qstr(blk_status_str_chinese.at(jobstatusnow));
                                                        }
                                                        else
                                                        {
                                                            show.ATStagetext = QString::fromStdString(blk_status_str.at(jobstatusnow));
                                                        }
                                                        //加入是否是relative的判断
                                                        auto definition = block_data_->GetCurrentAT()->GetLocalSrs();
                                                        if (block_data_->GetCurrentAT()->HasRegImages())
                                                        {

                                                            std::string relatstr, absstr;
                                                            if (BlockObject::isChineseVersion())
                                                            {
                                                                relatstr = "相对坐标系";
                                                                absstr = "绝对坐标系";
                                                            }
                                                            else
                                                            {
                                                                relatstr = "relative";
                                                                absstr = "absolute";
                                                            }
                                                            std::string modestr = (CoordinateDescriptor::GetSRSFromDefinition(definition).type == LOCAL) ? relatstr : absstr;
                                                            if (AI3D::CORE::BlockObject::isChineseVersion())
                                                            {
                                                                modestr = "位姿状态: " + modestr;
                                                            }
                                                            else
                                                            {
                                                                modestr = "Photo positioning level: " + modestr;
                                                            }
                                                            show.ATStatustext = QString::fromStdString(modestr);


                                                        }
                                                        
                                                    }
                                                }
                                                else if ((GetJobType(taskinfo.job_) == JOB_BATCH))
                                                {

                                                    //如果是batch处理完了则需要更改block的状态为new然后更改blockitem的颜色表示该block是通过批处理的方式
                                                    //将结果呈现到各个页卡中类似于到了一次xml文件
                                                    jobstatusnow = jobsta_e::STATUS_NEW;
                                                    block_data_->SetStatus(jobsta_e::STATUS_NEW);
                                                    bool ret = false;
                                                    //加载成功后batch置灰
                                                    std::string block_path = "";
                                                    if (BLK_USE_BIN) {
                                                        block_path = block_data_->GetPath() + PATH_SEPARATOR + block_data_->GetName() + BLOCKBINFILE;
                                                        //block_path = block_data_->GetPath() + PATH_SEPARATOR + BLKFILENAME + BLOCKBINFILE;
                                                    }
                                                    else {
                                                        block_path = block_data_->GetPath() + PATH_SEPARATOR + block_data_->GetName() + BLOCKFILE;
                                                        //block_path = block_data_->GetPath() + PATH_SEPARATOR + BLKFILENAME + BLOCKFILE;
                                                    }
                                                    ret = block_data_->Load(block_path, true);
                                                    block_data_->GetTaskInfoMutual().job_ = "";
                                                    if (BLK_USE_BIN) {
                                                        block_data_->GetTaskInfoMutual().WriteBlockInfoToBin(block_path, false);
                                                    }
                                                    else {
                                                        block_data_->GetTaskInfoMutual().WriteBlockInfoToJson(block_path, false);
                                                    }
                                                    
                                                    //show.progressstylestr = RUNNINGSTYLE;//因其原始颜色偏深，running的偏亮
                                                    //show.ATStagetext = QString::fromStdString(blk_status_str.at(jobstatusnow));
                                                    //同时删除feadback文件和time文件，并将AT页卡关闭，其余页卡跟导入xml一样的状态；

                                                    ShowATTab(false);
                                                    std::vector<int> tabvec;
                                                    ProjectManager* promanager = ProjectManager::GetInstance();
                                                    promanager->GetBlockManaget(block_data_->GetId())->ChangeTab(ExistsTab(ATTAB), block_data_->GetCurrentAT()->HasImages(), \
                                                        block_data_->GetCurrentAT()->HasControlPoints(), tabvec);
                                                    BlockManager* blockmanager = promanager->GetBlockManaget(block_data_->GetId());

                                                    auto blockstatus = blockmanager->GetBlockStatusMutual();

                                                    SetWgtStatus(blockstatus);
                                                    UpdateTabPaper(tabvec);
                                                    PopulatePhotoGroupTable();
                                                    if (current_tableview_index.isValid())
                                                    {
                                                        Slot_TableView_Clicked(current_tableview_index);
                                                    }
                                                    if (GetRunningInfoTime->isActive())
                                                        GetRunningInfoTime->stop();
                                                }

                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                            if (block_data_->GetStatusMutual() == jobsta_e::STATUS_COMPLETE)
                            {
                                
                                show.status = jobsta_e::STATUS_COMPLETE;
                                show.progressstylestr = RUNNINGSTYLE;//因其原始颜色偏深，running的偏亮
                                if (AI3D::CORE::BlockObject::isChineseVersion())
                                {
                                    show.ATStagetext = str2qstr(blk_status_str_chinese.at(jobsta_e::STATUS_COMPLETE));
                                }
                                else
                                {
                                    show.ATStagetext = QString::fromStdString(blk_status_str.at(jobsta_e::STATUS_COMPLETE));
                                }
                                show.progreesvalue = 100.0;
                                //加入是否是relative的判断
                                jobstatusnow = show.status;
                                auto definition = block_data_->GetCurrentAT()->GetLocalSrs();
                                if (block_data_->GetCurrentAT()->HasRegImages())
                                {
                                    std::string relatstr, absstr;
                                    if (BlockObject::isChineseVersion())
                                    {
                                        relatstr = "相对坐标系";
                                        absstr = "绝对坐标系";
                                    }
                                    else
                                    {
                                        relatstr = "relative";
                                        absstr = "absolute";
                                    }
                                    std::string modestr = (CoordinateDescriptor::GetSRSFromDefinition(definition).type == LOCAL) ? relatstr : absstr;
                                    
                                    if (AI3D::CORE::BlockObject::isChineseVersion())
                                    {
                                        modestr = "位姿状态: " + modestr;
                                    }
                                    else
                                    {
                                        modestr = "Photo positioning level: " + modestr;
                                    }
                                    show.ATStatustext = QString::fromStdString(modestr);


                                }
                            }
 }
                        }
                        catch (const std::filesystem::filesystem_error& fse)
                        {
                            std::ostringstream oss;
                            oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1().string() << " " << fse.path2().string();
                            LOGI(oss.str());
                        }
                        catch (std::exception& ex)
                        {
                            std::ostringstream oss;
                            oss << "exception:" << ex.what();
                            LOGI(oss.str());
                        }

                        show.status = jobstatusnow;
                        block_data_->SetStatus(jobsta_e(show.status));
                        ProjectManager* manager = ProjectManager::GetInstance();
                        if (manager->GetBlockManagers().count(block_data_->GetId()))
                        {

                            Block_Status_s& BlockStatus = manager->GetBlockManaget(block_data_->GetId())->GetBlockStatusMutual();
                            SetWgtStatus(BlockStatus);

                            UpdateATTabLabel(show);
                        }
                        else
                        {
                            bGotNewJobInfo = true;
                            bGettingJobInfo = false;
                            return;
                        }
                    }

                    ui->label_7->setVisible(true);
                    ui->label_blockID->setVisible(true);
                    std::string blockidstr = BLOCK_PRE + std::to_string(block_data_->GetId());;
                    ui->label_blockID->setText(QString(blockidstr.c_str()));

                    if (block_data_->GetTaskInfoMutual().isFinished && !atreportloaded)
                    {
                        /*std::string atreportfile = block_data_->GetPath() + "/AtReport.html";*/
                    /*  if (boost::filesystem::exists(atreportfile))*/
                        {
                            ui->label_view_report->setVisible(true);
                            QString urldisplay = "view ATreport";
                            QString urlstyle = "<a href=www.baidu.com style=\"color:blue;text-decotation:none;\">";
                            if (AI3D::CORE::BlockObject::isChineseVersion())
                            {
///                             ui->label_view_report->setText(QApplication::translate("label_view_report","<html><head/><body><p><a href=\" \"><span style=\" text-decoration: underline; color:#33CCFF;\">查看空三报告</span></a></p></body></html>",nullptr));
                                ui->label_view_report->setText(BlockWgt::getChineseString("label_view_report","<html><head/><body><p><a href=\" \"><span style=\" text-decoration: underline; color:#33CCFF;\">查看空三报告</span></a></p></body></html>"));
                            }
                            else
                            {
                                ui->label_view_report->setText(QString(QStringLiteral("<html><head/><body><p><a href=\" \"><span style=\" text-decoration: underline; color:#33CCFF;\">View AT report</span></a></p></body></html>")));
                            }
                            /*  ui->label_view_report->setText(QString("%1%2").arg(urlstyle).arg(urldisplay));*/
                            atreportloaded = true;
                        }
                    }
                }//== new
                else //
                {
                    //如果是complete类型删除后变为new则相应的atdata应该也变为group[0]的状态
                    if (!block_data_->GetCurrentAT()->HasImages())
                    {
                        if (AI3D::CORE::BlockObject::isChineseVersion())
                        {
                            show.ATStagetext = "空区块";
                        }
                        else
                        {
                            show.ATStagetext = "Empty block";
                        }
                    }
                    else
                    {
                        int numimages = block_data_->GetCurrentAT()->GetNumImages();
                        show.ATStagetext = QString::fromStdString(std::to_string(numimages));
                    }

                    show.status = jobsta_e::STATUS_NEW;//仅改变展示的状态
                        /*ProjectManager* manager = ProjectManager::GetInstance();
                        Block_Status_s& BlockStatus = manager->GetBlockManaget(block_data_->GetId())->GetBlockStatusMutual();
                        SetWgtStatus(BlockStatus);*/

                    UpdateATTabLabel(show);

                    ui->label_view_report->setVisible(false);
                    ui->taskList->setVisible(false);


                }
            }
            bGotNewJobInfo = true;
            bGettingJobInfo = false;
            //std::cout << __FUNCTION__ << " ======================= " << bGotNewJobInfo << block_data_->GetId() << std::endl;
        }

        void BlockWgt::GetRealTimeInfoV2()
        {
            RefreshRightSideKxPxEditable();

            if (bGettingJobInfo && !bGotNewJobInfo)
            {
                std::cout << " ------------- " << bGotNewJobInfo << " " << __LINE__ << bGotNewJobInfo << block_data_->GetId() << std::endl;

                return;
            }
            bGettingJobInfo = true;
            //读取time_json
            //是否存在time

            BlockObject::Task_Info taskinfo;

            if (!BlockObject::isValidBlockObject(block_data_))
                return;

            try
            {
                taskinfo = block_data_->GetTaskInfo();
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "GetRealTimeInfo/exception:" << ex.what();
                LOGI(oss.str());
                return;
            }

            //std::string timefile = block_data_->GetPath() + "/" + TIME_PREFIX + taskinfo.job_ + JSONFILE_POSTFIX;
            //std::string feedbackfile = MAKE_FEEDBAK_FILE(block_data_->GetPath(), taskinfo.job_);// block_data_->GetPath() + "/" + FEEDBACK_PREFIX + taskinfo.job_ + JSONFILE_POSTFIX;
            std::string feedbackfile = "";
            std::string timefile = "";
            if (JOB_FEEDBACK_USE_BIN) {
                feedbackfile = MAKE_FEEDBAK_BIN_FILE(block_data_->GetPath(), taskinfo.job_);
                timefile = MAKE_TIMESUM_BIN_FILE(block_data_->GetPath(), taskinfo.job_);
            }
            else {
                feedbackfile = MAKE_FEEDBAK_JSON_FILE(block_data_->GetPath(), taskinfo.job_);
                timefile = MAKE_TIMESUM_BIN_FILE(block_data_->GetPath(), taskinfo.job_);
                //feedbackName = blockitembase_path + FEEDBACK_PREFIX + jobstring + JSONFILE_POSTFIX;
            }
            //如果是new状态的话，就不会有AT页卡所以直接退出


            //
            InfoForShow_s show;
            jobsta_e jobstatusnow = jobsta_e::STATUS_UNKNOWN;
            int enginstatus = UpdateEngineStatus();
            bool bstatusknown = false;

            /*if (TaskProcess::GetJobType(taskinfo.job_) == JOB_AT)*/
            {

                //如果是New的状态的话是不读feedback的，但是在complete完毕后如果做全部删除操作则会引发new状态的产生
                if (block_data_->GetStatusMutual() != jobsta_e::STATUS_NEW)
                {

                    {
                        try
                        {
                            if (std::filesystem::exists(File::BoostPathFromUtf8(feedbackfile)) && std::filesystem::exists(File::BoostPathFromUtf8(timefile)))
                            {
                                JobFeedBack_s feadback; ATTimeSummary_s attimesum;
                                LOGI("11==========feedbackpath===== " + feedbackfile);
                                if (feadback.load_with_retry2(feedbackfile) && (attimesum.load(timefile)))
                                {

                                    jobstatusnow = feadback.Status;
                                    bstatusknown = (jobstatusnow == jobsta_e::STATUS_UNKNOWN) ? false : true;
                                    //如果是pending，分两种情况1：有engine，2：无引擎，两者唯一的差别就是一个有提示一个没提示
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

                                            if (AI3D::CORE::BlockObject::isChineseVersion())
                                            {
                                                show.ATStagetext = str2qstr(blk_status_str_chinese.at(jobstatusnow));
                                            }
                                            else
                                            {
                                                show.ATStagetext = QString::fromStdString(blk_status_str.at(jobstatusnow));
                                            }
                                            if (enginstatus == -1)
                                            {
                                                if (AI3D::CORE::BlockObject::isChineseVersion())
                                                {
                                                    show.ATStatustext = "当前无引擎运行，请启动引擎.";
                                                }
                                                else
                                                {
                                                    show.ATStatustext = "No engine running.Start an engine.";
                                                }
                                                show.ATStatustextstylestr = "color: #c80000;";
                                                LOGI("No engine running.Start an engine.");
                                                //std::cout << "No engine running.Start an engine." << std::endl;
                                            }
                                            else if (enginstatus == 0)
                                            {
                                                if (AI3D::CORE::BlockObject::isChineseVersion())
                                                {
                                                    show.ATStatustext = "等待处理中.";
                                                }
                                                else
                                                {
                                                    show.ATStatustext = "Waiting to be processed.";
                                                }
                                                //std::cout << "Waiting to be processed." << std::endl;
                                            }
                                            show.progreesvalue = 0;
                                            {
                                                ui->taskList->setShowGrid(false);
                                                ui->taskList->verticalHeader()->setHidden(true);
                                                ui->taskList->horizontalHeader()->setHidden(true);
                                                ui->taskList->setStyleSheet("QTableWidget{background:#000000;border:none;color:#FFFFFF;}");



                                                int tasklen = ui->taskList->rowCount();
                                                for (int i = tasklen - 1; i >= 0; i--)
                                                {
                                                    ui->taskList->removeRow(i);
                                                }
                                            }

                                        }
                                        //运行时 stage为feadback的msg，status不显示，cc是在status的位置显示进度条
                                        else
                                        {
                                            /*jobStats.rtnTotalTime = getTotalTime(attimesum.GetStartTime(), attimesum.GetLastTime());*/
                                            show.progreesvalue = feadback.Percent;

                                            QVector<JobStage> vec_job;
                                            if (GetJobStagesStatus(feadback, attimesum, vec_job) > 0)
                                            {
                                                ///                                         std::cout << __FILE__ << __FUNCTION__ << __LINE__ << "GetJobStagesStatus > 0"  << std::endl;
                                                UpdateJobStageLists(vec_job);

                                                ///QVector<JobStage> tempvecStates;
                                            }
                                            else
                                            {
                                                ///std::cout << __FILE__ << __FUNCTION__ << __LINE__ << "GetJobStagesStatus <= 0" << std::endl;
                                            }

                                            if (jobstatusnow == jobsta_e::STATUS_RUNNING)
                                            {
                                                std::string text = feadback.Msg;
                                                if (AI3D::CORE::BlockObject::isChineseVersion())
                                                {
                                                    /*if(text == )*/
                                                }
                                                //text.erase(0, 1);
                                                show.ATStagetext = QString::fromStdString(text);
                                                show.progressstylestr = RUNNINGSTYLE;
                                                //引擎突然不见了，

                                                if (enginstatus == -1)
                                                {
                                                    if (AI3D::CORE::BlockObject::isChineseVersion())
                                                    {
                                                        show.ATStatustext = "当前无引擎运行，请启动引擎.";
                                                    }
                                                    else
                                                    {
                                                        show.ATStatustext = "No engine running.Start an engine.";
                                                    }
                                                    show.ATStatustextstylestr = "color: #c80000;";

                                                    ///jobstatusnow = jobsta_e::STATUS_CANCLE;
                                                    ///feadback.Status = jobstatusnow;

                                                    LOGI("No engine running.Start an engine.");
                                                    std::cout << "No engine running.Start an engine." << std::endl;

                                                    /*
                                                    feadback.save_with_retry(feedbackfile);

                                                    int taskid = attimesum.GetLastRunningTaskId();
                                                    if (taskid != -1)
                                                    {
                                                        attimesum.tasksmap.at(taskid).Status = jobstatusnow;
                                                        attimesum.save(timefile);
                                                    }
                                                    auto jobfile = Settings::getMasterJobQueue().toStdString() + "/" + JOBRUNNINGSTR + "/" + taskinfo.job_ + JSONFILE_POSTFIX;
                                                    if (boost::filesystem::exists(jobfile))
                                                    {
                                                        LOGI("remove jobfile:" + jobfile);
                                                        QFile(QString::fromStdString(jobfile)).remove();
                                                    }
                                                    auto lockfile = jobfile + LOCKFILE_POSTFIX;
                                                    if (boost::filesystem::exists(lockfile))
                                                    {
                                                        LOGI("remove lockfile:" + lockfile);
                                                        QFile(QString::fromStdString(lockfile)).remove();
                                                    }*/

                                                    ///                 show.ATStatustext = QString::fromStdString(feadback.Msg + " " + blk_status_str.at(jobstatusnow));
                                                    std::string text = feadback.Msg;
                                                    //text.erase(0, 1);
                                                    if(AI3D::CORE::BlockObject::isChineseVersion())
                                                    {
                                                        
                                                        show.ATStatustext = QString::fromStdString(text + ".目前无引擎运行，请启动引擎.");
                                                    }
                                                    else
                                                    {
                                                        show.ATStatustext = QString::fromStdString(text + ".No engine running.Start an engine.");
                                                    }
                                                }

                                            }
                                            else if (jobstatusnow == jobsta_e::STATUS_FAILURE)
                                            {
                                                show.progressstylestr = FAILURESTYLE;
                                                if (AI3D::CORE::BlockObject::isChineseVersion())
                                                {
                                                    show.ATStagetext = str2qstr(blk_status_str_chinese.at(jobstatusnow));
                                                }
                                                else
                                                {
                                                    show.ATStagetext = QString::fromStdString(blk_status_str.at(jobstatusnow));
                                                }
                                                std::string text = feadback.Msg;
                                                //text.erase(0, 1);
                                                show.ATStatustext = QString::fromStdString(text);
                                                show.ATStatustextstylestr = "color: #c80000;";

                                                MasterInfo::Getinstance().GetAPPUseInfosMutual().rbegin()->AtJobPercent[taskinfo.job_] = (int)feadback.Percent;

                                                ///LOGI("engine:job status now:failure.");
            ///                                 std::cout << "engine:job status now:failure." << std::endl;
                                            }
                                            else if (jobstatusnow == jobsta_e::STATUS_CANCLE)
                                            {
                                                show.progressstylestr = CANCELSTYLE;
                                                if (AI3D::CORE::BlockObject::isChineseVersion())
                                                {
///                                                 show.ATStagetext = QString::fromStdString(blk_status_str_chinese.at(jobstatusnow));
                                                    show.ATStagetext = str2qstr(blk_status_str_chinese.at(jobstatusnow));
                                                }
                                                else
                                                {
                                                    show.ATStagetext = QString::fromStdString(blk_status_str.at(jobstatusnow));
                                                }
                                                std::string text = feadback.Msg;
                                                //text.erase(0, 1);
                                                show.ATStatustext = QString::fromStdString(text);
                                                show.ATStatustextstylestr = "color: #F7BA0B;";// "color: #aa0000;";



                                                MasterInfo::Getinstance().GetAPPUseInfosMutual().rbegin()->AtJobPercent[taskinfo.job_] = (int)feadback.Percent;

                                                ///LOGI("engine:job status now:cancel.");
                                                ///std::cout << "engine:job status now:cancel." << std::endl;

                                            }
                                            //此处需要沟通采用什么形式
                                            else if ((jobstatusnow == jobsta_e::STATUS_COMPLETE)/* && (feadback.Msg == GetTaskEndingString(taskinfo.job_))*/)
                                            {
                                                /// LOGI("engine:job status now:complete.");
                                                ///std::cout << "engine:job status now:complete." << std::endl;

                                                MasterInfo::Getinstance().GetAPPUseInfosMutual().rbegin()->AtJobPercent[taskinfo.job_] = 100;

                                                //此处因空三需要单独处理所以需要分开
                                                if ((GetJobType(taskinfo.job_) == JOB_AT))
                                                {

                                                    block_data_->SetStatus(jobsta_e::STATUS_COMPLETE);
                                                    if (UpdateCompleteJobATFile() != AI3D_SUCCESS)
                                                    {
                                                        jobstatusnow = jobsta_e::STATUS_FAILURE;
                                                        if (AI3D::CORE::BlockObject::isChineseVersion())
                                                        {
///                                                         show.ATStagetext = QString::fromStdString(blk_status_str_chinese.at(jobstatusnow));
                                                            show.ATStagetext = str2qstr(blk_status_str_chinese.at(jobstatusnow));
                                                        }
                                                        else
                                                        {
                                                            show.ATStagetext = QString::fromStdString(blk_status_str.at(jobstatusnow));
                                                        }
                                                        std::string text = feadback.Msg;
                                                        //text.erase(0, 1);
                                                        show.ATStatustext = QString::fromStdString(text);
                                                        
                                                        show.ATStatustextstylestr = "color: #c80000;";
                                                        feadback.Status = jobstatusnow;

                                                        feadback.save_with_retry(feedbackfile);
                                                    }
                                                    else
                                                    {
                                                        show.progressstylestr = RUNNINGSTYLE;//因其原始颜色偏深，running的偏亮
                                                        if (AI3D::CORE::BlockObject::isChineseVersion())
                                                        {
                                                            ///show.ATStagetext = QString::fromStdString(blk_status_str_chinese.at(jobstatusnow));
                                                            show.ATStagetext = str2qstr(blk_status_str_chinese.at(jobstatusnow));
                                                        }
                                                        else
                                                        { 
                                                            show.ATStagetext = QString::fromStdString(blk_status_str.at(jobstatusnow));
                                                        }
                                                        //加入是否是relative的判断
                                                        auto definition = block_data_->GetCurrentAT()->GetLocalSrs();
                                                        if (block_data_->GetCurrentAT()->HasRegImages())
                                                        {

                                                            std::string relatstr, absstr;
                                                            if (BlockObject::isChineseVersion())
                                                            {
                                                                relatstr = "相对坐标系";
                                                                absstr = "绝对坐标系";
                                                            }
                                                            else
                                                            {
                                                                relatstr = "relative";
                                                                absstr = "absolute";
                                                            }
                                                            std::string modestr = (CoordinateDescriptor::GetSRSFromDefinition(definition).type == LOCAL) ? relatstr : absstr;
                                                            if (AI3D::CORE::BlockObject::isChineseVersion())
                                                            {
                                                                modestr = "位姿状态: " + modestr;
                                                            }
                                                            else
                                                            {
                                                                modestr = "Photo postioning level: " + modestr;
                                                            }
                                                            show.ATStatustext = QString::fromStdString(modestr);


                                                        }

                                                    }
                                                }
                                                else if ((GetJobType(taskinfo.job_) == JOB_BATCH))
                                                {

                                                    //如果是batch处理完了则需要更改block的状态为new然后更改blockitem的颜色表示该block是通过批处理的方式
                                                    //将结果呈现到各个页卡中类似于到了一次xml文件
                                                    jobstatusnow = jobsta_e::STATUS_NEW;
                                                    block_data_->SetStatus(jobsta_e::STATUS_NEW);
                                                    bool ret = false;
                                                    //加载成功后batch置灰
                                                    //std::string block_path = block_data_->GetPath() + PATH_SEPARATOR + block_data_->GetName() + BLOCKFILE;
                                                    std::string block_path = "";
                                                    if (BLK_USE_BIN) {
                                                        block_path = block_data_->GetPath() + block_data_->GetName() + BLOCKBINFILE;
                                                        //block_path = block_data_->GetPath() + BLKFILENAME + BLOCKBINFILE;
                                                    }
                                                    else {
                                                        block_path = block_data_->GetPath() + block_data_->GetName() + BLOCKFILE;
                                                        //block_path = block_data_->GetPath() + BLKFILENAME + BLOCKFILE;
                                                    }
                                                    //std::string block_path = block_data_->GetPath() + block_data_->GetName() + BLOCKFILE;
                                                    LOGI(std::string("block_path 1==========" + block_path));
                                                    ret = block_data_->Load(block_path, true);
                                                    block_data_->GetTaskInfoMutual().job_ = "";
                                                    if (BLK_USE_BIN) {
                                                        block_data_->GetTaskInfoMutual().WriteBlockInfoToBin(block_path, false);
                                                    }
                                                    else {
                                                        block_data_->GetTaskInfoMutual().WriteBlockInfoToJson(block_path, false);
                                                    }
                                                    
                                                    //show.progressstylestr = RUNNINGSTYLE;//因其原始颜色偏深，running的偏亮
                                                    //show.ATStagetext = QString::fromStdString(blk_status_str.at(jobstatusnow));
                                                    //同时删除feadback文件和time文件，并将AT页卡关闭，其余页卡跟导入xml一样的状态；

                                                    ShowATTab(false);
                                                    std::vector<int> tabvec;
                                                    ProjectManager* promanager = ProjectManager::GetInstance();
                                                    promanager->GetBlockManaget(block_data_->GetId())->ChangeTab(ExistsTab(ATTAB), block_data_->GetCurrentAT()->HasImages(), \
                                                        block_data_->GetCurrentAT()->HasControlPoints(), tabvec);
                                                    BlockManager* blockmanager = promanager->GetBlockManaget(block_data_->GetId());

                                                    auto blockstatus = blockmanager->GetBlockStatusMutual();

                                                    SetWgtStatus(blockstatus);
                                                    UpdateTabPaper(tabvec);
                                                    PopulatePhotoGroupTable();
                                                    if (current_tableview_index.isValid())
                                                    {
                                                        Slot_TableView_Clicked(current_tableview_index);
                                                    }
                                                    if (GetRunningInfoTime->isActive())
                                                        GetRunningInfoTime->stop();
                                                }

                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (block_data_->GetStatusMutual() == jobsta_e::STATUS_COMPLETE)
                                {

                                    show.status = jobsta_e::STATUS_COMPLETE;
                                    show.progressstylestr = RUNNINGSTYLE;//因其原始颜色偏深，running的偏亮
                                    if (AI3D::CORE::BlockObject::isChineseVersion())
                                    {
                                        ///show.ATStagetext = QString::fromStdString(blk_status_str_chinese.at(jobsta_e::STATUS_COMPLETE));
                                        show.ATStagetext = str2qstr(blk_status_str_chinese.at(jobsta_e::STATUS_COMPLETE));
                                    }
                                    else
                                    {
                                        show.ATStagetext = QString::fromStdString(blk_status_str.at(jobsta_e::STATUS_COMPLETE));
                                    }
                                    show.progreesvalue = 100.0;
                                    //加入是否是relative的判断
                                    jobstatusnow = show.status;
                                    auto definition = block_data_->GetCurrentAT()->GetLocalSrs();
                                    if (block_data_->GetCurrentAT()->HasRegImages())
                                    {

                                        std::string relatstr, absstr;
                                        if (BlockObject::isChineseVersion())
                                        {
                                            relatstr = "相对坐标系";
                                            absstr = "绝对坐标系";
                                        }
                                        else
                                        {
                                            relatstr = "relative";
                                            absstr = "absolute";
                                        }
                                        std::string modestr = (CoordinateDescriptor::GetSRSFromDefinition(definition).type == LOCAL) ? relatstr : absstr;

                                        if (AI3D::CORE::BlockObject::isChineseVersion())
                                        {
                                            modestr = "位姿状态: " + modestr;
                                        }
                                        else
                                        {
                                            modestr = "Photo positioning level: " + modestr;
                                        }
                                        show.ATStatustext = QString::fromStdString(modestr);


                                    }
                                }
                            }
                        }
                        catch (const std::filesystem::filesystem_error& fse)
                        {
                            std::ostringstream oss;
                            oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1().string() << " " << fse.path2().string();
                            LOGI(oss.str());
                        }
                        catch (std::exception& ex)
                        {
                            std::ostringstream oss;
                            oss << "exception:" << ex.what();
                            LOGI(oss.str());
                        }

                        show.status = jobstatusnow;
                        block_data_->SetStatus(jobsta_e(show.status));
                        ProjectManager* manager = ProjectManager::GetInstance();
                        if (manager->GetBlockManagers().count(block_data_->GetId()))
                        {

                            
                            Block_Status_s& BlockStatus = manager->GetBlockManaget(block_data_->GetId())->GetBlockStatusMutual();
                            SetWgtStatus(BlockStatus);

                            UpdateATTabLabel(show);
                        }
                        else
                        {
                            bGotNewJobInfo = true;
                            bGettingJobInfo = false;
                            return;
                        }
                    }

                    ui->label_7->setVisible(true);
                    ui->label_blockID->setVisible(true);
                    std::string blockidstr = BLOCK_PRE + std::to_string(block_data_->GetId());;
                    ui->label_blockID->setText(QString(blockidstr.c_str()));

                    if (block_data_->GetTaskInfoMutual().isFinished && !atreportloaded)
                    {
                        /*std::string atreportfile = block_data_->GetPath() + "/AtReport.html";*/
                    /*  if (boost::filesystem::exists(atreportfile))*/
                        {
                            ui->label_view_report->setVisible(true);
                            QString urldisplay = "view ATreport";
                            QString urlstyle = "<a href=www.baidu.com style=\"color:blue;text=decotation:none;\">";
                            if (AI3D::CORE::BlockObject::isChineseVersion())
                            {
                                QString linkName = str2qstr(std::string("查看空三报告"));
///                             ui->label_view_report->setText(QApplication::translate("label_view_report","<html><head/><body><p><a href=\" \"><span style=\" text-decoration: underline; color:#33CCFF;\">查看空三报告</span></a></p></body></html>",nullptr));
                                ui->label_view_report->setText(BlockWgt::getChineseString("label_view_report","<html><head/><body><p><a href=\" \"><span style=\" text-decoration: underline; color:#33CCFF;\">查看空三报告</span></a></p></body></html>"));
                            }
                            else
                            {
                                ui->label_view_report->setText(QString(QStringLiteral("<html><head/><body><p><a href=\" \"><span style=\" text-decoration: underline; color:#33CCFF;\">View AT report</span></a></p></body></html>")));
                            }
                            /*  ui->label_view_report->setText(QString("%1%2").arg(urlstyle).arg(urldisplay));*/
                            atreportloaded = true;
                        }
                    }
                }//== new
                else //
                {
                    //如果是complete类型删除后变为new则相应的atdata应该也变为group[0]的状态
                    if (!block_data_->GetCurrentAT()->HasImages())
                    {

                        show.ATStagetext = "Empty block";
                    }
                    else
                    {
                        int numimages = block_data_->GetCurrentAT()->GetNumImages();
                        show.ATStagetext = QString::fromStdString(std::to_string(numimages));
                    }
                    show.status = jobsta_e::STATUS_NEW;//仅改变展示的状态
                        /*ProjectManager* manager = ProjectManager::GetInstance();
                        Block_Status_s& BlockStatus = manager->GetBlockManaget(block_data_->GetId())->GetBlockStatusMutual();
                        SetWgtStatus(BlockStatus);*/

                    UpdateATTabLabel(show);

                    ui->label_view_report->setVisible(false);
                    ui->taskList->setVisible(false);


                }
            }
            bGotNewJobInfo = true;
            bGettingJobInfo = false;
            //std::cout << __FUNCTION__ << " ======================= " << bGotNewJobInfo << block_data_->GetId() << std::endl;

           // GetRunningInfoTime->stop();  //modify by zhaobf
        }

        void BlockWgt::InitATWgt()
        {
            ui->label_Progress->setVisible(false);
            ui->progressBar_submit->setVisible(false);
            ui->label_ProgressValue->setVisible(false);
            ui->label_view_report->setVisible(false);
        }
        
        QString BlockWgt::getChineseString(const char *section, const char *text)
        {
            return QApplication::translate(section, text, nullptr);
        }
        
        //add by  chy
        void BlockWgt::slot_linkActivated_label_view_report(QString link)
        {
            ///QString blkPath = QString::fromStdString(block_data_->GetPath());
            QString blkPath = str2qstr(const_cast<std::string &>(block_data_->GetPathMutual()));

            QString dstHtmlPath = QString("%1/AtReport.html").arg(blkPath);
            QFileInfo fileinfo;
            fileinfo.setFile(dstHtmlPath);
            if (!fileinfo.exists())
            {
                QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
                if (AI3D_SUCCESS != block_data_->GenerateATReport())
                {
                    QApplication::restoreOverrideCursor();
                    LOGI("Generate AT Report failed.");
                    return;
                }
                QApplication::restoreOverrideCursor();
            }   
        
            // open html
            QUrl url = QUrl::fromLocalFile(dstHtmlPath);
            QDesktopServices::openUrl(url);
        }
        void BlockWgt::StartJobInfoTimer(bool bloaded)
        {
            
            if (!GetRunningInfoTime->isActive())
                GetRunningInfoTime->start(1000);

        }
        void BlockWgt::InitATTabConnections()
        {
            connect(ui->btn_at, &QPushButton::clicked, this, &BlockWgt::Slot_Btn_SubmitAerotri_Clicked2, Qt::QueuedConnection);
            connect(ui->btn_paus, &QPushButton::clicked, this, &BlockWgt::Slot_Btn_Cancle2, Qt::QueuedConnection);
            connect(ui->btn_rec, &QPushButton::clicked, this, &BlockWgt::Slot_Btn_Resubmit2, Qt::QueuedConnection);
            connect(ui->btn_newContruction, &QPushButton::clicked, this, &BlockWgt::Slot_Btn_SubmitReconstruct_Clicked);
        }


        

        void BlockWgt::Slot_Btn_SubmitAerotri_Clicked2()
        {
            VersionInfo versionInfo = checkSoftWareVersion();
            QString message;
            if (!versionInfo.checkReturn) {
                //请求出错
                message = "网络问题，请联网后再试";
                QMessageBox errBox;
                errBox.warning(this, "软件版本检测", message);
                return;
            }
            else if (!versionInfo.isValid) {
                //当前版本不合法
                message = "当前软件不可用，请联系软件开发商更新版本";
                QMessageBox errBox;
                errBox.warning(this, "软件版本检测", message);
                return;
            }
            //形成新的name;
            //std::string append = (block_data_->GetTaskInfo().AT_Num ? ("(" + std::to_string(block_data_->GetTaskInfo().AT_Num) + ")") : "");
            std::string newblockname = block_data_->GetTaskInfo().blockString + "-AT";
            ProjectManager* manager = ProjectManager::GetInstance();
            newblockname = manager->GetProject()->GenerateValidBlockName(newblockname);
            
            QString ATFileName = str2qstr(newblockname);

            bool tiepointstatus = (block_data_->GetTiepointStatus()) ? block_data_->GetCurrentAT()->HasTiepoints() : block_data_->GetTaskInfoMutual().statisticinfo_.tiepointnum > 0;
            bool gcpvalid = block_data_->GetCurrentAT()->GetNumValidControlPoints() >= 3 && block_data_->GetCurrentAT()->GetNumGCPElements() >= 7;
            bool posvalid = block_data_->GetCurrentAT()->HasAbsPriorPositionImages();
            tiepointstatus = tiepointstatus && block_data_->GetCurrentAT()->HasRegImages();


            ATDefinition definition(*block_data_->GetCurrentAT().get(), tiepointstatus);

            PosSigmaDia m_possigma;
            m_possigma.setName(ATFileName);
            
            m_possigma.InitEstimationPolicies(definition);
            AI3D::CORE::ATOptions& at_options = m_possigma.GetATOptionsMutual();
            
            int nGCPTotal = block_data_->GetCurrentAT()->GetNumControlPoints();
            int nMarkedGCP = block_data_->GetCurrentAT()->GetNumValidControlPoints();
            int nGCPMarkedPhotos = block_data_->GetCurrentAT()->GetNumGCPElements();

            m_possigma.setGCPResult(nGCPTotal, nMarkedGCP, nGCPMarkedPhotos);
            
            
            
            
            if (gcpvalid)
            {
                at_options.align_mode = ALIGN_WITHGCP;
                if (posvalid)
                {
                    at_options.align_mode = ALIGN_WITHGCP_POS;
                }
                else
                    at_options.align_mode = ALIGN_WITHGCP_ARBITRARY;
            }
            else if (posvalid)
            {
                at_options.align_mode = ALIGN_WITHPOS;
            }
            m_possigma.setPosModeRadioChecked(at_options.align_mode);

            

            if (m_possigma.exec() == QDialog::Accepted)
            {
                block_data_->GetTaskInfoMutual().AT_Num++;
                block_data_->GetTaskInfoMutual().isSaved = false;
                LOGI(String::StringPrintf("%s SubmitAT", block_data_->GetName().c_str()));
                /*at_options = m_possigma.GetATOptions();*/
                QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
                at_options.at_name = qstr2str(m_possigma.getName());
                
                //提交空三，加载Tiepoints
                block_data_->LoadTiepoints();
                /*if (!block_data_->GetTiepointStatus())
                {
                    if (block_data_->LoadTiepointsBinary(block_data_->GetTaskInfo().Tiepoints, block_data_->GetCurrentATMutual()))
                    {
                        block_data_->SetTiepointStatus(true);
                    }
                }*/
                //if (!SubmitAT2(at_options, jobparam_send))
                
                if (!SubmitATWithDefinition(at_options))
                {
                    LOGE("submit at failed.");
                    QApplication::restoreOverrideCursor();
                    return ;
                }

                ///MasterInfo::Getinstance().GetAPPUseInfosMutual().rbegin()->AtJobPercent[at_options.at_name] = 0;

                StartJobInfoTimer(true);
                
                QApplication::restoreOverrideCursor();
            }
            else
            {
                return;
            }
                    
            emit Signal_Submit_Block(manager->GetProject()->GetCurrentBlock());
        }

        void BlockWgt::Slot_Btn_Cancle2()
        {

            //执行取消//chy 需测试pending和running
            QString str1;
            int errornum;
            
            {
                std::ostringstream oss;     
                oss  << " doCancel.";
                LOGI(oss.str());
            }
            QString lsMasterJobQueue = Settings::getMasterJobQueue();
            std::string jobstring = lsMasterJobQueue.QString::toStdString();
            std::string feedbackfile = "";
            if (JOB_FEEDBACK_USE_BIN) {
                feedbackfile = MAKE_FEEDBAK_BIN_FILE(block_data_->GetPath(), block_data_->GetTaskInfo().job_);
            }
            else {
                feedbackfile = MAKE_FEEDBAK_JSON_FILE(block_data_->GetPath(), block_data_->GetTaskInfo().job_);
                //feedbackName = blockitembase_path + FEEDBACK_PREFIX + jobstring + JSONFILE_POSTFIX;
            }
            //std::string feadbackfile = block_data_->GetPath() + "/feedback_" + block_data_->GetTaskInfo().job_ + ".json";
            feedbackfile = File::EnsureUnifySlash(feedbackfile);
            bool flag = TaskCommandSet::DoCancelJob(jobstring, feedbackfile, block_data_->GetTaskInfo().job_, errornum);

            //bool flag = doCancelJob2(block_data_->GetPath(), block_data_->GetTaskInfo().job_, errornum);
        /*  if (!flag)
            {
                LOGW("Please wait engine cancle operation!");
            }*/
            //chy此处为啥没有界面设置相关内容

            {
                std::ostringstream oss;
                oss << __FUNCTION__ << " LINE " << __LINE__ << " doCancel.";
                LOGI(oss.str());
            }

        }

        void BlockWgt::Slot_Btn_Resubmit2()
        {
            auto& task = block_data_->GetTaskInfoMutual();
            ProjectManager* manager = ProjectManager::GetInstance();
            auto project_ptr = manager->GetProject();
            //更新按钮状态

            jobsta_e nextstatus = jobsta_e::STATUS_PENDDING;
            block_data_->SetStatus(nextstatus/*jobsta_e::STATUS_RUNNING*/);//chy mod 应该是pending 吧，测试一下
            //block_data_->GetTaskInfoMutual().status = nextstatus/*jobsta_e::STATUS_RUNNING*/;
            Block_Status_s& BlockStatus = manager->GetBlockManaget(block_data_->GetId())->GetBlockStatusMutual();



            SetWgtStatus(BlockStatus);
            std::string postFix = "";
            if (PROJECT_USE_BIN) {
                postFix = BINDOTPROJECTPOSTFIX;
            }
            else {
                postFix = DOTPROJECTPOSTFIX;
            }
            std::string projectfile = project_ptr->GetPath() + PATH_SEPARATOR_STR + project_ptr->GetName() + postFix;
            /*JobMonitor::CreateJobQueueDir(Settings::getMasterJobQueue());*/


            std::string jobpath = qstr2str(Settings::getMasterJobQueue()) + PATH_SEPARATOR_STR + "Pending" + PATH_SEPARATOR_STR + HIGHLEVEL;
            
            std::string hostName = QHostInfo::localHostName().toStdString();
            bool ret = JobMonitor::CreateDirs();
            if (!ret)
                return;
            std::string jobstr;
            if (!ATCommandSet::CreateATTaskInfo(hostName, jobpath, block_data_->GetPath(), task, jobstr))
            {
                LOGE("Submit AT CreateTask Failed");
                return;
            }
            task.job_ = jobstr;


            //重新提空三同样需要保存工程
            block_data_->GetTaskInfoMutual().isSaved = false;
            QApplication::processEvents();
            manager->GetProject()->Save(savetype_e::XML_SAVED);
            LOGI("Submit AT and Save project");
            emit Sig_SaveFinished();

            ui->progressBar_submit->setStyleSheet("QProgressBar::chunk{background-color: rgb(122, 237, 171)}");
        }


        void BlockWgt::UpdateJobStageLists(QVector<JobStage> vec_job)
        {
            int coloum = 4;
            ui->taskList->setColumnCount(coloum);

            //LOGI("inside UpdateJobStageLists");

            QStringList strList;

            QString strtaskID(tr("Task phase"));
            QString strStatus(tr("Task statistics"));
            QString strEngine(tr("Task status"));
            QString strTime(tr("Running time"));
            
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                strtaskID = "任务阶段";
                strStatus = "任务统计";
                strEngine = "任务状态";
                strTime = "运行时间";
            }

            strList << strtaskID << strStatus << strEngine << strTime;
            //set the horizontal title
            int coloumNum = 0;
            ui->taskList->horizontalHeader()->setHidden(false);
            for (auto per : strList) 
            {
                QTableWidgetItem* item = new QTableWidgetItem(per);
                ui->taskList->setHorizontalHeaderItem(coloumNum, item);
                coloumNum++;
            }

            ui->taskList->horizontalHeader()->setMinimumHeight(50);
            ui->taskList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            //set the alignment way
            ui->taskList->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);

            //初始化tablelist列表
            ui->taskList->setVisible(true);
            ui->taskList->setShowGrid(false);
            ui->taskList->verticalHeader()->setHidden(true);
            
            ui->taskList->setEditTriggers(QAbstractItemView::NoEditTriggers);

            
            ui->taskList->setStyleSheet("QTableWidget{ alternate-background-color: #282828; background-color: #3C3C3C;color: white; }"
                "QHeaderView::section{ background-color:#353535;color: #A5A5A5;border-right:1px solid #1D1D1D; }"
                "QTableWidget::item{border-right:1px solid #1D1D1D;}"
            );

            ui->taskList->setSelectionBehavior(QAbstractItemView::SelectRows);
            ui->taskList->setSelectionMode(QAbstractItemView::NoSelection);

            ui->taskList->setAlternatingRowColors(true);
            
            ui->taskList->setFocusPolicy(Qt::NoFocus);
            QVector<JobStage> tempvecStates;
            for (auto it = vec_job.begin(); it != vec_job.end(); it++)
            {

                if (it->functionName.toStdString() != StepAT_function.at(StepAT::GenTasks))
                {
                    tempvecStates.push_back(*it);
                }
            }
            
            //?chy告知用法
            ui->taskList->setUpdatesEnabled(false);

            int tasklen = ui->taskList->rowCount();
            for (int i = tasklen - 1; i >= 0; i--)
            {
                ui->taskList->removeRow(i);
            }

            if ( tempvecStates.size() > 0 )
            {
                //获取参数，初始化列表

                ui->taskList->setRowCount(tempvecStates.size());
                bool bHasGotFailedOrCancelled = false;

                for (int i = 0; i < tempvecStates.size(); i++)
                {
                    if (AI3D::CORE::BlockObject::isChineseVersion())
                    {
                        std::string funcName = tempvecStates.at(i).functionName.toStdString();
                        if (StepATFromfunctionToshow_chinese.count(funcName) > 0)
                        {
                            ///AddItemContent(i, 0, str2qstr(StepATFromfunctionToshow_chinese.at(funcName)));
                            AddItemContent(i, 0, BlockWgt::getChineseString("",StepATFromfunctionToshow_chinese.at(funcName).c_str()));
                        }
                        else
                        { 
                            AddItemContent(i, 0, tempvecStates.at(i).functionName);
                        }
                    }
                    else
                    {
                        AddItemContent(i, 0, tempvecStates.at(i).functionName);
                    }

                    AddItemContent(i, 1, QString::number(tempvecStates.at(i).completedNum) + "/" + QString::number(tempvecStates.at(i).stagedTotalNum));
                    if (tempvecStates.at(i).status == jobsta_e::STATUS_UNKNOWN)
                    {
                //      LOGI("display stage,status(unknown):" + std::to_string(tempvecStates.at(i).status) + " " + tempvecStates.at(i).functionName.toStdString());
                        AddItemContent(i, 2, "--");
                    }
                    else
                    {
                        if (AI3D::CORE::BlockObject::isChineseVersion())
                        {
///                         AddItemContent(i, 2, QString::fromStdString(blk_status_str_chinese.at(job_status_e(tempvecStates.at(i).status))));
///                         AddItemContent(i, 2, BlockWgt::getChineseString("",blk_status_str_chinese.at(job_status_e(tempvecStates.at(i).status)).c_str()));
                            AddItemContent(i, 2, 
                            //  BlockWgt::getChineseString("",blk_status_str_chinese.at(job_status_e(tempvecStates.at(i).status)).c_str())
                                    ///QString::fromStdString(blk_status_str.at(job_status_e(tempvecStates.at(i).status)))
                        ///         QString::fromStdString(blk_status_str_chinese.at(job_status_e(tempvecStates.at(i).status)))
                                    str2qstr(blk_status_str_chinese.at(job_status_e(tempvecStates.at(i).status)))
                                );
                        }
                        else
                        {
                            AddItemContent(i, 2, QString::fromStdString(blk_status_str.at(job_status_e(tempvecStates.at(i).status))));
                        }
                //      LOGI("display stage,status:" + std::to_string(tempvecStates.at(i).status) + " " + tempvecStates.at(i).functionName.toStdString() + " " 
                //          + blk_status_str.at(job_status_e(tempvecStates.at(i).status)));
                    }

                    AddItemContent(i, 3, tempvecStates.at(i).stageTotalTime);
                }


            }

            ui->taskList->setUpdatesEnabled(true);
        }

        


        void BlockWgt::AddItemContent(int row, int column, QString content)
        {
            QTableWidgetItem* item = new QTableWidgetItem(content);
            ui->taskList->setItem(row, column, item);

        }
        bool BlockWgt::SubmitATWithDefinition(AI3D::CORE::ATOptions& at_options)
        {
            ProjectManager* manager = ProjectManager::GetInstance();
            auto project_ptr = manager->GetProject();
            //拷贝blk，blk需改名需新建文件夹需更新内容；
            block_t id = block_data_->GetId();
            LOGI(String::StringPrintf("Cloning %s(%s)", block_data_->GetName().c_str(), block_data_->GetTaskInfo().blockString.c_str()));
            project_ptr->CloneBlock(id, true, "-AT");
            LOGI("Clone Block Finished!");

            BlockObject* newblock = project_ptr->GetBlock(id);
            if (at_options.align_mode == sfm_align_mode_e::ALIGN_ARBITRARY)
            {
                newblock->SetBlockSRS(LOCALSRS);
                newblock->GetCurrentATMutual()->SetLocalSrs(LOCALSRS);
            }
            //@attention之所以注释掉，是因为有时候调式的时候希望保留
            /*if (AI3D::CORE::File::ExistsDir(newblock->GetPath()))
            {
                AI3D::CORE::File::Remove(newblock->GetPath());
            }
            else*/
            {
                AI3D::CORE::File::CreateDirIfNotExists(newblock->GetPath());
            }
            /*auto atdata = newblock->GetCurrentAT().get();*/
            AI3D::CORE::ATData* atdata = new AI3D::CORE::ATData();
            *atdata = *newblock->GetCurrentAT().get();
            {
                
            }
            std::cout<< "=================at ====:" << atdata->GetDefaultEnuSRS().type << std::endl;
            bool  ret = ATCommandSet::CreateATFiles(*atdata, newblock->GetPath(), at_options);
            
            if (!ret)
            {
                LOGE("Submit AT CreateFiles Failed");
                return false;
            }
            BlockObject::Task_Info& task = newblock->GetTaskInfoMutual();
            appconfig_s options = Application::Getinstance().ParseConfig();

            Eigen::Vector3d possigma = at_options.sfmsettings.pos_sigma;
            options.at_options.sfmsettings.pos_sigma = possigma;
            options.at_options.feature_num = at_options.feature_num;
            options.at_options.at_name = at_options.at_name;
            task.debug_level_ = options.debug_level;
            task.focal_length_ = options.focal_length;
            task.keyMaxImgNum = options.keyMaxImgNum;
            task.matchMaxImgNum = options.matchMaxImgNum;
            //task.matchTaskNum = options.matchTaskNum;
            task.blockString = at_options.at_name;
            auto opttemp = task.at_options;
            task.at_options = options.at_options;
            task.at_options.sfmsettings.bapolicies = at_options.sfmsettings.bapolicies;
            task.at_options.align_mode = opttemp.align_mode;

            task.at_options.reconstruct_mode = at_options.reconstruct_mode;
            task.at_options.saveoptions.output_rawxml = options.at_options.saveoptions.output_rawxml;

            std::string postFix = "";
            if (PROJECT_USE_BIN) {
                postFix = BINDOTPROJECTPOSTFIX;
            }
            else {
                postFix = DOTPROJECTPOSTFIX;
            }
            std::string projectfile = project_ptr->GetPath() + PATH_SEPARATOR_STR + project_ptr->GetName() + postFix;
            
            std::string jobpath = qstr2str(Settings::getMasterJobQueue()) + PATH_SEPARATOR_STR + "Pending" + PATH_SEPARATOR_STR + HIGHLEVEL;
            
            std::string hostName = QHostInfo::localHostName().toStdString();
            bool ret1 = JobMonitor::CreateDirs();
            if (!ret1)
                return false;
            std::string jobstr;
            if (!ATCommandSet::CreateATTaskInfo(hostName, jobpath, newblock->GetPath(), task, jobstr))
            {
                LOGE("Submit AT CreateTask Failed");
                return false;
            }
            task.job_ = jobstr;
            LOGI(String::StringPrintf("%s(%s) Submit AT and Save project", newblock->GetName().c_str(), newblock->GetTaskInfo().blockString.c_str()));
            //保存工程
            QApplication::processEvents();
            //此处的保存会涉及到三个文件，chy
            manager->GetProject()->Save(savetype_e::XML_SAVED);
            emit Sig_SaveFinished();

            return true;
        }

        

    }
}
