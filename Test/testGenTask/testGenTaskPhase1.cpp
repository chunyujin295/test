#include "Core/GenTaskOptions.h"
#include "Util/GenTaskProcess.h"
#include "Core/Types.h"
#include "Core/Logging.h"

#include <cstdio>
#include <cstring>
#include <string>

#include "Core/BlockObject.h"

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
// ============================================================================
static void TestGenTaskParamsRoundtrip()
{
    // 1a. 构造完整参数
    GenTaskParams p;
    p.sub_type = GenTaskSubType::IMAGE_TO_MODEL;
    p.prompt = "a red sports car";
    p.negative_prompt = "blurry, low quality";
    p.polygon_limit = 50000;
    p.texture_size = 2048;
    p.model_version = "v2";
    p.file_key = "fk-abc123";

    // 1b. WriteToJson → rapidjson::Document
    rapidjson::Document doc;
    doc.SetObject();
    p.WriteToJson(doc, doc);

    // 1c. 验证写入的字段
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
    std::string json = p.ToJsonString();
    TEST_ASSERT(!json.empty(), "ToJsonString should not be empty");

    GenTaskParams p2 = GenTaskParams::CreateFromJsonString(json);
    TEST_ASSERT(p2.sub_type == GenTaskSubType::IMAGE_TO_MODEL, "sub_type roundtrip failed");
    TEST_ASSERT(p2.prompt == "a red sports car", "prompt roundtrip failed");
    TEST_ASSERT(p2.negative_prompt == "blurry, low quality", "negative_prompt roundtrip failed");
    TEST_ASSERT(p2.polygon_limit == 50000, "polygon_limit roundtrip failed");
    TEST_ASSERT(p2.texture_size == 2048, "texture_size roundtrip failed");
    TEST_ASSERT(p2.model_version == "v2", "model_version roundtrip failed");
    TEST_ASSERT(p2.file_key == "fk-abc123", "file_key roundtrip failed");

    // 1e. UNKNOWN sub_type 不应写入
    GenTaskParams p3;
    p3.sub_type = GenTaskSubType::UNKNOWN;
    p3.prompt = "test";
    rapidjson::Document doc3;
    doc3.SetObject();
    p3.WriteToJson(doc3, doc3);
    TEST_ASSERT(!doc3.HasMember("sub_type"), "UNKNOWN sub_type should not be written");

    // 1f. 空字符串不应写入
    GenTaskParams p4;
    p4.prompt = "";
    rapidjson::Document doc4;
    doc4.SetObject();
    p4.WriteToJson(doc4, doc4);
    TEST_ASSERT(!doc4.HasMember("prompt"), "empty prompt should not be written");
}

// ============================================================================
// 测试 2: GenTaskOptions 委托 rapidjson 往返
// ============================================================================
static void TestGenTaskOptionsRoundtrip()
{
    // 2a. 通过 GenTaskOptions 写入 (委托 gen_params)
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
    GenTaskOptions opts2;
    opts2.ParseJson(doc);
    TEST_ASSERT(opts2.gen_params.sub_type == GenTaskSubType::TEXT_TO_MODEL,
                "sub_type ParseJson failed");
    TEST_ASSERT(opts2.gen_params.prompt == "a cute cat", "prompt ParseJson failed");
    TEST_ASSERT(opts2.gen_params.polygon_limit == 10000, "polygon_limit ParseJson failed");
}

