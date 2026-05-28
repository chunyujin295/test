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
| `DataStruct.h` (修改) | `JobListFile` / `FeedBackFile` / `BLKBinFile` | 新增 `GenJobInfoData`/`GenJobFile`（对标 `JobInfoData`/`JobListFile` 的 BIN 模式, `GenJobFile` 含 `FeedBackData feedBackData`）；`BLKBinFile` 增加 `GenTaskOptions` 序列化字段 |
| `GenJobInfo_s` | `JobInfo_s` | 纯数据结构体 — `JobInfo_s` 存 `ProjectPath/ItemPath`，`GenJobInfo_s` 存 `task_uuid/GenTaskParams/server_task_id/result_url`。I/O 由 GenJobFullInfo_s 负责 (BIN 加密, params 序列化为 JSON 字符串存储) |
| `GenJobFullInfo_s` | `JobFullInfo_s` | 文件级结构体 — 持有 `job_name` + `GenJobInfo_s job` + `JobFeedBack_s feedback`（对标 TaskGraph_s 持有 JobFeedBack_s），提供 `save`/`load` (BIN 加密) + `WriteToBin`/`LoadFromBin`。feedback 序列化到 BIN (对标 JobListFile::feedBackData) |
| `GenTaskResponse` | 无现成对标 | HTTP 响应体。服务端返回的 task status/progress/result_url |
| `GenTaskStatus` | `jobsta_e`（局部类似） | 生成式任务的状态枚举。`jobsta_e` 是重建式 job 状态（PENDING/RUNNING/COMPLETE...），`GenTaskStatus` 多了 IDLE + 服务端状态 |
| `GenTaskOptions.h` | `ATOptions.h` | 生成式任务参数结构体，对标 `ATOptions`。包含 `GenTaskParams` (生成参数, 含 JSON 方法) + `GenTaskOptions` (block_task_category + params)，嵌入 `Task_Info` |
| `JobFeedBack_s` — **不修改** | 自身 | 生成式任务复用 `Status`/`Percent`/`Msg`/`TaskRetVal` 做进度反馈。结果数据（`result_url` 等）存在 `GenJobInfo_s` 中，不扩展 feedback 字段 |

### 调度与通信层

| 新增 | 对标现有 | 说明 |
|------|---------|------|
| `GenTaskThread` | `searchPendingJobThread2` (CallEngine.cpp) | 调度线程。`searchPendingJobThread2` 遍历 `jobs/` 调度重建式任务，`GenTaskThread::Run()` 遍历 `jobs_gen/` 调度生成式任务。两者独立运行，互不干扰 |
| `GenHttpClient` | `spawn Task.exe` 子进程 | 任务执行方式。重建式通过文件 IPC + spawn `MoldAITask.exe` 子进程执行，生成式通过 HttpClient::post/get (薄封装) 提交和轮询远程服务端。GenHttpClient 是 HttpClient 的适配层，不复刻传输逻辑 |
| `jobs_gen/` 目录 | `jobs/` 目录 | 文件系统 IPC 的工作目录。`jobs/` 存重建式 Job 文件（`J_*`），`jobs_gen/` 存生成式 Job 文件（同样 `J_*` 前缀，始终 BIN 加密）。启动时需同样清理残留 `.lock` 文件 |
| `getGenEngineJobQueue()` | `getEngineJobQueue()` | 获取队列根路径。`getEngineJobQueue()` 读注册表 `engine` key，`getGenEngineJobQueue()` 取其父目录 + `/jobs_gen`，无需新注册表项 |

### SDK 与接口层

| 新增 | 对标现有 | 说明 |
|------|---------|------|
| `GenTaskAPI` | 无现成对标 | 前端 SDK。现有系统前端直接操作 BlockObject + Job 文件（耦合高），`GenTaskAPI` 将生成式任务的"提交/查询/下载/回调"封装为静态方法。回调机制采用 `GenTaskAPI` 存储 → `GenTaskThread` 触发的单向依赖 |
| `TriggerTaskComplete/TriggerTaskFailed` | 无现成对标 | GenTaskThread 完成后调用 `GenTaskAPI::Trigger*`，`GenTaskAPI` 内部调用前端注册的回调。避免 `GenTaskAPI`（MoldAIData.dll）反向依赖 `GenTaskThread`（MoldAINode.exe） |

### 修改的现有文件

| 文件 | 对标什么 | 说明 |
|------|---------|------|
| `BlockObject.h/cpp` | 自身 | `Task_Info` 加 `GenTaskOptions gen_options`（对标 `ATOptions at_options`） |
| `Settings.h/cpp` | 自身 | 新增 `getGenEngineJobQueue()` |
| `CallEngine.cpp` | 自身 | MakePath 创建 `jobs_gen/` 目录；main 启动 `GenTaskThread` |
| `App/Engine/CMakeLists.txt` | 自身 | HEADER_LIST 添加 `GenTaskProcess.h`（唯一需改的 CMakeLists） |

---

## Phase 1: 数据结构基础

> 最先做，所有后续 Phase 都依赖这里的结构定义。

### 设计决策说明

**为什么 GenTaskProcess.h 放在 Util/ 而非 Core/?** TaskProcess.h（JobInfo_s / JobFeedBack_s）已在 Util/ 下，GenTaskProcess.h 定义的 GenJobInfo_s 与其同层级——都是 Engine 调度层（MoldAINode.exe）使用的 job 文件读写结构体，不是 Core 层（MoldAIData.dll）的基础数据模型。Core/ 下的结构体（如 BlockObject）被 DLL 和 EXE 共同链接，而 GenJobInfo_s 只被 EXE 使用。

**为什么枚举在 JSON 中存为 int？** 对标 TaskProcess.h 中 `jobsta_e` 的序列化方式：`document.AddMember("Status", rapidjson::Value((int)Status), allocator)`。存 int 而非字符串的好处：改枚举名不影响已持久化的 JSON 文件；服务端 HTTP API 也使用 int 状态码。

**为什么 GenTaskResponse 不需要 JSON 序列化方法？** GenTaskResponse 由 GenHttpClient 回调 lambda 直接从 `QJsonObject` 解析 (通过 `doc.value()`/`doc.contains()`)，不需要 nlohmann 的 free-function 或成员函数序列化。

**为什么引擎不解析生成参数？** 引擎只做三件事：写 job 文件、透传参数给服务端、更新 feedback。前端通过 `GenTaskParams` 结构体组装参数 → 引擎调用 `ToJsonString()` 序列化为 JSON 字符串 → BIN 存储为字符串 (不展开字段) → submit 时原样传给服务端 → 服务端自行解析。引擎不需要为每种参数变化而改代码。

### 1.1 新建 GenTaskProcess.h

- [ ] 创建 `Include/Util/GenTaskProcess.h`

