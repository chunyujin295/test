# 生成式任务系统 — 总体设计梳理

## 一、系统定位

在现有重建式任务系统基础上平行增加一个生成式模型任务系统。

| 维度 | 重建式 (现有) | 生成式 (新增) |
|------|-------------|-------------|
| 执行方式 | Engine spawn MoldAITask.exe 子进程 | Engine 线程直接发 HTTP |
| 通信 | 文件系统 IPC (lock + feedback) | HTTP (submit + query) |
| 工作目录 | `jobs/` | `jobs_gen/` |
| 任务结构 | DAG 子任务 (TaskGraph_s → tasksmap) | 单一任务 (无子任务拆分) |
| 进度 | 多阶段递增 | 服务端返回百分比 |
| 积分 | 需要 (后续接入) | 需要 (设计中) |

---

## 二、数据结构分层

### 2.1 三层架构

```
┌─────────────────────────────────────────────┐
│  内存层 (GenTaskProcess.h / PointManager.h)  │
│  GenJobInfo_s, PointInfoBase, ...            │
│  代码操作的对象，嵌套结构体，方便使用         │
├─────────────────────────────────────────────┤
│  序列化层 (DataStruct.h → GenJobFile)        │
│  GenJobInfoData, GenJobTaskData, FeedBackData │
│  BIN 格式，扁平字段，逐字段按序读写           │
├─────────────────────────────────────────────┤
│  文件层 (jobs_gen/ 目录)                     │
│  J_*.bin — 完整任务文件                      │
│  JF_*.bin — 独立 feedback 文件               │
│  *.blk — Block 元数据 (generations_info_)    │
└─────────────────────────────────────────────┘
```

### 2.2 内存层

| 结构体 | 文件 | 用途 |
|--------|------|------|
| `GenJobInfo_s` | `GenTaskProcess.h` | 单个生成式任务的全部数据 |
| `GenJobFullInfo_s` | `GenTaskProcess.h` | job 文件顶层：`job_name` + `GenJobInfo_s` + `JobFeedBack_s` |
| `GenTaskParams` | `GenTaskOptions.h` | 生成参数（prompt, polygon_limit, ...） |
| `GenTaskOptions` | `GenTaskOptions.h` | Block 级别的生成式选项（包裹 `GenTaskParams`） |
| `blk_generation_info_s` | `GenTaskOptions.h` | Block 中每条生成记录的摘要 |
| `PointFreezeInfo` | `PointManager.h` | 积分冻结/查询/估算的返回结构 |
| `PointSettleInfo` | `PointManager.h` | 积分结算的返回结构 |
| `PointInfoBase` | `PointManager.h` | 嵌入 `GenJobInfo_s` 的积分持久化字段 |
| `GenTaskResponse` | `GenTaskProcess.h` | Task API (submit/query) 的 HTTP 返回结构 |

**AI3D_API 规则**：纯数据 struct（含 `std::string` 成员、隐式拷贝构造）**不加** `AI3D_API`，否则跨 DLL 边界返回值拷贝时编译器会去 DLL 里找不存在的拷贝构造函数，导致崩溃（`读取位置 0x00000008`）。已从 `GenTaskParams`、`GenTaskOptions`、`blk_generation_info_s` 上去除。

### 2.3 序列化层 (BIN 格式)

遵循和重建式相同的"各节独立序列化"模式：

```
GenJobFile (对标 JobListFile)
├── jobName
├── GenJobInfoData         ← 指针节 (project_path + block_item)
├── GenJobTaskData          ← 任务数据 + 积分
└── FeedBackData            ← 反馈

重建式对照:
JobListFile
├── jobName
├── JobInfoData             ← 指针节 (projectPath + itemPath)
├── RunInfoData             ← 运行信息
├── FeedBackData            ← 反馈
└── taskNum + TaskItemData[] ← DAG 子任务
```

| 节 | 对标重建式 | 字段数 | 用途 |
|----|----------|--------|------|
| `GenJobInfoData` | `JobInfoData` | 2 (`project_path`, `block_item`) | 定位文件/Block |
| `GenJobTaskData` | 无直接对标 | ~20 | 任务全部字段 + 积分字段 |
| `FeedBackData` | `FeedBackData` | 4 | 进度反馈 |

**设计决策：GenJobInfoData 为什么是精简指针？**

`GenJobInfoData` 对标 `JobInfoData`，后者仅 2 个字段。如果塞入全部任务字段，会造成：
- Job 列表扫描开销大（每个条目都要反序列化 20+ 字段）
- 和重建式不一致
- 上线后改动即 BIN 兼容性问题

任务完整数据通过 `GenJobTaskData` 节承载，`GenJobFile` 统一序列化。

**BIN 序列化惯例**：所有字段扁平读写（`length + data` 或 `sizeof(T)`），不嵌套结构体。`GenJobFullInfo_s::WriteToBin` / `LoadFromBin` 负责 `GenJobInfo_s` ↔ `GenJobFile` 各节的字段映射。

---

## 三、积分系统

### 3.1 设计原则

- **积分结算先于状态更新**（解决结算丢失问题）
- 用户身份通过 `Authorization` header 的 `accessToken` 识别，不需要显式 `user_account` 参数
- 积分数据统一在 `PointInfoBase`，嵌入 `GenJobInfo_s`

### 3.2 API 与常量

| 端点 | 方法 | 用途 |
|------|------|------|
| `/point/outline` | 查询积分 | `PointManager::QueryUserPoints()` |
| `/point/estimate` | 估算消耗 | `PointManager::EstimateTaskPoints()` |
| `/point/freeze` | 冻结积分 | `PointManager::CreatePointTask()` |
| `/point/settle` | 积分结算 | `PointManager::SettlePoints()` |

