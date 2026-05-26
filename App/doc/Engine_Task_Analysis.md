# App/Engine 与 App/Task 模块分析文档

## 一、概览

本项目是一个基于 Qt6 + C++17 的 **3D 重建任务调度与执行系统**（MoldAI）。系统采用 **Engine/Task 分离架构**：

| 模块 | 编译产物 | 角色 | 代码量 |
|------|----------|------|--------|
| [App/Engine](Engine/) | `MoldAINode.exe` | 任务调度引擎（常驻守护进程） | ~5000 行 |
| [App/Task](Task/) | `MoldAITask.exe` | 任务执行器（按需拉起/退出） | ~1000 行 |

**核心设计思想**：Engine 作为常驻调度进程，维护任务队列并管理任务生命周期；Task 作为独立子进程，每次执行一个具体的算法任务，执行完毕后退出。两者通过文件系统（Job/Task JSON/BIN 文件 + Lock 文件 + Feedback 文件）进行解耦通信。

---

## 二、App/Engine 模块详解

### 2.1 文件清单

| 文件 | 说明 |
|------|------|
| [CallEngine.cpp](Engine/CallEngine.cpp) | **核心文件**，包含全部调度逻辑、main 函数 |
| [CallEngine.h](Engine/CallEngine.h) | 头文件，定义 XLog 日志宏 |
| [BootProbe.cpp](Engine/BootProbe.cpp) | 启动探针，在 `main` 之前写入启动标记到日志 |
| [BootProbe.h](Engine/BootProbe.h) | 探针头文件 |
| [MyrapidJson.cpp](Engine/MyrapidJson.cpp) | RapidJSON 封装，解析任务/作业 JSON 的 `meta_data` |
| [MyrapidJson.h](Engine/MyrapidJson.h) | JSON 封装头文件 |
| [CMakeLists.txt](Engine/CMakeLists.txt) | 构建脚本 |
| [MoldAINode.rc](Engine/MoldAINode.rc) | Windows 资源文件（图标） |

### 2.2 架构设计

Engine 采用 **多线程 + 文件锁 + 状态目录** 的架构模式：

```
                    ┌─────────────────────────────┐
                    │     MoldAINode.exe (Engine)  │
                    │                              │
                    │  main()                      │
                    │    ├─ execEngineTimeThread    │ ← 心跳维护
                    │    ├─ searchPendingJobThread2 │ ← 主调度循环
                    │    ├─ searchUnnormald...Thread│ ← 异常任务检测
                    │    ├─ execTaskTimeThread      │ ← Task PID 管理
                    │    └─ DoCleanupLockFiles      │ ← Lock 清理
                    │         │                     │
                    │         │ spawn & wait        │
                    │         ▼                     │
                    │  ┌──────────────────┐        │
                    │  │ MoldAITask.exe   │        │
                    │  └──────────────────┘        │
                    └─────────────────────────────┘
```

### 2.3 任务队列状态目录

所有 Job 文件在 `ENGINEJOBPATH`（默认 `%LOCALAPPDATA%/MoldAI/jobs`）下按状态分目录存放：

| 目录 | 宏 | 含义 |
|------|-----|------|
| `Pending/Normal/` | `pendingJobPath` | 等待执行 |
| `Pending/High/` | - | 高优先级等待 |
| `Running/` | `runningJobPath` | 执行中 |
| `Completed/` | `completedJobPath` | 已完成 |
| `Failed/` | `failedJobPath` | 执行失败 |
| `Cancelled/` | `cancelledJobPath` | 已取消 |
| `Engines/` | - | 引擎注册/心跳文件 |

### 2.4 核心调度流程

#### searchPendingJobThread2（主循环）