```cpp
// Include/Util/GenTaskProcess.h
// ============================================================================
// @file    GenTaskProcess.h
// @brief   生成式任务的数据结构定义, 对标 TaskProcess.h (重建式任务)
//
// 放在 Util/ 而非 Core/ 的原因: TaskProcess.h (JobInfo_s, JobFeedBack_s) 也在
// Util/ 下, GenJobInfo_s 本质上和 JobInfo_s 是同层级的 job 调度结构体.
//
// 设计要点:
//   - 引擎不解析生成参数 — GenTaskParams 是前端填入的结构体,
//     通过 ToJsonString() 序列化后存入 BIN / 发给服务端, 后端自行解析.
//   - GenTaskStatus 是唯一的枚举 — 引擎只需追踪任务生命周期状态,
//     不需要知道 category/sub_type (这些信息在 GenTaskParams 内部).
//   - GenTaskResponse 由 GenHttpClient 从 QJsonObject 直接解析,
//     不需要独立的 JSON 序列化方法.
// ============================================================================

#pragma once

#include <string>
#include <optional>
#include <thread>
#include <chrono>

#include "Core/GenTaskOptions.h" // GenTaskParams (JSON 序列化) + GenTaskOptions
#include "Core/File.h"            // FopenDenyWriteLockUtf8 / OpenOfstreamUtf8
#include "Core/Logging.h"         // LOGE / LOGI / LOGW
#include "Core/DataStruct.h"      // GenJobInfoData / GenJobFile (Phase 1.2)
#include "Util/TaskProcess.h"     // JobFeedBack_s (GenJobFullInfo_s 持有, 对标 TaskGraph_s)

namespace AI3D {
    namespace CORE {

        // ============================================================================
        // GenTaskStatus — 驱动 job 文件在状态目录间流转的唯一枚举
        //
        // 状态 → 目录映射 (GenTaskThread 执行):
        //   IDLE        → (仅内存状态, 尚未写入任何目录)
        //   PENDING     → jobs_gen/Pending/   (等待 submit)
        //   IN_PROGRESS → jobs_gen/Running/   (已 submit, 轮询中)
        //   COMPLETED   → jobs_gen/Completed/ (完成)
        //   FAILED      → jobs_gen/Failed/    (失败)
        //   CANCELLED   → jobs_gen/Cancelled/ (取消)
        //
        // 状态 → jobsta_e 映射 (UpdateFeedback 中):
        //   IDLE/PENDING → STATUS_PENDDING,  IN_PROGRESS → STATUS_RUNNING
        //   COMPLETED    → STATUS_COMPLETE,  FAILED      → STATUS_FAILED
        //   CANCELLED    → STATUS_CANCELLED
        // ============================================================================
        enum class GenTaskStatus {
            UNKNOWN = -1,   // 未指定 / 非法值, 调用者需过滤
            IDLE,           // 初始 / 网络不通
            PENDING,        // 已写入 jobs_gen/Pending/, 等待 GenTaskThread pick up
            IN_PROGRESS,    // 已 POST submit 到服务端, 正在周期性轮询
            COMPLETED,      // 服务端返回完成
            FAILED,         // 执行失败
            CANCELLED,      // 用户取消
        };

        // ============================================================================
        // GenTaskResponse — 服务端返回的 HTTP 响应体 (DTO, 与服务端 API 契约一一对应)
        //
        // 由 GenHttpClient 回调 lambda 直接从 QJsonObject 解析, 不需要 JSON 序列化方法.
        // optional 字段仅在服务端提供对应值时有值.
        // ============================================================================
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

        // ============================================================================
        // GenJobInfo_s — job 文件顶层结构, 对标 TaskProcess.h 的 JobInfo_s
        //
        // 生命周期: GenTaskAPI::SubmitGenTask 创建 (params 由前端填入) 并写入 Pending/
        //          → GenTaskThread::ProcessPendingJobs 透传 params 给服务端
        //          → ProcessRunningJobs 轮询并更新状态
        //          → 完成后移到 Completed/ 或 Failed/
        //
        // 序列化策略: 文件 I/O 始终走 BIN 加密, 不需要 JSON 序列化.
        //   GenJobInfo_s     — 纯数据 (对标 JobInfo_s), 无序列化方法
        //   GenJobFullInfo_s — 文件级结构体 (对标 JobFullInfo_s), save/load → WriteToBin/LoadFromBin
        //   GenJobInfoData/GenJobFile — DataStruct.h BIN 线格式 (对标 JobInfoData/JobListFile)
        // ============================================================================

        /// @brief 单个生成式任务的纯数据结构 (对标 JobInfo_s)
        struct GenJobInfo_s {
            // --- 客户端标识 ---
            std::string task_uuid;     // 客户端生成的 UUID, 全局唯一, 前端通过此 ID 查询
            std::string engine_id;     // 发起 Engine 的主机名
            std::string user_account;  // 用户账号 (submit 时传给服务端, 积分计费用)

            // --- Block 关联 (定位 feedback 文件用) ---
            std::string project_path;  // 所属项目目录
            std::string block_item;    // 所属 Block 名称 (feedback 路径 = project/block_item/JF_job_name)

            // --- 生成参数 (前端填入 → WriteToJson()/ToJsonString() → HTTP/文件) ---
            GenTaskParams params;       // 生成参数结构体, 自带 JSON 序列化方法

            // --- 运行时状态 (由 GenTaskThread 从 HTTP 响应回填) ---
            GenTaskStatus status = GenTaskStatus::IDLE;
            std::string server_task_id;    // 服务端 triverse_task_uuid (崩溃恢复: 非空 = 已 submit)
            std::string result_url;         // 结果下载链接 (COMPLETED 时填充)
            std::string preview_url;        // 预览图链接
            std::string error_message;      // 详细错误信息 (FAILED 时填充)
            int cost_credits = 0;           // 本次消耗积分
            int points_balance = 0;         // 积分余额
            int query_retry_count = 0;      // 连续轮询失败次数 (>= 5 则标记失败)

            /// @brief 将 HTTP 响应回填到自身 (消除 ProcessRunningJobs 逐字段拷贝)
            ///        optional 字段只在有值时覆盖, 避免空响应冲掉已有数据
            void ApplyResponse(const GenTaskResponse& resp) {
                if (resp.triverse_task_uuid) server_task_id = *resp.triverse_task_uuid;
                if (resp.result_url)        result_url     = *resp.result_url;
                if (resp.preview_url)       preview_url    = *resp.preview_url;
                if (resp.error_message)     error_message  = *resp.error_message;
                status         = resp.status;
                cost_credits   = resp.cost_credits;
                points_balance = resp.points_balance;
            }
        };

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
        // JobFeedBack_s feedback — 对标 TaskGraph_s 持有的 JobFeedBack_s, 内存中持有避免
        // UpdateFeedback 每次 load/save 独立文件, 减少不必要的 I/O。
        // feedback 同时序列化到 GenJobFile BIN (对标 JobListFile::feedBackData), 也独立持久化到 JF_* 文件。
        //
        // save/load — 始终走 BIN 加密 (XOR 0xAB), 无 JSON 分支。
        //   JOB_INFO_USE_BIN 当前恒为 true, JSON 调试路径已废弃。
        // ============================================================================

        struct GenJobFullInfo_s {
            std::string   job_name;  // J_<BlockName>_<timestamp>, 用作文件名, 对标 JobFullInfo_s::JobName
            GenJobInfo_s  job;       // 任务数据, 对标 JobFullInfo_s::tg.job (JobInfo_s)
            JobFeedBack_s feedback;  // 进度反馈, 对标 TaskGraph_s 持有的 JobFeedBack_s, 会序列化到 BIN

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
                d.params_json    = job.params.ToJsonString();
                d.status         = static_cast<int>(job.status);
                d.server_task_id = job.server_task_id;
                d.result_url     = job.result_url;
                d.preview_url    = job.preview_url;
                d.error_message  = job.error_message;
                d.cost_credits   = job.cost_credits;
                d.points_balance = job.points_balance;
                d.query_retry_count = job.query_retry_count;
                // Feedback (对标 JobListFile::feedBackData, 存在 GenJobFile 层面)
                genJobFile.feedBackData.status     = static_cast<int>(feedback.Status);
                genJobFile.feedBackData.percent    = feedback.Percent;
                genJobFile.feedBackData.taskRetVal = feedback.TaskRetVal;
                genJobFile.feedBackData.msg        = feedback.Msg;

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
                job.params         = GenTaskParams::CreateFromJsonString(d.params_json);
                job.status         = static_cast<GenTaskStatus>(d.status);
                job.server_task_id = d.server_task_id;
                job.result_url     = d.result_url;
                job.preview_url    = d.preview_url;
                job.error_message  = d.error_message;
                job.cost_credits   = d.cost_credits;
                job.points_balance = d.points_balance;
                job.query_retry_count = d.query_retry_count;
                // Feedback (对标 JobListFile::feedBackData, 存在 GenJobFile 层面)
                feedback.Status     = static_cast<jobsta_e>(genJobFile.feedBackData.status);
                feedback.Percent    = genJobFile.feedBackData.percent;
                feedback.TaskRetVal = genJobFile.feedBackData.taskRetVal;
                feedback.Msg        = genJobFile.feedBackData.msg;

                return true;
            }

            // ========================================================================
            // save / load — BIN 加密 (对标 JobFullInfo_s::save/load, 但只保留 BIN 路径)
            // ========================================================================

            bool save(const std::string& filePath) const {
                bool result = WriteToBin(filePath);
                if (!result) {
                    LOGE("GenJobFullInfo_s::save: WriteToBin failed: " + filePath);
                }
                return result;
            }

            bool load(const std::string& filePath) {
                bool result = LoadFromBin(filePath);
                if (!result) {
                    LOGE("GenJobFullInfo_s::load: LoadFromBin failed: " + filePath);
                }
                return result;
            }

            // ========================================================================
            // save_with_retry / load_with_retry — deny-write 锁 + 3 次重试
            // 对标 JobFullInfo_s::save / load, 但增加了 deny-write 锁 (参照 JobFeedBack_s 的锁模式)
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

            bool load_with_retry(const std::string& filePath) {
                int retryTimes = 0;
                do {
                    FILE* fpLock = File::FopenDenyWriteLockUtf8(filePath + ".lock");
                    if (fpLock == NULL) {
                        retryTimes++;
                        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                        continue;
                    }
                    bool result = load(filePath);
                    fclose(fpLock);
                    if (result) return true;

                    retryTimes++;
                    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                } while (retryTimes < 3);

                LOGE(std::string("GenJobFullInfo_s::load_with_retry failed after 3 retries: ") + filePath);
                return false;
            }
        };

    } // namespace CORE
} // namespace AI3D
```

