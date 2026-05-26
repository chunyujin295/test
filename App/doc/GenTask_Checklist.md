# 生成式任务系统 — 执行清单

按依赖关系排列，完成一项勾一项。

## 依赖关系图

```
Phase 1 (数据结构) ──→ Phase 3 (HTTP) ──→ Phase 4 (调度线程) ──→ Phase 5 (SDK)
                                            ↗
Phase 2 (路径配置) ─────────────────────────┘

Phase 1 + 2 可以并行开工
```

---

## Phase 1: 数据结构基础

> 最先做，所有后续 Phase 都依赖这里的结构定义。

### 1.1 新建 GenTaskDef.h

- [ ] 创建 `Include/Util/GenTaskDef.h`

```cpp
// Include/Util/GenTaskDef.h
#ifndef _AI3D_UTIL_GEN_TASK_DEF_H_
#define _AI3D_UTIL_GEN_TASK_DEF_H_

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

namespace AI3D {
namespace CORE {

// ========== 枚举 ==========

enum class GenTaskCategory {
    TEXT_TO_3D,         // 文生3D
    IMAGE_TO_3D,        // 图生3D
    TEXTURING,          // 纹理生成
    UTILITY,            // 工具
    IMAGE_GENERATION,   // 图片生成
};

enum class GenTaskSubType {
    TEXT_TO_MODEL,          // POST /tasks/text-to-model      文字→带纹理模型
    TEXT_TO_MESH,           // POST /tasks/text-to-mesh       文字→纯白模
    IMAGE_TO_MODEL,         // POST /tasks/image-to-model     图片→带纹理模型
    IMAGE_TO_MESH,          // POST /tasks/image-to-mesh      图片→纯白模
    TEXTURE_MODEL,          // POST /tasks/texture-model      图片+模型→纹理
    TEXT_TO_TEXTURE,        // POST /tasks/text-to-texture    文字+模型→纹理
    MODEL_PREVIEW_RENDER,   // POST /tasks/model-preview-render  模型→预览图
    MODEL_REMESH,           // POST /tasks/model-remesh          重网格/减面
    CONVERT_MODEL_FORMAT,   // POST /tasks/convert-model-format  格式转换
    IMAGE_GENERATION,       // POST /tasks/image-generation      文字→图片
};

enum class GenTaskStatus {
    IDLE,           // 未提交
    PENDING,        // 排队等待
    IN_PROGRESS,    // 处理中
    COMPLETED,      // 完成
    FAILED,         // 失败
    CANCELLED,      // 已取消
};

enum class AssetKind {
    NONE,
    FILE_PATH,      // 用户拖的本地文件
    FILE_KEY,       // 已上传到 Triverse 的文件 key
    URL,            // 公开链接
};

// ========== JSON 序列化宏 (nlohmann::json) ==========

NLOHMANN_JSON_SERIALIZE_ENUM(GenTaskCategory, {
    {GenTaskCategory::TEXT_TO_3D,         "text_to_3d"},
    {GenTaskCategory::IMAGE_TO_3D,        "image_to_3d"},
    {GenTaskCategory::TEXTURING,          "texturing"},
    {GenTaskCategory::UTILITY,            "utility"},
    {GenTaskCategory::IMAGE_GENERATION,   "image_generation"},
})

NLOHMANN_JSON_SERIALIZE_ENUM(GenTaskSubType, {
    {GenTaskSubType::TEXT_TO_MODEL,         "text_to_model"},
    {GenTaskSubType::TEXT_TO_MESH,          "text_to_mesh"},
    {GenTaskSubType::IMAGE_TO_MODEL,        "image_to_model"},
    {GenTaskSubType::IMAGE_TO_MESH,         "image_to_mesh"},
    {GenTaskSubType::TEXTURE_MODEL,         "texture_model"},
    {GenTaskSubType::TEXT_TO_TEXTURE,       "text_to_texture"},
    {GenTaskSubType::MODEL_PREVIEW_RENDER,  "model_preview_render"},
    {GenTaskSubType::MODEL_REMESH,          "model_remesh"},
    {GenTaskSubType::CONVERT_MODEL_FORMAT,  "convert_model_format"},
    {GenTaskSubType::IMAGE_GENERATION,      "image_generation"},
})

NLOHMANN_JSON_SERIALIZE_ENUM(GenTaskStatus, {
    {GenTaskStatus::IDLE,         "idle"},
    {GenTaskStatus::PENDING,      "pending"},
    {GenTaskStatus::IN_PROGRESS,  "in_progress"},
    {GenTaskStatus::COMPLETED,    "completed"},
    {GenTaskStatus::FAILED,       "failed"},
    {GenTaskStatus::CANCELLED,    "cancelled"},
})

NLOHMANN_JSON_SERIALIZE_ENUM(AssetKind, {
    {AssetKind::NONE,      "none"},
    {AssetKind::FILE_PATH, "file_path"},
    {AssetKind::FILE_KEY,  "file_key"},
    {AssetKind::URL,       "url"},
})

// ========== 结构体 ==========

struct AssetRef {
    AssetKind   kind = AssetKind::NONE;
    std::string value;          // 本地路径 / file_key / URL
    std::string content_type;   // MIME 类型，如 "image/png"，仅本地上传时用
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AssetRef, kind, value, content_type)

struct GenerationParams {
    // 任务类型
    GenTaskCategory category = GenTaskCategory::TEXT_TO_3D;
    GenTaskSubType  sub_type = GenTaskSubType::TEXT_TO_MODEL;

    // 文字输入
    std::optional<std::string> prompt;
    std::optional<std::string> negative_prompt;
    std::optional<std::string> style;

    // 素材
    std::optional<AssetRef> image_file;
    std::optional<AssetRef> model_file;

    // 生成控制
    std::optional<std::string> model_version;
    std::optional<int>         polygon_limit;
    std::optional<int>         texture_size;

    // 预览渲染 [MPR]
    std::optional<std::string>              render_mode;
    std::optional<int>                      image_count;
    std::optional<std::vector<std::string>> camera_angles;
    std::optional<std::string>              preset_name;

    // 重网格 [RMS]
    std::optional<int>    target_poly_count;

    // 格式转换 [CVT]
    std::optional<std::string> output_format;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GenerationParams,
    category, sub_type,
    prompt, negative_prompt, style,
    image_file, model_file,
    model_version, polygon_limit, texture_size,
    render_mode, image_count, camera_angles, preset_name,
    target_poly_count, output_format)

struct GenTaskRequest {
    std::string     task_id;        // 客户端生成的 UUID，全程追踪
    std::string     engine_id;      // 引擎实例标识，如 "engine-sh-01"
    std::string     user_account;   // 用户账号
    GenerationParams params;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GenTaskRequest, task_id, engine_id, user_account, params)

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
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GenTaskResponse,
    task_id, triverse_task_uuid, status, progress,
    result_url, preview_url, error_message,
    cost_credits, points_balance)

// ========== GenJobInfo — job 文件顶层结构 ==========

struct GenJobInfo {
    // 任务标识
    std::string task_uuid;        // 客户端 UUID (QUuid::createUuid())
    std::string job_name;         // job 文件名 = blockName + "_" + timestamp
    std::string engine_id;        // 本机 hostname
    std::string user_account;     // 用户账号

    // 关联信息
    std::string project_path;     // 项目路径 (Block.projectfile_)
    std::string block_item;       // Block 名称 (Block.block_info_.blockName)

    // 任务定义
    GenTaskCategory category = GenTaskCategory::TEXT_TO_3D;
    GenTaskSubType  sub_type = GenTaskSubType::TEXT_TO_MODEL;
    GenerationParams params;

    // 状态追踪
    GenTaskStatus status = GenTaskStatus::IDLE;
    std::string server_task_id;   // Triverse 返回后回填
    std::string result_url;       // 下载链接 (完成后回填)
    int query_retry_count = 0;     // 连续轮询失败计数

    // 序列化 (只用 JSON)
    bool save(const std::string& filePath) const;
    static GenJobInfo load(const std::string& filePath);
};

inline void to_json(nlohmann::json& j, const GenJobInfo& info) {
    j = nlohmann::json{
        {"task_uuid",    info.task_uuid},
        {"job_name",     info.job_name},
        {"engine_id",    info.engine_id},
        {"user_account", info.user_account},
        {"project_path", info.project_path},
        {"block_item",   info.block_item},
        {"category",     info.category},
        {"sub_type",     info.sub_type},
        {"params",       info.params},
        {"status",       info.status},
        {"server_task_id", info.server_task_id},
        {"result_url",     info.result_url},
        {"query_retry_count", info.query_retry_count},
    };
}

inline void from_json(const nlohmann::json& j, GenJobInfo& info) {
    j.at("task_uuid").get_to(info.task_uuid);
    j.at("job_name").get_to(info.job_name);
    j.at("engine_id").get_to(info.engine_id);
    j.at("user_account").get_to(info.user_account);
    j.at("project_path").get_to(info.project_path);
    j.at("block_item").get_to(info.block_item);
    j.at("category").get_to(info.category);
    j.at("sub_type").get_to(info.sub_type);
    j.at("params").get_to(info.params);
    j.at("status").get_to(info.status);
    if (j.contains("server_task_id")) j.at("server_task_id").get_to(info.server_task_id);
    if (j.contains("result_url"))     j.at("result_url").get_to(info.result_url);
    if (j.contains("query_retry_count")) j.at("query_retry_count").get_to(info.query_retry_count);
}

} // namespace CORE
} // namespace AI3D

#endif // _AI3D_UTIL_GEN_TASK_DEF_H_
```

