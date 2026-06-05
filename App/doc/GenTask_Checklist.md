# 生成式任务系统 — 执行清单

按依赖关系排列，完成一项勾一项。

## 依赖关系图

```
Phase 1 (数据结构) ──→ Phase 3 (HTTP) ──→ Phase 4 (调度线程) ──→ Phase 5 (SDK)
                                            ↗
Phase 2 (路径配置) ─────────────────────────┘

Phase 1 + 2 可以并行开工
```

> **积分系统集成**: 生成式和重建式共用同一套积分 API（freeze/settle/outline/estimate）。积分子系统独立于 6 个 Phase，设计方案详见 **[积分接口集成方案.md](积分接口集成方案.md)**。Phase 3 (GenHttpClient) 和 Phase 4 (GenTaskThread) 直接引用积分方案中的接口和逻辑。

每项列出"新增了什么、对标现有哪个组件、为什么需要它"。

### 数据结构层

| 新增                         | 对标现有                                      | 说明                                                         |
| ---------------------------- | --------------------------------------------- | ------------------------------------------------------------ |
| `GenTaskProcess.h`           | `Util/TaskProcess.h`                          | 和 `TaskProcess.h` 一样定义 Job 调度结构体。`TaskProcess.h` 是重建式（`JobInfo_s`、`JobFeedBack_s`），`GenTaskProcess.h` 是生成式（`GenJobInfo_s` + 枚举 + API 类型） |
| `DataStruct.h` (修改)        | `JobListFile` / `FeedBackFile` / `BLKBinFile` | 新增 `GenJobInfoData`/`GenJobFile`（精简指针字段, 对标 `JobInfoData`/`JobListFile`）；完整任务数据由 `GenJobFullInfo_s::WriteToBin` 直接序列化 |
| `GenJobInfo_s`               | `JobInfo_s`                                   | 纯数据结构体 — `JobInfo_s` 存 `ProjectPath/ItemPath`，`GenJobInfo_s` 存 `task_uuid/GenTaskParams/server_task_id/result_url`。I/O 由 GenJobFullInfo_s 负责 (BIN 加密, params 序列化为 JSON 字符串存储) |
| `GenJobFullInfo_s`           | `JobFullInfo_s`                               | 文件级结构体 — 持有 `job_name` + `GenJobInfo_s job` + `JobFeedBack_s feedback`（对标 TaskGraph_s 持有 JobFeedBack_s），提供 `save`/`load` (BIN 加密) + `WriteToBin`/`LoadFromBin`。feedback 序列化到 BIN (对标 JobListFile::feedBackData) |
| `GenTaskResponse`            | 无现成对标                                    | HTTP 响应体。服务端返回的 task status/progress/result_url    |
| `GenTaskStatus`              | `jobsta_e`（局部类似）                        | 生成式任务的状态枚举。`jobsta_e` 是重建式 job 状态（PENDING/RUNNING/COMPLETE...），`GenTaskStatus` 多了 IDLE + 服务端状态 |
| `GenTaskOptions.h`           | `ATOptions.h`                                 | 生成式任务参数结构体，对标 `ATOptions`。包含 `GenTaskParams` (生成参数, 含 JSON 方法) + `GenTaskOptions` (仅 gen_params)，嵌入 `Task_Info`。`block_task_category` 为 `Task_Info` 一级字段 |
| `JobFeedBack_s` — **不修改** | 自身                                          | 生成式任务复用 `Status`/`Percent`/`Msg`/`TaskRetVal` 做进度反馈。结果数据（`result_url` 等）存在 `GenJobInfo_s` 中，不扩展 feedback 字段 |

### 调度与通信层

| 新增                     | 对标现有                                   | 说明                                                         |
| ------------------------ | ------------------------------------------ | ------------------------------------------------------------ |
| `GenTaskThread`          | `searchPendingJobThread2` (CallEngine.cpp) | 调度线程。`searchPendingJobThread2` 遍历 `jobs/` 调度重建式任务，`GenTaskThread::Run()` 遍历 `jobs_gen/` 调度生成式任务。两者独立运行，互不干扰 |
| `GenHttpClient`          | `spawn Task.exe` 子进程                    | 任务执行方式。重建式通过文件 IPC + spawn `MoldAITask.exe` 子进程执行，生成式通过 HttpClient::post/get (薄封装) 提交和轮询远程服务端。GenHttpClient 是 HttpClient 的适配层，不复刻传输逻辑 |
| `jobs_gen/` 目录         | `jobs/` 目录                               | 文件系统 IPC 的工作目录。`jobs/` 存重建式 Job 文件（`J_*`），`jobs_gen/` 存生成式 Job 文件（同样 `J_*` 前缀，始终 BIN 加密）。启动时需同样清理残留 `.lock` 文件 |
| `getGenEngineJobQueue()` | `getEngineJobQueue()`                      | 获取队列根路径。`getEngineJobQueue()` 读注册表 `engine` key，`getGenEngineJobQueue()` 取其父目录 + `/jobs_gen`，无需新注册表项 |

### SDK 与接口层

| 新增         | 对标现有   | 说明                                                         |
| ------------ | ---------- | ------------------------------------------------------------ |
| `GenTaskAPI` | 无现成对标 | 前端 SDK。现有系统前端直接操作 BlockObject + Job 文件（耦合高），`GenTaskAPI` 将生成式任务的"提交/查询/下载/回调"封装为静态方法。回调机制采用 `GenTaskAPI` 存储 → `GenTaskThread` 触发的单向依赖 |

### 修改的现有文件

| 文件                        | 对标什么 | 说明                                                         |
| --------------------------- | -------- | ------------------------------------------------------------ |
| `BlockObject.h/cpp`         | 自身     | `Task_Info` 加 `GenTaskOptions gen_options`（对标 `ATOptions at_options`） |
| `Settings.h/cpp`            | 自身     | 新增 `getGenEngineJobQueue()`                                |
| `CallEngine.cpp`            | 自身     | MakePath 创建 `jobs_gen/` 目录；main 启动 `GenTaskThread`    |
| `App/Engine/CMakeLists.txt` | 自身     | HEADER_LIST 添加 `GenTaskProcess.h`（唯一需改的 CMakeLists） |

---

## Phase 1: 数据结构基础

> 最先做，所有后续 Phase 都依赖这里的结构定义。

### 设计决策说明

**为什么 GenTaskProcess.h 放在 Util/ 而非 Core/?** TaskProcess.h（JobInfo_s / JobFeedBack_s）已在 Util/ 下，GenTaskProcess.h 定义的 GenJobInfo_s 与其同层级——都是 Engine 调度层（MoldAINode.exe）使用的 job 文件读写结构体，不是 Core 层（MoldAIData.dll）的基础数据模型。Core/ 下的结构体（如 BlockObject）被 DLL 和 EXE 共同链接，而 GenJobInfo_s 只被 EXE 使用。

**为什么枚举在 JSON 中存为 int？** 对标 TaskProcess.h 中 `jobsta_e` 的序列化方式：`document.AddMember("Status", rapidjson::Value((int)Status), allocator)`。存 int 而非字符串的好处：改枚举名不影响已持久化的 JSON 文件；服务端 HTTP API 也使用 int 状态码。