// ============================================================================
// 测试 3: GenJobFullInfo_s BIN 往返
// ============================================================================
static void TestGenJobFullInfoBinRoundtrip()
{
    // 3a. 构造完整 job + feedback
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
    info.job.server_task_id = "trv-abc123def456";
    info.job.result_url = "https://cdn.example.com/result.glb";
    info.job.preview_url = "https://cdn.example.com/preview.png";
    info.job.result_path = "/tmp/result.png";
    info.job.preview_path = "/tmp/preview.png";
    info.job.query_retry_count = 2;
    info.feedback.Status = jobsta_e::STATUS_RUNNING;
    info.feedback.Percent = 45.0f;
    info.feedback.TaskRetVal = 0;
    info.feedback.Msg = "processing mesh generation";

    // 3b. 写 BIN
    const std::string path = "test_genjob_phase1.bin";
    bool ok = info.WriteToBin(path);
    TEST_ASSERT(ok, "WriteToBin should succeed");

    // 3c. 读 BIN
    GenJobFullInfo_s loaded;
    ok = loaded.LoadFromBin(path);
    TEST_ASSERT(ok, "LoadFromBin should succeed");

    // 3d. 逐字段验证 — GenJobInfo_s
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
    TEST_ASSERT(loaded.job.server_task_id == info.job.server_task_id,
                "server_task_id mismatch");
    TEST_ASSERT(loaded.job.result_url == info.job.result_url, "result_url mismatch");
    TEST_ASSERT(loaded.job.preview_url == info.job.preview_url, "preview_url mismatch");
    TEST_ASSERT(loaded.job.result_path == info.job.result_path, "result_path mismatch");
    TEST_ASSERT(loaded.job.preview_path == info.job.preview_path, "preview_path mismatch");
    TEST_ASSERT(loaded.job.query_retry_count == info.job.query_retry_count,
                "query_retry_count mismatch");

    // 3e. 验证 feedback 字段
    TEST_ASSERT(loaded.feedback.Status == info.feedback.Status, "feedback.Status mismatch");
    TEST_ASSERT(loaded.feedback.Percent == info.feedback.Percent, "feedback.Percent mismatch");
    TEST_ASSERT(loaded.feedback.TaskRetVal == info.feedback.TaskRetVal,
                "feedback.TaskRetVal mismatch");
    TEST_ASSERT(loaded.feedback.Msg == info.feedback.Msg, "feedback.Msg mismatch");

    // 3f. 清理
    std::remove(path.c_str());
}

// ============================================================================
// 测试 4: 默认值 / 旧数据兼容
// ============================================================================
static void TestDefaults()
{
    // 4a. GenTaskParams 默认构造
    GenTaskParams p;
    TEST_ASSERT(p.sub_type == GenTaskSubType::UNKNOWN, "default sub_type should be UNKNOWN");
    TEST_ASSERT(p.prompt.empty(), "default prompt should be empty");
    TEST_ASSERT(p.polygon_limit == 0, "default polygon_limit should be 0");
    TEST_ASSERT(p.texture_size == 0, "default texture_size should be 0");

    // 4b. 空 JSON 字符串 → 返回默认值
    GenTaskParams p2 = GenTaskParams::CreateFromJsonString("");
    TEST_ASSERT(p2.sub_type == GenTaskSubType::UNKNOWN, "empty json should give UNKNOWN");
    TEST_ASSERT(p2.prompt.empty(), "empty json should give empty prompt");

    // 4c. 不完整 JSON → 缺失字段保持默认值
    GenTaskParams p3 = GenTaskParams::CreateFromJsonString("{\"prompt\":\"hello\"}");
    TEST_ASSERT(p3.prompt == "hello", "prompt should be parsed");
    TEST_ASSERT(p3.sub_type == GenTaskSubType::UNKNOWN, "missing sub_type should be UNKNOWN");
    TEST_ASSERT(p3.polygon_limit == 0, "missing polygon_limit should be 0");

    // 4d. GenTaskOptions 默认构造
    GenTaskOptions opts;
    TEST_ASSERT(opts.gen_params.sub_type == GenTaskSubType::UNKNOWN,
                "GenTaskOptions default sub_type should be UNKNOWN");
}

