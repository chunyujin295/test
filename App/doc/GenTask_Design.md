# 生成式模型任务系统 — 设计方案

## 一、需求概述

在现有本地分布式计算任务系统基础上, 平行增加一个**三方生成式模型任务**系统。核心差异:

| 维度 | 本地计算任务 (现有) | 生成式任务 (新增) |
|------|-------------------|-------------------|
| 执行位置 | 本地 MoldAITask.exe | 远程三方服务器 |
| 通信方式 | 文件系统 (feedback) | HTTP (submit + query) |
| 进程模型 | Node spawn Task 子进程 | Node 线程直接发 HTTP |
| 工作目录 | `jobs/` | `jobs_gen/` |
| 任务量 | 不定 | ~10 个级别 |
| 进度粒度 | 多阶段递增 (5/15/30/60/90/100) | 可能只有开始和完成 |

---

## 二、可行性评估

### 2.1 复用文件调度机制的合理性

**结论: 基本可行, 但有明确边界条件。**

**可行的理由:**
- 现有系统的文件锁协议已被验证 (Windows deny-write sharing), 直接复用
- 目录即状态机的设计简单可靠 (Pending → Running → Completed/Failed/Cancelled)
- 系统任务量小 (~10), 顺序轮询足够, 不需要复杂的并发调度
- 前后端已理解文件 IPC 模式, 团队学习成本低

**不可行的场景 (需要关注):**
- 任务量增长到 100+ 时, 顺序轮询延迟会线性增长
- 如果服务端需要毫秒级提交响应, 文件系统轮询的 1s 粒度不够
- 如果需要服务端主动推送进度 (webhook), 文件系统是纯拉模型

### 2.2 核心风险分析

#### 风险 1: 重复提交 (高概率, 中影响)

**场景**: Node 从 Pending 读取 job 文件 → HTTP submit 成功 → 在移动到 Running 之前 Node 崩溃 → 重启后再次读取同一个 Pending 文件 → 再次 submit。

**当前文件 IPC 方案的风险放大**: 因为 Pending → Running 的文件移动不是原子的 (save to Running → sleep → verify exists → delete from Pending), 在这个窗口期崩溃会导致任务被提交两次。

**缓解方案**:
1. **客户端生成唯一 task_uuid**, 写入 job 文件, submit 时发送给服务端
2. 服务端以 task_uuid 做幂等: 相同 uuid 的重复提交返回已有结果而非新建
3. Node 在 submit 前先检查 job 文件中的 `server_task_id` 字段是否已存在, 避免崩溃恢复后的重复提交

#### 风险 2: 状态不一致 (中概率, 高影响)

**场景**: HTTP submit 成功, 但移动文件到 Running 时磁盘满 / 网络路径断开, 导致文件移动失败。

**缓解方案**:
- 严格的事务顺序: ① submit HTTP → ② 获取 server_task_id → ③ 更新 job 文件(写入 server_task_id) → ④ 移动文件到 Running
- 如果步骤 ③ 或 ④ 失败, 在下次轮询时检测 job 文件中的 server_task_id, 如果存在则直接查状态而非重新 submit
- Running 目录的轮询也检查 server_task_id 是否有效

#### 风险 3: 网络分区下的状态判断 (中概率, 高影响)

**场景**: 轮询 Running 目录时发 HTTP query, 网络超时。此时不知道任务在服务端是否已完成。如果把文件移动到 Failed, 但实际上服务端已完成, 用户会困惑。

**缓解方案**:
- HTTP 超时不等于任务失败, 只代表本次查询失败
- 设计重试计数 `query_retry_count` 写入 job 文件, 每次 query 失败 +1
- 只有连续失败 N 次 (建议 5 次 = 5 个轮询周期) 才标记为疑似失败
- 前端可以手动触发重新查询 (通过 SDK 的 querystatus 方法)

#### 风险 4: 下载 URL 时效性 (中概率, 中影响)

**场景**: 服务端返回的下载 URL 可能是预签名 URL (如 OSS/S3 presigned URL), 有过期时间。如果前端在 feedback 标记完成后较长时间才调用 download, URL 可能已过期。