### 1.2 修改 DataStruct.h — 增加 GenJobInfoData/GenJobFile + JobInfoData/BLKBinFile 扩展

> `GenJobFullInfo_s::WriteToBin/LoadFromBin` 引用了 `GenJobInfoData`/`GenJobFile`，两者定义在 DataStruct.h 中，对标 `JobInfoData`/`JobListFile` + `FeedBackData`/`FeedBackFile` 的 BIN 序列化模式。`BLKBinFile` 增加 `GenTaskOptions` 序列化字段。

- [ ] `Include/Core/DataStruct.h`：在 `FeedBackFile` 之后、`JobInfoData` 之前增加 `GenJobInfoData` + `GenJobFile`
- [ ] `Include/Core/DataStruct.h`：`GenJobFile` 增加 `FeedBackData feedBackData` (对标 `JobListFile::feedBackData`)
- [ ] `Include/Core/DataStruct.h`：`BLKBinFile` 增加 `gen_block_task_category` + `gen_params_json` + 更新 Serialize/Deserialize

#### GenJobInfoData / GenJobFile — 生成式任务 BIN 序列化 (XOR 0xAB 加密)

> 对标 `FeedBackData`/`FeedBackFile` + `JobInfoData`/`JobListFile`。`params_json` 为 opaque JSON 字符串，前端的完整生成参数原样存入，BIN 格式无需感知参数结构。

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
    std::string params_json;       // opaque JSON 字符串, 前端填入 -> 引擎透传 -> 服务端解析
    int status;
    std::string server_task_id;
    std::string result_url;
    std::string preview_url;
    std::string error_message;
    int cost_credits;
    int points_balance;
    int query_retry_count;
    ByteCrypt byteCrypt;

    GenJobInfoData() {
        task_uuid = "";
        job_name = "";
        engine_id = "";
        user_account = "";
        project_path = "";
        block_item = "";
        params_json = "";
        status = 0;
        server_task_id = "";
        result_url = "";
        preview_url = "";
        error_message = "";
        cost_credits = 0;
        points_balance = 0;
        query_retry_count = 0;
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

        unsigned int params_json_len = params_json.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&params_json_len), sizeof(params_json_len));
        byteCrypt.WriteByteDecrypted(out, params_json.c_str(), params_json_len);

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
        unsigned int params_json_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&params_json_len), sizeof(unsigned int));
        params_json.resize(params_json_len);
        byteCrypt.ReadByteDecrypted(in, &params_json[0], params_json_len);
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
        return true;
    };
};

struct GenJobFile {
    std::string jobName;       // 文件名 (对标 JobListFile::jobName)
    GenJobInfoData genJobInfoData;
    FeedBackData feedBackData;  // 对标 JobListFile::feedBackData
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
        feedBackData.Serialize(out);
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
        feedBackData.Deserialize(in);
        return true;
    };
};
```

#### BLKBinFile — 在现有字段末尾增加

```cpp
// BLKBinFile 中增加:
int    gen_block_task_category = 0;     // 0=重建(默认), 1=生成式
std::string gen_params_json;            // 生成式参数 JSON 字符串

// Serialize 中增加:
byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&gen_block_task_category), sizeof(int));

unsigned int gen_params_json_len = gen_params_json.size();
byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&gen_params_json_len), sizeof(gen_params_json_len));
byteCrypt.WriteByteDecrypted(out, gen_params_json.c_str(), gen_params_json_len);

// Deserialize 中增加:
byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&gen_block_task_category), sizeof(int));

unsigned int gen_params_json_len = 0;
byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&gen_params_json_len), sizeof(unsigned int);
gen_params_json.resize(gen_params_json_len);                           
byteCrypt.ReadByteDecrypted(in, &gen_params_json[0], gen_params_json_len);
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

### 1.3 新增 GenTaskOptions.h + 修改 BlockObject

> 对标 `Include/Core/ATOptions.h`。`ATOptions` 是独立的头文件，然后被 `BlockObject.h` 通过 `#include "Core/ATOptions.h"` 引用，作为 `Task_Info` 的成员 `ATOptions at_options`。`GenTaskOptions` 遵循完全相同的模式。

- [ ] 创建 `Include/Core/GenTaskOptions.h`
- [ ] `Include/Core/BlockObject.h`：`#include "Core/GenTaskOptions.h"` + `Task_Info` 增加 `GenTaskOptions gen_options`
- [ ] `Include/Core/DataStruct.h`：`BLKBinFile` 增加 `gen_block_task_category` + `gen_params_json`（见 Phase 1.2）
- [ ] `Include/Core/BlockObject.cpp`：4 个序列化方法更新

#### GenTaskOptions.h — 对标 ATOptions.h (包含 GenTaskParams)

```cpp
// Include/Core/GenTaskOptions.h
// ============================================================================
// @brief 生成式任务参数结构体 — 对标 ATOptions (Include/Core/ATOptions.h)
//        GenTaskParams  — 生成参数 (prompt/texture_size/...), 自带 JSON 序列化
//        GenTaskOptions — 嵌入 BlockObject::Task_Info (block_task_category + params)
// ============================================================================
#pragma once

#include <string>
#include <Core/json.h>

namespace AI3D {
    namespace CORE {

        // ============================================================================
        // GenTaskSubType — 生成式任务类型 (扁平枚举, 对应 Triverse 10 个端点)
        // ToString() 返回 URL 路径段 (枚举名小写 + _→-), 由 GenHttpClient 拼完整 URL
        // SubTypeFromString() 接受 URL 格式, 反序列化回枚举
        // ============================================================================
        enum class GenTaskSubType {
            UNKNOWN = -1,           // 未指定 / 非法值, 调用者需过滤
            TEXT_TO_MODEL,          // → text-to-model     文字→带纹理模型
            TEXT_TO_MESH,           // → text-to-mesh      文字→纯白模
            IMAGE_TO_MODEL,         // → image-to-model    图片→带纹理模型
            IMAGE_TO_MESH,          // → image-to-mesh     图片→纯白模
            TEXTURE_MODEL,          // → texture-model     图片+模型→纹理
            TEXT_TO_TEXTURE,        // → text-to-texture   文字+模型→纹理
            MODEL_PREVIEW_RENDER,   // → model-preview-render  模型→预览图
            MODEL_REMESH,           // → model-remesh          重网格/减面
            CONVERT_MODEL_FORMAT,   // → convert-model-format  格式转换
            IMAGE_GENERATION,       // → image-generation      文字→图片
        };
        // 枚举 → URL 路径段: 小写 + 下划线换横线
        inline const char* ToString(GenTaskSubType st) {
            switch (st) {
                case GenTaskSubType::TEXT_TO_MODEL:        return "text-to-model";
                case GenTaskSubType::TEXT_TO_MESH:         return "text-to-mesh";
                case GenTaskSubType::IMAGE_TO_MODEL:       return "image-to-model";
                case GenTaskSubType::IMAGE_TO_MESH:        return "image-to-mesh";
                case GenTaskSubType::TEXTURE_MODEL:        return "texture-model";
                case GenTaskSubType::TEXT_TO_TEXTURE:      return "text-to-texture";
                case GenTaskSubType::MODEL_PREVIEW_RENDER: return "model-preview-render";
                case GenTaskSubType::MODEL_REMESH:         return "model-remesh";
                case GenTaskSubType::CONVERT_MODEL_FORMAT: return "convert-model-format";
                case GenTaskSubType::IMAGE_GENERATION:     return "image-generation";
                default: return nullptr;
            }
        }
        inline GenTaskSubType SubTypeFromString(const std::string& s) {
            if (s == "text-to-model")        return GenTaskSubType::TEXT_TO_MODEL;
            if (s == "text-to-mesh")         return GenTaskSubType::TEXT_TO_MESH;
            if (s == "image-to-model")       return GenTaskSubType::IMAGE_TO_MODEL;
            if (s == "image-to-mesh")        return GenTaskSubType::IMAGE_TO_MESH;
            if (s == "texture-model")        return GenTaskSubType::TEXTURE_MODEL;
            if (s == "text-to-texture")      return GenTaskSubType::TEXT_TO_TEXTURE;
            if (s == "model-preview-render") return GenTaskSubType::MODEL_PREVIEW_RENDER;
            if (s == "model-remesh")         return GenTaskSubType::MODEL_REMESH;
            if (s == "convert-model-format") return GenTaskSubType::CONVERT_MODEL_FORMAT;
            if (s == "image-generation")     return GenTaskSubType::IMAGE_GENERATION;
            return GenTaskSubType::UNKNOWN;
        }

        // ============================================================================
        // GenTaskParams — 生成式任务的具体参数
        // 前端填充 → WriteToJson()/ToJsonString() → JSON → HTTP 请求 / BIN 持久化
        // ============================================================================
        struct AI3D_API GenTaskParams
        {
            GenTaskSubType  sub_type = GenTaskSubType::UNKNOWN;     // 任务类型 (调用者需过滤)
            std::string prompt;            // 文本提示词
            std::string negative_prompt;   // 反向提示词
            int         polygon_limit = 0; // 面数限制
            int         texture_size = 0;  // 纹理分辨率
            std::string model_version;     // 模型版本 (字符串: 服务端可能新增版本)
            std::string file_key;          // 已上传文件的 key (前端上传后填入)

            nlohmann::json WriteToJson() const {
                nlohmann::json j;
                if (const char* s = ToString(sub_type)) j["sub_type"] = s;
                if (!prompt.empty())          j["prompt"]          = prompt;
                if (!negative_prompt.empty()) j["negative_prompt"] = negative_prompt;
                if (polygon_limit != 0)       j["polygon_limit"]   = polygon_limit;
                if (texture_size != 0)        j["texture_size"]    = texture_size;
                if (!model_version.empty())   j["model_version"]   = model_version;
                if (!file_key.empty())        j["file_key"]        = file_key;
                return j;
            }

            std::string ToJsonString() const { return WriteToJson().dump(); }

            static GenTaskParams CreateFromJson(const nlohmann::json& j) {
                GenTaskParams p;
                p.sub_type        = SubTypeFromString(j.value("sub_type", ""));
                p.prompt          = j.value("prompt", "");
                p.negative_prompt = j.value("negative_prompt", "");
                p.polygon_limit   = j.value("polygon_limit", 0);
                p.texture_size    = j.value("texture_size", 0);
                p.model_version   = j.value("model_version", "");
                p.file_key        = j.value("file_key", "");
                return p;
            }

            static GenTaskParams CreateFromJsonString(const std::string& jsonStr) {
                if (jsonStr.empty()) return {};
                return CreateFromJson(nlohmann::json::parse(jsonStr));
            }
        };

        // ============================================================================
        // GenTaskOptions — 嵌入 BlockObject::Task_Info 的生成式任务配置
        // 对标 ATOptions at_options
        // ============================================================================
        struct AI3D_API GenTaskOptions
        {
            int           block_task_category = 0;   // 0=重建(默认), 1=生成式
            GenTaskParams gen_params;                // 生成参数 (对标 ATOptions 的各字段)
        };

    } // namespace CORE
} // namespace AI3D
```

