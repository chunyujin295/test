# Engine 层与 Core 层的调用关系分析

## 一、概览

Engine（`MoldAINode.exe`）通过 **三层间接引用** 使用 Core 层（`MoldAIData.dll`）的 API：

```
MoldAINode.exe (Engine)
    │
    ├── 直接引用 Core 头文件  ← 6 个 Core 头文件
    │   ├── Core/File.h         (文件路径/IO/锁)
    │   ├── Core/TaskDef.h      (ATTaskInfo / TaskDescriptor)
    │   ├── Core/ReturnCode.h   (AI3D_SUCCESS 等)
    │   ├── Core/ReturnState.h  (MOLDAI_SUCCESS 等)
    │   ├── Core/Logging.h      (LOGI/LOGE 等日志宏)
    │   └── Core/WorkPath.h     (GetWorkPath())
    │
    ├── 通过 Util 层间接引用  ← 5 个 Util 头文件
    │   ├── Util/TaskProcess.h → 引用 Core/Types.h, Core/File.h, Core/String.h, Core/TaskDef.h, Core/Logging.h
    │   ├── Util/JobMonitor.h  → 引用 Core/Types.h, Core/String.h
    │   ├── Util/Settings.h
    │   ├── Util/Statistic.h   → 包含 EngineInfo_s 等
    │   └── Util/CatchProcess.h
    │
    └── Engine 自己的 MyrapidJson.cpp
        └── 引用 Core/File.h, Core/Logging.h, RapidJSON
```

**关键发现**: Engine 不直接依赖 Core 层的重量级数据模型（ATData, BlockObject, Camera, Image 等），而是通过 **文件系统** 间接操作这些数据——Task 子进程负责加载和操作这些模型。

---

## 二、Core API 分类与使用频次

### 2.1 File.h — 文件系统 API（最频繁，~90+ 次调用）

这是 Engine 使用最密集的 Core API，几乎每个函数都在用。

| API | 调用次数 | 用途 |
|-----|----------|------|
| `File::FopenDenyWriteLockUtf8` | ~25 | 对 Job/Task/Feedback 文件加排他写锁 |
| `File::BoostPathFromUtf8` + `std::filesystem::exists` | ~18 | 检查文件/目录是否存在 |
| `File::GetParentDir` | ~15 | 从文件路径提取父目录 |
| `File::OpenIfstreamUtf8` | ~6 | 以 UTF-8 路径打开输入流 |
| `File::OpenOfstreamUtf8` | ~3 | 以 UTF-8 路径打开输出流 |
| `File::EnsureUnifySlash` | ~5 | 统一路径分隔符 |
| `File::EnsureTrailingSlash` | ~3 | 确保路径以分隔符结尾 |
| `File::JoinPaths` | ~2 | 拼接多段路径 |
| `File::GetFileNameWithoutExtension` | ~1 | 获取无后缀文件名 |
| `File::GetFileList` | ~1 | 获取目录下文件列表 |
| `File::RemoveFile` | ~2 | 删除文件 |
| `File::BoostPathToUtf8String` | ~8 | Boost 路径转 UTF-8 字符串 |

**典型调用模式**（文件锁 + 路径拼接）：

```cpp
// 1. 加锁保护临界区
FILE* fpjob = AICORE::File::FopenDenyWriteLockUtf8(qstr2str(jobFilePathLock));

// 2. 路径构造
std::string taskBasePath = AICORE::File::GetParentDir(blockpath) + "/" + block + "/" + jobName + "/";
taskpath = AI3D::CORE::File::JoinPaths(basepath, block, jobName);

// 3. 文件存在检查
if (std::filesystem::exists(AICORE::File::BoostPathFromUtf8(job_file)))

// 4. 打开文件流
std::ifstream in = AICORE::File::OpenIfstreamUtf8(file, std::ios::binary);
```

### 2.2 Types.h — 类型与常量（~100+ 次引用）

通过 `Util/TaskProcess.h` 间接引入，Engine 自身也大量使用：

