# 生成式任务数据结构整合方案

## 一、整合思路总览

核心原则: **最小侵入现有代码, 在 Block 层做分叉**。

```
                                    ┌─── block_task_category == 0 (现有) ───→ jobs/      → 本地 Task.exe
Project → Block ───block_task_category│
                                    └─── block_task_category == 1 (新增) ───→ jobs_gen/  → HTTP 服务端
```

Block 是整个系统的"工作单元"。目前 Block 只支持本地计算。增加 `block_task_category` 字段后, Block 的分类决定:
- 创建 Job 时写入哪个目录 (`jobs/` vs `jobs_gen/`)
- Job 文件使用哪套数据结构 (TaskGraph_s vs GenJobInfo)
- Node 哪个线程处理 (searchPendingJobThread2 vs generateThread)

---

## 二、现有数据结构改动

### 2.1 BlockObject::Task_Info — 增加生成式标记

文件: [BlockObject.h:81-151](Include/Core/BlockObject.h#L81-L151)

```cpp
// === 修改: BlockObject::Task_Info 结构体 ===

struct Task_Info
{
    // ===== 现有字段 (保持不变) =====
    bool hasstatisinfo = false;
    bool hasatsetting = false;
    std::string blockName;
    std::string blockString;
    std::string mergedFrom;
    int  blockId;
    // ... 其余现有字段 ...

    // ===== 新增字段 =====
    int  block_task_category = 0;  // 0=本地计算任务, 1=生成式任务
                                   // 默认 0, 向后兼容
};
```

**影响范围**:
- `WriteBlockInfoToJson()` — 序列化时写入 `"BlockTaskCategory": 0/1`
- `ReadBlockInfoJson()` — 反序列化时读取, 若 JSON 中不存在该字段则保持默认值 0
- `WriteBlockInfoToBin()` — BIN 序列化时增加一个 int 字段
- `ReadBlockInfoBin()` — BIN 反序列化时读取

### 2.2 JobInfo_s — 增加任务分类字段

文件: [TaskProcess.h:266-403](Include/Util/TaskProcess.h#L266-L403)

```cpp
// === 修改: JobInfo_s ===

struct JobInfo_s
{
    // ===== 现有字段 =====
    std::string ProjectPath = "";
    std::string ItemPath = "";
    std::string ProjectPath2 = "";  // GBK 备份 (历史遗留)
    std::string ItemPath2 = "";

    // ===== 新增字段 =====
    int  task_category = 0;  // 0=本地计算, 1=生成式 (继承自 Block)

    // ===== 序列化方法需要更新 =====
    // WriteToJson()     — 增加 "TaskCategory"
    // WriteToJson2()    — 增加 RapidJSON 写入
    // WriteToJson3()    — 增加 RapidJSON Value 写入
    // CreateFromJson()  — 增加 nlohmann 读取
    // CreateFromJsonV2()— 增加 RapidJSON 读取
    // CreateFromJsonV3()— 增加 RapidJSON Value 读取
};
```

**为什么放在 JobInfo_s 而不是其他地方**:
- `JobInfo_s` 是 `TaskGraph_s` 的成员 (`tg.job`)
- `TaskGraph_s` 是 `JobFullInfo_s` 的成员
- JobFullInfo_s 的 save/load 构成了 job 文件的核心
- Node 读取 pending job 后, 可以通过 `job.tg.job.task_category` 判断路由

### 2.3 JobFeedBack_s — 扩展以容纳服务端返回信息

文件: [TaskProcess.h:1422-2096](Include/Util/TaskProcess.h#L1422-L2096)

```cpp
// === 修改: JobFeedBack_s ===

struct JobFeedBack_s
{
    // ===== 现有字段 (保持不变) =====
    jobsta_e Status = jobsta_e::STATUS_PENDDING;
    float Percent = 0.0f;
    std::string Msg = "";
    std::string Msg2 = "";
    int TaskRetVal = -1;

    // ===== 新增字段 (生成式任务专用) =====
    std::string result_url;        // 结果下载链接
    std::string preview_url;       // 预览图链接
    std::string server_task_id;    // 服务端分配的任务ID
    std::string error_message;     // 详细错误信息
    int cost_credits = 0;          // 本次消耗积分
    int points_balance = 0;        // 积分余额

    // ===== 序列化更新 =====
    // WriteToJson() — 所有新字段序列化
    // WriteToJsonV2() — RapidJSON 版本
    // CreateFromJson() / CreateFromJsonV2() — 反序列化
    // WriteToBin() / LoadFeedbackBin() — BIN 格式也需更新
};
```

**设计考量**: 选择扩展 `JobFeedBack_s` 而非新建结构的原因是:
1. 前端读取 feedback 的代码已有, 扩展字段不会破坏现有逻辑
2. 新增字段对本地任务无影响 (保持默认值)
3. 保持 feedback 文件的统一读写路径

---

## 三、新增文件与数据结构

### 3.1 文件清单

```
Include/Util/GenTaskProcess.h     ← 生成式任务数据结构定义 (用户提供的结构体)
App/Engine/GenHttpClient.h    ← HTTP 客户端 (同步请求)
App/Engine/GenHttpClient.cpp  
App/Engine/GenTaskThread.h    ← generateThread 调度线程
App/Engine/GenTaskThread.cpp  
Include/Core/GenTaskAPI.h     ← SDK 对外接口
Src/Core/GenTaskAPI.cpp       
```

### 3.2 GenTaskProcess.h — 整合用户的枚举与结构体

```cpp
// === 新文件: Include/Util/GenTaskProcess.h ===

#pragma once
#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

namespace AI3D { namespace GEN {

// ===== 枚举 (直接采用用户定义) =====

enum class GenTaskCategory {
    TEXT_TO_3D        = 0,
    IMAGE_TO_3D       = 1,
    TEXTURING         = 2,
    UTILITY           = 3,
    IMAGE_GENERATION  = 4,
};

enum class GenTaskSubType {
    TEXT_TO_MODEL           = 0,   // POST /tasks/text-to-model
    TEXT_TO_MESH            = 1,   // POST /tasks/text-to-mesh
    IMAGE_TO_MODEL          = 2,   // POST /tasks/image-to-model
    IMAGE_TO_MESH           = 3,   // POST /tasks/image-to-mesh
    TEXTURE_MODEL           = 4,   // POST /tasks/texture-model
    TEXT_TO_TEXTURE         = 5,   // POST /tasks/text-to-texture
    MODEL_PREVIEW_RENDER    = 6,   // POST /tasks/model-preview-render
    MODEL_REMESH            = 7,   // POST /tasks/model-remesh
    CONVERT_MODEL_FORMAT    = 8,   // POST /tasks/convert-model-format
    IMAGE_GENERATION        = 9,   // POST /tasks/image-generation
};

enum class GenTaskStatus {
    IDLE        = 0,
    PENDING     = 1,
    IN_PROGRESS = 2,
    COMPLETED   = 3,
    FAILED      = 4,
    CANCELLED   = 5,
};

enum class AssetKind {
    NONE      = 0,
    FILE_PATH = 1,
    FILE_KEY  = 2,
    URL       = 3,
};

// ===== 结构体 (直接采用用户定义) =====

struct AssetRef {
    AssetKind   kind = AssetKind::NONE;
    std::string value;          // 本地路径 / file_key / URL
    std::string content_type;   // MIME, 如 "image/png"

    nlohmann::json toJson() const;
    static AssetRef fromJson(const nlohmann::json& j);
};

struct GenerationParams {
    GenTaskCategory category = GenTaskCategory::TEXT_TO_3D;
    GenTaskSubType  sub_type = GenTaskSubType::TEXT_TO_MODEL;

    std::optional<std::string> prompt;
    std::optional<std::string> negative_prompt;
    std::optional<std::string> style;

    std::optional<AssetRef>    image_file;
    std::optional<AssetRef>    model_file;

    std::optional<std::string> model_version;
    std::optional<int>         polygon_limit;
    std::optional<int>         texture_size;

    // 预览渲染
    std::optional<std::string>              render_mode;
    std::optional<int>                      image_count;
    std::optional<std::vector<std::string>> camera_angles;
    std::optional<std::string>              preset_name;

    // 重网格
    std::optional<int>         target_poly_count;

    // 格式转换
    std::optional<std::string> output_format;

    nlohmann::json toJson() const;
    static GenerationParams fromJson(const nlohmann::json& j);
};

struct GenTaskRequest {
    std::string     task_id;        // 客户端 UUID
    std::string     engine_id;      // 引擎标识
    std::string     user_account;   // 用户账号
    GenerationParams params;

    nlohmann::json toJson() const;
};

struct GenTaskResponse {
    std::string                task_id;
    std::optional<std::string> triverse_task_uuid;
    GenTaskStatus              status = GenTaskStatus::IDLE;
    int                        progress = 0;
    std::optional<std::string> result_url;
    std::optional<std::string> preview_url;
    std::optional<std::string> error_message;
    int                        cost_credits = 0;
};

// ===== GenJobInfo: 生成式 Job 文件的顶层结构 =====
// 这是写入 jobs_gen/ 目录下的 Job 文件的数据结构

struct GenJobInfo {
    // 基础标识
    std::string     task_uuid;        // 客户端生成的唯一ID
    std::string     job_name;         // Job 名称
    std::string     engine_id;        // 引擎标识
    std::string     user_account;     // 用户账号

    // 任务参数
    GenTaskCategory  category;
    GenTaskSubType   sub_type;
    GenerationParams params;

    // 服务端信息 (submit 后回填)
    std::string     server_task_id;
    std::string     triverse_task_uuid;

    // 结果
    std::string     result_url;
    std::string     preview_url;

    // 状态
    GenTaskStatus   status = GenTaskStatus::IDLE;
    int             query_retry_count = 0;

    // 积分
    int             cost_credits = 0;

    // 时间戳
    std::string     submit_time;
    std::string     complete_time;

    // Block 关联 (路由用)
    std::string     project_path;     // 对应现有 JobInfo_s::ProjectPath
    std::string     block_item;       // 对应现有 JobInfo_s::ItemPath

    // === 序列化 ===
    nlohmann::json toJson() const;
    static GenJobInfo fromJson(const nlohmann::json& j);

    bool save(const std::string& filePath) const;
    bool load(const std::string& filePath);
};
```

### 3.3 与现有 jobsta_e 的映射

`GenTaskStatus` 和 `jobsta_e` 在 feedback 文件中需要映射。在扩展后的 `JobFeedBack_s` 中:

| GenTaskStatus | jobsta_e (写入 feedback) | 目录位置 |
|--------------|-------------------------|---------|
| IDLE         | STATUS_PENDDING         | Pending/ |
| PENDING      | STATUS_PENDDING         | Pending/ |
| IN_PROGRESS  | STATUS_RUNNING          | Running/ |
| COMPLETED    | STATUS_COMPLETE         | Completed/ |
| FAILED       | STATUS_FAILURE          | Failed/ |
| CANCELLED    | STATUS_CANCLE           | Cancelled/ |

---

## 四、Block 层的 type 字段整合

### 4.1 Block 内部参数区分

`BlockObject::Task_Info` 增加 `block_task_category` 字段用于路由。此外，Block 内部的 settings/参数 根据任务类型分化:

```cpp
// === BlockObject::Task_Info 现有字段 (两套任务共用) ===
struct Task_Info {
    // ... 现有基础字段: blockName, blockId, projectfile_ 等 ...

    // ==== 新增: 任务分类路由 ====
    int  block_task_category = 0;   // 0=重建(默认), 1=生成式
};
```

两类任务共用 `blockName`、`blockId`、`projectfile_` 等基础字段，差异在于:

| 维度 | 重建任务 (category=0) | 生成式任务 (category=1) |
|------|---------------------|----------------------|
| 配置来源 | `at_options`, `atsetting`, 重建参数 | `GenerationParams` (从 UI 传入, 序列化为 JSON) |
| Settings | `ATOptions at_options` | 不使用 (无空三/重建参数) |
| 统计信息 | `StatisticInfo_s statisticinfo_` | 不使用 |
| 结果产出 | 空三结果 / 重建 tile | 下载的模型/纹理文件 |
| 目录结构 | 嵌套 (Job → Task 子文件夹) | 扁平 (结果目录直接挂在 Block 下) |

### 4.2 两种任务的目录结构对比

#### 重建式 Block 完整层级 (现有)

```
project/                                                  ← 项目根目录
└── Project.mai                                           ← 项目文件

└── Block_1/                                              ← BLOCK_PRE + blockId
    ├── Block_1.blk  (.bbin)                              ← Block 元数据
    ├── SCB.bin                                           ← Block 二进制数据
    ├── CP.bin                                            ← 连接点
    ├── OD.bin                                            ← 原始数据
    │
    ├── J_20230316095227_SC/                              ← 空三 Job
    │   ├── TI_0.bin (.json)                              ← task_def_0: 主任务 (type=4,拆分)
    │   ├── TI_1.bin                                      ← task_def_1: 特征检测
    │   ├── TI_2.bin                                      ← task_def_2: 像对选择
    │   ├── TI_3.bin                                      ← task_def_3: 特征匹配
    │   ├── TI_4.bin                                      ← task_def_4: SfM
    │   └── TI_5.bin                                      ← task_def_5: 完成 (type=0)
    │
    ├── JF_J_20230316095227_SC.bin                        ← Feedback
    ├── JT_J_20230316095227_SC.bin                        ← 时间统计
    │
    ├── Reconstruction_0/                                 ← 重建实例
    │   └── Productions/
    │       └── Production_0/                             ← 生产实例
    │           ├── tile_0/                               ← 瓦片
    │           │   └── J_20230316_TILE_0_xxx/            ← 瓦片 Job
    │           │       └── TI_0.bin
    │           ├── tile_1/
    │           │   └── J_20230316_TILE_1_xxx/
    │           │       └── TI_0.bin
    │           └── tile_N/...
    │
    └── J_20230316120000_SC/                              ← 第二个空三 Job (如有)

└── Block_2/ ...
```

**层级**: `Project → Block → Job → Task` (4 层), 重建路径再加 `Reconstruction → Production → Tile` (最深 7 层)。

**路径公式** (`CallEngine.cpp:3776`):

```
Job 目录  = <projectDir>/<blockItem>/<jobName>/
Task 文件 = <jobDir>/TI_<taskId>.bin
Feedback  = <blockDir>/JF_<jobName>.bin
```

#### 生成式 Block 层级 (新增)

```
project/
├── Project.mai
├── Block_gen1/                    ← 生成式 Block
│   ├── Block_gen1.blk             ← Block 元数据 (block_task_category=1)
│   ├── 生成结果_001/               ← 结果目录 (扁平, 直接挂 Block 下)
│   │   ├── result.glb
│   │   └── preview.png
│   ├── 生成结果_002/
│   │   └── result.glb
│   ├── JF_gen_001.json            ← Feedback (与结果目录平级)
│   └── JF_gen_002.json
│
├── Block_2/                       ← 重建 Block (不变)
│   └── ...
└── ...
```

**层级**: `Project → Block → 生成结果` (3 层, 无 Job/Task 嵌套)。

**路径公式**:

```
结果目录  = <projectDir>/<blockItem>/<job_name>/
Feedback  = <blockDir>/JF_<job_name>.json
```

#### 对比总结

| 维度 | 重建式 | 生成式 |
|------|-------|-------|
| Block 前缀 | `Block_` | `Block_gen` (建议) |
| 子目录结构 | Job → Task, 多层嵌套 | 生成结果, 单层扁平 |
| 任务原子单位 | TI_N (Task 定义文件) | 生成结果目录 |
| 任务间关系 | DAG 依赖 (FatherId + Depends) | 无依赖, 各自独立 |
| 中间文件 | task_def_N, SCB, CP, OD | 无 |
| 结果产出 | 空三结果 / 重建 tile | 下载的模型/纹理文件 |
| Feedback 位置 | `<blockDir>/JF_<jobName>.bin` | `<blockDir>/JF_<jobName>.json` |
| Feedback 格式 | BIN (XOR 0xAB) / JSON | JSON  only |

### 4.3 Job 与"结果节点"的关系

两类体系中, **Job 始终是调度单位**, 但 UI 呈现和目录产物的概念不同。

#### 重建式的 Block → Job → 结果节点

```
UI 树:                             目录:
Block_1  (参数容器)                 Block_1/
├── 空三结果  (子节点)  ←── Job完成产生    ├── J_xxx_SC/     ← Job 目录
│   └── 重建结果 (孙节点) ←── Job完成产生   │   └── TI_0.bin  ← Task 文件
│                                          ├── JF_xxx.bin   ← Feedback
│                                          └── Reconstruction_0/
└── (用户可多次空三, 产生多个结果节点)
```

重建式的 Block 是"参数输入 + 构建工厂":
1. 用户创建 Block → 导入影像、设 AT 参数
2. 点击"空三" → 创建 Job → Node 调度 Task 链 → 完成后在 Block 下生成"空三结果"子节点
3. 用户对空三结果点击"重建" → 创建 Tile Job → 完成后生成"重建结果"孙节点

#### 生成式的 Block → Job → 结果节点

```
UI 树:                             目录:
Block_gen1  (参数容器)              Block_gen1/
├── 生成结果_001 (子节点) ←── Job完成    ├── 生成结果_001/  ← 结果目录 (下载产物)
├── 生成结果_002 (子节点) ←── Job完成    │   ├── result.glb
│                                       │   └── preview.png
│                                       ├── 生成结果_002/
│                                       ├── JF_gen_001.json ← Feedback
│                                       └── JF_gen_002.json
```

生成式的 Block 也是"参数输入 + 生成工厂", 但跳过了空三→重建两步:
1. 用户创建 Block_gen → 填写 prompt、模型、参考图等 GenerationParams
2. 点击"生成" → 创建 Job (写入 jobs_gen/Pending/) → Node 提交服务端 → 完成后下载结果到 `生成结果_N/` 目录, Block 下新增子节点
3. 用户双击子节点 → UI 预览窗口展示

#### 关键区别

| | 重建式 | 生成式 |
|---|---|---|
| Block 的角色 | 影像+AT参数容器, AT结果+重建结果的父节点 | 生成参数容器, 生成结果的父节点 |
| Job | 存在, 在 jobs/ 和 Block 目录下均有痕迹 | 存在, 在 jobs_gen/ 和 Block 目录下均有痕迹 |
| 结果节点 | AT结果 / 重建结果 (由 Job 完成后产生) | 生成结果 (由 Job 完成后产生) |
| 结果节点数 | 多个 (可多次空三+重建) | 多个 (可多次生成) |
| 节点层级 | Block → 空三结果 → 重建结果 (嵌套) | Block → 生成结果 (扁平) |
| 中间子步骤 | Task DAG (特征→匹配→SfM) | 无 (服务端全权处理) |

**核心**: Job 仍然存在且是调度基本单位, "生成结果"是 Job 完成后的**产出**, 不是 Job 本身。

### 4.4 Block 创建时的路由

```cpp
// 前端创建 Block 时
if (block_task_category == 0) {
    // 重建 Block: 现有逻辑不变
    // - 创建 Block_N/ 目录
    // - 初始化 Block_N.blk
    // - 等待用户导入影像、设置 AT 参数等
} else {
    // 生成式 Block: 新逻辑
    // - 创建 Block_genN/ 目录
    // - 初始化 Block_genN.blk (block_task_category=1)
    // - 用户直接在 Block 面板设置 GenerationParams
    // - 每次"生成"创建一个结果目录
}
```

### 4.5 一次"生成"操作的数据流

用户在一个生成式 Block 下点击"生成":

```
1. 前端收集 GenerationParams (prompt, model, 参考图等)

2. SDK 构造 GenJobInfo:
   task_uuid     = QUuid::createUuid()
   job_name      = block.blockName + "_" + timestamp   // 作为结果目录名
   project_path  = block.projectFile 的父目录
   block_item    = block.blockName
   params        = <用户填写的 GenerationParams>
   status        = IDLE

3. 创建结果目录:
   project/Block_gen1/<job_name>/     ← 当前为空, 等下载后才有文件

4. Job 文件写入 jobs_gen/Pending/<job_name>.json

5. 创建 feedback:
   project/Block_gen1/JF_<job_name>.json
   内容: { Status: PENDING, Percent: 0 }

6. → generateThread 接管后续流程
```

### 4.6 GenerationParams 传入方式

前端在调用 SDK Submit 时, 直接把 `GenerationParams` 作为参数传入, SDK 组装 `GenJobInfo` 写入文件。

`GenerationParams` 不需要持久化到 Block 的存储结构中 — 它们的生命周期始于用户点击"生成", 止于写入 job 文件。后续如果需要"Block 级别的默认生成参数", 可以再加一个 `gen_params_json` 字段到 Task_Info。

---

## 五、完整数据流

### 5.1 创建生成式任务 (前端 → jobs_gen/Pending/)

```
┌──────────┐
│ 前端 App  │  用户点击"生成"按钮
└────┬─────┘
     │
     │  SDK: GenTaskAPI::SubmitGenTask(block, genParams)
     │
     ├── 1. 检查 Block.block_info_.block_task_category == 1
     │      如果不是 → 返回错误 "Block 不支持生成式任务"
     │
     ├── 2. 构造 GenJobInfo:
     │      task_uuid  = QUuid::createUuid().toString()
     │      job_name   = block.blockName + "_" + timestamp
     │      engine_id  = QHostInfo::localHostName()
     │      user_account = getCurrentUser()
     │      category   = genParams.category
     │      sub_type   = genParams.sub_type
     │      params     = genParams
     │      project_path = block.projectFile
     │      block_item   = block.blockName
     │      status     = IDLE
     │
     ├── 3. 计算 jobs_gen/Pending/ 路径 (从 engine key 父目录推导, 无需新注册表项)
     │
     ├── 4. GenJobInfo::save(jobs_gen/Pending/<job_name>.json)
     │      写入 JSON (不用 BIN, 原因见 GenTask_Design.md §4.5)
     │
     ├── 5. 创建初始 feedback:
     │      JobFeedBack_s fb;
     │      fb.Status  = STATUS_PENDDING
     │      fb.Percent = 0.0
     │      fb.save_with_retry(feedback_file)
     │
     └── 6. 返回 task_uuid 给前端 (用于后续查询)
```

### 5.2 Node 处理 (Pending → HTTP Submit → Running)

```
generateThread (GenTaskThread.cpp)
│
├── 遍历 jobs_gen/Pending/*.json
│
├── 对每个文件:
│   ├── lock(file.lock)
│   ├── GenJobInfo::load(filePath)
│   ├── 检查 job.server_task_id 是否为空
│   │   ├── 不为空 → submit 已成功过, 直接移动文件到 Running/
│   │   └── 为空 → 构造 GenTaskRequest → HTTP POST /api/v1/task/submit
│   │       │
│   │       ├── 成功:
│   │       │   ├── job.server_task_id = response.triverse_task_uuid
│   │       │   ├── job.status = PENDING
│   │       │   ├── GenJobInfo::save(filePath)  // 回填 server_task_id
│   │       │   ├── 文件从 Pending/ 移到 Running/
│   │       │   └── 更新 feedback (Status=RUNNING)
│   │       │
│   │       ├── 失败 (业务错误: 余额不足等):
│   │       │   ├── job.status = FAILED
│   │       │   ├── 文件从 Pending/ 移到 Failed/
│   │       │   └── feedback.Status = STATUS_FAILURE, Msg = 错误信息
│   │       │
│   │       └── 失败 (网络错误):
│   │           └── 保持 Pending, 等下次轮询重试 (不做改动)
│   │
│   └── unlock
│
└── sleep(2000)
```

### 5.3 Node 轮询 (Running → HTTP Query → Completed/Failed)

```
generateThread
│
├── 遍历 jobs_gen/Running/*.json
│
├── 对每个文件:
│   ├── lock(file.lock)
│   ├── GenJobInfo::load(filePath)
│   │
│   ├── 检查 job.status 是否为 CANCELLED
│   │   └── 是 → 移动到 Cancelled/ → update feedback → 下一个文件
│   │
│   ├── HTTP GET /api/v1/task/status?server_task_id=xxx
│   │   (超时 2s, 最多重试 3 次)
│   │
│   ├── 成功:
│   │   ├── 更新 job 中的进度字段
│   │   ├── 更新 feedback (Percent, Msg)
│   │   │
│   │   ├── status == "completed":
│   │   │   ├── job.status = COMPLETED
│   │   │   ├── job.result_url = response.result_url
│   │   │   ├── job.complete_time = now()
│   │   │   ├── GenJobInfo::save(filePath)
│   │   │   ├── feedback.Status = STATUS_COMPLETE
│   │   │   ├── feedback.result_url = response.result_url
│   │   │   ├── feedback.cost_credits = response.cost_credits
│   │   │   ├── feedback.save_with_retry(feedback_file)
│   │   │   ├── 文件从 Running/ 移到 Completed/
│   │   │   └── 触发完成回调 (若已注册)
│   │   │
│   │   ├── status == "failed":
│   │   │   ├── job.status = FAILED
│   │   │   ├── job.error_message = response.error_message
│   │   │   ├── 文件从 Running/ 移到 Failed/
│   │   │   ├── feedback.Status = STATUS_FAILURE
│   │   │   └── 触发失败回调 (若已注册)
│   │   │
│   │   └── status == "queued" / "running":
│   │       ├── GenJobInfo::save(filePath)  // 更新进度信息
│   │       └── 文件保持在 Running/
│   │
│   ├── 失败 (网络):
│   │   ├── job.query_retry_count++
│   │   ├── GenJobInfo::save(filePath)
│   │   └── ≥5 次 → 标记 FAILED + 移动
│   │
│   └── unlock
│
└── sleep(2000)
```

### 5.4 前端查询 (读取 feedback 文件)

```
前端 App
│
├── GenTaskAPI::QueryTaskStatus(task_uuid)
│   ├── 定位 feedback 文件: jobs_gen/*/JF_<job_name>.json
│   │   (需要遍历目录找, 或维护 task_uuid → job_name 映射)
│   ├── JobFeedBack_s::load_with_retry(feedback_file)
│   │   (文件锁保护, 不会被 generateThread 的并发写入干扰)
│   └── 返回 { Status, Percent, Msg, result_url, cost_credits, ... }
│
├── GenTaskAPI::DownloadResult(task_uuid, save_path)
│   ├── 1. 检查 feedback → Status 是否为 COMPLETE
│   ├── 2. 获取 result_url
│   ├── 3. HTTP GET result_url → 保存到 save_path
│   └── 4. 如果 URL 过期 (404) → 重新向服务端请求新 URL
│       (POST /api/v1/task/refresh-url)
│
├── GenTaskAPI::QueryPoints(user_id)
│   └── HTTP GET /api/v1/user/points → 返回积分信息
│
└── GenTaskAPI::CancelGenTask(task_uuid)
    ├── 如果 job 在 Pending/ → 直接删除 + feedback 更新
    ├── 如果 job 在 Running/ → 标记 CANCELLED → generateThread 下次轮询时处理
    │   (generateThread 发送 cancel 请求给服务端 → 移动文件 → 更新 feedback)
    └── 如果 job 已完成 → 返回错误
```

---

## 六、文件改动清单

### 6.1 修改现有文件

| 文件 | 改动 | 风险等级 |
|------|------|---------|
| `Include/Core/BlockObject.h` | Task_Info 增加 `block_task_category` (int, 默认0) | 低 — 新增字段默认值确保向后兼容 |
| `Include/Core/BlockObject.cpp` | 4 个序列化方法增加新字段的读写 | 低 |
| `Include/Util/TaskProcess.h` | JobInfo_s + JobFeedBack_s 增加新字段; 更新 10+ 序列化方法 | 中 — 序列化方法多, 容易遗漏 |
| `Include/Util/Settings.h` | 增加 `getGenEngineJobQueue()` 声明 | 低 |
| `Src/Util/Settings.cpp` | 实现 `getGenEngineJobQueue()`, 从 `engine` key 父目录推导, 无需新注册表项 | 低 |
| `App/Engine/CallEngine.cpp` (main) | 增加 jobs_gen 目录创建; 启动 generateThread | 低 |
| `App/Engine/CallEngine.cpp` (MakePath) | 调用新方法创建 jobs_gen 子目录 | 低 |
| `App/Engine/CMakeLists.txt` | 增加 GenHttpClient.cpp, GenTaskThread.cpp; link Qt6::Network | 低 |
| `Src/Core/CMakeLists.txt` | 增加 GenTaskAPI.cpp | 低 |

### 6.2 新增文件

| 文件 | 说明 |
|------|------|
| `Include/Util/GenTaskProcess.h` | 所有生成式任务枚举 + 结构体 + GenJobInfo 序列化 |
| `App/Engine/GenHttpClient.h` | 同步 HTTP 客户端封装 |
| `App/Engine/GenHttpClient.cpp` | 实现 submit/queryStatus/queryPoints/cancel |
| `App/Engine/GenTaskThread.h` | generateThread 声明 |
| `App/Engine/GenTaskThread.cpp` | generateThread 主逻辑 (processPendingJobs + processRunningJobs) |
| `Include/Core/GenTaskAPI.h` | SDK 对外接口声明 |
| `Src/Core/GenTaskAPI.cpp` | SDK 实现 (submit, queryStatus, download, cancel, callbacks) |

### 6.3 不改动的文件

- `TaskGraph_s` / `Task_s` — 仅本地任务使用
- `ATTaskInfo` / `TaskDescriptor` — 仅本地任务使用
- `SPTaskInfoFile` / BIN 格式 — 生成式任务只用 JSON
- `ExecTaskFileV2` / `GetPendingJob` / `GetRunningTaskInRunningJob` — 不动
- `JobMonitor` — 可复用其静态方法但不修改

---

## 七、路由逻辑: Node 如何区分两类任务

### 7.1 main() 中的线程分工

```cpp
// CallEngine.cpp main() — 现有 (不动)
std::thread searchPendingJob1(searchPendingJobThread2);  // → 只处理 jobs/
searchPendingJob1.detach();

// CallEngine.cpp main() — 新增
std::thread genTaskThread(GenTaskThread::run);           // → 只处理 jobs_gen/
genTaskThread.detach();
```

### 7.2 前端创建 Job 时的路由

```cpp
// GenTaskAPI::SubmitGenTask() — SDK 实现

std::string GenTaskAPI::SubmitGenTask(const GenJobInfo& job) {
    // 1. 获取 gen 工作目录
    QString genRoot = Settings::getGenEngineJobQueue();
    QString pendingPath = genRoot + "/Pending/";

    // 2. 确保目录存在
    QDir().mkpath(pendingPath);

    // 3. 写入 job 文件
    std::string jobPath = qstr2str(pendingPath) + job.job_name + ".json";
    job.save(jobPath);

    // 4. 创建 feedback
    QString feedbackPath = ...; // 计算 feedback 路径
    JobFeedBack_s fb;
    fb.Status = jobsta_e::STATUS_PENDDING;
    fb.save_with_retry(qstr2str(feedbackPath));

    return job.task_uuid;
}
```

### 7.3 现有 submit 流程的路由 (如果有)

如果现有代码中有统一的 "创建 Job" 入口 (例如 `BlockObject::CreateJob`), 需要在其中加入分支:

```cpp
void BlockObject::CreateJob(...) {
    if (block_info_.block_task_category == 0) {
        // 现有逻辑: 创建 JobFullInfo_s → 写入 jobs/Pending/
        CreateLocalJob(...);
    } else {
        // 新逻辑: 构造 GenJobInfo → 写入 jobs_gen/Pending/
        GenTaskAPI::SubmitGenTask(...);
    }
}
```

---

## 八、需要注意的实现细节

### 8.1 JobFeedBack_s 新字段的 JSON Key 命名

统一使用 camelCase (与现有 `Status`/`Percent`/`Msg` 的风格不同, 但新字段不应再延续 PascalCase 的历史风格):

```json
{
    "Status": 3,
    "Percent": 100.0,
    "Msg": "completed",
    "TaskRetVal": 0,
    "result_url": "https://cdn.example.com/xxx.glb",
    "preview_url": "https://cdn.example.com/xxx.png",
    "server_task_id": "triverse-abc123",
    "error_message": "",
    "cost_credits": 10,
    "points_balance": 990
}
```

### 8.2 GenerationParams 中 optional 字段的 JSON 序列化

nlohmann::json 原生支持 `std::optional` (C++17), 序列化时:
- `std::nullopt` → key 不出现在 JSON 中
- `std::optional<T>` 有值 → key 正常序列化

这正好符合 HTTP API 的需求 (不发送空字段)。

### 8.3 AssetRef 的上传处理

当 `AssetRef.kind == AssetKind::FILE_PATH` 时, 文件在本地。submit 前需要先上传文件到服务端获取 `file_key`:

```
如果 kind == FILE_PATH:
    1. 读取本地文件
    2. POST /api/v1/upload (multipart/form-data)
    3. 获取 file_key
    4. 将 AssetRef.kind 改为 FILE_KEY, value 改为 file_key
    5. 然后再 submit 任务
```

这个文件上传逻辑应放在 `GenHttpClient` 或 `GenTaskAPI::SubmitGenTask` 中。

### 8.4 前端读取 feedback 时的文件定位

由于 feedback 文件在项目目录下 (跟 Block 路径关联), 前端需要通过 `task_uuid` 找到对应的 feedback。建议维护一个简单的索引:

**方案**: 在 GenJobInfo 的 job 文件中存储 block 关联信息 (已有的 `project_path` + `block_item`), feedback 路径可以从这些信息重构。前端通过扫描 `jobs_gen/` 下各目录中的 job 文件来定位, 或者更简单地:

```
feedback 文件名 = "JF_" + job_name + ".json"
job_name = gen_<timestamp>_<sub_type>
task_uuid → job_name 的映射需要前端自己记录, 或 SDK 返回时一并告知 job_name
```

建议 `GenTaskAPI::SubmitGenTask` 的返回值包含 `{ task_uuid, job_name, feedback_path }`。

### 8.5 回调机制的线程安全

由于 `generateThread` 在独立线程中运行, 回调函数可能在非 Qt UI 线程执行。SDK 需要处理线程切换:

```cpp
// GenTaskAPI.cpp

static TaskCompleteCallback g_completeCallback;

void GenTaskAPI::SetTaskCompleteCallback(TaskCompleteCallback cb) {
    g_completeCallback = std::move(cb);
}

// 在 generateThread 中:
void onTaskCompleted(const GenJobInfo& job) {
    if (g_completeCallback) {
        // 如果回调需要操作 Qt UI, 由调用者用 QMetaObject::invokeMethod 处理
        g_completeCallback(job.task_uuid, job.result_url);
    }
}
```

文档中注明: 回调在 generateThread 线程中执行, 调用者如需更新 UI, 自行使用 `QMetaObject::invokeMethod` 或 `emit` 信号。

---

## 九、实施建议

> **详细执行清单见 [GenTask_Checklist.md](GenTask_Checklist.md)** — 包含每项的勾选框、伪代码、依赖关系图和文件改动汇总。

### 概要 Phase: 数据结构基础 (1-2天)
1. 创建 `GenTaskProcess.h` (用户提供的枚举+结构体 + GenJobInfo)
2. 修改 `BlockObject.h/cpp` (增加 block_task_category)
3. 修改 `TaskProcess.h` (JobInfo_s + JobFeedBack_s 扩展)
4. 编译通过验证

### Phase 2: HTTP 客户端 (1天)
1. 创建 `GenHttpClient.h/cpp`
2. 实现 submit / queryStatus / cancel 方法
3. 手动测试 HTTP 请求与响应解析

### Phase 3: 调度线程 (1-2天)
1. 创建 `GenTaskThread.h/cpp`
2. 实现 processPendingJobs + processRunningJobs
3. 与现有线程并行运行测试

### Phase 4: SDK (1天)
1. 创建 `GenTaskAPI.h/cpp`
2. 实现 submit / queryStatus / download / cancel
3. 实现回调机制

### Phase 5: 集成与测试 (2-3天)
1. 端到端测试: 前端 → Pending → HTTP → Running → Completed → 前端读取
2. 异常测试: Node 重启、网络断开、服务端错误
3. 并发测试: 本地任务与生成式任务并行运行