// ============================================================================
// 测试 5: blk_generation_info_s rapidjson 往返 (写入 .blk 的子结构)
// ============================================================================
static void TestBlkGenerationInfoRoundtrip()
{
    // 5a. 构造完整的生成结果元数据
    blk_generation_info_s info;
    info.task_uuid = "uuid-test-12345";
    info.job_name = "J_TestBlock_20260129120000";
    info.sub_type = static_cast<int>(GenTaskSubType::TEXT_TO_MODEL);
    info.status = static_cast<int>(GenTaskStatus::COMPLETED);
    info.preview_url = "https://cdn.example.com/preview.png";
    info.result_url = "https://cdn.example.com/result.glb";
    info.preview_path = "preview.png";
    info.result_path = "result.png";
    info.created_time = "20260129120000";

    // 5b. 写入 rapidjson Document (模拟 WriteBlockInfoToJson 中的一个数组元素)
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Value genJson(rapidjson::kObjectType);
    info.CreateJson(genJson, doc);
    doc.AddMember("item", genJson, doc.GetAllocator());

    // 5c. 验证写入
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
    blk_generation_info_s loaded;
    loaded.ParseJson(item);
    TEST_ASSERT(loaded.task_uuid == info.task_uuid, "ParseJson task_uuid failed");
    TEST_ASSERT(loaded.job_name == info.job_name, "ParseJson job_name failed");
    TEST_ASSERT(loaded.sub_type == info.sub_type, "ParseJson sub_type failed");
    TEST_ASSERT(loaded.status == info.status, "ParseJson status failed");
    TEST_ASSERT(loaded.preview_url == info.preview_url, "ParseJson preview_url failed");
    TEST_ASSERT(loaded.result_url == info.result_url, "ParseJson result_url failed");
    TEST_ASSERT(loaded.preview_path == info.preview_path, "ParseJson preview_path failed");
    TEST_ASSERT(loaded.result_path == info.result_path, "ParseJson result_path failed");
    TEST_ASSERT(loaded.created_time == info.created_time, "ParseJson created_time failed");
}

// ============================================================================
// 测试 6: generations_info_ JSON 数组往返 (模拟 .blk 中 generations_info 段的读写)
// ============================================================================
static void TestGenerationsInfoArrayRoundtrip()
{
    // 6a. 构造两个条目
    blk_generation_info_s gen1;
    gen1.task_uuid = "uuid-1";
    gen1.job_name = "J_Block_001";
    gen1.sub_type = static_cast<int>(GenTaskSubType::TEXT_TO_MODEL);
    gen1.status = static_cast<int>(GenTaskStatus::COMPLETED);
    gen1.result_url = "https://cdn.example.com/result1.glb";
    gen1.preview_path = "preview.png";

    blk_generation_info_s gen2;
    gen2.task_uuid = "uuid-2";
    gen2.job_name = "J_Block_002";
    gen2.sub_type = static_cast<int>(GenTaskSubType::IMAGE_TO_MODEL);
    gen2.status = static_cast<int>(GenTaskStatus::IN_PROGRESS);

    // 6b. 写入 JSON 数组 (模拟 WriteBlockInfoToJson 中 generationsInfo 段)
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
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    std::string jsonStr = buffer.GetString();

    rapidjson::Document doc2;
    TEST_ASSERT(!doc2.Parse(jsonStr.c_str()).HasParseError() && doc2.IsArray(),
                "generations_info array should parse");
    TEST_ASSERT(doc2.Size() == 2, "should have 2 entries");

    // 6d. 读回
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
    std::string jobEntry = gen1.task_uuid + ":" + gen1.job_name;
    // 解析:
    size_t colonPos = jobEntry.find(":");
    std::string parsed_uuid = jobEntry.substr(0, colonPos);
    std::string parsed_job = jobEntry.substr(colonPos + 1);
    TEST_ASSERT(parsed_uuid == "uuid-1", "generationjobs uuid mismatch");
    TEST_ASSERT(parsed_job == "J_Block_001", "generationjobs job_name mismatch");
}