| API | 调用次数 | 用途 |
|-----|----------|------|
| `jobsta_e` (STATUS_PENDDING/RUNNING/COMPLETE/CANCLE/FAILURE) | ~60+ | 任务状态判断与流转 |
| `ATSTARTTYPE / RECONSTRUCTIONSTARTTYPE / ATCOMPLETETYPE / ATLASTTASKTYPE` | ~20+ | 任务类型分支 |
| `JOB_INFO_USE_BIN / TASK_USE_BIN / JOB_FEEDBACK_USE_BIN / ENGINE_USE_BIN` | ~25+ | 文件格式切换 |
| `BINFILE_POSTFIX / JSONFILE_POSTFIX` | ~15+ | 文件后缀选择 |
| `MAKE_FEEDBAK_BIN_FILE / MAKE_FEEDBAK_JSON_FILE / MAKE_TASK_BIN_FILE / MAKE_TASK_JSON_FILE` | ~15+ | 文件路径宏 |
| `JOBPENDINGSTR / JOBRUNNINGSTR / JOBCOMPLETEDSTR / JOBFAILEDSTR / JOBCANCELLEDSTR` | ~5 | 状态目录路径 |
| `PATH_SEPARATOR_STR` | ~2 | 路径分隔符 |

**状态目录初始化**（直接使用 Types.h 宏）:
```cpp
pendingJobPath   = Settings::getEngineJobQueue() + pathSeperator + JOBPENDINGSTR + pathSeperator;
runningJobPath   = Settings::getEngineJobQueue() + pathSeperator + JOBRUNNINGSTR + pathSeperator;
completedJobPath = Settings::getEngineJobQueue() + pathSeperator + JOBCOMPLETEDSTR + pathSeperator;
failedJobPath    = Settings::getEngineJobQueue() + pathSeperator + JOBFAILEDSTR + pathSeperator;
cancelledJobPath = Settings::getEngineJobQueue() + pathSeperator + JOBCANCELLEDSTR + pathSeperator;
```

### 2.3 TaskDef.h — 任务定义（~18 次调用）

Engine 通过 `ATTaskInfo` 解析 task 文件来获取任务参数：

| 方法/字段 | 用途 |
|-----------|------|
| `ATTaskInfo::load(filepath)` | 从磁盘加载 task_def_N 文件（JSON 或 BIN） |
| `ATTaskInfo::task_.fun_name_` | 获取算法函数名（"RunGenTasks", "RunFeatureDetection" 等） |
| `ATTaskInfo::task_.type_` | 获取任务类型 |
| `ATTaskInfo::task_.id_` | 获取任务 ID |
| `ATTaskInfo::job_` | 获取 Job 标识名 |
| `ATTaskInfo::blockItem_` | 获取 Block 路径 |
| `ATTaskInfo::projectFile_` | 获取项目文件路径 |

**核心调用模式**——Engine 通过 `ATTaskInfo` 获取调度所需的所有元数据：

```cpp
// 所有关键分支都从这里开始
ATTaskInfo attask;
attask.load(fileName);

std::string function = attask.task_.fun_name_;  // 决定执行哪个算法
int type = attask.task_.type_;                  // 决定处理流程
int taskid = attask.task_.id_;                  // 用于 tasksmap 查找
std::string jobname = attask.job_;              // 用于定位 job/feedback 文件
std::string blockpath = attask.projectFile_;    // 用于定位 Block 数据
```

**Engine 使用 ATTaskInfo 的 6 个关键函数**：

| 函数 | 行号 | 场景 |
|------|------|------|
| `ProcessUnnormaldRunningJobV2()` | 752 | 异常 Job 检测 |
| `GetRunningTaskInRunningJob()` | 842 | Running 队列任务恢复 |
| `ExecTaskFileV2()` | 2500 | 主调度执行入口 |
| Task 列表遍历 | 2572 | 构建 maptaskfunction |
| 失败处理（PostQuitProcess 路径） | 2975, 3986, 4161 | 重置/回滚 |
| Job 路径拼接工具函数 | 3778 | getATBlockJobPath |

### 2.4 Logging.h — 日志系统（~120+ 次调用）

```cpp
LOGI(msg);   // 最常用，记录关键节点
LOGE(msg);   // 错误日志
// 宏: CHECK_OPTION, CHECK_LOG (断言检查)
```

### 2.5 ReturnCode.h / ReturnState.h — 返回值（~15 次调用）

| 常量 | 用途 |
|------|------|
| `MOLDAI_SUCCESS` (100000) | 任务成功 |
| `MOLDAI_USER_CANCEL` (100001) | 用户取消 |
| `MOLDAI_FAILURE` (100001) | 任务失败 |
| `AI3D_SUCCESS` (1000) | 操作成功 |
| 1099 (隐式约定) | Task 进程崩溃 |

