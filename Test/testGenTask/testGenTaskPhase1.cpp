// ============================================================================
// testGenTaskPhase1 — Phase 1 数据结构验证
//
// 验证点:
//   1. GenTaskParams rapidjson 序列化往返 (WriteToJson / ParseJson / ToJsonString / CreateFromJsonString)
//   2. GenTaskOptions 委托 gen_params 的 rapidjson 往返
//   3. GenJobFullInfo_s BIN 序列化往返 (WriteToBin / LoadFromBin, 含 feedback)
//   4. 默认值 / 旧数据兼容
// ============================================================================

#include "Core/GenTaskOptions.h"
#include "Util/GenTaskProcess.h"
#include "Core/Types.h"
#include "Core/Logging.h"

#include <cstdio>
#include <cstring>
#include <string>

#include "Core/BlockObject.h"
#include "Core/GenTaskAPI.h"

using namespace AI3D::CORE;

static int g_failures = 0;

#define TEST_ASSERT(cond, msg)                                                 \
    do {                                                                       \
        if (!(cond)) {                                                         \
            ++g_failures;                                                      \
            LOGE(std::string("FAIL: ") + msg);                                 \
        }                                                                      \
    } while (0)

// ============================================================================
// 测试 1: GenTaskParams rapidjson 往返
// Test 1: GenTaskParams rapidjson roundtrip
// ============================================================================
static void TestGenTaskParamsRoundtrip()
{
    // 1a. 构造完整参数
    // 1a. Build complete parameters
    GenTaskParams p;
    p.sub_type = GenTaskSubType::IMAGE_TO_MODEL;
    p.prompt = "a red sports car";
    p.negative_prompt = "blurry, low quality";
    p.polygon_limit = 50000;
    p.texture_size = 2048;
    p.model_version = "v2";
    p.upload_file_key = "fk-abc123";

    // 1b. WriteToJson → rapidjson::Document
    rapidjson::Document doc;
    doc.SetObject();
    p.WriteToJson(doc, doc);

    // 1c. 验证写入的字段
    // 1c. Verify written fields
    TEST_ASSERT(doc.HasMember("sub_type"), "sub_type should be written");
    TEST_ASSERT(std::strcmp(doc["sub_type"].GetString(), "image-to-model") == 0,
                "sub_type should be image-to-model");
    TEST_ASSERT(doc.HasMember("prompt"), "prompt should be written");
    TEST_ASSERT(std::strcmp(doc["prompt"].GetString(), "a red sports car") == 0,
                "prompt mismatch");
    TEST_ASSERT(doc.HasMember("polygon_limit"), "polygon_limit should be written");
    TEST_ASSERT(doc["polygon_limit"].GetInt() == 50000, "polygon_limit mismatch");
    TEST_ASSERT(doc.HasMember("texture_size"), "texture_size should be written");
    TEST_ASSERT(doc["texture_size"].GetInt() == 2048, "texture_size mismatch");

    // 1d. ToJsonString → CreateFromJsonString 字符串往返
    // 1d. ToJsonString → CreateFromJsonString string roundtrip
    std::string json = p.ToJsonString();
    TEST_ASSERT(!json.empty(), "ToJsonString should not be empty");

    GenTaskParams p2 = GenTaskParams::CreateFromJsonString(json);
    TEST_ASSERT(p2.sub_type == GenTaskSubType::IMAGE_TO_MODEL, "sub_type roundtrip failed");
    TEST_ASSERT(p2.prompt == "a red sports car", "prompt roundtrip failed");
    TEST_ASSERT(p2.negative_prompt == "blurry, low quality", "negative_prompt roundtrip failed");
    TEST_ASSERT(p2.polygon_limit == 50000, "polygon_limit roundtrip failed");
    TEST_ASSERT(p2.texture_size == 2048, "texture_size roundtrip failed");
    TEST_ASSERT(p2.model_version == "v2", "model_version roundtrip failed");
    TEST_ASSERT(p2.upload_file_key == "fk-abc123", "upload_file_key roundtrip failed");

    // 1e. UNKNOWN sub_type 不应写入
    // 1e. UNKNOWN sub_type should not be written
    GenTaskParams p3;
    p3.sub_type = GenTaskSubType::UNKNOWN;
    p3.prompt = "test";
    rapidjson::Document doc3;
    doc3.SetObject();
    p3.WriteToJson(doc3, doc3);
    TEST_ASSERT(!doc3.HasMember("sub_type"), "UNKNOWN sub_type should not be written");

    // 1f. 空字符串不应写入
    // 1f. Empty string should not be written
    GenTaskParams p4;
    p4.prompt = "";
    rapidjson::Document doc4;
    doc4.SetObject();
    p4.WriteToJson(doc4, doc4);
    TEST_ASSERT(!doc4.HasMember("prompt"), "empty prompt should not be written");
}