#### 修改 BlockObject.h

```cpp
// 在现有 #include 区域增加:
#include "Core/GenTaskOptions.h"

// 在 Task_Info 结构体中 (ATOptions at_options 旁边):
GenTaskOptions gen_options;   // 生成式任务参数 (对标 ATOptions at_options)
```

#### 修改 BlockObject.cpp —— JSON 序列化

`WriteBlockInfoToJson()` 中增加：

```cpp
// 写入 GenTaskOptions
njson["block_task_category"] = block_info_.gen_options.block_task_category;
njson["gen_params"] = block_info_.gen_options.gen_params.WriteToJson();
```

`ReadBlockInfoJson()` 中增加：

```cpp
// 读取 GenTaskOptions
if (njson.contains("block_task_category"))
    block_info_.gen_options.block_task_category = njson["block_task_category"].get<int>();
if (njson.contains("gen_params"))
    block_info_.gen_options.gen_params = GenTaskParams::CreateFromJson(njson["gen_params"]);
```

#### 修改 BlockObject.cpp —— BIN 序列化

`WriteBlockInfoToBin()` 中增加：

```cpp
bLKBinFile.gen_block_task_category = block_info_.gen_options.block_task_category;
bLKBinFile.gen_params_json         = block_info_.gen_options.gen_params.ToJsonString();
```

`ReadBlockInfoBin()` 中增加：

```cpp
block_info_.gen_options.block_task_category = bLKBinFile.gen_block_task_category;
block_info_.gen_options.gen_params          = GenTaskParams::CreateFromJsonString(bLKBinFile.gen_params_json);
```

> **兼容性**: 旧 `.blk` 文件中没有 `block_task_category`/`gen_params` 字段 → JSON 读取走 `if contains` 检查, 字段不存在时 struct 保持默认值；BIN 读取走默认构造。前端判断 `block_task_category == 0` 为重建式（旧 Block 全默认为 0），`== 1` 为生成式。

### 1.4 JobFeedBack_s — 不修改，但增加持有关系

> `JobFeedBack_s`（`FeedBackFile`）是固定格式的进度反馈文件，只有 `Status`/`Percent`/`Msg`/`TaskRetVal` 四个字段。生成式任务直接复用：
> - `Status` → 映射 `GenTaskStatus` → `jobsta_e`
> - `Percent` → 进度百分比
> - `Msg` → 状态描述文字
> - `TaskRetVal` → 0=成功, 非0=失败
>
> **持有关系**: `GenJobFullInfo_s` 新增 `JobFeedBack_s feedback` 成员（对标 `TaskGraph_s` 持有 `JobFeedBack_s`），在调度线程中加载 job 时同步加载 feedback 到内存，`UpdateFeedback()` 直接修改内存中的 `feedback` 成员，调用者负责写回独立 `JF_*` 文件。避免了每次更新 feedback 都要 `load_with_retry` + `save_with_retry` 的额外 I/O。
>
> **结果数据**（`result_url`/`preview_url`/`server_task_id`/`cost_credits`/`error_message`）全部存在 `GenJobInfo_s` 自身中，不做为 feedback 的扩展字段。前端通过 `GenTaskAPI::QueryTaskStatus` 读取 `GenJobInfo_s` 获取结果。

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

`GenJobFullInfo_s::save_with_retry/load_with_retry` 内联在 `GenTaskProcess.h` 头文件中（参照 `JobFeedBack_s` 的 deny-write 锁模式，`JobFullInfo_s` 本身没有 save_with_retry），不需要单独的 `.cpp` 文件，也无需修改 `Src/Util/` 下的 CMakeLists。

---

## Phase 2: 路径配置

> 不依赖 Phase 1，可与 Phase 1 并行开工。

### 设计决策说明

**为什么 jobs_gen/ 使用独立的目录而不是复用 jobs/？** 两个调度线程 `searchPendingJobThread2` 和 `GenTaskThread` 各自独立轮询。如果共用同一个目录，每个线程扫描时都需要过滤不属于自己的 job 文件（重建式 vs 生成式），增加不必要的 I/O 和复杂度。独立目录让两个线程完全解耦，互不干扰。

**为什么 getGenEngineJobQueue() 读注册表而不是新增注册表项？** 现有 `engine` key 指向 `jobs/` 的父目录（如 `C:\...\MoldAI\engine\jobs`），取其父目录 + `/jobs_gen` 即可得到生成式队列路径。不需要新增注册表项意味着不需要改安装脚本、不需要迁移已有用户的注册表，对现有部署零影响。

**为什么不支持优先级子目录？** 重建式有 `High/Normal/Low` 子目录是因为本地算力有限需要排队。生成式任务提交到远程 GPU 集群，服务端自行调度，优先级由服务端 SLA 保证，本地仅负责 submit + 轮询。

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

> 依赖 Phase 1 的数据结构（GenTaskResponse）。

### 设计决策说明

**为什么 GenHttpClient 委托 HttpClient 而不是重新实现？** HttpClient (Src/Util/HttpClient.cpp) 已封装了: QNetworkAccessManager + QEventLoop 同步阻塞、token 读取 (注册表每请求实时读)、calSign 签名算法。GenHttpClient 作为薄适配层，只做 HttpClient 做不到的事: 构造 QMap<QString,QString> (嵌套 JSON 预序列化为字符串值)、GET 请求手动计算 Authorization 头 (HttpClient::get 不带鉴权)、multipart 文件上传 (HttpClient 不支持)、GenTaskResponse 解析。

**为什么 GenTaskParams 通过 ToJsonString() 转为字符串再传？** HttpClient::post() 的 param 参数是 QMap<QString,QString>，内部序列化为扁平 JSON `{"key":"value",...}`。GenTaskParams 本身是带类型的 C++ 结构体，调用 ToJsonString() 得到 `{"prompt":"hello","model_version":"v2",...}` 字符串，作为 QMap 的 `"params"` 值传入。服务端收到后自行 JSON.parse(params)。既保持了 HttpClient 的 QMap 接口不变，又提供了前端类型安全。

**为什么 token 不缓存？** 与 `HttpClient::post()` 的行为对齐——每次请求都从注册表实时读取 token。token 可能被另一个进程 (MoldAIDesktop.exe 登录模块) 刷新，缓存旧 token 会导致 401 无法自愈。

### 3.1 新建 GenHttpClient

- [ ] 创建 `App/Engine/GenHttpClient.h`