**Engine 在 Task 子进程返回后的分支逻辑**：
```cpp
int iTaskRetVal = process.exitCode();

if (iTaskRetVal == MOLDAI_SUCCESS && type == ATSTARTTYPE)
    // → 注册子任务到 tasksmap，继续调度
if (iTaskRetVal == MOLDAI_USER_CANCEL || feadback.Status == STATUS_CANCLE)
    // → Job 移到 Cancelled/
if (iTaskRetVal > MOLDAI_SUCCESS || feadback.Status == STATUS_FAILURE)
    // → Job 移到 Failed/，生成 MiniDump
```

### 2.6 WorkPath.h — 工作路径（1 次调用）

```cpp
std::string workPath = GetWorkPath();  // 获取用户数据根目录
```

---

## 三、通过 Util 层的间接调用

Util 层是 Engine 和 Core 之间的 **桥接层**，它封装了大量 Core API：

### 3.1 Util/TaskProcess.h — 核心任务数据处理

`TaskProcess.h` 是 Engine 引入的最重要的 Util 头文件，它内部使用了以下 Core API：

```
Util/TaskProcess.h 引用的 Core 头文件:
├── Core/Types.h          → 全部类型/枚举/宏/常量
├── Core/File.h           → File::OpenIfstreamUtf8, OpenOfstreamUtf8, FopenDenyWriteLockUtf8
├── Core/String.h         → String::LocaleToUtf8, StringSplit
├── Core/TaskDef.h        → TaskDescriptor, ATTaskInfo
├── Core/StringResource.h → 阶段文字常量
├── Core/Logging.h        → LOGI/LOGE
└── Core/json.h           → nlohmann::json (完整 JSON 库)
```

**Engine 通过 TaskProcess.h 使用的关键结构**（虽在 Util 中定义，但底层依赖 Core）：

| 结构体 | 底层 Core 依赖 | Engine 使用场景 |
|--------|---------------|----------------|
| `JobFeedBack_s` | `jobsta_e` (Types.h), `File`, `String` | 反馈文件读写、状态判断 |
| `TaskGraph_s` | `JobInfo_s`, `RunInfo_s`, `Task_s`, `JobFeedBack_s` | 任务图管理、依赖检查 |
| `Task_s` | `jobsta_e` (Types.h) | tasksmap 节点 |
| `JobFullInfo_s` | `TaskGraph_s` | Job 文件完整加载/保存 |
| `EngineInfo_s` | Core/Types.h 类型 | 引擎心跳注册 |

### 3.2 Util/JobMonitor.h — 任务监控

引用 `Core/Types.h`, `Core/String.h`，提供 `GetJobListsInfo()` 供 Engine 查询集群中所有引擎和 Job 的状态。

### 3.3 Util/Statistic.h — 统计信息

引用 `EngineInfo_s` (TaskProcess.h)，提供 `EngineInfo` 单例类管理引擎注册信息。

---

## 四、Engine 中 Core API 调用的完整生命周期

以下按 Engine 的调度流程追踪 Core API 的使用：