// ============================================================================
// 测试 2: GenTaskOptions 委托 rapidjson 往返
// Test 2: GenTaskOptions delegation rapidjson roundtrip
// ============================================================================
static void TestGenTaskOptionsRoundtrip()
{
    // 2a. 通过 GenTaskOptions 写入 (委托 gen_params)
    // 2a. Write via GenTaskOptions (delegates to gen_params)
    GenTaskOptions opts;
    opts.gen_params.sub_type = GenTaskSubType::TEXT_TO_MODEL;
    opts.gen_params.prompt = "a cute cat";
    opts.gen_params.polygon_limit = 10000;

    rapidjson::Document doc;
    doc.SetObject();
    opts.WriteToJson(doc, doc);

    TEST_ASSERT(doc.HasMember("sub_type"), "sub_type should be written via GenTaskOptions");
    TEST_ASSERT(doc.HasMember("prompt"), "prompt should be written via GenTaskOptions");
    TEST_ASSERT(doc.HasMember("polygon_limit"), "polygon_limit should be written");

    // 2b. 读取回 GenTaskOptions
    // 2b. Read back into GenTaskOptions
    GenTaskOptions opts2;
    opts2.ParseJson(doc);
    TEST_ASSERT(opts2.gen_params.sub_type == GenTaskSubType::TEXT_TO_MODEL,
                "sub_type ParseJson failed");
    TEST_ASSERT(opts2.gen_params.prompt == "a cute cat", "prompt ParseJson failed");
    TEST_ASSERT(opts2.gen_params.polygon_limit == 10000, "polygon_limit ParseJson failed");
}

