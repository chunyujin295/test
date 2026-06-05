#include "Core/GenTaskOptions.h"
#include "Core/GenTaskAPI.h"
#include "Util/GenTaskProcess.h"
#include "Util/TaskProcess.h"
#include "Core/BlockObject.h"
#include "Core/Types.h"
#include "Core/Logging.h"
#include "GenTaskThread.h"
#include "GenHttpClient.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QUuid>
#include <iostream>
#include <thread>
#include <chrono>

using namespace AI3D::CORE;

// GenTaskThread 需要的 extern 变量 (对标 CallEngine.cpp)
bool bQuitingApplication = false;
QString genPendingJobPath;
QString genRunningJobPath;
QString genCompletedJobPath;
QString genFailedJobPath;
QString genCancelledJobPath;

static int g_failures = 0;
#define TEST_ASSERT(cond, msg) \
    do { if (!(cond)) { ++g_failures; std::cerr << "FAIL: " << msg << std::endl; } } while(0)

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    std::cout << "\n===== GenTask Node Thread Integration Test =====\n" << std::endl;

    // 1. 创建临时目录
    QString tmpRoot = QString(".") + QString("/test_gen_node_")
        + QUuid::createUuid().toString(QUuid::WithoutBraces);
    QDir().mkpath(tmpRoot);

    // 2. 设置全局路径 (对标 MakePath)
    QString genRoot = tmpRoot + "/jobs_gen";
    genPendingJobPath = genRoot + "/Pending/";
    genRunningJobPath = genRoot + "/Running/";
    genCompletedJobPath = genRoot + "/Completed/";
    genFailedJobPath = genRoot + "/Failed/";
    genCancelledJobPath = genRoot + "/Cancelled/";
    QDir().mkpath(genPendingJobPath);
    QDir().mkpath(genRunningJobPath);
    QDir().mkpath(genCompletedJobPath);
    QDir().mkpath(genFailedJobPath);
    QDir().mkpath(genCancelledJobPath);

    // 3. 创建 Block
    std::string projectPath = tmpRoot.toStdString();
    BlockObject block(projectPath);
    block.GetIdMutual() = 1;
    block.Init();
    block.Save();

    BlockObject::Task_Info& task = block.GetTaskInfoMutual();
    task.blockString = "Block_1";
    task.blockId = 1;
    task.projectfile_ = projectPath;
    task.isSaved = false;
    task.isLoaded = true;
    task.block_task_category = 1;
    task.gen_options.gen_params.sub_type = GenTaskSubType::TEXT_TO_MODEL;
    task.gen_options.gen_params.prompt = "a dragon statue";
    task.gen_options.gen_params.polygon_limit = 30000;

    // 4. 前端提交任务
    std::cout << "--- Submit ---" << std::endl;
    GenTaskAPI::SubmitResult result = GenTaskAPI::SubmitGenTask(
        task, "testuser@example.com", genPendingJobPath.toStdString());
    TEST_ASSERT(result.success, "SubmitGenTask should succeed");
    std::cout << "  task_uuid=" << result.task_uuid
        << "  job=" << result.job_name << std::endl;

    // 5. 启动 GenTaskThread 线程 (真实调度逻辑, HTTP 走 mock)
    std::cout << "\n--- GenTaskThread::Run in std::thread ---" << std::endl;
    std::thread genThread(GenTaskThread::Run);

    // 6. 主线程轮询 feedback 等待完成
    std::string fbPath = projectPath + "/JF_"+ result.job_name
        + (JOB_FEEDBACK_USE_BIN ? BINFILE_POSTFIX : JSONFILE_POSTFIX);
    bool completed = false;
    for (int i = 0; i < 60; i++)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        JobFeedBack_s fb;
        if (!fb.load_with_retry(fbPath, false)) continue;
        std::cout << "  [" << i << "] status=" << (int)fb.Status
            << " percent=" << fb.Percent << "%" << std::endl;
        if (fb.Status == jobsta_e::STATUS_COMPLETE)
        {
            completed = true;
            break;
        }
        if (fb.Status == jobsta_e::STATUS_FAILURE) break;
    }
    TEST_ASSERT(completed, "task should complete within 60s");

    // 7. 停止线程
    bQuitingApplication = true;
    genThread.join();

    // 8. 验证结果
    std::cout << "\n--- Verify ---" << std::endl;
    JobFeedBack_s fb;
    TEST_ASSERT(fb.load_with_retry(fbPath, false), "should load feedback");
    TEST_ASSERT(fb.Status == jobsta_e::STATUS_COMPLETE, "feedback COMPLETE");
    TEST_ASSERT(fb.Percent == 100.0f, "feedback 100%");

    QDir completedDir(genCompletedJobPath);
    QStringList done = completedDir.entryList({"J_*"}, QDir::Files);
    TEST_ASSERT(!done.isEmpty(), "job in Completed/");

    GenJobFullInfo_s doneInfo;
    TEST_ASSERT(doneInfo.load_with_retry(
                    QString(genCompletedJobPath + done[0]).toStdString()), "load completed job");
    std::cout << "  result_url=" << doneInfo.job.result_url << std::endl;
    TEST_ASSERT(!doneInfo.job.result_url.empty(), "result_url not empty");

    // 9. 清理
    QDir(tmpRoot).removeRecursively();

    if (g_failures == 0)
    {
        std::cout << "\n===== ALL TESTS PASSED =====" << std::endl;
        return 0;
    }
    std::cout << "\n===== " << g_failures << " TEST(S) FAILED =====" << std::endl;
    return 1;
}