**缓解方案**:
- feedback 中存储 result_url
- SDK 的 download 方法: 先检查本地是否已有缓存文件 → 没有则用 URL 下载 → URL 过期返回特定错误码 → 前端可调用 querystatus 重新获取 URL
- 或者 download 不依赖 feedback 中的 URL, 而是每次直接向服务端请求最新的下载 URL

#### 风险 5: 前后端对 feedback 文件的并发读 (低概率, 低影响)

**场景**: generateThread 正在写 feedback, 同时前端通过 SDK 的 querystatus 读取 feedback。

**缓解方案**: 现有 `save_with_retry` / `load_with_retry` 机制已经有 `.lock` 文件保护。顺序轮询设计下 generateThread 写完一个文件立即释放锁, 前端读取时获取锁, 冲突窗口极小。

---

## 三、总体架构设计

### 3.1 目录结构

```
~/AppData/Local/MoldAI/
├── jobs/              ← 现有本地计算任务 (不动)
│   ├── Pending/
│   ├── Running/
│   ├── Completed/
│   ├── Failed/
│   ├── Cancelled/
│   └── Engines/
│
└── jobs_gen/          ← 新增生成式任务 (结构保持一致)
    ├── Pending/
    ├── Running/
    ├── Completed/
    ├── Failed/
    ├── Cancelled/
    └── Engines/       ← Node 心跳信息文件 (gen 专用的 engine info)
```

### 3.2 架构图

```
┌────────────────────────────────────────────────────────────────────┐
│                     Frontend (MoldAI.exe / SDK)                     │
│  submit()  querypoint()  querystatus()  download()  canceljob()   │
└──────────┬──────────┬──────────┬──────────┬──────────┬────────────┘
           │ 写文件    │ HTTP     │ 读文件    │ HTTP+文件 │ 删/移文件
           ▼          ▼          ▼          ▼          ▼
┌─────────────────────────────────────────────────────────────────────┐
│                        jobs_gen/ 工作目录                            │
│  Pending/  │  Running/  │  Completed/  │  Failed/  │  Cancelled/   │
│  job.bin   │  job.bin   │   job.bin    │  job.bin  │   job.bin     │
└─────────────────────────────────────────────────────────────────────┘
       ▲ 轮询+HTTP submit      ▲ 轮询+HTTP query       │ 移动+更新
       │                       │                        │ feedback
┌──────┴───────────────────────┴────────────────────────┴────────────┐
│                       MoldAINode.exe                                │
│                                                                     │
│  generateThread:                                                    │
│    Phase 1: 遍历 Pending/ → HTTP POST /api/submit                  │
│    Phase 2: 遍历 Running/ → HTTP GET  /api/querystatus             │
│                                                                     │
│  (现有线程不受影响: searchPendingJobThread2, execEngineTimeThread等) │
└─────────────────────────────────────────────────────────────────────┘
       │  HTTP                          ▲  HTTP Response
       ▼                                │
┌─────────────────────────────────────────────────────────────────┐
│                   三方生成式模型服务端                              │
│  POST /api/submit       → { task_uuid, status: "accepted" }      │
│  GET  /api/querystatus  → { status, percent, result_url, ... }   │
│  GET  /api/querypoint   → { user_id, balance, ... }              │
│  POST /api/cancel       → { success: true }                      │
│  GET  /api/download     → binary file stream                     │
└─────────────────────────────────────────────────────────────────┘
```

### 3.3 与现有系统的隔离边界

```
generateThread 只操作 jobs_gen/ 目录, 不触碰 jobs/ 目录。
现有 searchPendingJobThread2 只操作 jobs/ 目录, 不触碰 jobs_gen/ 目录。

共享的:
  - AICORE::File::FopenDenyWriteLockUtf8() 锁机制 (复用)
  - AICORE::File::BoostPathFromUtf8() 路径处理 (复用)
  - LOGI/LOGE 日志宏 (复用)
  - Settings: 从现有 `engine` 路径推导 `jobs_gen`, 无需新注册表项
  - Qt6 网络模块 (QNetworkAccessManager + QNetworkReply)
```

---

## 四、数据结构设计

> 最终数据结构定义见 [GenTask_Integration.md](GenTask_Integration.md) §3.2 和 [可能用到的数据结构.md](可能用到的数据结构.md)。本节仅说明设计决策。