// ============================================================================
// 测试 3: GenJobFullInfo_s BIN 往返
// Test 3: GenJobFullInfo_s BIN roundtrip
// ============================================================================
static void TestGenJobFullInfoBinRoundtrip()
{
    // 3a. 构造完整 job + feedback
    // 3a. Build complete job + feedback
    GenJobFullInfo_s info;
    info.job_name = "J_TestBlock_20260101000000";
    info.job.task_uuid = "550e8400-e29b-41d4-a716-446655440000";
    info.job.engine_id = "engine-test-01";
    info.job.user_account = "user@example.com";
    info.job.project_path = "/test/project";
    info.job.block_item = "TestBlock";
    info.job.params.sub_type = GenTaskSubType::TEXT_TO_MESH;
    info.job.params.prompt = "a dragon statue";
    info.job.params.polygon_limit = 20000;
    info.job.params.texture_size = 1024;
    info.job.status = GenTaskStatus::IN_PROGRESS;
    info.job.result_path = "/local/path/result.glb";
    info.job.preview_path = "/local/path/preview.png";
    info.job.point_info.freeze_no = "test-freeze-no-123";
    info.job.point_info.frozen_points = 100;
    info.job.point_info.consumed = 10;
    info.job.point_info.total_balance = 90;
    info.job.point_info.available_points = 90;
    info.job.point_info.points_settled = false;
    info.job.query_retry_count = 2;
    info.feedback.Status = jobsta_e::STATUS_RUNNING;
    info.feedback.Percent = 45.0f;
    info.feedback.TaskRetVal = 0;
    info.feedback.Msg = "processing mesh generation";

    // 3b. 写 BIN
    // 3b. Write BIN
    const std::string path = "test_genjob_phase1.bin";
    bool ok = info.WriteToBin(path);
    TEST_ASSERT(ok, "WriteToBin should succeed");

    // 3c. 读 BIN
    // 3c. Read BIN
    GenJobFullInfo_s loaded;
    ok = loaded.LoadFromBin(path);
    TEST_ASSERT(ok, "LoadFromBin should succeed");

    // 3d. 逐字段验证 — GenJobInfo_s
    // 3d. Field-by-field verification — GenJobInfo_s
    TEST_ASSERT(loaded.job_name == info.job_name, "job_name mismatch");
    TEST_ASSERT(loaded.job.task_uuid == info.job.task_uuid, "task_uuid mismatch");
    TEST_ASSERT(loaded.job.engine_id == info.job.engine_id, "engine_id mismatch");
    TEST_ASSERT(loaded.job.user_account == info.job.user_account, "user_account mismatch");
    TEST_ASSERT(loaded.job.project_path == info.job.project_path, "project_path mismatch");
    TEST_ASSERT(loaded.job.block_item == info.job.block_item, "block_item mismatch");
    TEST_ASSERT(loaded.job.params.sub_type == info.job.params.sub_type,
                "params.sub_type mismatch");
    TEST_ASSERT(loaded.job.params.prompt == info.job.params.prompt,
                "params.prompt mismatch");
    TEST_ASSERT(loaded.job.params.polygon_limit == info.job.params.polygon_limit,
                "params.polygon_limit mismatch");
    TEST_ASSERT(loaded.job.params.texture_size == info.job.params.texture_size,
                "params.texture_size mismatch");
    TEST_ASSERT(loaded.job.status == info.job.status, "status mismatch");
    TEST_ASSERT(loaded.job.result_path == info.job.result_path, "result_path mismatch");
    TEST_ASSERT(loaded.job.preview_path == info.job.preview_path, "preview_path mismatch");
    TEST_ASSERT(loaded.job.point_info.freeze_no == info.job.point_info.freeze_no,
                "freeze_no mismatch");
    TEST_ASSERT(loaded.job.point_info.consumed == info.job.point_info.consumed,
                "consumed mismatch");
    TEST_ASSERT(loaded.job.point_info.total_balance == info.job.point_info.total_balance,
                "total_balance mismatch");
    TEST_ASSERT(loaded.job.query_retry_count == info.job.query_retry_count,
                "query_retry_count mismatch");

    // 3e. 验证 feedback 字段
    // 3e. Verify feedback fields
    TEST_ASSERT(loaded.feedback.Status == info.feedback.Status, "feedback.Status mismatch");
    TEST_ASSERT(loaded.feedback.Percent == info.feedback.Percent, "feedback.Percent mismatch");
    TEST_ASSERT(loaded.feedback.TaskRetVal == info.feedback.TaskRetVal,
                "feedback.TaskRetVal mismatch");
    TEST_ASSERT(loaded.feedback.Msg == info.feedback.Msg, "feedback.Msg mismatch");

    // 3f. 清理
    // 3f. Cleanup
    std::remove(path.c_str());
}

// ============================================================================
// 测试 4: 默认值 / 旧数据兼容
// Test 4: Defaults / old data compatibility
// ============================================================================
static void TestDefaults()
{
    // 4a. GenTaskParams 默认构造
    // 4a. GenTaskParams default construction
    GenTaskParams p;
    TEST_ASSERT(p.sub_type == GenTaskSubType::UNKNOWN, "default sub_type should be UNKNOWN");
    TEST_ASSERT(p.prompt.empty(), "default prompt should be empty");
    TEST_ASSERT(p.polygon_limit == 0, "default polygon_limit should be 0");
    TEST_ASSERT(p.texture_size == 0, "default texture_size should be 0");

    // 4b. 空 JSON 字符串 → 返回默认值
    // 4b. Empty JSON string → returns defaults
    GenTaskParams p2 = GenTaskParams::CreateFromJsonString("");
    TEST_ASSERT(p2.sub_type == GenTaskSubType::UNKNOWN, "empty json should give UNKNOWN");
    TEST_ASSERT(p2.prompt.empty(), "empty json should give empty prompt");

    // 4c. 不完整 JSON → 缺失字段保持默认值
    // 4c. Partial JSON → missing fields keep defaults
    GenTaskParams p3 = GenTaskParams::CreateFromJsonString("{\"prompt\":\"hello\"}");
    TEST_ASSERT(p3.prompt == "hello", "prompt should be parsed");
    TEST_ASSERT(p3.sub_type == GenTaskSubType::UNKNOWN, "missing sub_type should be UNKNOWN");
    TEST_ASSERT(p3.polygon_limit == 0, "missing polygon_limit should be 0");

    // 4d. GenTaskOptions 默认构造
    // 4d. GenTaskOptions default construction
    GenTaskOptions opts;
    TEST_ASSERT(opts.gen_params.sub_type == GenTaskSubType::UNKNOWN,
                "GenTaskOptions default sub_type should be UNKNOWN");
}