```
loop:
  ├─ 检查是否有正在处理的任务 → sleep, continue
  ├─ 检查 task 是否在运行 → sleep, continue
  │
  ├─ GetRunningTaskInRunningJob()  ← 优先处理 Running 队列中未完成的任务
  │   ├─ 遍历 Running/ 目录
  │   ├─ 查找 tasksmap 中未完成/等待中的 task
  │   ├─ 通过 .lock 文件获取互斥访问权
  │   ├─ 校验 feedback 状态
  │   └─ 设置 NewFileForRun, gotNewPendingJobFile = true
  │
  ├─ 若 Running 队列为空 → GetPendingJob()  ← 从 Pending 队列取新任务
  │   ├─ 对 job 文件加 .lock
  │   ├─ 将 job 写入 Running/ 目录
  │   ├─ 删除 Pending 中的原文件
  │   ├─ 对 task_def_0 文件加 .lock
  │   └─ 设置 NewFileForRun
  │
  └─ ExecTaskFileV2()  ← 启动任务子进程并等待
      ├─ 启动 MoldAITask.exe（QProcess）
      ├─ 阻塞等待子进程结束 (waitForFinished)
      └─ 根据返回码处理后续逻辑（Complete/Cancel/Fail）
```

#### ExecTaskFileV2（任务执行）

此函数根据 task 类型分三条路径：

1. **ATSTARTTYPE（任务拆分）**：执行 `GenTasks` 将大任务拆分为多个子任务 → 注册到 `tasksmap` → 保存 job 文件
2. **ATLASTTASKTYPE / ATCOMPLETETYPE**：处理任务完成逻辑 → 将 job 从 Running 移到 Completed
3. **其他类型**：启动 `MoldAITask.exe` 子进程，传递 task 文件路径，阻塞等待完成

#### 子进程完成后的后处理

```
Task 子进程结束
├─ returnCode == MOLDAI_SUCCESS (100000)
│   ├─ ATSTARTTYPE → 注册子任务到 tasksmap，保存 job
│   ├─ RECONSTRUCTIONSTARTTYPE → 标记任务完成，job → Completed/
│   └─ 其他 → 标记当前 task 完成
│
├─ returnCode == MOLDAI_USER_CANCEL (100001)
│   └─ job → Cancelled/
│
└─ 其他（异常/崩溃 exitCode 1099）
    ├─ 生成 MiniDump
    ├─ 执行 ExecTaskPostHandle（向 Task 进程发送 cancel 命令）
    └─ job → Failed/
```

### 2.5 Lock 文件机制

Engine 通过 **Windows 排他文件锁**（`FopenDenyWriteLockUtf8`）实现多进程互斥：

- **Job Lock**：`<jobname>.json.lock`，保护 Job 文件的读写
- **Task Lock**：`task_def_N.json.lock`，保护 Task 文件的读写
- **Feedback Lock**：`<feedback>.json.lock`，保护 Feedback 文件的读写

全局变量 `fpTaskLock` 持有当前正在处理的任务的 lock，在任务处理完毕后释放。

### 2.6 异常处理机制

| 机制 | 说明 |
|------|------|
| `searchUnnormaldRunningJobThread` | 后台线程，检测运行中的任务是否异常（job 文件丢失但 task 仍在运行等） |
| `ExceptionFilter` | SEH 异常过滤器，生成 MiniDump（`Dump2Engine-*.dmp`） |
| `HandlerRoutine` | Windows Console Control Handler，处理 `CTRL_CLOSE_EVENT` 等 |
| `PostQuitProcess` | 优雅退出：杀死 Task 子进程 → 将 job 移回 Pending → 删除 Engine 注册文件 |
| `DoCleanupLockFiles` | 清理已完成/取消/失败任务遗留的 .lock 文件 |

### 2.7 辅助线程

| 线程 | 函数 | 周期 | 职责 |
|------|------|------|------|
| 引擎心跳 | `execEngineTimeThread` | 1s | 刷新 `Engines/<hostname>.json`，包含当前任务信息 |
| 任务心跳 | `execTaskTimeThread` | 1s | 刷新 `task_def_N.json.pid`，记录 PID 和活跃时间 |
| Lock 清理 | `DoCleanupLockFiles` | 1s | 检查已结束的 job，清理遗留下的 .lock 文件 |

---

## 三、App/Task 模块详解

### 3.1 文件清单