```cpp
// App/Engine/GenHttpClient.h
// ============================================================================
// @brief GenTask HTTP 适配层 — 薄封装 HttpClient, 处理 GenTask 专用逻辑
//
// 设计原则: 不复刻 HTTP 传输层。HttpClient (Src/Util/HttpClient.cpp) 已封装了
//   QNetworkAccessManager + QEventLoop + token 读取 + 签名鉴权 + JSON 序列化。
//   GenHttpClient 只做 HttpClient 做不到的事:
//     1. GenTaskParams → ToJsonString() → QMap value (HttpClient 内部 QMap → JSON)
//     2. GET 请求手动计算 Authorization 头 (HttpClient::get 不带鉴权)
//     3. multipart 文件上传 (HttpClient 不支持)
//     4. QJsonObject 响应 → GenTaskResponse 解析
// ============================================================================
#pragma once

#include "Util/GenTaskProcess.h"
#include <QString>
#include <QMap>

namespace AI3D {
    namespace CORE {

        class GenHttpClient
        {
            public:
            /// @brief 从 MoldAIConfig.ini [GenTask] 段读取 ServerUrl / ApiPrefix
            static void Init(const std::string& configPath);

            /// @brief POST /api/v1/tasks/<type> — 提交生成任务(端点由 sub_type 决定)
            ///        如 TEXT_TO_MODEL → POST /api/v1/tasks/text-to-model
            ///        genParams: 生成参数结构体, 通过 ToJsonString() 序列化后作为 QMap value
            static GenTaskResponse SubmitTask(const std::string& task_uuid,
                                              const std::string& user_account,
                                              const GenTaskParams& genParams,
                                              int timeout_ms = 5000,
                                              int max_retries = 3);

            /// @brief GET /api/v1/task/status?task_id=<server_task_id> — 查询任务状态
            ///        通过 headers 参数手动传入 Authorization 头 (HttpClient::get 不带鉴权)
            static GenTaskResponse QueryTaskStatus(const std::string& server_task_id,
                                                   int timeout_ms = 3000,
                                                   int max_retries = 3);

            /// @brief POST /api/v1/task/cancel — 取消任务
            static bool CancelTask(const std::string& server_task_id,
                                   int timeout_ms = 3000,
                                   int max_retries = 3);

            /// @brief POST /api/v1/upload — 上传本地文件, 返回 file_key (失败返回空字符串)
            ///        multipart/form-data, HttpClient 不支持, 自建 QHttpMultiPart
            static std::string UploadFile(const std::string& local_path,
                                          int timeout_ms = 10000,
                                          int max_retries = 2);

            private:
            /// @brief 构建 Authorization 头 (算法与 HttpClient::calSign() 完全一致)
            ///        sign = MD5(moldai:<path>:<timestamp>:<data>:<token>).toBase64()
            ///        每次调用内部实时读取注册表 token
            static QString BuildAuthHeader(const QString& url,
                                           const QString& dataJson = "");

            /// @brief 从注册表读取当前登录用户的 accessToken
            ///        复制自 HttpClient::post() 的 token 读取逻辑
            static QString LoadAccessToken();

            /// @brief 同步 POST multipart/form-data (HttpClient 不支持, 自建实现)
            static QByteArray SyncPostMultipart(const QString& url,
                                                const QString& filePath,
                                                int timeout_ms);

            static QString s_serverUrl;   // MoldAIConfig.ini [GenTask] ServerUrl
            static QString s_apiPrefix;   // MoldAIConfig.ini [GenTask] ApiPrefix
        };

    }} // namespace AI3D::CORE
```

- [ ] 创建 `App/Engine/GenHttpClient.cpp`

```cpp
// App/Engine/GenHttpClient.cpp
// ============================================================================
// @brief GenTask HTTP 适配层实现
//
// 核心思路: 不重复造轮子。所有 JSON POST 委托给 HttpClient::post() (已封装
//   QNetworkAccessManager + QEventLoop + token 读取 + 签名鉴权 + QMap→JSON)。
//   GenHttpClient 只负责:
//     1. 构造 QMap<QString,QString> (嵌套 JSON 预序列化为字符串值)
//     2. GET 请求时手动计算 Authorization 头并入 headers
//     3. multipart 文件上传 (HttpClient 不支持)
//     4. QJsonObject 响应解析为 GenTaskResponse
// ============================================================================

#include "GenHttpClient.h"
#include "Util/HttpClient.h"       // 复用现有 HttpClient::post/get
#include "Util/constant.h"         // SETTINGS_PREFIX / CURRENT_PREFIX / TOKEN_PREFIX
#include "Core/Logging.h"
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
#include <QDateTime>
#include <QCryptographicHash>
#include <thread>
#include <chrono>

namespace AI3D {
    namespace CORE {

        QString GenHttpClient::s_serverUrl = "http://localhost:8080";
        QString GenHttpClient::s_apiPrefix = "/api/v1";

        // ============================================================================
        // Init — 读取 MoldAIConfig.ini [GenTask] 段
        // ============================================================================

        void GenHttpClient::Init(const std::string& configPath)
        {
            QSettings settings(QString::fromStdString(configPath), QSettings::IniFormat);
            settings.beginGroup("GenTask");
            s_serverUrl = settings.value("ServerUrl", s_serverUrl).toString();
            s_apiPrefix = settings.value("ApiPrefix", s_apiPrefix).toString();
            settings.endGroup();
        }

        // ============================================================================
        // LoadAccessToken — 从注册表读取 token (复制自 HttpClient::post())
        // ============================================================================

        QString GenHttpClient::LoadAccessToken()
        {
            QSettings settings(SETTINGS_PREFIX + "\\User", QSettings::NativeFormat);
            QString currentUser = settings.value(CURRENT_PREFIX, "").toString();
            if (currentUser.isEmpty())
                return "";
            QString tokenKey = TOKEN_PREFIX + currentUser;
            return settings.value(tokenKey, "").toString();
        }

        // ============================================================================
        // BuildAuthHeader — 计算签名 (算法与 HttpClient::calSign() 完全一致)
        // ============================================================================

        QString GenHttpClient::BuildAuthHeader(const QString& url,
                                               const QString& dataJson)
        {
            QString path = url;
            path.remove(s_serverUrl);

            QString timestampStr = QString::number(QDateTime::currentDateTime().toSecsSinceEpoch());
            QString accessToken = LoadAccessToken();

            QString finalStr = "moldai:" + path + ":" + timestampStr;
            if (!dataJson.isEmpty())
                finalStr += ":" + dataJson;
            if (!accessToken.isEmpty())
                finalStr += ":" + accessToken;

            QByteArray signBase64 = QCryptographicHash::hash(
                finalStr.toUtf8(), QCryptographicHash::Md5).toBase64();

            QString authHeader = "timestamp:" + timestampStr + ",sign:" + signBase64;
            if (!accessToken.isEmpty())
                authHeader += ",accessToken:" + accessToken;

            return authHeader;
        }

        // ============================================================================
        // SubmitTask — POST /api/v1/tasks/<type> — URL 由 genParams.sub_type 经 ToString() 生成
        // ============================================================================

        GenTaskResponse GenHttpClient::SubmitTask(const std::string& task_uuid,
                                                  const std::string& user_account,
                                                  const GenTaskParams& genParams,
                                                  int timeout_ms,
                                                  int max_retries)
        {
            QString url = s_serverUrl + s_apiPrefix + "/tasks/" + QString::fromUtf8(ToString(genParams.sub_type));

            // 构造扁平 QMap — GenTaskParams 通过 ToJsonString() 序列化后作为字符串值
            QMap<QString, QString> params;
            params["task_id"]      = QString::fromStdString(task_uuid);
            params["user_account"] = QString::fromStdString(user_account);
            params["params"]       = QString::fromStdString(genParams.ToJsonString());

            QMap<QString, QString> headers;

            for (int attempt = 0; attempt <= max_retries; attempt++) {
                GenTaskResponse response;
                bool ok = false;

                HttpClient client(nullptr);
                client.post(url, params, headers, [&](int, int errorCode, QString errorMsg, QJsonObject doc) {
                    if (errorCode == 0) {
                        response.task_id = task_uuid;
                        if (doc.contains("triverse_task_uuid"))
                            response.triverse_task_uuid = doc["triverse_task_uuid"].toString().toStdString();
                        response.status   = static_cast<GenTaskStatus>(doc.value("status").toInt());
                        response.progress = doc.value("progress").toInt();
                        if (doc.contains("result_url"))
                            response.result_url = doc["result_url"].toString().toStdString();
                        if (doc.contains("preview_url"))
                            response.preview_url = doc["preview_url"].toString().toStdString();
                        if (doc.contains("error_message"))
                            response.error_message = doc["error_message"].toString().toStdString();
                        response.cost_credits   = doc.value("cost_credits").toInt();
                        response.points_balance = doc.value("points_balance").toInt();
                        ok = true;
                    } else {
                        response.task_id = task_uuid;
                        response.status = GenTaskStatus::IDLE;
                        response.error_message = errorMsg.toStdString();
                    }
                });

                if (ok && response.status != GenTaskStatus::IDLE)
                    return response;

                if (attempt < max_retries) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
            }

            GenTaskResponse failResp;
            failResp.task_id = task_uuid;
            failResp.status = GenTaskStatus::IDLE;
            failResp.error_message = "submit timeout after " + std::to_string(max_retries + 1) + " attempts";
            return failResp;
        }

        // ============================================================================
        // QueryTaskStatus — GET /api/v1/task/status (委托 HttpClient::get)
        //                   手动计算 Authorization 头 (HttpClient::get 不带鉴权)
        // ============================================================================

        GenTaskResponse GenHttpClient::QueryTaskStatus(const std::string& server_task_id,
                                                       int timeout_ms,
                                                       int max_retries)
        {
            QString url = s_serverUrl + s_apiPrefix + "/task/status?task_id="
                + QString::fromStdString(server_task_id);

            // HttpClient::get 不带鉴权, 手动计算并通过 headers 传入
            QMap<QString, QString> headers;
            headers["Authorization"] = BuildAuthHeader(url, "");

            for (int attempt = 0; attempt <= max_retries; attempt++) {
                GenTaskResponse response;
                bool ok = false;

                HttpClient client(nullptr);
                client.get(url, headers, [&](int, int errorCode, QString errorMsg, QJsonObject doc) {
                    if (errorCode == 0) {
                        response.task_id = doc.value("task_id").toString().toStdString();
                        if (doc.contains("triverse_task_uuid"))
                            response.triverse_task_uuid = doc["triverse_task_uuid"].toString().toStdString();
                        response.status   = static_cast<GenTaskStatus>(doc.value("status").toInt());
                        response.progress = doc.value("progress").toInt();
                        if (doc.contains("result_url"))
                            response.result_url = doc["result_url"].toString().toStdString();
                        if (doc.contains("preview_url"))
                            response.preview_url = doc["preview_url"].toString().toStdString();
                        if (doc.contains("error_message"))
                            response.error_message = doc["error_message"].toString().toStdString();
                        response.cost_credits   = doc.value("cost_credits").toInt();
                        response.points_balance = doc.value("points_balance").toInt();
                        ok = true;
                    } else {
                        response.server_task_id = server_task_id;
                        response.status = GenTaskStatus::IDLE;
                        response.error_message = errorMsg.toStdString();
                    }
                });

                if (ok && response.status != GenTaskStatus::IDLE)
                    return response;

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
        // CancelTask — POST /api/v1/task/cancel (委托 HttpClient::post)
        // ============================================================================

        bool GenHttpClient::CancelTask(const std::string& server_task_id,
                                       int timeout_ms,
                                       int max_retries)
        {
            QString url = s_serverUrl + s_apiPrefix + "/task/cancel";

            QMap<QString, QString> params;
            params["task_id"] = QString::fromStdString(server_task_id);

            QMap<QString, QString> headers;

            for (int attempt = 0; attempt <= max_retries; attempt++) {
                bool success = false;

                HttpClient client(nullptr);
                client.post(url, params, headers, [&](int, int errorCode, QString, QJsonObject doc) {
                    success = (errorCode == 0 && doc.value("success").toBool(false));
                });

                if (success)
                    return true;

                if (attempt < max_retries) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
            }

            return false;
        }

        // ============================================================================
        // SyncPostMultipart — multipart 文件上传 (HttpClient 不支持, 自建实现)
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
            filePart.setHeader(QNetworkRequest::ContentTypeHeader, mimeDb.mimeTypeForFile(fi).name());
            filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                               QString("form-data; name=\"file\"; filename=\"%1\"").arg(fi.fileName()));
            filePart.setBodyDevice(&file);
            file.setParent(multiPart);
            multiPart->append(filePart);

            QNetworkAccessManager manager;
            QNetworkRequest request(QUrl(url));
            request.setTransferTimeout(timeout_ms);
            request.setRawHeader("Authorization", BuildAuthHeader(url, "").toUtf8());

            QNetworkReply* reply = manager.post(request, multiPart);
            multiPart->setParent(reply);

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
        // UploadFile — POST /api/v1/upload (multipart, HttpClient 不支持)
        // ============================================================================

        std::string GenHttpClient::UploadFile(const std::string& local_path,
                                              int timeout_ms,
                                              int max_retries)
        {
            QString url = s_serverUrl + s_apiPrefix + "/upload";

            for (int attempt = 0; attempt <= max_retries; attempt++) {
                QByteArray raw = SyncPostMultipart(url, QString::fromStdString(local_path), timeout_ms);

                if (!raw.isEmpty()) {
                    QJsonObject doc = QJsonDocument::fromJson(raw).object();
                    if (doc.value("errorCode").toInt() == 0) {
                        return doc.value("file_key").toString().toStdString();
                    }
                }

                if (attempt < max_retries) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
            }

            return "";
        }

    }} // namespace AI3D::CORE
```


