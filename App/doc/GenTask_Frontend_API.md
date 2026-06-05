# GenTask 前端 API 文档

## 核心 API

所有接口在 `AI3D::CORE::GenTaskAPI` 中，编译进 `MoldAIData.dll`。

### 提交任务

```cpp
#include "Core/GenTaskAPI.h"

// 1. 获取 pendingPath
std::string pendingPath = qstr2str(Settings::getGenEngineJobQueue()) + "/Pending/";

// 2. 设置 Block 参数
BlockObject::Task_Info& task = block->GetTaskInfoMutual();
task.block_task_category = 1;                            // 标记为生成式 Block
task.gen_options.gen_params.sub_type      = GenTaskSubType::TEXT_TO_MODEL;
task.gen_options.gen_params.prompt        = "a red sports car";
task.gen_options.gen_params.polygon_limit = 50000;
task.gen_options.gen_params.texture_size  = 1024;

// 3. 提交 (generation_id 内部自动推算, 无需传参)
SubmitResult result = GenTaskAPI::SubmitGenTask(task, currentUserAccount, pendingPath);
if (result.success) {
    // result.generation_id = 分配的 id, result.job_name 用于匹配
}
```

### 查询积分

```cpp
int credits = GenTaskAPI::QueryCredits("user@example.com");
```

### 下载结果

```cpp
// 方式 1: 通过 task_uuid 自动定位 result_dir 下载 (推荐)
//         支持进度回调, 结果保存在 Generations/Generation_<id>/result.glb
GenTaskAPI::DownloadResultByTaskUuid(task_uuid, blockInfo,
    [](qint64 received, qint64 total) {
        int pct = total > 0 ? (int)(received * 100 / total) : 0;
        // 更新 UI 进度条
    });

// 方式 2: 通过 task_uuid 获取 result_dir
std::string resultDir = GenTaskAPI::GetResultDir(task_uuid, blockInfo);

// 方式 3: 指定 URL 和路径直接下载 (保留)
GenTaskAPI::DownloadResult(result_url, customSavePath);
```

---

## 数据结构

### GenTaskSubType — 任务类型枚举

| 枚举值 | URL 路径段 | 说明 |
|--------|----------|------|
| `TEXT_TO_MODEL` | text-to-model | 文字→带纹理模型 |
| `TEXT_TO_MESH` | text-to-mesh | 文字→纯白模 |
| `IMAGE_TO_MODEL` | image-to-model | 图片→带纹理模型 |
| `IMAGE_TO_MESH` | image-to-mesh | 图片→纯白模 |
| `TEXTURE_MODEL` | texture-model | 图片+模型→纹理 |
| `TEXT_TO_TEXTURE` | text-to-texture | 文字+模型→纹理 |
| `MODEL_PREVIEW_RENDER` | model-preview-render | 模型→预览图 |
| `MODEL_REMESH` | model-remesh | 重网格/减面 |
| `CONVERT_MODEL_FORMAT` | convert-model-format | 格式转换 |
| `IMAGE_GENERATION` | image-generation | 文字→图片 |

### GenTaskParams — 生成参数

```cpp
struct GenTaskParams {
    GenTaskSubType  sub_type;         // 任务类型
    std::string prompt;               // 正向提示词
    std::string negative_prompt;      // 反向提示词
    int         polygon_limit = 0;    // 面数限制 (0=不限制)
    int         texture_size = 0;     // 纹理分辨率 (0=默认)
    int         provider_id = 0;      // 供应商类型 (默认 0)
    std::string model_version;        // 模型版本 (空=默认)
    std::string file_key;             // 已上传文件的 key (纯文字任务留空)
};
```

### SubmitResult — 提交返回值

```cpp
struct SubmitResult {
    std::string task_uuid;     // 全局唯一, 取消任务时用
    std::string job_name;      // J_BlockName_timestamp, 在 generations_info_ 中匹配
    int         generation_id; // 分配的 generation id (前端用于更新 Block)
    bool        success;       // 是否成功
    std::string error_msg;     // 失败原因
};
```

---

## 前端对接流程

### 1. 提交生成任务

对照重建式 `Sig_NewProductionStarted` 的流程：

```cpp
void Frontend::SubmitGeneration(BlockObject* block, const GenTaskParams& params) {
    BlockObject::Task_Info& task = block->GetTaskInfoMutual();
    task.block_task_category = 1;
    task.gen_options.gen_params = params;

    std::string pendingPath = qstr2str(Settings::getGenEngineJobQueue()) + "/Pending/";
    auto result = GenTaskAPI::SubmitGenTask(task, currentUserAccount, pendingPath);

    if (result.success) {
        // 任务已进入 jobs_gen/Pending/, 等待 GenTaskThread 处理
        // result.job_name 用于后续匹配
    }
}
```