| 文件 | 说明 |
|------|------|
| [MoldAITask.cpp](Task/MoldAITask.cpp) | **核心文件**，包含 main 函数及全部算法调度逻辑 |
| [MoldAITask.h](Task/MoldAITask.h) | 头文件，定义 `JobStatusInfo`/`SubTask` 等辅助类 |
| [ATPreprocessTask.cpp](Task/ATPreprocessTask.cpp) | `RunBatchPrePare` 算法实现（批量预处理） |
| [ATPreprocessTask.h](Task/ATPreprocessTask.h) | 预处理算法声明 |
| [MyrapidJson.cpp](Task/MyrapidJson.cpp) | RapidJSON 封装（与 Engine 版本重复） |
| [MyrapidJson.h](Task/MyrapidJson.h) | JSON 封装头文件 |
| [CMakeLists.txt](Task/CMakeLists.txt) | 构建脚本 |
| [MoldAINode.rc](Task/MoldAINode.rc) | Windows 资源文件 |

### 3.2 架构设计

```
MoldAITask.exe 启动
    │
    ▼
main(argc, argv)
    ├─ 异常处理注册 (ExceptionFilter, HandlerRoutine)
    ├─ 信号处理 (SIGINT, SIGBREAK, SIGTERM)
    ├─ GDAL/PROJ 环境变量设置
    │
    ▼
doTaskInProcess(argc, argv)
    ├─ argc==2: execTaskFile2Only(taskfile)    ← 正常执行
    └─ argc==3: execCancelCommandOnly(taskfile) ← 取消命令
```

### 3.3 核心执行流程（execTaskFile2Only）

```
execTaskFile2Only(taskFilePath)
│
├─ 解析 task 文件 → ATTaskInfo
│   ├─ function (算法函数名)
│   ├─ type (任务类型)
│   ├─ projectFile_, blockItem_, job_
│   └─ task_.id_
│
├─ 确定 job 类型 (AT / BATCH / TILE)
│
├─ 设置反馈文件路径
├─ 检查 feedback 状态（若已 Cancel/Fail 则退出）
│
├─ 更新 feedback 为 RUNNING 状态
│
├─ 按 function 名分发算法：
│   ├─ RunGenTasks          → 空三任务拆分
│   ├─ RunFeatureDetection  → 特征点检测
│   ├─ RunMatchPairs        → 图像匹配
│   ├─ RunPairSelection     → 匹配对筛选
│   ├─ RunSfM               → 运动恢复结构
│   ├─ RunReconstruction    → 三维重建
│   └─ RunBatchPrepare      → 批量预处理
│
├─ 算法执行期间通过 cbProgress 回调更新进度
│
└─ 算法返回后：
    ├─ resultCode == MOLDAI_SUCCESS    → feedback.Status = RUNNING
    ├─ resultCode == MOLDAI_USER_CANCEL → feedback.Status = CANCEL
    └─ 其他                           → feedback.Status = FAILURE
```

### 3.4 进度回调机制（cbProgress）

```cpp
cbProgress(float fvalue)
    ├─ 检查 current_job_file 是否存在
    ├─ 加载 current_feedback_file
    ├─ 若 feedback 状态为 Cancel/Fail → 直接返回（停止更新）
    ├─ 根据 job 类型计算实际进度百分比
    │   ├─ JOB_AT: curjob.tg.GetProgress(fvalue, task_id)
    │   ├─ JOB_BATCH: fvalue
    │   └─ JOB_TILE: fvalue * 100
    ├─ 只在进度增长时更新 feedback.Percent
    ├─ feedback.Status = RUNNING
    └─ feedback.save_with_retry(...)
```

### 3.5 取消机制（execCancelCommandOnly）

当以 3 个参数启动（`MoldAITask.exe cancel <taskfile>`）时：

1. 解析 task 文件
2. 找到对应算法函数
3. 调用 `RunCancel` 通知算法层取消
4. 设置 `GShoudStop = true` 通知运行中的算法线程停止

### 3.6 异常处理