// ============================================================================
// 测试 5: blk_generation_info_s rapidjson 往返 (写入 .blk 的子结构)
// Test 5: blk_generation_info_s rapidjson roundtrip (sub-structure written to .blk)
// ============================================================================
static void TestBlkGenerationInfoRoundtrip()
{
    // 5a. 构造完整的生成结果元数据
    // 5a. Build complete generation result metadata
    blk_generation_info_s info;
    info.task_uuid = "uuid-test-12345";
    info.job_name = "J_TestBlock_20260129120000";
    info.sub_type = static_cast<int>(GenTaskSubType::TEXT_TO_MODEL);
    info.status = static_cast<int>(GenTaskStatus::COMPLETED);
    info.result_url = "https://cdn.example.com/result.glb";
    info.result_path = "/local/path/result.glb";
    info.preview_path = "/local/path/preview.png";
    info.created_time = "20260129120000";

    // 5b. 写入 rapidjson Document (模拟 WriteBlockInfoToJson 中的一个数组元素)
    // 5b. Write to rapidjson Document (simulating an array element in WriteBlockInfoToJson)
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Value genJson(rapidjson::kObjectType);
    info.CreateJson(genJson, doc);
    doc.AddMember("item", genJson, doc.GetAllocator());

    // 5c. 验证写入
    // 5c. Verify write
    TEST_ASSERT(doc.HasMember("item"), "item should exist");
    const rapidjson::Value& item = doc["item"];
    TEST_ASSERT(strcmp(item["task_uuid"].GetString(), "uuid-test-12345") == 0,
                "task_uuid mismatch");
    TEST_ASSERT(strcmp(item["job_name"].GetString(), "J_TestBlock_20260129120000") == 0,
                "job_name mismatch");
    TEST_ASSERT(item["sub_type"].GetInt() == static_cast<int>(GenTaskSubType::TEXT_TO_MODEL),
                "sub_type mismatch");
    TEST_ASSERT(item["status"].GetInt() == static_cast<int>(GenTaskStatus::COMPLETED),
                "status mismatch");

    // 5d. 读回
    // 5d. Read back
    blk_generation_info_s loaded;
    loaded.ParseJson(item);
    TEST_ASSERT(loaded.task_uuid == info.task_uuid, "ParseJson task_uuid failed");
    TEST_ASSERT(loaded.job_name == info.job_name, "ParseJson job_name failed");
    TEST_ASSERT(loaded.sub_type == info.sub_type, "ParseJson sub_type failed");
    TEST_ASSERT(loaded.status == info.status, "ParseJson status failed");
    TEST_ASSERT(loaded.result_url == info.result_url, "ParseJson result_url failed");
    TEST_ASSERT(loaded.result_path == info.result_path, "ParseJson result_path failed");
    TEST_ASSERT(loaded.preview_path == info.preview_path, "ParseJson preview_path failed");
    TEST_ASSERT(loaded.created_time == info.created_time, "ParseJson created_time failed");
}