```
Engine 启动 (main)
│
├─ [Types.h] JOBPENDINGSTR, JOBRUNNINGSTR 等 → 初始化状态目录路径
├─ [WorkPath.h] GetWorkPath() → 获取数据根目录
├─ [Types.h] ENGINE_USE_BIN → 选择引擎注册文件格式
├─ [File.h] BoostPathFromUtf8 + exists → 检查引擎注册文件
│
├─ searchPendingJobThread2 (主循环)
│   │
│   ├─ GetRunningTaskInRunningJob (优先处理 Running 队列)
│   │   ├─ [TaskDef.h] ATTaskInfo::load() → 解析 task_def_N
│   │   ├─ [TaskDef.h] attask.job_, attask.projectFile_, attask.blockItem_
│   │   ├─ [Types.h] JOB_INFO_USE_BIN, BINFILE_POSTFIX → 确定文件格式
│   │   ├─ [File.h] FopenDenyWriteLockUtf8 → 对 job/task 文件加锁
│   │   ├─ [TaskProcess.h] JobFullInfo_s::load() → 加载 job 文件
│   │   ├─ [TaskProcess.h] TaskGraph_s::tasksmap → 遍历任务图
│   │   ├─ [Types.h] jobsta_e::STATUS_COMPLETE/CANCLE/FAILURE → 状态判断
│   │   └─ [TaskProcess.h] JobFeedBack_s::load_with_retry() → 读取反馈
│   │
│   └─ GetPendingJob (取新任务)
│       ├─ [Types.h] JOB_INFO_USE_BIN, BINFILE_POSTFIX → 文件格式
│       ├─ [File.h] GetFileList → 列出 Pending 文件
│       ├─ [File.h] FopenDenyWriteLockUtf8 → 对 pending job 加锁
│       ├─ [TaskProcess.h] JobFullInfo_s → 读 job 文件
│       ├─ [TaskProcess.h] SetPendingInfo → 设置提交信息
│       ├─ [Types.h] MAKE_FEEDBAK_XXX_FILE → 构造 feedback 路径
│       ├─ [Types.h] MAKE_TASK_XXX_FILE → 构造 task 文件路径
│       └─ [File.h] EnsureUnifySlash, JoinPaths → 路径拼接
│
├─ ExecTaskFileV2 (执行任务)
│   ├─ [TaskDef.h] ATTaskInfo::load() → 解析 task 文件
│   ├─ [TaskDef.h] attask.task_.fun_name_ → 获取算法函数名
│   ├─ [TaskDef.h] attask.task_.type_ → 获取任务类型
│   ├─ [TaskDef.h] attask.task_.id_ → 获取任务 ID
│   ├─ [TaskDef.h] attask.job_, attask.projectFile_, attask.blockItem_
│   ├─ [Types.h] JOB_FEEDBACK_USE_BIN, MAKE_FEEDBAK_XXX_FILE → feedback 路径
│   ├─ [File.h] GetParentDir → 提取父目录
│   ├─ [TaskProcess.h] JobFeedBack_s::load_with_retry → 读反馈
│   ├─ [Types.h] jobsta_e::STATUS_PENDDING/RUNNING → 状态检查
│   │
│   ├─ [ATSTARTTYPE 分支] GenTasks 任务拆分
│   │   └─ [TaskDef.h] ATTaskInfo::load → 遍历子 task 文件构建 maptaskfunction
│   │
│   ├─ [ATLASTTASKTYPE/ATCOMPLETETYPE 分支] 完成处理
│   │   ├─ [TaskProcess.h] TaskGraph_s::IsTaskComplete → 依赖检查
│   │   ├─ [TaskProcess.h] JobFullInfo_s::save → 保存 job
│   │   └─ [Types.h] jobsta_e::STATUS_COMPLETE → 标记完成
│   │
│   └─ [其他类型] 启动 MoldAITask.exe 子进程
│       ├─ QProcess::start("MoldAITask.exe", {taskfile})
│       ├─ process.waitForFinished(-1) → 阻塞等待
│       └─ 子进程返回后:
│           ├─ [ReturnState.h] MOLDAI_SUCCESS / MOLDAI_USER_CANCEL
│           ├─ [TaskProcess.h] JobFeedBack_s::load_with_retry → 读最终反馈
│           ├─ [Types.h] jobsta_e → 判断 final status
│           ├─ [File.h] FopenDenyWriteLockUtf8 → 加锁后移动文件
│           ├─ [File.h] BoostPathFromUtf8 + exists → 检查文件
│           ├─ [File.h] RemoveFile → 删除旧文件
│           └─ [Logging.h] LOGI/LOGE → 记录结果
│
├─ execEngineTimeThread (心跳线程)
│   ├─ [Types.h] ENGINE_USE_BIN → 文件格式
│   ├─ [File.h] BoostPathFromUtf8 + exists → 存在检查
│   └─ [File.h] OpenOfstreamUtf8 → 写引擎心跳
│
├─ execTaskTimeThread (PID 心跳线程)
│   ├─ [File.h] OpenOfstreamUtf8 → 写 .pid 文件
│   │   └── PIDFile { pid, taskFile, lastActivateTime }
│   └─ [File.h] OpenIfstreamUtf8 → 读 .pid 文件 (checkTaskInstanceStatus)
│
└─ DoCleanupLockFiles (锁清理线程)
    ├─ [File.h] BoostPathFromUtf8 + exists
    ├─ [File.h] FopenDenyWriteLockUtf8
    └─ [Types.h] JOB_INFO_USE_BIN, BINFILE_POSTFIX
```