| 机制 | 说明 |
|------|------|
| `ExceptionFilter` | SEH → MiniDump（`Dump4MoldAITask-*.dmp`）→ `exit(1099)` |
| `HandlerRoutine` | 控制台事件处理，标记 `GShoudStop = true` |
| `SigInt_Handler` / `SigBreak_Handler` | POSIX 信号处理 → `exit(0)` / `exit(1099)` |
| `customized_exit` | atexit 注册，打印退出日志 |

---

## 四、Engine 与 Task 交互协议

```
┌──────────────┐           ┌──────────────┐
│   Engine     │           │    Task      │
│ MoldAINode   │           │  MoldAITask  │
└──────┬───────┘           └──────┬───────┘
       │                          │
       │  1. 写 job 到 Running/   │
       │  2. 写 feedback (RUNNING)│
       │                          │
       │  3. QProcess 启动 ──────►│
       │     传参: taskfile path  │
       │                          │
       │                    4. 读取 task 文件
       │                    5. 执行算法
       │                    6. 定期写 feedback (进度%)
       │                          │
       │  7. 阻塞等待 ◄────────── │
       │                          │
       │  8. 读 feedback (最终状态)
       │  9. 按状态移动 job:
       │     Running→Completed
       │     Running→Failed
       │     Running→Cancelled
       │                          │
       ▼                          ▼
```

### 文件系统交互

| 文件 | 写入方 | 读取方 | 用途 |
|------|--------|--------|------|
| `job_*.json` | Engine | Engine/Task | Job 元信息、TaskGraph |
| `task_def_N.json` | Engine (GenTasks) | Task | 任务定义（算法名、参数） |
| `job_feedback_*.json` | Task (进度) / Engine (最终状态) | Engine/Task | 任务状态和进度 |
| `*.lock` | Engine/Task | Engine/Task | 互斥锁，防止并发写 |
| `*.pid` | Engine | Engine | 记录 Task PID 和活跃时间 |
| `Engines/<host>.json` | Engine | App 层 | 引擎上线注册 |

---

## 五、关键常量与配置

### 5.1 任务类型

| 常量 | 值 | 含义 |
|------|-----|------|
| `ATSTARTTYPE` | 1 | 空三起始任务（需要拆分） |
| `RECONSTRUCTIONSTARTTYPE` | 2? | 重建起始任务 |
| `ATLASTTASKTYPE` | 3? | 空三最后一个任务 |
| `ATCOMPLETETYPE` | 4? | 完成标记类型 |

### 5.2 Job 状态

| 状态 | 含义 |
|------|------|
| `STATUS_PENDDING` | 等待中 |
| `STATUS_RUNNING` | 执行中 |
| `STATUS_COMPLETE` | 已完成 |
| `STATUS_CANCLE` | 已取消 |
| `STATUS_FAILURE` | 执行失败 |

### 5.3 返回值

| 常量 | 值 | 含义 |
|------|-----|------|
| `MOLDAI_SUCCESS` | 100000 | 执行成功 |
| `MOLDAI_USER_CANCEL` | 100001 | 用户取消 |
| `AI3D_SUCCESS` | 0 | 操作成功 |
| 1099 | - | Task 崩溃退出码 |

### 5.4 文件格式控制宏

系统支持 JSON 和 BIN 两种文件格式，通过宏切换：

- `JOB_INFO_USE_BIN` — Job 文件格式
- `TASK_USE_BIN` — Task 文件格式
- `JOB_FEEDBACK_USE_BIN` — Feedback 文件格式
- `ENGINE_USE_BIN` — Engine 注册文件格式
- `STAT_USE_BIN` — 统计信息格式
- `BLK_USE_BIN` — Block 文件格式

---

## 六、代码质量问题

### 6.1 严重问题