// ============================================================================
// 测试 6: generations_info_ JSON 数组往返 (模拟 .blk 中 generations_info 段的读写)
// Test 6: generations_info_ JSON array roundtrip (simulating .blk generations_info segment read/write)
// ============================================================================
static void TestGenerationsInfoArrayRoundtrip()
{
    // 6a. 构造两个条目
    // 6a. Build two entries
    blk_generation_info_s gen1;
    gen1.task_uuid = "uuid-1";
    gen1.job_name = "J_Block_001";
    gen1.sub_type = static_cast<int>(GenTaskSubType::TEXT_TO_MODEL);
    gen1.status = static_cast<int>(GenTaskStatus::COMPLETED);
    gen1.result_url = "https://cdn.example.com/result1.glb";

    blk_generation_info_s gen2;
    gen2.task_uuid = "uuid-2";
    gen2.job_name = "J_Block_002";
    gen2.sub_type = static_cast<int>(GenTaskSubType::IMAGE_TO_MODEL);
    gen2.status = static_cast<int>(GenTaskStatus::IN_PROGRESS);

    // 6b. 写入 JSON 数组 (模拟 WriteBlockInfoToJson 中 generationsInfo 段)
    // 6b. Write JSON array (simulating generationsInfo in WriteBlockInfoToJson)
    rapidjson::Document doc;
    doc.SetArray();
    auto& allocator = doc.GetAllocator();
    rapidjson::Value v1(rapidjson::kObjectType);
    gen1.CreateJson(v1, doc);
    doc.PushBack(v1, allocator);
    rapidjson::Value v2(rapidjson::kObjectType);
    gen2.CreateJson(v2, doc);
    doc.PushBack(v2, allocator);

    // 6c. 序列化为字符串 → 再解析 (模拟 BIN 中 gen_info_json 的读写)
    // 6c. Serialize to string → re-parse (simulating gen_info_json BIN read/write)
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    std::string jsonStr = buffer.GetString();

    rapidjson::Document doc2;
    TEST_ASSERT(!doc2.Parse(jsonStr.c_str()).HasParseError() && doc2.IsArray(),
                "generations_info array should parse");
    TEST_ASSERT(doc2.Size() == 2, "should have 2 entries");

    // 6d. 读回
    // 6d. Read back
    std::vector<blk_generation_info_s> loaded;
    for (rapidjson::SizeType i = 0; i < doc2.Size(); i++)
    {
        blk_generation_info_s info;
        info.ParseJson(doc2[i]);
        loaded.push_back(info);
    }
    TEST_ASSERT(loaded[0].task_uuid == "uuid-1", "array[0] task_uuid mismatch");
    TEST_ASSERT(loaded[0].status == static_cast<int>(GenTaskStatus::COMPLETED),
                "array[0] status mismatch");
    TEST_ASSERT(loaded[1].task_uuid == "uuid-2", "array[1] task_uuid mismatch");
    TEST_ASSERT(loaded[1].sub_type == static_cast<int>(GenTaskSubType::IMAGE_TO_MODEL),
                "array[1] sub_type mismatch");

    // 6e. generationjobs_ 序列化往返 (模拟 "task_uuid:job_name" 格式)
    // 6e. generationjobs_ roundtrip (simulating "task_uuid:job_name" format)
    std::string jobEntry = gen1.task_uuid + ":" + gen1.job_name;
    // 解析:
    size_t colonPos = jobEntry.find(":");
    std::string parsed_uuid = jobEntry.substr(0, colonPos);
    std::string parsed_job = jobEntry.substr(colonPos + 1);
    TEST_ASSERT(parsed_uuid == "uuid-1", "generationjobs uuid mismatch");
    TEST_ASSERT(parsed_job == "J_Block_001", "generationjobs job_name mismatch");
}

