# MoldAI 文件系统架构文档

## 1. 架构概览

MoldAI 采用**双进程架构**，通过文件系统进行进程间通信（IPC）和状态管理：

```
┌──────────────────────────────────────────────────────────────────────┐
│                     MoldAIEngine.exe (调度引擎)                        │
│  - 搜索待处理作业 (Pending Jobs)                                       │
│  - 管理作业生命周期 (Pending → Running → Completed/Failed/Cancelled)    │
│  - 启动 MoldAITask.exe 子进程                                         │
│  - 通过 Feedback 文件监控任务进度                                       │
│  - 处理异常/崩溃恢复                                                    │
└──────────────────────────────┬───────────────────────────────────────┘
                               │ 启动子进程 (传入 Task 文件路径)
                               ▼
┌──────────────────────────────────────────────────────────────────────┐
│                     MoldAITask.exe (算法执行器)                        │
│  - 加载 Task 定义文件，解析算法函数名                                     │
│  - 执行具体算法 (FeatureDetection, MatchPairs, SfM, Reconstruction...) │
│  - 通过 cbProgress() 回调将进度写入 Feedback 文件                       │
│  - 支持取消命令 (Cancel Command)                                       │
└──────────────────────────────────────────────────────────────────────┘
```

### 1.1 作业生命周期状态机

```
                    ┌──────────┐
                    │ Pending  │  ← 新作业提交到此目录
                    │ (待处理)  │
                    └────┬─────┘
                         │ Engine 获取作业，获取 .lock 锁
                         ▼
                    ┌──────────┐
                    │ Running  │  ← 作业正在执行
                    │ (运行中)  │
                    └────┬─────┘
                         │ 根据 Feedback 文件中的 Status 字段
              ┌──────────┼──────────┐
              ▼          ▼          ▼
        ┌─────────┐ ┌────────┐ ┌─────────┐
        │Completed│ │ Failed │ │Cancelled│
        │ (完成)   │ │ (失败)  │ │ (已取消) │
        └─────────┘ └────────┘ └─────────┘
```

每种状态对应 `Pending/`、`Running/`、`Completed/`、`Failed/`、`Cancelled/` 子目录，Engine 通过**移动文件**来改变作业状态。作业目录还支持优先级分层：`High/`、`Normal/`、`Low/`。

---

## 2. 文件类型目录

### 2.1 概览表

| 序号 | 文件类型 | 前缀/后缀 | 格式 | 用途 |
|------|----------|-----------|------|------|
| 1 | Job 文件 | `J_*.bin` / `J_*.json` | BIN/JSON | 作业完整信息（任务图、运行信息、反馈） |
| 2 | Task 文件 | `TI_*.bin` / `TI_*.json` | BIN/JSON | 单个任务的定义（算法名、参数、依赖） |
| 3 | Feedback 文件 | `JF_*.bin` / `JF_*.json` | BIN/JSON | 任务执行进度和状态反馈 |
| 4 | TimeSum 文件 | `JT_*.bin` | BIN | 各任务阶段耗时统计 |
| 5 | Engine 信息文件 | `engine_info.bin` | BIN | Engine 注册信息（主机、版本、内存） |
| 6 | Lock 文件 | `*.lock` | 空文件 | 文件级互斥锁（FopenDenyWriteLockUtf8） |
| 7 | PID 文件 | `pid_*.bin` | BIN | Engine 进程 ID 记录 |
| 8 | Block 文件 | `.bbin` / `.blk` | BIN/JSON | 持久化数据容器（影像、相机、连接点等） |
| 9 | AT Data 文件 | `BI_*.bin` | BIN | AT 数据（不含连接点） |
| 10 | Tiepoints 文件 | `CP.bin` | BIN | 连接点（匹配点云） |
| 11 | SC Block 文件 | `SCB.bin` | BIN | 空三子区块数据 |

### 2.2 Job 文件 (`J_*.bin` / `J_*.json`)

**数据结构**: `JobFullInfo_s` (定义于 `Include/Util/TaskProcess.h`)
- `JobName`: 作业名称
- `TaskGraph_s tg`: 任务图
  - `JobInfo`: 作业元信息
  - `RunInfo`: 运行信息（提交主机、用户、时间）
  - `JobFeedBack`: 作业级反馈
  - `tasksmap: std::map<int, Task_s>`: 任务集合

