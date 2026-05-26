# App/Engine 数据结构分析文档

## 一、概览

Engine（`MoldAINode.exe`）使用的数据结构分布在三个层次：

| 层次 | 来源 | 角色 |
|------|------|------|
| **内存运行时结构** | `Util/TaskProcess.h` | Job/Task 图、反馈、引擎信息的运行时表示，支持 JSON/BIN 双序列化 |
| **磁盘文件格式** | `Core/DataStruct.h` | Task 文件、Feedback 文件、JobList 文件、PID 文件的二进制格式 |
| **引擎局部结构** | `CallEngine.cpp` | Engine 自己的状态变量和辅助结构 |

每种结构都同时支持 **JSON（nlohmann + RapidJSON）** 和 **BIN（异或加密二进制）** 两种序列化格式，由编译期宏切换。

---

## 二、核心数据流与结构关系图

```
Job 文件 (Running/Pending 目录)
├── job_xxx.json / job_xxx.bin
│   ├── JobInfo_s        → 项目路径 + Block 路径
│   ├── RunInfo_s         → 提交者/主机/时间 + 运行时信息(Run_s)
│   ├── JobFeedBack_s     → Status/Percent/Msg/TaskRetVal
│   └── TaskGraph_s       → 任务图
│       └── tasksmap: map<int, Task_s>
│           ├── Id, FatherId, Depends (任务依赖关系)
│           ├── Status, Percent (每个任务状态)
│           └── Run_s (每个任务的运行信息)
│
├── task_def_0.json / task_def_0.bin  → ATTaskInfo
│   ├── TaskDescriptor   → 算法函数名/类型/ID/依赖/图片列表
│   ├── JobInfo_s        → 项目和Block路径
│   └── ATOptions        → SFM/BA/特征 配置参数
│
├── job_feedback_xxx.json / fdb_xxx.bin
│   └── FeedBackFile / JobFeedBack_s  → Status/Percent/Msg
│
├── task_def_N.json.lock   → 排他文件锁
├── task_def_N.json.pid    → PIDFile { pid, taskFile, lastActivateTime }
│
└── Engines/<hostname>.json → EngineInfo_s → 引擎心跳注册
```

---

## 三、Job 层数据结构（运行时核心）

### 3.1 JobInfo_s — Job 基础信息

