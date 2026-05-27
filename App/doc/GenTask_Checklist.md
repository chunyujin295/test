# 生成式任务系统 — 执行清单

按依赖关系排列，完成一项勾一项。

## 依赖关系图

```
Phase 1 (数据结构) ──→ Phase 3 (HTTP) ──→ Phase 4 (调度线程) ──→ Phase 5 (SDK)
                                            ↗
Phase 2 (路径配置) ─────────────────────────┘

Phase 1 + 2 可以并行开工
```

## 新增文件与现有系统的对应关系

每项列出"新增了什么、对标现有哪个组件、为什么需要它"。

### 数据结构层

| 新增 | 对标现有 | 说明 |
|------|---------|------|
| `GenTaskProcess.h` | `Util/TaskProcess.h` | 和 `TaskProcess.h` 一样定义 Job 调度结构体。`TaskProcess.h` 是重建式（`JobInfo_s`、`JobFeedBack_s`），`GenTaskProcess.h` 是生成式（`GenJobInfo_s` + 枚举 + API 类型） |
| `DataStruct.h` (修改) | `FeedBackFile` / `JobInfoData` / `BLKBinFile` | 新增 `GenJobInfoData`/`GenJobFile`（对标 `FeedBackData`/`FeedBackFile` 的 BIN 模式）；`JobInfoData` 增加 `task_category`；`BLKBinFile` 增加 `block_task_category` |
| `GenJobInfo_s` | `JobInfo_s` | 纯数据结构体 — `JobInfo_s` 存 `ProjectPath/ItemPath`，`GenJobInfo_s` 存 `task_uuid/GenerationParams/server_task_id/result_url`。仅含 `to_json`/`from_json`，无 I/O 方法 |
| `GenJobFullInfo_s` | `JobFullInfo_s` | 文件级结构体 — 持有 `job_name` + `GenJobInfo_s job`，提供 `save`/`load`（`JOB_INFO_USE_BIN` 分发）+ `WriteToBin`/`LoadFromBin`（手动拷贝到 `GenJobInfoData`/`GenJobFile`） |
| `GenTaskRequest` | 无现成对标 | HTTP submit 请求体，序列化为 JSON POST 到服务端。重建式没有远程 API，所以这是全新概念 |
| `GenTaskResponse` | 无现成对标 | HTTP 响应体。服务端返回的 task status/progress/result_url |
| `GenerationParams` | `ATOptions`（局部类似） | 用户填写的生成参数（prompt、模型版本、面数等）。概念上类似 `ATOptions`（空三参数），但结构完全不同 |
| `GenTaskCategory/SubType/Status` | `jobsta_e`（局部类似） | 生成式任务的状态枚举。`jobsta_e` 是重建式 job 状态（PENDING/RUNNING/COMPLETE...），`GenTaskStatus` 多了 IDLE + 服务端状态 |
| `block_task_category` (Task_Info 新增) | 无，新增字段 | 在已有的 `BlockObject::Task_Info` 中增加的 int 字段，区分 Block 是重建式(0)还是生成式(1)。类似 `type_` 字段的作用 |
| `JobFeedBack_s` — **不修改** | 自身 | 生成式任务复用 `Status`/`Percent`/`Msg`/`TaskRetVal` 做进度反馈。结果数据（`result_url` 等）存在 `GenJobInfo_s` 中，不扩展 feedback 字段 |

### 调度与通信层

| 新增 | 对标现有 | 说明 |
|------|---------|------|
| `GenTaskThread` | `searchPendingJobThread2` (CallEngine.cpp) | 调度线程。`searchPendingJobThread2` 遍历 `jobs/` 调度重建式任务，`GenTaskThread::Run()` 遍历 `jobs_gen/` 调度生成式任务。两者独立运行，互不干扰 |
| `GenHttpClient` | `spawn Task.exe` 子进程 | 任务执行方式。重建式通过文件 IPC + spawn `MoldAITask.exe` 子进程执行，生成式通过 HTTP POST/GET 提交和轮询远程服务端。都是"提交任务→等待结果"的模式 |
| `jobs_gen/` 目录 | `jobs/` 目录 | 文件系统 IPC 的工作目录。`jobs/` 存重建式 Job 文件（`J_*`），`jobs_gen/` 存生成式 Job 文件（同样 `J_*` 前缀，BIN/JSON 格式由 `JOB_INFO_USE_BIN` 控制）。启动时需同样清理残留 `.lock` 文件 |
| `getGenEngineJobQueue()` | `getEngineJobQueue()` | 获取队列根路径。`getEngineJobQueue()` 读注册表 `engine` key，`getGenEngineJobQueue()` 取其父目录 + `/jobs_gen`，无需新注册表项 |

### SDK 与接口层

| 新增 | 对标现有 | 说明 |
|------|---------|------|
| `GenTaskAPI` | 无现成对标 | 前端 SDK。现有系统前端直接操作 BlockObject + Job 文件（耦合高），`GenTaskAPI` 将生成式任务的"提交/查询/下载/回调"封装为静态方法。回调机制采用 `GenTaskAPI` 存储 → `GenTaskThread` 触发的单向依赖 |
| `TriggerTaskComplete/TriggerTaskFailed` | 无现成对标 | GenTaskThread 完成后调用 `GenTaskAPI::Trigger*`，`GenTaskAPI` 内部调用前端注册的回调。避免 `GenTaskAPI`（MoldAIData.dll）反向依赖 `GenTaskThread`（MoldAINode.exe） |

### 修改的现有文件

| 文件 | 对标什么 | 说明 |
|------|---------|------|
| `BlockObject.h/cpp` | 自身 | `Task_Info` 加 `block_task_category` 区分 Block 类型 |
| `TaskProcess.h` | 自身 | `JobInfo_s` 加 `task_category`；`JobFullInfo_s::WriteToBin/LoadFromBin` 加 `task_category` 拷贝。`JobFeedBack_s` 不动 |
| `Settings.h/cpp` | 自身 | 新增 `getGenEngineJobQueue()` |
| `CallEngine.cpp` | 自身 | MakePath 创建 `jobs_gen/` 目录；main 启动 `GenTaskThread` |
| `App/Engine/CMakeLists.txt` | 自身 | HEADER_LIST 添加 `GenTaskProcess.h`（唯一需改的 CMakeLists） |

---

## Phase 1: 数据结构基础

> 最先做，所有后续 Phase 都依赖这里的结构定义。

### 1.1 新建 GenTaskProcess.h

- [ ] 创建 `Include/Util/GenTaskProcess.h`