**作用**: 作业的"主文件"，包含完整的任务图定义和所有子任务信息。

**命名**: `J_<JobName>.bin`（J_ 前缀内置于作业名），如 `J_20250527143022_SC.bin`。文件在 Pending/Running/Completed 等队列目录间移动。

---

### 2.3 Task 文件 (`TI_*.bin` / `TI_*.json`)

**数据结构**: `ATTaskInfo` / `SPTaskInfoFile` (定义于 `Include/Core/TaskDef.h`, `Include/Core/DataStruct.h`)

BIN 格式魔数: `"TASKDEF-FILE-3MO"`

```
SPTaskInfoFile:
├── jobName          (作业名称)
├── blockItem        (关联的 Block 标识)
├── projectfile      (项目文件路径)
├── TaskMetaData:
│   ├── id, type     (任务 ID、类型)
│   ├── fun_name     (算法函数名: "RunFeatureDetection", "RunSfM" 等)
│   ├── msg, name    (显示名称)
│   ├── depends[]    (依赖任务 ID 列表)
│   ├── imgIds[]     (关联影像 ID)
│   ├── sfmId, matchIds[]
│   ├── sfm_task_num, match_task_num
│   └── keyMaxImgNum, matchMaxImgNum
├── ATSettingData    (空三参数)
├── ReconstrutionData (重建参数)
└── ROIData          (ROI 参数)
```

**任务拆分机制**: Task_0 (`TI_0.bin`) 包含完整参数，Engine/Task 将其拆分为多个子任务 (`TI_1.bin`, `TI_2.bin`, ...)。

**作用**: 算法执行的最小单元定义，作为命令行参数传递给 MoldAITask.exe。

---

### 2.4 Feedback 文件 (`JF_*.bin` / `JF_*.json`)

**数据结构**: `JobFeedBack_s` / `FeedBackFile` (定义于 `Include/Util/TaskProcess.h`, `Include/Core/DataStruct.h`)

BIN 格式魔数: `"FEED-FILE-3MO"`

```
FeedBackFile / JobFeedBack_s:
├── Status     (int)  — 作业状态码
├── Percent    (int)  — 进度百分比 (0-100)
├── Msg        (string) — 状态消息
└── TaskRetVal (int)  — 任务返回值
```

**作用**: Task 进程向 Engine 进程报告进度的核心 IPC 机制。

- Task 通过回调函数 `cbProgress()` 周期性写入
- Engine 通过 `load_with_retry()` 读取（带重试，防止读写冲突）
- 状态码决定作业下一状态：
  - `STATUS_COMPLETE (2)` → 移动到 `Completed/`
  - `STATUS_FAILURE (4)` → 移动到 `Failed/`
  - `STATUS_CANCLE (3)` → 移动到 `Cancelled/`

---

### 2.5 TimeSum 文件 (`JT_*.bin`)

**数据结构**: `ATTimeSummary_s` / `TaskTimeFile` (定义于 `Include/Core/DataStruct.h`, `Include/Util/TaskProcess.h`)

BIN 格式魔数: `"TTIME-FILE-3MO"`

```
TaskTimeFile / ATTimeSummary_s:
├── RunInfoData            (运行信息)
├── taskNum                (TaskTime_s 条目数)
└── TaskInfoData[]:
    ├── id, type
    ├── fun_name
    ├── startTime, endTime
    └── elapsed (耗时)
```

**作用**: 记录每个作业/任务各阶段的执行耗时，用于性能分析和统计。Engine 在作业完成后调用 `ExportTimeSum()` 写入。

---

### 2.6 Engine 信息文件 (`engine_info.bin`)

**数据结构**: `EngineInfo_s` → `EngineFile` (定义于 `Include/Core/DataStruct.h`)

```
EngineInfo_s:
├── Version    (引擎版本)
├── HostName   (主机名)
├── UserName   (用户名)
├── IPAddr     (IP 地址)
├── Status     (状态)
├── TotalMem   (总内存)
├── FreeMem    (可用内存)
├── StartTime  (启动时间)
├── EndTime    (结束时间)
├── TaskFile   (当前处理的任务文件)
└── ProcessId  (进程 ID)
```