`GenJobInfo::save` / `GenJobInfo::load` 实现放到 `Src/Util/GenTaskDef.cpp`（也可以直接内联在 .h 中）：

```cpp
// Src/Util/GenTaskDef.cpp
#include "Util/GenTaskDef.h"
#include <fstream>
#include <Core/File.h>

namespace AI3D {
namespace CORE {

bool GenJobInfo::save(const std::string& filePath) const {
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

GenJobInfo GenJobInfo::load(const std::string& filePath) {
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

} // namespace CORE
} // namespace AI3D
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

### 1.5 编译验证

- [ ] 头文件变更后全量编译通过
- [ ] 确认无遗漏的序列化方法

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

#include "Util/GenTaskDef.h"
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

### 3.2 修改 CMakeLists

- [ ] `App/Engine/CMakeLists.txt`

```cmake
# 在现有 target_sources 中增加:
target_sources(MoldAINode PRIVATE
    GenHttpClient.cpp
    GenTaskThread.cpp
)

# 在现有 target_link_libraries 中增加:
target_link_libraries(MoldAINode PRIVATE
    Qt6::Network
)
```

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

namespace AI3D {
namespace CORE {

class GenTaskThread
{
public:
    // 线程入口 — 在独立 std::thread 中调用
    static void Run();

    // 注册回调 (供 GenTaskAPI 设置)
    using TaskCompleteCallback = std::function<void(const std::string& task_uuid,
                                                     const std::string& result_url)>;
    using TaskFailedCallback   = std::function<void(const std::string& task_uuid,
                                                     const std::string& error)>;

    static void SetTaskCompleteCallback(TaskCompleteCallback cb);
    static void SetTaskFailedCallback(TaskFailedCallback cb);

private:
    static void ProcessPendingJobs();
    static void ProcessRunningJobs();
    static void SleepMs(int ms);

    static TaskCompleteCallback s_completeCallback;
    static TaskFailedCallback   s_failedCallback;
};

}} // namespace AI3D::CORE

#endif
```