```cpp
// Include/Util/GenTaskProcess.h
// ============================================================================
// @file    GenTaskProcess.h
// @brief   生成式任务的数据结构定义, 对标 TaskProcess.h (重建式任务)
//
// 放在 Util/ 而非 Core/ 的原因: TaskProcess.h (JobInfo_s, JobFeedBack_s) 也在
// Util/ 下, GenJobInfo_s 本质上和 JobInfo_s 是同层级的 job 调度结构体 — 都是
// Engine 调度层 (MoldAINode.exe) 使用的 job 文件读写结构体, 不是 Core 层的
// 基础数据模型。
//
// JSON 库: 使用项目内 "Core/json.h" (vendored nlohmann 3.7.2), 与 TaskProcess.h
//          一致。所有序列化均为手动 to_json/from_json, 因为 3.7.2 不支持
//          NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE (需 3.10+)。
//          枚举在 JSON 中存为 int, 对标 TaskProcess.h 中 jobsta_e 的序列化方式。
//
// std::optional: C++17 标准库类型, 项目内首次使用。用于区分"字段显式设为空"
//                和"字段未提供", 替代空字符串/哨兵值(-1)的模糊语义。
//                JSON 序列化时仅在 has_value() 为 true 时才写入字段。
// ============================================================================

#pragma once

#include <string>
#include <vector>
#include <optional>   // C++17, 项目内首次使用: 区分"未提供"与"显式设空"
#include <thread>      // save_with_retry / load_with_retry 中的 sleep_for
#include <chrono>

#include "Core/json.h"       // 项目内 vendored nlohmann 3.7.2
#include "Core/File.h"       // FopenDenyWriteLockUtf8 / OpenOfstreamUtf8 (save/load + 锁)
#include "Core/Logging.h"    // LOGE / LOGI / LOGW
#include "Core/DataStruct.h" // GenJobInfoData / GenJobFile (BIN I/O, Phase 1.2 定义)
#include "Core/Types.h"      // JOB_INFO_USE_BIN (GenJobFullInfo_s::save/load 分发用)

namespace AI3D {
namespace CORE {

// ============================================================================
// 四个枚举 — 在整个生成式任务系统中的角色
//
//                            ┌─────────────────┐
//  前端填写 ───────────────→ │ GenerationParams │ → JSON → GenTaskRequest → HTTP POST
//  (MoldAIData.dll)          │  · category      │
//                            │  · sub_type      │
//                            └─────────────────┘
//
//  调度线程维护 ───────────→ │ GenJobInfo_s       │ → JSON → jobs_gen/ 下各状态目录
//  (MoldAINode.exe)          │  · status        │
//                            └─────────────────┘
//
//  AssetKind 是资源引用的解析指令 — GenTaskThread 据此决定是否 upload
// ============================================================================

/// @brief 任务大类 — 决定服务端路由到哪个 processing pipeline
///
/// 使用位置:
///   - GenerationParams::category — 前端填写, 序列化到 HTTP submit body
///   - GenJobInfo_s::category       — job 文件记录, 用于日志/监控分类
///
/// 数据流: 前端 SDK → GenJobInfo_s JSON → GenTaskThread 读取 → GenTaskRequest JSON → HTTP POST
enum class GenTaskCategory {
    TEXT_TO_3D,         // 文生3D (文字描述 → 3D 模型)
    IMAGE_TO_3D,        // 图生3D (参考图 → 3D 模型)
    TEXTURING,          // 贴图生成 (模型 + 文字 → 带贴图的模型)
    UTILITY,            // 工具类 (格式转换 / 减面 / 渲染)
    IMAGE_GENERATION,   // 图片生成 (文字 → 图片 / 预览图)
};

/// @brief 任务子类型 — 决定服务端 API endpoint 和具体的 algorithm
///
/// 使用位置:
///   - GenerationParams::sub_type — 前端填写, 序列化到 HTTP submit body
///   - GenJobInfo_s::sub_type       — job 文件记录
///
/// 与 GenTaskCategory 的关系: category 定义大类, sub_type 在同类内细化路由
///   例: category=TEXT_TO_3D + sub_type=TEXT_TO_MODEL → 输出 .glb 模型
///       category=TEXT_TO_3D + sub_type=TEXT_TO_MESH  → 输出 .obj mesh
enum class GenTaskSubType {
    TEXT_TO_MODEL,          // 文本 → 3D 模型文件 (.glb)
    TEXT_TO_MESH,           // 文本 → Mesh 数据 (.obj)
    IMAGE_TO_MODEL,         // 图片 → 3D 模型文件 (.glb)
    IMAGE_TO_MESH,          // 图片 → Mesh 数据 (.obj)
    TEXTURE_MODEL,          // 模型 + 文字 → 贴图 (输入 model_file + prompt)
    TEXT_TO_TEXTURE,        // 文字 → 贴图 (无输入模型)
    MODEL_PREVIEW_RENDER,   // 模型预览渲染 (360° 旋转图)
    MODEL_REMESH,           // 模型减面 / 重构拓扑
    CONVERT_MODEL_FORMAT,   // 模型格式转换 (obj↔glb↔fbx)
    IMAGE_GENERATION,       // 图片生成 (2D)
};

/// @brief 任务状态 — 驱动 job 文件在状态目录间流转
///
/// 使用位置:
///   - GenJobInfo_s::status          — job 文件内记录, GenTaskThread 读写
///   - GenTaskResponse::status     — 服务端 HTTP 响应解析
///   - GenTaskAPI::TaskStatusResult::status — 前端查询返回值 (从 JobFeedBack_s 映射)
///
/// 状态 → 目录映射 (GenTaskThread 执行):
///   IDLE        → (仅内存状态, 尚未写入任何目录)
///   PENDING     → jobs_gen/Pending/   (等待 submit)
///   IN_PROGRESS → jobs_gen/Running/   (已 submit, 轮询中)
///   COMPLETED   → jobs_gen/Completed/ (完成)
///   FAILED      → jobs_gen/Failed/    (失败)
///   CANCELLED   → jobs_gen/Cancelled/ (取消)
///
/// 状态 → jobsta_e 映射 (UpdateFeedback 中):
///   IDLE/PENDING → STATUS_PENDDING,  IN_PROGRESS → STATUS_RUNNING
///   COMPLETED    → STATUS_COMPLETE,  FAILED      → STATUS_FAILED
///   CANCELLED    → STATUS_CANCELLED
enum class GenTaskStatus {
    IDLE,           // 初始 / 网络不通 — GenTaskResponse.status==IDLE 且 error_message 有值 = 超时
    PENDING,        // 已写入 jobs_gen/Pending/, 等待 GenTaskThread pick up
    IN_PROGRESS,    // 已 POST submit 到服务端, GenTaskThread 正在周期性 GET query
    COMPLETED,      // 服务端返回完成, GenTaskThread 回填 result_url 并移到 Completed/
    FAILED,         // 执行失败 — 可能是服务端 FAILED, 也可能是连续 5 次轮询超时
    CANCELLED,      // 用户取消 — GenTaskThread 发送 POST cancel 后移到 Cancelled/
};

/// @brief 资源引用类型 — 决定 AssetRef::value 的解析方式
///
/// 使用位置:
///   - AssetRef::kind — 标记 value 字段是本地路径、服务端 key 还是远程 URL
///
/// GenTaskThread::ProcessPendingJobs 中的处理逻辑:
///   FILE_PATH → 调用 GenHttpClient::UploadFile 上传 → 改为 FILE_KEY
///   FILE_KEY  → 直接放入 GenTaskRequest JSON, 服务端根据 key 查找已上传文件
///   URL       → 直接放入 GenTaskRequest JSON, 服务端自行下载
///   NONE      → AssetRef 无有效数据
enum class AssetKind {
    NONE,           // 无资源引用 (AssetRef 为空)
    FILE_PATH,      // value = 本地文件绝对路径 → GenTaskThread 需 upload → 变为 FILE_KEY
    FILE_KEY,       // value = 服务端返回的 file_key → submit 时直接传给 API
    URL,            // value = 完整 URL → submit 时直接传给 API, 服务端自行下载
};

// ============================================================================
// 数据结构体 + 手动 to_json / from_json
// 枚举在 JSON 中存为 int (对标 TaskProcess.h 中 jobsta_e 的序列化方式)
// ============================================================================

/// @brief 资源引用 (图片文件 / 模型文件)
/// kind 决定 value 的解析方式: FILE_PATH 需先 upload, FILE_KEY 直接传给 API
struct AssetRef {
    AssetKind   kind = AssetKind::NONE;
    std::string value;
    std::string content_type;
};
inline void to_json(nlohmann::json& j, const AssetRef& v) {
    j["kind"]         = static_cast<int>(v.kind);
    j["value"]        = v.value;
    j["content_type"] = v.content_type;
}
inline void from_json(const nlohmann::json& j, AssetRef& v) {
    if (j.contains("kind"))         v.kind         = static_cast<AssetKind>(j.at("kind").get<int>());
    if (j.contains("value"))        v.value        = j.at("value").get<std::string>();
    if (j.contains("content_type")) v.content_type = j.value("content_type", "");
}

/// @brief 生成参数 — 用户指定的生成配置
/// 所有 optional 字段: 有值时在 JSON 中包含, 无值时不出现在 JSON 中
struct GenerationParams {
    GenTaskCategory category = GenTaskCategory::TEXT_TO_3D;  // 任务大类
    GenTaskSubType  sub_type = GenTaskSubType::TEXT_TO_MODEL; // 子类型

    // --- 文字输入 ---
    std::optional<std::string> prompt;           // 正向提示词 (TEXT_TO_3D / TEXTURING 必填)
    std::optional<std::string> negative_prompt;  // 反向提示词 (可选)
    std::optional<std::string> style;            // 风格预设名 (可选)

    // --- 资源输入 (FILE_PATH → 由 GenTaskThread 上传 → FILE_KEY) ---
    std::optional<AssetRef> image_file;  // 输入图片 (IMAGE_TO_3D / TEXTURING)
    std::optional<AssetRef> model_file;  // 输入模型 (TEXTURING / REMESH / CONVERT)

    // --- 模型参数 ---
    std::optional<std::string> model_version;   // 模型版本号 (可选, 默认最新)
    std::optional<int>         polygon_limit;   // 面数上限 (TEXT_TO_3D / IMAGE_TO_3D)
    std::optional<int>         texture_size;    // 贴图分辨率 (TEXTURING)

    // --- 渲染 / 工具参数 ---
    std::optional<std::string>              render_mode;    // 渲染模式 (如 "360_spin")
    std::optional<int>                      image_count;    // 图片生成数量
    std::optional<std::vector<std::string>> camera_angles;  // 相机角度列表
    std::optional<std::string>              preset_name;    // 预设名

    // --- 后处理参数 ---
    std::optional<int>         target_poly_count;  // 目标面数 (减面)
    std::optional<std::string> output_format;       // 输出格式 (如 "glb", "obj")
};
inline void to_json(nlohmann::json& j, const GenerationParams& v) {
    j["category"] = static_cast<int>(v.category);
    j["sub_type"] = static_cast<int>(v.sub_type);
    if (v.prompt)           j["prompt"]           = *v.prompt;
    if (v.negative_prompt)  j["negative_prompt"]  = *v.negative_prompt;
    if (v.style)            j["style"]            = *v.style;
    if (v.image_file)       j["image_file"]       = *v.image_file;
    if (v.model_file)       j["model_file"]       = *v.model_file;
    if (v.model_version)    j["model_version"]    = *v.model_version;
    if (v.polygon_limit)    j["polygon_limit"]    = *v.polygon_limit;
    if (v.texture_size)     j["texture_size"]     = *v.texture_size;
    if (v.render_mode)      j["render_mode"]      = *v.render_mode;
    if (v.image_count)      j["image_count"]      = *v.image_count;
    if (v.camera_angles)    j["camera_angles"]    = *v.camera_angles;
    if (v.preset_name)      j["preset_name"]      = *v.preset_name;
    if (v.target_poly_count) j["target_poly_count"] = *v.target_poly_count;
    if (v.output_format)    j["output_format"]    = *v.output_format;
}
inline void from_json(const nlohmann::json& j, GenerationParams& v) {
    v.category = static_cast<GenTaskCategory>(j.value("category", 0));
    v.sub_type = static_cast<GenTaskSubType>(j.value("sub_type", 0));
    auto maybe = [&](const char* key, auto& opt) {
        if (j.contains(key)) opt = j.at(key).get<typename std::remove_reference_t<decltype(opt)>::value_type>();
    };
    maybe("prompt",            v.prompt);
    maybe("negative_prompt",   v.negative_prompt);
    maybe("style",             v.style);
    maybe("image_file",        v.image_file);
    maybe("model_file",        v.model_file);
    maybe("model_version",     v.model_version);
    maybe("polygon_limit",     v.polygon_limit);
    maybe("texture_size",      v.texture_size);
    maybe("render_mode",       v.render_mode);
    maybe("image_count",       v.image_count);
    maybe("camera_angles",     v.camera_angles);
    maybe("preset_name",       v.preset_name);
    maybe("target_poly_count", v.target_poly_count);
    maybe("output_format",     v.output_format);
}

/// @brief HTTP 提交请求体 — POST /api/v1/task/submit
struct GenTaskRequest {
    std::string     task_id;       // 客户端生成的 UUID (幂等键)
    std::string     engine_id;     // 发起请求的 Engine 标识 (主机名)
    std::string     user_account;  // 用户账号 (积分计费用)
    GenerationParams params;       // 生成参数
};
inline void to_json(nlohmann::json& j, const GenTaskRequest& v) {
    j["task_id"]      = v.task_id;
    j["engine_id"]    = v.engine_id;
    j["user_account"] = v.user_account;
    j["params"]       = v.params;
}
inline void from_json(const nlohmann::json& j, GenTaskRequest& v) {
    j.at("task_id").get_to(v.task_id);
    j.at("engine_id").get_to(v.engine_id);
    j.at("user_account").get_to(v.user_account);
    j.at("params").get_to(v.params);
}

/// @brief HTTP 响应体 — 服务端返回的任务状态
/// 所有 optional 字段仅在服务端提供对应值时才有值
struct GenTaskResponse {
    std::string                task_id;              // 回显客户端的 task_uuid
    std::optional<std::string> triverse_task_uuid;   // 服务端分配的任务ID (save 到 job 的 server_task_id)
    GenTaskStatus              status = GenTaskStatus::IDLE;  // 任务状态
    int                        progress = 0;          // 进度百分比 0-100
    std::optional<std::string> result_url;            // 结果文件下载链接 (COMPLETED 时有值)
    std::optional<std::string> preview_url;           // 预览图链接
    std::optional<std::string> error_message;         // 错误详情 (FAILED 时有值)
    int                        cost_credits = 0;      // 本次消耗积分
    int                        points_balance = 0;    // 积分余额
};
inline void to_json(nlohmann::json& j, const GenTaskResponse& v) {
    j["task_id"]   = v.task_id;
    j["status"]    = static_cast<int>(v.status);
    j["progress"]  = v.progress;
    if (v.triverse_task_uuid) j["triverse_task_uuid"] = *v.triverse_task_uuid;
    if (v.result_url)         j["result_url"]         = *v.result_url;
    if (v.preview_url)        j["preview_url"]        = *v.preview_url;
    if (v.error_message)      j["error_message"]      = *v.error_message;
    j["cost_credits"]   = v.cost_credits;
    j["points_balance"] = v.points_balance;
}
inline void from_json(const nlohmann::json& j, GenTaskResponse& v) {
    v.task_id   = j.value("task_id", "");
    v.progress  = j.value("progress", 0);
    v.status    = static_cast<GenTaskStatus>(j.value("status", 0));
    if (j.contains("triverse_task_uuid")) v.triverse_task_uuid = j.at("triverse_task_uuid").get<std::string>();
    if (j.contains("result_url"))         v.result_url         = j.at("result_url").get<std::string>();
    if (j.contains("preview_url"))        v.preview_url        = j.at("preview_url").get<std::string>();
    if (j.contains("error_message"))      v.error_message      = j.at("error_message").get<std::string>();
    v.cost_credits   = j.value("cost_credits", 0);
    v.points_balance = j.value("points_balance", 0);
}

// ============================================================================
// GenJobInfo_s — job 文件顶层结构, 对标 TaskProcess.h 的 JobInfo_s
//
// 生命周期: GenTaskAPI::SubmitGenTask 创建并写入 jobs_gen/Pending/
//          → GenTaskThread::ProcessPendingJobs 读取并 submit
//          → GenTaskThread::ProcessRunningJobs 轮询并更新状态
//          → 完成后移到 jobs_gen/Completed/ 或 jobs_gen/Failed/
//
// 序列化策略: 对标 JobInfo_s + JobFullInfo_s 模式。
//   GenJobInfo_s     — 纯数据 (对标 JobInfo_s), 仅 to_json/from_json
//   GenJobFullInfo_s — 文件级结构体 (对标 JobFullInfo_s), 持有 GenJobInfo_s + job_name,
//                    save()/load() 通过 JOB_INFO_USE_BIN 分发 BIN/JSON
//   GenJobInfoData/GenJobFile — DataStruct.h 中的 BIN 线格式 (对标 JobInfoData/JobListFile)
//
// 内联实现, 参照 JobFullInfo_s::save/load 模式, 无需 .cpp
// ============================================================================

/// @brief 单个生成式任务的纯数据结构 (对标 JobInfo_s, 不包含任何 I/O 方法)
struct GenJobInfo_s {
    // --- 客户端标识 ---
    std::string task_uuid;     // 客户端生成的 UUID, 全局唯一, 前端通过此 ID 查询
    std::string engine_id;     // 发起 Engine 的主机名
    std::string user_account;  // 用户账号 (submit 时传给服务端)

    // --- Block 关联 (定位 feedback 文件用) ---
    std::string project_path;  // 所属项目目录
    std::string block_item;    // 所属 Block 名称 (feedback 路径 = project/block_item/JF_job_name.bin/.json)

    // --- 任务参数 ---
    GenTaskCategory category = GenTaskCategory::TEXT_TO_3D;
    GenTaskSubType  sub_type = GenTaskSubType::TEXT_TO_MODEL;
    GenerationParams params;

    // --- 运行时状态 (终态时由 GenTaskThread 从 HTTP 响应回填) ---
    GenTaskStatus status = GenTaskStatus::IDLE;
    std::string server_task_id;    // 服务端 triverse_task_uuid (崩溃恢复判断依据)
    std::string result_url;         // 结果下载链接 (COMPLETED 时填充)
    std::string preview_url;        // 预览图链接
    std::string error_message;      // 详细错误信息 (FAILED 时填充)
    int cost_credits = 0;           // 本次消耗积分
    int points_balance = 0;         // 积分余额
    int query_retry_count = 0;      // 连续轮询失败次数 (>= 5 则标记失败)
};

// GenJobInfo_s 纯数据 to_json / from_json
inline void to_json(nlohmann::json& j, const GenJobInfo_s& v) {
    j["task_uuid"]    = v.task_uuid;
    j["engine_id"]    = v.engine_id;
    j["user_account"] = v.user_account;
    j["project_path"] = v.project_path;
    j["block_item"]   = v.block_item;
    j["category"]     = static_cast<int>(v.category);
    j["sub_type"]     = static_cast<int>(v.sub_type);
    j["params"]       = v.params;
    j["status"]       = static_cast<int>(v.status);
    if (!v.server_task_id.empty()) j["server_task_id"] = v.server_task_id;
    if (!v.result_url.empty())     j["result_url"]     = v.result_url;
    if (!v.preview_url.empty())    j["preview_url"]    = v.preview_url;
    if (!v.error_message.empty())  j["error_message"]  = v.error_message;
    j["cost_credits"]   = v.cost_credits;
    j["points_balance"] = v.points_balance;
    j["query_retry_count"] = v.query_retry_count;
}
inline void from_json(const nlohmann::json& j, GenJobInfo_s& v) {
    j.at("task_uuid").get_to(v.task_uuid);
    j.at("engine_id").get_to(v.engine_id);
    j.at("user_account").get_to(v.user_account);
    j.at("project_path").get_to(v.project_path);
    j.at("block_item").get_to(v.block_item);
    v.category = static_cast<GenTaskCategory>(j.at("category").get<int>());
    v.sub_type = static_cast<GenTaskSubType>(j.at("sub_type").get<int>());
    j.at("params").get_to(v.params);
    v.status = static_cast<GenTaskStatus>(j.value("status", 0));
    v.server_task_id    = j.value("server_task_id", "");
    v.result_url        = j.value("result_url", "");
    v.preview_url       = j.value("preview_url", "");
    v.error_message     = j.value("error_message", "");
    v.cost_credits      = j.value("cost_credits", 0);
    v.points_balance    = j.value("points_balance", 0);
    v.query_retry_count = j.value("query_retry_count", 0);
}

// ============================================================================
// GenJobFullInfo_s — 文件级结构体 (对标 JobFullInfo_s)
//
// 与 JobFullInfo_s 的对应关系:
//   JobFullInfo_s { JobName, TaskGraph_s { JobInfo_s, RunInfo_s, JobFeedBack_s, tasksmap } }
//   GenJobFullInfo_s { JobName, GenJobInfo_s }
//
// GenJobFullInfo_s 比 JobFullInfo_s 简单: 生成式任务没有子任务拆分 (tasksmap),
// 没有 RunInfo_s, 执行方式为 HTTP 调用而非 spawn 子进程。
//
// save/load 模式与 JobFullInfo_s 完全一致:
//   - JOB_INFO_USE_BIN = true  → WriteToBin/LoadFromBin (XOR 0xAB 加密)
//   - JOB_INFO_USE_BIN = false → WriteToJson/LoadFromJson (明文调试)
// ============================================================================

struct GenJobFullInfo_s {
    std::string job_name;    // J_<BlockName>_<timestamp>, 用作文件名, 对标 JobFullInfo_s::JobName
    GenJobInfo_s  job;         // 任务数据, 对标 JobFullInfo_s::tg.job (JobInfo_s)

    GenJobFullInfo_s() {}
    GenJobFullInfo_s(const std::string& file) {
        load(file);
    }

    // ========================================================================
    // WriteToBin / LoadFromBin — XOR 0xAB 加密 BIN
    // 对标 JobFullInfo_s::WriteToBin / LoadFromBin
    // ========================================================================

    bool WriteToBin(const std::string& filePath) const {
        std::ofstream out = File::OpenOfstreamUtf8(filePath, std::ios::binary);
        if (!out.is_open()) {
            LOGE("GenJobFullInfo_s::WriteToBin: failed to open: " + filePath);
            return false;
        }

        GenJobFile genJobFile;
        genJobFile.jobName = job_name;
        // GenJobInfo_s → GenJobInfoData (手动拷贝, 对标 JobFullInfo_s::WriteToBin)
        GenJobInfoData& d = genJobFile.genJobInfoData;
        d.task_uuid      = job.task_uuid;
        d.job_name       = job_name;
        d.engine_id      = job.engine_id;
        d.user_account   = job.user_account;
        d.project_path   = job.project_path;
        d.block_item     = job.block_item;
        d.category       = static_cast<int>(job.category);
        d.sub_type       = static_cast<int>(job.sub_type);
        d.status         = static_cast<int>(job.status);
        d.server_task_id = job.server_task_id;
        d.result_url     = job.result_url;
        d.preview_url    = job.preview_url;
        d.error_message  = job.error_message;
        d.cost_credits   = job.cost_credits;
        d.points_balance = job.points_balance;
        d.query_retry_count = job.query_retry_count;
        // GenerationParams 序列化为内嵌 JSON 字符串
        nlohmann::json paramsJson = job.params;
        d.params_json = paramsJson.dump();

        genJobFile.Serialize(out);
        out.close();
        return true;
    }

    bool LoadFromBin(const std::string& filePath) {
        std::ifstream in = File::OpenIfstreamUtf8(filePath, std::ios::binary);
        if (!in.is_open())
            return false;

        GenJobFile genJobFile;
        if (!genJobFile.Deserialize(in)) {
            in.close();
            return false;
        }
        in.close();

        job_name = genJobFile.jobName;
        // GenJobInfoData → GenJobInfo_s (手动拷贝, 对标 JobFullInfo_s::LoadFromBin)
        GenJobInfoData& d = genJobFile.genJobInfoData;
        job.task_uuid      = d.task_uuid;
        job.engine_id      = d.engine_id;
        job.user_account   = d.user_account;
        job.project_path   = d.project_path;
        job.block_item     = d.block_item;
        job.category       = static_cast<GenTaskCategory>(d.category);
        job.sub_type       = static_cast<GenTaskSubType>(d.sub_type);
        job.status         = static_cast<GenTaskStatus>(d.status);
        job.server_task_id = d.server_task_id;
        job.result_url     = d.result_url;
        job.preview_url    = d.preview_url;
        job.error_message  = d.error_message;
        job.cost_credits   = d.cost_credits;
        job.points_balance = d.points_balance;
        job.query_retry_count = d.query_retry_count;

        // 从内嵌 JSON 反序列化 GenerationParams
        try {
            job.params = nlohmann::json::parse(d.params_json).get<GenerationParams>();
        } catch (std::exception& ex) {
            LOGE(std::string("GenJobFullInfo_s::LoadFromBin: params parse error: ") + ex.what());
        }

        return true;
    }

    // ========================================================================
    // save / load — JOB_INFO_USE_BIN 分发 (对标 JobFullInfo_s::save/load)
    // ========================================================================

    bool save(const std::string& filePath) const {
        if (JOB_INFO_USE_BIN) {
            bool result = WriteToBin(filePath);
            if (!result) {
                LOGE("GenJobFullInfo_s::save: WriteToBin failed: " + filePath);
            }
            return result;
        }
        else {
            try {
                nlohmann::json j;
                j["job_name"] = job_name;
                j["job"]      = job;
                std::ofstream ofs = File::OpenOfstreamUtf8(filePath, std::ios::out);
                if (ofs.fail()) return false;
                ofs << j.dump(4);
                ofs.close();
            } catch (std::exception& ex) {
                LOGE(std::string("GenJobFullInfo_s::save JSON error: ") + ex.what());
                return false;
            }
            return true;
        }
    }

    bool load(const std::string& filePath) {
        if (JOB_INFO_USE_BIN) {
            bool result = LoadFromBin(filePath);
            if (!result) {
                LOGE("GenJobFullInfo_s::load: LoadFromBin failed: " + filePath);
            }
            return result;
        }
        else {
            std::ifstream ifs = File::OpenIfstreamUtf8(filePath, std::ios::in);
            if (ifs.fail()) return false;
            std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            ifs.close();
            if (str.empty()) return false;

            try {
                nlohmann::json j = nlohmann::json::parse(str);
                job_name = j.at("job_name").get<std::string>();
                job      = j.at("job").get<GenJobInfo_s>();
            } catch (std::exception& ex) {
                LOGE(std::string("GenJobFullInfo_s::load JSON error: ") + ex.what());
                return false;
            }
            return true;
        }
    }

    // ========================================================================
    // save_with_retry / load_with_retry — deny-write 锁 + 3 次重试
    // 对标 JobFeedBack_s::save_with_retry / load_with_retry, 但操作 GenJobFullInfo_s
    // ========================================================================

    bool save_with_retry(const std::string& filePath) const {
        int retryTimes = 0;
        do {
            FILE* fpLock = File::FopenDenyWriteLockUtf8(filePath + ".lock");
            if (fpLock == NULL) {
                retryTimes++;
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                continue;
            }
            bool result = save(filePath);
            fclose(fpLock);
            return result;
        } while (retryTimes < 3);

        LOGE(std::string("GenJobFullInfo_s::save_with_retry failed after 3 retries: ") + filePath);
        return false;
    }

    static GenJobFullInfo_s load_with_retry(const std::string& filePath) {
        int retryTimes = 0;
        do {
            FILE* fpLock = File::FopenDenyWriteLockUtf8(filePath + ".lock");
            if (fpLock == NULL) {
                retryTimes++;
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                continue;
            }
            GenJobFullInfo_s info;
            bool result = info.load(filePath);
            fclose(fpLock);
            if (result) return info;

            retryTimes++;
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        } while (retryTimes < 3);

        LOGE(std::string("GenJobFullInfo_s::load_with_retry failed after 3 retries: ") + filePath);
        return {};
    }
};

} // namespace CORE
} // namespace AI3D

#endif // _AI3D_UTIL_GEN_TASK_PROCESS_H_
```