---

## 五、Engine NOT 使用的 Core API

Engine 刻意避免使用以下 Core 层重量级 API（这些由 Task 子进程使用）：

| Core API | 为何不使用 | 谁使用 |
|----------|-----------|--------|
| `ATData` (空三数据集) | Engine 不操作空三数据本身 | Task (MoldAITask.exe) |
| `BlockObject` (区块管理) | 级联了大量依赖（ATData/Camera/Image 等） | Task + GUI |
| `Camera / Image / Point3D / Track` | 属于算法数据层 | Task |
| `AlgorithmBase` (三角化/投影) | 属于算法层 | Task |
| `CoordinateSystem / Proj/*` | 坐标变换不是调度职责 | Task + GUI |
| `ATCommandSet / ReconstructionCommandSet` | 提交任务的逻辑在 GUI 层 | GUI |
| `Tiling` (分块策略) | 重建阶段使用 | Task (RunReconstruction) |
| `ExifIO` (EXIF 读取) | 预处理阶段使用 | Task |
| `SimilarityTransform3` | 数学算法 | Task |
| `ControlPoint / ControlPoints` | GCP 数据管理 | Task + GUI |

**架构原则**: Engine 是纯调度器，不理解"图像、点云、空三"等业务概念，只理解"Job、Task、状态文件"。

---

## 六、调用关系总图

```
┌─────────────────────────────────────────────────────────────────┐
│                    MoldAINode.exe (Engine)                       │
│                                                                  │
│  直接使用的 Core API:                                             │
│  ┌──────────┐ ┌───────────┐ ┌──────────┐ ┌──────────┐           │
│  │ File.h   │ │ TaskDef.h │ │ Logging.h│ │ Types.h  │           │
│  │ ~90 calls│ │ ~18 calls │ │~120 calls│ │~100 refs │           │
│  │ 文件锁    │ │ ATTaskInfo│ │ LOGI/LOGE│ │ 枚举/宏   │           │
│  │ 路径工具  │ │ 任务解析   │ │ CHECK_OPT│ │ 状态/格式  │           │
│  └────┬─────┘ └────┬──────┘ └────┬─────┘ └────┬─────┘           │
│       │            │             │            │                  │
│  ┌────┴────────────┴─────────────┴────────────┴─────┐            │
│  │              Util 桥接层                           │            │
│  │  ┌──────────────┐ ┌──────────┐ ┌──────────────┐   │            │
│  │  │ TaskProcess.h│ │JobMonitor│ │  Statistic.h │   │            │
│  │  │ JobFeedBack_s│ │.h        │ │  EngineInfo  │   │            │
│  │  │ TaskGraph_s  │ │          │ │              │   │            │
│  │  │ JobFullInfo_s│ │          │ │              │   │            │
│  │  └──────┬───────┘ └────┬─────┘ └──────┬───────┘   │            │
│  └─────────┼──────────────┼──────────────┼───────────┘            │
│            │              │              │                        │
│            │    内部再引用 Core/Types.h, Core/File.h,             │
│            │    Core/String.h, Core/TaskDef.h, Core/Logging.h     │
└────────────┼──────────────┼──────────────┼────────────────────────┘
             │              │              │
             ▼              ▼              ▼
┌─────────────────────────────────────────────────────────────────┐
│                   MoldAIData.dll (Core)                          │
│                                                                  │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────────────┐   │
│  │ Types.h  │ │ File.h   │ │TaskDef.h │ │ Logging.h        │   │
│  │ 枚举/常量 │ │ 路径/IO  │ │ 任务定义  │ │ CHECK/LOGI/LOGE  │   │
│  └──────────┘ └──────────┘ └──────────┘ └──────────────────┘   │
│                                                                  │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │  Engine 不使用，但与 Engine 间接相关（Task 子进程使用）:       │   │
│  │  ATData, BlockObject, Camera, Image, Point3D, Track,       │   │
│  │  ControlPoint, AlgorithmBase, CoordinateSystem, Tiling,    │   │
│  │  ExifIO, CameraDatabase                                   │   │
│  └──────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘

    Engine 间接操作（通过文件系统和 Task 子进程）────────────────────►
    
┌─────────────────────────────────────────────────────────────────┐
│                   MoldAITask.exe (Task)                          │
│                                                                  │
│  直接使用的 Core API:                                             │
│  ┌──────────┐ ┌──────────┐ ┌───────────┐ ┌────────────────┐    │
│  │ File.h   │ │TaskDef.h │ │ATCommand  │ │ AlgorithmBase  │    │
│  │          │ │          │ │Set.h      │ │                │    │
│  ├──────────┤ ├──────────┤ ├───────────┤ ├────────────────┤    │
│  │ ATData   │ │BlockObj  │ │CoordSystem│ │ Camera/Image   │    │
│  │          │ │          │ │           │ │ /Point3D/Track │    │
│  └──────────┘ └──────────┘ └───────────┘ └────────────────┘    │
└─────────────────────────────────────────────────────────────────┘
```

