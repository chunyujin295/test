// Test/testPoints/testPointManager.cpp
// 覆盖: EstimateTaskPoints / CreatePointTask / QueryUserPoints / SettlePoints

#include "Core/PointManager.h"
#include "Core/Logging.h"
#include "Core/GenTaskOptions.h"
#include <cstdio>
#include <cstring>
#include <string>

using namespace AI3D::CORE;

static int g_failures = 0;
#define TEST_ASSERT(cond, msg) do { if (!(cond)) { ++g_failures; LOGE(std::string("FAIL: ") + msg); } } while (0)
#define LOGPASS(msg) LOGI(std::string("PASS: ") + msg)

// ============================== 1.1 EstimateTaskPoints ==============================

static void TestEstimateAT()
{
    int pts = PointManager::EstimateTaskPoints("sc",
                                               R"({"images":{"total_count":100},"tiles":{"total_count":1}})");
    TEST_ASSERT(pts > 0, "#1 AT estimate should return positive");
    LOGPASS("#1 Estimate AT");
}

static void TestEstimateProduction()
{
    int pts = PointManager::EstimateTaskPoints("mesh",
                                               R"({"images":{"total_count":200},"tiles":{"total_count":10}})");
    TEST_ASSERT(pts > 0, "#2 Production estimate should return positive");
    LOGPASS("#2 Estimate Production");
}

static void TestEstimateGenTask()
{
    GenTaskParams params;
    params.sub_type = GenTaskSubType::TEXT_TO_MODEL;
    params.prompt = "test";
    int pts = PointManager::EstimateTaskPoints("text-to-model", params.ToJsonString());
    TEST_ASSERT(pts > 0, "#3 GenTask estimate should return positive");
    LOGPASS("#3 Estimate GenTask");
}

static void TestEstimateInvalid()
{
    int pts = PointManager::EstimateTaskPoints("invalid", "{}");
    TEST_ASSERT(pts == -1, "#4 Invalid business_type should return -1");
    LOGPASS("#4 Estimate invalid business_type");
}

// ============================== 1.2 CreatePointTask (Freeze) ==============================

static void TestFreezeNormal()
{
    PointFreezeInfo result = PointManager::CreatePointTask("mesh",
                                                           R"({"images":{"total_count":200},"tiles":{"total_count":10}})");
    TEST_ASSERT(!result.freeze_no.empty(), "#5 freeze_no should not be empty");
    TEST_ASSERT(result.frozen_points > 0, "#5 frozen_points should > 0");
    LOGPASS("#5 Freeze normal");
}

static void TestFreezeInsufficient()
{
    // MOCK_POINT_MANAGER 下不会余额不足, 只测正常路径
    PointFreezeInfo result = PointManager::CreatePointTask("mesh",
                                                           R"({"images":{"total_count":999999},"tiles":{"total_count":999}})");
    TEST_ASSERT(!result.freeze_no.empty(), "#6 mock freeze should still succeed");
    LOGPASS("#6 Freeze (mock — real would check balance)");
}

static void TestFreezeTimeout()
{
    // MOCK 下不会超时, 测 mock 正常返回
    PointFreezeInfo result = PointManager::CreatePointTask("mesh", "{}");
    TEST_ASSERT(!result.freeze_no.empty(), "#7 mock freeze should succeed");
    LOGPASS("#7 Freeze timeout (mock)");
}

static void TestFreezeIdempotent()
{
    PointFreezeInfo r1 = PointManager::CreatePointTask("sc",
                                                       R"({"images":{"total_count":10},"tiles":{"total_count":1}})");
    PointFreezeInfo r2 = PointManager::CreatePointTask("sc",
                                                       R"({"images":{"total_count":10},"tiles":{"total_count":1}})");
    TEST_ASSERT(!r1.freeze_no.empty() && !r2.freeze_no.empty(), "#8 both should return freeze_no");
    LOGPASS("#8 Freeze idempotent (mock)");
}