### 1.2 修改 DataStruct.h — 增加 GenJobInfoData/GenJobFile + JobInfoData/BLKBinFile 扩展

> `GenJobFullInfo_s::WriteToBin/LoadFromBin` 引用了 `GenJobInfoData`/`GenJobFile`，两者定义在 DataStruct.h 中，对标 `FeedBackData`/`FeedBackFile` + `JobInfoData`/`JobListFile` 的 BIN 序列化模式。同时 `JobInfoData` 增加 `task_category`，`BLKBinFile` 增加 `block_task_category`。

- [ ] `Include/Core/DataStruct.h`：在 `FeedBackFile` 之后、`JobInfoData` 之前增加 `GenJobInfoData` + `GenJobFile`
- [ ] `Include/Core/DataStruct.h`：`JobInfoData` 增加 `int task_category = 0` + 更新 Serialize/Deserialize
- [ ] `Include/Core/DataStruct.h`：`BLKBinFile` 增加 `int block_task_category = 0` + 更新 Serialize/Deserialize

#### GenJobInfoData / GenJobFile — 生成式任务 BIN 序列化 (XOR 0xAB 加密)

> 对标 `FeedBackData`/`FeedBackFile` + `JobInfoData`/`JobListFile`。`GenerationParams` 序列化为 JSON 字符串存入 `params_json`，避免 BIN 格式随参数增减而频繁变动。

```cpp
// ============================================================================
// GenJobInfoData / GenJobFile — 生成式任务 BIN 序列化 (XOR 0xAB 加密)
// 对标 FeedBackData/FeedBackFile, 供 GenJobFullInfo_s::WriteToBin/LoadFromBin 使用
// ============================================================================

struct GenJobInfoData {
    std::string task_uuid;
    std::string job_name;
    std::string engine_id;
    std::string user_account;
    std::string project_path;
    std::string block_item;
    int category;
    int sub_type;
    int status;
    std::string server_task_id;
    std::string result_url;
    std::string preview_url;
    std::string error_message;
    int cost_credits;
    int points_balance;
    int query_retry_count;
    std::string params_json;       // GenerationParams 序列化为内嵌 JSON
    ByteCrypt byteCrypt;

    GenJobInfoData() {
        task_uuid = "";
        job_name = "";
        engine_id = "";
        user_account = "";
        project_path = "";
        block_item = "";
        category = 0;
        sub_type = 0;
        status = 0;
        server_task_id = "";
        result_url = "";
        preview_url = "";
        error_message = "";
        cost_credits = 0;
        points_balance = 0;
        query_retry_count = 0;
        params_json = "";
    }

    bool Serialize(std::ofstream& out) const {
        unsigned int task_uuid_len = task_uuid.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&task_uuid_len), sizeof(task_uuid_len));
        byteCrypt.WriteByteDecrypted(out, task_uuid.c_str(), task_uuid_len);
        unsigned int job_name_len = job_name.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&job_name_len), sizeof(job_name_len));
        byteCrypt.WriteByteDecrypted(out, job_name.c_str(), job_name_len);
        unsigned int engine_id_len = engine_id.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&engine_id_len), sizeof(engine_id_len));
        byteCrypt.WriteByteDecrypted(out, engine_id.c_str(), engine_id_len);
        unsigned int user_account_len = user_account.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&user_account_len), sizeof(user_account_len));
        byteCrypt.WriteByteDecrypted(out, user_account.c_str(), user_account_len);
        unsigned int project_path_len = project_path.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&project_path_len), sizeof(project_path_len));
        byteCrypt.WriteByteDecrypted(out, project_path.c_str(), project_path_len);
        unsigned int block_item_len = block_item.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&block_item_len), sizeof(block_item_len));
        byteCrypt.WriteByteDecrypted(out, block_item.c_str(), block_item_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&category), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&sub_type), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&status), sizeof(int));
        unsigned int server_task_id_len = server_task_id.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&server_task_id_len), sizeof(server_task_id_len));
        byteCrypt.WriteByteDecrypted(out, server_task_id.c_str(), server_task_id_len);
        unsigned int result_url_len = result_url.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&result_url_len), sizeof(result_url_len));
        byteCrypt.WriteByteDecrypted(out, result_url.c_str(), result_url_len);
        unsigned int preview_url_len = preview_url.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&preview_url_len), sizeof(preview_url_len));
        byteCrypt.WriteByteDecrypted(out, preview_url.c_str(), preview_url_len);
        unsigned int error_message_len = error_message.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&error_message_len), sizeof(error_message_len));
        byteCrypt.WriteByteDecrypted(out, error_message.c_str(), error_message_len);
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&cost_credits), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&points_balance), sizeof(int));
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&query_retry_count), sizeof(int));
        unsigned int params_json_len = params_json.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&params_json_len), sizeof(params_json_len));
        byteCrypt.WriteByteDecrypted(out, params_json.c_str(), params_json_len);
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        unsigned int task_uuid_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&task_uuid_len), sizeof(unsigned int));
        task_uuid.resize(task_uuid_len);
        byteCrypt.ReadByteDecrypted(in, &task_uuid[0], task_uuid_len);
        unsigned int job_name_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&job_name_len), sizeof(unsigned int));
        job_name.resize(job_name_len);
        byteCrypt.ReadByteDecrypted(in, &job_name[0], job_name_len);
        unsigned int engine_id_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&engine_id_len), sizeof(unsigned int));
        engine_id.resize(engine_id_len);
        byteCrypt.ReadByteDecrypted(in, &engine_id[0], engine_id_len);
        unsigned int user_account_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&user_account_len), sizeof(unsigned int));
        user_account.resize(user_account_len);
        byteCrypt.ReadByteDecrypted(in, &user_account[0], user_account_len);
        unsigned int project_path_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&project_path_len), sizeof(unsigned int));
        project_path.resize(project_path_len);
        byteCrypt.ReadByteDecrypted(in, &project_path[0], project_path_len);
        unsigned int block_item_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&block_item_len), sizeof(unsigned int));
        block_item.resize(block_item_len);
        byteCrypt.ReadByteDecrypted(in, &block_item[0], block_item_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&category), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&sub_type), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&status), sizeof(int));
        unsigned int server_task_id_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&server_task_id_len), sizeof(unsigned int));
        server_task_id.resize(server_task_id_len);
        byteCrypt.ReadByteDecrypted(in, &server_task_id[0], server_task_id_len);
        unsigned int result_url_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&result_url_len), sizeof(unsigned int));
        result_url.resize(result_url_len);
        byteCrypt.ReadByteDecrypted(in, &result_url[0], result_url_len);
        unsigned int preview_url_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&preview_url_len), sizeof(unsigned int));
        preview_url.resize(preview_url_len);
        byteCrypt.ReadByteDecrypted(in, &preview_url[0], preview_url_len);
        unsigned int error_message_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&error_message_len), sizeof(unsigned int));
        error_message.resize(error_message_len);
        byteCrypt.ReadByteDecrypted(in, &error_message[0], error_message_len);
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&cost_credits), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&points_balance), sizeof(int));
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&query_retry_count), sizeof(int));
        unsigned int params_json_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&params_json_len), sizeof(unsigned int));
        params_json.resize(params_json_len);
        byteCrypt.ReadByteDecrypted(in, &params_json[0], params_json_len);
        return true;
    };
};

struct GenJobFile {
    std::string jobName;       // 文件名 (对标 JobListFile::jobName)
    GenJobInfoData genJobInfoData;
    ByteCrypt byteCrypt;

    GenJobFile() {}

    bool Serialize(std::ofstream& out) const {
        const char SOURCE_HEADER_LABEL[] = "GENJOB-FILE-3MO";
        std::string header(SOURCE_HEADER_LABEL, 15);
        byteCrypt.WriteByteDecrypted(out, header.c_str(), 15);

        unsigned int jobName_len = jobName.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&jobName_len), sizeof(jobName_len));
        byteCrypt.WriteByteDecrypted(out, jobName.c_str(), jobName_len);

        genJobInfoData.Serialize(out);
        return true;
    };

    bool Deserialize(std::ifstream& in) {
        char header[15];
        byteCrypt.ReadByteDecrypted(in, header, sizeof(header));
        std::string decrypted_header(header, 15);
        const char SOURCE_HEADER_LABEL[] = "GENJOB-FILE-3MO";
        if (decrypted_header != std::string(SOURCE_HEADER_LABEL, 15)) {
            return false;
        }

        unsigned int jobName_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&jobName_len), sizeof(unsigned int));
        jobName.resize(jobName_len);
        byteCrypt.ReadByteDecrypted(in, &jobName[0], jobName_len);

        genJobInfoData.Deserialize(in);
        return true;
    };
};
```

#### JobInfoData — 在现有字段末尾增加

```cpp
// JobInfoData 中增加 (在 itemPath 字段之后):
int task_category = 0;  // 0=重建, 1=生成式

// Serialize 中增加:
byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&task_category), sizeof(int));

// Deserialize 中增加:
byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&task_category), sizeof(int));
```

#### BLKBinFile — 在现有字段末尾增加

```cpp
// BLKBinFile 中增加:
int block_task_category = 0;  // 0=重建(默认), 1=生成式

// Serialize 中增加:
byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&block_task_category), sizeof(int));

// Deserialize 中增加:
byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&block_task_category), sizeof(int));
```

> **兼容性**: 旧文件没有这些字段 → Deserialize 后保持默认值 0。新字段追加在末尾，旧 reader 读完已知字段后停止，不受影响。

> **魔数汇总**:
> | 文件类型 | 魔数 | 用途 |
> |----------|------|------|
> | Feedback | `FEED-FILE-3MO` | JobFeedBack_s |
> | Task Def | `TASKDEF-FILE-3MO` | ATTaskInfo |
> | Job List | `JLIST-FILE-3MO` | JobFullInfo_s |
> | Block | `BBLK-FILE-3MO` | BlockObject |
> | TimeSum | `TTIME-FILE-3MO` | ATTimeSummary_s |
> | **GenJob** | **`GENJOB-FILE-3MO`** | **GenJobFullInfo_s (新增)** |

### 1.3 修改 BlockObject — Task_Info 扩展

- [ ] `Include/Core/BlockObject.h`：`Task_Info` 增加 `int block_task_category = 0`
- [ ] `Include/Core/DataStruct.h`：`BLKBinFile` 增加 `int block_task_category = 0` + `Serialize`/`Deserialize` 更新
- [ ] `Include/Core/BlockObject.cpp`：4 个序列化方法 (`WriteBlockInfoToJson`/`ReadBlockInfoJson`/`WriteBlockInfoToBin`/`ReadBlockInfoBin`) 增加新字段读写

#### 修改 BlockObject.h —— 在 `Task_Info` 中增加字段

在 `task_keyMaxImgNum` 之前（line ~122）插入：

```cpp
// ==== 新增: 任务分类路由 ====
int  block_task_category = 0;   // 0=重建(默认), 1=生成式
```

#### 修改 BlockObject.cpp —— JSON 序列化

`WriteBlockInfoToJson()` 中，在现有 blockName 等字段写入之后增加：

```cpp
// 写入
njson["block_task_category"] = block_info_.block_task_category;
```

`ReadBlockInfoJson()` 中，读取处增加：

```cpp
// 读取
if (njson.contains("block_task_category"))
    block_info_.block_task_category = njson["block_task_category"].get<int>();
```

#### 修改 BlockObject.cpp —— BIN 序列化

`WriteBlockInfoToBin()` 中，在现有字段后追加：

```cpp
bLKBinFile.block_task_category = block_task_category;
```

`ReadBlockInfoBin()` 中，读取处增加：

```cpp
block_task_category = bLKBinFile.block_task_category;
```

> **注**: BIN 序列化使用 `BLKBinFile`（定义在 `Include/Core/DataStruct.h`），不经过 `BlockInfo.h`。`BLKBinFile::Serialize/Deserialize` 中已增加 `block_task_category` 的读写（见 Phase 1.1b）。

### 1.4 修改 JobInfo_s

- [ ] `Include/Util/TaskProcess.h`：`JobInfo_s` 增加 `int task_category = 0`

在 `JobInfo_s` 结构体中（ProjectPath/ItemPath 之后）增加：

```cpp
int task_category = 0;  // 0=重建, 1=生成式
```

#### 更新 6 个序列化方法

**WriteToJson()** — 在 return 前：
```cpp
json_str["task_category"] = task_category;
```

**WriteToJson2()** — 在 document.Accept(writer) 前：
```cpp
document.AddMember("task_category", rapidjson::Value(task_category), allocator);
```

**WriteToJson3()** — 在末尾：
```cpp
value.AddMember("task_category", rapidjson::Value(task_category), allocator);
```

**CreateFromJson()** — 在 return 前：
```cpp
if (json_str.find("task_category") != json_str.end())
    jobinfo.task_category = json_str.at("task_category").get<int>();
```

**CreateFromJsonV2()** — 在 return 前：
```cpp
if (document.HasMember("task_category") && document["task_category"].IsInt())
    jobinfo.task_category = document["task_category"].GetInt();
```

**CreateFromJsonV3()** — 在 return 前：
```cpp
if (value.HasMember("task_category"))
    jobinfo.task_category = value["task_category"].GetInt();
```

#### JobFullInfo_s::WriteToBin / LoadFromBin — BIN 路径增加 task_category

> `JobFullInfo_s` (J_*.bin 的主结构体) 的 `WriteToBin` 和 `LoadFromBin` 手动逐字段拷贝 `JobInfo_s` → `JobInfoData`。`task_category` 需要在 BIN 中持久化，否则重建式 Job 读回时丢失。

**WriteToBin()** — 在 `jobListFile.jobInfoData.itemPath = ...` 之后增加：

```cpp
jobListFile.jobInfoData.task_category = tg.job.task_category;
```

**LoadFromBin()** — 在 `tg.job.ItemPath = jobListFile.jobInfoData.itemPath` 之后增加：