- [ ] 创建 `App/Engine/GenTaskThread.cpp`

```cpp
// App/Engine/GenTaskThread.cpp
#include "GenTaskThread.h"
#include "GenHttpClient.h"
#include "Util/GenTaskDef.h"
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

// extern from CallEngine.cpp (MakePath 中赋值的全局变量)
extern std::string genPendingJobPath;
extern std::string genRunningJobPath;
extern std::string genCompletedJobPath;
extern std::string genFailedJobPath;
extern std::string genCancelledJobPath;

namespace AI3D {
namespace CORE {

using namespace AI3D::CORE;

GenTaskThread::TaskCompleteCallback GenTaskThread::s_completeCallback = nullptr;
GenTaskThread::TaskFailedCallback   GenTaskThread::s_failedCallback   = nullptr;

void GenTaskThread::SetTaskCompleteCallback(TaskCompleteCallback cb) {
    s_completeCallback = std::move(cb);
}
void GenTaskThread::SetTaskFailedCallback(TaskFailedCallback cb) {
    s_failedCallback = std::move(cb);
}

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
                if (s_failedCallback) {
                    s_failedCallback(job.task_uuid, "连续 5 次轮询超时");
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
            if (s_completeCallback) {
                s_completeCallback(job.task_uuid, job.result_url);
            }
            break;
        }

        case GenTaskStatus::FAILED: {
            job.status = GenTaskStatus::FAILED;
            job.save(filePathStr);
            UpdateFeedback(job);
            MoveFile(filePathStr, genFailedJobPath);
            LOGE("Failed: " + job.task_uuid);
            if (s_failedCallback) {
                s_failedCallback(job.task_uuid,
                    resp.error_message.value_or("server returned failed"));
            }
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

### 4.4 启动线程

- [ ] `App/Engine/CallEngine.cpp` `main()` — 在 `MakePath()` 调用之后、现有 `searchPendingJob1` 启动之前增加：

```cpp
// ===== 新增: 初始化 HTTP 客户端 =====
std::string configpath = apppath + "/" + "MoldAIConfig.ini";
GenHttpClient::Init(configpath);