### 3.2 CMakeLists — 无需修改

`GenHttpClient.h/cpp` 放在 `App/Engine/` 目录下，被现有 `FILE(GLOB *.cpp *.h)` 自动拾取。`Qt6::Network` 已在 `App/Engine/CMakeLists.txt` line 55 链接，无需额外添加。

> 注意: GenHttpClient 使用 `#include "Util/HttpClient.h"` 复用 HttpClient 的 post/get 方法, 因此 `App/Engine/CMakeLists.txt` 需要确保 `Src/Util/HttpClient.cpp` 被链接 (当前已通过 `FILE(GLOB)` 在 `Src/Util/` 下自动拾取, 无需修改)。

### 3.3 手动验证

- [ ] 使用 Mock Server 测试 Submit / Query / Cancel / Upload 四个接口
- [ ] 验证多次重试后超时返回的 GenTaskResponse.status == IDLE

---

## Phase 4: 调度线程

> 依赖 Phase 2（路径可用）和 Phase 3（HTTP 客户端可用）。

### 设计决策说明

**为什么 ProcessPendingJobs 不解析 GenTaskParams？** 引擎只透传参数，上传、解析、校验全由前端和服务端负责。前端通过 GenTaskParams 结构体填入参数 (包括 file_key)；引擎调用 GenHttpClient::SubmitTask 时自动序列化为 JSON；服务端收到 submit 后解析 params 并调度 GPU。引擎在这一层是纯粹的 "消息代理"。

**为什么崩溃恢复路径是检查 server_task_id 而非额外标记？** submit 成功回包后立即 `save_with_retry` 写入 server_task_id。如果 Node 在此写入与文件移动到 Running 之间的窗口期崩溃，重启后 ProcessPendingJobs 发现 job 文件仍在 Pending/ 但 server_task_id 非空，直接跳过 HTTP submit 移到 Running。

**为什么连续 5 次超时才标记 FAILED 而非立即失败？** 5 次 * 2s 间隔 = 10s 容忍窗口，足够覆盖暂时的网络中断或服务端重启。

**为什么 SearchUnnormalRunningJob 要单独线程而非嵌入 ProcessRunningJobs？** 职责分离。ProcessRunningJobs 每 2s 一次处理正常轮询（热路径），SearchUnnormalRunningJob 每 30s 一次处理异常兜底（冷路径）。

### 4.1 新建 GenTaskThread

- [ ] 创建 `App/Engine/GenTaskThread.h`

```cpp
// App/Engine/GenTaskThread.h
// ============================================================================
// @brief 生成式任务调度线程 (对标 searchPendingJobThread2)
//        在独立 std::thread 中运行, 每 2s 轮询 jobs_gen/ 目录
// ============================================================================
#ifndef _AI3D_ENGINE_GEN_TASK_THREAD_H_
#define _AI3D_ENGINE_GEN_TASK_THREAD_H_

#include <string>

namespace AI3D {
    namespace CORE {

        class GenTaskThread
        {
            public:
            /// @brief 主循环 — 对标 searchPendingJobThread2()
            static void Run();

            /// @brief 取消任务 — 遍历 jobs_gen/ 找 task_uuid → HTTP cancel → 移 Cancelled/
            static bool CancelGenTask(const std::string& task_uuid);

            /// @brief 异常任务扫描 — 对标 searchUnnormaldRunningJobThread
            static void SearchUnnormalRunningJob();

            private:
            static void ProcessPendingJobs();
            static void ProcessRunningJobs();
            static void UpdateFeedback(GenJobFullInfo_s& info);  // 非 const: 修改 info.feedback
            static void MoveJobFile(const std::string& from, const std::string& toDir);
        };

    }} // namespace AI3D::CORE

#endif

// GenTaskThread.cpp 实现要点 (详见 Phase 4.1 完整代码):
//   ProcessPendingJobs():
//     1. 遍历 jobs_gen/Pending/J_*
//     2. load_with_retry → 检查 server_task_id 是否已存在 (崩溃恢复)
//        - 已存在: 直接 MoveJobFile → Running/ (跳过重复 submit)
//        - 不存在: GenHttpClient::SubmitTask(task_uuid, user_account, job.params)
//                   注意: GenTaskParams 通过 ToJsonString() 序列化后发给服务端
//     3. submit 成功 → save server_task_id → MoveJobFile → Running/
//
//   ProcessRunningJobs():
//     1. 遍历 jobs_gen/Running/J_*
//     2. GenHttpClient::QueryTaskStatus(server_task_id)
//     3. 根据 status 处理: COMPLETED/FAILED → MoveJobFile → 终态目录
//        IN_PROGRESS → 更新 feedback Percent
//     4. 连续 5 次超时 → 标记 FAILED
//
//   UpdateFeedback():
//     直接操作 info.feedback (内存), 调用者负责 load/save 独立 JF_* 文件
//     GenTaskStatus → jobsta_e 映射 (与之前一致)

```