```cpp
tg.job.task_category = jobListFile.jobInfoData.task_category;
```

> **注**: `JobInfoData` (DataStruct.h) 中 `task_category` 的 Serialize/Deserialize 已在 Phase 1.2 中完成。JSON 路径 (`WriteToJson`/`load` JSON 分支) 委托给 `JobInfo_s` 自己的序列化方法，`task_category` 自动传导，无需额外修改。

### 1.5 JobFeedBack_s — 不修改

> `JobFeedBack_s`（`FeedBackFile`）是固定格式的进度反馈文件，只有 `Status`/`Percent`/`Msg`/`TaskRetVal` 四个字段。生成式任务直接复用：
> - `Status` → 映射 `GenTaskStatus` → `jobsta_e`
> - `Percent` → 进度百分比
> - `Msg` → 状态描述文字
> - `TaskRetVal` → 0=成功, 非0=失败
>
> **结果数据**（`result_url`/`preview_url`/`server_task_id`/`cost_credits`/`error_message`）全部存在 `GenJobInfo_s` 自身中，不做为 feedback 的扩展字段。前端通过 `GenTaskAPI::QueryTaskStatus` 读取 `GenJobInfo_s` 获取结果。

### 1.6 编译验证与 CMake 修改

- [ ] 头文件变更后全量编译通过

#### App/Engine/CMakeLists.txt — 增加 Util/ 下的 GenTaskProcess.h

`App/Engine/CMakeLists.txt` 对 Engine 目录使用 `FILE(GLOB *.cpp *.h)` 自动拾取，但 `Include/Util/` 下的文件需要**显式添加**（模仿已有的 `TaskProcess.h` 模式）。

找到 HEADER_LIST GLOB（约 line 28-37），在 `TaskProcess.h` 下方新增一行：

```cmake
FILE(GLOB HEADER_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/Util/CatchProcess.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/Util/Settings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/Util/TaskProcess.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/Util/GenTaskProcess.h     # ← 新增
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/Util/Statistic.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/Util/JobMonitor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/Util/OTA.h
)
```

> `GenHttpClient.h/cpp` 和 `GenTaskThread.h/cpp` 放在 `App/Engine/` 目录下，被 GLOB 自动拾取，**不需要**显式添加。

#### Src/Core/CMakeLists.txt — 无需修改

`GenTaskAPI.h` 在 `Include/Core/` 下，`GenTaskAPI.cpp` 在 `Src/Core/` 下，被 `FILE(GLOB)` 自动拾取。`Qt6::Network` 已链接。

#### 不需要 GenTaskProcess.cpp

`GenJobFullInfo_s::save_with_retry/load_with_retry` 内联在 `GenTaskProcess.h` 头文件中（参照 `JobFeedBack_s::save_with_retry/load_with_retry` 的同款写法，内含 deny-write 锁 + 最多 3 次重试），不需要单独的 `.cpp` 文件，也无需修改 `Src/Util/` 下的 CMakeLists。

---

## Phase 2: 路径配置

> 不依赖 Phase 1，可与 Phase 1 并行开工。

### 2.1 Settings 增加 gen 队列路径

- [ ] `Include/Util/Settings.h`：增加声明

在 `getEngineJobQueue()` 下方增加：

```cpp
// Include/Util/Settings.h
static QString getGenEngineJobQueue();  // 新增: 生成式任务队列路径
```

- [ ] `Src/Util/Settings.cpp`：实现

在 `getEngineJobQueue()` 实现下方增加：

```cpp
// Src/Util/Settings.cpp
QString Settings::getGenEngineJobQueue()
{
    QString enginePath = getEngineJobQueue();
    if (enginePath.isEmpty())
        return "";

    QDir parent = QFileInfo(enginePath).dir();
    return parent.absolutePath() + "/jobs_gen";
}
```

### 2.2 main() 创建 jobs_gen 目录

- [ ] `App/Engine/CallEngine.cpp`：在文件顶部（其他全局变量声明附近，如 `bQuitingApplication`）增加全局变量定义：

```cpp
// ===== 新增: 生成式任务路径 (全局变量, GenTaskThread 通过 extern 引用) =====
std::string genPendingJobPath;
std::string genRunningJobPath;
std::string genCompletedJobPath;
std::string genFailedJobPath;
std::string genCancelledJobPath;
```

- [ ] `App/Engine/CallEngine.cpp`：在 `MakePath()` 函数末尾增加目录创建 + **对上述全局变量赋值**（无类型声明）：

在 `MakePath()` 函数末尾（line ~4343，cancelledJobPath 赋值之后）增加：

```cpp
// ===== 新增: 生成式任务目录 =====
std::string genEnginePath = qstr2str(Settings::getGenEngineJobQueue());
if (!JobMonitor::CreateJobQueueDir(str2qstr(genEnginePath)))
{
    JobMonitor::CreateLocalJobQueueDir();
    JobMonitor::CreateJobQueueDir(str2qstr(genEnginePath));
}

// 对全局变量赋值 (去掉 std::string 类型前缀, 否则会变成局部变量遮蔽全局)
genPendingJobPath   = genEnginePath + pathSeperator + JOBPENDINGSTR   + pathSeperator;
genRunningJobPath   = genEnginePath + pathSeperator + JOBRUNNINGSTR   + pathSeperator;
genCompletedJobPath = genEnginePath + pathSeperator + JOBCOMPLETEDSTR + pathSeperator;
genFailedJobPath    = genEnginePath + pathSeperator + JOBFAILEDSTR     + pathSeperator;
genCancelledJobPath = genEnginePath + pathSeperator + JOBCANCELLEDSTR + pathSeperator;
```

### 2.3 启动时清理 jobs_gen/ 残留锁文件

- [ ] `App/Engine/CallEngine.cpp`：`doCleanupJobLockOnceWhileEngineStart()` 增加 jobs_gen/ 的清理

现有 `doCleanupJobLockOnceWhileEngineStart()` 调用 `DoCleanupLockFiles()` 扫描 `jobs/` 下所有状态目录移除残留 `.lock` 文件和 `pid_*.bin`。需要为 `jobs_gen/` 增加同样的清理：

```cpp
// 在 doCleanupJobLockOnceWhileEngineStart() 中，现有 jobs/ 清理逻辑之后增加:
std::string genEnginePath = qstr2str(Settings::getGenEngineJobQueue());
if (!genEnginePath.empty()) {
    std::vector<std::string> genDirs = {
        genEnginePath + pathSeperator + JOBPENDINGSTR,
        genEnginePath + pathSeperator + JOBRUNNINGSTR,
        genEnginePath + pathSeperator + JOBCOMPLETEDSTR,
        genEnginePath + pathSeperator + JOBFAILEDSTR,
        genEnginePath + pathSeperator + JOBCANCELLEDSTR
    };
    for (const auto& dir : genDirs) {
        DoCleanupLockFiles(str2qstr(dir));
    }
}
```

> **为什么不支持优先级子目录**: `jobs_gen/` 暂不实现 `High/Normal/Low` 优先级分层。生成式任务由 HTTP 服务端调度，本地仅串行 submit + 并行轮询，优先级由服务端 SLA 保证。

---

## Phase 3: HTTP 通信层

> 依赖 Phase 1 的数据结构（GenTaskRequest / GenTaskResponse）。

### 3.1 新建 GenHttpClient

- [ ] 创建 `App/Engine/GenHttpClient.h`

```cpp
// App/Engine/GenHttpClient.h
// ============================================================================
// @brief 同步 HTTP 客户端 (QNetworkAccessManager + QEventLoop + QTimer)
//        所有 HTTP 方法均为同步阻塞, 内部带超时和自动重试
//        对标重建式任务的 spawn MoldAITask.exe 子进程: 两种"提交任务并等待结果"方式
// ============================================================================
#pragma once

#include "Util/GenTaskProcess.h"
#include <QString>

namespace AI3D {
namespace CORE {

/// @brief 同步 HTTP 客户端 — 封装与 MoldAI 后端 (→ Triverse) 的 REST API 通信
class GenHttpClient
{
public:
    /// @brief 从 MoldAIConfig.ini [GenTask] 段读取 ServerUrl / ApiPrefix
    ///        accessToken 从注册表读取 (与现有 HttpClient 一致, 登录后由 User 模块写入)
    static void Init(const std::string& configPath);

    // --- HTTP 方法 (同步阻塞, 带超时和重试) ---

    /// @brief POST /api/v1/task/submit — 提交生成任务
    static GenTaskResponse SubmitTask(const GenTaskRequest& req,
                                       int timeout_ms = 5000,
                                       int max_retries = 3);

    /// @brief GET /api/v1/task/status?task_id=<server_task_id> — 查询任务状态
    static GenTaskResponse QueryTaskStatus(const std::string& server_task_id,
                                            int timeout_ms = 3000,
                                            int max_retries = 3);

    /// @brief POST /api/v1/task/cancel — 取消任务
    static bool CancelTask(const std::string& server_task_id,
                           int timeout_ms = 3000,
                           int max_retries = 3);

    /// @brief POST /api/v1/upload — 上传本地文件, 返回 file_key (失败返回空)
    static std::string UploadFile(const std::string& local_path,
                                   int timeout_ms = 10000,
                                   int max_retries = 2);

private:
    /// @brief 根据 服务端接口安全.md 规范构建 Authorization 头
    ///        sign = MD5(moldai:<path>:<timestamp>:<data>:<token>).toBase64()
    ///        与 Src/Util/HttpClient::calSign() 算法完全一致
    static QString BuildAuthHeader(const QString& url,
                                   const QString& dataJson = "");

    /// @brief 从注册表读取当前登录用户的 accessToken
    static QString LoadAccessToken();

    /// @brief 同步 GET (QNetworkAccessManager + QEventLoop + QTimer)
    static QByteArray SyncGet(const QString& url, int timeout_ms);

    /// @brief 同步 POST JSON (JSON body)
    static QByteArray SyncPost(const QString& url,
                                const QJsonObject& body,
                                int timeout_ms);

    /// @brief 同步 POST multipart/form-data (文件上传, 无 JSON body, 签名中 data 为空)
    static QByteArray SyncPostMultipart(const QString& url,
                                         const QString& filePath,
                                         int timeout_ms);

    static QString s_serverUrl;   // 服务端基地址, 如 http://api.example.com
    static QString s_apiPrefix;   // API 前缀, 如 /api/v1
    static QString s_accessToken; // 登录后的 accessToken (从注册表读取, 可被 Init 刷新)
};

}} // namespace AI3D::CORE

- [ ] 创建 `App/Engine/GenHttpClient.cpp`

```cpp
// App/Engine/GenHttpClient.cpp
// ============================================================================
// @brief 同步 HTTP 客户端实现 (Triverse AI 生成服务)
//
// 使用 QNetworkAccessManager + QEventLoop + QTimer 实现同步 HTTP 请求,
// 此模式与项目中 Src/Util/HttpClient.cpp 一致 (现有 HttpClient 用于 MoldAI
// 自有后端的登录/更新/鉴权, 本类用于 Triverse 生成服务, 两者服务端不同)。
//
// 与现有 HttpClient 的区别:
//   - 静态方法工具类 (非 QObject), 适合在 std::thread 中使用
//   - 直接 return 值, 不使用 std::function 回调
//   - 解析 GenTaskRequest/GenTaskResponse (而非 errorCode/errorMsg)
//   - 内置自动重试 (max_retries 次, 间隔 500ms)
//   - 支持 multipart 文件上传
//
// 每个方法内部自动重试, 失败后 sleep(500ms) 再试。
// ============================================================================

#include "GenHttpClient.h"
#include "Core/Logging.h"          // LOGI / LOGE / LOGW
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHttpMultiPart>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QSettings>               // 注册表读取 accessToken (与 HttpClient 一致)
#include <QDateTime>               // BuildAuthHeader 时间戳
#include <QCryptographicHash>      // BuildAuthHeader MD5 签名
#include <thread>
#include <chrono>

namespace AI3D {
namespace CORE {

// 默认值 — 开发环境 (Mock Server 不需要 accessToken)
QString GenHttpClient::s_serverUrl   = "http://localhost:8080";
QString GenHttpClient::s_apiPrefix   = "/api/v1";
QString GenHttpClient::s_accessToken = "";

// ============================================================================
// Init — 从 MoldAIConfig.ini [GenTask] 段读取 ServerUrl / ApiPrefix
//        accessToken 从注册表读取 (与 HttpClient 相同路径, 登录后由 User 模块写入)
// ============================================================================

void GenHttpClient::Init(const std::string& configPath)
{
    QSettings settings(QString::fromStdString(configPath), QSettings::IniFormat);
    settings.beginGroup("GenTask");
    s_serverUrl = settings.value("ServerUrl", s_serverUrl).toString();
    s_apiPrefix = settings.value("ApiPrefix", s_apiPrefix).toString();
    settings.endGroup();

    // 从注册表读取当前登录用户的 accessToken (与 Src/Util/HttpClient.cpp 一致)
    s_accessToken = LoadAccessToken();
}

// ============================================================================
// LoadAccessToken — 从注册表读取 accessToken
// 路径: HKCU\Software\MoldAI\User → CURRENT_PREFIX (当前用户名)
//       → TOKEN_PREFIX + 用户名 → accessToken
// 与 Src/Util/HttpClient::post() 中的读取逻辑完全一致
// ============================================================================

QString GenHttpClient::LoadAccessToken()
{
    QSettings* pSettings = new QSettings(SETTINGS_PREFIX + "\\User", QSettings::NativeFormat);

    QString currentUser = pSettings->value(CURRENT_PREFIX, "").toString();
    if (currentUser.isEmpty()) {
        delete pSettings;
        return "";
    }

    QString tokenKey = TOKEN_PREFIX + currentUser;
    QString currentToken = pSettings->value(tokenKey, "").toString();
    delete pSettings;
    return currentToken;
}

// ============================================================================
// BuildAuthHeader — 按 服务端接口安全.md 规范构建 Authorization 头
//
// 算法 (与 Src/Util/HttpClient::calSign() 完全一致):
//   1. 提取 URL 路径 (去掉域名)
//   2. 拼接 finalStr = moldai:<path>:<timestamp>:<data>:<accessToken>
//      空值部分 (data, accessToken) 不参与拼接
//   3. sign = MD5(finalStr).toBase64()
//   4. 返回 "timestamp:<ts>,sign:<sign>[,accessToken:<token>]"
// ============================================================================

QString GenHttpClient::BuildAuthHeader(const QString& url,
                                        const QString& dataJson)
{
    // 1. 提取 URL 路径 (去掉 scheme://host:port 前缀)
    QString path = url;
    path.remove(s_serverUrl);  // s_serverUrl 形如 http://api.example.com

    // 2. 时间戳 (秒級 Unix timestamp)
    QString timestampStr = QString::number(QDateTime::currentDateTime().toSecsSinceEpoch());

    // 3. 按规范拼接: moldai:<path>:<timestamp>:<data>:<accessToken>
    //    空值部分跳过 (data 可能为空, accessToken 可能为空)
    QString finalStr = "moldai:" + path + ":" + timestampStr;

    if (!dataJson.isEmpty()) {
        finalStr += ":" + dataJson;
    }

    if (!s_accessToken.isEmpty()) {
        finalStr += ":" + s_accessToken;
    }

    // 4. MD5 → Base64
    QByteArray byteArray  = finalStr.toUtf8();
    QByteArray md5        = QCryptographicHash::hash(byteArray, QCryptographicHash::Md5);
    QByteArray signBase64 = md5.toBase64();

    // 5. 组装 Authorization header
    QString authHeader = "timestamp:" + timestampStr + ",sign:" + signBase64;

    if (!s_accessToken.isEmpty()) {
        authHeader += ",accessToken:" + s_accessToken;
    }

    return authHeader;
}

// ============================================================================
// SyncGet — 同步 GET, 使用 QEventLoop 阻塞等待
// 返回 QByteArray, 失败时为空
// ============================================================================

QByteArray GenHttpClient::SyncGet(const QString& url, int timeout_ms)
{
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(timeout_ms);
    // 签名鉴权 (GET 无 body, dataJson 为空)
    request.setRawHeader("Authorization", BuildAuthHeader(url, "").toUtf8());

    QNetworkReply* reply = manager.get(request);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        reply->abort();
        loop.quit();
    });
    timer.start(timeout_ms);
    loop.exec();