### 4.1 双 ID 设计 (task_uuid vs server_task_id)

生成式任务系统中存在两个 ID，职责不同，不可互相替代：

| 字段 | 生成方 | 存储位置 | 用途 |
|------|--------|----------|------|
| `task_uuid` | 客户端 (SDK, `QUuid::createUuid()`) | GenJobInfo.task_uuid, GenTaskRequest.task_id | 客户端全链路追踪、幂等去重 |
| `server_task_id` | 服务端 (Triverse) | GenJobInfo.server_task_id, JobFeedBack_s.server_task_id | 向服务端轮询/取消任务 |

#### task_uuid 解决什么问题

1. **重复提交去重（幂等）**：Node 从 Pending 读取 job → HTTP submit 成功 → 移动到 Running 之前崩溃 → 重启后再次读取同一 Pending 文件。此时服务端用 `task_uuid` 识别重复请求，返回已有结果而非新建任务。见 §2.2 风险 1。
2. **全链路追踪**：`task_uuid` 在用户点击"生成"时立即生成，贯穿 job 文件、HTTP 请求、feedback 文件、前端回调，整个生命周期唯一标识。出问题时可以通过一个 uuid 定位到所有相关日志和文件。
3. **前端查询索引**：`SubmitGenTask` 返回 `task_uuid` 给前端，后续 `QueryTaskStatus`、`DownloadResult`、`CancelGenTask` 均以此作为参数。前端只需要记住这一个 ID。

#### server_task_id 解决什么问题

1. **向服务端查询任务状态**：`task_uuid` 是客户端自己生成的，服务端（Triverse）内部有自己的任务标识体系。Node 的 `generateThread` 在轮询 `/api/v1/task/status` 时，必须用服务端返回的 `server_task_id` 作为查询参数。
2. **崩溃恢复时跳过重复提交**：Node 重启后扫描 Pending 目录，如果发现 job 文件中 `server_task_id` 已有值，说明之前已经 submit 成功过，直接跳到轮询状态而非重新 POST。见 §2.2 风险 1 缓解方案 3。
3. **取消任务**：调用服务端 cancel 接口时需要传入 `server_task_id`。

#### 生命周期关系

```
用户点击"生成" → task_uuid 诞生 (QUuid::createUuid)
    │
    └── HTTP POST /submit { task_id: task_uuid } → 服务端返回 { triverse_task_uuid: "tsk-xxx" }
            │
            └── server_task_id = "tsk-xxx" (回填到 job 文件 + feedback)
                    │
                    └── 轮询 GET /status?task_id=tsk-xxx  (用 server_task_id)
                            │
                            └── 前端用 task_uuid 查询/下载  (用 task_uuid)
```

> **简单记忆**：`task_uuid` 是"客户端的主键"，`server_task_id` 是"跟服务端对话时用的凭证"。客户端内部只用 `task_uuid`，跟服务端通信时才用 `server_task_id`。

### 4.2 数据结构来源

生成式任务的数据结构分为两层:

| 层级 | 定义位置 | 包含 |
|------|---------|------|
| 用户交互层 | [可能用到的数据结构.md](可能用到的数据结构.md) | `GenTaskCategory`, `GenTaskSubType`, `GenTaskStatus`, `AssetKind`, `AssetRef`, `GenerationParams` |
| HTTP 通信层 | [可能用到的数据结构.md](可能用到的数据结构.md) | `GenTaskRequest` (发给服务端), `GenTaskResponse` (服务端返回) |
| Job 文件层 | [GenTask_Integration.md](GenTask_Integration.md) §3.2 | `GenJobInfo` (写入 jobs_gen/ 的顶层结构, 包含上述所有) |

### 4.3 关键结构关系

```
前端 UI 填写
    │
    ▼
GenerationParams  ←── 用户填写的参数 (prompt, model, 参考图等)
    │
    ├──→ GenJobInfo.params   (持久化到 jobs_gen/Pending/ 的 Job 文件)
    │
    └──→ GenTaskRequest      (HTTP POST /api/v1/task/submit 的 body)
            │
            ▼
         GenTaskResponse     (服务端返回, 包含 server_task_id, status, result_url)
            │
            ├──→ GenJobInfo 字段回填 (server_task_id, status, result_url)
            │
            └──→ JobFeedBack_s 更新 (Status, Percent, result_url, cost_credits)
```