### 2. 监控任务状态

前端遍历每个 Block 的 `generations_info_` 获取所有生成任务：

```cpp
// 读 Block 文件
BlockObject::Task_Info blkInfo;
blkInfo.ReadBlockInfoBin(blkPath);  // 或 ReadBlockInfoJson

for (auto& gen : blkInfo.generations_info_) {
    GenTaskStatus status = static_cast<GenTaskStatus>(gen.status);
    
    if (status == GenTaskStatus::PENDING || status == GenTaskStatus::IN_PROGRESS) {
        // 任务进行中, 读 feedback 获取进度
        // feedback 放在 Generations/Generation_<id>/ 下, result_dir 已包含完整路径
        std::string fbPath = gen.result_dir + "/JF_" + gen.job_name + ".bin";
        JobFeedBack_s fb;
        fb.load_with_retry(fbPath, false);
        int progress = static_cast<int>(fb.Percent);
        // 更新 UI 进度条
    }
    
    if (status == GenTaskStatus::COMPLETED && !gen.result_url.empty()) {
        // 自动下载到 Generations/Generation_<id>/result.glb
        GenTaskAPI::DownloadResultByTaskUuid(gen.task_uuid, blkInfo,
            [](qint64 received, qint64 total) { /* 进度 */ });
    }
    
    if (status == GenTaskStatus::FAILED) {
        // 失败, gen.error_message 可能有错误信息
    }
}
```

### 3. 下载结果

```cpp
if (static_cast<GenTaskStatus>(gen.status) == GenTaskStatus::COMPLETED) {
    std::string savePath = downloadDir + "/" + gen.job_name + "_result.glb";
    GenTaskAPI::DownloadResult(gen.result_url, savePath);
}
```

### 4. 更新 UI 树节点

对标重建式的 `Sig_NewProduction` → 前端树节点追加：

```cpp
// 注册新的生成结果到 Block
blk_generation_info_s genInfo;
genInfo.generation_id = result.generation_id;
genInfo.task_uuid     = result.task_uuid;
genInfo.job_name      = result.job_name;
genInfo.sub_type      = static_cast<int>(params.sub_type);
genInfo.status        = static_cast<int>(GenTaskStatus::PENDING);
genInfo.result_dir    = projectPath + "/" + blockName + "/" GENERATION_DIR "/" GENERATION_PREFIX + std::to_string(result.generation_id);
genInfo.created_time  = QDateTime::currentDateTime().toString("yyyyMMddhhmmss").toStdString();

// 写入 Block
blkInfo.generations_info_.push_back(genInfo);
blkInfo.generationjobs_[result.task_uuid] = result.job_name;
blkInfo.WriteBlockInfoToBin(blkPath, false);

// 树节点追加 (对标 NewBlock → NewProduction)
QStandardItem* item = new QStandardItem(icon, str2qstr(result.job_name));
item->setData(AI3D::GUI::ItemType::ITGeneration, AI3D::GUI::CustomRole::CRItemType);
item->setData(QVariant::fromValue(block), AI3D::GUI::CustomRole::CRParentBlockData);
blockItem->appendRow(item);
```

---

## 状态流转

```
前端提交 SubmitGenTask → job 进 Pending/
                           │
              GenTaskThread 扫描 Pending/
                  ├─ submit 到服务端
                  ├─ RegisterBlockGenResult (PENDING)
                  └─ 移到 Running/
                           │
              GenTaskThread 轮询 Running/
                  ├─ IN_PROGRESS → 更新 feedback.Percent
                  ├─ COMPLETED → 更新 generations_info_.status/result_url
                  │             更新 feedback → COMPLETE
                  │             移到 Completed/
                  └─ FAILED → 更新 generations_info_.status
                                更新 feedback → FAILED
                                移到 Failed/
```

前端只需读 Block 的 `generations_info_` 和对应的 feedback 文件即可获取完整状态。

---

## 与重建式对照

| 操作 | 重建式 | 生成式 |
|------|--------|--------|
| 提交任务 | `ReconstructionCommandSet::SubmitProduction` | `GenTaskAPI::SubmitGenTask` |
| 任务目录 | `jobs/Pending/High/` | `jobs_gen/Pending/` |
| 结果列表 | `reconstructions_info_` | `generations_info_` |
| 进度反馈 | `JF_*.bin` (JobFeedBack_s) | `JF_*.bin` (复用, 不改结构) |
| 树节点 | `Sig_NewProduction` | 读 `generations_info_` 后自行追加 |
| 下载结果 | 本地文件 | `GenTaskAPI::DownloadResult` |
