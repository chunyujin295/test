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
| `GenTaskProcess.h` | `Util/TaskProcess.h` | 和 `TaskProcess.h` 一样定义 Job 调度结构体。`TaskProcess.h` 是重建式（`JobInfo_s`、`JobFeedBack_s`），`GenTaskProcess.h` 是生成式（`GenJobInfo` + 枚举 + API 类型） |
| `GenJobInfo` | `JobInfo_s` | Job 文件读写结构体。`JobInfo_s` 存 `ProjectPath/ItemPath`，`GenJobInfo` 存 `task_uuid/GenerationParams/server_task_id/result_url` |
| `GenTaskRequest` | 无现成对标 | HTTP submit 请求体，序列化为 JSON POST 到服务端。重建式没有远程 API，所以这是全新概念 |
| `GenTaskResponse` | 无现成对标 | HTTP 响应体。服务端返回的 task status/progress/result_url |
| `GenerationParams` | `ATOptions`（局部类似） | 用户填写的生成参数（prompt、模型版本、面数等）。概念上类似 `ATOptions`（空三参数），但结构完全不同 |
| `GenTaskCategory/SubType/Status` | `jobsta_e`（局部类似） | 生成式任务的状态枚举。`jobsta_e` 是重建式 job 状态（PENDING/RUNNING/COMPLETE...），`GenTaskStatus` 多了 IDLE + 服务端状态 |
| `block_task_category` (Task_Info 新增) | 无，新增字段 | 在已有的 `BlockObject::Task_Info` 中增加的 int 字段，区分 Block 是重建式(0)还是生成式(1)。类似 `type_` 字段的作用 |
| `JobFeedBack_s` 新增 6 字段 | 无，字段扩展 | 在已有的 `JobFeedBack_s` 尾部追加 `result_url/preview_url/server_task_id/error_message/cost_credits/points_balance`，重建式不写这些字段 |

### 调度与通信层

| 新增 | 对标现有 | 说明 |
|------|---------|------|
| `GenTaskThread` | `searchPendingJobThread2` (CallEngine.cpp) | 调度线程。`searchPendingJobThread2` 遍历 `jobs/` 调度重建式任务，`GenTaskThread::Run()` 遍历 `jobs_gen/` 调度生成式任务。两者独立运行，互不干扰 |
| `GenHttpClient` | `spawn Task.exe` 子进程 | 任务执行方式。重建式通过文件 IPC + spawn `MoldAITask.exe` 子进程执行，生成式通过 HTTP POST/GET 提交和轮询远程服务端。都是"提交任务→等待结果"的模式 |
| `jobs_gen/` 目录 | `jobs/` 目录 | 文件系统 IPC 的工作目录。`jobs/` 存重建式 Job 文件（BIN+JSON），`jobs_gen/` 存生成式 Job 文件（仅 JSON） |
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
| `BlockInfo.h` | 自身 | `BlockFile` 加 `block_task_category`（BIN 格式兼容） |
| `TaskProcess.h` | 自身 | `JobInfo_s` 加 `task_category`；`JobFeedBack_s` 加 6 个生成式扩展字段，更新 10+ 个序列化方法 |
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
// 位置说明: 放在 Util/ 而非 Core/ 是因为 TaskProcess.h (JobInfo_s, JobFeedBack_s)
// 也在 Util/ 下。GenJobInfo 本质上和 JobInfo_s 是同层级的 job 调度结构体。
// json 使用项目内 "Core/json.h" (vendored nlohmann 3.7.2), 与 TaskProcess.h 一致。

#ifndef _AI3D_UTIL_GEN_TASK_PROCESS_H_
#define _AI3D_UTIL_GEN_TASK_PROCESS_H_

#include <string>
#include <vector>
#include <optional>
#include "Core/json.h"       // 项目内 vendored nlohmann 3.7.2, 与 TaskProcess.h 一致