### 4.4 Feedback 文件

为最大限度复用, 生成式任务的 feedback 扩展 `JobFeedBack_s` (详见 [GenTask_Integration.md](GenTask_Integration.md) §2.3):

```cpp
// JobFeedBack_s 新增字段:
std::string result_url;        // 结果下载链接
std::string preview_url;       // 预览图链接
std::string server_task_id;    // 服务端分配的任务ID
std::string error_message;     // 详细错误信息
int cost_credits = 0;          // 本次消耗积分
int points_balance = 0;        // 积分余额
```

### 4.5 不使用 BIN 格式

- 文件数量少 (~10), JSON 解析开销可忽略
- 数据要通过 HTTP 发送, 本来就是 JSON
- BIN 格式增加调测难度
- 新系统不需要承担历史包袱

**生成式任务只使用 JSON 格式。**

---

## 五、HTTP 通信设计

### 5.1 服务端 API 约定

API 请求/响应 body 直接映射自 [可能用到的数据结构.md](可能用到的数据结构.md) 中的 `GenTaskRequest` / `GenTaskResponse`。

#### 5.1.1 Submit 任务

```
POST /api/v1/task/submit
Content-Type: application/json

Request (映射自 GenTaskRequest):
{
    "task_id": "550e8400-e29b-41d4-a716-446655440000",
    "engine_id": "engine-sh-01",
    "user_account": "user@example.com",
    "params": {
        "category": "TEXT_TO_3D",
        "sub_type": "TEXT_TO_MODEL",
        "prompt": "a beautiful landscape",
        "model_version": "v2",
        "polygon_limit": 50000,
        "texture_size": 2048
    }
}

Response (200 OK, 映射自 GenTaskResponse):
{
    "task_id": "550e8400-e29b-41d4-a716-446655440000",
    "triverse_task_uuid": "triverse-abc123",
    "status": "PENDING",
    "progress": 0
}
```

#### 5.1.2 查询任务状态

```
GET /api/v1/task/status?server_task_id=triverse-abc123

Response (映射自 GenTaskResponse):
{
    "task_id": "550e8400-e29b-41d4-a716-446655440000",
    "triverse_task_uuid": "triverse-abc123",
    "status": "IN_PROGRESS",
    "progress": 65,
    "result_url": null,
    "preview_url": null,
    "error_message": null,
    "cost_credits": 0
}

// 完成时:
{
    "task_id": "...",
    "triverse_task_uuid": "triverse-abc123",
    "status": "COMPLETED",
    "progress": 100,
    "result_url": "https://cdn.example.com/results/xxx.glb",
    "preview_url": "https://cdn.example.com/previews/xxx.png",
    "cost_credits": 10
}
```

#### 5.1.3 查询积分

```
GET /api/v1/user/points?user_account=user@example.com

Response:
{
    "user_account": "user@example.com",
    "total_points": 1000,
    "frozen_points": 100
}
```

#### 5.1.4 取消任务

```
POST /api/v1/task/cancel
{ "server_task_id": "triverse-abc123" }

Response:
{ "success": true }
```

### 5.2 Node 端 HTTP 客户端设计

```cpp
// 新增: App/Engine/GenHttpClient.h
// 请求/响应直接使用 GenTaskRequest / GenTaskResponse (见 可能用到的数据结构.md)

class GenHttpClient : public QObject {
    Q_OBJECT
public:
    // 使用 GenTaskRequest/GenTaskResponse 作为参数和返回值
    GenTaskResponse submitTask(const GenTaskRequest& req, int timeout_ms = 5000);
    GenTaskResponse queryStatus(const std::string& server_task_id, int timeout_ms = 2000);
    bool          cancelTask(const std::string& server_task_id, int timeout_ms = 5000);

    // 查询积分 (独立接口)
    struct PointsResult {
        bool success;
        int total_points;
        int frozen_points;
    };

    // 同步请求 (在 generateThread 中调用)
    // submitTask 接受 GenTaskRequest (来自 GenTaskDef.h)
    // 返回 GenTaskResponse (来自 GenTaskDef.h)
    GenTaskResponse submitTask(const GenTaskRequest& req, int timeout_ms = 5000);
    GenTaskResponse queryStatus(const std::string& server_task_id, int timeout_ms = 2000);
    PointsResult    queryPoints(const std::string& user_account, int timeout_ms = 2000);
    bool            cancelTask(const std::string& server_task_id, int timeout_ms = 5000);

private:
    // 使用 QNetworkAccessManager 的同步包装
    // 利用 QEventLoop + QTimer::singleShot 实现超时
    QNetworkAccessManager* m_manager;

    // 带超时和重试的通用方法
    template<typename Func>
    auto withRetry(Func func, int max_retries, int timeout_ms)
        -> decltype(func());
};
```