---

## 七、关键设计决策

### 7.1 文件系统作为接口边界

Engine 和 Task 之间不共享任何内存中的 Core 对象（如 `ATData`, `BlockObject`），而是：

1. Engine 通过 `ATTaskInfo::load()` 只读取 task 文件的**元数据**（函数名、类型、ID、路径）
2. Engine 将 task 文件**路径**作为命令行参数传给 Task 子进程
3. Task 子进程自行调用 `ATTaskInfo::load()` 获取完整参数，再加载 `BlockObject`、`ATData` 等重量级数据
4. 状态同步通过 `JobFeedBack_s` 反馈文件实现

### 7.2 格式切换的传染性

`Types.h` 中的 `XXX_USE_BIN` 宏在 Engine 中大量使用，形成了典型的 if-else 模式：

```cpp
if (JOB_INFO_USE_BIN)    { postFix = BINFILE_POSTFIX;  }
else                     { postFix = JSONFILE_POSTFIX; }

if (JOB_FEEDBACK_USE_BIN) { file = MAKE_FEEDBAK_BIN_FILE(...); }
else                      { file = MAKE_FEEDBAK_JSON_FILE(...); }

if (TASK_USE_BIN)        { taskfile = MAKE_TASK_BIN_FILE(...); }
else                     { taskfile = MAKE_TASK_JSON_FILE(...); }
```

这些宏是 **编译期**常量，意味着整个系统的文件格式在编译时就固定了。

### 7.3 字符串编码边界

Engine 内部使用 QString（Qt），Core 层使用 std::string（UTF-8）。边界处需要转换：

```cpp
// Util/TaskProcess.h 提供的转换函数
static QString str2qstr(std::string str) { return QString::fromUtf8(...); }
static std::string qstr2str(QString qstr) { return qstr.toUtf8().toStdString(); }

// Core/File.h 提供的 UTF-8 路径支持
File::OpenIfstreamUtf8(path, mode);    // std::string UTF-8 → 文件流
File::BoostPathFromUtf8(path);         // UTF-8 → boost::filesystem::path
File::BoostPathToUtf8String(path);     // boost::path → UTF-8 std::string
```

### 7.4 日志系统的统一

Engine 使用 Core/Logging.h 的 `LOGI/LOGE` 宏，底层基于 glog。Engine 在 `main()` 中通过 `InitializeLogEngine(argv)` 初始化日志系统。这意味着 Engine 和 Core 共享同一套日志输出（glog sink）。

---

## 八、总结

| 维度 | 数据 |
|------|------|
| Engine 直接 include 的 Core 头文件 | 6 个 |
| Engine 通过 Util 间接使用的 Core 头文件 | 7 个 |
| File.h API 调用次数 | ~90 |
| Logging.h 宏调用次数 | ~120 |
| Types.h 枚举/宏引用次数 | ~100 |
| TaskDef.h API 调用次数 | ~18 |
| Engine **未使用**的 Core 重量级 API | ATData, BlockObject, Camera, Image, Point3D, Track, ControlPoint, AlgorithmBase, CoordinateSystem, Tiling 等 |

**核心结论**: Engine 对 Core 的依赖是 **薄层依赖**——它只使用 Core 层最底层的基础设施（文件 I/O、类型定义、任务文件解析、日志），而将重量级的领域模型（空三数据、重建、坐标变换）完全留给 Task 子进程。这种设计实现了调度逻辑与算法逻辑的**进程级隔离**。