// ============================================================================
// 测试 7: Block 文件落地 → 重新加载 (端到端验证 .blk 中 gen 字段读写)
// ============================================================================
static void TestBlockFileGenFieldsRoundtrip(bool useBin)
{
    // 7a. 构造一个含生成式字段的 Task_Info
    BlockObject::Task_Info info;
    info.blockName = "TestGenBlock";
    info.projectfile_ = "/tmp/test_project";
    info.blockString = "test_block_string";
    info.blockId = 999;
    info.block_task_category = 1; // 生成式

    // gen_options
    info.gen_options.gen_params.sub_type = GenTaskSubType::TEXT_TO_MODEL;
    info.gen_options.gen_params.prompt = "a test cube";
    info.gen_options.gen_params.polygon_limit = 10000;
    info.gen_options.gen_params.texture_size = 512;

    // generations_info_
    blk_generation_info_s gen1;
    gen1.task_uuid = "uuid-blk-test-1";
    gen1.job_name = "J_TestGenBlock_20260129000000";
    gen1.sub_type = static_cast<int>(GenTaskSubType::TEXT_TO_MODEL);
    gen1.status = static_cast<int>(GenTaskStatus::COMPLETED);
    gen1.result_url = "https://cdn.example.com/result.glb";
    gen1.preview_url = "https://cdn.example.com/thumb.png";
    gen1.result_path = "/tmp/test_genblock/result.png";
    gen1.preview_path = "/tmp/test_genblock/preview.png";
    gen1.created_time = "20260129000000";
    info.generations_info_.push_back(gen1);

    blk_generation_info_s gen2;
    gen2.task_uuid = "uuid-blk-test-2";
    gen2.job_name = "J_TestGenBlock_20260129010000";
    gen2.sub_type = static_cast<int>(GenTaskSubType::IMAGE_TO_MODEL);
    gen2.status = static_cast<int>(GenTaskStatus::IN_PROGRESS);
    gen2.created_time = "20260129010000";
    info.generations_info_.push_back(gen2);

    // generationjobs_
    info.generationjobs_[gen1.task_uuid] = gen1.job_name;
    info.generationjobs_[gen2.task_uuid] = gen2.job_name;

    // 7b. 落地文件
    std::string filePath = std::string("test_genblock") + (useBin ? ".bbin" : ".json");
    // std::string filePath = std::string("D:\\Project\\NewProject\\Block_6\\Block_6.bbin");
    bool ok;
    if (useBin)
        ok = info.WriteBlockInfoToBin(filePath, false);
    else
        ok = info.WriteBlockInfoToJson(filePath, false);
    TEST_ASSERT(ok, (useBin ? "WriteBlockInfoToBin failed" : "WriteBlockInfoToJson failed"));

    // 7c. 重新加载
    BlockObject::Task_Info loaded;
    if (useBin)
        ok = loaded.ReadBlockInfoBin(filePath);
    else
        ok = loaded.ReadBlockInfoJson(filePath);
    TEST_ASSERT(ok, (useBin ? "ReadBlockInfoBin failed" : "ReadBlockInfoJson failed"));

    // 7d. 验证 block_task_category
    TEST_ASSERT(loaded.block_task_category == 1, "block_task_category roundtrip failed");

    // 7e. 验证 gen_options.gen_params
    TEST_ASSERT(loaded.gen_options.gen_params.sub_type == GenTaskSubType::TEXT_TO_MODEL,
                "sub_type roundtrip failed");
    TEST_ASSERT(loaded.gen_options.gen_params.prompt == "a test cube",
                "prompt roundtrip failed");
    TEST_ASSERT(loaded.gen_options.gen_params.polygon_limit == 10000,
                "polygon_limit roundtrip failed");
    TEST_ASSERT(loaded.gen_options.gen_params.texture_size == 512,
                "texture_size roundtrip failed");

    // 7f. 验证 generations_info_
    TEST_ASSERT(loaded.generations_info_.size() == 2,
                "generations_info_ size mismatch");
    TEST_ASSERT(loaded.generations_info_[0].task_uuid == "uuid-blk-test-1",
                "generations_info_[0].task_uuid mismatch");
    TEST_ASSERT(loaded.generations_info_[0].status == static_cast<int>(GenTaskStatus::COMPLETED),
                "generations_info_[0].status mismatch");
    TEST_ASSERT(loaded.generations_info_[0].result_url == "https://cdn.example.com/result.glb",
                "generations_info_[0].result_url mismatch");
    TEST_ASSERT(loaded.generations_info_[0].result_path == "/tmp/test_genblock/result.png",
                "generations_info_[0].result_path mismatch");
    TEST_ASSERT(loaded.generations_info_[0].preview_path == "/tmp/test_genblock/preview.png",
                "generations_info_[0].preview_path mismatch");
    TEST_ASSERT(loaded.generations_info_[1].task_uuid == "uuid-blk-test-2",
                "generations_info_[1].task_uuid mismatch");
    TEST_ASSERT(loaded.generations_info_[1].status == static_cast<int>(GenTaskStatus::IN_PROGRESS),
                "generations_info_[1].status mismatch");

    // 7g. 验证 generationjobs_
    TEST_ASSERT(loaded.generationjobs_.size() == 2,
                "generationjobs_ size mismatch");
    TEST_ASSERT(loaded.generationjobs_.count("uuid-blk-test-1") == 1,
                "generationjobs_ uuid-1 missing");
    TEST_ASSERT(loaded.generationjobs_.at("uuid-blk-test-1") == "J_TestGenBlock_20260129000000",
                "generationjobs_ uuid-1 job_name mismatch");
    TEST_ASSERT(loaded.generationjobs_.count("uuid-blk-test-2") == 1,
                "generationjobs_ uuid-2 missing");

    // 7h. 清理
    // std::remove(filePath.c_str());
}