// ============================================================================
// 测试 7: Block 文件落地 → 重新加载 (对标 Slot_Action_NewBlock 创建流程)
// Test 7: Block file persist → reload (matching Slot_Action_NewBlock creation flow)
//   BlockObject(path) → Init() → 设 gen 字段 → Save() → ReadBlockInfoBin 读回
// ============================================================================
static void TestBlockFileGenFieldsRoundtrip()
{
    // 7a. 创建临时项目目录 (对标 Project path)
    // 7a. Create temp project directory (matching Project path)
    QString tmpDir = "./test_genblock_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
    QDir().mkpath(tmpDir);
    std::string projectPath = tmpDir.toStdString();

    // 7b. 对标 Slot_Action_NewBlock: new BlockObject(projectPath)
    // 7b. Matching Slot_Action_NewBlock: new BlockObject(projectPath)
    BlockObject block(projectPath);
    block.GetIdMutual() = 1;
    block.Init(); // → name_ = "Block_1", path_ = projectPath/Block_1
    block.SetStatus(jobsta_e::STATUS_NEW);

    QDir().mkpath(QString::fromStdString(block.GetPath()));

    // 对标 AddBlock 中的设置
    // Matching settings from AddBlock
    block.GetTaskInfoMutual().blockString = "Block_1";
    block.GetTaskInfoMutual().blockName = block.GetName();
    block.GetTaskInfoMutual().blockId = 1;
    block.GetTaskInfoMutual().projectfile_ = projectPath;
    block.GetTaskInfoMutual().isSaved = false;
    block.GetTaskInfoMutual().isLoaded = true;

    // 7c. 设置生成式字段
    // 7c. Set generative fields
    AI3D::CORE::BlockObject::Task_Info& info = block.GetTaskInfoMutual();
    info.block_task_category = 1;
    info.gen_options.gen_params.sub_type = GenTaskSubType::TEXT_TO_MODEL;
    info.gen_options.gen_params.prompt = "a test cube";
    info.gen_options.gen_params.polygon_limit = 10000;

    blk_generation_info_s gen1;
    gen1.task_uuid = "uuid-blk-test-1";
    gen1.job_name = "J_Block_1_20260129000000";
    gen1.sub_type = static_cast<int>(GenTaskSubType::TEXT_TO_MODEL);
    gen1.status = static_cast<int>(GenTaskStatus::COMPLETED);
    gen1.result_url = "https://cdn.example.com/result.glb";
    gen1.result_path = "/local/path/result.glb";
    gen1.created_time = "20260129000000";
    info.generations_info_.push_back(gen1);

    info.generationjobs_[gen1.task_uuid] = gen1.job_name;

    // 7d. 落地 (对标 Project::Save → BlockObject::Save)
    // 7d. Persist (matching Project::Save → BlockObject::Save)
    std::string blkPath = block.GetPath() + "/" + block.GetName() + ".blk";
    bool ok = info.WriteBlockInfoToBin(blkPath);
    TEST_ASSERT(ok, "WriteBlockInfoToBin failed");

    // 7e. 读回 .blk 文件
    // 7e. Read back .blk file
    //     Save 输出路径: path_/name_ + ".blk" = projectPath/Block_1/Block_1.blk
    //     Save output path: path_/name_ + ".blk" = projectPath/Block_1/Block_1.blk
    BlockObject::Task_Info loaded;
    ok = loaded.ReadBlockInfoBin(blkPath);
    TEST_ASSERT(ok, "ReadBlockInfoBin failed");

    // 7f. 验证 gen 字段往返
    // 7f. Verify gen field roundtrip
    TEST_ASSERT(loaded.block_task_category == 1,
                "block_task_category roundtrip failed");
    TEST_ASSERT(loaded.gen_options.gen_params.sub_type == GenTaskSubType::TEXT_TO_MODEL,
                "sub_type roundtrip failed");
    TEST_ASSERT(loaded.gen_options.gen_params.prompt == "a test cube",
                "prompt roundtrip failed");
    TEST_ASSERT(loaded.gen_options.gen_params.polygon_limit == 10000,
                "polygon_limit roundtrip failed");
    TEST_ASSERT(loaded.generations_info_.size() == 1,
                "generations_info_ size mismatch");
    TEST_ASSERT(loaded.generations_info_[0].task_uuid == "uuid-blk-test-1",
                "generations_info_ task_uuid mismatch");
    TEST_ASSERT(loaded.generationjobs_.size() == 1,
                "generationjobs_ size mismatch");

    // 7g. 清理
    // 7g. Cleanup
    // QDir(tmpDir).removeRecursively();
}