常量定义在 `Types.h`：
```cpp
namespace BusinessType { ... }  // 13 个业务类型
namespace SettleStatus { ... }  // 4 个结算状态
```

### 3.3 PointManager — Core 层积分管理类

位置：`Include/Core/PointManager.h` + `Src/Core/PointManager.cpp`，编译进 `MoldAIData.dll`。

```
前端 (MoldAI.exe)                      引擎 (MoldAINode.exe)
    │                                       │
    └────────── PointManager ───────────────┘
              (MoldAIData.dll)
         ├── 4 个公开静态方法
         └── 4 个私有 HTTP 方法 (直接调 HttpClient::post)

GenHttpClient (App/Engine) — 仅负责 Task API (submit/query/cancel)
```

公开接口：

| 方法 | HTTP | 返回 |
|------|------|------|
| `QueryUserPoints()` | `/point/outline` | `PointFreezeInfo` (balance) |
| `EstimateTaskPoints(type, param)` | `/point/estimate` | `int` (estimate_points) |
| `CreatePointTask(type, param)` | `/point/freeze` | `PointFreezeInfo` (freeze_no + balance) |
| `SettlePoints(freeze_no, status, param)` | `/point/settle` | `PointSettleInfo` (consumed/refunded) |

### 3.4 积分状态流转

```
前端: EstimateTaskPoints → 确认积分够 → SubmitGenTask
                                        │
引擎: ProcessPendingJobs                │
  ├─ HTTP SubmitTask → server_task_id   │
  ├─ PointManager::CreatePointTask      │  ← 冻结积分 → freeze_no
  └─ 移到 Running/                      │
                                        │
引擎: ProcessRunningJobs                │
  ├─ HTTP QueryTaskStatus               │
  ├─ IN_PROGRESS: UpdateFeedback(progress)
  └─ COMPLETED:                         │
       ├─ !points_settled → PointManager::SettlePoints  ← ★ 先结算
       │    ├─ 成功 → points_settled=true → 落盘
       │    └─ 失败 → break (不更新状态, 下轮重试)
       └─ points_settled → 正常完成流程 (更新状态/Block/移文件)
```

### 3.5 崩溃恢复

- **Freeze 前崩溃**：`server_task_id` 已有 → 下轮直接移到 Running → 兜底补 Freeze
- **Settle 成功但落盘前崩溃**：`points_settled=false` → 下轮重试 Settle（服务端须幂等）
- **Settle 成功后崩溃**：`points_settled=true` 已落盘 → 下轮跳过 Settle 直接完成

### 3.6 已有字段清理

| 旧字段 | 位置 | 处理 |
|--------|------|------|
| `cost_credits` | `GenJobInfo_s`, `GenTaskResponse` | 删除 — 被 Credit API 的 `estimate_points`(预估) + `consumed`(实扣) 替代 |
| `points_balance` | `GenJobInfo_s`, `GenTaskResponse` | 删除 — 被 `available_points` / `total_balance` 替代 |

---

## 四、文件改动总览

### 新建

| 文件 | 内容 |
|------|------|
| `Include/Core/PointManager.h` | 积分数据结构 + PointManager 类声明 |
| `Src/Core/PointManager.cpp` | PointManager 实现 (4 个公开方法 + 4 个私有 HTTP 方法) |
| `App/doc/积分接口集成方案.md` | 积分系统完整设计文档 |
| `App/doc/GenTask_总体设计梳理.md` | 本文档 |

### 修改

| 文件 | 改动 |
|------|------|
| `Include/Core/Types.h` | 新增 `GEN_SERVER_URL` / `GEN_API_PREFIX` / `BusinessType` / `SettleStatus` |
| `Include/Core/GenTaskOptions.h` | 去掉 `GenTaskParams` / `GenTaskOptions` / `blk_generation_info_s` 的 `AI3D_API` |
| `Include/Util/GenTaskProcess.h` | `GenTaskResponse` 删 cost/balance；`GenJobInfo_s` 删 cost/balance + 增 `PointInfoBase point_info` |
| `Include/Core/BlockObject.h` | `Task_Info` 增 `block_task_category` / `gen_options` / `generations_info_` / `generationjobs_` |
| `Include/Core/DataStruct.h` | 新增 `GenJobInfoData`(精简) / `GenJobTaskData` / `GenJobFile` |
| `App/Engine/GenHttpClient.h/cpp` | 删积分方法（迁移至 PointManager）；SubmitTask/QueryTaskStatus 删 cost/balance 解析 |
| `App/Engine/GenTaskThread.cpp` | Pending: FreezePoints；Running: 终态前置 SettlePoints + progress 修复；兜底补 Freeze |
| `Include/Core/TaskDef.h` | `JobInfo_s` 增 `PointInfoBase point_info` (重建式积分) |
| `App/Engine/CallEngine.cpp` | 重建式 spawn 前 freeze + 终态后 settle |
| `Src/Core/CMakeLists.txt` | 加入 `HttpClient.cpp` / `HttpClient.h` / `constant.h` |

---

## 五、相关文档索引

| 文档 | 内容 |
|------|------|
| [GenTask_Checklist.md](GenTask_Checklist.md) | 分 Phase 执行清单，含完整代码 |
| [GenTask_Design.md](GenTask_Design.md) | 总体设计方案 |
| [GenTask_Integration.md](GenTask_Integration.md) | 与现有代码体系的整合方案 |
| [GenTask_Frontend_API.md](GenTask_Frontend_API.md) | 前端对接 API |
| [积分接口.md](积分接口.md) | 后端积分 API 定义 |
| [积分接口集成方案.md](积分接口集成方案.md) | 积分系统完整设计 + 代码变更详情 |
| 本文档 | 总体设计梳理 |