// ===== 新增: 声明全局变量 (extern) =====
// genPendingJobPath, genRunningJobPath 等在 MakePath() 中已赋值

// ===== 新增: 生成式任务线程 =====
std::thread genTaskThread(GenTaskThread::Run);
genTaskThread.detach();
```

同时需要在 `CallEngine.cpp` 顶部 include：

```cpp
#include "GenHttpClient.h"
#include "GenTaskThread.h"
```

- [ ] `App/Engine/CMakeLists.txt`：增加 `GenTaskThread.cpp` + `GenHttpClient.cpp` + `GenTaskDef.cpp`

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

#include "Util/GenTaskDef.h"
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

    // ========== 核心接口 ==========

    // 提交生成式任务 — 写入 jobs_gen/Pending/ 并创建 feedback
    static SubmitResult SubmitGenTask(
        const AI3D::CORE::BlockObject::Task_Info& blockInfo,
        const GenerationParams& params,
        const std::string& user_account);

    // 查询任务状态 — 读取 feedback 文件
    static TaskStatusResult QueryTaskStatus(const std::string& task_uuid);

    // 下载结果到本地
    static bool DownloadResult(const std::string& task_uuid,
                                const std::string& save_path);

    // 取消任务
    static bool CancelGenTask(const std::string& task_uuid);

    // ========== 回调 (在 generateThread 线程中调用) ==========

    using TaskCompleteCallback = std::function<void(
        const std::string& task_uuid,
        const std::string& result_url)>;

    using TaskFailedCallback = std::function<void(
        const std::string& task_uuid,
        const std::string& error)>;

    static void SetTaskCompleteCallback(TaskCompleteCallback cb);
    static void SetTaskFailedCallback(TaskFailedCallback cb);

private:
    // 通过 task_uuid 查找 feedback 文件路径
    // 遍历 project_path 下的 Block 目录, 查找 JF_<job_name>.json
    static std::string FindFeedbackPath(const std::string& task_uuid);

    // 通过 feedback 文件找到 job 文件中的 server_task_id
    static std::string FindServerTaskId(const std::string& task_uuid);
};

}} // namespace AI3D::CORE

#endif
```