1. **代码重复**：`XLog` 类在 [CallEngine.h](Engine/CallEngine.h) 和 [MoldAITask.h](Task/MoldAITask.h) 中几乎完全相同，[MyrapidJson.cpp](Engine/MyrapidJson.cpp) 在 Engine 和 Task 中完全重复。应提取到公共库。
2. **全局变量过多**：`CallEngine.cpp` 有 30+ 个文件级全局变量（`gotNewPendingJobFile`, `taskrunning`, `fpTaskLock` 等），状态管理极其脆弱。
3. **goto 语句**：[CallEngine.cpp:733](Engine/CallEngine.cpp#L733) 使用 `goto Retry_It` 实现重试逻辑。
4. **使用已弃用 API**：`vsprintf_s`, `_vscprintf`, `GetVersionEx` 等。
5. **线程安全缺失**：多个线程同时读写全局变量（如 `gotNewPendingJobFile`, `fpTaskLock`），无 mutex 保护。
6. **硬编码路径**：[BootProbe.cpp:15](Engine/BootProbe.cpp#L15) 硬编码了日志路径 `C:\Users\Microsoft\Code\...`，在不同环境中会失败。
7. **空循环体/死代码**：大量 `if (1)`, `#if 0`, 空的 catch 块，空的 if/else 分支。

### 6.2 中等问题

1. **Win32 耦合过深**：大量 `#ifdef WIN32` / `#ifdef _MSC_VER`，`Windows.h` 深耦合。
2. **字符串编码混乱**：`qstr2str`/`str2qstr` (QString↔std::string), `UTF82GBK` (注释掉), 混用 `QString::toStdString()` 和自定义转换。
3. **Magic Number**：`Sleep(100)`, `Sleep(500)`, `Sleep(1000)` 散落各处，无注释说明等待原因。
4. **函数过长**：`ExecTaskFileV2` ~900 行, `main` ~230 行, `ProcessUnnormaldRunningJob` ~320 行。
5. **资源泄漏风险**：`new int(0)` 在 `ATPreprocessTask.cpp` 中未配对 `delete`。
6. **内存泄漏**：`msgBoxThread = new MsgBoxThread(nullptr)` 从未释放。

### 6.3 轻度问题

1. **中文注释混用**：注释既有中文也有英文。
2. **命名不一致**：`ATLASTTASKTYPE` vs `ATCOMPLETETYPE`，部分宏/常量命名风格不统一。
3. **缺少单元测试**：整个模块未见任何测试代码。

---

## 七、依赖关系

```
Engine (MoldAINode.exe)
├── Qt6::Core, Qt6::Widgets, Qt6::Network, Qt6::Xml, Qt6::Sql, Qt6::OpenGL
├── Boost (algorithm/string)
├── RapidJSON
├── AI3D Core (File, Logging, TaskDef, ReturnCode, WorkPath)
├── AI3D Util (CatchProcess, Settings, TaskProcess, JobMonitor, Statistic, OTA)
├── Ceres Solver, OpenSSL, libcurl, libexpat, GLog
└── MoldAIData

Task (MoldAITask.exe)
├── Qt6::Core, Qt6::Widgets, Qt6::Network, Qt6::Xml, Qt6::Sql, Qt6::OpenGL
├── Boost (algorithm/string)
├── RapidJSON
├── AI3D Core (File, ATData, BlockObject, CoordinateSystem, TaskDef, Application)
├── AI3D Util (CatchProcess, Settings, TaskProcess, Statistic, JobMonitor, OTA)
├── Reconstruction (Reconstruct.h, RunSfM, RunFeatureDetection, etc.)
├── Ceres Solver, OpenSSL, libcurl, libexpat
└── MoldAIData, MoldAIBase.dll, MoldAIProc.dll
```

---

## 八、总体评价

**优点**：
- Engine/Task 分离设计合理，子进程崩溃不影响引擎稳定性
- 文件锁机制保证了多进程/多引擎场景下的互斥访问
- 状态目录设计直观，便于运维排查（直接查看文件系统）
- MiniDump 崩溃转储机制便于生产环境问题定位

**主要风险**：
- 全局状态管理混乱，并发场景下可能出现竞态条件
- 代码可维护性差，函数过长且职责不清
- 硬编码路径和平台耦合限制了可移植性
- 缺乏测试覆盖，回归风险高