**文件**: [TaskProcess.h:266](Include/Util/TaskProcess.h#L266)

```cpp
struct JobInfo_s {
    std::string ProjectPath;     // 项目路径
    std::string ItemPath;        // Block 路径
    std::string ProjectPath2;    // GBK 备份（已废弃，现等于 ProjectPath）
    std::string ItemPath2;       // GBK 备份（已废弃）
};
```

**Engine 中的使用场景**:
- 标识一个 Job 对应哪个项目（Project）的哪个区块（Block）
- Job 文件名由 ProjectPath 和 ItemPath 哈希生成
- 通过 `getATBlockJobPath()` 从 Job 文件名反推 Block 路径

### 3.2 RunInfo_s — 运行生命周期信息

**文件**: [TaskProcess.h:1064](Include/Util/TaskProcess.h#L1064)

```cpp
struct RunInfo_s {
    std::string SubmitHostName;  // 提交引擎主机名
    std::string SubmitUser;      // 提交用户
    std::string SubmitTime;      // 提交时间
    std::string EndTime;         // 结束时间
    Run_s runninginfo;           // 实际运行信息（谁在跑、开始/结束时间）
};
```

**Run_s**（子结构）:
```cpp
struct Run_s {
    std::string RunHostName;     // 执行引擎主机名
    std::string RunUserName;     // 执行用户
    std::string StartTime;       // 任务开始时间
    std::string EndTime;         // 任务结束时间
};
```

### 3.3 JobFeedBack_s — 反馈/进度信息

**文件**: [TaskProcess.h:1422](Include/Util/TaskProcess.h#L1422)

```cpp
struct JobFeedBack_s {
    jobsta_e Status;             // PENDDING/RUNNING/COMPLETE/CANCLE/FAILURE
    float Percent;               // 进度 0.0~100.0
    int TaskRetVal;              // 任务返回值 (-1=无, 100000=成功, 100001=取消)
    std::string Msg;             // 状态消息（如 "keypoints extracting"）
};
```

**Engine 中的使用**:
- Task 子进程通过 `cbProgress` 回调定期写入 feedback 文件
- Engine 通过 `LoadFeedbackFile()` 读取 feedback 监控任务进度
- Engine 在任务完成后根据 feedback 的 Status 决定 Job 去向（Complete/Cancel/Fail）

**磁盘对应**: `FeedBackFile` (DataStruct.h:1731) — 带 "FEED-FILE-3MO" 魔术头的异或加密格式

### 3.4 Task_s — 单个任务节点

**文件**: [TaskProcess.h:2098](Include/Util/TaskProcess.h#L2098)

```cpp
struct Task_s {
    std::string Msg;             // 任务描述
    float Percent;               // 当前进度
    int Status;                  // jobsta_e 枚举值
    int Type;                    // ATSTARTTYPE=1 / RECONSTRUCTIONSTARTTYPE=4 等
    std::string ProjectPath;     // 项目路径
    std::string ItemPath;        // Block 路径
    int Id;                      // 任务ID
    int FatherId;                // 父任务ID（-1=无父任务，即由 GenTasks 拆分产生）
    std::set<int> Depends;       // 依赖的前置任务ID集合
    Run_s runinfo;               // 此任务的运行信息
};
```

**依赖关系（DAG）**:
- `FatherId = -1`: 顶级任务，无父任务
- `Depends`: 必须等待 `Depends` 中所有子任务完成，且 `FatherId` 对应的父任务完成，此任务才算 `IsTaskComplete()`
- 这形成了 AT 流水线的有向无环图（DAG）：`GenTasks → FeatureDetection → MatchPairs → SfM → Reconstruction`

### 3.5 TaskGraph_s — 任务图（核心容器）

**文件**: [TaskProcess.h:3541](Include/Util/TaskProcess.h#L3541)

```cpp
struct TaskGraph_s {
    JobInfo_s job;                     // Job 基础信息
    RunInfo_s runinfo;                 // 提交运行信息
    JobFeedBack_s feedback;            // 反馈状态
    std::map<int, Task_s> tasksmap;    // 任务图: taskId → Task_s
};
```

**关键方法**:
| 方法 | 用途 |
|------|------|
| `HasTaskDef0()` | 检查 task_0 是否为 ATSTARTTYPE（空三起始类型） |
| `IsTaskComplete(id)` | 递归检查某任务的所有依赖和父任务是否都已完成 |
| `GetFirstPendingTaskId()` | 遍历 tasksmap，返回第一个 PENDDING 状态且条件满足的任务 |
| `GetLastRunningTaskId()` | 返回当前 RUNNING 的任务ID |
| `SetPendingInfo(type, job, runinfo, msg)` | 初始化 Job 并创建 task_0 |
| `GetSubTasksPercent(taskid, lastvalue, depends)` | 基于子任务完成数计算父任务进度百分比 |

**Engine 中的核心逻辑**:
- `searchPendingJobThread2()` 中：先处理 `Running/` 目录中 tasksmap 有未完成任务（PENDDING/Waiting/NotStarted）的 Job
- 遍历 tasksmap 找到下一个应执行的 task，启动 MoldAITask.exe 执行
- task_0 执行 `RunGenTasks` 后会拆分成 feature detection → matching → SfM 等子任务，填充 tasksmap
- 所有子任务完成后，job 从 Running 移到 Completed

### 3.6 TaskTime_s — 任务耗时统计

**文件**: [TaskProcess.h:2526](Include/Util/TaskProcess.h#L2526)

```cpp
struct TaskTime_s {
    int Id;
    int Type;
    int Status;
    std::string FunctionName;    // "RunFeatureDetection", "RunSfM" 等
    std::string StartTime;
    std::string EndTime;
};
```

**用途**: 统计每个 pipeline 阶段的耗时（用于性能分析和 Job 完成后的时间汇总文件 `timesum_*.bin`）

---

## 四、磁盘文件格式结构（DataStruct.h）

DataStruct.h 定义了一套完整的 **异或加密（XOR key=0xAB）+ 魔术头校验** 的二进制序列化体系。

### 4.1 加密基类 ByteCrypt

```cpp
struct ByteCrypt {
    uint16_t kInvalideNum = 0;
    unsigned char SOURCE_XOR_KEY = 0xAB;  // 固定异或密钥
    // ReadByteDecrypted: file.read → XOR decrypt
    // WriteByteDecrypted: XOR encrypt → file.write
};
```

所有 BIN 格式都共享此加密机制。这**不是安全加密**，仅防止文本编辑器直接查看。

### 4.2 SPTaskInfoFile — Task 定义文件（task_def_N.bin）

**魔术头**: `"TASKDEF-FILE-3MO"` (16字节)

```
SPTaskInfoFile
├── jobName (string)
├── blockItem (string)          // Block 文件路径
├── projectfile (string)        // 项目文件路径
├── sdebug (int)                // 调试级别
├── hasAT (bool)                // 是否有AT数据
│   └── ATFile (string)
├── hasGCP (bool)               // 是否有GCP文件
│   └── GCPFile (string)
├── TaskMetaData                // 任务元数据
│   ├── id, type, msg, name
│   ├── fatherId, match_id, sfmId
│   ├── imgIds (vector<int>)    // 分配的图片ID列表
│   ├── depends (vector<int>)   // 依赖的任务ID
│   ├── functionName            // "RunFeatureDetection" 等
│   ├── keyMaxImgNum, matchMaxImgNum
│   ├── sfm_task_num, match_task_num
│   └── matchIds (vector<int>)
├── hasATParam (bool)
│   └── ATSettingData          // AT参数
│       ├── keyNum, maxthreads_num
│       ├── minOverlap, maxOverlap
│       ├── maxTieptNum, max_projection_error
│       ├── mode (sfm_mode_e)
│       ├── ba1_grid_count, ba2_grid_count
│       ├── use_gcp, use_user_tiepoints
│       ├── use_constraint, use_image_position
│       └── control_point_path, constraint_path, pos_path, at_path, usertiepoints_path
├── hasRecParam (bool)
│   └── ReconstrutionData      // 重建参数
│       ├── Geometric_Level, ColorBalanced
│       ├── Untexture_Fill_Mode, Texture_Fill_Color
│       ├── DiscardEmptyTiles, HoleFillingMode
│       ├── ProductionData (production_format, destination, name, tiles)
│       ├── boundingbox_custom (BBoxData)
│       ├── boundary_custom (ROI多边形)
│       └── ROISrs
```

**Engine 使用**: Task 子进程启动时读取此文件来获取算法函数名和参数。

### 4.3 FeedBackFile — 任务反馈文件

**魔术头**: `"FEED-FILE-3MO"` (13字节)

```cpp
struct FeedBackFile {
    FeedBackData feedBackData;
    // FeedBackData { int status; float percent; int taskRetVal; string msg; }
};
```

**JSON 对应**: `job_feedback_xxx.json` → `JobFeedBack_s`

### 4.4 PIDFile — 任务进程心跳文件

```cpp
struct PIDFile {
    qint64 pid;                    // Task 进程 PID
    std::string taskFile;          // 正在执行的 task 文件路径
    std::string lastActivateTime;  // 最后活跃时间 "yyyyMMddhhmmss"
};
```

**Engine 使用**: `execTaskTimeThread` 每秒更新 PID 文件；`checkTaskInstanceStatus` 通过比较 PID 和时间戳判断 Task 进程是否僵死（超过 3s 无心跳 = 可安全重新调度）。

### 4.5 JobListFile — 任务队列汇总文件

**魔术头**: `"JLIST-FILE-3MO"` (14字节)

```cpp
struct JobListFile {
    std::string jobName;
    JobInfoData jobInfoData;       // projectPath + itemPath
    RunInfoData runInfoData;       // submitHost/User/Time
    FeedBackData feedBackData;     // status/percent/msg
    int taskNum;
    std::vector<TaskItemData> taskVec;  // 每个 task 的详细信息
};
```

### 4.6 TaskTimeFile — 耗时统计文件 (timesum_*.bin)

**魔术头**: `"TTIME-FILE-3MO"` (14字节)

```cpp
struct TaskTimeFile {
    RunInfoData runInfoData;
    int taskNum;
    std::vector<TaskInfoData> taskVec;
    // TaskInfoData { id, type, status, functionName, startTime, endTime }
};
```

### 4.7 EngineInfoData — 引擎注册/心跳文件

```cpp
struct EngineInfoData {
    int status;                    // 引擎状态
    std::string version;
    std::string hostName;
    std::string userName;
    std::string IPAddr;
    std::string projectName;
    std::string startTime;
    std::string engineFile;        // Engines/<hostname>.json
};
```

**写入**: `Engines/<hostname>.json`（JSON）或 `Engines/<hostname>.bin`（BIN），由 `execEngineTimeThread` 每秒刷新。

---

## 五、配置与选项结构

### 5.1 ATOptions — 空三参数

**来源**: `Core/ATOptions.h`（通过 TaskDef.h 引入）

控制整个 AT pipeline 的参数：
- **特征提取**: `feature_num`, `maxthreads_num`, `max_feature_count_1/2`
- **匹配策略**: `min_overlap`, `max_overlap`, `pair_selection_mode`
- **SFM 设置**: `sfm_mode`, `grid_count_1/2`, `reconstruct_mode`
- **BA 策略**: `bapolicies` — 控制 tiepoints/GCP/constraints/POS/PPA/RDIS 的使用
- **保存选项**: `output_tiepoint`, `output_rawxml`, `max_projection_error`, `max_tiepoint_num`
- **特殊功能**: `use_klt_tracking`, `use_image_position_`, `mesh_recon_mode`

### 5.2 EngineInfo_s — 引擎信息

**文件**: [TaskProcess.h:405](Include/Util/TaskProcess.h#L405)

```cpp
struct EngineInfo_s {
    int Status;              // -1=未初始化
    std::string Version;     // 软件版本
    std::string HostName;    // 主机名
    std::string UserName;    // 用户名
    std::string IPAddr;      // IP 地址
    std::string ProjectName; // 当前处理的项目
    std::string StartTime;   // 引擎启动时间
    int ProcessId;           // 引擎进程 PID
    std::string TaskFile;    // 当前 task 文件
    int TotalMem, FreeMem;   // 内存信息
};
```

### 5.3 appconfig_s / jconfigopt_s — 应用全局配置

**文件**: `Core/Application.h`

- `appconfig_s`: ATOptions + version + focal_length + debug_level
- `jconfigopt_s`: 完整的 XML 配置文件解析（DL, UG, GI, GR, UT, KN, MN, RL, UD, LI, PNT, TG, BN, BO 等元素）

---

## 六、Engine 局部数据结构

### 6.1 全局状态变量

**文件**: [CallEngine.cpp:79-112](App/Engine/CallEngine.cpp#L79)

| 变量 | 类型 | 用途 |
|------|------|------|
| `gotNewPendingJobFile` | `bool` | 是否有待处理的新任务 |
| `NewFileForRun` | `QString` | 当前要执行的 task 文件路径 |
| `taskrunning` | `bool` | 是否有任务正在运行 |
| `fpTaskLock` | `FILE*` | 当前持有的 task 文件锁句柄 |
| `taskPid` | `qint64` | 子进程 Task 的 PID |
| `taskPidFile` | `QString` | PID 心跳文件路径 |
| `projectfilefullpath` | `std::string` | 当前项目文件完整路径 |
| `engineinfofile` | `std::string` | 引擎注册文件路径 |
| `bQuitingApplication` | `bool` | 引擎是否正在退出 |
| `bWorkingOnEngineFile` | `bool` | 是否正在写引擎文件 |
| `maptaskfunction` | `map<int,string>` | taskId → 算法函数名映射 |
| `cancelledJobFile` | `QString` | 被取消的 Job 路径 |
| `failedJobFile` | `QString` | 失败的 Job 路径 |
| `toBeCleanedJobMap` | `QMap<QString,QString>` | 待清理的 Job 映射 |
| `hasFinishedJobMap` | `QMap<QString,int>` | 已完成的 Job 记录 |

### 6.2 局部结构体

```cpp
// TaskTimeInfo — 从 PID 文件解析的任务活跃信息
struct TaskTimeInfo {
    QString sLastActiveTime;   // 最后活跃时间
    qint64 iTaskPid;           // Task 进程 PID
    QString sTaskFile;         // Task 文件路径
};

// JobFileTimeInfo — Job 文件的时间信息
struct JobFileTimeInfo {
    QString filename;
    QDateTime lastModified;
};
```

### 6.3 状态目录路径（全局 QString）

```cpp
QString pendingJobPath;     // %LOCALAPPDATA%/MoldAI/jobs/Pending/Normal/
QString runningJobPath;     // %LOCALAPPDATA%/MoldAI/jobs/Running/
QString completedJobPath;   // %LOCALAPPDATA%/MoldAI/jobs/Completed/
QString failedJobPath;      // %LOCALAPPDATA%/MoldAI/jobs/Failed/
QString cancelledJobPath;   // %LOCALAPPDATA%/MoldAI/jobs/Cancelled/
```

---

## 七、算法任务参数结构

### 7.1 TaskDescriptor — 任务定义

**文件**: [TaskDef.h (Core)](Include/Core/TaskDef.h) (~985行)

```cpp
struct TaskDescriptor {
    int id_;                       // 任务ID
    std::string msg_, name_;      // 显示名/消息
    int type_;                     // ATSTARTTYPE=1, ATRUNNINGTYPE=2, ATCOMPLETETYPE=0 等
    int fatherId_;                 // 父任务ID
    std::set<int> depends_;        // 依赖的任务ID集合
    std::vector<int> imgIds_;      // 分配的图片ID列表
    std::string fun_name_;         // 算法函数名
    int key_maximage_num_;         // 关键点提取最大图片数
    int match_maximage_num_;       // 匹配最大图片数
    int sfm_task_num_;             // SFM 子任务数
    int match_task_num_;           // 匹配子任务数
    // ... 更多字段用于匹配/SFM 子任务拆分
};
```

**序列化**:
- `CreateFromJson` (nlohmann) / `CreateFromJsonV2` (RapidJSON string) / `CreateFromJsonV3` (RapidJSON Value&)
- `WriteToJson` / `WriteToJsonV2` / `WriteToJsonV3`

### 7.2 ATTaskInfo — Task 文件顶层结构

**文件**: [TaskDef.h (Core)](Include/Core/TaskDef.h)

```cpp
struct ATTaskInfo {
    std::string job_;            // Job JSON 字符串
    std::string blockItem_;      // Block 文件路径
    std::string projectFile_;    // 项目文件路径
    std::string ATJson_;         // AT 数据 JSON
    std::string GCPJson_;        // GCP 数据 JSON
    TaskDescriptor task_;        // 任务描述符
};
```

**Engine 使用**: Task 子进程通过 `ATTaskInfo::LoadJson/LoadBin` 解析 task 文件，从中获取 `task_.fun_name_` 决定调用哪个算法。

### 7.3 preparetaskinfo_s — 预处理任务信息

```cpp
struct preparetaskinfo_s {
    std::string ImagePath;       // 图片目录
    std::string Prefix;          // 名称前缀
    std::string SRS;             // 坐标参考系
    std::string GcpPath;         // GCP 文件路径
    std::string PosfilePath;     // POS 文件路径
};
```

---

## 八、数据结构关系总图

```
                         Engine (MoldAINode.exe)
                              │
           ┌──────────────────┼──────────────────┐
           │                  │                  │
    读 Job 文件         读 Task 文件        写 Engine 心跳
           │                  │                  │
           ▼                  ▼                  ▼
    TaskGraph_s          ATTaskInfo        EngineInfo_s
    ┌──────────────┐    ┌─────────────┐    ┌────────────┐
    │ JobInfo_s    │    │TaskDescriptor│    │ HostName   │
    │ RunInfo_s    │    │  id/type     │    │ IPAddr     │
    │ JobFeedBack_s│    │  fun_name_   │    │ Version    │
    │ tasksmap ────│───►│  fatherId_   │    │ TaskFile   │
    │  Task_s[0]   │    │  depends_    │    │ FreeMem    │
    │  Task_s[1]   │    │  imgIds_     │    └────────────┘
    │  ...         │    │ job_         │
    └──────────────┘    │ blockItem_   │
                        │ projectFile_ │
      JSON/BIN 序列化   └─────────────┘
           │                  │
           ▼                  ▼
    ┌──────────────┐    ┌──────────────┐
    │ JobListFile  │    │SPTaskInfoFile│
    │ (DataStruct) │    │ (DataStruct) │
    └──────────────┘    └──────────────┘
           │                  │
           ▼                  ▼
    job_xxx.json/bin   task_def_N.json/bin

    反馈文件:              心跳文件:
    FeedBackFile           PIDFile
    job_feedback_xxx       task_def_N.pid
```

---

## 九、序列化体系总结

所有结构体同时维护 **三套序列化代码**（历史原因）：

| 方式 | API | 使用场景 |
|------|-----|----------|
| nlohmann::json | `WriteToJson()` / `CreateFromJson()` | 旧版 JSON 读写 |
| RapidJSON (string) | `WriteToJsonV2()` / `CreateFromJsonV2()` | 新版 JSON 读写（首选） |
| RapidJSON (Value&) | `WriteToJsonV3()` / `CreateFromJsonV3()` | JSON 对象内嵌 |
| Binary + XOR | `Serialize(ostream&)` / `Deserialize(istream&)` | 磁盘 BIN 文件 |

**宏前缀体系**（Types.h）:
```
JOB_INFO_USE_BIN      → job 文件用 BIN 还是 JSON
TASK_USE_BIN          → task 文件用 BIN 还是 JSON
JOB_FEEDBACK_USE_BIN  → feedback 文件用 BIN 还是 JSON
ENGINE_USE_BIN        → engine 心跳文件用 BIN 还是 JSON
STAT_USE_BIN          → 统计文件用 BIN 还是 JSON
BLK_USE_BIN           → block 文件用 BIN 还是 JSON
```

BIN 格式使用 `"XXXX-FILE-3MO"` 魔术头 + XOR(0xAB) 逐字节加解密，所有字段以长度前缀+内容的方式序列化。

---

## 十、关键观察

1. **JSON/BIN 双轨制是历史债务**：每种结构同时有 nlohmann 和 RapidJSON 两套 JSON 代码，加上 BIN 一套，共三套序列化路径，维护成本极高。

2. **GBK 残留**：大量结构体中存在 `field2_` 备份字段和已注释掉的 `UTF82GBK/GBK2UTF8` 调用，表明系统从 GBK → UTF-8 迁移已完成但未清理。

3. **异或加密形同虚设**：`SOURCE_XOR_KEY = 0xAB` 硬编码在头文件中，无密钥管理，仅提供表面上的"二进制不可读"。

4. **全局状态无保护**：Engine 的 30+ 个全局变量在多个线程间读写但无 mutex 保护，是竞态条件的主要来源。

5. **TaskGraph_s 的依赖模型**：使用 FatherId + Depends 实现了灵活的 DAG 任务调度，支持 AT 流水线从单任务（GenTasks）自动拆分为多阶段并行子任务。
