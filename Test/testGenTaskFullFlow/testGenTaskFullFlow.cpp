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
    // 1. Create temp directory
    QString tmpRoot = "./test_gen_node_"
        + QUuid::createUuid().toString(QUuid::WithoutBraces);
    QDir().mkpath(tmpRoot);

    // 2. 设置全局路径 (对标 MakePath)
    // 2. Set global paths (matching MakePath)
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
    // 3. Create Block
    std::string projectPath = tmpRoot.toStdString();
    BlockObject block(projectPath);
    block.GetIdMutual() = 1;
    block.Init();

    BlockObject::Task_Info& task = block.GetTaskInfoMutual();
    task.blockString = "Block_1";
    task.blockName = "Block_1"; // Init() 不设 blockName, 需手动赋值
    task.blockId = 1;
    task.projectfile_ = projectPath;
    task.isSaved = false;
    task.isLoaded = true;
    task.block_task_category = 1;
    task.gen_options.gen_params.sub_type = GenTaskSubType::TEXT_TO_MODEL;
    task.gen_options.gen_params.prompt = "a dragon statue";
    task.gen_options.gen_params.polygon_limit = 30000;

    // 4. 落地 Block 文件 (Block_1 目录 + .blk)
    // 4. Persist Block file (Block_1 dir + .blk)
    block.Save();

    // 5. 前端提交任务
    // 5. Frontend submits task
    std::cout << "--- Submit ---" << std::endl;
    SubmitResult result = GenTaskAPI::SubmitGenTask(
        task, "testuser@example.com", genPendingJobPath.toStdString());
    TEST_ASSERT(result.success, "SubmitGenTask should succeed");
    std::cout << "  task_uuid=" << result.task_uuid
        << "  job=" << result.job_name << std::endl;

    // 6. 启动 GenTaskThread 线程 (真实调度逻辑, HTTP 走 mock)
    // 6. Start GenTaskThread (real scheduling, HTTP mock)
    std::cout << "\n--- GenTaskThread::Run in std::thread ---" << std::endl;
    std::thread genThread(GenTaskThread::Run);

    // 7. 主线程轮询 feedback 等待完成
    // 7. Main thread polls feedback waiting for completion
    std::string expectedResultDir = projectPath + "/Block_1/Generations/Generation_1";
    std::string fbPath = expectedResultDir + "/JF_" + result.job_name
        + (JOB_FEEDBACK_USE_BIN ? BINFILE_POSTFIX : JSONFILE_POSTFIX);
    bool completed = false;
    for (int i = 0; i < 60; i++)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        JobFeedBack_s fb;
        if (!fb.load_with_retry(fbPath, false)) continue;
        std::cout << "  [-----" << fbPath << "-----] status=" << (int)fb.Status
            << " percent=" << fb.Percent << "%" << std::endl;
        if (fb.Status == jobsta_e::STATUS_COMPLETE)
        {
            completed = true;
            break;
        }
        if (fb.Status == jobsta_e::STATUS_FAILURE) break;
    }
    TEST_ASSERT(completed, "task should complete within 60s");

    // 8. 停止线程
    // 8. Stop thread
    bQuitingApplication = true;
    genThread.join();

    // 9. 验证结果
    // 9. Verify results
    std::cout << "\n--- Verify ---" << std::endl;
    JobFeedBack_s fb;
    TEST_ASSERT(fb.load_with_retry(fbPath, false), "should load feedback");
    TEST_ASSERT(fb.Status == jobsta_e::STATUS_COMPLETE, "feedback COMPLETE");
    TEST_ASSERT(fb.Percent == 100.0f, "feedback 100%");

    QDir completedDir(genCompletedJobPath);
    QStringList done = completedDir.entryList({"J_*.bin"}, QDir::Files);
    TEST_ASSERT(!done.isEmpty(), "job in Completed/");

    GenJobFullInfo_s doneInfo;
    TEST_ASSERT(doneInfo.load_with_retry(
                    (genCompletedJobPath.toStdString() + done[0].toStdString())), "load completed job");
    std::cout << "  consumed=" << doneInfo.job.point_info.consumed
        << " total_balance=" << doneInfo.job.point_info.total_balance << std::endl;

    // 9. 清理
    // 9. Cleanup
    //QDir(tmpRoot).removeRecursively();

    if (g_failures == 0)
    {
        std::cout << "\n===== ALL TESTS PASSED =====" << std::endl;
        return 0;
    }
    std::cout << "\n===== " << g_failures << " TEST(S) FAILED =====" << std::endl;
    return 1;
}