---

### 4.1 完整实现代码

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

        /// @brief 更新内存中的 JobFeedBack_s — 仅修改 Status/Percent/Msg/TaskRetVal
        ///        结果数据 (result_url 等) 存在 GenJobInfo_s 自身中, 不写入 feedback
        ///        调用者负责在 UpdateFeedback 之后 save feedback 到独立文件
        static void UpdateFeedback(GenJobFullInfo_s& info) {
            GenJobInfo_s& job = info.job;
            JobFeedBack_s& fb = info.feedback;

            // GenTaskStatus → jobsta_e 映射 (覆盖已有值, 对标 TaskGraph_s 的同步更新)
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
                GenJobFullInfo_s info;
                if (!info.load_with_retry(filePathStr)) {
                    LOGE("ProcessPendingJobs: failed to load: " + filePathStr);
                    continue;
                }
                GenJobInfo_s& job = info.job;

                // 1.5. 加载 feedback 到内存 (独立 JF_* 文件, 对标 TaskGraph_s 持有 JobFeedBack_s)
                std::string fbPath = BuildFeedbackPath(info);
                info.feedback.load_with_retry(fbPath, false);

                // 2. 崩溃恢复: 已有 server_task_id 则直接移到 Running
                if (!job.server_task_id.empty()) {
                    LOGI("Crash recovery: " + job.task_uuid + " already submitted, moving to Running");
                    MoveJobFile(filePathStr, genRunningJobPath);
                    continue;
                }

                // 3. HTTP POST submit — GenTaskParams 自动序列化为 JSON
                //    前端已处理文件上传 (如有), engine 只负责透传参数
                GenTaskResponse resp = GenHttpClient::SubmitTask(
                    job.task_uuid, job.user_account, job.params);

                // 4. 处理响应
                if (resp.status == GenTaskStatus::IDLE && resp.error_message.has_value()) {
                    // 网络超时 → 不移动文件, 下轮重试
                    LOGW("SubmitTask network timeout for: " + job.task_uuid);
                    continue;
                }

                if (resp.status == GenTaskStatus::FAILED || resp.status == GenTaskStatus::CANCELLED) {
                    // 服务端拒绝
                    job.ApplyResponse(resp);
                    UpdateFeedback(info);
                    info.feedback.save_with_retry(fbPath, false);
                    info.save_with_retry(filePathStr);
                    MoveJobFile(filePathStr, genFailedJobPath);
                    continue;
                }

                // 5. 提交成功 → 回填 server_task_id → 写入 → 移到 Running
                job.ApplyResponse(resp);
                job.status = GenTaskStatus::PENDING;  // submit 成功后进入 PENDING 等待首次 query
                info.save_with_retry(filePathStr);
                UpdateFeedback(info);
                info.feedback.save_with_retry(fbPath, false);
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
                GenJobFullInfo_s info;
                if (!info.load_with_retry(filePathStr))
                    continue;
                GenJobInfo_s& job = info.job;
                if (job.server_task_id.empty()) {
                    continue;
                }

                // 1.5. 加载 feedback 到内存
                std::string fbPath = BuildFeedbackPath(info);
                info.feedback.load_with_retry(fbPath, false);

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
                        info.feedback.save_with_retry(fbPath, false);
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
                        job.ApplyResponse(resp);
                        info.save_with_retry(filePathStr);
                        UpdateFeedback(info);
                        info.feedback.save_with_retry(fbPath, false);
                        MoveJobFile(filePathStr, genCompletedJobPath);
                        LOGI("Completed: " + job.task_uuid);
                        GenTaskAPI::TriggerTaskComplete(job.task_uuid, job.result_url);
                        break;
                    }

                    case GenTaskStatus::FAILED: {
                        job.ApplyResponse(resp);
                        info.save_with_retry(filePathStr);
                        UpdateFeedback(info);
                        info.feedback.save_with_retry(fbPath, false);
                        MoveJobFile(filePathStr, genFailedJobPath);
                        LOGE("Failed: " + job.task_uuid);
                        GenTaskAPI::TriggerTaskFailed(job.task_uuid, job.error_message);
                        break;
                    }

                    case GenTaskStatus::IN_PROGRESS: {
                        // 进度更新: 保存状态 + 写 feedback
                        job.status = GenTaskStatus::IN_PROGRESS;
                        info.save_with_retry(filePathStr);
                        UpdateFeedback(info);
                        info.feedback.save_with_retry(fbPath, false);
                        break;
                    }

                    case GenTaskStatus::CANCELLED: {
                        // 服务端返回取消 (用户可能通过其他渠道取消)
                        job.status = GenTaskStatus::CANCELLED;
                        if (resp.error_message.has_value())
                            job.error_message = resp.error_message.value();
                        info.save_with_retry(filePathStr);
                        UpdateFeedback(info);
                        info.feedback.save_with_retry(fbPath, false);
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

                    GenJobFullInfo_s info;
                    if (!info.load_with_retry(filePathStr))
                        continue;
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

                    // 2. 加载 feedback → 更新状态 → 保存
                    std::string fbPath = BuildFeedbackPath(info);
                    info.feedback.load_with_retry(fbPath, false);
                    job.status = GenTaskStatus::CANCELLED;
                    info.save_with_retry(filePathStr);
                    UpdateFeedback(info);
                    info.feedback.save_with_retry(fbPath, false);

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
                GenJobFullInfo_s info;
                if (!info.load_with_retry(filePathStr)) {
                    // 损坏文件, 直接删除
                    QFile::remove(fi.absoluteFilePath());
                    continue;
                }

                LOGE("UnnormalRunning: " + info.job.task_uuid
                     + " stuck for " + std::to_string(secsSinceMod / 3600) + "h, moving to Failed");

                info.job.status = GenTaskStatus::FAILED;
                info.job.error_message = "task stuck in Running for over 24h";
                std::string fbPath = BuildFeedbackPath(info);
                info.feedback.load_with_retry(fbPath, false);
                info.save_with_retry(filePathStr);
                UpdateFeedback(info);
                info.feedback.save_with_retry(fbPath, false);
                MoveJobFile(filePathStr, genFailedJobPath);
                GenTaskAPI::TriggerTaskFailed(info.job.task_uuid, info.job.error_message);
                continue;
            }

            // 2. 超过 1h 无更新 且无 server_task_id → submit 阶段残留, 移回 Pending 重试
            if (secsSinceMod > 3600) {
                GenJobFullInfo_s info;
                if (!info.load_with_retry(filePathStr))
                    continue;
                if (info.job.server_task_id.empty()) {
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

### 设计决策说明

**为什么 GenTaskAPI 不直接调用 GenHttpClient？** GenTaskAPI 编译进 MoldAIData.dll（Src/Core/），GenHttpClient 编译进 MoldAINode.exe（App/Engine/）。DLL 不能反向依赖 EXE 中的符号。因此 GenTaskAPI 只做文件系统操作（写 job 文件到 Pending/、读 feedback 文件），HTTP 通信完全由 GenTaskThread（在 EXE 侧）负责。这是进程内分层而非进程间通信——两者编译进同一个 EXE，但通过 DLL 边界隔离。

**为什么使用回调机制而非信号槽？** 回调机制的单向依赖（GenTaskAPI 存储 → GenTaskThread 触发）确保 DLL 不依赖 EXE。信号槽需要 QObject 派生和 moc 预处理，在这个跨 DLL/EXE 边界的场景中增加了不必要的复杂度。简单的 `std::function` 回调足够：GenTaskAPI 暴露 `SetTaskCompleteCallback` / `SetTaskFailedCallback` 注册入口，GenTaskThread 完成后调用 `TriggerTaskComplete` / `TriggerTaskFailed` 触发。

**为什么 DownloadResult 在 GenTaskAPI 中直接做 HTTP GET？** DownloadResult 下载的是服务端返回的 result_url（可能是 CDN URL），不经过 Triverse API 网关，不需要签名鉴权。因此它可以简单地用 QNetworkAccessManager 直接 GET，不需要通过 GenHttpClient。这也避免了 DownloadResult 依赖 GenHttpClient 的编译问题。

**为什么 QueryTaskStatus 返回的是 feedback 的状态而非 GenJobInfo_s 的状态？** feedback 文件（JobFeedBack_s）是 MoldAI 通用的进度反馈机制，前端已有的进度轮询逻辑都基于 feedback。生成式复用这个机制：GenTaskThread 通过 UpdateFeedback() 将 GenTaskStatus 映射为 jobsta_e 写入 feedback，前端无需感知 GenTaskStatus 枚举。

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
            ///        从 blockInfo.gen_options 读取 block_task_category 和 gen_params (GenTaskParams)
            static SubmitResult SubmitGenTask(
                const AI3D::CORE::BlockObject::Task_Info& blockInfo,
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
        //   1. 校验 blockInfo.gen_options.block_task_category == 1 (必须是生成式 Block)
        //   2. 从 blockInfo.gen_options.gen_params 读取生成参数 (GenTaskParams 结构体)
        //   3. 构造 GenJobInfo_s (task_uuid = QUuid, job_name = BlockName + 时间戳)
        //   4. 创建结果目录 project/BlockName/job_name/
        //   5. 写 job JSON 到 jobs_gen/Pending/
        //   6. 创建初始 JobFeedBack_s
        // ============================================================================

        GenTaskAPI::SubmitResult GenTaskAPI::SubmitGenTask(
            const BlockObject::Task_Info& blockInfo,
            const std::string& user_account)
        {
            SubmitResult result;

            // 1. 校验 Block 类型 (从 gen_options 读取, 对标 at_options 的用法)
            if (blockInfo.gen_options.block_task_category != 1) {
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
            job.params       = blockInfo.gen_options.gen_params;   // 从 gen_options 读取 GenTaskParams
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
            std::string jobFilePath = pendingPath.toStdString() + fullInfo.job_name + ".bin";
            if (!fullInfo.save_with_retry(jobFilePath)) {
                result.success   = false;
                result.error_msg = "Failed to write job file: " + jobFilePath;
                return result;
            }

            // 6. 创建初始 feedback (对标 CallEngine.cpp 中 MAKE_FEEDBAK_BIN/JSON_FILE 宏)
            //    GenJobFullInfo_s 持有 JobFeedBack_s (对标 TaskGraph_s), 直接在内存中操作
            std::string feedbackBase = blockInfo.projectfile_ + "/"
                + blockInfo.blockName + "/JF_"
                + fullInfo.job_name;
            std::string feedbackPath = feedbackBase
                + (JOB_FEEDBACK_USE_BIN ? BINFILE_POSTFIX : JSONFILE_POSTFIX);

            fullInfo.feedback.Status  = jobsta_e::STATUS_PENDDING;
            fullInfo.feedback.Percent = 0.0f;
            fullInfo.feedback.save_with_retry(feedbackPath, false);

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
                    GenJobFullInfo_s info;
                    if (!info.load_with_retry(it.filePath().toStdString()))
                        continue;
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

    // 1. 构造一个生成式 Block 的 Task_Info (对标 AT 的 at_options 用法)
    BlockObject::Task_Info blockInfo;
    blockInfo.gen_options.block_task_category = 1;  // 生成式
    blockInfo.gen_options.gen_params.sub_type     = GenTaskSubType::TEXT_TO_MODEL;
    blockInfo.gen_options.gen_params.prompt       = "a cute cat figurine";
    blockInfo.gen_options.gen_params.polygon_limit = 50000;
    blockInfo.gen_options.gen_params.texture_size  = 1024;
    blockInfo.blockName    = "TestBlock_Gen";
    blockInfo.projectfile_ = "C:/Users/Test/AppData/Local/MoldAI/TestProject";

    // 2. 设置回调
    GenTaskAPI::SetTaskCompleteCallback([](const std::string& uuid, const std::string& url) {
        std::cout << "[COMPLETED] task_uuid=" << uuid << " result_url=" << url << std::endl;
    });
    GenTaskAPI::SetTaskFailedCallback([](const std::string& uuid, const std::string& err) {
        std::cout << "[FAILED] task_uuid=" << uuid << " error=" << err << std::endl;
    });

    // 3. 提交任务 (参数已在 blockInfo.gen_options 中)
    GenTaskAPI::SubmitResult result = GenTaskAPI::SubmitGenTask(
        blockInfo, "testuser@example.com");

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
    // Step 5: 检查旧 Block (gen_options.block_task_category == 0) 的序列化:
    //   - WriteBlockInfoToJson → gen_options.block_task_category: 0
    //   - ReadBlockInfoJson    → gen_options 正确读取为默认值
    //   - 旧 .blk 文件 (没有 gen_options 字段) → 读取后 gen_options 为默认构造
}
```

- [ ] **验证点**: 两个线程操作不同目录，互不干扰
- [ ] **验证点**: 重建式任务行为完全不变
- [ ] **验证点**: 旧 Block 文件（无 `gen_options` 字段）读取后 `gen_options` 为默认构造 (block_task_category=0, gen_params 为空)

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

@app.route('/api/v1/tasks/<task_type>', methods=['POST'])
def submit(task_type):
    # task_type 为 URL 路径段 (如 text-to-model), 映射到 GenTaskSubType 枚举
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

### 新建（8 个，除 GenTaskProcess.h 外均被 GLOB 自动拾取）

| 文件 | 位置 | CMake | 说明 |
|------|------|-------|------|
| `Include/Util/GenTaskProcess.h` | Util/ | 需显式添加 | 对标 TaskProcess.h，`GenJobInfo_s` + 枚举 + API 类型 |
| `Include/Core/GenTaskOptions.h` | Core/ | GLOB 自动拾取 | 对标 ATOptions.h，`GenTaskParams` + `GenTaskOptions` |
| `App/Engine/GenHttpClient.h` | Engine/ | GLOB 自动拾取 | 同步 HTTP 客户端 |
| `App/Engine/GenHttpClient.cpp` | Engine/ | GLOB 自动拾取 | |
| `App/Engine/GenTaskThread.h` | Engine/ | GLOB 自动拾取 | GenTaskThread 调度线程 |
| `App/Engine/GenTaskThread.cpp` | Engine/ | GLOB 自动拾取 | |
| `Include/Core/GenTaskAPI.h` | Core/ | GLOB 自动拾取 | 前端 SDK 接口 |
| `Src/Core/GenTaskAPI.cpp` | Core/ | GLOB 自动拾取 | 编译进 MoldAIData.dll |

### 修改（7 个）

| 文件 | Phase | 改动 |
|------|-------|------|
| `Include/Core/DataStruct.h` | P1 | 新增 `GenJobInfoData`/`GenJobFile` (含 `FeedBackData feedBackData`)；`BLKBinFile` 增加 `gen_block_task_category` + `gen_params_json` |
| `Include/Core/BlockObject.h` | P1 | `Task_Info` 增加 `GenTaskOptions gen_options` (对标 `ATOptions at_options`) |
| `Include/Core/BlockObject.cpp` | P1 | 4 个序列化方法增加 `gen_options` 读写 |
| `Include/Util/Settings.h` | P2 | 增加 `getGenEngineJobQueue()` |
| `Src/Util/Settings.cpp` | P2 | 实现 `getGenEngineJobQueue()` |
| `App/Engine/CallEngine.cpp` | P2+P4 | MakePath 创建 `jobs_gen/` 目录; `doCleanupJobLockOnceWhileEngineStart` 清理 `jobs_gen/` 残留锁; main 启动 GenTaskThread |
| `App/Engine/CMakeLists.txt` | P1 | HEADER_LIST 显式添加 `GenTaskProcess.h` (唯一需改的 CMakeLists) |

### 不动

`JobFeedBack_s` 不扩展 — 生成式复用 Status/Percent/Msg/TaskRetVal 做进度反馈，结果数据存 GenJobInfo_s。`GenJobFullInfo_s` 新增 `JobFeedBack_s feedback` 成员（对标 TaskGraph_s 持有 JobFeedBack_s），序列化到 BIN（对标 JobListFile::feedBackData）并独立持久化到 JF_* 文件。`TaskGraph_s`、`Task_s`、`ATTaskInfo`、`ExecTaskFileV2`、`GetPendingJob`、`Src/Core/CMakeLists.txt` — 全部不动。