// ============================================================================
// 测试 8: 前端创建生成式任务端到端流程 (对标 Slot_Action_NewBlock 的逻辑)
// Test 8: Frontend gen task creation end-to-end (matching Slot_Action_NewBlock logic)
//   1. 创建 Block → 设 gen 参数 → SubmitGenTask → 验证 job + feedback 落地
//   1. Create Block → set gen params → SubmitGenTask → verify job + feedback persist
// ============================================================================
static void TestFrontendSubmitGenTask()
{
    // 8a. 创建临时项目目录和 Block (对标 new BlockObject(projectPath))
    // 8a. Create temp project dir and Block (matching new BlockObject(projectPath))
    QString tmpDir = QDir::tempPath() + "/test_frontend_gen_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
    QDir().mkpath(tmpDir);
    std::string projectPath = tmpDir.toStdString();

    BlockObject block(projectPath);
    block.GetIdMutual() = 1;
    block.Init();
    block.SetStatus(jobsta_e::STATUS_NEW);

    // 8b. 前端设置 Block 参数 (对标 ParamSettings4Production 设 options)
    // 8b. Frontend sets Block params (matching ParamSettings4Production set options)
    BlockObject::Task_Info& task = block.GetTaskInfoMutual();
    task.blockString = "Block_1";
    task.blockName = "Block_1";
    task.blockId = 1;
    task.projectfile_ = projectPath;
    task.isSaved = false;
    task.isLoaded = true;
    task.block_task_category = 1; // 生成式

    // 8c. 前端填入生成参数 (对标 options = ParamSettings4Production::GetSavedOptions)
    // 8c. Frontend fills gen params (matching options = ParamSettings4Production::GetSavedOptions)
    task.gen_options.gen_params.sub_type = GenTaskSubType::TEXT_TO_MODEL;
    task.gen_options.gen_params.prompt = "a red sports car";
    task.gen_options.gen_params.polygon_limit = 50000;
    task.gen_options.gen_params.texture_size = 1024;

    // 8d. 前端构造 pendingPath 并提交 (对标 SubmitProduction)
    // 8d. Frontend constructs pendingPath and submits (matching SubmitProduction)
    std::string pendingPath = tmpDir.toStdString() + "/jobs_gen/Pending/";
    QDir().mkpath(QString::fromStdString(pendingPath));

    std::string userAccount = "testuser@example.com";
    SubmitResult result = GenTaskAPI::SubmitGenTask(task, userAccount, pendingPath);

    // 8e. 验证返回值
    // 8e. Verify return value
    TEST_ASSERT(result.success, "SubmitGenTask should succeed");
    TEST_ASSERT(!result.task_uuid.empty(), "task_uuid should not be empty");
    TEST_ASSERT(!result.job_name.empty(), "job_name should not be empty");
    std::cout << "  task_uuid=" << result.task_uuid << " job=" << result.job_name << std::endl;

    // 8f. 验证 job 文件已落地
    // 8f. Verify job file persisted
    std::string jobFilePath = pendingPath + result.job_name + ".bin";
    TEST_ASSERT(QFileInfo(QString::fromStdString(jobFilePath)).exists(),
                "job file should exist");

    // 8g. 验证 job 文件可读, 参数一致
    // 8g. Verify job file readable, params match
    GenJobFullInfo_s jobInfo;
    TEST_ASSERT(jobInfo.load_with_retry(jobFilePath), "should load job file");
    TEST_ASSERT(jobInfo.job.params.sub_type == GenTaskSubType::TEXT_TO_MODEL,
                "sub_type should match");
    TEST_ASSERT(jobInfo.job.params.prompt == "a red sports car",
                "prompt should match");
    TEST_ASSERT(jobInfo.job.params.polygon_limit == 50000,
                "polygon_limit should match");
    TEST_ASSERT(jobInfo.job.user_account == userAccount,
                "user_account should match");
    TEST_ASSERT(jobInfo.job.project_path == projectPath,
                "project_path should match");
    std::string expectedResultDir = projectPath + "/Block_1/" GENERATION_DIR "/" GENERATION_PREFIX "1";
    TEST_ASSERT(jobInfo.job.result_dir == expectedResultDir,
                "result_dir should be Generations/Generation_1/");
    TEST_ASSERT(QDir(QString::fromStdString(expectedResultDir)).exists(),
                "Generations/Generation_1/ directory should be created");

    // 8h. 验证 feedback 文件已创建
    // 8h. Verify feedback file created
    std::string fbPath = expectedResultDir + "/JF_" + result.job_name
        + (JOB_FEEDBACK_USE_BIN ? BINFILE_POSTFIX : JSONFILE_POSTFIX);
    TEST_ASSERT(QFileInfo(QString::fromStdString(fbPath)).exists(),
                "feedback file should exist in Generations/Generation_1/");
    JobFeedBack_s fb;
    TEST_ASSERT(fb.load_with_retry(fbPath, false), "should load feedback");
    TEST_ASSERT(fb.Status == jobsta_e::STATUS_PENDDING, "feedback should be PENDING");

    // 8i. 清理
    // 8i. Cleanup
    QDir(tmpDir).removeRecursively();
}