**作用**: Engine 启动时注册自身信息，用于多机调度场景下的节点管理。

---

### 2.7 Lock 文件 (`*.lock`)

**实现**: `FopenDenyWriteLockUtf8()` — 利用 Windows 文件系统的排他写锁实现互斥。

**作用**:
- 确保同一时间只有一个 Engine 实例处理同一个作业
- Engine 启动时调用 `doCleanupJobLockOnceWhileEngineStart()` 清理上次崩溃遗留的锁文件
- 在 `GetPendingJob()` 中获取作业锁，处理完毕后释放

---

### 2.8 PID 文件 (`pid_*.bin`)

**作用**: 记录 Engine 进程的 PID，用于检测 Engine 是否仍在运行（崩溃恢复）。

---

## 3. Block 文件系统

Block 是 MoldAI 中的**持久化数据容器**，与 Job/Task 的关系为：

> **Block = 持久数据容器** (影像、相机参数、连接点、重建结果)
> **Job/Task = 临时处理单元** (对 Block 数据的操作)

### 3.1 Block 文件类型

| 文件 | 扩展名 | 格式 | 内容 |
|------|--------|------|------|
| Block 主文件 | `.bbin` / `.blk` | BIN/JSON | 完整 Block 元数据（相机、影像组、POS、SRS、重建配置） |
| AT Data 文件 | `BI_*.bin` | BIN | 空三数据（不含连接点），供快速加载 |
| Tiepoints 文件 | `CP.bin` | BIN | 连接点（匹配点云），支持按需延迟加载 |
| SC Block 文件 | `SCB.bin` | BIN | 空三子区块数据 |

### 3.2 Block 主文件数据结构

**数据结构**: `BlockObject::Task_Info` / `BLKBinFile` (定义于 `Include/Core/BlockObject.h`, `Include/Core/DataStruct.h`)

BIN 格式魔数: `"BBLK-FILE-3MO"`

```
BLKBinFile:
├── blkString           (Block 序列化字符串)
├── blkId               (Block ID)
├── job                 (关联作业)
├── isFinished          (是否完成)
├── BlockXML            (Block XML 描述)
├── Tiepoints           (连接点，可延迟加载)
├── ATJson / GCPJson    (空三/GCP JSON 参数)
├── tiepointNum         (连接点数量)
├── atSetting           (空三设置)
├── reconstructionNum   (重建数量)
├── reconstructionDataVec[] (重建数据列表)
├── jobNum              (关联作业数量)
└── jobVec[]            (关联作业列表)
```

### 3.3 延迟加载机制

`BlockImportOptions` 控制加载行为：

```cpp
struct BlockImportOptions {
    bool load_tiepoint_;   // 是否加载连接点 (CP.bin)
    bool load_images_;     // 是否加载影像
};
```

**加载流程** (`BlockObject::Load()`):
1. 根据扩展名分发：`.bbin` → `ReadBlockInfoBin()`, `.blk` → `ReadBlockInfoJson()`
2. 检查 `BlockImportOptions.load_tiepoint_`：
   - 为 `true`：从 Block 主文件或 `CP.bin` 加载连接点
   - 为 `false`：跳过连接点，仅加载 AT Data (`BI_*.bin`)
3. 加载关联的 Feedback 文件恢复状态

**保存流程** (`BlockObject::Save()`):
1. 备份旧文件 (`.old` 后缀)
2. 调用 `ExportBlockATData()` 分别导出 AT 数据不含连接点 + 连接点
3. 写入临时文件，成功后重命名（保证原子性）

---

## 4. JSON vs BIN 格式切换

### 4.1 编译时常量

格式选择由 `Include/Core/Types.h` 中的编译时常量控制：

```cpp
const bool JOB_FEEDBACK_USE_BIN  = true;   // Feedback 文件使用 BIN 格式
const bool JOB_INFO_USE_BIN      = true;   // Job 文件使用 BIN 格式
const bool TASK_USE_BIN          = true;   // Task 文件使用 BIN 格式
const bool BLK_USE_BIN           = true;   // Block 文件使用 BIN 格式
const bool ENGINE_USE_BIN        = true;   // Engine 信息文件使用 BIN 格式
```