namespace AI3D {
namespace CORE {

// ========== 枚举 ==========

enum class GenTaskCategory {
    TEXT_TO_3D,
    IMAGE_TO_3D,
    TEXTURING,
    UTILITY,
    IMAGE_GENERATION,
};

enum class GenTaskSubType {
    TEXT_TO_MODEL,
    TEXT_TO_MESH,
    IMAGE_TO_MODEL,
    IMAGE_TO_MESH,
    TEXTURE_MODEL,
    TEXT_TO_TEXTURE,
    MODEL_PREVIEW_RENDER,
    MODEL_REMESH,
    CONVERT_MODEL_FORMAT,
    IMAGE_GENERATION,
};

enum class GenTaskStatus {
    IDLE,
    PENDING,
    IN_PROGRESS,
    COMPLETED,
    FAILED,
    CANCELLED,
};

enum class AssetKind {
    NONE,
    FILE_PATH,
    FILE_KEY,
    URL,
};

// ========== 枚举 ↔ 字符串 转换 (nlohmann 3.7.2 无 NLOHMANN_JSON_SERIALIZE_ENUM) ==========

inline std::string to_string(GenTaskCategory v) {
    switch (v) {
    case GenTaskCategory::TEXT_TO_3D:       return "text_to_3d";
    case GenTaskCategory::IMAGE_TO_3D:      return "image_to_3d";
    case GenTaskCategory::TEXTURING:        return "texturing";
    case GenTaskCategory::UTILITY:          return "utility";
    case GenTaskCategory::IMAGE_GENERATION: return "image_generation";
    }
    return "unknown";
}
inline GenTaskCategory to_GenTaskCategory(const std::string& s) {
    if (s == "text_to_3d")       return GenTaskCategory::TEXT_TO_3D;
    if (s == "image_to_3d")      return GenTaskCategory::IMAGE_TO_3D;
    if (s == "texturing")        return GenTaskCategory::TEXTURING;
    if (s == "utility")          return GenTaskCategory::UTILITY;
    if (s == "image_generation") return GenTaskCategory::IMAGE_GENERATION;
    return GenTaskCategory::TEXT_TO_3D;
}

inline std::string to_string(GenTaskSubType v) {
    switch (v) {
    case GenTaskSubType::TEXT_TO_MODEL:         return "text_to_model";
    case GenTaskSubType::TEXT_TO_MESH:          return "text_to_mesh";
    case GenTaskSubType::IMAGE_TO_MODEL:        return "image_to_model";
    case GenTaskSubType::IMAGE_TO_MESH:         return "image_to_mesh";
    case GenTaskSubType::TEXTURE_MODEL:         return "texture_model";
    case GenTaskSubType::TEXT_TO_TEXTURE:       return "text_to_texture";
    case GenTaskSubType::MODEL_PREVIEW_RENDER:  return "model_preview_render";
    case GenTaskSubType::MODEL_REMESH:          return "model_remesh";
    case GenTaskSubType::CONVERT_MODEL_FORMAT:  return "convert_model_format";
    case GenTaskSubType::IMAGE_GENERATION:      return "image_generation";
    }
    return "unknown";
}
inline GenTaskSubType to_GenTaskSubType(const std::string& s) {
    if (s == "text_to_model")         return GenTaskSubType::TEXT_TO_MODEL;
    if (s == "text_to_mesh")          return GenTaskSubType::TEXT_TO_MESH;
    if (s == "image_to_model")        return GenTaskSubType::IMAGE_TO_MODEL;
    if (s == "image_to_mesh")         return GenTaskSubType::IMAGE_TO_MESH;
    if (s == "texture_model")         return GenTaskSubType::TEXTURE_MODEL;
    if (s == "text_to_texture")       return GenTaskSubType::TEXT_TO_TEXTURE;
    if (s == "model_preview_render")  return GenTaskSubType::MODEL_PREVIEW_RENDER;
    if (s == "model_remesh")          return GenTaskSubType::MODEL_REMESH;
    if (s == "convert_model_format")  return GenTaskSubType::CONVERT_MODEL_FORMAT;
    if (s == "image_generation")      return GenTaskSubType::IMAGE_GENERATION;
    return GenTaskSubType::TEXT_TO_MODEL;
}

inline std::string to_string(GenTaskStatus v) {
    switch (v) {
    case GenTaskStatus::IDLE:        return "idle";
    case GenTaskStatus::PENDING:     return "pending";
    case GenTaskStatus::IN_PROGRESS: return "in_progress";
    case GenTaskStatus::COMPLETED:   return "completed";
    case GenTaskStatus::FAILED:      return "failed";
    case GenTaskStatus::CANCELLED:   return "cancelled";
    }
    return "unknown";
}
inline GenTaskStatus to_GenTaskStatus(const std::string& s) {
    if (s == "idle")        return GenTaskStatus::IDLE;
    if (s == "pending")     return GenTaskStatus::PENDING;
    if (s == "in_progress") return GenTaskStatus::IN_PROGRESS;
    if (s == "completed")   return GenTaskStatus::COMPLETED;
    if (s == "failed")      return GenTaskStatus::FAILED;
    if (s == "cancelled")   return GenTaskStatus::CANCELLED;
    return GenTaskStatus::IDLE;
}

inline std::string to_string(AssetKind v) {
    switch (v) {
    case AssetKind::NONE:      return "none";
    case AssetKind::FILE_PATH: return "file_path";
    case AssetKind::FILE_KEY:  return "file_key";
    case AssetKind::URL:       return "url";
    }
    return "none";
}
inline AssetKind to_AssetKind(const std::string& s) {
    if (s == "file_path") return AssetKind::FILE_PATH;
    if (s == "file_key")  return AssetKind::FILE_KEY;
    if (s == "url")       return AssetKind::URL;
    return AssetKind::NONE;
}

// ========== 结构体 + 手动 to_json / from_json ==========

struct AssetRef {
    AssetKind   kind = AssetKind::NONE;
    std::string value;
    std::string content_type;
};
inline void to_json(nlohmann::json& j, const AssetRef& v) {
    j["kind"]         = to_string(v.kind);
    j["value"]        = v.value;
    j["content_type"] = v.content_type;
}
inline void from_json(const nlohmann::json& j, AssetRef& v) {
    if (j.contains("kind"))         v.kind         = to_AssetKind(j.at("kind").get<std::string>());
    if (j.contains("value"))        v.value        = j.at("value").get<std::string>();
    if (j.contains("content_type")) v.content_type = j.value("content_type", "");
}

struct GenerationParams {
    GenTaskCategory category = GenTaskCategory::TEXT_TO_3D;
    GenTaskSubType  sub_type = GenTaskSubType::TEXT_TO_MODEL;

    std::optional<std::string> prompt;
    std::optional<std::string> negative_prompt;
    std::optional<std::string> style;

    std::optional<AssetRef> image_file;
    std::optional<AssetRef> model_file;

    std::optional<std::string> model_version;
    std::optional<int>         polygon_limit;
    std::optional<int>         texture_size;

    std::optional<std::string>              render_mode;
    std::optional<int>                      image_count;
    std::optional<std::vector<std::string>> camera_angles;
    std::optional<std::string>              preset_name;