### 5.3 超时与重试策略

```
Submit:
  超时: 5s (提交一般比查询慢)
  重试: 3 次, 间隔 1s

Query Status:
  超时: 2s
  重试: 3 次, 间隔 500ms

Query Points:
  超时: 2s
  重试: 1 次

Cancel:
  超时: 5s
  重试: 1 次
```

重试仅针对网络层错误 (连接超时、连接拒绝、DNS 解析失败), HTTP 4xx/5xx 不重试。

---

## 六、generateThread 线程设计

### 6.1 核心流程

```cpp
void generateThreadFunc()
{
    GenHttpClient httpClient;
    QString genPendingPath = genJobRoot + "/Pending/";
    QString genRunningPath = genJobRoot + "/Running/";
    QString genCompletedPath = genJobRoot + "/Completed/";
    QString genFailedPath = genJobRoot + "/Failed/";
    QString genCancelledPath = genJobRoot + "/Cancelled/";

    while (true) {
        if (bQuitingApplication) break;

        // ===== Phase 1: 处理 Pending → 提交到服务端 =====
        processPendingJobs(httpClient,
                           genPendingPath, genRunningPath);

        // ===== Phase 2: 处理 Running → 查询服务端状态 =====
        processRunningJobs(httpClient,
                           genRunningPath, genCompletedPath,
                           genFailedPath, genCancelledPath);

        // ===== Phase 3: 处理待取消任务 =====
        processCancelledJobs(httpClient,
                             genCancelledPath);

        sleep(2000);  // 轮询间隔 2s
    }
}
```

### 6.2 Phase 1: processPendingJobs 详细逻辑

```cpp
void processPendingJobs(GenHttpClient& client,
                        const QString& pendingPath,
                        const QString& runningPath)
{
    QDir dir(pendingPath);
    QFileInfoList files = dir.entryInfoList(QDir::Files);
    // 过滤掉 .lock 文件
    files = filterOutLockFiles(files);

    for (const QFileInfo& fileInfo : files) {
        if (bQuitingApplication) break;

        QString jobPath = fileInfo.absoluteFilePath();
        QString jobLockPath = jobPath + ".lock";

        // 1. 获取文件锁
        FILE* fpLock = AICORE::File::FopenDenyWriteLockUtf8(
            qstr2str(jobLockPath));
        if (!fpLock) {
            continue;  // 被其他进程占用, 跳过
        }

        // 2. 读取 job 文件
        GenJobInfo job;
        if (!job.load(qstr2str(jobPath))) {
            fclose(fpLock);
            continue;
        }

        // 3. 幂等检查: 如果已有 server_task_id, 说明之前 submit 成功
        if (!job.server_task_id.empty()) {
            moveToRunning(job, jobPath, runningPath);
            fclose(fpLock);
            continue;
        }

        // 4. 构造请求 (GenJobInfo → GenTaskRequest)
        GenTaskRequest req;
        req.task_id     = job.task_uuid;
        req.engine_id   = job.engine_id;
        req.user_account = job.user_account;
        req.params      = job.params;

        // 5. HTTP Submit
        GenTaskResponse resp = client.submitTask(req, /*timeout=*/5000);

        if (resp.triverse_task_uuid.has_value()) {
            // 5a. 回填服务端分配的任务ID
            job.server_task_id = resp.triverse_task_uuid.value();
            job.status = GenTaskStatus::PENDING;

            // 5b. 更新 job 文件并移动
            job.save(qstr2str(jobPath));
            moveToRunning(job, jobPath, runningPath);
            updateFeedback(job);
        } else if (!resp.error_message.value_or("").empty()) {
            // 业务错误 (余额不足等) → 直接移到 Failed
            job.status = GenTaskStatus::FAILED;
            job.save(qstr2str(jobPath));
            moveToFailed(job, jobPath, failedPath);
            updateFeedback(job);
        }
        // 网络错误 → 保持 Pending, 下次轮询重试
        }

        fclose(fpLock);
    }
}
```