所有常量默认为 `true` — 生产环境统一使用加密 BIN 格式。

### 4.2 格式分发模式

各数据结构的 `Load()`/`Save()` 方法均采用相同的分发模式：

```cpp
bool Load(const string& file) {
    if (JOB_INFO_USE_BIN)  // (或对应的常量)
        return LoadBin(file);
    else
        return LoadJson(file);
}
```

### 4.3 格式对比

| 特性 | JSON | BIN |
|------|------|-----|
| 人可读性 | ✓ 可读 | ✗ 二进制 |
| 加密保护 | ✗ 明文 | ✓ XOR 加密 |
| 调试便利 | ✓ 方便 | ✗ 需解密 |
| 文件大小 | 较大 | 较小 |
| 使用场景 | 开发调试 | 生产环境 |

---

## 5. BIN 文件加密机制

### 5.1 加密算法

**类**: `ByteCrypt` (定义于 `Include/Core/DataStruct.h`)

- 算法: **XOR 单字节加密**
- 密钥: `0xAB`
- 按字节加解密（对称加密，加密即解密）

```cpp
struct ByteCrypt {
    static const unsigned char XOR_KEY = 0xAB;
    
    static unsigned char ReadByteDecrypted(FILE* f) {
        unsigned char b = fgetc(f);
        return b ^ XOR_KEY;  // XOR 0xAB
    }
    
    static void WriteByteEncrypted(FILE* f, unsigned char b) {
        fputc(b ^ XOR_KEY, f);  // XOR 0xAB
    }
};
```

### 5.2 魔数头部

每种 BIN 文件以固定魔数开头（未加密），用于格式验证：

| 文件类型 | 魔数字符串 |
|----------|-----------|
| Feedback | `FEED-FILE-3MO` |
| Task Def | `TASKDEF-FILE-3MO` |
| Job List | `JLIST-FILE-3MO` |
| Block | `BBLK-FILE-3MO` |
| TimeSum | `TTIME-FILE-3MO` |

---

## 6. 核心数据结构速查

### 6.1 `TaskGraph_s` (Include/Util/TaskProcess.h)

```cpp
struct TaskGraph_s {
    JobInfo        jobInfo_;     // 作业元信息
    RunInfo_s      runinfo_;     // 运行信息
    JobFeedBack_s  jobfeedback_; // 作业级反馈
    std::map<int, Task_s> tasksmap_; // 任务图 (taskId → Task_s)
    
    // 核心方法
    bool SplitTask(int taskId);  // 将 Task_0 拆分为子任务
};
```

### 6.2 `Task_s` (Include/Util/TaskProcess.h)

```cpp
struct Task_s {
    string Msg, Name;
    int Percent, Status, Type;
    string ProjectPath, ItemPath;
    int Id, FatherId;
    vector<int> Depends;
    RunInfo_s runinfo_;
};
```

### 6.3 `JobFeedBack_s` (Include/Util/TaskProcess.h)

```cpp
struct JobFeedBack_s {
    int Status;       // jobsta_e 枚举值
    int Percent;      // 0-100
    string Msg;
    int TaskRetVal;
    
    bool load_with_retry(const string& file);  // 带重试读取（IPC 安全）
    bool save_with_retry(const string& file);  // 带重试写入
};
```

### 6.4 `ATTaskInfo` / `SPTaskInfoFile` (Include/Core/TaskDef.h / DataStruct.h)

```cpp
struct ATTaskInfo {
    string job_;
    string blockItem_;
    string projectFile_;
    string ATJson_, GCPJson_;
    TaskDescriptor task_;
};

struct TaskDescriptor {
    int id_, type_;
    string fun_name_;  // "RunFeatureDetection", "RunSfM", etc.
    string msg_, name_;
    vector<int> imgIds_, depends_;
    int sfmId_;
    vector<int> matchIds_;
    int sfm_task_num_, match_task_num_;
    int keyMaxImgNum_, matchMaxImgNum_;
};
```

### 6.5 `ATTimeSummary_s` (Include/Util/TaskProcess.h)