    std::optional<int>         target_poly_count;
    std::optional<std::string> output_format;
};
inline void to_json(nlohmann::json& j, const GenerationParams& v) {
    j["category"] = to_string(v.category);
    j["sub_type"] = to_string(v.sub_type);
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
    v.category = to_GenTaskCategory(j.value("category", "text_to_3d"));
    v.sub_type = to_GenTaskSubType(j.value("sub_type", "text_to_model"));
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

struct GenTaskRequest {
    std::string     task_id;
    std::string     engine_id;
    std::string     user_account;
    GenerationParams params;
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

struct GenTaskResponse {
    std::string                task_id;
    std::optional<std::string> triverse_task_uuid;
    GenTaskStatus              status = GenTaskStatus::IDLE;
    int                        progress = 0;
    std::optional<std::string> result_url;
    std::optional<std::string> preview_url;
    std::optional<std::string> error_message;
    int                        cost_credits = 0;
    int                        points_balance = 0;
};
inline void to_json(nlohmann::json& j, const GenTaskResponse& v) {
    j["task_id"]   = v.task_id;
    j["status"]    = to_string(v.status);
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
    v.status    = to_GenTaskStatus(j.value("status", "idle"));
    if (j.contains("triverse_task_uuid")) v.triverse_task_uuid = j.at("triverse_task_uuid").get<std::string>();
    if (j.contains("result_url"))         v.result_url         = j.at("result_url").get<std::string>();
    if (j.contains("preview_url"))        v.preview_url        = j.at("preview_url").get<std::string>();
    if (j.contains("error_message"))      v.error_message      = j.at("error_message").get<std::string>();
    v.cost_credits   = j.value("cost_credits", 0);
    v.points_balance = j.value("points_balance", 0);
}

// ========== GenJobInfo — job 文件顶层结构 ==========

struct GenJobInfo {
    std::string task_uuid;
    std::string job_name;
    std::string engine_id;
    std::string user_account;

    std::string project_path;
    std::string block_item;

    GenTaskCategory category = GenTaskCategory::TEXT_TO_3D;
    GenTaskSubType  sub_type = GenTaskSubType::TEXT_TO_MODEL;
    GenerationParams params;

    GenTaskStatus status = GenTaskStatus::IDLE;
    std::string server_task_id;
    std::string result_url;
    int query_retry_count = 0;

    // 保存/加载 (内联实现, 参照 JobInfo_s::WriteToJson 的模式, 无需 .cpp)
    bool save(const std::string& filePath) const {
        try {
            nlohmann::json j = *this;
            std::ofstream ofs = File::OpenOfstreamUtf8(filePath, std::ios::out);
            if (!ofs.good()) return false;
            ofs << j.dump(4);
            ofs.close();
            return true;
        } catch (std::exception& ex) {
            LOGE(std::string("GenJobInfo::save failed: ") + ex.what());
            return false;
        }
    }

    static GenJobInfo load(const std::string& filePath) {
        std::ifstream ifs = File::OpenIfstreamUtf8(filePath, std::ios::in);
        if (!ifs.is_open()) return {};
        std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();
        if (str.empty()) return {};
        try {
            return nlohmann::json::parse(str).get<GenJobInfo>();
        } catch (std::exception& ex) {
            LOGE(std::string("GenJobInfo::load failed: ") + ex.what());
            return {};
        }
    }
};

inline void to_json(nlohmann::json& j, const GenJobInfo& v) {
    j["task_uuid"]    = v.task_uuid;
    j["job_name"]     = v.job_name;
    j["engine_id"]    = v.engine_id;
    j["user_account"] = v.user_account;
    j["project_path"] = v.project_path;
    j["block_item"]   = v.block_item;
    j["category"]     = to_string(v.category);
    j["sub_type"]     = to_string(v.sub_type);
    j["params"]       = v.params;
    j["status"]       = to_string(v.status);
    if (!v.server_task_id.empty()) j["server_task_id"] = v.server_task_id;
    if (!v.result_url.empty())     j["result_url"]     = v.result_url;
    j["query_retry_count"] = v.query_retry_count;
}

inline void from_json(const nlohmann::json& j, GenJobInfo& v) {
    j.at("task_uuid").get_to(v.task_uuid);
    j.at("job_name").get_to(v.job_name);
    j.at("engine_id").get_to(v.engine_id);
    j.at("user_account").get_to(v.user_account);
    j.at("project_path").get_to(v.project_path);
    j.at("block_item").get_to(v.block_item);
    v.category = to_GenTaskCategory(j.at("category").get<std::string>());
    v.sub_type = to_GenTaskSubType(j.at("sub_type").get<std::string>());
    j.at("params").get_to(v.params);
    v.status = to_GenTaskStatus(j.value("status", "idle"));
    v.server_task_id    = j.value("server_task_id", "");
    v.result_url        = j.value("result_url", "");
    v.query_retry_count = j.value("query_retry_count", 0);
}

} // namespace CORE
} // namespace AI3D

#endif // _AI3D_UTIL_GEN_TASK_PROCESS_H_
```

### 1.2 修改 BlockObject — Task_Info 扩展

- [ ] `Include/Core/BlockObject.h`：`Task_Info` 增加 `int block_task_category = 0`
- [ ] `Include/Core/BlockObject.cpp`：4 个序列化方法增加新字段读写

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
engineFile.block_task_category = block_info_.block_task_category;
```

同时需要修改 `BlockInfo.h` 中的 `BlockFile` 结构体，增加字段：

```cpp
// Include/Core/BlockInfo.h — BlockFile 结构体中增加:
int block_task_category = 0;
```

`ReadBlockInfoBin()` 中，读取处增加：

```cpp
block_info_.block_task_category = engineFile.block_task_category;
```

#### BlockInfo.h 中 BlockFile 的序列化更新

在 `BlockFile::Serialize` 中增加：

```cpp
ar & block_task_category;
```

（cereal 自动处理，只需在 BlockFile 结构体中增加字段声明即可）

### 1.3 修改 JobInfo_s

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

### 1.4 修改 JobFeedBack_s

- [ ] `Include/Util/TaskProcess.h`：`JobFeedBack_s` 增加以下字段

在 `JobFeedBack_s` 结构体中（TaskRetVal 之后）增加：

```cpp
// ===== 新增: 生成式任务扩展字段 =====
std::string result_url;       // 结果下载链接
std::string preview_url;      // 预览图链接
std::string server_task_id;   // 服务端分配的任务ID (Triverse)
std::string error_message;    // 详细错误信息
int cost_credits = 0;         // 本次消耗积分
int points_balance = 0;       // 积分余额
```

#### 更新所有序列化方法

**WriteToJson()** — 在 return 前追加：

```cpp
if (!result_url.empty())    json_str["result_url"] = result_url;
if (!preview_url.empty())   json_str["preview_url"] = preview_url;
if (!server_task_id.empty()) json_str["server_task_id"] = server_task_id;
if (!error_message.empty())  json_str["error_message"] = error_message;
json_str["cost_credits"]   = cost_credits;
json_str["points_balance"] = points_balance;
```

**WriteToJsonV2()** — 在 document.Accept(writer) 前追加：
```cpp
if (!result_url.empty())
    document.AddMember("result_url", rapidjson::Value(result_url.c_str(), allocator), allocator);
if (!preview_url.empty())
    document.AddMember("preview_url", rapidjson::Value(preview_url.c_str(), allocator), allocator);
if (!server_task_id.empty())
    document.AddMember("server_task_id", rapidjson::Value(server_task_id.c_str(), allocator), allocator);
if (!error_message.empty())
    document.AddMember("error_message", rapidjson::Value(error_message.c_str(), allocator), allocator);
document.AddMember("cost_credits",   rapidjson::Value(cost_credits),   allocator);
document.AddMember("points_balance", rapidjson::Value(points_balance), allocator);
```

**WriteToJsonV3()** — 在末尾追加：
```cpp
if (!result_url.empty())
    document.AddMember("result_url", rapidjson::Value(result_url.c_str(), allocator), allocator);
if (!preview_url.empty())
    document.AddMember("preview_url", rapidjson::Value(preview_url.c_str(), allocator), allocator);
if (!server_task_id.empty())
    document.AddMember("server_task_id", rapidjson::Value(server_task_id.c_str(), allocator), allocator);
if (!error_message.empty())
    document.AddMember("error_message", rapidjson::Value(error_message.c_str(), allocator), allocator);
document.AddMember("cost_credits",   rapidjson::Value(cost_credits),   allocator);
document.AddMember("points_balance", rapidjson::Value(points_balance), allocator);
```

**CreateFromJson()** — 在 return 前追加：
```cpp
if (json_str.find("result_url") != json_str.end())
    jobinfo.result_url = json_str.at("result_url").get<std::string>();
if (json_str.find("preview_url") != json_str.end())
    jobinfo.preview_url = json_str.at("preview_url").get<std::string>();
if (json_str.find("server_task_id") != json_str.end())
    jobinfo.server_task_id = json_str.at("server_task_id").get<std::string>();
if (json_str.find("error_message") != json_str.end())
    jobinfo.error_message = json_str.at("error_message").get<std::string>();
if (json_str.find("cost_credits") != json_str.end())
    jobinfo.cost_credits = json_str.at("cost_credits").get<int>();
if (json_str.find("points_balance") != json_str.end())
    jobinfo.points_balance = json_str.at("points_balance").get<int>();
```

**CreateFromJsonV2()** — 在 return 前追加：
```cpp
if (document.HasMember("result_url") && document["result_url"].IsString())
    jobinfo.result_url = document["result_url"].GetString();
if (document.HasMember("preview_url") && document["preview_url"].IsString())
    jobinfo.preview_url = document["preview_url"].GetString();
if (document.HasMember("server_task_id") && document["server_task_id"].IsString())
    jobinfo.server_task_id = document["server_task_id"].GetString();
if (document.HasMember("error_message") && document["error_message"].IsString())
    jobinfo.error_message = document["error_message"].GetString();
if (document.HasMember("cost_credits") && document["cost_credits"].IsInt())
    jobinfo.cost_credits = document["cost_credits"].GetInt();
if (document.HasMember("points_balance") && document["points_balance"].IsInt())
    jobinfo.points_balance = document["points_balance"].GetInt();
```

**CreateFromJsonV3()** — 在 return 前追加（同上模式）：
```cpp
if (value.HasMember("result_url") && value["result_url"].IsString())
    jobinfo.result_url = value["result_url"].GetString();
if (value.HasMember("preview_url") && value["preview_url"].IsString())
    jobinfo.preview_url = value["preview_url"].GetString();
if (value.HasMember("server_task_id") && value["server_task_id"].IsString())
    jobinfo.server_task_id = value["server_task_id"].GetString();
if (value.HasMember("error_message") && value["error_message"].IsString())
    jobinfo.error_message = value["error_message"].GetString();
if (value.HasMember("cost_credits") && value["cost_credits"].IsInt())
    jobinfo.cost_credits = value["cost_credits"].GetInt();
if (value.HasMember("points_balance") && value["points_balance"].IsInt())
    jobinfo.points_balance = value["points_balance"].GetInt();
```

**load() / load_with_retry() / load_with_retry2()** — 无需修改（这些方法通过 `CreateFromJsonV2` 或 nlohmann::json 构造函数间接调用反序列化，字段自动传递）。

**BIN 相关（LoadFeedbackBin / WriteToBin）** — 不修改。生成式任务只用 JSON，BIN 路径仅服务于重建式任务，新字段保持默认值即可。

### 1.5 编译验证与 CMake 修改

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

`GenJobInfo::save/load` 内联在 `GenTaskProcess.h` 头文件中（参照 `JobInfo_s::WriteToJson` 的同款写法），不需要单独的 `.cpp` 文件，也无需修改 `Src/Util/` 下的 CMakeLists。

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

- [ ] `App/Engine/CallEngine.cpp`：在 `MakePath()` 函数末尾增加

在 `MakePath()` 函数末尾（line ~4343，cancelledJobPath 赋值之后）增加：

```cpp
// ===== 新增: 生成式任务目录 =====
std::string genEnginePath = qstr2str(Settings::getGenEngineJobQueue());
if (!JobMonitor::CreateJobQueueDir(str2qstr(genEnginePath)))
{
    JobMonitor::CreateLocalJobQueueDir();
    JobMonitor::CreateJobQueueDir(str2qstr(genEnginePath));
}

std::string genPendingJobPath   = genEnginePath + pathSeperator + JOBPENDINGSTR   + pathSeperator;
std::string genRunningJobPath   = genEnginePath + pathSeperator + JOBRUNNINGSTR   + pathSeperator;
std::string genCompletedJobPath = genEnginePath + pathSeperator + JOBCOMPLETEDSTR + pathSeperator;
std::string genFailedJobPath    = genEnginePath + pathSeperator + JOBFAILEDSTR     + pathSeperator;
std::string genCancelledJobPath = genEnginePath + pathSeperator + JOBCANCELLEDSTR + pathSeperator;
```

- [ ] `App/Engine/CallEngine.cpp`：声明 extern 变量

在文件顶部（其他 extern 声明附近）增加：

```cpp
// 生成式任务路径 (供 GenTaskThread 使用)
std::string genPendingJobPath;
std::string genRunningJobPath;
std::string genCompletedJobPath;
std::string genFailedJobPath;
std::string genCancelledJobPath;
```

> **注意**: 上面 `MakePath()` 中的局部变量需改为对全局变量的赋值（去掉 `std::string` 类型声明），或者在 CallEngine.cpp 顶部声明全局变量供 GenTaskThread 引用。

---

## Phase 3: HTTP 通信层

> 依赖 Phase 1 的数据结构（GenTaskRequest / GenTaskResponse）。

### 3.1 新建 GenHttpClient

- [ ] 创建 `App/Engine/GenHttpClient.h`

```cpp
// App/Engine/GenHttpClient.h
#ifndef _AI3D_ENGINE_GEN_HTTP_CLIENT_H_
#define _AI3D_ENGINE_GEN_HTTP_CLIENT_H_

#include "Util/GenTaskProcess.h"
#include <QString>

namespace AI3D {
namespace CORE {

class GenHttpClient
{
public:
    // 初始化: 从 MoldAIConfig.ini [GenTask] 段读取 ServerUrl
    static void Init(const std::string& configPath);

    // --- HTTP 方法 (同步, 带超时和重试) ---

    // POST /api/v1/task/submit
    static GenTaskResponse SubmitTask(const GenTaskRequest& req,
                                       int timeout_ms = 5000,
                                       int max_retries = 3);

    // GET /api/v1/task/status?task_id=<server_task_id>
    static GenTaskResponse QueryTaskStatus(const std::string& server_task_id,
                                            int timeout_ms = 3000,
                                            int max_retries = 3);

    // POST /api/v1/task/cancel
    static bool CancelTask(const std::string& server_task_id,
                           int timeout_ms = 3000,
                           int max_retries = 3);

    // POST /api/v1/upload — 上传本地文件, 返回 file_key (失败返回空)
    static std::string UploadFile(const std::string& local_path,
                                   int timeout_ms = 10000,
                                   int max_retries = 2);

private:
    // 核心: 同步 HTTP GET
    static QByteArray SyncGet(const QString& url, int timeout_ms);

    // 核心: 同步 HTTP POST (JSON body)
    static QByteArray SyncPost(const QString& url,
                                const QJsonObject& body,
                                int timeout_ms);

    // 核心: 同步 HTTP POST (multipart 文件上传)
    static QByteArray SyncPostMultipart(const QString& url,
                                         const QString& filePath,
                                         int timeout_ms);

    static QString s_serverUrl;  // http://api.example.com
    static QString s_apiPrefix;  // /api/v1
};

}} // namespace AI3D::CORE

#endif
```

- [ ] 创建 `App/Engine/GenHttpClient.cpp`

```cpp
// App/Engine/GenHttpClient.cpp
#include "GenHttpClient.h"
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
#include <QSettings>
#include <thread>
#include <chrono>

namespace AI3D {
namespace CORE {

QString GenHttpClient::s_serverUrl = "http://localhost:8080";
QString GenHttpClient::s_apiPrefix = "/api/v1";

// ========== 初始化 ==========

void GenHttpClient::Init(const std::string& configPath)
{
    QSettings settings(QString::fromStdString(configPath), QSettings::IniFormat);
    settings.beginGroup("GenTask");
    s_serverUrl = settings.value("ServerUrl", s_serverUrl).toString();
    s_apiPrefix  = settings.value("ApiPrefix",  s_apiPrefix).toString();
    settings.endGroup();
}

// ========== 同步 GET ==========

QByteArray GenHttpClient::SyncGet(const QString& url, int timeout_ms)
{
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(timeout_ms);

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

// ========== 同步 POST (JSON) ==========

QByteArray GenHttpClient::SyncPost(const QString& url,
                                     const QJsonObject& body,
                                     int timeout_ms)
{
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(timeout_ms);

    QJsonDocument doc(body);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

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

// ========== 同步 POST (multipart) ==========

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

// ========== SubmitTask ==========

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

// ========== QueryTaskStatus ==========

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

// ========== CancelTask ==========

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

// ========== UploadFile ==========

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
#ifndef _AI3D_ENGINE_GEN_TASK_THREAD_H_
#define _AI3D_ENGINE_GEN_TASK_THREAD_H_

#include <string>

namespace AI3D {
namespace CORE {

class GenTaskThread
{
public:
    // 线程入口 — 在独立 std::thread 中调用
    static void Run();

    // 取消任务 (需要在 App/Engine 层, 因为依赖 GenHttpClient)
    static bool CancelGenTask(const std::string& task_uuid);

private:
    static void ProcessPendingJobs();
    static void ProcessRunningJobs();
    static void SleepMs(int ms);
};

}} // namespace AI3D::CORE

#endif
```

- [ ] 创建 `App/Engine/GenTaskThread.cpp`

```cpp
// App/Engine/GenTaskThread.cpp
#include "GenTaskThread.h"
#include "GenHttpClient.h"
#include "Core/GenTaskAPI.h"       // ← 回调触发: GenTaskAPI::TriggerTaskComplete
#include "Util/GenTaskProcess.h"
#include "Util/TaskProcess.h"
#include "Util/Settings.h"
#include "Core/Types.h"
#include "Core/File.h"
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QHostInfo>
#include <filesystem>
#include <thread>
#include <chrono>

// extern from CallEngine.cpp
extern std::string genPendingJobPath;
extern std::string genRunningJobPath;
extern std::string genCompletedJobPath;
extern std::string genFailedJobPath;
extern std::string genCancelledJobPath;

namespace AI3D {
namespace CORE {

using namespace AI3D::CORE;

// ========== 工具函数 ==========

static std::string LockFilePath(const std::string& filePath) {
    return filePath + ".lock";
}

static bool AcquireLock(const std::string& filePath) {
    FILE* fp = File::FopenDenyWriteLockUtf8(LockFilePath(filePath));
    if (!fp) return false;
    // 锁文件保持打开 = 持有锁, 调用者负责在操作完成后 fclose
    return true;
}

static void ReleaseLock(const std::string& filePath) {
    // 锁通过 fclose 释放; 这里直接删掉 .lock 文件
    std::remove(LockFilePath(filePath).c_str());
}

static bool MoveFile(const std::string& src, const std::string& dstDir) {
    namespace fs = std::filesystem;
    fs::path srcPath = File::BoostPathFromUtf8(src);
    fs::path dstPath = File::BoostPathFromUtf8(dstDir)
                     / srcPath.filename();
    std::error_code ec;
    fs::rename(srcPath, dstPath, ec);
    if (ec) {
        LOGE("MoveFile failed: " + src + " -> " + dstDir + " err:" + ec.message());
        return false;
    }
    return true;
}

static std::string BuildFeedbackPath(const GenJobInfo& job) {
    // feedback 放在项目目录下: project/BlockName/JF_<job_name>.json
    return job.project_path + "/" + job.block_item
         + "/JF_" + job.job_name + ".json";
}

static void UpdateFeedback(const GenJobInfo& job) {
    JobFeedBack_s fb;
    std::string fbPath = BuildFeedbackPath(job);

    // 先尝试加载已有 feedback (保留已有字段)
    fb.load_with_retry(fbPath, false);

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
        if (!job.result_url.empty())
            fb.result_url = job.result_url;
        break;
    case GenTaskStatus::FAILED:
        fb.Status = jobsta_e::STATUS_FAILED;
        fb.error_message = job.params.prompt.value_or("unknown error");
        break;
    case GenTaskStatus::CANCELLED:
        fb.Status = jobsta_e::STATUS_CANCELLED;
        break;
    }

    fb.server_task_id = job.server_task_id;
    fb.save_with_retry(fbPath, false);
}

// ========== 主循环 ==========

void GenTaskThread::Run()
{
    LOGI("GenTaskThread started");

    while (true) {
        ProcessPendingJobs();
        ProcessRunningJobs();
        SleepMs(2000);  // 2s 轮询间隔
    }
}

// ========== 处理 Pending ==========

void GenTaskThread::ProcessPendingJobs()
{
    QString pendingDir = QString::fromStdString(genPendingJobPath);
    QDirIterator it(pendingDir, {"*.json"}, QDir::Files);

    while (it.hasNext()) {
        it.next();
        QString filePath = it.filePath();
        std::string filePathStr = filePath.toStdString();

        // 1. 获取锁
        FILE* fpLock = File::FopenDenyWriteLockUtf8(LockFilePath(filePathStr));
        if (!fpLock) continue;  // 被他人持有, skip

        // 2. 加载 job 文件
        GenJobInfo job = GenJobInfo::load(filePathStr);
        if (job.task_uuid.empty()) {
            fclose(fpLock);
            ReleaseLock(filePathStr);
            LOGE("ProcessPendingJobs: invalid job file: " + filePathStr);
            continue;
        }

        // 3. 崩溃恢复: 已有 server_task_id 则直接移到 Running
        if (!job.server_task_id.empty()) {
            LOGI("Crash recovery: " + job.task_uuid + " already submitted, moving to Running");
            MoveFile(filePathStr, genRunningJobPath);
            fclose(fpLock);
            ReleaseLock(filePathStr);
            continue;
        }

        // 4. 处理 AssetRef 上传 (FILE_PATH → FILE_KEY)
        auto uploadAsset = [&](std::optional<AssetRef>& asset) {
            if (asset.has_value() && asset->kind == AssetKind::FILE_PATH) {
                std::string fileKey = GenHttpClient::UploadFile(asset->value);
                if (!fileKey.empty()) {
                    asset->kind  = AssetKind::FILE_KEY;
                    asset->value = fileKey;
                } else {
                    LOGW("UploadFile failed for: " + asset->value + ", will retry next round");
                }
            }
        };
        uploadAsset(job.params.image_file);
        uploadAsset(job.params.model_file);

        // 5. 构造请求并 submit
        GenTaskRequest req;
        req.task_id     = job.task_uuid;
        req.engine_id   = job.engine_id;
        req.user_account = job.user_account;
        req.params      = job.params;

        GenTaskResponse resp = GenHttpClient::SubmitTask(req);

        // 6. 处理响应
        if (resp.status == GenTaskStatus::IDLE && resp.error_message.has_value()) {
            // 网络超时 → 不移动文件, 下轮重试
            LOGW("SubmitTask network timeout for: " + job.task_uuid);
            fclose(fpLock);
            ReleaseLock(filePathStr);
            continue;
        }

        if (resp.status == GenTaskStatus::FAILED || resp.status == GenTaskStatus::CANCELLED) {
            // 服务端拒绝
            job.status = GenTaskStatus::FAILED;
            job.server_task_id = resp.triverse_task_uuid.value_or("");
            UpdateFeedback(job);
            MoveFile(filePathStr, genFailedJobPath);
            fclose(fpLock);
            ReleaseLock(filePathStr);
            continue;
        }

        // 7. 提交成功 → 回填 server_task_id → 移动到 Running
        if (resp.triverse_task_uuid.has_value()) {
            job.server_task_id = resp.triverse_task_uuid.value();
        }
        job.status = GenTaskStatus::PENDING;
        job.save(filePathStr);
        UpdateFeedback(job);
        MoveFile(filePathStr, genRunningJobPath);

        fclose(fpLock);
        ReleaseLock(filePathStr);

        LOGI("Submitted: " + job.task_uuid + " server_task_id=" + job.server_task_id);
    }
}

// ========== 处理 Running ==========

void GenTaskThread::ProcessRunningJobs()
{
    QString runningDir = QString::fromStdString(genRunningJobPath);
    QDirIterator it(runningDir, {"*.json"}, QDir::Files);

    while (it.hasNext()) {
        it.next();
        QString filePath = it.filePath();
        std::string filePathStr = filePath.toStdString();

        // 1. 获取锁
        FILE* fpLock = File::FopenDenyWriteLockUtf8(LockFilePath(filePathStr));
        if (!fpLock) continue;

        // 2. 加载
        GenJobInfo job = GenJobInfo::load(filePathStr);
        if (job.task_uuid.empty() || job.server_task_id.empty()) {
            fclose(fpLock);
            ReleaseLock(filePathStr);
            continue;
        }

        // 3. 查询状态
        GenTaskResponse resp = GenHttpClient::QueryTaskStatus(job.server_task_id);

        if (resp.status == GenTaskStatus::IDLE && resp.error_message.has_value()) {
            // 网络超时: query_retry_count++
            job.query_retry_count++;
            if (job.query_retry_count >= 5) {
                LOGE("Task " + job.task_uuid + " query failed 5 times, moving to Failed");
                job.status = GenTaskStatus::FAILED;
                UpdateFeedback(job);
                MoveFile(filePathStr, genFailedJobPath);
                GenTaskAPI::TriggerTaskFailed(job.task_uuid, "连续 5 次轮询超时");
                }
            } else {
                job.save(filePathStr);  // 仅更新计数
            }
            fclose(fpLock);
            ReleaseLock(filePathStr);
            continue;
        }

        // 4. 重置重试计数 (成功获取到响应)
        job.query_retry_count = 0;

        switch (resp.status) {

        case GenTaskStatus::COMPLETED: {
            job.status     = GenTaskStatus::COMPLETED;
            if (resp.result_url.has_value())
                job.result_url = resp.result_url.value();
            job.save(filePathStr);
            UpdateFeedback(job);
            MoveFile(filePathStr, genCompletedJobPath);
            LOGI("Completed: " + job.task_uuid);
            GenTaskAPI::TriggerTaskComplete(job.task_uuid, job.result_url);
            break;
        }

        case GenTaskStatus::FAILED: {
            job.status = GenTaskStatus::FAILED;
            job.save(filePathStr);
            UpdateFeedback(job);
            MoveFile(filePathStr, genFailedJobPath);
            LOGE("Failed: " + job.task_uuid);
            GenTaskAPI::TriggerTaskFailed(job.task_uuid,
                resp.error_message.value_or("server returned failed"));
            break;
        }

        case GenTaskStatus::IN_PROGRESS: {
            // 更新进度到 feedback
            job.status = GenTaskStatus::IN_PROGRESS;
            job.save(filePathStr);
            UpdateFeedback(job);
            break;
        }

        default:
            break;
        }

        fclose(fpLock);
        ReleaseLock(filePathStr);
    }
}

void GenTaskThread::SleepMs(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

}} // namespace AI3D::CORE
```

### 4.4 启动线程 + CMakeLists

- [ ] `App/Engine/CallEngine.cpp` `main()` — 在 `MakePath()` 调用之后增加：

```cpp
// ===== 新增: 初始化 HTTP 客户端 =====
std::string configpath = apppath + "/" + "MoldAIConfig.ini";
GenHttpClient::Init(configpath);

// ===== 新增: 生成式任务线程 =====
std::thread genTaskThread(GenTaskThread::Run);
genTaskThread.detach();
```

同时在 `CallEngine.cpp` 顶部 include：

```cpp
#include "GenHttpClient.h"
#include "GenTaskThread.h"
```

- [ ] **CMakeLists**: `GenTaskThread.h/cpp` + `GenHttpClient.h/cpp` 均在 `App/Engine/` 下，被 `FILE(GLOB)` 自动拾取，**无需修改 CMakeLists**。

### 4.5 并行验证

- [ ] 重建式任务（`searchPendingJobThread2`）和生成式任务（`generateThread`）同时运行
- [ ] 确认两个线程互不干扰（操作不同目录）

---

## Phase 5: SDK 接口

> 依赖 Phase 4（线程已就绪，可直接往 Pending 写文件测试）。

### 5.1 新建 GenTaskAPI

- [ ] 创建 `Include/Core/GenTaskAPI.h`

```cpp
// Include/Core/GenTaskAPI.h
#ifndef _AI3D_CORE_GEN_TASK_API_H_
#define _AI3D_CORE_GEN_TASK_API_H_

#include "Util/GenTaskProcess.h"
#include <string>
#include <functional>

// 前向声明
namespace AI3D { namespace CORE { class BlockObject; } }

namespace AI3D {
namespace CORE {

class GenTaskAPI
{
public:
    // ========== 返回值结构 ==========

    struct SubmitResult {
        std::string task_uuid;
        std::string job_name;
        std::string feedback_path;
        bool        success = false;
        std::string error_msg;
    };

    struct TaskStatusResult {
        GenTaskStatus status = GenTaskStatus::IDLE;
        int           progress = 0;
        std::string   result_url;
        std::string   preview_url;
        std::string   error_message;
        int           cost_credits = 0;
    };

    // ========== 核心接口 (前端 SDK, 编译进 MoldAIData.dll) ==========

    static SubmitResult SubmitGenTask(
        const AI3D::CORE::BlockObject::Task_Info& blockInfo,
        const GenerationParams& params,
        const std::string& user_account);

    static TaskStatusResult QueryTaskStatus(const std::string& task_uuid);

    static bool DownloadResult(const std::string& task_uuid,
                                const std::string& save_path);

    // ========== 回调 — 前端注册, GenTaskThread 触发 ==========

    using TaskCompleteCallback = std::function<void(
        const std::string& task_uuid,
        const std::string& result_url)>;

    using TaskFailedCallback = std::function<void(
        const std::string& task_uuid,
        const std::string& error)>;

    static void SetTaskCompleteCallback(TaskCompleteCallback cb);
    static void SetTaskFailedCallback(TaskFailedCallback cb);

    // 由 GenTaskThread 调用 (不依赖 App/Engine, 单向依赖)
    static void TriggerTaskComplete(const std::string& task_uuid,
                                     const std::string& result_url);
    static void TriggerTaskFailed(const std::string& task_uuid,
                                   const std::string& error);

private:
    static std::string FindFeedbackPath(const std::string& task_uuid);
    static std::string FindServerTaskId(const std::string& task_uuid);

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
// 编译进 MoldAIData.dll, 不依赖 App/Engine/ 下的任何文件
#include "Core/GenTaskAPI.h"
#include "Core/BlockObject.h"
#include "Util/GenTaskProcess.h"
#include "Util/TaskProcess.h"
#include "Util/Settings.h"
#include "Core/json.h"
#include <QUuid>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QHostInfo>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <fstream>

namespace AI3D {
namespace CORE {

// 回调存储
GenTaskAPI::TaskCompleteCallback GenTaskAPI::s_completeCallback = nullptr;
GenTaskAPI::TaskFailedCallback   GenTaskAPI::s_failedCallback   = nullptr;

// ========== 回调注册 (前端调用) ==========

void GenTaskAPI::SetTaskCompleteCallback(TaskCompleteCallback cb) {
    s_completeCallback = std::move(cb);
}
void GenTaskAPI::SetTaskFailedCallback(TaskFailedCallback cb) {
    s_failedCallback = std::move(cb);
}

// ========== 回调触发 (GenTaskThread 调用) ==========

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

// ========== SubmitGenTask ==========

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

    // 2. 构造 GenJobInfo
    GenJobInfo job;
    job.task_uuid    = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    job.job_name     = blockInfo.blockName + "_"
                     + QDateTime::currentDateTime().toString("yyyyMMddhhmmss").toStdString();
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
                      + "/" + QString::fromStdString(job.job_name);
    QDir().mkpath(resultDir);

    // 4. 获取 jobs_gen/Pending/ 路径
    QString genRoot    = Settings::getGenEngineJobQueue();
    QString pendingPath = genRoot + "/" + JOBPENDINGSTR + "/";
    QDir().mkpath(pendingPath);

    // 5. 写入 job 文件
    std::string jobFilePath = pendingPath.toStdString() + job.job_name + ".json";
    if (!job.save(jobFilePath)) {
        result.success   = false;
        result.error_msg = "Failed to write job file: " + jobFilePath;
        return result;
    }

    // 6. 创建初始 feedback
    std::string feedbackPath = blockInfo.projectfile_ + "/"
                             + blockInfo.blockName + "/JF_"
                             + job.job_name + ".json";

    JobFeedBack_s fb;
    fb.Status  = jobsta_e::STATUS_PENDDING;
    fb.Percent = 0.0f;
    fb.save_with_retry(feedbackPath, false);

    // 7. 返回
    result.success       = true;
    result.task_uuid     = job.task_uuid;
    result.job_name      = job.job_name;
    result.feedback_path = feedbackPath;
    return result;
}

// ========== QueryTaskStatus ==========

GenTaskAPI::TaskStatusResult GenTaskAPI::QueryTaskStatus(const std::string& task_uuid)
{
    TaskStatusResult result;

    std::string fbPath = FindFeedbackPath(task_uuid);
    if (fbPath.empty()) {
        result.status        = GenTaskStatus::IDLE;
        result.error_message = "feedback not found for task_uuid: " + task_uuid;
        return result;
    }

    JobFeedBack_s fb;
    if (!fb.load_with_retry(fbPath, false)) {
        result.status        = GenTaskStatus::IDLE;
        result.error_message = "failed to load feedback: " + fbPath;
        return result;
    }

    // 映射 jobsta_e → GenTaskStatus
    switch (fb.Status) {
    case jobsta_e::STATUS_PENDDING:   result.status = GenTaskStatus::PENDING;      break;
    case jobsta_e::STATUS_RUNNING:    result.status = GenTaskStatus::IN_PROGRESS;  break;
    case jobsta_e::STATUS_COMPLETE:   result.status = GenTaskStatus::COMPLETED;    break;
    case jobsta_e::STATUS_FAILED:     result.status = GenTaskStatus::FAILED;       break;
    case jobsta_e::STATUS_CANCELLED:  result.status = GenTaskStatus::CANCELLED;    break;
    default:                          result.status = GenTaskStatus::IDLE;         break;
    }

    result.progress      = static_cast<int>(fb.Percent);
    result.result_url    = fb.result_url;
    result.preview_url   = fb.preview_url;
    result.error_message = fb.error_message;
    result.cost_credits  = fb.cost_credits;

    return result;
}

// ========== DownloadResult ==========

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

std::string GenTaskAPI::FindFeedbackPath(const std::string& task_uuid)
{
    // 遍历 jobs_gen/ 下各状态目录, 通过 job_name 定位 feedback
    // job 文件路径: jobs_gen/*/<job_name>.json
    // feedback 路径: project_path/BlockName/JF_<job_name>.json

    QString genRoot = Settings::getGenEngineJobQueue();
    QStringList subDirs = {JOBPENDINGSTR, JOBRUNNINGSTR, JOBCOMPLETEDSTR,
                           JOBFAILEDSTR, JOBCANCELLEDSTR};

    for (const QString& sub : subDirs) {
        QDirIterator it(genRoot + "/" + sub, {"*.json"}, QDir::Files);
        while (it.hasNext()) {
            it.next();
            GenJobInfo job = GenJobInfo::load(it.filePath().toStdString());
            if (job.task_uuid == task_uuid) {
                return job.project_path + "/" + job.block_item
                     + "/JF_" + job.job_name + ".json";
            }
        }
    }

    return "";
}

std::string GenTaskAPI::FindServerTaskId(const std::string& task_uuid)
{
    QString genRoot = Settings::getGenEngineJobQueue();
    QStringList subDirs = {JOBPENDINGSTR, JOBRUNNINGSTR, JOBCOMPLETEDSTR,
                           JOBFAILEDSTR, JOBCANCELLEDSTR};

    for (const QString& sub : subDirs) {
        QDirIterator it(genRoot + "/" + sub, {"*.json"}, QDir::Files);
        while (it.hasNext()) {
            it.next();
            GenJobInfo job = GenJobInfo::load(it.filePath().toStdString());
            if (job.task_uuid == task_uuid) {
                return job.server_task_id;
            }
        }
    }

    return "";
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

`CancelGenTask` 也需要 `GenHttpClient`，实现放在 `GenTaskThread` 中（或前期不实现，由前端直接操作文件取消）：

```cpp
// GenTaskThread 中额外提供:
static bool CancelGenTask(const std::string& task_uuid);
```

### 5.4 线程安全

回调在 `generateThread`（非 Qt 主线程）中执行。调用者如需更新 UI，自行处理：

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
- [ ] **验证点**: generateThread 读取 → HTTP submit → 文件移到 `Running/`
- [ ] **验证点**: 轮询完成 → 文件移到 `Completed/` → feedback 更新为 COMPLETED
- [ ] **验证点**: 前端通过 SDK 读取 feedback 并下载结果文件

### 6.2 崩溃恢复测试

```cpp
// Test/GenTaskCrashRecoveryTest.cpp
// 手动测试步骤:
//
// Step 1: 提交一个任务, 拿到 task_uuid
// Step 2: 观察 jobs_gen/Pending/ 确认 job 文件存在
// Step 3: 等待 generateThread 的 HTTP submit 返回 (日志: "Submitted: xxx server_task_id=...")
// Step 4: 在 job 文件被移动到 Running 之前 (2s 窗口), 立即 kill Node 进程
// Step 5: 检查 Pending 中的 job 文件内容 — server_task_id 应该已被回填 (非空)
// Step 6: 重启 Node
// Step 7: 观察 generateThread 日志:
//         "Crash recovery: xxx already submitted, moving to Running"
// Step 8: 确认服务端没有收到重复的 submit 请求
// Step 9: 确认任务最终正常完成

void SimulateCrashRecoveryTest()
{
    // 此测试需要配合服务端 mock, 验证逻辑:
    //
    // 1. GenTaskThread::ProcessPendingJobs() 中:
    //    if (!job.server_task_id.empty()) {  ← 崩溃恢复路径
    //        MoveFile(filePathStr, genRunningJobPath);
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
// Step 3: 观察 generateThread 日志: "SubmitTask network timeout for: xxx"
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
    //   generateThread            → 只扫描 jobs_gen/
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
Timeout=5000
MaxRetries=3
```

测试时 Node 启动后会自动读取此配置（需在 `main()` 中调用 `GenHttpClient::Init(configpath)`）。

---

## 文件改动汇总

### 新建（6 个，无需修改 CMakeLists 即可自动编译）

| 文件 | 位置 | CMake | 说明 |
|------|------|-------|------|
| `Include/Util/GenTaskProcess.h` | Util/ | 需显式添加 | 对标 TaskProcess.h，`GenJobInfo` + 枚举 + API 类型 |
| `App/Engine/GenHttpClient.h` | Engine/ | GLOB 自动拾取 | 同步 HTTP 客户端 |
| `App/Engine/GenHttpClient.cpp` | Engine/ | GLOB 自动拾取 | |
| `App/Engine/GenTaskThread.h` | Engine/ | GLOB 自动拾取 | generateThread 调度线程 |
| `App/Engine/GenTaskThread.cpp` | Engine/ | GLOB 自动拾取 | |
| `Include/Core/GenTaskAPI.h` | Core/ | GLOB 自动拾取 | 前端 SDK 接口 |
| `Src/Core/GenTaskAPI.cpp` | Core/ | GLOB 自动拾取 | 编译进 MoldAIData.dll |

### 修改（6 个）

| 文件 | Phase | 改动 |
|------|-------|------|
| `Include/Core/BlockObject.h` | P1 | `Task_Info` 增加 `block_task_category` |
| `Include/Core/BlockObject.cpp` | P1 | 4 个序列化方法增加新字段 |
| `Include/Core/BlockInfo.h` | P1 | `BlockFile` 增加 `block_task_category` (BIN 格式) |
| `Include/Util/TaskProcess.h` | P1 | `JobInfo_s` + `JobFeedBack_s` 增加新字段; 更新 10+ 序列化方法 |
| `Include/Util/Settings.h` | P2 | 增加 `getGenEngineJobQueue()` |
| `Src/Util/Settings.cpp` | P2 | 实现 `getGenEngineJobQueue()` |
| `App/Engine/CallEngine.cpp` | P2+P4 | MakePath 创建 jobs_gen/ 目录; main 启动 generateThread |
| `App/Engine/CMakeLists.txt` | P1 | HEADER_LIST 显式添加 `GenTaskProcess.h` (唯一需改的 CMakeLists) |

### 不动

`TaskGraph_s`、`Task_s`、`ATTaskInfo`、`ExecTaskFileV2`、`GetPendingJob`、`Src/Core/CMakeLists.txt` — 全部不动。