// ============================================================================
// 9. 验证 AI3D_API 去除后 GetTaskInfo() 返回值拷贝不崩溃
//    根因: AI3D_API (__declspec(dllimport)) 导致 MSVC 认为 GenTaskParams/
//    GenTaskOptions/blk_generation_info_s 的拷贝构造在 DLL 中, 实际不存在。
//    修复: 去掉这三个 struct 的 AI3D_API, 让编译器内联生成拷贝构造。
// ============================================================================
void TestGetTaskInfoCopy()
{
    std::string tmpDir = QDir::tempPath().toStdString() + "/test_gen_task_info_copy/";
    QDir().mkpath(QString::fromStdString(tmpDir));
    std::string blkPath = tmpDir + "test.blk";

    // a. 构造含 gen_options 的 BlockObject
    AI3D::CORE::BlockObject block;
    block.SetName("test_block");
    block.SetPath(tmpDir);

    AI3D::CORE::BlockObject::Task_Info& info = block.GetTaskInfoMutual();
    info.block_task_category = 1;
    info.gen_options.gen_params.sub_type = GenTaskSubType::TEXT_TO_MODEL;
    info.gen_options.gen_params.prompt = "a red sports car";
    info.gen_options.gen_params.polygon_limit = 50000;
    info.gen_options.gen_params.texture_size = 1024;
    info.gen_options.gen_params.model_version = "v2";
    info.blockString = "test_block";
    info.projectfile_ = tmpDir;

    // b. 写 BIN, 再读回
    TEST_ASSERT(info.WriteBlockInfoToBin(blkPath), "WriteBlockInfoToBin should succeed");

    AI3D::CORE::BlockObject::Task_Info readBack;
    TEST_ASSERT(readBack.ReadBlockInfoBin(blkPath), "ReadBlockInfoBin should succeed");

    // c. ★ 关键: 通过 GetTaskInfo() 返回值触发拷贝构造
    //    如果 AI3D_API 未去除, 此行崩溃在偏移 0x8
    AI3D::CORE::BlockObject::Task_Info copied = readBack; // 显式拷贝
    (void)copied;

    // d. 验证拷贝后的数据一致
    AI3D::CORE::BlockObject block2;
    block2.SetName("test_block2");
    block2.SetPath(tmpDir);
    block2.SetTaskInfo(readBack); // 通过 BlockObject 间接触发 GetTaskInfo() 路径
    AI3D::CORE::BlockObject::Task_Info gotBack = block2.GetTaskInfo(); // ← GetTaskInfo() 返回值拷贝

    TEST_ASSERT(gotBack.block_task_category == 1, "block_task_category mismatch");
    TEST_ASSERT(gotBack.gen_options.gen_params.sub_type == GenTaskSubType::TEXT_TO_MODEL, "sub_type mismatch");
    TEST_ASSERT(gotBack.gen_options.gen_params.prompt == "a red sports car", "prompt mismatch");
    TEST_ASSERT(gotBack.gen_options.gen_params.polygon_limit == 50000, "polygon_limit mismatch");
    TEST_ASSERT(gotBack.gen_options.gen_params.texture_size == 1024, "texture_size mismatch");
    TEST_ASSERT(gotBack.blockString == "test_block", "blockName mismatch");

    // e. 清理
    // QDir(tmpDir).removeRecursively();

    LOGI("TestGetTaskInfoCopy PASSED — AI3D_API fix verified");
}

int main()
{
    TestGenTaskParamsRoundtrip();
    TestGenTaskOptionsRoundtrip();
    TestGenJobFullInfoBinRoundtrip();
    TestDefaults();
    TestBlkGenerationInfoRoundtrip();
    TestGenerationsInfoArrayRoundtrip();
    TestBlockFileGenFieldsRoundtrip();
    TestFrontendSubmitGenTask();
    TestGetTaskInfoCopy(); // ← 新增: 验证 AI3D_API 修复

    if (g_failures == 0)
    {
        LOGI("All Phase 1 tests passed.");
        return 0;
    }
    else
    {
        LOGE(std::to_string(g_failures) + " test(s) failed.");
        return 1;
    }
}