    QByteArray result;
    if (reply->error() == QNetworkReply::NoError) {
        result = reply->readAll();
    } else {
        LOGW("SyncGet failed: " + reply->errorString().toStdString()
             + " url: " + url.toStdString());
    }

    reply->deleteLater();
    return result;
}

// ============================================================================
// SyncPost — 同步 POST JSON, 与 SyncGet 相同的 QEventLoop 模式
// ============================================================================

QByteArray GenHttpClient::SyncPost(const QString& url,
                                     const QJsonObject& body,
                                     int timeout_ms)
{
    QNetworkAccessManager manager;

    // 先序列化 body → 签名时 dataJson 必须与实际发送的 body 一致
    QJsonDocument doc(body);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    QNetworkRequest request(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(timeout_ms);
    // 签名鉴权 (dataJson = body 的 JSON 字符串)
    request.setRawHeader("Authorization",
        BuildAuthHeader(url, QString::fromUtf8(jsonData)).toUtf8());

    QNetworkReply* reply = manager.post(request, jsonData);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        reply->abort();
        loop.quit();
    });
    timer.start(timeout_ms);
    loop.exec();

    QByteArray result;
    if (reply->error() == QNetworkReply::NoError) {
        result = reply->readAll();
    } else {
        LOGW("SyncPost failed: " + reply->errorString().toStdString()
             + " url: " + url.toStdString());
    }

    reply->deleteLater();
    return result;
}

// ============================================================================
// SyncPostMultipart — 同步 POST multipart/form-data (文件上传)
// QHttpMultiPart 生命周期由 QNetworkReply 管理 (setParent)
// ============================================================================

QByteArray GenHttpClient::SyncPostMultipart(const QString& url,
                                              const QString& filePath,
                                              int timeout_ms)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        LOGE("UploadFile: cannot open " + filePath.toStdString());
        return {};
    }

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    QFileInfo fi(filePath);
    QMimeDatabase mimeDb;
    QString mimeType = mimeDb.mimeTypeForFile(fi).name();
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, mimeType);
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QString("form-data; name=\"file\"; filename=\"%1\"").arg(fi.fileName()));
    filePart.setBodyDevice(&file);
    file.setParent(multiPart);  // multiPart 负责释放
    multiPart->append(filePart);

    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(url));
    request.setTransferTimeout(timeout_ms);
    // 签名鉴权 (multipart 无 JSON body, dataJson 为空)
    request.setRawHeader("Authorization", BuildAuthHeader(url, "").toUtf8());

    QNetworkReply* reply = manager.post(request, multiPart);
    multiPart->setParent(reply);  // reply 负责释放 multiPart

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        reply->abort();
        loop.quit();
    });
    timer.start(timeout_ms);
    loop.exec();

    QByteArray result;
    if (reply->error() == QNetworkReply::NoError) {
        result = reply->readAll();
    } else {
        LOGW("SyncPostMultipart failed: " + reply->errorString().toStdString());
    }

    reply->deleteLater();
    return result;
}

// ============================================================================
// SubmitTask — POST /api/v1/task/submit
// 将 GenTaskRequest 序列化为 JSON, 发送到服务端, 解析返回的 GenTaskResponse
// ============================================================================