static void TestBlockFileGenFieldsRoundtrip2(bool useBin)
{
    // 7a. 构造一个含生成式字段的 Task_Info
    BlockObject::Task_Info info;

    // 读取文件
    // std::string filePath = std::string("test_genblock") + (useBin ? ".bbin" : ".json");
    std::string filePath = std::string("D:\\Project\\ProJectForGenerationTest\\Block_2\\Block_2.bbin");
    bool ok;
    if (useBin)
        ok = info.ReadBlockInfoBin(filePath);
    else
        ok = info.ReadBlockInfoJson(filePath);
    TEST_ASSERT(ok, (useBin ? "ReadBlockInfoBin failed" : "ReadBlockInfoJson failed"));

    // 7b. 更新文件
    info.blockName = "TestGenBlock";
    info.blockString = "TestGenBlockString";
    info.block_task_category = 0; // 生成式
    // info.block_task_category = 1; // 生成式

    // gen_options
    info.gen_options.gen_params.sub_type = GenTaskSubType::TEXT_TO_MODEL;
    info.gen_options.gen_params.prompt = "a test cube";
    info.gen_options.gen_params.polygon_limit = 10000;
    info.gen_options.gen_params.texture_size = 512;

    // generations_info_
    blk_generation_info_s gen1;
    gen1.task_uuid = "uuid-blk-test-1";
    gen1.job_name = "J_TestGenBlock_20260129000000";
    gen1.sub_type = static_cast<int>(GenTaskSubType::TEXT_TO_MODEL);
    gen1.status = static_cast<int>(GenTaskStatus::COMPLETED);
    gen1.result_url = "https://cdn.example.com/result.glb";
    gen1.preview_url = "https://cdn.example.com/thumb.png";
    gen1.result_path = "/tmp/test_genblock/result.png";
    gen1.preview_path = "/tmp/test_genblock/preview.png";
    gen1.created_time = "20260129000000";
    info.generations_info_.push_back(gen1);

    blk_generation_info_s gen2;
    gen2.task_uuid = "uuid-blk-test-2";
    gen2.job_name = "J_TestGenBlock_20260129010000";
    gen2.sub_type = static_cast<int>(GenTaskSubType::IMAGE_TO_MODEL);
    gen2.status = static_cast<int>(GenTaskStatus::IN_PROGRESS);
    gen2.created_time = "20260129010000";
    info.generations_info_.push_back(gen2);

    // generationjobs_
    info.generationjobs_[gen1.task_uuid] = gen1.job_name;
    info.generationjobs_[gen2.task_uuid] = gen2.job_name;

    // 保存
    if (useBin)
        ok = info.WriteBlockInfoToBin(filePath, false);
    else
        ok = info.WriteBlockInfoToJson(filePath, false);
    TEST_ASSERT(ok, (useBin ? "WriteBlockInfoToBin failed" : "WriteBlockInfoToJson failed"));

    // 7c. 重新加载
    BlockObject::Task_Info loaded;
    if (useBin)
        ok = loaded.ReadBlockInfoBin(filePath);
    else
        ok = loaded.ReadBlockInfoJson(filePath);
    TEST_ASSERT(ok, (useBin ? "ReadBlockInfoBin failed" : "ReadBlockInfoJson failed"));

    // 7d. 验证 block_task_category
    TEST_ASSERT(loaded.block_task_category == 0, "block_task_category roundtrip failed");

    // 7e. 验证 gen_options.gen_params
    TEST_ASSERT(loaded.gen_options.gen_params.sub_type == GenTaskSubType::TEXT_TO_MODEL,
                "sub_type roundtrip failed");
    TEST_ASSERT(loaded.gen_options.gen_params.prompt == "a test cube",
                "prompt roundtrip failed");
    TEST_ASSERT(loaded.gen_options.gen_params.polygon_limit == 10000,
                "polygon_limit roundtrip failed");
    TEST_ASSERT(loaded.gen_options.gen_params.texture_size == 512,
                "texture_size roundtrip failed");

    // 7f. 验证 generations_info_
    TEST_ASSERT(loaded.generations_info_.size() == 2,
                "generations_info_ size mismatch");
    TEST_ASSERT(loaded.generations_info_[0].task_uuid == "uuid-blk-test-1",
                "generations_info_[0].task_uuid mismatch");
    TEST_ASSERT(loaded.generations_info_[0].status == static_cast<int>(GenTaskStatus::COMPLETED),
                "generations_info_[0].status mismatch");
    TEST_ASSERT(loaded.generations_info_[0].result_url == "https://cdn.example.com/result.glb",
                "generations_info_[0].result_url mismatch");
    TEST_ASSERT(loaded.generations_info_[0].result_path == "/tmp/test_genblock/result.png", "generations_info_[0].result_path mismatch");
    TEST_ASSERT(loaded.generations_info_[0].preview_path == "/tmp/test_genblock/preview.png","generations_info_[0].preview_path mismatch");
    TEST_ASSERT(loaded.generations_info_[1].task_uuid == "uuid-blk-test-2",
                "generations_info_[1].task_uuid mismatch");
    TEST_ASSERT(loaded.generations_info_[1].status == static_cast<int>(GenTaskStatus::IN_PROGRESS),
                "generations_info_[1].status mismatch");

    // 7g. 验证 generationjobs_
    TEST_ASSERT(loaded.generationjobs_.size() == 2,
                "generationjobs_ size mismatch");
    TEST_ASSERT(loaded.generationjobs_.count("uuid-blk-test-1") == 1,
                "generationjobs_ uuid-1 missing");
    TEST_ASSERT(loaded.generationjobs_.at("uuid-blk-test-1") == "J_TestGenBlock_20260129000000",
                "generationjobs_ uuid-1 job_name mismatch");
    TEST_ASSERT(loaded.generationjobs_.count("uuid-blk-test-2") == 1,
                "generationjobs_ uuid-2 missing");

    // 7h. 清理
    // std::remove(filePath.c_str());
}