### 6.3 Phase 2: processRunningJobs 详细逻辑

```cpp
void processRunningJobs(GenHttpClient& client,
                        const QString& runningPath,
                        const QString& completedPath,
                        const QString& failedPath,
                        const QString& cancelledPath)
{
    QDir dir(runningPath);
    QFileInfoList files = dir.entryInfoList(QDir::Files);
    files = filterOutLockFiles(files);

    for (const QFileInfo& fileInfo : files) {
        if (bQuitingApplication) break;

        QString jobPath = fileInfo.absoluteFilePath();
        QString jobLockPath = jobPath + ".lock";

        FILE* fpLock = AICORE::File::FopenDenyWriteLockUtf8(
            qstr2str(jobLockPath));
        if (!fpLock) continue;

        GenJobInfo job;
        if (!job.load(qstr2str(jobPath))) {
            fclose(fpLock);
            continue;
        }

        // 检查是否被标记为取消
        if (job.status == GenTaskStatus::CANCELLED) {
            moveToCancelled(job, jobPath, cancelledPath);
            fclose(fpLock);
            continue;
        }

        // HTTP Query Status → 得到 GenTaskResponse
        GenTaskResponse resp = client.queryStatus(job.server_task_id,
                                                   /*timeout=*/2000);

        if (!resp.error_message.value_or("").empty() && resp.status == GenTaskStatus::FAILED) {
            // 服务端返回了错误
            // ... (网络错误处理逻辑见下方)
        } else {
            job.query_retry_count = 0;

            // 根据 resp.status (GenTaskStatus 枚举) 处理
            switch (resp.status) {
            case GenTaskStatus::COMPLETED:
                job.status = GenTaskStatus::COMPLETED;
                job.result_url = resp.result_url.value_or("");
                job.preview_url = resp.preview_url.value_or("");
                job.cost_credits = resp.cost_credits;
                job.complete_time = now();
                job.save(qstr2str(jobPath));
                moveToCompleted(job, jobPath, completedPath);
                updateFeedback(job);
                break;

            case GenTaskStatus::FAILED:
                job.status = GenTaskStatus::FAILED;
                job.save(qstr2str(jobPath));
                moveToFailed(job, jobPath, failedPath);
                updateFeedback(job);
                break;

            default:  // IDLE, PENDING, IN_PROGRESS
                job.save(qstr2str(jobPath));
                updateFeedback(job);
                break;
            }
        }

        // 如果 HTTP 查询本身失败 (网络超时等), resp.status 保持 IDLE
        // 此时记录重试次数
        if (resp.status == GenTaskStatus::IDLE) {
            job.query_retry_count++;
            job.save(qstr2str(jobPath));

            if (job.query_retry_count >= 5) {
                LOGI("task " + job.task_uuid +
                     " query failed 5 times, marking as suspected failed");
                job.status = GenTaskStatus::FAILED;
                job.save(qstr2str(jobPath));
                moveToFailed(job, jobPath, failedPath);
                updateFeedback(job);
            }
        }

        fclose(fpLock);
    }
}
```

### 6.4 文件移动的原子性优化

现有系统的文件移动 (save → sleep → verify → delete) 存在不一致窗口。针对生成式任务, 建议优化为:

```cpp
bool atomicMoveJob(const QString& srcPath, const QString& dstPath) {
    // 1. 先将新文件写到一个 .tmp 文件
    QString tmpPath = dstPath + ".tmp";
    QFile::remove(tmpPath);  // 清理上次残留
    if (!QFile::copy(srcPath, tmpPath)) {
        return false;
    }

    // 2. 将 .tmp 重命名为正式文件 (同一文件系统下是原子操作)
    QFile::remove(dstPath);
    if (!QFile::rename(tmpPath, dstPath)) {
        QFile::remove(tmpPath);
        return false;
    }

    // 3. 删除源文件
    QFile::remove(srcPath);

    // 4. 同时移动 .lock 文件 (如果存在)
    QString srcLock = srcPath + ".lock";
    QString dstLock = dstPath + ".lock";
    if (QFile::exists(srcLock)) {
        QFile::remove(dstLock);
        QFile::rename(srcLock, dstLock);
    }

    return true;
}
```