GenTaskResponse GenHttpClient::SubmitTask(const GenTaskRequest& req,
                                            int timeout_ms,
                                            int max_retries)
{
    nlohmann::json reqJson = req;
    QJsonObject body = QJsonDocument::fromJson(
        QByteArray::fromStdString(reqJson.dump())).object();

    QString url = s_serverUrl + s_apiPrefix + "/task/submit";

    for (int attempt = 0; attempt <= max_retries; attempt++) {
        QByteArray raw = SyncPost(url, body, timeout_ms);

        if (!raw.isEmpty()) {
            try {
                nlohmann::json respJson = nlohmann::json::parse(raw.toStdString());
                return respJson.get<GenTaskResponse>();
            } catch (std::exception& ex) {
                LOGE("SubmitTask parse error: " + std::string(ex.what()));
            }
        }

        if (attempt < max_retries) {
            LOGW("SubmitTask retry " + std::to_string(attempt + 1) + "/" + std::to_string(max_retries));
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    // 所有重试失败, 返回网络超时状态
    GenTaskResponse failResp;
    failResp.task_id = req.task_id;
    failResp.status = GenTaskStatus::IDLE;  // IDLE 表示网络不通 (区别于服务端 FAILED)
    failResp.error_message = "network timeout after " + std::to_string(max_retries + 1) + " attempts";
    return failResp;
}

// ============================================================================
// QueryTaskStatus — GET /api/v1/task/status?task_id=<server_task_id>
// ============================================================================

GenTaskResponse GenHttpClient::QueryTaskStatus(const std::string& server_task_id,
                                                 int timeout_ms,
                                                 int max_retries)
{
    QString url = s_serverUrl + s_apiPrefix + "/task/status?task_id="
                + QString::fromStdString(server_task_id);

    for (int attempt = 0; attempt <= max_retries; attempt++) {
        QByteArray raw = SyncGet(url, timeout_ms);

        if (!raw.isEmpty()) {
            try {
                nlohmann::json respJson = nlohmann::json::parse(raw.toStdString());
                return respJson.get<GenTaskResponse>();
            } catch (std::exception& ex) {
                LOGE("QueryTaskStatus parse error: " + std::string(ex.what()));
            }
        }

        if (attempt < max_retries) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    GenTaskResponse failResp;
    failResp.server_task_id = server_task_id;
    failResp.status = GenTaskStatus::IDLE;
    failResp.error_message = "query timeout after retries";
    return failResp;
}

// ============================================================================
// CancelTask — POST /api/v1/task/cancel, 返回 true=成功
// ============================================================================

bool GenHttpClient::CancelTask(const std::string& server_task_id,
                                int timeout_ms,
                                int max_retries)
{
    QJsonObject body;
    body["task_id"] = QString::fromStdString(server_task_id);

    QString url = s_serverUrl + s_apiPrefix + "/task/cancel";

    for (int attempt = 0; attempt <= max_retries; attempt++) {
        QByteArray raw = SyncPost(url, body, timeout_ms);

        if (!raw.isEmpty()) {
            // 服务端返回 200 即可, 不强校验响应体
            return true;
        }

        if (attempt < max_retries) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    return false;
}

// ============================================================================
// UploadFile — POST /api/v1/upload, 上传本地文件, 返回 file_key (失败返回空)
// ============================================================================

std::string GenHttpClient::UploadFile(const std::string& local_path,
                                        int timeout_ms,
                                        int max_retries)
{
    QString url = s_serverUrl + s_apiPrefix + "/upload";

    for (int attempt = 0; attempt <= max_retries; attempt++) {
        QByteArray raw = SyncPostMultipart(url, QString::fromStdString(local_path), timeout_ms);

        if (!raw.isEmpty()) {
            try {
                nlohmann::json respJson = nlohmann::json::parse(raw.toStdString());
                if (respJson.contains("file_key")) {
                    return respJson["file_key"].get<std::string>();
                }
            } catch (std::exception& ex) {
                LOGE("UploadFile parse error: " + std::string(ex.what()));
            }
        }

        if (attempt < max_retries) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    return ""; // 空字符串表示失败
}

}} // namespace AI3D::CORE
```

### 3.2 CMakeLists — 无需修改

`GenHttpClient.h/cpp` 放在 `App/Engine/` 目录下，被现有 `FILE(GLOB *.cpp *.h)` 自动拾取。`Qt6::Network` 已在 `App/Engine/CMakeLists.txt` line 55 链接，无需额外添加。

> 参照: `App/Engine/CMakeLists.txt` line 40-43 的 `FILE(GLOB SRC_LIST ... "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")`，所有 `App/Engine/*.cpp` 自动编译。

### 3.3 手动验证

- [ ] 用已知的 Triverse endpoint 测试 submit / query / cancel
- [ ] 测试超时和重试行为

---

## Phase 4: 调度线程

> 依赖 Phase 2（路径可用）和 Phase 3（HTTP 客户端可用）。

### 4.1 新建 GenTaskThread

- [ ] 创建 `App/Engine/GenTaskThread.h`

```cpp
// App/Engine/GenTaskThread.h
// ============================================================================
// @brief 生成式任务调度线程 (对标 searchPendingJobThread2)
//        在独立 std::thread 中运行, 每 2s 轮询 jobs_gen/ 目录
// ============================================================================
#pragma once

#include <string>

namespace AI3D {
namespace CORE {

/// @brief 生成式任务调度线程 (所有方法均为静态, 通过 Run() 启动)
class GenTaskThread
{
public:
    /// @brief 线程入口 — 在独立 std::thread 中调用, 死循环轮询
    static void Run();

    /// @brief 取消生成式任务 (HTTP POST /cancel + 移文件到 Cancelled/)
    static bool CancelGenTask(const std::string& task_uuid);

    /// @brief 异常任务扫描线程入口 (对标 searchUnnormaldRunningJobThread)
    ///        扫描 Running/ 中长时间无进展的 job, 超时标记 FAILED 或移回 Pending
    static void SearchUnnormalRunningJob();

private:
    /// @brief 扫描 jobs_gen/Pending/ → submit 新任务 → 移到 Running/
    static void ProcessPendingJobs();
    /// @brief 扫描 jobs_gen/Running/ → 轮询服务端 → 移到 Completed/ 或 Failed/
    static void ProcessRunningJobs();
    /// @brief 跨平台 sleep (std::this_thread::sleep_for)
    static void SleepMs(int ms);
};

}} // namespace AI3D::CORE
```

- [ ] 创建 `App/Engine/GenTaskThread.cpp`

```cpp
// App/Engine/GenTaskThread.cpp
// ============================================================================
// @file    GenTaskThread.cpp
// @brief   生成式任务调度线程, 对标 searchPendingJobThread2 (重建式)
//
// 独立线程, 在 CallEngine::main() 中以 std::thread 启动。
// 每 2 秒轮询一次 jobs_gen/ 目录:
//   ProcessPendingJobs() — 扫描 jobs_gen/Pending/, submit 新任务到服务端
//   ProcessRunningJobs() — 扫描 jobs_gen/Running/, 轮询服务端任务状态
//
// 状态流转:
//   IDLE → (写入 Pending/) → (submit) → Running/ → (轮询) → Completed/ 或 Failed/
//
// 锁机制: 操作 job 文件前通过 deny-write 锁文件 (.lock) 确保互斥,
//         参照 TaskProcess.h 中的 FopenDenyWriteLockUtf8 模式。
// ============================================================================

#include "GenTaskThread.h"
#include "GenHttpClient.h"
#include "Core/GenTaskAPI.h"       // GenTaskAPI::TriggerTaskComplete / TriggerTaskFailed
#include "Util/GenTaskProcess.h"
#include "Util/TaskProcess.h"
#include "Util/Settings.h"
#include "Core/Types.h"
#include "Core/File.h"             // File::FopenDenyWriteLockUtf8 / BoostPathFromUtf8
#include "Core/Logging.h"          // LOGI / LOGE / LOGW
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFile>                   // QFile::rename (job 文件在状态目录间移动)
#include <QDateTime>               // QDateTime::currentDateTime (SearchUnnormalRunningJob)
#include <QHostInfo>
#include <thread>
#include <chrono>

// extern 声明
extern bool bQuitingApplication;   // 由 CallEngine.cpp 定义, Node 关闭时置 true
extern std::string genPendingJobPath;    // 由 CallEngine::MakePath() 初始化
extern std::string genRunningJobPath;
extern std::string genCompletedJobPath;
extern std::string genFailedJobPath;
extern std::string genCancelledJobPath;

namespace AI3D {
namespace CORE {

// ============================================================================
// 文件工具函数 (文件作用域, 仅本 .cpp 可见)
//
// 注: job 文件的读写锁已由 GenJobFullInfo_s::save_with_retry / load_with_retry 封装
//     (内部使用 FopenDenyWriteLockUtf8, 与 TaskProcess.h 的 JobFeedBack_s 一致),
//     因此本文件无需手动管理锁, 直接调用 save_with_retry / load_with_retry 即可。
// ============================================================================

/// @brief 移动 job 文件到目标目录, 使用 QFile::rename
///        对标项目中使用 QFile 操作文件的惯例 (而非 std::filesystem::rename)
/// @param src    源文件完整路径
/// @param dstDir 目标目录路径
/// @return true=成功, false=失败
static bool MoveJobFile(const std::string& src, const std::string& dstDir) {
    QFileInfo fi(QString::fromStdString(src));
    QString dstPath = QString::fromStdString(dstDir) + "/" + fi.fileName();

    // 1. 尝试直接 rename (原子操作, 同盘高效)
    if (QFile::rename(QString::fromStdString(src), dstPath)) {
        return true;
    }

    // 2. rename 失败 (可能跨盘) → copy + remove 兜底
    LOGW("MoveJobFile: rename failed, trying copy+remove: " + src + " -> " + dstDir);
    if (QFile::copy(QString::fromStdString(src), dstPath)) {
        QFile::remove(QString::fromStdString(src));
        return true;
    }

    LOGE("MoveJobFile: both rename and copy+remove failed: " + src + " -> " + dstDir);
    return false;
}

static std::string BuildFeedbackPath(const GenJobFullInfo_s& info) {
    // feedback 放在项目目录下: project/BlockName/JF_<job_name>.bin (.json)
    // 对标 CallEngine.cpp 中 MAKE_FEEDBAK_BIN_FILE / MAKE_FEEDBAK_JSON_FILE 宏
    std::string base = info.job.project_path + "/" + info.job.block_item
                     + "/JF_" + info.job_name;
    if (JOB_FEEDBACK_USE_BIN) {
        return base + BINFILE_POSTFIX;   // ".bin"
    } else {
        return base + JSONFILE_POSTFIX;  // ".json"
    }
}

/// @brief 更新 JobFeedBack_s — 仅写标准字段 (Status/Percent/Msg/TaskRetVal)
///        结果数据 (result_url 等) 存在 GenJobInfo_s 自身中, 不写入 feedback
static void UpdateFeedback(const GenJobFullInfo_s& info) {
    JobFeedBack_s fb;
    std::string fbPath = BuildFeedbackPath(info);
    const GenJobInfo_s& job = info.job;

    // 先尝试加载已有 feedback (保留可能已存在的其他字段)
    fb.load_with_retry(fbPath, false);

    // GenTaskStatus → jobsta_e 映射
    switch (job.status) {
    case GenTaskStatus::IDLE:
    case GenTaskStatus::PENDING:
        fb.Status  = jobsta_e::STATUS_PENDDING;
        fb.Percent = 0.0f;
        break;
    case GenTaskStatus::IN_PROGRESS:
        fb.Status  = jobsta_e::STATUS_RUNNING;
        break;
    case GenTaskStatus::COMPLETED:
        fb.Status  = jobsta_e::STATUS_COMPLETE;
        fb.Percent = 100.0f;
        break;
    case GenTaskStatus::FAILED:
        fb.Status = jobsta_e::STATUS_FAILED;
        fb.Msg    = "generation task failed";
        break;
    case GenTaskStatus::CANCELLED:
        fb.Status = jobsta_e::STATUS_CANCELLED;
        break;
    }

    fb.save_with_retry(fbPath, false);
}

// ============================================================================
// 主循环 (在独立 std::thread 中运行, 对标 searchPendingJobThread2)
// ============================================================================

void GenTaskThread::Run()
{
    LOGI("GenTaskThread started");

    while (true) {
        if (bQuitingApplication)
            break;

        ProcessPendingJobs();     // 1. 扫描 Pending → submit → 移到 Running
        ProcessRunningJobs();    // 2. 扫描 Running → 轮询状态 → 移到 Completed/Failed
        SleepMs(2000);           // 2 秒轮询间隔
    }

    LOGI("GenTaskThread stopped");
}

// ============================================================================
// ProcessPendingJobs — 处理 Pending 目录中的新任务
//
// 流程:
//   1. 遍历 jobs_gen/Pending/*.json
//   2. deny-write 锁 → 加载 job → 检查崩溃恢复
//   3. 上传本地文件 (FILE_PATH → FILE_KEY)
//   4. HTTP POST submit → 回填 server_task_id → 保存 → 移到 Running/
//   5. submit 失败不移动文件, 下轮重试
// ============================================================================

void GenTaskThread::ProcessPendingJobs()
{
    QString pendingDir = QString::fromStdString(genPendingJobPath);
    QDirIterator it(pendingDir, {"J_*"}, QDir::Files);

    while (it.hasNext()) {
        it.next();
        QString filePath = it.filePath();
        std::string filePathStr = filePath.toStdString();

        // 1. 加载 job 文件 (load_with_retry 内部已处理 deny-write 锁)
        GenJobFullInfo_s info = GenJobFullInfo_s::load_with_retry(filePathStr);
        GenJobInfo_s& job = info.job;
        if (job.task_uuid.empty()) {
            LOGE("ProcessPendingJobs: invalid job file: " + filePathStr);
            continue;
        }

        // 2. 崩溃恢复: 已有 server_task_id 则直接移到 Running
        if (!job.server_task_id.empty()) {
            LOGI("Crash recovery: " + job.task_uuid + " already submitted, moving to Running");
            MoveJobFile(filePathStr, genRunningJobPath);
            continue;
        }

        // 3. 处理 AssetRef 上传 (FILE_PATH → FILE_KEY)
        bool uploadFailed = false;
        auto uploadAsset = [&](std::optional<AssetRef>& asset) {
            if (asset.has_value() && asset->kind == AssetKind::FILE_PATH) {
                std::string fileKey = GenHttpClient::UploadFile(asset->value);
                if (!fileKey.empty()) {
                    asset->kind  = AssetKind::FILE_KEY;
                    asset->value = fileKey;
                } else {
                    LOGW("UploadFile failed for: " + asset->value + ", will retry next round");
                    uploadFailed = true;
                }
            }
        };
        uploadAsset(job.params.image_file);
        uploadAsset(job.params.model_file);

        // 任一上传失败 → 保存参数变更 (已拿到的 file_key 不丢) → 下轮重试
        if (uploadFailed) {
            info.save_with_retry(filePathStr);
            continue;
        }

        // 4. 构造请求并 submit
        GenTaskRequest req;
        req.task_id      = job.task_uuid;
        req.engine_id    = job.engine_id;
        req.user_account = job.user_account;
        req.params       = job.params;

        GenTaskResponse resp = GenHttpClient::SubmitTask(req);

        // 5. 处理响应
        if (resp.status == GenTaskStatus::IDLE && resp.error_message.has_value()) {
            // 网络超时 → 不移动文件, 下轮重试
            LOGW("SubmitTask network timeout for: " + job.task_uuid);
            continue;
        }

        if (resp.status == GenTaskStatus::FAILED || resp.status == GenTaskStatus::CANCELLED) {
            // 服务端拒绝
            job.status = GenTaskStatus::FAILED;
            job.server_task_id = resp.triverse_task_uuid.value_or("");
            UpdateFeedback(info);
            info.save_with_retry(filePathStr);
            MoveJobFile(filePathStr, genFailedJobPath);
            continue;
        }

        // 6. 提交成功 → 回填 server_task_id → 写入 → 移到 Running
        if (resp.triverse_task_uuid.has_value()) {
            job.server_task_id = resp.triverse_task_uuid.value();
        }
        job.status = GenTaskStatus::PENDING;
        info.save_with_retry(filePathStr);
        UpdateFeedback(info);
        MoveJobFile(filePathStr, genRunningJobPath);

        LOGI("Submitted: " + job.task_uuid + " server_task_id=" + job.server_task_id);
    }
}

// ============================================================================
// ProcessRunningJobs — 轮询运行中的任务
//
// 流程:
//   1. 遍历 jobs_gen/Running/J_*
//   2. deny-write 锁 → 加载 job
//   3. HTTP GET query 服务端状态
//   4. 根据返回: COMPLETED → 移 Completed/, FAILED → 移 Failed/, IN_PROGRESS → 更新 feedback
//   5. 连续 5 次网络超时 → 标记 FAILED
// ============================================================================

void GenTaskThread::ProcessRunningJobs()
{
    QString runningDir = QString::fromStdString(genRunningJobPath);
    QDirIterator it(runningDir, {"J_*"}, QDir::Files);

    while (it.hasNext()) {
        it.next();
        QString filePath = it.filePath();
        std::string filePathStr = filePath.toStdString();

        // 1. 加载 job 文件 (load_with_retry 内部已处理 deny-write 锁)
        GenJobFullInfo_s info = GenJobFullInfo_s::load_with_retry(filePathStr);
        GenJobInfo_s& job = info.job;
        if (job.task_uuid.empty() || job.server_task_id.empty()) {
            continue;
        }

        // 2. 查询服务端状态
        GenTaskResponse resp = GenHttpClient::QueryTaskStatus(job.server_task_id);

        // 3. 网络超时 → 递增重试计数
        if (resp.status == GenTaskStatus::IDLE && resp.error_message.has_value()) {
            job.query_retry_count++;
            if (job.query_retry_count >= 5) {
                LOGE("Task " + job.task_uuid + " query failed 5 times, moving to Failed");
                job.status = GenTaskStatus::FAILED;
                info.save_with_retry(filePathStr);
                UpdateFeedback(info);
                MoveJobFile(filePathStr, genFailedJobPath);
                GenTaskAPI::TriggerTaskFailed(job.task_uuid, "连续 5 次轮询超时");
            } else {
                info.save_with_retry(filePathStr);
            }
            continue;
        }

        // 4. 重置重试计数 (成功获取到响应)
        job.query_retry_count = 0;

        switch (resp.status) {

        case GenTaskStatus::COMPLETED: {
            job.status = GenTaskStatus::COMPLETED;
            if (resp.result_url.has_value())
                job.result_url = resp.result_url.value();
            if (resp.preview_url.has_value())
                job.preview_url = resp.preview_url.value();
            job.cost_credits   = resp.cost_credits;
            job.points_balance = resp.points_balance;
            info.save_with_retry(filePathStr);
            UpdateFeedback(info);
            MoveJobFile(filePathStr, genCompletedJobPath);
            LOGI("Completed: " + job.task_uuid);
            GenTaskAPI::TriggerTaskComplete(job.task_uuid, job.result_url);
            break;
        }

        case GenTaskStatus::FAILED: {
            job.status = GenTaskStatus::FAILED;
            if (resp.error_message.has_value())
                job.error_message = resp.error_message.value();
            job.cost_credits   = resp.cost_credits;
            job.points_balance = resp.points_balance;
            info.save_with_retry(filePathStr);
            UpdateFeedback(info);
            MoveJobFile(filePathStr, genFailedJobPath);
            LOGE("Failed: " + job.task_uuid);
            GenTaskAPI::TriggerTaskFailed(job.task_uuid,
                resp.error_message.value_or("server returned failed"));
            break;
        }

        case GenTaskStatus::IN_PROGRESS: {
            // 进度更新: 保存状态 + 写 feedback
            job.status = GenTaskStatus::IN_PROGRESS;
            info.save_with_retry(filePathStr);
            UpdateFeedback(info);
            break;
        }

        case GenTaskStatus::CANCELLED: {
            // 服务端返回取消 (用户可能通过其他渠道取消)
            job.status = GenTaskStatus::CANCELLED;
            if (resp.error_message.has_value())
                job.error_message = resp.error_message.value();
            info.save_with_retry(filePathStr);
            UpdateFeedback(info);
            MoveJobFile(filePathStr, genCancelledJobPath);
            LOGI("Cancelled by server: " + job.task_uuid);
            break;
        }

        default:
            break;
        }
    }
}

void GenTaskThread::SleepMs(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// ============================================================================
// CancelGenTask — 取消生成式任务
//
// 流程:
//   1. 遍历 jobs_gen/ 下所有状态目录, 通过 task_uuid 找到 job 文件
//   2. HTTP POST cancel 到服务端 (如果有 server_task_id)
//   3. 更新状态为 CANCELLED → 保存 → 移到 Cancelled/
//   4. 更新 feedback
//
// 对标: CallEngine.cpp 中 searchCancelledRunningJob 的取消逻辑
//       (重建式通过 kill 子进程 + 移文件取消, 生成式通过 HTTP cancel + 移文件)
// ============================================================================

bool GenTaskThread::CancelGenTask(const std::string& task_uuid)
{
    QString genRoot = Settings::getGenEngineJobQueue();
    QStringList subDirs = {JOBPENDINGSTR, JOBRUNNINGSTR, JOBCOMPLETEDSTR,
                           JOBFAILEDSTR, JOBCANCELLEDSTR};

    for (const QString& sub : subDirs) {
        QDirIterator it(genRoot + "/" + sub, {"J_*"}, QDir::Files);
        while (it.hasNext()) {
            it.next();
            std::string filePathStr = it.filePath().toStdString();

            GenJobFullInfo_s info = GenJobFullInfo_s::load_with_retry(filePathStr);
            if (info.job.task_uuid != task_uuid)
                continue;

            GenJobInfo_s& job = info.job;

            // 已处于终态, 不再操作
            if (job.status == GenTaskStatus::COMPLETED ||
                job.status == GenTaskStatus::FAILED ||
                job.status == GenTaskStatus::CANCELLED) {
                LOGW("CancelGenTask: task " + task_uuid + " already in terminal state");
                return true;
            }

            // 1. HTTP cancel (有 server_task_id 才需要)
            if (!job.server_task_id.empty()) {
                GenHttpClient::CancelTask(job.server_task_id);
            }

            // 2. 更新状态 → 保存
            job.status = GenTaskStatus::CANCELLED;
            info.save_with_retry(filePathStr);
            UpdateFeedback(info);

            // 3. 移到 Cancelled/
            MoveJobFile(filePathStr, genCancelledJobPath);

            LOGI("Cancelled: " + task_uuid);
            return true;
        }
    }

    LOGW("CancelGenTask: task_uuid not found: " + task_uuid);
    return false;
}

}} // namespace AI3D::CORE
```

### 4.2 异常任务扫描线程

> 对标 `searchUnnormaldRunningJobThread`，处理崩溃/孤儿/卡死任务。

- [ ] `App/Engine/GenTaskThread.h`：增加声明
- [ ] `App/Engine/GenTaskThread.cpp`：增加实现
- [ ] `App/Engine/CallEngine.cpp` `main()`：启动线程

#### 为什么需要

现有系统有两个调度线程：
1. `searchPendingJobThread2` — 正常调度（Pending → Running → Completed/Failed）
2. `searchUnnormaldRunningJobThread` — 异常兜底（处理 Running 中卡死的任务）

生成式任务同样存在异常场景：

| 场景 | 后果 | 需要 |
|------|------|------|
| Node 在 HTTP submit **回包前**崩溃 | job 在 Pending/，但 `server_task_id` 已回填 → 重启后被 ProcessPendingJobs 的崩溃恢复路径处理 | 已有 |
| Node 在轮询期间崩溃 | job 在 Running/，重启后 ProcessRunningJobs 会继续轮询 | 已有 |
| 服务端任务被管理员删除/过期 | job 在 Running/ 永久轮询，永远拿不到终态 | **需兜底** |
| 网络长时间中断 (>5 次轮询) | ProcessRunningJobs 会标记 FAILED | 已有 |
| Running/ 中残留超过 24h 的任务 | 可能是早期 bug 遗留的孤儿文件 | **需兜底** |

#### 头文件声明

在 `GenTaskThread.h` 中增加：

```cpp
/// @brief 异常任务扫描线程入口 — 对标 searchUnnormaldRunningJobThread
///        扫描 Running/ 中长时间无进展的 job, 超时标记 FAILED 或重试
static void SearchUnnormalRunningJob();
```

#### 实现

在 `GenTaskThread.cpp` 中增加：

```cpp
// ============================================================================
// SearchUnnormalRunningJob — 扫描异常运行态任务 (对标 searchUnnormaldRunningJobThread)
//
// 独立线程, 每 30s 扫描一次 jobs_gen/Running/:
//   1. 遍历所有 job 文件
//   2. 检查 job 最后修改时间 (QFileInfo::lastModified)
//   3. 超过 24h 无更新 → 标记 FAILED 并移到 Failed/
//   4. 超过 1h 无更新 且 server_task_id 为空 → 移除 (submit 阶段残留)
//
// 与 ProcessRunningJobs 的分工:
//   ProcessRunningJobs  — 2s 间隔, 正常轮询服务端状态
//   SearchUnnormalRunningJob — 30s 间隔, 仅兜底清理长时间卡死的孤儿任务
// ============================================================================

void GenTaskThread::SearchUnnormalRunningJob()
{
    while (true) {
        if (bQuitingApplication)
            break;

        QString runningDir = QString::fromStdString(genRunningJobPath);
        QDirIterator it(runningDir, {"J_*"}, QDir::Files);
        QDateTime now = QDateTime::currentDateTime();

        while (it.hasNext()) {
            it.next();
            QFileInfo fi(it.filePath());
            std::string filePathStr = fi.absoluteFilePath().toStdString();

            qint64 secsSinceMod = fi.lastModified().secsTo(now);

            // 1. 超过 24h 未更新 → 标记失败
            if (secsSinceMod > 86400) {  // 24 * 3600
                GenJobFullInfo_s info = GenJobFullInfo_s::load_with_retry(filePathStr);
                if (info.job.task_uuid.empty()) {
                    // 损坏文件, 直接删除
                    QFile::remove(fi.absoluteFilePath());
                    continue;
                }

                LOGE("UnnormalRunning: " + info.job.task_uuid
                     + " stuck for " + std::to_string(secsSinceMod / 3600) + "h, moving to Failed");

                info.job.status = GenTaskStatus::FAILED;
                info.job.error_message = "task stuck in Running for over 24h";
                info.save_with_retry(filePathStr);
                UpdateFeedback(info);
                MoveJobFile(filePathStr, genFailedJobPath);
                GenTaskAPI::TriggerTaskFailed(info.job.task_uuid, info.job.error_message);
                continue;
            }

            // 2. 超过 1h 无更新 且无 server_task_id → submit 阶段残留, 移回 Pending 重试
            if (secsSinceMod > 3600) {
                GenJobFullInfo_s info = GenJobFullInfo_s::load_with_retry(filePathStr);
                if (info.job.server_task_id.empty() && !info.job.task_uuid.empty()) {
                    LOGW("UnnormalRunning: " + info.job.task_uuid
                         + " no server_task_id for 1h, moving back to Pending");
                    MoveJobFile(filePathStr, genPendingJobPath);
                }
            }
        }

        SleepMs(30000);  // 30s 扫描间隔
    }
}
```

#### 启动线程

在 `CallEngine.cpp` `main()` 中，GenTaskThread 启动之后增加：

```cpp
// ===== 新增: 生成式异常任务扫描线程 =====
std::thread searchUnnormalGenRunningJob(GenTaskThread::SearchUnnormalRunningJob);
searchUnnormalGenRunningJob.detach();
```

#### CMakeLists

无需修改 — 代码在 `GenTaskThread.cpp` 中，已被 `FILE(GLOB)` 自动拾取。

### 4.4 启动线程 + CMakeLists

- [ ] `App/Engine/CallEngine.cpp` `main()` — 在 `MakePath()` 调用之后增加：

```cpp
// ===== 新增: 初始化 HTTP 客户端 =====
std::string configpath = apppath + "/" + "MoldAIConfig.ini";
GenHttpClient::Init(configpath);

// ===== 新增: 生成式任务线程 =====
std::thread genTaskThread(GenTaskThread::Run);
genTaskThread.detach();

// ===== 新增: 生成式异常任务扫描线程 =====
std::thread searchUnnormalGenRunningJob(GenTaskThread::SearchUnnormalRunningJob);
searchUnnormalGenRunningJob.detach();
```

同时在 `CallEngine.cpp` 顶部 include：

```cpp
#include "GenHttpClient.h"
#include "GenTaskThread.h"
```

- [ ] **CMakeLists**: `GenTaskThread.h/cpp` + `GenHttpClient.h/cpp` 均在 `App/Engine/` 下，被 `FILE(GLOB)` 自动拾取，**无需修改 CMakeLists**。

### 4.5 并行验证

- [ ] 重建式任务（`searchPendingJobThread2`）和生成式任务（`GenTaskThread`）同时运行
- [ ] 确认两个线程互不干扰（操作不同目录）

---

## Phase 5: SDK 接口

> 依赖 Phase 4（线程已就绪，可直接往 Pending 写文件测试）。

### 5.1 新建 GenTaskAPI

- [ ] 创建 `Include/Core/GenTaskAPI.h`

```cpp
// Include/Core/GenTaskAPI.h
// ============================================================================
// @file    GenTaskAPI.h
// @brief   生成式任务前端 SDK — 编译进 MoldAIData.dll
//
// 提供生成式任务的提交 / 查询 / 下载接口, 供 GUI 前端调用。
//
// 依赖方向: GenTaskAPI (MoldAIData.dll) ← GenTaskThread (MoldAINode.exe)
//           GenTaskAPI 存储回调 → GenTaskThread 调用 TriggerXxx 触发回调
//           单向依赖, GenTaskAPI 不依赖 App/Engine/ 下的任何文件
//
// 上下文: 本文件在 Include/Core/ 下, 被 Src/Core/CMakeLists.txt 的
//         FILE(GLOB) 自动拾取, 编译进 MoldAIData.dll
// ============================================================================

#ifndef _AI3D_CORE_GEN_TASK_API_H_
#define _AI3D_CORE_GEN_TASK_API_H_

#include "Util/GenTaskProcess.h"
#include <string>
#include <functional>

// 前向声明 — BlockObject 在 Include/Core/BlockObject.h 中定义
namespace AI3D { namespace CORE { class BlockObject; } }

namespace AI3D {
namespace CORE {

/// @brief 生成式任务前端 SDK (所有方法均为静态)
class GenTaskAPI
{
public:
    // ========== 返回值结构 ==========

    /// @brief SubmitGenTask 返回值
    struct SubmitResult {
        std::string task_uuid;       // 生成的 UUID, 前端后续查询用
        std::string job_name;        // job 文件名 (BlockName + 时间戳)
        std::string feedback_path;   // feedback 文件路径 (前端可直接读取)
        bool        success = false; // 提交是否成功
        std::string error_msg;       // 失败原因
    };

    /// @brief QueryTaskStatus 返回值 (字段来源: GenJobInfo_s)
    struct TaskStatusResult {
        GenTaskStatus status = GenTaskStatus::IDLE;  // 当前状态
        int           progress = 0;                   // 进度 0-100 (来自 JobFeedBack_s.Percent)
        std::string   result_url;                     // 结果下载链接
        std::string   preview_url;                    // 预览图链接
        std::string   server_task_id;                 // 服务端任务ID
        std::string   error_message;                  // 错误详情
        int           cost_credits = 0;               // 花费积分
        int           points_balance = 0;             // 积分余额
    };

    // ========== 核心接口 (前端 SDK, 编译进 MoldAIData.dll) ==========

    /// @brief 提交生成式任务 — 构造 GenJobInfo_s 并写入 jobs_gen/Pending/
    static SubmitResult SubmitGenTask(
        const AI3D::CORE::BlockObject::Task_Info& blockInfo,
        const GenerationParams& params,
        const std::string& user_account);

    /// @brief 查询任务状态 — 先读 feedback 获取状态/进度, 终态时再读 GenJobInfo_s 获取结果详情
    static TaskStatusResult QueryTaskStatus(const std::string& task_uuid);

    /// @brief 下载结果文件 — HTTP GET result_url → 保存到本地
    static bool DownloadResult(const std::string& task_uuid,
                                const std::string& save_path);

    // ========== 回调 — 前端注册, GenTaskThread 触发 ==========

    /// @brief 任务完成回调 (task_uuid, result_url)
    using TaskCompleteCallback = std::function<void(
        const std::string& task_uuid,
        const std::string& result_url)>;

    /// @brief 任务失败回调 (task_uuid, error_message)
    using TaskFailedCallback = std::function<void(
        const std::string& task_uuid,
        const std::string& error)>;

    static void SetTaskCompleteCallback(TaskCompleteCallback cb);
    static void SetTaskFailedCallback(TaskFailedCallback cb);

    // 以下两个方法由 GenTaskThread 调用 (GenTaskAPI ← GenTaskThread, 单向依赖)
    static void TriggerTaskComplete(const std::string& task_uuid,
                                     const std::string& result_url);
    static void TriggerTaskFailed(const std::string& task_uuid,
                                   const std::string& error);

private:
    /// @brief 遍历 jobs_gen/ 下所有状态目录, 通过 task_uuid 找到 GenJobFullInfo_s
    static GenJobFullInfo_s FindGenJob(const std::string& task_uuid);

    // 回调存储 (文件级静态变量, 在 GenTaskAPI.cpp 中定义)
    static TaskCompleteCallback s_completeCallback;
    static TaskFailedCallback   s_failedCallback;
};

}} // namespace AI3D::CORE

#endif
```

### 5.2 创建 GenTaskAPI.cpp

```cpp
// Src/Core/GenTaskAPI.cpp
// ============================================================================
// @brief   生成式任务前端 SDK 实现
//          编译进 MoldAIData.dll, 不依赖 App/Engine/ 下的任何文件
//
// 依赖方向 (单向):
//   GenTaskAPI (MoldAIData.dll) ← GenTaskThread (MoldAINode.exe)
//   GenTaskAPI 定义回调接口 → GenTaskAPI.cpp 存储静态回调变量
//   → GenTaskThread 调用 TriggerXxx 触发
//
// 文件写入:
//   SubmitGenTask — 写 job JSON (GenJobInfo_s) 到 jobs_gen/Pending/ + 创建初始 feedback
//   QueryTaskStatus — 读 GenJobInfo_s (结果) + JobFeedBack_s (仅取 Percent)
//   DownloadResult — HTTP GET 下载结果文件到本地
// ============================================================================

#include "Core/GenTaskAPI.h"
#include "Core/BlockObject.h"
#include "Util/GenTaskProcess.h"
#include "Util/TaskProcess.h"
#include "Util/Settings.h"
#include "Core/Types.h"            // JOB_FEEDBACK_USE_BIN / BINFILE_POSTFIX / JSONFILE_POSTFIX
#include "Core/json.h"
#include "Core/Logging.h"          // LOGE / LOGW
#include <QUuid>                   // QUuid::createUuid (生成 task_uuid)
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFile>                   // QFile (DownloadResult 写文件)
#include <QHostInfo>               // QHostInfo::localHostName (engine_id)
#include <QDateTime>               // QDateTime::currentDateTime (job_name 时间戳)
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <fstream>

namespace AI3D {
namespace CORE {

// 回调静态存储 (GenTaskThread 完成后调用 TriggerXxx → 触发前端注册的回调)
GenTaskAPI::TaskCompleteCallback GenTaskAPI::s_completeCallback = nullptr;
GenTaskAPI::TaskFailedCallback   GenTaskAPI::s_failedCallback   = nullptr;

// ============================================================================
// 回调注册 — 前端调用, 设置回调函数指针
// ============================================================================

void GenTaskAPI::SetTaskCompleteCallback(TaskCompleteCallback cb) {
    s_completeCallback = std::move(cb);
}
void GenTaskAPI::SetTaskFailedCallback(TaskFailedCallback cb) {
    s_failedCallback = std::move(cb);
}

// ============================================================================
// 回调触发 — GenTaskThread 调用 (MoldAINode.exe → MoldAIData.dll)
// 检查回调是否已注册, 是则执行
// ============================================================================

void GenTaskAPI::TriggerTaskComplete(const std::string& task_uuid,
                                      const std::string& result_url) {
    if (s_completeCallback) {
        s_completeCallback(task_uuid, result_url);
    }
}
void GenTaskAPI::TriggerTaskFailed(const std::string& task_uuid,
                                    const std::string& error) {
    if (s_failedCallback) {
        s_failedCallback(task_uuid, error);
    }
}

// ============================================================================
// SubmitGenTask — 前端提交生成式任务
//
// 流程:
//   1. 校验 block_task_category == 1 (必须是生成式 Block)
//   2. 构造 GenJobInfo_s (task_uuid = QUuid, job_name = BlockName + 时间戳)
//   3. 创建结果目录 project/BlockName/job_name/
//   4. 写 job JSON 到 jobs_gen/Pending/
//   5. 创建初始 JobFeedBack_s
// ============================================================================

GenTaskAPI::SubmitResult GenTaskAPI::SubmitGenTask(
    const BlockObject::Task_Info& blockInfo,
    const GenerationParams& params,
    const std::string& user_account)
{
    SubmitResult result;

    // 1. 校验 Block 类型
    if (blockInfo.block_task_category != 1) {
        result.success   = false;
        result.error_msg = "Block does not support generative tasks (block_task_category != 1)";
        return result;
    }

    // 2. 构造 GenJobFullInfo_s (文件级结构体) + GenJobInfo_s (纯数据)
    GenJobFullInfo_s fullInfo;
    fullInfo.job_name = "J_" + blockInfo.blockName + "_"
                      + QDateTime::currentDateTime().toString("yyyyMMddhhmmss").toStdString();

    GenJobInfo_s& job = fullInfo.job;
    job.task_uuid    = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    job.engine_id    = QHostInfo::localHostName().toStdString();
    job.user_account = user_account;
    job.project_path = blockInfo.projectfile_;
    job.block_item   = blockInfo.blockName;
    job.category     = params.category;
    job.sub_type     = params.sub_type;
    job.params       = params;
    job.status       = GenTaskStatus::IDLE;

    // 3. 创建结果目录 (如果还不存在)
    QString resultDir = QString::fromStdString(blockInfo.projectfile_)
                      + "/" + QString::fromStdString(blockInfo.blockName)
                      + "/" + QString::fromStdString(fullInfo.job_name);
    QDir().mkpath(resultDir);

    // 4. 获取 jobs_gen/Pending/ 路径
    QString genRoot    = Settings::getGenEngineJobQueue();
    QString pendingPath = genRoot + "/" + JOBPENDINGSTR + "/";
    QDir().mkpath(pendingPath);

    // 5. 写入 job 文件 (GenJobFullInfo_s::save_with_retry → BIN/JSON 自动分发)
    std::string jobFilePath = pendingPath.toStdString() + fullInfo.job_name
                            + (JOB_INFO_USE_BIN ? ".bin" : ".json");
    if (!fullInfo.save_with_retry(jobFilePath)) {
        result.success   = false;
        result.error_msg = "Failed to write job file: " + jobFilePath;
        return result;
    }

    // 6. 创建初始 feedback (对标 CallEngine.cpp 中 MAKE_FEEDBAK_BIN/JSON_FILE 宏)
    std::string feedbackBase = blockInfo.projectfile_ + "/"
                             + blockInfo.blockName + "/JF_"
                             + fullInfo.job_name;
    std::string feedbackPath = feedbackBase
                             + (JOB_FEEDBACK_USE_BIN ? BINFILE_POSTFIX : JSONFILE_POSTFIX);

    JobFeedBack_s fb;
    fb.Status  = jobsta_e::STATUS_PENDDING;
    fb.Percent = 0.0f;
    fb.save_with_retry(feedbackPath, false);

    // 7. 返回
    result.success       = true;
    result.task_uuid     = job.task_uuid;
    result.job_name      = fullInfo.job_name;
    result.feedback_path = feedbackPath;
    return result;
}

// ============================================================================
// QueryTaskStatus — 前端查询任务状态
// 1. 先读 JobFeedBack_s 获取状态和进度 (标准 feedback, 固定字段)
// 2. 终态时 (COMPLETED/FAILED) 再读 GenJobInfo_s 获取结果详情
// ============================================================================

GenTaskAPI::TaskStatusResult GenTaskAPI::QueryTaskStatus(const std::string& task_uuid)
{
    TaskStatusResult result;

    // 1. 找到 GenJobFullInfo_s (用于构造 feedback 路径)
    GenJobFullInfo_s info = FindGenJob(task_uuid);
    GenJobInfo_s& job = info.job;
    if (job.task_uuid.empty()) {
        result.status        = GenTaskStatus::IDLE;
        result.error_message = "job not found for task_uuid: " + task_uuid;
        return result;
    }

    // 2. 先读 feedback — 获取状态 + 进度 (标准字段, 不动 JobFeedBack_s)
    std::string fbBase = job.project_path + "/" + job.block_item
                       + "/JF_" + info.job_name;
    std::string fbPath = fbBase
                       + (JOB_FEEDBACK_USE_BIN ? BINFILE_POSTFIX : JSONFILE_POSTFIX);
    JobFeedBack_s fb;
    if (!fb.load_with_retry(fbPath, false)) {
        result.status  = GenTaskStatus::IDLE;
        result.progress = 0;
        return result;
    }

    result.progress = static_cast<int>(fb.Percent);

    // 3. jobsta_e → GenTaskStatus 映射
    switch (fb.Status) {
    case jobsta_e::STATUS_PENDDING:   result.status = GenTaskStatus::PENDING;      break;
    case jobsta_e::STATUS_RUNNING:    result.status = GenTaskStatus::IN_PROGRESS;  break;
    case jobsta_e::STATUS_COMPLETE:   result.status = GenTaskStatus::COMPLETED;    break;
    case jobsta_e::STATUS_FAILED:     result.status = GenTaskStatus::FAILED;       break;
    case jobsta_e::STATUS_CANCELLED:  result.status = GenTaskStatus::CANCELLED;    break;
    default:                          result.status = GenTaskStatus::IDLE;         break;
    }

    // 4. 仅在终态时读 GenJobInfo_s 获取结果详情
    if (result.status == GenTaskStatus::COMPLETED) {
        result.result_url    = job.result_url;
        result.preview_url   = job.preview_url;
        result.cost_credits  = job.cost_credits;
        result.points_balance = job.points_balance;
    } else if (result.status == GenTaskStatus::FAILED) {
        result.error_message = job.error_message;
    }

    result.server_task_id = job.server_task_id;
    return result;
}

// ============================================================================
// DownloadResult — 下载任务结果文件
// 先 QueryTaskStatus 获取 result_url, 然后 HTTP GET 下载到 save_path
// ============================================================================

bool GenTaskAPI::DownloadResult(const std::string& task_uuid,
                                 const std::string& save_path)
{
    TaskStatusResult status = QueryTaskStatus(task_uuid);
    if (status.status != GenTaskStatus::COMPLETED) {
        LOGE("DownloadResult: task not completed, uuid=" + task_uuid);
        return false;
    }
    if (status.result_url.empty()) {
        LOGE("DownloadResult: result_url is empty, uuid=" + task_uuid);
        return false;
    }

    // 使用 Qt Network 同步下载
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(QString::fromStdString(status.result_url)));
    request.setTransferTimeout(30000);  // 下载超时 30s

    QNetworkReply* reply = manager.get(request);
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        reply->abort();
        loop.quit();
    });
    timer.start(30000);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        LOGE("DownloadResult: download failed: " + reply->errorString().toStdString());
        reply->deleteLater();
        return false;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QFile file(QString::fromStdString(save_path));
    if (!file.open(QIODevice::WriteOnly)) {
        LOGE("DownloadResult: cannot write to " + save_path);
        return false;
    }
    file.write(data);
    file.close();
    return true;
}

// ========== 内部辅助 ==========

/// @brief 遍历 jobs_gen/ 下所有状态目录, 通过 task_uuid 找到 GenJobFullInfo_s
static GenJobFullInfo_s FindGenJob(const std::string& task_uuid)
{
    QString genRoot = Settings::getGenEngineJobQueue();
    QStringList subDirs = {JOBPENDINGSTR, JOBRUNNINGSTR, JOBCOMPLETEDSTR,
                           JOBFAILEDSTR, JOBCANCELLEDSTR};

    for (const QString& sub : subDirs) {
        QDirIterator it(genRoot + "/" + sub, {"J_*"}, QDir::Files);
        while (it.hasNext()) {
            it.next();
            GenJobFullInfo_s info = GenJobFullInfo_s::load_with_retry(it.filePath().toStdString());
            if (info.job.task_uuid == task_uuid) {
                return info;
            }
        }
    }

    return {};
}

}} // namespace AI3D::CORE
```

### 5.3 GenTaskAPI 与 GenTaskThread 的调用关系

`GenTaskAPI.cpp` 在 `Src/Core/` 下，编译进 **MoldAIData.dll**。它不能反向依赖 `App/Engine/` 下的 `GenHttpClient` / `GenTaskThread`。

因此回调采用 **GenTaskAPI 存储 → GenTaskThread 调用** 的单向依赖：

```
GenTaskThread (App/Engine, MoldAINode.exe)
    │  #include "Core/GenTaskAPI.h"   ← GenTaskThread 可以看到 GenTaskAPI
    │  任务完成时调用:
    │    GenTaskAPI::TriggerTaskComplete(task_uuid, result_url)
    │    GenTaskAPI::TriggerTaskFailed(task_uuid, error)
    ▼
GenTaskAPI (Src/Core, MoldAIData.dll)
    │  存储回调指针 (静态变量)
    │  TriggerTaskComplete() 内部调用已注册的回调
    ▼
前端 (GUI 进程)
    │  SetTaskCompleteCallback(cb) 注册回调
```

`CancelGenTask` 实现在 `GenTaskThread` 中（详见 Phase 4.1），通过 HTTP cancel + 移文件到 Cancelled/ 完成取消。

### 5.4 线程安全

回调在 `GenTaskThread`（非 Qt 主线程）中执行。调用者如需更新 UI，自行处理：

```cpp
GenTaskAPI::SetTaskCompleteCallback([](const std::string& task_uuid, const std::string& url) {
    QMetaObject::invokeMethod(qApp, [=]() {
        // UI 更新...
    }, Qt::QueuedConnection);
});
```

### 5.5 CMakeLists — 无需修改

`GenTaskAPI.h` 在 `Include/Core/` 下，`GenTaskAPI.cpp` 在 `Src/Core/` 下，被 `FILE(GLOB)` 自动拾取。`Qt6::Network` 已在 `Src/Core/CMakeLists.txt` line 56 链接。

---

## Phase 6: 集成测试

> 所有实现完成后。

### 6.1 端到端测试

```cpp
// Test/GenTaskE2ETest.cpp
// 手动测试: 启动 Node, 然后运行此测试程序

#include "Core/GenTaskAPI.h"
#include "Core/BlockObject.h"
#include <QCoreApplication>
#include <QTimer>
#include <iostream>

using namespace AI3D::CORE;

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    // 1. 构造一个生成式 Block 的 Task_Info
    BlockObject::Task_Info blockInfo;
    blockInfo.block_task_category = 1;  // 生成式
    blockInfo.blockName           = "TestBlock_Gen";
    blockInfo.projectfile_        = "C:/Users/Test/AppData/Local/MoldAI/TestProject";

    // 2. 构造生成参数
    GenerationParams params;
    params.category    = GenTaskCategory::TEXT_TO_3D;
    params.sub_type    = GenTaskSubType::TEXT_TO_MODEL;
    params.prompt      = "a cute cat figurine";
    params.polygon_limit = 50000;
    params.texture_size  = 1024;

    // 3. 设置回调
    GenTaskAPI::SetTaskCompleteCallback([](const std::string& uuid, const std::string& url) {
        std::cout << "[COMPLETED] task_uuid=" << uuid << " result_url=" << url << std::endl;
    });
    GenTaskAPI::SetTaskFailedCallback([](const std::string& uuid, const std::string& err) {
        std::cout << "[FAILED] task_uuid=" << uuid << " error=" << err << std::endl;
    });

    // 4. 提交任务
    GenTaskAPI::SubmitResult result = GenTaskAPI::SubmitGenTask(
        blockInfo, params, "testuser@example.com");

    if (!result.success) {
        std::cerr << "Submit failed: " << result.error_msg << std::endl;
        return 1;
    }

    std::cout << "Submitted: " << result.task_uuid << std::endl;
    std::cout << "Job: " << result.job_name << std::endl;
    std::cout << "Feedback: " << result.feedback_path << std::endl;

    // 5. 轮询状态 (等待 Node 处理)
    QTimer pollTimer;
    int pollCount = 0;
    QObject::connect(&pollTimer, &QTimer::timeout, [&]() {
        pollCount++;
        auto status = GenTaskAPI::QueryTaskStatus(result.task_uuid);

        std::cout << "[" << pollCount << "] status="
                  << static_cast<int>(status.status)
                  << " progress=" << status.progress << "%";

        if (!status.result_url.empty())
            std::cout << " url=" << status.result_url;
        std::cout << std::endl;

        if (status.status == GenTaskStatus::COMPLETED) {
            // 6. 下载结果
            std::string savePath = result.feedback_path.substr(
                0, result.feedback_path.rfind('/')) + "/result.glb";
            if (GenTaskAPI::DownloadResult(result.task_uuid, savePath)) {
                std::cout << "Downloaded to: " << savePath << std::endl;
            }
            pollTimer.stop();
            app.quit();
        }

        if (status.status == GenTaskStatus::FAILED || pollCount > 300) {
            pollTimer.stop();
            app.quit();
        }
    });
    pollTimer.start(2000);  // 每 2s 轮询

    return app.exec();
}
```

- [ ] **验证点**: 前端 `SubmitGenTask` → 文件出现在 `jobs_gen/Pending/`
- [ ] **验证点**: GenTaskThread 读取 → HTTP submit → 文件移到 `Running/`
- [ ] **验证点**: 轮询完成 → 文件移到 `Completed/` → feedback 更新为 COMPLETED
- [ ] **验证点**: 前端通过 SDK 读取 feedback 并下载结果文件

### 6.2 崩溃恢复测试

```cpp
// Test/GenTaskCrashRecoveryTest.cpp
// 手动测试步骤:
//
// Step 1: 提交一个任务, 拿到 task_uuid
// Step 2: 观察 jobs_gen/Pending/ 确认 job 文件存在
// Step 3: 等待 GenTaskThread 的 HTTP submit 返回 (日志: "Submitted: xxx server_task_id=...")
// Step 4: 在 job 文件被移动到 Running 之前 (2s 窗口), 立即 kill Node 进程
// Step 5: 检查 Pending 中的 job 文件内容 — server_task_id 应该已被回填 (非空)
// Step 6: 重启 Node
// Step 7: 观察 GenTaskThread 日志:
//         "Crash recovery: xxx already submitted, moving to Running"
// Step 8: 确认服务端没有收到重复的 submit 请求
// Step 9: 确认任务最终正常完成

void SimulateCrashRecoveryTest()
{
    // 此测试需要配合服务端 mock, 验证逻辑:
    //
    // 1. GenTaskThread::ProcessPendingJobs() 中:
    //    if (!job.server_task_id.empty()) {  ← 崩溃恢复路径
    //        MoveJobFile(filePathStr, genRunningJobPath);
    //        continue;  ← 跳过 HTTP submit
    //    }
    //
    // 2. 服务端 mock 应该记录收到的 submit 请求次数:
    //    正常流程: 1 次
    //    崩溃恢复: 仍为 1 次 (不增加)
}
```

- [ ] **验证点**: crash 前 `server_task_id` 已回填
- [ ] **验证点**: Node 重启后识别已有 `server_task_id`，直接移到 Running，不重复 submit
- [ ] **验证点**: 服务端仅收到 1 次 submit 请求

### 6.3 网络异常测试

```cpp
// Test/GenTaskNetworkFailTest.cpp
// 手动测试步骤:
//
// 测试 A: submit 阶段超时
// Step 1: 关闭 Mock Server 或断网
// Step 2: 提交任务到 Pending
// Step 3: 观察 GenTaskThread 日志: "SubmitTask network timeout for: xxx"
// Step 4: 确认 job 文件仍停留在 Pending/ (没有移到 Failed)
// Step 5: 恢复网络 / 启动 Mock Server
// Step 6: 下一轮轮询中, 任务被正常 submit 并移到 Running
//
// 测试 B: query 阶段累计超时
// Step 1: submit 成功, job 在 Running/
// Step 2: 断网
// Step 3: 观察每一轮轮询: query_retry_count 从 0 递增到 5
// Step 4: 第 5 次超时后: 任务移到 Failed/
// Step 5: 确认 feedback 中 error_message = "连续 5 次轮询超时"
// Step 6: 确认 TaskFailedCallback 被触发
//
// 测试 C: 网络间歇恢复
// Step 1: 查询超时 2 次后恢复网络
// Step 2: 第 3 次查询成功, query_retry_count 重置为 0
// Step 3: 确认任务正常流转到 Completed

void SimulateNetworkFailureTest()
{
    // 对应代码路径:
    //
    // GenTaskThread::ProcessRunningJobs():
    //   if (resp.status == GenTaskStatus::IDLE && resp.error_message.has_value()) {
    //       job.query_retry_count++;          ← 超时计数
    //       if (job.query_retry_count >= 5) { ← 阈值判断
    //           MoveFile(..., genFailedJobPath);
    //           s_failedCallback(...);
    //       }
    //   }
    //   job.query_retry_count = 0;  ← 恢复后重置
}
```

- [ ] **验证点**: submit 超时不移动文件，网络恢复后自动重试
- [ ] **验证点**: query 连续 5 次超时 → Failed
- [ ] **验证点**: query 中途恢复 → 计数重置，任务继续

### 6.4 兼容性测试

```cpp
// Test/GenTaskCompatTest.cpp
// 验证新旧两套任务系统并行运行时互不干扰

void CompatibilityTest()
{
    // Step 1: 同时提交 3 个重建式任务 + 3 个生成式任务
    //
    // 重建式: Block_normal → jobs/Pending/
    // 生成式: Block_gen    → jobs_gen/Pending/
    //
    // Step 2: 确认两个线程各自工作:
    //   searchPendingJobThread2  → 只扫描 jobs/
    //   GenTaskThread            → 只扫描 jobs_gen/
    //
    // Step 3: 分别检查 jobs/ 和 jobs_gen/ 的状态流转是否正常
    //
    // Step 4: 确认重建式任务的行为与改动前完全一致
    //   - feedback 文件格式不变
    //   - 新增字段对重建式保持默认值
    //   - BIN 格式读写不受影响
    //
    // Step 5: 检查旧 Block (block_task_category == 0) 的序列化:
    //   - WriteBlockInfoToJson → block_task_category: 0
    //   - ReadBlockInfoJson    → block_task_category 正确读取为 0
    //   - 旧 .blk 文件 (没有 block_task_category 字段) → 读取后默认为 0
}
```

- [ ] **验证点**: 两个线程操作不同目录，互不干扰
- [ ] **验证点**: 重建式任务行为完全不变
- [ ] **验证点**: 旧 Block 文件（无 `block_task_category` 字段）读取后默认值为 0

### 6.5 Mock Server(快速搭建)

```python
# Test/mock_triverse_server.py
# 用于本地测试的最小化 Mock Server
#
# 启动: pip install flask && python mock_triverse_server.py

from flask import Flask, request, jsonify
import uuid
import time
import threading

app = Flask(__name__)

# 内存存储
tasks = {}

@app.route('/api/v1/task/submit', methods=['POST'])
def submit():
    data = request.get_json()
    client_task_id = data.get('task_id', '')

    # 幂等: 相同 task_id 返回已有 server_task_id
    for triverse_id, task in tasks.items():
        if task.get('client_task_id') == client_task_id:
            return jsonify({
                'task_id': client_task_id,
                'triverse_task_uuid': triverse_id,
                'status': task['status'],
                'progress': task['progress'],
            })

    # 新建任务
    triverse_id = 'trv-' + uuid.uuid4().hex[:12]
    tasks[triverse_id] = {
        'client_task_id': client_task_id,
        'status': 'in_progress',
        'progress': 0,
    }

    # 模拟异步处理: 10s 后自动完成
    def auto_complete():
        time.sleep(10)
        tasks[triverse_id]['status'] = 'completed'
        tasks[triverse_id]['progress'] = 100
        tasks[triverse_id]['result_url'] = f'http://localhost:5000/api/v1/download/{triverse_id}'

    threading.Thread(target=auto_complete, daemon=True).start()

    return jsonify({
        'task_id': client_task_id,
        'triverse_task_uuid': triverse_id,
        'status': 'in_progress',
        'progress': 0,
    })

@app.route('/api/v1/task/status', methods=['GET'])
def status():
    task_id = request.args.get('task_id', '')
    task = tasks.get(task_id)
    if not task:
        return jsonify({'task_id': task_id, 'status': 'failed', 'error_message': 'not found'})
    return jsonify({
        'task_id': task['client_task_id'],
        'triverse_task_uuid': task_id,
        'status': task['status'],
        'progress': task['progress'],
        'result_url': task.get('result_url'),
    })

@app.route('/api/v1/task/cancel', methods=['POST'])
def cancel():
    data = request.get_json()
    task_id = data.get('task_id', '')
    if task_id in tasks:
        tasks[task_id]['status'] = 'cancelled'
    return jsonify({'status': 'cancelled'})

@app.route('/api/v1/upload', methods=['POST'])
def upload():
    file = request.files.get('file')
    if file:
        file_key = 'fk-' + uuid.uuid4().hex[:8]
        return jsonify({'file_key': file_key, 'filename': file.filename})
    return jsonify({'error': 'no file'}), 400

@app.route('/api/v1/download/<task_id>')
def download(task_id):
    # 返回一个占位 glb 文件
    return b'glTF mock binary', 200, {'Content-Type': 'model/gltf-binary'}

if __name__ == '__main__':
    app.run(port=5000, debug=False)
```

### 6.6 测试配置

在 `MoldAIConfig.ini` 中加入:

```ini
[GenTask]
ServerUrl=http://localhost:5000
ApiPrefix=/api/v1
; accessToken 从注册表读取 (登录后自动写入, 与现有 HttpClient 共用)
; Mock Server 不需要鉴权 → 不登录即可 (accessToken 为空, 签名不含 token)
Timeout=5000
MaxRetries=3
```

测试时 Node 启动后会自动读取此配置（需在 `main()` 中调用 `GenHttpClient::Init(configpath)`）。

---

## 文件改动汇总

### 新建（7 个，除 GenTaskProcess.h 外均被 GLOB 自动拾取）

| 文件 | 位置 | CMake | 说明 |
|------|------|-------|------|
| `Include/Util/GenTaskProcess.h` | Util/ | 需显式添加 | 对标 TaskProcess.h，`GenJobInfo_s` + 枚举 + API 类型 |
| `App/Engine/GenHttpClient.h` | Engine/ | GLOB 自动拾取 | 同步 HTTP 客户端 |
| `App/Engine/GenHttpClient.cpp` | Engine/ | GLOB 自动拾取 | |
| `App/Engine/GenTaskThread.h` | Engine/ | GLOB 自动拾取 | GenTaskThread 调度线程 |
| `App/Engine/GenTaskThread.cpp` | Engine/ | GLOB 自动拾取 | |
| `Include/Core/GenTaskAPI.h` | Core/ | GLOB 自动拾取 | 前端 SDK 接口 |
| `Src/Core/GenTaskAPI.cpp` | Core/ | GLOB 自动拾取 | 编译进 MoldAIData.dll |

### 修改（8 个）

| 文件 | Phase | 改动 |
|------|-------|------|
| `Include/Core/DataStruct.h` | P1 | 新增 `GenJobInfoData`/`GenJobFile`；`JobInfoData` 增加 `task_category`；`BLKBinFile` 增加 `block_task_category` |
| `Include/Core/BlockObject.h` | P1 | `Task_Info` 增加 `block_task_category` |
| `Include/Core/BlockObject.cpp` | P1 | 4 个序列化方法增加 `block_task_category` |
| `Include/Util/TaskProcess.h` | P1 | `JobInfo_s` 加 `task_category`；`JobFullInfo_s::WriteToBin/LoadFromBin` 加 `task_category` 拷贝 |
| `Include/Util/Settings.h` | P2 | 增加 `getGenEngineJobQueue()` |
| `Src/Util/Settings.cpp` | P2 | 实现 `getGenEngineJobQueue()` |
| `App/Engine/CallEngine.cpp` | P2+P4 | MakePath 创建 `jobs_gen/` 目录; `doCleanupJobLockOnceWhileEngineStart` 清理 `jobs_gen/` 残留锁; main 启动 GenTaskThread |
| `App/Engine/CMakeLists.txt` | P1 | HEADER_LIST 显式添加 `GenTaskProcess.h` (唯一需改的 CMakeLists) |

### 不动

`JobFeedBack_s` 不扩展 — 生成式复用 Status/Percent/Msg/TaskRetVal 做进度反馈，结果数据存 GenJobInfo_s。`TaskGraph_s`、`Task_s`、`ATTaskInfo`、`ExecTaskFileV2`、`GetPendingJob`、`Src/Core/CMakeLists.txt` — 全部不动。