```cpp
struct ATTimeSummary_s {
    struct TaskTime_s {
        int id, type;
        string fun_name;
        long long startTime, endTime;
    };
    RunInfo_s runinfo_;
    int taskNum_;
    vector<TaskTime_s> tasksmap_;
};
```

---

## 7. 关键枚举

### 7.1 作业状态 (`jobsta_e`)

```cpp
enum jobsta_e {
    STATUS_PENDDING  = 0,  // 待处理
    STATUS_RUNNING   = 1,  // 运行中
    STATUS_COMPLETE  = 2,  // 已完成
    STATUS_CANCLE    = 3,  // 已取消
    STATUS_FAILURE   = 4,  // 失败
    STATUS_NEW       = 5,  // 新建
    STATUS_UNKNOWN   = 6   // 未知
};
```

### 7.2 作业类型 (`jobtype_e`)

```cpp
enum jobtype_e {
    JOB_AT,       // 空三作业 (名称含 "_SC")
    JOB_TILE,     // 瓦片作业 (名称含 "_TILE")
    JOB_BATCH,    // 批处理作业 (名称含 "BATCH")
    JOB_UNKNOWN
};
```

### 7.3 AT 处理步骤 (`StepAT`)

```cpp
enum StepAT {
    GenTasks,          // 生成任务
    FeatureDetection,  // 特征检测
    PairSelection,     // 像对选择
    MatchPairs,        // 影像匹配
    SfM,               // 运动恢复结构
    OptimizeAT,        // 空三优化
    Reconstruction,    // 三维重建
    BatchPrepare       // 批处理准备
};
```

### 7.4 任务类型常量

```cpp
const int ATSTARTTYPE      = 1;  // AT 启动
const int ATRUNNINGTYPE    = 2;  // AT 运行中
const int ATCOMPLETETYPE   = 0;  // AT 完成
const int ATLASTTASKTYPE   = 3;  // AT 最后任务
const int BATCHSTARTTYPE   = 4;  // 批处理启动
```

---

## 8. 文件名宏定义速查

所有定义在 `Include/Core/Types.h`：

```cpp
// BIN 格式
MAKE_FEEDBAK_BIN_FILE(prefix, jobName)    → "JF_<jobName>.bin"
MAKE_TIMESUM_BIN_FILE(prefix, jobName)    → "JT_<jobName>.bin"
MAKE_TASK_BIN_FILE(prefix, index)         → "TI_<index>.bin"
// JSON 格式
MAKE_FEEDBAK_JSON_FILE(prefix, jobName)   → "JF_<jobName>.json"
MAKE_TIMESUM_JSON_FILE(prefix, jobName)   → "JT_<jobName>.json"
MAKE_TASK_JSON_FILE(prefix, index)        → "TI_<index>.json"

// 前缀/后缀常量
FEEDBACK_BIN_PREFIX = "JF_"
TIME_BIN_PREFIX     = "JT_"
JOB_BIN_PREFIX      = "JI_"        // 已定义但未使用，Job 文件实际无前缀
TASK_DEF_BIN_PREFIX = "TI_"
BINFILE_POSTFIX     = ".bin"
JSONFILE_POSTFIX    = ".json"
LOCKFILE_POSTFIX    = ".lock"

// Block 相关
BLOCKBINFILE = ".bbin"
BLOCKFILE    = ".blk"
TIEPOINTS    = "CP.bin"
SCBLOCKBIN   = "SCB.bin"
ORIDATABIN   = "OD.bin"
```

---

## 9. 目录结构

Feedback、TimeSum、Task、Block 文件均位于**项目目录**下，只有 Job 文件（以及临时锁文件）在**引擎队列目录**中。两套目录是分离的。