使用 `rename` (同一文件系统内) 比 copy+delete 更原子, 且避免了 1s sleep 等待。

---

## 七、Core 模块 SDK 设计

### 7.1 新增文件

```
Include/Core/GenTaskAPI.h    ← SDK 对外接口声明
Src/Core/GenTaskAPI.cpp      ← SDK 实现
```

### 7.2 API 接口

```cpp
// Include/Core/GenTaskAPI.h

namespace AI3D { namespace CORE {

class GenTaskAPI {
public:
    // 提交生成式任务 (前端调用)
    // 前端传入 Block 信息和 GenerationParams, SDK 内部构造 GenJobInfo
    // 返回 { task_uuid, job_name, feedback_path }
    struct SubmitResult {
        std::string task_uuid;
        std::string job_name;
        std::string feedback_path;
    };
    static SubmitResult SubmitGenTask(const BlockObject::Task_Info& blockInfo,
                                      const GenerationParams& params,
                                      const std::string& user_account);

    // 查询积分余额
    static PointsQueryResult QueryPoints(const std::string& user_account);

    // 查询任务状态 (读取 feedback 文件)
    static TaskStatusResult QueryTaskStatus(const std::string& task_uuid);

    // 下载结果文件
    static DownloadResult DownloadResult(
        const std::string& task_uuid,
        const std::string& save_path);

    // 取消任务
    // 1. 如果还在 Pending → 直接删除文件
    // 2. 如果在 Running → 移动到 Cancelled, Node 线程会处理
    // 3. 如果在 Running 且 server_task_id 已存在 → 还会调服务端 cancel
    static bool CancelGenTask(const std::string& task_uuid);

    // 设置任务完成回调 (SDK 使用者注册)
    // 当 generateThread 检测到任务完成时触发
    using TaskCompleteCallback = std::function<void(const std::string& task_uuid,
                                                     const std::string& result_url)>;
    static void SetTaskCompleteCallback(TaskCompleteCallback cb);

    // 设置任务失败回调
    using TaskFailedCallback = std::function<void(const std::string& task_uuid,
                                                   const std::string& error)>;
    static void SetTaskFailedCallback(TaskFailedCallback cb);
};

}} // namespace AI3D::CORE
```

### 7.3 回调机制的实现

由于 generateThread 是顺序轮询的 (非异步), 回调不需要信号槽, 而是在轮询循环中直接调用:

```cpp
// generateThread 检测到完成时:
if (result.status == "completed") {
    // ... 移动文件、更新 feedback ...
    // 触发回调 (如果已注册)
    auto& cb = GenTaskAPI::GetTaskCompleteCallback();
    if (cb) {
        cb(job.task_uuid, job.result_url);
    }
}
```

如果使用者需要在 Qt UI 线程中处理回调, SDK 内部使用 `QMetaObject::invokeMethod` 或让使用者在回调中自行 `emit` 信号。

---

## 八、实现计划

> **详细执行清单见 [GenTask_Checklist.md](GenTask_Checklist.md)** — 6 个 Phase 按依赖排列，每项可勾选，包含完整的文件改动汇总。以下为概要。

### 8.1 新增文件 (7 个)

| 文件 | 说明 |
|------|------|
| `Include/Util/GenTaskDef.h` | 生成式任务数据结构 (GenTaskCategory, GenerationParams, GenTaskRequest, GenTaskResponse, GenJobInfo 等) |
| `App/Engine/GenHttpClient.h` | HTTP 客户端封装 |
| `App/Engine/GenHttpClient.cpp` | HTTP 客户端实现 |
| `App/Engine/GenTaskThread.h` | generateThread 线程逻辑 |
| `App/Engine/GenTaskThread.cpp` | generateThread 实现 |
| `Include/Core/GenTaskAPI.h` | SDK 对外接口 |
| `Src/Core/GenTaskAPI.cpp` | SDK 实现 |