static void TestBlockFileGenFieldsRoundtrip3(bool useBin)
{
    // 读取文件
    // std::string filePath = std::string("test_genblock") + (useBin ? ".bbin" : ".json");
    std::string filePath = std::string("D:\\Project\\NewProject_for_Generation\\Block_3\\Block_3.bbin");
    bool ok;

    BlockObject::Task_Info loaded;
    if (useBin)
        ok = loaded.ReadBlockInfoBin(filePath);
    else
        ok = loaded.ReadBlockInfoJson(filePath);

    // DEBUG查看空项目的加载情况
    int a = 0;

    // 7h. 清理
    // std::remove(filePath.c_str());
}

static void TestBlockObjectTastInfo()
{
    AI3D::CORE::BlockObject block;
    block.SetName("test");
    block.SetPath("/test/test");

    AI3D::CORE::BlockObject::Task_Info& info  = block.GetTaskInfoMutual();
    info.block_task_category  =1;
    info.gen_options.gen_params.sub_type = GenTaskSubType::IMAGE_TO_MESH;
    info.blockName = "test_block";
    info.projectfile_ = "testPath";

    AI3D::CORE::BlockObject::Task_Info testInfo = block.GetTaskInfo();
    int a = 0;
}

int main()
{
    TestGenTaskParamsRoundtrip();
    TestGenTaskOptionsRoundtrip();
    TestGenJobFullInfoBinRoundtrip();
    TestDefaults();
    TestBlkGenerationInfoRoundtrip();
    TestGenerationsInfoArrayRoundtrip();
    // TestBlockFileGenFieldsRoundtrip(false); // JSON 文件落地 → 加载
    // TestBlockFileGenFieldsRoundtrip(true); // BIN  文件落地 → 加载
    TestBlockFileGenFieldsRoundtrip2(true); // BIN  文件落地 → 加载
    // TestBlockFileGenFieldsRoundtrip3(true); // BIN  原版重建式block文件加载
    TestBlockObjectTastInfo();

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