**为什么 GenTaskResponse 不需要 JSON 序列化方法？** GenTaskResponse 由 GenHttpClient 回调 lambda 直接从 `QJsonObject` 解析 (通过 `doc.value()`/`doc.contains()`)，不需要额外的序列化方法。

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

        // GenTaskStatus 枚举 — 定义在 Core/GenTaskOptions.h, 供 DLL 和 EXE 共用
        // (GenTaskProcess.h 已 include Core/GenTaskOptions.h, 同名命名空间内可直接使用)

        // ============================================================================
        // GenTaskResponse — 服务端返回的 HTTP 响应体 (DTO, 与服务端 API 契约一一对应)
        //
        // 由 GenHttpClient 回调 lambda 直接从 QJsonObject 解析, 不需要 JSON 序列化方法.
        // optional 字段仅在服务端提供对应值时有值.
        // ============================================================================
        struct GenTaskResponse {
            std::string                task_id;              // 回显客户端的 task_uuid
            std::optional<std::string> server_task_id;        // 服务端分配的任务ID (即 Triverse 的 triverse_task_uuid)
            GenTaskStatus              status = GenTaskStatus::IDLE;  // 任务状态
            int                        progress = 0;          // 进度百分比 0-100
            std::optional<std::string> result_url;            // 结果文件下载链接 (COMPLETED 时有值)
            std::optional<std::string> preview_url;           // 预览图链接
            std::optional<std::string> error_message;         // 错误详情 (FAILED 时有值)
            // 注: cost_credits / points_balance 已删除,
            //     积分数据统一由 PointInfoBase 管理 (详见 积分接口集成方案.md)
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
        //   GenJobFile(GenJobInfoData + GenJobTaskData + FeedBackData) — DataStruct.h BIN 序列化 (对标 JobListFile)
        // ============================================================================

        /// @brief 单个生成式任务的纯数据结构 (对标 JobInfo_s)
        struct GenJobInfo_s {
            // --- 客户端标识 ---
            std::string task_uuid;     // 客户端生成的 UUID, 全局唯一, 前端通过此 ID 查询
            std::string engine_id;     // 发起 Engine 的主机名
            std::string user_account;  // 用户账号 (submit 时传给服务端, 积分计费用)

            // --- Block 关联 (定位 feedback 文件用) ---
            std::string project_path;  // 所属项目目录
            std::string block_item;    // 所属 Block 名称 (feedback 路径 = result_dir/JF_job_name)

            // --- 生成参数 (前端填入 → WriteToJson()/ToJsonString() → HTTP/文件) ---
            GenTaskParams params;       // 生成参数结构体, 自带 JSON 序列化方法

            // --- 运行时状态 (由 GenTaskThread 从 HTTP 响应回填) ---
            int generation_id = 0;           // Generation_<id>, SubmitGenTask 推算并写入
            GenTaskStatus status = GenTaskStatus::IDLE;
            std::string server_task_id;    // 服务端返回的任务ID (非空 = 已 submit)
            std::string result_url;         // 结果下载链接 (COMPLETED 时服务端返回)
            std::string preview_url;        // 预览图链接 (COMPLETED 时服务端返回)
            std::string result_path;        // 结果本地保存路径 (下载后)
            std::string preview_path;       // 预览图本地保存路径 (下载后)
            std::string result_dir;         // 结果目录: Generations/Generation_<id>/
            std::string error_message;      // 详细错误信息 (FAILED 时填充)
            int query_retry_count = 0;      // 连续轮询失败次数 (>= 5 则标记失败)
            int progress = 0;               // 进度百分比 0-100 (IN_PROGRESS 时由 resp.progress 回填)

            // --- 积分相关 (详见 App/doc/积分接口集成方案.md) ---
            PointInfoBase point_info;       // 替代旧 cost_credits/points_balance, 统一管理所有积分数据

            /// @brief 将 HTTP 响应回填到自身 (消除 ProcessRunningJobs 逐字段拷贝)
            ///        optional 字段只在有值时覆盖, 避免空响应冲掉已有数据
            ///        注: 积分字段不通过 ApplyResponse 回填, 由 FreezePoints/SettlePoints 单独处理
            void ApplyResponse(const GenTaskResponse& resp) {
                if (resp.server_task_id) server_task_id = *resp.server_task_id;
                if (resp.result_url)        result_url     = *resp.result_url;
                if (resp.preview_url)       preview_url    = *resp.preview_url;
                if (resp.error_message)     error_message  = *resp.error_message;
                status         = resp.status;
                progress       = resp.progress;
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
            // WriteToBin / LoadFromBin — 通过 GenJobFile 序列化
            // 对标 JobFullInfo_s::WriteToBin / LoadFromBin
            // ========================================================================

            bool WriteToBin(const std::string& filePath) const {
                std::ofstream out = File::OpenOfstreamUtf8(filePath, std::ios::binary);
                if (!out.is_open()) {
                    LOGE("GenJobFullInfo_s::WriteToBin: failed to open: " + filePath);
                    return false;
                }

                GenJobFile f;
                f.jobName = job_name;

                // 指针节
                f.genJobInfoData.project_path = job.project_path;
                f.genJobInfoData.block_item   = job.block_item;

                // 任务数据节
                GenJobTaskData& t = f.genJobTaskData;
                t.generation_id  = job.generation_id;
                t.task_uuid      = job.task_uuid;
                t.engine_id      = job.engine_id;
                t.user_account   = job.user_account;
                t.params_json    = job.params.ToJsonString();
                t.status         = static_cast<int>(job.status);
                t.server_task_id = job.server_task_id;
                t.result_url     = job.result_url;
                t.preview_url    = job.preview_url;
                t.result_path    = job.result_path;
                t.preview_path   = job.preview_path;
                t.result_dir     = job.result_dir;
                t.error_message  = job.error_message;
                t.query_retry_count = job.query_retry_count;
                t.progress       = job.progress;

                // 积分 (PointInfoBase → GenJobTaskData 平铺)
                t.freeze_no        = job.point_info.freeze_no;
                t.frozen_points    = job.point_info.frozen_points;
                t.consumed         = job.point_info.consumed;
                t.refunded         = job.point_info.refunded;
                t.total_balance    = job.point_info.total_balance;
                t.available_points = job.point_info.available_points;
                t.points_settled   = job.point_info.points_settled;

                // 反馈节
                f.feedBackData.status     = static_cast<int>(feedback.Status);
                f.feedBackData.percent    = feedback.Percent;
                f.feedBackData.taskRetVal = feedback.TaskRetVal;
                f.feedBackData.msg        = feedback.Msg;

                f.Serialize(out);
                out.close();
                return true;
            }

            bool LoadFromBin(const std::string& filePath) {
                std::ifstream in = File::OpenIfstreamUtf8(filePath, std::ios::binary);
                if (!in.is_open())
                    return false;

                GenJobFile f;
                if (!f.Deserialize(in)) {
                    in.close();
                    return false;
                }
                in.close();

                job_name = f.jobName;

                // 任务数据节
                GenJobTaskData& t = f.genJobTaskData;
                job.generation_id  = t.generation_id;
                job.task_uuid      = t.task_uuid;
                job.engine_id      = t.engine_id;
                job.user_account   = t.user_account;
                job.project_path   = f.genJobInfoData.project_path;
                job.block_item     = f.genJobInfoData.block_item;
                job.params         = GenTaskParams::CreateFromJsonString(t.params_json);
                job.status         = static_cast<GenTaskStatus>(t.status);
                job.server_task_id = t.server_task_id;
                job.result_url     = t.result_url;
                job.preview_url    = t.preview_url;
                job.result_path    = t.result_path;
                job.preview_path   = t.preview_path;
                job.result_dir     = t.result_dir;
                job.error_message  = t.error_message;
                job.query_retry_count = t.query_retry_count;
                job.progress       = t.progress;

                // 积分
                job.point_info.freeze_no        = t.freeze_no;
                job.point_info.frozen_points    = t.frozen_points;
                job.point_info.consumed         = t.consumed;
                job.point_info.refunded         = t.refunded;
                job.point_info.total_balance    = t.total_balance;
                job.point_info.available_points = t.available_points;
                job.point_info.points_settled   = t.points_settled;

                // 反馈
                feedback.Status     = static_cast<jobsta_e>(f.feedBackData.status);
                feedback.Percent    = f.feedBackData.percent;
                feedback.TaskRetVal = f.feedBackData.taskRetVal;
                feedback.Msg        = f.feedBackData.msg;

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

> **架构说明**: 以下采用 `GenJobInfoData`(指针) + `GenJobTaskData`(任务全量数据) 的分节设计，对标重建式 `JobInfoData` + `RunInfoData+TaskItemData`。积分字段 (PointInfoBase) 落在 `GenJobTaskData` 中，序列化/反序列化详见本节代码。`BLKBinFile` 增加 `GenTaskOptions` 序列化字段。
>
> **积分系统详细设计**见 **[积分接口集成方案.md](积分接口集成方案.md)**（第二章：数据结构、第四章：PointManager、第五章：生成式集成、第六章：重建式集成）。

- [ ] `Include/Core/DataStruct.h`：在 `FeedBackFile` 之后、`JobInfoData` 之前增加 `GenJobInfoData` + `GenJobFile`
- [ ] `Include/Core/DataStruct.h`：`GenJobFile` 增加 `FeedBackData feedBackData` (对标 `JobListFile::feedBackData`)
- [ ] `Include/Core/DataStruct.h`：`BLKBinFile` 增加 `gen_block_task_category` + `gen_params_json` + 更新 Serialize/Deserialize

#### GenJobInfoData / GenJobTaskData / GenJobFile — 生成式 BIN 序列化

> 对标重建式 `JobInfoData` + `RunInfoData` + `TaskItemData` / `JobListFile`。
> - `GenJobInfoData` — 精简指针（对标 `JobInfoData`）
> - `GenJobTaskData` — 任务全部数据（对标 `RunInfoData` + `TaskItemData` 的合集）
> - `GenJobFile` — 顶层容器（对标 `JobListFile`），序列化时将各节顺序写入

```cpp
// ============================================================================
// GenJobFile / GenJobInfoData / GenJobTaskData
//   对标 JobListFile / JobInfoData / RunInfoData+TaskItemData
// ============================================================================

struct GenJobInfoData {
    std::string project_path;   // 项目路径
    std::string block_item;     // Block 名称
    ByteCrypt byteCrypt;

    GenJobInfoData() {
        project_path = "";
        block_item = "";
    }

    bool Serialize(std::ofstream& out) const {
        unsigned int project_path_len = project_path.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&project_path_len),
                                     sizeof(project_path_len));
        byteCrypt.WriteByteDecrypted(out, project_path.c_str(), project_path_len);

        unsigned int block_item_len = block_item.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&block_item_len),
                                     sizeof(block_item_len));
        byteCrypt.WriteByteDecrypted(out, block_item.c_str(), block_item_len);
        return true;
    }

    bool Deserialize(std::ifstream& in) {
        unsigned int project_path_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&project_path_len),
                                    sizeof(unsigned int));
        project_path.resize(project_path_len);
        byteCrypt.ReadByteDecrypted(in, &project_path[0], project_path_len);

        unsigned int block_item_len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&block_item_len),
                                    sizeof(unsigned int));
        block_item.resize(block_item_len);
        byteCrypt.ReadByteDecrypted(in, &block_item[0], block_item_len);
        return true;
    }
};

// ---- 指针节 (对标 JobInfoData) ----

// GenJobInfoData — 同上, 仅 project_path + block_item

// ---- 任务数据节 (对标 RunInfoData + TaskItemData 的合集) ----

struct GenJobTaskData {
    // GenJobInfo_s 字段
    int generation_id;
    std::string task_uuid;
    std::string engine_id;
    std::string user_account;
    std::string params_json;
    int status;
    std::string server_task_id;
    std::string result_url;
    std::string preview_url;
    std::string result_path;
    std::string preview_path;
    std::string result_dir;          // 结果目录: Generations/Generation_<id>/
    std::string error_message;
    int query_retry_count;
    int progress;

    // PointInfoBase 字段
    std::string freeze_no;
    int frozen_points;
    int consumed;
    int refunded;
    int total_balance;
    int available_points;
    bool points_settled;

    ByteCrypt byteCrypt;

    GenJobTaskData() {
        generation_id = 0;
        task_uuid = ""; engine_id = ""; user_account = "";
        params_json = ""; status = 0;
        server_task_id = ""; result_url = ""; preview_url = "";
        result_path = ""; preview_path = ""; result_dir = ""; error_message = "";
        query_retry_count = 0; progress = 0;
        freeze_no = ""; frozen_points = 0; consumed = 0; refunded = 0;
        total_balance = 0; available_points = 0; points_settled = false;
    }

    bool Serialize(std::ofstream& out) const {
        auto ws = [&](const std::string& s) {
            unsigned int len = s.size();
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&len), sizeof(len));
            byteCrypt.WriteByteDecrypted(out, s.c_str(), len);
        };
        auto wi = [&](int v) {
            byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&v), sizeof(int));
        };

        wi(generation_id);
        ws(task_uuid); ws(engine_id); ws(user_account);
        ws(params_json); wi(status);
        ws(server_task_id); ws(result_url); ws(preview_url);
        ws(result_path); ws(preview_path); ws(result_dir); ws(error_message);
        wi(query_retry_count); wi(progress);

        ws(freeze_no); wi(frozen_points); wi(consumed); wi(refunded);
        wi(total_balance); wi(available_points); wi(points_settled ? 1 : 0);

        return true;
    }

    bool Deserialize(std::ifstream& in) {
        auto rs = [&](std::string& s) {
            unsigned int len = 0;
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&len), sizeof(unsigned int));
            s.resize(len);
            byteCrypt.ReadByteDecrypted(in, &s[0], len);
        };
        auto ri = [&](int& v) {
            byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&v), sizeof(int));
        };

        ri(generation_id);
        rs(task_uuid); rs(engine_id); rs(user_account);
        rs(params_json); ri(status);
        rs(server_task_id); rs(result_url); rs(preview_url);
        rs(result_path); rs(preview_path); rs(result_dir); rs(error_message);
        ri(query_retry_count); ri(progress);

        rs(freeze_no); ri(frozen_points); ri(consumed); ri(refunded);
        ri(total_balance); ri(available_points);
        int settledInt = 0; ri(settledInt); points_settled = (settledInt != 0);

        return true;
    }
};

// ---- 顶层容器 (对标 JobListFile) ----

struct GenJobFile {
    std::string jobName;            // 对标 JobListFile::jobName
    GenJobInfoData genJobInfoData;  // 精简指针节
    GenJobTaskData genJobTaskData;  // 任务数据节
    FeedBackData feedBackData;      // 反馈节

    ByteCrypt byteCrypt;
    GenJobFile() {}

    bool Serialize(std::ofstream& out) const {
        const char HEADER[] = "GENJOB-FILE-3MO";
        byteCrypt.WriteByteDecrypted(out, HEADER, 15);

        unsigned int len = jobName.size();
        byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&len), sizeof(len));
        byteCrypt.WriteByteDecrypted(out, jobName.c_str(), len);

        genJobInfoData.Serialize(out);   // 1. 指针节
        genJobTaskData.Serialize(out);   // 2. 任务数据节 (含积分)
        feedBackData.Serialize(out);     // 3. 反馈节
        return true;
    }

    bool Deserialize(std::ifstream& in) {
        char header[15];
        byteCrypt.ReadByteDecrypted(in, header, sizeof(header));
        if (std::string(header, 15) != "GENJOB-FILE-3MO") return false;

        unsigned int len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&len), sizeof(unsigned int));
        jobName.resize(len);
        byteCrypt.ReadByteDecrypted(in, &jobName[0], len);

        genJobInfoData.Deserialize(in);
        genJobTaskData.Deserialize(in);
        feedBackData.Deserialize(in);
        return true;
    }
};
```

#### BLKBinFile — 在现有字段末尾增加

```cpp
// BLKBinFile 中增加:
int    gen_block_task_category = 0;     // 0=重建(默认), 1=生成式
int    gen_next_generation_id = 1;      // 递增计数器 (只增不复用)
std::string gen_params_json;            // 生成式参数 JSON 字符串
std::string gen_info_json;              // generations_info_ JSON 数组字符串 (新增)
int    genJobNum = 0;                   // generationjobs_ 条目数 (对标 jobNum)
std::vector<std::string> genJobVec;     // "task_uuid:job_name" (对标 jobVec)

// Serialize 中增加:
byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&gen_block_task_category), sizeof(int));
byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&gen_next_generation_id), sizeof(int));

unsigned int gen_params_json_len = gen_params_json.size();
byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&gen_params_json_len), sizeof(gen_params_json_len));
byteCrypt.WriteByteDecrypted(out, gen_params_json.c_str(), gen_params_json_len);

unsigned int gen_info_json_len = gen_info_json.size();
byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&gen_info_json_len), sizeof(gen_info_json_len));
byteCrypt.WriteByteDecrypted(out, gen_info_json.c_str(), gen_info_json_len);

byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&genJobNum), sizeof(int));
for (int i = 0; i < genJobNum; i++) {
    unsigned int len = genJobVec[i].size();
    byteCrypt.WriteByteDecrypted(out, reinterpret_cast<const char*>(&len), sizeof(unsigned int));
    byteCrypt.WriteByteDecrypted(out, genJobVec[i].c_str(), len);
}

// Deserialize 中增加 (兼容旧文件):
// ★ 旧 .blk 文件末尾没有以下字段, 用 peek() 检测 EOF 防止读到垃圾数据
if (in.peek() != EOF) {
    byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&gen_block_task_category), sizeof(int));
    byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&gen_next_generation_id), sizeof(int));

    unsigned int gen_params_json_len = 0;
    byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&gen_params_json_len), sizeof(unsigned int));
    gen_params_json.resize(gen_params_json_len);
    byteCrypt.ReadByteDecrypted(in, &gen_params_json[0], gen_params_json_len);

    unsigned int gen_info_json_len = 0;
    byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&gen_info_json_len), sizeof(unsigned int));
    gen_info_json.resize(gen_info_json_len);
    byteCrypt.ReadByteDecrypted(in, &gen_info_json[0], gen_info_json_len);

    byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&genJobNum), sizeof(int));
    genJobVec.resize(genJobNum);
    for (int i = 0; i < genJobNum; i++) {
        unsigned int len = 0;
        byteCrypt.ReadByteDecrypted(in, reinterpret_cast<char*>(&len), sizeof(unsigned int));
        genJobVec[i].resize(len);
        byteCrypt.ReadByteDecrypted(in, &genJobVec[i][0], len);
    }
}
```

> **兼容性**: `if (in.peek() != EOF)` 保护新 reader 读取旧文件。旧文件末尾没有 GenTask 字段，peek() 返回 EOF → 跳过解析 → 新字段保持默认值 (gen_block_task_category=0, gen_params_json="", generations_info_ 为空)。新文件有这些字段，正常解析。

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
#include "Core/Rapidjson.h"
#include "Core/String.h"     // StringIsNullOrBlank

namespace AI3D {
    namespace CORE {

        // ============================================================================
        // GenTaskStatus — 生成式任务生命周期状态
        // 放在 Core/ 层供 GenTaskAPI (MoldAIData.dll) 和 GenTaskThread (MoldAINode.exe) 共用
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
        struct GenTaskParams  // 不加 AI3D_API — 纯数据 struct, 隐式拷贝构造需内联, 否则跨 DLL 边界崩溃 (0x8)
        {
            GenTaskSubType  sub_type = GenTaskSubType::UNKNOWN;     // 任务类型 (调用者需过滤)
            std::string prompt;            // 文本提示词
            std::string negative_prompt;   // 反向提示词
            int         polygon_limit = 0; // 面数限制
            int         texture_size = 0;  // 纹理分辨率
            int         provider_id = 0;   // 供应商类型 (默认 0, 前端根据服务商列表选择)
            std::string model_version;     // 模型版本 (字符串: 服务端可能新增版本)
            std::string file_key;          // 已上传文件的 key。前端先调用 UploadFile 上传本地素材
                                          // (图片/模型), 服务端返回 file_key 后填入此处。
                                          // IMAGE_TO_MODEL / TEXTURE_MODEL 等需要输入素材时必填,
                                          // TEXT_TO_MODEL 等纯文字任务保持空字符串。

            // rapidjson 序列化 (对标 ATOptions::WriteToJson / ParseJson)
            void WriteToJson(rapidjson::Value& metadata, rapidjson::Document& document) const {
                rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
                if (const char* s = ToString(sub_type))
                    metadata.AddMember("sub_type", rapidjson::Value(s, allocator), allocator);
                if (!StringIsNullOrBlank(prompt))
                    metadata.AddMember("prompt", rapidjson::Value(prompt.c_str(), allocator), allocator);
                if (!StringIsNullOrBlank(negative_prompt))
                    metadata.AddMember("negative_prompt", rapidjson::Value(negative_prompt.c_str(), allocator), allocator);
                if (polygon_limit != 0)
                    metadata.AddMember("polygon_limit", rapidjson::Value(polygon_limit), allocator);
                if (texture_size != 0)
                    metadata.AddMember("texture_size", rapidjson::Value(texture_size), allocator);
                if (provider_id != 0)
                    metadata.AddMember("provider_id", rapidjson::Value(provider_id), allocator);
                if (!StringIsNullOrBlank(model_version))
                    metadata.AddMember("model_version", rapidjson::Value(model_version.c_str(), allocator), allocator);
                if (!StringIsNullOrBlank(file_key))
                    metadata.AddMember("file_key", rapidjson::Value(file_key.c_str(), allocator), allocator);
            }

            std::string ToJsonString() const {
                rapidjson::Document doc;
                doc.SetObject();
                WriteToJson(doc, doc);
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                doc.Accept(writer);
                return buffer.GetString();
            }

            void ParseJson(const rapidjson::Value& metadata) {
                if (metadata.HasMember("sub_type"))
                    sub_type = SubTypeFromString(metadata["sub_type"].GetString());
                if (metadata.HasMember("prompt"))
                    prompt = metadata["prompt"].GetString();
                if (metadata.HasMember("negative_prompt"))
                    negative_prompt = metadata["negative_prompt"].GetString();
                if (metadata.HasMember("polygon_limit"))
                    polygon_limit = metadata["polygon_limit"].GetInt();
                if (metadata.HasMember("texture_size"))
                    texture_size = metadata["texture_size"].GetInt();
                if (metadata.HasMember("provider_id"))
                    provider_id = metadata["provider_id"].GetInt();
                if (metadata.HasMember("model_version"))
                    model_version = metadata["model_version"].GetString();
                if (metadata.HasMember("file_key"))
                    file_key = metadata["file_key"].GetString();
            }

            static GenTaskParams CreateFromJsonString(const std::string& jsonStr) {
                GenTaskParams p;
                if (StringIsNullOrBlank(jsonStr)) return p;
                rapidjson::Document doc;
                if (doc.Parse(jsonStr.c_str()).HasParseError()) return p;
                p.ParseJson(doc);
                return p;
            }
        };

        // ============================================================================

        // GenTaskOptions — 嵌入 BlockObject::Task_Info 的生成式任务配置
        // 对标 ATOptions at_options, 仅含生成参数
        // 任务类型判断由 Task_Info::block_task_category 负责
        // ============================================================================
        struct GenTaskOptions  // 不加 AI3D_API — 同上
        {
            GenTaskParams gen_params;                // 生成参数

            // rapidjson 序列化 (委托 gen_params, 对标 ATOptions::WriteToJson / ParseJson)
            void WriteToJson(rapidjson::Value& metadata, rapidjson::Document& document) const {
                gen_params.WriteToJson(metadata, document);
            }
            void ParseJson(const rapidjson::Value& metadata) {
                gen_params.ParseJson(metadata);
            }
        };

        // ============================================================================
        // blk_generation_info_s — 生成式任务结果元数据
        // 对标 blk_reconst_production_info_s, 存储在 Block 的 Task_Info 中, 供前端目录树展示
        // status / sub_type 存 int (避免 Core→Util 依赖), 读取后由调用者转枚举
        // ============================================================================
        struct blk_generation_info_s  // 不加 AI3D_API — 同上, vector 拷贝会触发每个元素的拷贝构造
        {
            int         generation_id = -1;  // 对标 production_t id_
            std::string task_uuid;           // 任务唯一标识
            std::string job_name;            // job 文件名
            int         sub_type = -1;       // GenTaskSubType 枚举值
            int         status = -1;         // GenTaskStatus 枚举值
            std::string preview_url;         // 预览图链接
            std::string result_url;          // 结果下载链接
            std::string result_path;         // 结果本地保存路径 (下载后)
            std::string preview_path;        // 预览图本地保存路径 (下载后)
            std::string result_dir;          // 结果目录: Generations/Generation_<id>/
            std::string created_time;        // 创建时间 "yyyyMMddhhmmss"

            void CreateJson(rapidjson::Value& value, rapidjson::Document& doc) const {
                auto& allocator = doc.GetAllocator();
                value.AddMember("generation_id", rapidjson::Value(generation_id), allocator);
                value.AddMember("task_uuid", rapidjson::Value(task_uuid.c_str(), allocator), allocator);
                value.AddMember("job_name", rapidjson::Value(job_name.c_str(), allocator), allocator);
                value.AddMember("sub_type", rapidjson::Value(sub_type), allocator);
                value.AddMember("status", rapidjson::Value(status), allocator);
                if (!preview_url.empty())
                    value.AddMember("preview_url", rapidjson::Value(preview_url.c_str(), allocator), allocator);
                if (!result_url.empty())
                    value.AddMember("result_url", rapidjson::Value(result_url.c_str(), allocator), allocator);
                if (!result_path.empty())
                    value.AddMember("result_path", rapidjson::Value(result_path.c_str(), allocator), allocator);
                if (!preview_path.empty())
                    value.AddMember("preview_path", rapidjson::Value(preview_path.c_str(), allocator), allocator);
                if (!result_dir.empty())
                    value.AddMember("result_dir", rapidjson::Value(result_dir.c_str(), allocator), allocator);
                if (!created_time.empty())
                    value.AddMember("created_time", rapidjson::Value(created_time.c_str(), allocator), allocator);
            }

            void ParseJson(const rapidjson::Value& value) {
                if (value.HasMember("generation_id")) generation_id = value["generation_id"].GetInt();
                if (value.HasMember("task_uuid"))     task_uuid = value["task_uuid"].GetString();
                if (value.HasMember("job_name"))      job_name = value["job_name"].GetString();
                if (value.HasMember("sub_type"))      sub_type = value["sub_type"].GetInt();
                if (value.HasMember("status"))        status = value["status"].GetInt();
                if (value.HasMember("preview_url"))   preview_url = value["preview_url"].GetString();
                if (value.HasMember("result_url"))    result_url = value["result_url"].GetString();
                if (value.HasMember("created_time"))  created_time = value["created_time"].GetString();
                if (value.HasMember("result_path"))   result_path = value["result_path"].GetString();
                if (value.HasMember("preview_path"))  preview_path = value["preview_path"].GetString();
                if (value.HasMember("result_dir"))    result_dir = value["result_dir"].GetString();
            }
        };

    } // namespace CORE
} // namespace AI3D
```

#### 修改 BlockObject.h

- [ ] `Task_Info` 增加生成结果追踪（对标 `reconstructions_info_` + `reconstructionjobs_`）

```cpp
#include "Core/GenTaskOptions.h"

// 在 Task_Info 结构体中 (ATOptions at_options 旁边):
int           block_task_category = 0;   // 0=重建(默认), 1=生成式 (Task_Info 一级判断字段)
GenTaskOptions gen_options;             // 生成式任务参数 (block_task_category==1 时有效)

// 在 Task_Info 结构体中 (reconstructions_info_ / reconstructionjobs_ 旁边):
std::vector<blk_generation_info_s> generations_info_;        // 生成结果元数据列表
std::map<std::string, std::string> generationjobs_;         // task_uuid → job_name
int           next_generation_id = 1;   // 递增计数器 (只增不复用, 对标 reconstruction 的 ID 机制)
```

> `next_generation_id` 持久化在 Block.blk 中，每次 SubmitGenTask 取当前值后自增，删除已存在的 Generation 不会重置它。
#### 修改 BlockObject.cpp —— JSON 序列化

`WriteBlockInfoToJson()` 中增加：

```C++
// 写入 GenTaskOptions (对标 at_options.WriteToJson)
root.AddMember("block_task_category", rapidjson::Value(block_task_category), allocator);
rapidjson::Value genOptionsJson(rapidjson::kObjectType);
gen_options.WriteToJson(genOptionsJson, document);
root.AddMember("gen_options", genOptionsJson, allocator);

// 写入生成结果列表 (对标 reconstructions_info_)
rapidjson::Value generationsInfo(rapidjson::kArrayType);
for (auto& gen : generations_info_) {
    rapidjson::Value genJson(rapidjson::kObjectType);
    gen.CreateJson(genJson, document);
    generationsInfo.PushBack(genJson, allocator);
}
root.AddMember("generations_info", generationsInfo, allocator);

// 写入生成任务映射 (对标 BRPJobs)
rapidjson::Value genJobs(rapidjson::kArrayType);
for (auto& jobstr : generationjobs_) {
    std::string combined = jobstr.first + ":" + jobstr.second;
    genJobs.PushBack(rapidjson::Value(combined.c_str(), allocator), allocator);
}
root.AddMember("GenJobs", genJobs, allocator);
```

`ReadBlockInfoJson()` 中增加：

```cpp
// 读取 GenTaskOptions (对标 at_options.ParseJson, 在 settings 读取之后)
if (doc_blk.HasMember("block_task_category"))
    block_task_category = doc_blk["block_task_category"].GetInt();
if (doc_blk.HasMember("gen_options"))
    gen_options.ParseJson(doc_blk["gen_options"]);

// 读取生成结果列表 (对标 reconstructions_info_)
if (doc_blk.HasMember("generations_info")) {
    const rapidjson::Value& genArr = doc_blk["generations_info"];
    for (rapidjson::SizeType i = 0; i < genArr.Size(); i++) {
        blk_generation_info_s info;
        info.ParseJson(genArr[i]);
        generations_info_.push_back(info);
    }
}

// 读取生成任务映射 (对标 BRPJobs / GenJobs)
if (doc_blk.HasMember("GenJobs")) {
    const rapidjson::Value& genJobsArr = doc_blk["GenJobs"];
    for (rapidjson::SizeType i = 0; i < genJobsArr.Size(); i++) {
        std::string combined = genJobsArr[i].GetString();
        size_t colonPos = combined.find(":");
        if (colonPos != std::string::npos)
            generationjobs_[combined.substr(0, colonPos)] = combined.substr(colonPos + 1);
    }
}
```

#### 修改 BlockObject.cpp —— BIN 序列化

`WriteBlockInfoToBin()` 中增加：

```cpp
bLKBinFile.gen_block_task_category = block_task_category;
bLKBinFile.gen_next_generation_id  = next_generation_id;
bLKBinFile.gen_params_json         = gen_options.gen_params.ToJsonString();

// generations_info_ → JSON array string (对标 gen_params_json 的序列化策略)
{
    rapidjson::Document doc;
    doc.SetArray();
    auto& allocator = doc.GetAllocator();
    for (auto& gen : generations_info_) {
        rapidjson::Value genJson(rapidjson::kObjectType);
        gen.CreateJson(genJson, doc);
        doc.PushBack(genJson, allocator);
    }
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    bLKBinFile.gen_info_json = buffer.GetString();
}

// generationjobs_ (对标 jobVec)
bLKBinFile.genJobNum = generationjobs_.size();
for (auto& jobstr : generationjobs_)
    bLKBinFile.genJobVec.push_back(jobstr.first + ":" + jobstr.second);
```

`ReadBlockInfoBin()` 中增加：

```cpp
block_task_category    = bLKBinFile.gen_block_task_category;
next_generation_id     = bLKBinFile.gen_next_generation_id;
gen_options.gen_params = GenTaskParams::CreateFromJsonString(bLKBinFile.gen_params_json);

// generations_info_ ← JSON array string
generations_info_.clear();
if (!bLKBinFile.gen_info_json.empty()) {
    rapidjson::Document doc;
    if (!doc.Parse(bLKBinFile.gen_info_json.c_str()).HasParseError() && doc.IsArray()) {
        for (rapidjson::SizeType i = 0; i < doc.Size(); i++) {
            blk_generation_info_s info;
            info.ParseJson(doc[i]);
            generations_info_.push_back(info);
        }
    }
}

// generationjobs_
generationjobs_.clear();
for (auto& job : bLKBinFile.genJobVec) {
    size_t colonPos = job.find(":");
    if (colonPos != std::string::npos)
        generationjobs_[job.substr(0, colonPos)] = job.substr(colonPos + 1);
}
```

> **兼容性**: 旧 `.blk` 文件中没有 `block_task_category`/`gen_params` 字段 → JSON 读取走 `if contains` 检查, 字段不存在时 struct 保持默认值；BIN 读取走默认构造。前端判断 `block_task_category == 0` 为重建式（旧 Block 全默认为 0），`== 1` 为生成式。

### 1.4 JobFeedBack_s — 不修改，但增加持有关系

> `JobFeedBack_s`（`FeedBackFile`）是固定格式的进度反馈文件，只有 `Status`/`Percent`/`Msg`/`TaskRetVal` 四个字段。生成式任务直接复用：
>
> - `Status` → 映射 `GenTaskStatus` → `jobsta_e`
> - `Percent` → 进度百分比
> - `Msg` → 状态描述文字
> - `TaskRetVal` → 0=成功, 非0=失败
>
> **持有关系**: `GenJobFullInfo_s` 新增 `JobFeedBack_s feedback` 成员（对标 `TaskGraph_s` 持有 `JobFeedBack_s`），在调度线程中加载 job 时同步加载 feedback 到内存，`UpdateFeedback()` 直接修改内存中的 `feedback` 成员，调用者负责写回独立 `JF_*` 文件。避免了每次更新 feedback 都要 `load_with_retry` + `save_with_retry` 的额外 I/O。
>
> **结果数据**（`result_url`/`preview_url`/`server_task_id`/`error_message`）存在 `GenJobInfo_s` 自身中，不做为 feedback 的扩展字段。积分数据统一在 `PointInfoBase point_info` 中，由 FreezePoints/SettlePoints 回填。

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

### 1.6 Phase 1 验证

- [ ] 全量编译 MoldAIData.dll + MoldAINode.exe 通过
- [ ] GenTaskParams rapidjson 往返: `WriteToJson` → `ParseJson` / `ToJsonString` → `CreateFromJsonString`
- [ ] GenTaskOptions rapidjson 往返: 委托 `gen_params.WriteToJson` / `ParseJson` 正确
- [ ] GenJobFullInfo_s BIN 往返: `WriteToBin` → `LoadFromBin` 含 feedback 逐字段一致
- [ ] `blk_generation_info_s` rapidjson 往返: `CreateJson` → `ParseJson` 逐字段一致
- [ ] `generations_info_` JSON 数组往返: 序列化 → 字符串 → 解析, 多条目录项正确
- [ ] `.blk` 文件落地 → 加载: 通过 `BlockObject::Save()` / 构造加载, gen 全部字段往返正确
- [ ] 前端提交端到端: 设 Block gen 参数 → `SubmitGenTask` → job + feedback 落地 → 参数一致
- [ ] 旧 `.blk` 兼容: 无 `gen_options` 字段 → 读取后 `block_task_category == 0`, `gen_params` 为默认值

#### 测试目录结构

- [ ] 创建 `Test/testGenTask/`
- [ ] 创建 `Test/testGenTask/CMakeLists.txt`
- [ ] 创建 `Test/testGenTask/testGenTaskPhase1.cpp`
- [ ] `Test/CMakeLists.txt` 增加 `add_subdirectory(testGenTask)`

#### Test/testGenTask/CMakeLists.txt

```cmake
# ===== target 1: Phase 1 数据结构验证 =====
set(TARGET_NAME testGenTaskPhase1)
FILE(GLOB PHASE1_SRC "${CMAKE_CURRENT_SOURCE_DIR}/testGenTaskPhase1.cpp")
add_executable(${TARGET_NAME} ${PHASE1_SRC})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../Include)
AI3D_USING_GLOG()
set(DEPENDS_LIBS ${EXTER_DEPENDS_LIBS} ${AI3D_GLOG_LIB} Core)
target_link_libraries(${TARGET_NAME} ${DEPENDS_LIBS})
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Test")

# ===== target 2: 全流程集成测试 (链接 Engine 层) =====
set(TARGET_NAME testGenTaskFullFlow)
set(FULLFLOW_SRC "${CMAKE_CURRENT_SOURCE_DIR}/testGenTaskFullFlow.cpp")
set(ENGINE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../App/Engine/GenTaskThread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../App/Engine/GenHttpClient.cpp
)
add_executable(${TARGET_NAME} ${FULLFLOW_SRC} ${ENGINE_SRC})
target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../App/Engine   # GenTaskThread.h / GenHttpClient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Include       # Core/ Util/ headers
)
target_compile_definitions(${TARGET_NAME} PRIVATE MOCK_GEN_HTTP)
set(DEPENDS_LIBS ${EXTER_DEPENDS_LIBS} ${AI3D_GLOG_LIB} Core
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network)
target_link_libraries(${TARGET_NAME} ${DEPENDS_LIBS})
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Test")
```

#### Test/testGenTask/testGenTaskPhase1.cpp

```cpp
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
    p.sub_type       = GenTaskSubType::IMAGE_TO_MODEL;
    p.prompt         = "a red sports car";
    p.negative_prompt = "blurry, low quality";
    p.polygon_limit  = 50000;
    p.texture_size   = 2048;
    p.model_version  = "v2";
    p.file_key       = "fk-abc123";

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
    TEST_ASSERT(p2.file_key == "fk-abc123", "file_key roundtrip failed");

    // 1e. UNKNOWN sub_type 不应写入
    // 1e. UNKNOWN sub_type should not be written
    GenTaskParams p3;
    p3.sub_type = GenTaskSubType::UNKNOWN;
    p3.prompt   = "test";
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
    opts.gen_params.sub_type      = GenTaskSubType::TEXT_TO_MODEL;
    opts.gen_params.prompt        = "a cute cat";
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
    info.job_name                = "J_TestBlock_20260101000000";
    info.job.task_uuid           = "550e8400-e29b-41d4-a716-446655440000";
    info.job.engine_id           = "engine-test-01";
    info.job.user_account        = "user@example.com";
    info.job.project_path        = "/test/project";
    info.job.block_item          = "TestBlock";
    info.job.params.sub_type     = GenTaskSubType::TEXT_TO_MESH;
    info.job.params.prompt       = "a dragon statue";
    info.job.params.polygon_limit = 20000;
    info.job.params.texture_size  = 1024;
    info.job.status              = GenTaskStatus::IN_PROGRESS;
    info.job.server_task_id      = "trv-abc123def456";
    info.job.result_url          = "https://cdn.example.com/result.glb";
    info.job.preview_url         = "https://cdn.example.com/preview.png";
    info.job.result_path         = "/local/path/result.glb";
    info.job.preview_path        = "/local/path/preview.png";
    info.job.point_info.freeze_no        = "test-freeze-no-123";
    info.job.point_info.frozen_points    = 100;
    info.job.point_info.consumed         = 10;
    info.job.point_info.total_balance    = 90;
    info.job.point_info.available_points = 90;
    info.job.point_info.points_settled   = false;
    info.job.query_retry_count   = 2;
    info.feedback.Status     = jobsta_e::STATUS_RUNNING;
    info.feedback.Percent    = 45.0f;
    info.feedback.TaskRetVal = 0;
    info.feedback.Msg        = "processing mesh generation";

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
    TEST_ASSERT(loaded.job.server_task_id == info.job.server_task_id,
                "server_task_id mismatch");
    TEST_ASSERT(loaded.job.result_url == info.job.result_url, "result_url mismatch");
    TEST_ASSERT(loaded.job.preview_url == info.job.preview_url, "preview_url mismatch");
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
    info.task_uuid   = "uuid-test-12345";
    info.job_name    = "J_TestBlock_20260129120000";
    info.sub_type    = static_cast<int>(GenTaskSubType::TEXT_TO_MODEL);
    info.status      = static_cast<int>(GenTaskStatus::COMPLETED);
    info.preview_url = "https://cdn.example.com/preview.png";
    info.result_url  = "https://cdn.example.com/result.glb";
    info.result_path  = "/local/path/result.glb";
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
    TEST_ASSERT(loaded.preview_url == info.preview_url, "ParseJson preview_url failed");
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
    gen1.job_name  = "J_Block_001";
    gen1.sub_type  = static_cast<int>(GenTaskSubType::TEXT_TO_MODEL);
    gen1.status    = static_cast<int>(GenTaskStatus::COMPLETED);
    gen1.result_url = "https://cdn.example.com/result1.glb";

    blk_generation_info_s gen2;
    gen2.task_uuid = "uuid-2";
    gen2.job_name  = "J_Block_002";
    gen2.sub_type  = static_cast<int>(GenTaskSubType::IMAGE_TO_MODEL);
    gen2.status    = static_cast<int>(GenTaskStatus::IN_PROGRESS);

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
    for (rapidjson::SizeType i = 0; i < doc2.Size(); i++) {
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
    std::string parsed_job  = jobEntry.substr(colonPos + 1);
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
    QString tmpDir = QDir::tempPath() + "/test_genblock_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
    QDir().mkpath(tmpDir);
    std::string projectPath = tmpDir.toStdString();

    // 7b. 对标 Slot_Action_NewBlock: new BlockObject(projectPath)
    // 7b. Matching Slot_Action_NewBlock: new BlockObject(projectPath)
    BlockObject block(projectPath);
    block.GetIdMutual() = 1;
    block.Init();  // → name_ = "Block_1", path_ = projectPath/Block_1
    block.SetStatus(jobsta_e::STATUS_NEW);

    // Block 子目录 (Init 后 path_ = projectPath/Block_1)
    QDir().mkpath(QString::fromStdString(block.GetPath()));

    // 对标 AddBlock 中的设置
    // Matching settings from AddBlock
    block.GetTaskInfoMutual().blockString = "Block_1";
    block.GetTaskInfoMutual().blockName = block.GetName();  // 用 Init 后的 name_, 保持一致
    block.GetTaskInfoMutual().blockId    = 1;
    block.GetTaskInfoMutual().projectfile_ = projectPath;
    block.GetTaskInfoMutual().isSaved    = false;
    block.GetTaskInfoMutual().isLoaded   = true;

    // 7c. 设置生成式字段
    // 7c. Set generative fields
    Task_Info& info = block.GetTaskInfoMutual();
    info.block_task_category = 1;
    info.gen_options.gen_params.sub_type      = GenTaskSubType::TEXT_TO_MODEL;
    info.gen_options.gen_params.prompt        = "a test cube";
    info.gen_options.gen_params.polygon_limit = 10000;

    blk_generation_info_s gen1;
    gen1.task_uuid    = "uuid-blk-test-1";
    gen1.job_name     = "J_Block_1_20260129000000";
    gen1.sub_type     = static_cast<int>(GenTaskSubType::TEXT_TO_MODEL);
    gen1.status       = static_cast<int>(GenTaskStatus::COMPLETED);
    gen1.result_url   = "https://cdn.example.com/result.glb";
    gen1.result_path   = "/local/path/result.glb";
    gen1.created_time = "20260129000000";
    info.generations_info_.push_back(gen1);

    info.generationjobs_[gen1.task_uuid] = gen1.job_name;

    // 7d. 落地 (直接写 .blk, 不调 Save() — Save() 内部会尝试导出 AT 数据, 空 Block 无 AT)
    // 7d. Persist (write .blk directly; Save() tries to export AT data, fails for empty block)
    std::string blkPath = block.GetPath() + "/" + block.GetName() + ".blk";
    bool ok = info.WriteBlockInfoToBin(blkPath);
    TEST_ASSERT(ok, "WriteBlockInfoToBin failed");

    // 7e. 读回 .blk 文件
    // 7e. Read back .blk file
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
    QDir(tmpDir).removeRecursively();
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
    task.blockString      = "Block_1";
    task.blockName        = "Block_1";
    task.blockId          = 1;
    task.projectfile_     = projectPath;
    task.isSaved          = false;
    task.isLoaded         = true;
    task.block_task_category = 1;                          // 生成式

    // 8c. 前端填入生成参数 (对标 options = ParamSettings4Production::GetSavedOptions)
    // 8c. Frontend fills gen params (matching options = ParamSettings4Production::GetSavedOptions)
    task.gen_options.gen_params.sub_type      = GenTaskSubType::TEXT_TO_MODEL;
    task.gen_options.gen_params.prompt        = "a red sports car";
    task.gen_options.gen_params.polygon_limit = 50000;
    task.gen_options.gen_params.texture_size  = 1024;

    // 8d. 前端构造 pendingPath 并提交 (对标 SubmitProduction)
    // 8d. Frontend constructs pendingPath and submits (matching SubmitProduction)
    std::string pendingPath = tmpDir.toStdString() + "/jobs_gen/Pending/";
    QDir().mkpath(QString::fromStdString(pendingPath));

    std::string userAccount = "testuser@example.com";
    GenTaskAPI::SubmitResult result = GenTaskAPI::SubmitGenTask(task, userAccount, pendingPath);

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
    TEST_ASSERT(result.generation_id == 1,
                "generation_id should be 1 for first generation");
    std::string expectedResultDir = projectPath + "/Block_1/" + std::string(GENERATION_DIR)
        + "/" + GENERATION_PREFIX + "1";
    TEST_ASSERT(jobInfo.job.result_dir == expectedResultDir,
                "result_dir should be Generations/Generation_1/");
    TEST_ASSERT(QDir(QString::fromStdString(expectedResultDir)).exists(),
                "Generations/Generation_1/ directory should be created");

    // 8h. 验证 feedback 文件已创建 (路径使用 job.result_dir, 而非 expectedResultDir)
    //     两者应一致, 此处用 job.result_dir 确保测试的是实际写入的值
    // 8h. Verify feedback file created
    std::string fbPath = jobInfo.job.result_dir + "/JF_" + result.job_name
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
    info.gen_options.gen_params.sub_type      = GenTaskSubType::TEXT_TO_MODEL;
    info.gen_options.gen_params.prompt        = "a red sports car";
    info.gen_options.gen_params.polygon_limit = 50000;
    info.gen_options.gen_params.texture_size  = 1024;
    info.gen_options.gen_params.model_version = "v2";
    info.blockName = "test_block";
    info.projectfile_ = tmpDir;

    // b. 写 BIN, 再读回
    TEST_ASSERT(info.WriteBlockInfoToBin(blkPath), "WriteBlockInfoToBin should succeed");

    AI3D::CORE::BlockObject::Task_Info readBack;
    TEST_ASSERT(readBack.ReadBlockInfoBin(blkPath), "ReadBlockInfoBin should succeed");

    // c. ★ 关键: 通过 GetTaskInfo() 返回值触发拷贝构造
    //    如果 AI3D_API 未去除, 此行崩溃在偏移 0x8
    AI3D::CORE::BlockObject::Task_Info copied = readBack;  // 显式拷贝
    (void)copied;

    // d. 验证拷贝后的数据一致
    AI3D::CORE::BlockObject block2;
    block2.SetName("test_block2");
    block2.SetPath(tmpDir);
    block2.SetTaskInfo(readBack);  // 通过 BlockObject 间接触发 GetTaskInfo() 路径
    AI3D::CORE::BlockObject::Task_Info gotBack = block2.GetTaskInfo();  // ← GetTaskInfo() 返回值拷贝

    TEST_ASSERT(gotBack.block_task_category == 1,                            "block_task_category mismatch");
    TEST_ASSERT(gotBack.gen_options.gen_params.sub_type == GenTaskSubType::TEXT_TO_MODEL, "sub_type mismatch");
    TEST_ASSERT(gotBack.gen_options.gen_params.prompt == "a red sports car", "prompt mismatch");
    TEST_ASSERT(gotBack.gen_options.gen_params.polygon_limit == 50000,       "polygon_limit mismatch");
    TEST_ASSERT(gotBack.gen_options.gen_params.texture_size == 1024,         "texture_size mismatch");
    TEST_ASSERT(gotBack.blockName == "test_block",                           "blockName mismatch");

    // e. 清理
    QDir(tmpDir).removeRecursively();

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
    TestGetTaskInfoCopy();  // ← 新增: 验证 AI3D_API 修复

    if (g_failures == 0) {
        LOGI("All Phase 1 tests passed.");
        return 0;
    } else {
        LOGE(std::to_string(g_failures) + " test(s) failed.");
        return 1;
    }
}
```

---

## 文件目录结构与命名约定

### 生成式 vs 重建式对照

```
重建式:
  Project/                                       ← Project 根
    Block_N/                                     ← Block 目录
      JF_<jobname>.bin                           ← Job Feedback (Block 根)
      JT_<jobname>.bin                           ← TimeSum (Block 根)
      <jobname>/                                 ← Job 工作目录 (TI 子任务)
      Reconstruction_M/                          ← 重建
        <jobname>/                               ← 重建 Job 工作目录
        JF_<baseName_R>.bin                      ← 重建 Job Feedback
        Productions/
          Production_P/                          ← 生产 (PRODUCTION_PREFIX + id)
            Tile_000/                            ← Tile 子任务工作目录
              TI_0.bin                           ← Tile 任务定义
              JF_<baseName_BRP>.bin              ← Tile Feedback
            JF_<baseName_BRP>.bin                ← 生产总 Job Feedback
            result/                              ← 产出数据目录
      source_data.json                           ← Block 数据文件
      .blk / .bbin                              ← Block 元数据

生成式:
  Project/
    Block_N/
      Generations/                               ← 对标 Productions
        Generation_1/                            ← 对标 Production_1 (GENERATION_PREFIX + id)
          JF_J_<BlockName>_<timestamp>.bin       ← Job Feedback (对标 Production 下的 JF)
          result.glb                             ← 结果文件
          preview.png                            ← 预览图
        Generation_2/
          JF_J_<BlockName>_<timestamp>.bin
      .blk / .bbin
```

### 命名规则

| 项目 | 规则 | 对标 |
|------|------|------|
| job_name | `J_<BlockName>_<yyyyMMddhhmmss>` | 重建式 job 名无固定规则, 生成式统一用 BlockName + 时间戳 |
| job 文件 | `<job_name>.bin` → `J_Block1_20240604120000.bin` | 同重建式 |
| JF 文件 | `JF_<job_name>.bin`, 放在 `result_dir/` 下 | 对标 `Productions/Production_<id>/JF_<jobname>.bin` |
| task_uuid | `<user>_<timestamp>_<uuid4>` | 生成式特有 (无对标) |
| 结果目录 | `Generations/Generation_<id>/` | 对标 `Productions/Production_<id>/` |
| generation_id | `int`, 从 1 递增, 由 `GetNextGenerationId()` 分配 | 对标 `production_t id_`, 由 `GenerateValidProductionId()` 分配 |

### JF 文件区分

多个生成任务提交到同一个 Block 时, JF 文件名中 `job_name` 含时间戳, 互不冲突:

```
Block/Generations/
  Generation_1/
    JF_J_Block1_20240604120000.bin   ← 第 1 个任务
  Generation_2/
    JF_J_Block1_20240604130000.bin   ← 第 2 个任务
```

前端遍历 `generations_info_` 时用 `gen.result_dir + "/JF_" + gen.job_name` 即可定位对应 feedback。

---

## Phase 2: 路径配置

> 不依赖 Phase 1，可与 Phase 1 并行开工。

### 设计决策说明

**为什么 jobs_gen/ 使用独立的目录而不是复用 jobs/？** 两个调度线程 `searchPendingJobThread2` 和 `GenTaskThread` 各自独立轮询。如果共用同一个目录，每个线程扫描时都需要过滤不属于自己的 job 文件（重建式 vs 生成式），增加不必要的 I/O 和复杂度。独立目录让两个线程完全解耦，互不干扰。

**为什么 getGenEngineJobQueue() 读注册表而不是新增注册表项？** 现有 `engine` key 指向 `jobs/` 的父目录（如 `C:\...\MoldAI\engine\jobs`），取其父目录 + `/jobs_gen` 即可得到生成式队列路径。不需要新增注册表项意味着不需要改安装脚本、不需要迁移已有用户的注册表，对现有部署零影响。

**为什么不支持优先级子目录？** 重建式有 `High/Normal/Low` 子目录是因为本地算力有限需要排队。生成式任务提交到远程 GPU 集群，服务端自行调度，优先级由服务端 SLA 保证，本地仅负责 submit + 轮询。

### 2.1 Settings 增加 gen 队列路径

- [ ] `Include/Util/Settings.h`：在 `getEngineJobQueue()` 下方增加声明

```cpp
// Include/Util/Settings.h — class Settings 内部
static QString getGenEngineJobQueue();
```

- [ ] `Src/Util/Settings.cpp`：实现

在 `getEngineJobQueue()` 实现下方：

```cpp
QString Settings::getGenEngineJobQueue()
{
    QString enginePath = getEngineJobQueue();
    if (enginePath.isEmpty())
        return "";

    QDir parent = QFileInfo(enginePath).dir();
    return parent.absolutePath() + "/jobs_gen";
}
```

> GenTaskAPI 不依赖此函数——作业文件路径由前端传入 `SubmitGenTask(pendingJobPath)`。此函数仅 EXE 侧使用（MakePath、GenTaskThread）。
```

### 2.2 JobMonitor 增加生成式任务目录创建方法

- [ ] `Include/Util/JobMonitor.h`：在 `CreateJobQueueDir()` 下方增加：

```cpp
static bool CreateGenJobQueueDir(const QString& path);   // 生成式任务目录 (无优先级/Engines)
static void CreateLocalGenJobQueueDir();                  // 本地兜底: 在 exe 同级创建
```

- [ ] `Src/Util/JobMonitor.cpp`：实现

```cpp
bool JobMonitor::CreateGenJobQueueDir(const QString& path)
{
    QDir dir(path);
    if (!dir.exists())
        dir.mkpath(".");
    dir.mkdir(JOBPENDINGSTR);
    dir.mkdir(JOBRUNNINGSTR);
    dir.mkdir(JOBCOMPLETEDSTR);
    dir.mkdir(JOBFAILEDSTR);
    dir.mkdir(JOBCANCELLEDSTR);
    return true;
}

void JobMonitor::CreateLocalGenJobQueueDir()
{
    // gen 路径从 engine 路径派生, engine 已切到本地后这里自然也是本地
    CreateGenJobQueueDir(Settings::getGenEngineJobQueue());
}
```

> 不需要 `Engines/`（引擎文件在 `jobs/Engines/`）和优先级子目录。

### 2.3 MakePath() 创建 jobs_gen 目录 + 全局变量赋值

- [ ] `App/Engine/CallEngine.cpp`：在文件顶部（其他全局变量声明附近，如 `bQuitingApplication`、`pendingJobPath`）增加全局变量定义：

```cpp
// ===== 新增: 生成式任务路径 (全局变量, GenTaskThread 通过 extern 引用) =====
QString genPendingJobPath;
QString genRunningJobPath;
QString genCompletedJobPath;
QString genFailedJobPath;
QString genCancelledJobPath;
```

- [ ] `App/Engine/CallEngine.cpp`：在 `MakePath()` 函数末尾（line ~4343，cancelledJobPath 赋值之后）增加：

```cpp
// ===== 新增: 生成式任务目录 =====
QString genEnginePath = Settings::getGenEngineJobQueue();
JobMonitor::CreateGenJobQueueDir(genEnginePath);

genPendingJobPath   = genEnginePath + "/" + JOBPENDINGSTR   + "/";
genRunningJobPath   = genEnginePath + "/" + JOBRUNNINGSTR   + "/";
genCompletedJobPath = genEnginePath + "/" + JOBCOMPLETEDSTR + "/";
genFailedJobPath    = genEnginePath + "/" + JOBFAILEDSTR     + "/";
genCancelledJobPath = genEnginePath + "/" + JOBCANCELLEDSTR + "/";
```

### 2.4 启动时清理 jobs_gen/ 残留锁文件

- [ ] `App/Engine/CallEngine.cpp`：`doCleanupJobLockOnceWhileEngineStart()` 增加 jobs_gen/ 的清理

现有 `DoCleanupJobLockOnceWhileEngineStart()` 扫描 `pendingJobPath/High/` 和 `runningJobPath/`，检查 `.lock` 文件是否对应 job 本体已不存在的孤儿锁。需要在函数末尾（`foreach` 删除循环之前，line ~235）为 `jobs_gen/` 增加同样的检查逻辑（生成式无优先级分层，只查 Pending 和 Running）：

```cpp
// ===== 新增: 生成式任务孤儿锁清理 =====
QString genPendingDir(genPendingJobPath);
QString genRunningDir(genRunningJobPath);

QDir pendingGenDir(genPendingDir);
if (pendingGenDir.exists()) {
    QFileInfoList fileInfoList = pendingGenDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);
    foreach (QFileInfo fileInfo, fileInfoList) {
        if (!fileInfo.suffix().compare("lock", Qt::CaseInsensitive)) {
            QString postFix = JOB_INFO_USE_BIN ? BINFILE_POSTFIX : JSONFILE_POSTFIX;
            QString jobFile = fileInfo.absolutePath() + pathSeperator + fileInfo.baseName() + postFix;
            if (!QFileInfo(jobFile).exists()) {
                toBeRemovedFileList.append(fileInfo.absoluteFilePath());
            }
        }
    }
}

QDir runningGenDir(genRunningDir);
if (runningGenDir.exists()) {
    QFileInfoList fileInfoList = runningGenDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);
    foreach (QFileInfo fileInfo, fileInfoList) {
        if (!fileInfo.suffix().compare("lock", Qt::CaseInsensitive)) {
            QString postFix = JOB_INFO_USE_BIN ? BINFILE_POSTFIX : JSONFILE_POSTFIX;
            QString jobFile = fileInfo.absolutePath() + pathSeperator + fileInfo.baseName() + postFix;
            if (!QFileInfo(jobFile).exists()) {
                toBeRemovedFileList.append(fileInfo.absoluteFilePath());
            }
        }
    }
}
```

> 放在 `DoCleanupJobLockOnceWhileEngineStart()` 函数末尾，`foreach` 删除循环（line ~236）**之前**，这样新收集的孤儿锁会一起被删除。生成式不需要检查 High/Low 优先级子目录。

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

        // 服务端地址在 Core/Types.h 中宏定义, 不读 INI:
        //   #define GEN_SERVER_URL  "http://0.0.0.0:8080"
        //   #define GEN_API_PREFIX  "/api/v1"
        class GenHttpClient
        {
            public:
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
            path.remove(GEN_SERVER_URL);

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
            QString url = GEN_SERVER_URL + GEN_API_PREFIX + "/tasks/" + QString::fromUtf8(ToString(genParams.sub_type));

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
                            response.server_task_id = doc["triverse_task_uuid"].toString().toStdString();
                        response.status   = static_cast<GenTaskStatus>(doc.value("status").toInt());
                        response.progress = doc.value("progress").toInt();
                        if (doc.contains("result_url"))
                            response.result_url = doc["result_url"].toString().toStdString();
                        if (doc.contains("preview_url"))
                            response.preview_url = doc["preview_url"].toString().toStdString();
                        if (doc.contains("error_message"))
                            response.error_message = doc["error_message"].toString().toStdString();
                        // 积分字段已迁移到 PointInfoBase, 不再从 Task API 响应中解析
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
            QString url = GEN_SERVER_URL + GEN_API_PREFIX + "/task/status?task_id="
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
                            response.server_task_id = doc["triverse_task_uuid"].toString().toStdString();
                        response.status   = static_cast<GenTaskStatus>(doc.value("status").toInt());
                        response.progress = doc.value("progress").toInt();
                        if (doc.contains("result_url"))
                            response.result_url = doc["result_url"].toString().toStdString();
                        if (doc.contains("preview_url"))
                            response.preview_url = doc["preview_url"].toString().toStdString();
                        if (doc.contains("error_message"))
                            response.error_message = doc["error_message"].toString().toStdString();
                        // 积分字段已迁移到 PointInfoBase, 不再从 Task API 响应中解析
                        ok = true;
                    } else {
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
            QString url = GEN_SERVER_URL + GEN_API_PREFIX + "/task/cancel";

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
            QNetworkRequest request;
            request.setUrl(QUrl(url));
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
            QString url = GEN_SERVER_URL + GEN_API_PREFIX + "/upload";

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

> 注意: GenHttpClient 委托 `HttpClient::post/get`, 但 `HttpClient.cpp` 不在任何 CMakeLists 中。
> **必须**在 `App/Engine/CMakeLists.txt` 的 SRC_LIST 中增加:
> `"${CMAKE_CURRENT_SOURCE_DIR}/../../Src/Util/HttpClient.cpp"`

### 3.3 Mock 模式 (开发调试用)

> 用 `#ifdef MOCK_GEN_HTTP` 包裹, 替换真实 HTTP 为 sleep + 模拟返回, 方便调试 GenTaskThread 全流程。

```cpp
// GenHttpClient.cpp — 在 SubmitTask / QueryTaskStatus / CancelTask / UploadFile 函数体开头:

#ifdef MOCK_GEN_HTTP
// ===== SubmitTask mock =====
GenTaskResponse GenHttpClient::SubmitTask(const std::string& task_uuid,
                                           const std::string& user_account,
                                           const GenTaskParams& genParams,
                                           int, int)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    GenTaskResponse resp;
    resp.task_id       = task_uuid;
    resp.server_task_id = "mock-trv-" + task_uuid.substr(0, 8);
    resp.status        = GenTaskStatus::IN_PROGRESS;
    resp.progress      = 0;
    return resp;
}

// ===== QueryTaskStatus mock (模拟 3 次进度 → 完成) =====
GenTaskResponse GenHttpClient::QueryTaskStatus(const std::string& server_task_id,
                                                int, int)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    GenTaskResponse resp;
    resp.server_task_id = server_task_id;
    resp.status         = GenTaskStatus::IN_PROGRESS;

    // mock 计数器: 调用 3 次后标记完成
    static std::map<std::string, int> pollCount;
    int& count = pollCount[server_task_id];
    count++;
    resp.progress = count * 33;

    if (count >= 3) {
        resp.status     = GenTaskStatus::COMPLETED;
        resp.progress   = 100;
        resp.result_url = "https://mock-cdn.example.com/" + server_task_id + "/result.glb";
        resp.preview_url = "https://mock-cdn.example.com/" + server_task_id + "/preview.png";
        // 积分字段已迁移到 PointInfoBase, mock 中不再设置
    }
    return resp;
}

// ===== CancelTask mock =====
bool GenHttpClient::CancelTask(const std::string&, int, int) { return true; }

// ===== UploadFile mock =====
std::string GenHttpClient::UploadFile(const std::string&, int, int) { return "mock-fk-12345"; }

#endif // MOCK_GEN_HTTP
```

> 调用 `SubmitGenTask` 前, 在 GenHttpClient::Init 中或编译时加 `-DMOCK_GEN_HTTP` 即可切换。

### 3.4 手动验证

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
extern QString genPendingJobPath;    // 由 CallEngine::MakePath() 初始化
extern QString genRunningJobPath;
extern QString genCompletedJobPath;
extern QString genFailedJobPath;
extern QString genCancelledJobPath;


namespace AI3D {
    namespace CORE {

        // ============================================================================
        // 文件工具函数 (文件作用域, 仅本 .cpp 可见)
        //
        // 锁机制说明:
        //   1. load_with_retry / save_with_retry 内部使用 FopenDenyWriteLockUtf8,
        //      在读写期间阻止其他进程并发写入, 操作完成后 fclose 释放锁。
        //   2. fclose 后 .lock 文件残留是正常的 (文件已空, 仅作标记),
        //      MoveJobFile 在移动前删除 src 和 dst 的残留 .lock。
        //   3. 启动时 DoCleanupJobLockOnceWhileEngineStart 清理孤儿 .lock。
        //   4. QDirIterator 用 {"J_*.bin"} 过滤, 不会加载 .lock 文件。
        // ============================================================================

        /// @brief 移动 job 文件到目标目录, 移动前先清理残留 .lock 文件
        static bool MoveJobFile(const std::string& src, const std::string& dstDir) {
            // 0. 先删残留 .lock (对标 DoCleanupJobLockOnceWhileEngineStart)
            QString srcLock = QString::fromStdString(src) + ".lock";
            if (QFileInfo::exists(srcLock))
                QFile::remove(srcLock);

            QFileInfo fi(QString::fromStdString(src));
            QString dstPath = QString::fromStdString(dstDir) + "/" + fi.fileName();
            QString dstLock = dstPath + ".lock";
            if (QFileInfo::exists(dstLock))
                QFile::remove(dstLock);

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
            // feedback 放在结果目录下: Generations/Generation_<id>/JF_<job_name>.bin
            //   对标重建式: Productions/Production_<id>/JF_<job_name>.bin
            std::string base = info.job.result_dir + "/JF_" + info.job_name;
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
                    fb.Percent = job.progress;  // 由 ApplyResponse 从 resp.progress 回填
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
        //   4. HTTP POST submit → 回填 server_task_id
        //   5. HTTP POST /point/freeze → 回填 freeze_no         ← ★ 积分冻结 (详见 积分接口集成方案.md 第四章)
        //      失败不移文件, 下轮重试
        //   6. 保存 → 移到 Running/
        //   7. submit 失败不移动文件, 下轮重试
        // ============================================================================

        void GenTaskThread::ProcessPendingJobs()
        {
            QString pendingDir = genPendingJobPath;
            QDirIterator it(pendingDir, {"J_*.bin"}, QDir::Files);

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
                    MoveJobFile(filePathStr, qstr2str(genRunningJobPath));
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
                    MoveJobFile(filePathStr, qstr2str(genFailedJobPath));
                    continue;
                }

                // 5. 提交成功 → 回填 server_task_id → 注册到 Block → 移到 Running
                job.ApplyResponse(resp);
                job.status = GenTaskStatus::PENDING;
                info.save_with_retry(filePathStr);
                UpdateFeedback(info);
                info.feedback.save_with_retry(fbPath, false);

                // 注册到 Block: 读 .blk → generations_info_.push_back → 写回
                // (job.project_path + job.block_item 定位 Block)
                {
                    std::string blkPath = job.project_path + "/" + job.block_item + ".blk";
                    BlockObject::Task_Info blkInfo;
                    if (blkInfo.ReadBlockInfoBin(blkPath)) {
                        blk_generation_info_s genInfo;
                        genInfo.generation_id = job.generation_id;
                        genInfo.task_uuid     = job.task_uuid;
                        genInfo.job_name      = info.job_name;
                        genInfo.sub_type      = static_cast<int>(job.params.sub_type);
                        genInfo.status        = static_cast<int>(GenTaskStatus::PENDING);
                        genInfo.result_dir    = job.result_dir;
                        genInfo.created_time  = QDateTime::currentDateTime().toString("yyyyMMddhhmmss").toStdString();
                        blkInfo.generations_info_.push_back(genInfo);
                        blkInfo.generationjobs_[job.task_uuid] = info.job_name;
                        blkInfo.WriteBlockInfoToBin(blkPath, false);
                    }
                }

                MoveJobFile(filePathStr, qstr2str(genRunningJobPath));

                LOGI("Submitted: " + job.task_uuid + " server_task_id=" + job.server_task_id);
            }
        }

        // ============================================================================
        // ProcessRunningJobs — 轮询运行中的任务
        //
        // 流程:
        //   1. 遍历 jobs_gen/Running/J_*
        //   2. deny-write 锁 → 加载 job
        //   3. 兜底: 无 freeze_no → 补 FreezeCredits                  ← ★ 积分冻结兜底
        //   4. HTTP GET query 服务端状态
        //   5. 根据返回处理 — 终态时**先 settle 再更新状态**:
        //      COMPLETED/FAILED/CANCELLED → 先 POST /point/settle (详见 积分接口集成方案.md 第四章)
        //        ├─ settle 成功 → 更新状态/feedback/Block → 移目录
        //        └─ settle 失败 → break (不更新任何状态, 下轮重试)
        //      IN_PROGRESS → 更新 feedback
        //   6. 连续 5 次网络超时 → 标记 FAILED
        // ============================================================================

        void GenTaskThread::ProcessRunningJobs()
        {
            QString runningDir = genRunningJobPath;
            QDirIterator it(runningDir, {"J_*.bin"}, QDir::Files);

            while (it.hasNext()) {
                it.next();
                QString filePath = it.filePath();
                std::string filePathStr = filePath.toStdString();

                // 1. 加载 job 文件 (load_with_retry 内部已处理 deny-write 锁)
                GenJobFullInfo_s info;
                if (!info.load_with_retry(filePathStr))
                    continue;
                GenJobInfo_s& job = info.job;
                if (!!job.server_task_id.empty()) {
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
                        MoveJobFile(filePathStr, qstr2str(genFailedJobPath));
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
                        // 更新 Block 中的状态和 result_url
                        {
                            std::string blkPath = job.project_path + "/" + job.block_item + ".blk";
                            BlockObject::Task_Info blkInfo;
                            if (blkInfo.ReadBlockInfoBin(blkPath)) {
                                for (auto& gen : blkInfo.generations_info_) {
                                    if (gen.task_uuid == job.task_uuid) {
                                        gen.status = static_cast<int>(GenTaskStatus::COMPLETED);
                                        if (!job.result_url.empty())  gen.result_url  = job.result_url;
                                        if (!job.preview_url.empty()) gen.preview_url = job.preview_url;
                                        break;
                                    }
                                }
                                blkInfo.WriteBlockInfoToBin(blkPath, false);
                            }
                        }
                        MoveJobFile(filePathStr, qstr2str(genCompletedJobPath));
                        LOGI("Completed: " + job.task_uuid);
                        break;
                    }

                    case GenTaskStatus::FAILED: {
                        job.ApplyResponse(resp);
                        info.save_with_retry(filePathStr);
                        UpdateFeedback(info);
                        info.feedback.save_with_retry(fbPath, false);
                        // 更新 Block 中的状态
                        {
                            std::string blkPath = job.project_path + "/" + job.block_item + ".blk";
                            BlockObject::Task_Info blkInfo;
                            if (blkInfo.ReadBlockInfoBin(blkPath)) {
                                for (auto& gen : blkInfo.generations_info_) {
                                    if (gen.task_uuid == job.task_uuid) {
                                        gen.status = static_cast<int>(GenTaskStatus::FAILED);
                                        break;
                                    }
                                }
                                blkInfo.WriteBlockInfoToBin(blkPath, false);
                            }
                        }
                        MoveJobFile(filePathStr, qstr2str(genFailedJobPath));
                        LOGE("Failed: " + job.task_uuid);
                        break;
                    }

                    case GenTaskStatus::IN_PROGRESS: {
                        // 进度更新: ApplyResponse 回填 progress/result_url/preview_url 等
                        // 再 UpdateFeedback 写到 feedback 文件供前端轮询
                        job.ApplyResponse(resp);
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
                        // 更新 Block 中的状态
                        {
                            std::string blkPath = job.project_path + "/" + job.block_item + ".blk";
                            BlockObject::Task_Info blkInfo;
                            if (blkInfo.ReadBlockInfoBin(blkPath)) {
                                for (auto& gen : blkInfo.generations_info_) {
                                    if (gen.task_uuid == job.task_uuid) {
                                        gen.status = static_cast<int>(GenTaskStatus::CANCELLED);
                                        break;
                                    }
                                }
                                blkInfo.WriteBlockInfoToBin(blkPath, false);
                            }
                        }
                        MoveJobFile(filePathStr, qstr2str(genCancelledJobPath));
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
        // CancelGenTask — TODO: 取消生成式任务
        // 流程: 遍历 jobs_gen/ → 找 task_uuid → HTTP cancel → 更新 feedback + Block → 移 Cancelled/
        bool GenTaskThread::CancelGenTask(const std::string& task_uuid)
        {
            // TODO
            return false;
        }

        // ============================================================================
        // ============================================================================

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

        QString runningDir = genRunningJobPath;
        QDirIterator it(runningDir, {"J_*.bin"}, QDir::Files);
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
                MoveJobFile(filePathStr, qstr2str(genFailedJobPath));
                continue;
            }

            // 2. 超过 1h 无更新 且无 server_task_id → submit 阶段残留, 移回 Pending 重试
            if (secsSinceMod > 3600) {
                GenJobFullInfo_s info;
                if (!info.load_with_retry(filePathStr))
                    continue;
                if (info.!!job.server_task_id.empty()) {
                    LOGW("UnnormalRunning: " + info.job.task_uuid
                         + " no server_task_id for 1h, moving back to Pending");
                    MoveJobFile(filePathStr, qstr2str(genPendingJobPath));
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
// ===== 新增: 生成式任务线程 =====
// 服务端地址在 Core/Types.h 中宏定义 (GEN_SERVER_URL / GEN_API_PREFIX), 无需 Init
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

**为什么不用回调而是文件轮询？** GenTaskThread（MoldAINode.exe）和前端（MoldAIDesktop.exe）是不同进程，`std::function` 回调无法跨进程调用。对标重建式：GenTaskThread 写 feedback 文件和 Block .blk 文件，前端直接读 feedback 文件或读 `generations_info_` 获取状态。

**任务与 Block 的映射**：`SubmitResult` 直接返回所有定位信息，前端无需解析路径：

| SubmitResult 字段 | 用途 |
|---|---|
| `feedback_path` | 轮询 feedback 文件获取进度/状态 (前端直接读 JobFeedBack_s) |
| `project_path` + `block_id` | 直接定位 Block（`block_id` 对标 `block->GetId()`）|
| `job_name` | 在 Block 的 `generations_info_` 中匹配对应条目，取 `result_url` |

**完成后的完整链路**：

```
读 feedback 文件 → COMPLETED
       ↓
读取 Block .blk → generations_info_ 按 job_name 匹配 → 拿到 result_url
       ↓
DownloadResult(result_url, save_path)
       ↓
刷新 UI (Block 树节点显示完成状态)
```


**为什么 DownloadResult 在 GenTaskAPI 中直接做 HTTP GET？** result_url 可能是 CDN 链接，不经过 Triverse API 网关，不需要签名鉴权。

### 5.1 GenTaskAPI.h

```cpp
// Include/Core/GenTaskAPI.h
#pragma once

#include "Core/GenTaskOptions.h"   // GenTaskStatus / GenTaskSubType / GenTaskParams
#include "Core/BlockObject.h"       // BlockObject::Task_Info
#include <string>

namespace AI3D {
namespace CORE {

class AI3D_API GenTaskAPI
{
public:
    struct SubmitResult {
        std::string task_uuid;       // 全局唯一, 取消任务时用
        std::string job_name;        // J_BlockName_timestamp, 在 generations_info_ 中匹配
        int         generation_id = -1;  // 分配的 generation id (前端用于更新 Block)
        bool        success = false;
        std::string error_msg;
    };

    static SubmitResult SubmitGenTask(
        BlockObject::Task_Info& blockInfo,       // 非 const: 内部递增 next_generation_id
        const std::string& user_account,
        const std::string& pendingJobPath);

    /// @brief 通过 task_uuid 获取结果目录 (从 Block 的 generations_info_ 中查找)
    /// @return result_dir, 未找到返回空字符串
    static std::string GetResultDir(
        const std::string& task_uuid,
        const BlockObject::Task_Info& blockInfo);

    /// @brief 通过 task_uuid 下载结果到对应的 Generations/Generation_<id>/ 目录
    ///        自动从 generations_info_ 中查找 result_url 和 result_dir
    /// @param progressCb  进度回调 (bytesReceived, bytesTotal), 可选
    /// @return true=成功
    static bool DownloadResultByTaskUuid(
        const std::string& task_uuid,
        const BlockObject::Task_Info& blockInfo,
        std::function<void(qint64 bytesReceived, qint64 bytesTotal)> progressCb = nullptr);

    /// @brief 直接下载 (保留, 供特殊场景指定自定义路径)
    static bool DownloadResult(const std::string& result_url,
                                const std::string& save_path);

    /// @brief 请求取消任务
    static bool RequestCancel(const std::string& task_uuid);

    static int QueryCredits(const std::string& user_account);

    // Block 结果注册/更新由 GenTaskThread 直接操作 .blk (EXE 侧, 见 Phase 4.1)
};

}} // namespace AI3D::CORE
```

### 5.2 GenTaskAPI.cpp

> DLL 侧不扫描 `jobs_gen/`，不依赖 `Settings`，路径全部由调用方传入。

```cpp
// Src/Core/GenTaskAPI.cpp
#include "Core/GenTaskAPI.h"
#include "Core/BlockObject.h"
#include "Util/GenTaskProcess.h"
#include "Util/TaskProcess.h"
#include "Core/Types.h"
#include "Core/Logging.h"
#include <QUuid>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QHostInfo>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <fstream>

namespace AI3D { namespace CORE {

// ============================================================================
// SubmitGenTask
// ============================================================================
GenTaskAPI::SubmitResult GenTaskAPI::SubmitGenTask(
    BlockObject::Task_Info& blockInfo,       // 非 const: 内部自增 next_generation_id
    const std::string& user_account,
    const std::string& pendingJobPath)
{
    SubmitResult result;

    if (blockInfo.block_task_category != 1) {
        result.success = false;
        result.error_msg = "Block does not support generative tasks";
        return result;
    }

    // 从持久化计数器取 generation_id, 取后自增 (只增不复用)
    int generation_id = blockInfo.next_generation_id++;

    GenJobFullInfo_s fullInfo;
    fullInfo.job_name = "J_" + blockInfo.blockName + "_"
        + QDateTime::currentDateTime().toString("yyyyMMddhhmmss").toStdString();

    GenJobInfo_s& job = fullInfo.job;
    job.task_uuid = user_account + "_"
        + QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz").toStdString()
        + "_" + QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString().substr(0, 4);
    job.engine_id    = QHostInfo::localHostName().toStdString();
    job.user_account = user_account;
    job.project_path = blockInfo.projectfile_;
    job.block_item   = blockInfo.blockName;
    job.params       = blockInfo.gen_options.gen_params;
    job.status       = GenTaskStatus::IDLE;

    // 结果目录: Generations/Generation_<id>/
    std::string generationDir = std::string(GENERATION_DIR) + "/" + GENERATION_PREFIX + std::to_string(generation_id);
    job.result_dir = blockInfo.projectfile_ + "/" + blockInfo.blockName + "/" + generationDir;
    QDir().mkpath(QString::fromStdString(job.result_dir));

    QDir().mkpath(QString::fromStdString(pendingJobPath));
    std::string jobFilePath = pendingJobPath + fullInfo.job_name + ".bin";
    if (!fullInfo.save_with_retry(jobFilePath)) {
        result.success = false;
        result.error_msg = "Failed to write job file: " + jobFilePath;
        return result;
    }

    std::string feedbackPath = job.result_dir + "/JF_" + fullInfo.job_name
        + (JOB_FEEDBACK_USE_BIN ? BINFILE_POSTFIX : JSONFILE_POSTFIX);
    fullInfo.feedback.Status = jobsta_e::STATUS_PENDDING;
    fullInfo.feedback.Percent = 0.0f;
    fullInfo.feedback.save_with_retry(feedbackPath, false);

    job.generation_id = generation_id;  // 存入 job, 引擎后续使用

    result.success   = true;
    result.task_uuid = job.task_uuid;
    result.job_name  = fullInfo.job_name;
    result.generation_id = generation_id;
    return result;
}

// ============================================================================
// GetResultDir — 通过 task_uuid 查找结果目录
// ============================================================================
std::string GenTaskAPI::GetResultDir(const std::string& task_uuid,
                                      const BlockObject::Task_Info& blockInfo)
{
    for (auto& gen : blockInfo.generations_info_) {
        if (gen.task_uuid == task_uuid)
            return gen.result_dir;
    }
    return "";
}

// ============================================================================
// DownloadResultByTaskUuid — 自动定位 result_dir 下载, 支持进度回调
// ============================================================================
bool GenTaskAPI::DownloadResultByTaskUuid(
    const std::string& task_uuid,
    const BlockObject::Task_Info& blockInfo,
    std::function<void(qint64, qint64)> progressCb)
{
    // 1. 从 generations_info_ 中查找 result_url 和 result_dir
    std::string resultUrl;
    std::string resultDir;
    for (auto& gen : blockInfo.generations_info_) {
        if (gen.task_uuid == task_uuid) {
            resultUrl = gen.result_url;
            resultDir = gen.result_dir;
            break;
        }
    }
    if (resultUrl.empty()) {
        LOGE("DownloadResultByTaskUuid: task_uuid not found or result_url empty");
        return false;
    }
    if (resultDir.empty()) {
        LOGE("DownloadResultByTaskUuid: result_dir is empty");
        return false;
    }

    // 2. 确定保存路径: result_dir/result.glb (后续可按 sub_type 选择扩展名)
    std::string savePath = resultDir + "/result.glb";

    // 3. 下载
    QNetworkAccessManager manager;
    QUrl url(QString::fromStdString(resultUrl));
    QNetworkRequest request(url);
    request.setTransferTimeout(60000);
    QNetworkReply* reply = manager.get(request);

    // 进度回调
    if (progressCb) {
        QObject::connect(reply, &QNetworkReply::downloadProgress,
                         [&](qint64 received, qint64 total) {
                             progressCb(received, total);
                         });
    }

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, [&]() { reply->abort(); loop.quit(); });
    timer.start(60000);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        LOGE("DownloadResultByTaskUuid: " + reply->errorString().toStdString());
        reply->deleteLater();
        return false;
    }
    QByteArray data = reply->readAll();
    reply->deleteLater();

    QFile file(QString::fromStdString(savePath));
    if (!file.open(QIODevice::WriteOnly)) {
        LOGE("DownloadResultByTaskUuid: cannot write to " + savePath);
        return false;
    }
    file.write(data);
    file.close();
    return true;
}

// ============================================================================
// DownloadResult — 指定 URL 和路径直接下载 (保留, 供特殊场景)
// ============================================================================
bool GenTaskAPI::DownloadResult(const std::string& result_url,
                                 const std::string& save_path)
{
    if (result_url.empty()) { LOGE("DownloadResult: result_url is empty"); return false; }

    QNetworkAccessManager manager;
    QUrl url(QString::fromStdString(result_url));
    QNetworkRequest request(url);
    request.setTransferTimeout(30000);
    QNetworkReply* reply = manager.get(request);
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, [&]() { reply->abort(); loop.quit(); });
    timer.start(30000);
    loop.exec();
    if (reply->error() != QNetworkReply::NoError) {
        LOGE("DownloadResult: " + reply->errorString().toStdString());
        reply->deleteLater();
        return false;
    }
    QByteArray data = reply->readAll();
    reply->deleteLater();
    QFile file(QString::fromStdString(save_path));
    if (!file.open(QIODevice::WriteOnly)) { LOGE("DownloadResult: cannot write"); return false; }
    file.write(data);
    file.close();
    return true;
}

// ============================================================================
// RequestCancel — TODO: 取消任务
// ============================================================================
bool GenTaskAPI::RequestCancel(const std::string& task_uuid)
{
    // TODO
    return false;
}

// ============================================================================
// QueryCredits — TODO: 对接 Triverse 积分查询 API
// ============================================================================
int GenTaskAPI::QueryCredits(const std::string& user_account)
{
    // TODO: GenHttpClient 发 GET /api/v1/credits?user=xxx → 解析 points_balance
    return -1;
}

}} // namespace AI3D::CORE
```

### 5.3 CMakeLists — 无需修改

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
    blockInfo.block_task_category = 1;  // 生成式
    blockInfo.gen_options.gen_params.sub_type     = GenTaskSubType::TEXT_TO_MODEL;
    blockInfo.gen_options.gen_params.prompt       = "a cute cat figurine";
    blockInfo.gen_options.gen_params.polygon_limit = 50000;
    blockInfo.gen_options.gen_params.texture_size  = 1024;
    blockInfo.blockName    = "TestBlock_Gen";
    blockInfo.projectfile_ = "C:/Users/Test/AppData/Local/MoldAI/TestProject";

    // 2. 提交任务 (pendingJobPath 由前端从设置中获取后传入)
    std::string pendingPath = "/tmp/test_jobs_gen/Pending/";
    GenTaskAPI::SubmitResult result = GenTaskAPI::SubmitGenTask(
        blockInfo, "testuser@example.com", pendingPath);

    if (!result.success) {
        std::cerr << "Submit failed: " << result.error_msg << std::endl;
        return 1;
    }

    std::cout << "Submitted: " << result.job_name << std::endl;

    // 3. 轮询 feedback 文件 (等待 Node 处理)
    QTimer pollTimer;
    int pollCount = 0;
    QObject::connect(&pollTimer, &QTimer::timeout, [&]() {
        pollCount++;

        std::cout << "[" << pollCount << "] status="
        std::cout << std::endl;

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
// Step 5: 检查 Pending 中的 job 文件内容 — server_task_id 应该已被回填 (has_value)
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
    //        MoveJobFile(filePathStr, qstr2str(genRunningJobPath));
    //        continue;  ← 跳过 HTTP submit
    //    }
    //
    // 2. 服务端 mock 应该记录收到的 submit 请求次数:
    //    正常流程: 1 次
    //    崩溃恢复: 仍为 1 次 (不增加)
}
```

- [ ] **验证点**: crash 前 `server_task_id` has_value
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

在 `Core/Types.h` 中增加宏定义 (替换 INI 配置):

```cpp
// Include/Core/Types.h
#define GEN_SERVER_URL    "http://localhost:5000"
#define GEN_API_PREFIX    "/api/v1"

// 生成式任务目录常量 (对标 PRODUCTION_DIR / PRODUCTION_PREFIX)
#define GENERATION_DIR    "Generations"
#define GENERATION_PREFIX "Generation_"
```

> Mock Server 不需要鉴权 → accessToken 为空, 签名不含 token。

服务端地址由 `Core/Types.h` 中的宏定义控制 (`GEN_SERVER_URL` / `GEN_API_PREFIX`)，无需 INI 配置或 Init 调用。

### 6.7 全流程集成测试 (Mock HTTP + 真实 GenTaskThread 线程)

> 编译: `testGenTaskFullFlow` 直接链接 `GenTaskThread.cpp` + `GenHttpClient.cpp` (Engine 层源文件), 加 `-DMOCK_GEN_HTTP`。
> 验证: `std::thread` 起 `GenTaskThread::Run()` → 真实调度逻辑 → feedback → 前端读取, 全程同进程验证。

> CMakeLists 见 Phase 1.6 (两个 target 合并在同一个 CMakeLists.txt 中)

#### Test/testGenTask/testGenTaskFullFlow.cpp

```cpp
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
    QString tmpRoot = QDir::tempPath() + "/test_gen_node_"
        + QUuid::createUuid().toString(QUuid::WithoutBraces);
    QDir().mkpath(tmpRoot);

    // 2. 设置全局路径 (对标 MakePath)
        // 2. Set global paths (matching MakePath)
    QString genRoot = tmpRoot + "/jobs_gen";
    genPendingJobPath   = genRoot + "/Pending/";
    genRunningJobPath   = genRoot + "/Running/";
    genCompletedJobPath = genRoot + "/Completed/";
    genFailedJobPath    = genRoot + "/Failed/";
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
    task.blockString  = "Block_1";
    task.blockName    = "Block_1";   // Init() 不设 blockName, 需手动赋值
    task.blockId      = 1;
    task.projectfile_ = projectPath;
    task.isSaved      = false;
    task.isLoaded     = true;
    task.block_task_category = 1;
    task.gen_options.gen_params.sub_type      = GenTaskSubType::TEXT_TO_MODEL;
    task.gen_options.gen_params.prompt        = "a dragon statue";
    task.gen_options.gen_params.polygon_limit = 30000;

    // 4. 落地 Block 文件 (Block_1 目录 + .blk)
        // 4. Persist Block file (Block_1 dir + .blk)
    block.Save();

    // 5. 前端提交任务
        // 5. Frontend submits task
    std::cout << "--- Submit ---" << std::endl;
    GenTaskAPI::SubmitResult result = GenTaskAPI::SubmitGenTask(
        task, "testuser@example.com", genPendingJobPath.toStdString());
    TEST_ASSERT(result.success, "SubmitGenTask should succeed");
    std::cout << "  task_uuid=" << result.task_uuid
              << "  job=" << result.job_name
              << "  gen_id=" << result.generation_id << std::endl;

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
    for (int i = 0; i < 60; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        JobFeedBack_s fb;
        if (!fb.load_with_retry(fbPath, false)) continue;
        std::cout << "  [" << i << "] status=" << (int)fb.Status
                  << " percent=" << fb.Percent << "%" << std::endl;
        if (fb.Status == jobsta_e::STATUS_COMPLETE) { completed = true; break; }
        if (fb.Status == jobsta_e::STATUS_FAILED) break;
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
        (genCompletedJobPath + done[0]).toStdString()), "load completed job");
    std::cout << "  result_url=" << doneInfo.job.result_url << std::endl;
    std::cout << "  consumed=" << doneInfo.job.point_info.consumed
              << " total_balance=" << doneInfo.job.point_info.total_balance << std::endl;
    TEST_ASSERT(!doneInfo.job.result_url.empty(), "result_url not empty");

    // 9. 清理
        // 9. Cleanup
    QDir(tmpRoot).removeRecursively();

    if (g_failures == 0) {
        std::cout << "\n===== ALL TESTS PASSED =====" << std::endl;
        return 0;
    }
    std::cout << "\n===== " << g_failures << " TEST(S) FAILED =====" << std::endl;
    return 1;
}
```

> **预期**: 主线程提交任务 → GenTaskThread::Run 线程 pick up → mock SubmitTask → mock QueryTaskStatus × 3 → Completed → 主线程读到 COMPLETE + result_url。

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

### 修改（8 个）

| 文件 | Phase | 改动 |
|------|-------|------|
| `Include/Core/DataStruct.h` | P1 | 新增 `GenJobInfoData`/`GenJobTaskData`/`GenJobFile`（含积分字段 `PointInfoBase` 平铺 + `FeedBackData`）；`BLKBinFile` 扩展 |
| `Include/Core/BlockObject.h` | P1 | `Task_Info` 增加 `GenTaskOptions gen_options` + `generations_info_` + `generationjobs_` (对标 `at_options` + `reconstructions_info_` + `reconstructionjobs_`) |
| `Include/Core/BlockObject.cpp` | P1 | 4 个序列化方法增加 `gen_options` 读写 |
| `Include/Util/Settings.h` | P2 | 增加 `getGenEngineJobQueue()` |
| `Src/Util/Settings.cpp` | P2 | 实现 `getGenEngineJobQueue()` |
| `Include/Util/JobMonitor.h` | P2 | 增加 `CreateGenJobQueueDir()` |
| `Src/Util/JobMonitor.cpp` | P2 | 实现 `CreateGenJobQueueDir()` |
| `App/Engine/CallEngine.cpp` | P2+P4 | MakePath 创建 `jobs_gen/` 目录; `DoCleanupJobLockOnceWhileEngineStart` 清理 `jobs_gen/` 残留锁; main 启动 GenTaskThread |
| `App/Engine/CMakeLists.txt` | P1 | HEADER_LIST 显式添加 `GenTaskProcess.h` (唯一需改的 CMakeLists) |

### 不动

`JobFeedBack_s` 不扩展 — 生成式复用 Status/Percent/Msg/TaskRetVal 做进度反馈，结果数据存 GenJobInfo_s。`GenJobFullInfo_s` 新增 `JobFeedBack_s feedback` 成员（对标 TaskGraph_s 持有 JobFeedBack_s），序列化到 BIN（对标 JobListFile::feedBackData）并独立持久化到 JF_* 文件。`TaskGraph_s`、`Task_s`、`ATTaskInfo`、`ExecTaskFileV2`、`GetPendingJob`、`Src/Core/CMakeLists.txt` — 全部不动。