```
引擎队列目录 (EngineJobQueue):
├── Pending/
│   ├── High/              ← 高优先级待处理
│   ├── Normal/            ← 普通优先级
│   └── Low/               ← 低优先级
├── Running/               ← 当前运行中
│   └── J_<JobName>.bin      ← Job 文件（JobFullInfo_s，唯一存于此的文件）
├── Completed/
│   └── J_<JobName>.bin
├── Failed/
│   └── J_<JobName>.bin
├── Cancelled/
│   └── J_<JobName>.bin
└── engine_info.bin        ← Engine 注册信息

项目目录 (Project/Block):
<项目目录>/
└── <Block名>/
    ├── <Block名>.bbin              ← Block 主文件
    ├── JF_<JobName>.bin            ← Feedback 文件（Task 写入，Engine 读取）
    ├── JT_<JobName>.bin            ← TimeSum 耗时统计
    ├── BI_*.bin                    ← AT Data（不含连接点）
    ├── CP.bin                      ← Tiepoints 连接点
    ├── SCB.bin                     ← SC Block 子区块数据
    └── <JobName>/                  ← 以作业名为名的子目录
        ├── TI_0.bin                ← Task_0 (总任务定义)
        └── TI_1.bin, TI_2.bin...   ← 拆分后的子任务定义
```

---

## 10. 完整处理流程

```
1. 外部系统提交作业
   └→ 在 Pending/High|Normal|Low/ 下直接放置 J_<JobName>.bin
      (JobFullInfo_s 序列化，J_ 前缀在作业创建时写入名称)

2. Engine 主循环 (searchPendingJobThread2)
   ├→ 扫描 Running/ 目录，处理异常作业
   ├→ 扫描 Pending/ 目录（优先级 High > Normal > Low）
   ├→ GetPendingJob() 获取候选作业
   │   ├→ 尝试获取 J_<JobName>.bin.lock 文件锁
   │   ├→ 失败 → 跳过（其他 Engine 已处理）
   │   └→ 成功 → 将 J_<JobName>.bin 从 Pending/ 移动到 Running/
   ├→ GetRunningTaskInRunningJob()
   │   ├→ 从项目目录 <Project>/<Block>/JF_<JobName>.bin 读取 Feedback
   │   │   (load_with_retry，Feedback 不在队列目录)
   │   ├→ STATUS_COMPLETE/FAILURE/CANCLE → 移动 Job 文件到对应目录
   │   └→ 否则 → 运行中，获取下一个待执行 Task
   ├→ ExecTaskFileV2()
   │   ├→ 从 TaskGraph 获取/拆分 Task
   │   ├→ 在项目目录生成 <Project>/<Block>/<JobName>/TI_<id>.bin
   │   └→ 启动 MoldAITask.exe "<TI_文件路径>"
   └→ ExecTaskPostHandle()
       └→ 处理 Task 完成后的逻辑

3. MoldAITask.exe 执行 (doTaskInProcess)
   ├→ 验证 Task 文件名格式
   ├→ execTaskFile2Only()
   │   ├→ 加载 TI_*.bin → ATTaskInfo（路径来自启动参数）
   │   ├→ 从 EngineJobQueue/Running/J_<JobName>.bin 加载 JobFullInfo_s
   │   ├→ 解析 fun_name (RunFeatureDetection / RunSfM / ...)
   │   ├→ 调用对应 Run* 函数
   │   └→ cbProgress() 周期性写入 <Project>/<Block>/JF_<JobName>.bin Feedback
   └→ 进程退出

4. Engine 检测完成
   ├→ 从项目目录读取 <Project>/<Block>/JF_<JobName>.bin 的 Feedback.Status
   ├→ STATUS_COMPLETE → ExportTimeSum() → 移动 Job 文件到 Completed/
   ├→ STATUS_FAILURE  → 移动 Job 文件到 Failed/
   └→ STATUS_CANCLE   → 移动 Job 文件到 Cancelled/
```

---

## 11. 异常恢复机制

Engine 实现了多层异常恢复：

1. **启动时清理** (`doCleanupJobLockOnceWhileEngineStart`)
   - 清理上次崩溃遗留的 `.lock` 文件和 `pid_*.bin` 文件
   - 调用 `DoCleanupLockFiles()` 扫描所有锁文件并移除

2. **运行中异常处理** (`ProcessUnnormaldRunningJobV2`)
   - 扫描 `Running/` 目录
   - 检查 PID 文件：进程不存在 → 标记为失败
   - 检查 Feedback 长时间未更新 → 超时处理

3. **原子性文件写入**
   - 写入临时文件 → 重命名为正式文件
   - Block 保存前备份旧文件 (`.old` 后缀)
   - 防止写入过程中崩溃导致文件损坏