### 5.2 创建 GenTaskAPI.cpp

```cpp
// Src/Core/GenTaskAPI.cpp
#include "Core/GenTaskAPI.h"
#include "Core/BlockObject.h"
#include "Util/GenTaskDef.h"
#include "Util/TaskProcess.h"
#include "Util/Settings.h"
#include "Util/JobMonitor.h"
#include "Engine/GenTaskThread.h"
#include "Engine/GenHttpClient.h"
#include <QUuid>
#include <QDir>
#include <QFileInfo>
#include <QHostInfo>
#include <QDateTime>
#include <fstream>

namespace AI3D {
namespace CORE {

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

// ========== CancelGenTask ==========

bool GenTaskAPI::CancelGenTask(const std::string& task_uuid)
{
    std::string serverTaskId = FindServerTaskId(task_uuid);
    if (serverTaskId.empty()) {
        LOGE("CancelGenTask: cannot find server_task_id for " + task_uuid);
        return false;
    }
    return GenHttpClient::CancelTask(serverTaskId);
}

// ========== 回调 ==========

void GenTaskAPI::SetTaskCompleteCallback(TaskCompleteCallback cb) {
    GenTaskThread::SetTaskCompleteCallback(std::move(cb));
}
void GenTaskAPI::SetTaskFailedCallback(TaskFailedCallback cb) {
    GenTaskThread::SetTaskFailedCallback(std::move(cb));
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

### 5.3 线程安全说明

回调在 `generateThread`（非 Qt 主线程）中执行。调用者如需更新 UI，需自行处理线程切换：

```cpp
// 前端使用示例:
GenTaskAPI::SetTaskCompleteCallback([](const std::string& task_uuid, const std::string& url) {
    // 此回调在 generateThread 线程中执行!
    // 如需更新 UI, 使用:
    QMetaObject::invokeMethod(qApp, [=]() {
        // UI 更新代码...
    }, Qt::QueuedConnection);
});
```

### 5.4 编译配置

- [ ] `Src/Core/CMakeLists.txt`：增加 `GenTaskAPI.cpp`

```cmake
target_sources(MoldAIData PRIVATE
    GenTaskAPI.cpp
)
```

> **注意**: `GenTaskAPI.cpp` 引用了 `GenHttpClient` 和 `GenTaskThread`（App/Engine/ 下的文件）。如果 `MoldAIData.dll` 不应依赖 Engine 层，可以把 `GenTaskAPI.cpp` 放到 `App/Engine/` 目录下，或把回调设置函数拆分到 Engine 侧。

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

### 新建（7 个）

| 文件 | Phase |
|------|-------|
| `Include/Util/GenTaskDef.h` | P1 |
| `App/Engine/GenHttpClient.h` | P3 |
| `App/Engine/GenHttpClient.cpp` | P3 |
| `App/Engine/GenTaskThread.h` | P4 |
| `App/Engine/GenTaskThread.cpp` | P4 |
| `Include/Core/GenTaskAPI.h` | P5 |
| `Src/Core/GenTaskAPI.cpp` | P5 |

### 修改（7 个）

| 文件 | Phase | 风险 |
|------|-------|------|
| `Include/Core/BlockObject.h` | P1 | 低 |
| `Include/Core/BlockObject.cpp` | P1 | 低 |
| `Include/Util/TaskProcess.h` | P1 | 中 — 序列化方法多 |
| `Include/Util/Settings.h` | P2 | 低 |
| `Src/Util/Settings.cpp` | P2 | 低 |
| `App/Engine/CallEngine.cpp` | P2+P4 | 低 |
| `App/Engine/CMakeLists.txt` | P3+P4 | 低 |
| `Src/Core/CMakeLists.txt` | P5 | 低 |

### 不动（5 个）

`TaskGraph_s`、`Task_s`、`ATTaskInfo`、`ExecTaskFileV2`、`GetPendingJob` — 全部不动。