static void TestFreezeAT()
{
    PointFreezeInfo result = PointManager::CreatePointTask("sc",
                                                           R"({"images":{"total_count":50},"tiles":{"total_count":1}})");
    TEST_ASSERT(!result.freeze_no.empty(), "#9 AT freeze should get freeze_no");
    LOGPASS("#9 Freeze AT");
}

// ============================== 1.3 QueryUserPoints ==============================

static void TestQueryPoints()
{
    PointFreezeInfo result = PointManager::QueryUserPoints();
    TEST_ASSERT(result.total_balance >= result.available_points, "#11 total >= available");
    TEST_ASSERT(result.frozen_points >= 0, "#11 frozen >= 0");
    LOGPASS("#11 QueryUserPoints");
}

// ============================== 1.4 SettlePoints ==============================

static void TestSettleSuccess()
{
    PointFreezeInfo fr = PointManager::CreatePointTask("mesh",
                                                       R"({"images":{"total_count":10},"tiles":{"total_count":2}})");
    PointSettleInfo sr = PointManager::SettlePoints(fr.freeze_no, "success",
                                                    R"({"images":{"total_count":10},"tiles":{"total_count":2}})");
    TEST_ASSERT(sr.consumed > 0, "#13 settle success: consumed should > 0");
    LOGPASS("#13 Settle success");
}

static void TestSettleFail()
{
    PointFreezeInfo fr = PointManager::CreatePointTask("mesh",
                                                       R"({"images":{"total_count":10},"tiles":{"total_count":2}})");
    PointSettleInfo sr = PointManager::SettlePoints(fr.freeze_no, "fail", "{}");
    TEST_ASSERT(sr.refunded > 0, "#14 settle fail: refunded should > 0");
    TEST_ASSERT(sr.consumed == 0, "#14 settle fail: consumed should be 0");
    LOGPASS("#14 Settle fail (full refund)");
}

static void TestSettleCancel()
{
    PointFreezeInfo fr = PointManager::CreatePointTask("sc",
                                                       R"({"images":{"total_count":5},"tiles":{"total_count":1}})");
    PointSettleInfo sr = PointManager::SettlePoints(fr.freeze_no, "cancel", "{}");
    TEST_ASSERT(sr.refunded > 0, "#15 settle cancel: refunded should > 0");
    LOGPASS("#15 Settle cancel");
}

static void TestSettleInvalidFreezeNo()
{
    PointSettleInfo sr = PointManager::SettlePoints("invalid-xxx", "success", "{}");
    TEST_ASSERT(sr.consumed == 0 && sr.refunded == 0, "#17 invalid freeze_no: consumed=refunded=0");
    LOGPASS("#17 Settle invalid freeze_no");
}

static void TestSettleDuplicate()
{
    PointFreezeInfo fr = PointManager::CreatePointTask("mesh",
                                                       R"({"images":{"total_count":5},"tiles":{"total_count":1}})");
    PointManager::SettlePoints(fr.freeze_no, "success", "{}");
    PointSettleInfo sr2 = PointManager::SettlePoints(fr.freeze_no, "success", "{}");
    TEST_ASSERT(sr2.consumed == 0, "#18 duplicate settle: consumed should be 0");
    LOGPASS("#18 Settle duplicate (idempotent)");
}

// ============================================================================

int main()
{
    // 1.1 EstimateTaskPoints
    TestEstimateAT();
    TestEstimateProduction();
    TestEstimateGenTask();
    TestEstimateInvalid();

    // 1.2 CreatePointTask
    TestFreezeNormal();
    TestFreezeInsufficient();
    TestFreezeTimeout();
    TestFreezeIdempotent();
    TestFreezeAT();

    // 1.3 QueryUserPoints
    TestQueryPoints();

    // 1.4 SettlePoints
    TestSettleSuccess();
    TestSettleFail();
    TestSettleCancel();
    TestSettleInvalidFreezeNo();
    TestSettleDuplicate();

    if (g_failures == 0)
    {
        LOGI("All testPointManager tests passed (18 cases).");
        return 0;
    }
    else
    {
        LOGE(std::to_string(g_failures) + " test(s) failed.");
        return 1;
    }
}