### 8.2 修改现有文件 (8 个)

| 文件 | 改动内容 |
|------|---------|
| `Include/Core/BlockObject.h` | `Task_Info` 增加 `block_task_category` |
| `Include/Core/BlockObject.cpp` | 4 个序列化方法增加新字段 |
| `Include/Util/TaskProcess.h` | `JobInfo_s` + `JobFeedBack_s` 增加新字段; 更新序列化方法 |
| `Include/Util/Settings.h` | 增加 `getGenEngineJobQueue()` 声明 |
| `Src/Util/Settings.cpp` | 实现 `getGenEngineJobQueue()`, 从 `engine` 路径父目录推导 |
| `App/Engine/CallEngine.cpp` | 增加 `jobs_gen` 目录创建; 启动 `generateThread` |
| `App/Engine/CMakeLists.txt` | 增加新源文件; 链接 `Qt6::Network` |
| `Src/Core/CMakeLists.txt` | 增加 `GenTaskAPI.cpp` |

### 8.3 不需要修改的部分

- `JobFullInfo_s` / `TaskGraph_s` / `Task_s` — 仅本地任务使用
- `searchPendingJobThread2` / `GetPendingJob` / `ExecTaskFileV2` — 不动
- `FeedBackFile` / BIN 格式 — 生成式任务只用 JSON
- `JobMonitor` — 可以复用其目录创建逻辑

### 8.4 实施顺序 (推荐)

```
Phase 1: 基础设施
  1.1 GenTaskDef.h — 数据结构定义
  1.2 Settings — 从现有 engine 路径推导 jobs_gen
  1.3 main() — jobs_gen 目录创建

Phase 2: HTTP 通信层
  2.1 GenHttpClient — 同步 HTTP 请求封装
  2.2 单元测试 (手动测试 submit/query 无误)

Phase 3: 调度线程
  3.1 GenTaskThread — generateThread 主逻辑
  3.2 与现有线程并行运行测试 (确认不冲突)

Phase 4: SDK
  4.1 GenTaskAPI — 前端可用接口
  4.2 回调机制

Phase 5: 集成测试
  5.1 端到端流程: 前端 submit → Node 调度 → 服务端执行 → 完成通知
  5.2 异常场景: Node 崩溃恢复、网络断开、服务端异常
```

---

## 九、简化设计带来的优势

按照用户提出的"顺序轮询 + 同步 HTTP"方案, 与异步方案相比:

| 维度 | 同步顺序轮询 | 异步事件驱动 |
|------|------------|------------|
| 代码复杂度 | 低 — 单线程, 顺序逻辑 | 高 — 需要信号槽连接管理 |
| feedback 锁冲突 | 极低 — 写完即释放, 无并发 | 中等 — 多个异步回调可能同时写 |
| 内存占用 | 低 — 不持有 pending request | 较高 — 需要管理多个未完成请求 |
| 状态恢复 | 简单 — 文件即状态 | 复杂 — 内存中的 pending request 丢失 |
| 延迟 | 2s × N 轮询间隔 | 近乎实时 |
| 适用任务量 | < 50 | > 100 |

对于 ~10 任务量, 同步方案完全够用, 且大幅降低了复杂度。

---

## 十、配置说明

### 10.1 路径配置

`jobs_gen` 路径从现有 `engine` 注册表值推导, 无需新增注册表项:

```
engine = "C:/Users/xxx/AppData/Local/MoldAI/jobs"
                                          ↓ 取父目录 + "/jobs_gen"
jobs_gen = "C:/Users/xxx/AppData/Local/MoldAI/jobs_gen"
```

```cpp
// Settings.cpp
QString Settings::getGenEngineJobQueue()
{
    QString enginePath = getEngineJobQueue();
    QDir parent = QFileInfo(enginePath).dir();
    return parent.absolutePath() + "/jobs_gen";
}
```

### 10.2 服务端 URL 配置

建议放在 MoldAIConfig.ini 中 (与现有 `configpath` 同一文件):

```ini
[GenTask]
ServerUrl=http://api.example.com
ApiPrefix=/api/v1
Timeout=5000
MaxRetries=3
```

或直接硬编码 — 取决于部署灵活性需求。
