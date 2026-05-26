# Engine 文件系统 IPC 机制详细分析

## 一、总体架构

MoldAI 引擎系统采用 **纯文件系统 IPC** 实现 Node（调度守护进程）与 Task（算法执行进程）之间的通信。这种设计不依赖 Socket、共享内存或消息队列，所有信息交换均通过磁盘文件完成。

### 两个进程的角色

```
MoldAINode.exe (常驻)              MoldAITask.exe (按需创建/销毁)
├── 扫描任务队列                    ├── 读取 task_def_N 文件
├── 调度决策（优先级排序）           ├── 执行具体算法（特征检测/SfM/重建等）
├── 生成 Task 子进程                ├── 写入 feedback 进度文件
├── 监控 Task 运行状态              └── 退出，Node 接管后续流程
├── 写 PID 心跳文件
└── 写 Engine 心跳文件
```

### 架构示意图

```
┌──────────────────────────────────────────────────────────────────────┐
│                          App/GUI (MoldAI.exe)                        │
│                               │ 写入 Job 文件                         │
│                               ▼                                      │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │              Engine Job Queue (工作目录)                      │    │
│  │  Pending/  │  Running/  │  Completed/  │  Failed/  │  Cancelled/ │
│  │   job.bin   │  job.bin   │   job.bin    │  job.bin  │   job.bin   │
│  │   + .lock   │  + .lock   │              │           │            │
│  └─────────────────────────────────────────────────────────────┘    │
│         ▲ 扫描                    │ 移动/状态变更                    │
│         │                         ▼                                  │
│  ┌──────────────┐    spawn    ┌──────────────┐                      │
│  │ MoldAINode   │ ──────────→ │ MoldAITask   │                      │
│  │   .exe       │             │   .exe        │                      │
│  │              │ ←── read ── │               │                      │
│  │  写 PID 文件  │   feedback │  写 feedback  │                      │
│  └──────┬───────┘             └──────────────┘                      │
│         │ 写 Engine 心跳                                              │
│         ▼                                                            │
│  Engines/<hostname>.bin  (引擎注册/发现)                              │
└──────────────────────────────────────────────────────────────────────┘
│                                                                      │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │             Project Directory (项目目录)                      │    │
│  │  Block_N/                                                    │    │
│  │  ├── job_20230316_095227_AT/    ← 任务目录                   │    │
│  │  │   ├── task_def_0.bin          ← 主任务定义                 │    │
│  │  │   ├── task_def_1.bin          ← 子任务定义（拆分后）       │    │
│  │  │   ├── TI_1.bin + .lock       ← BIN 格式 + 锁              │    │
│  │  │   └── ...                                                 │    │
│  │  ├── JF_job_20230316_095227_AT.bin  ← Feedback 文件          │    │
│  │  └── JF_job_20230316_095227_AT.bin.lock ← Feedback 锁        │    │
│  └─────────────────────────────────────────────────────────────┘    │
└──────────────────────────────────────────────────────────────────────┘
```

---

## 二、目录结构与文件类型总览

### 2.1 Engine Job Queue 工作目录 (C:\Users\<user>\AppData\Local\MoldAI\jobs\)

源码: [CallEngine.cpp:4338-4342](App/Engine/CallEngine.cpp#L4338-L4342)

```cpp
pendingJobPath   = jobQueue + "/Pending/";
runningJobPath   = jobQueue + "/Running/";
completedJobPath = jobQueue + "/Completed/";
failedJobPath    = jobQueue + "/Failed/";
cancelledJobPath = jobQueue + "/Cancelled/";
```

这些路径由 [Types.h:91-95](Include/Core/Types.h#L91-L95) 的宏定义:

```cpp
#define JOBPENDINGSTR   "Pending"
#define JOBRUNNINGSTR   "Running"
#define JOBCOMPLETEDSTR "Completed"
#define JOBCANCELLEDSTR "Cancelled"
#define JOBFAILEDSTR    "Failed"
```

### 2.2 文件类型一览表

| 文件类型 | 所在目录 | 文件名格式 | 数据结构 | 写入者 | 读取者 | 用途 |
|---------|---------|-----------|---------|-------|-------|------|
| **Job 文件** | Pending/Running/Completed/Failed/Cancelled | `<jobname>.bin/.json` | `JobFullInfo_s` | App, Node | Node, App | 任务调度状态,跨目录移动=状态变更 |
| **Task 定义文件** | 项目目录/Block_N/job_xxx/ | `task_def_N.bin/.json` 或 `TI_N.bin/.json` | `ATTaskInfo` / `SPTaskInfoFile` | App (创建), Task (读取) | Node, Task | 描述执行什么算法、参数等 |
| **Feedback 文件** | 项目目录/Block_N/ | `feedback_<jobname>.json` 或 `JF_<jobname>.bin` | `JobFeedBack_s` / `FeedBackFile` | Task (进度), Node (状态更新) | Node, App | 进度百分比、完成/失败/取消状态 |
| **PID 文件** | 项目目录/Block_N/job_xxx/ | `<taskfile>.pid` | `PIDFile` | Node (execTaskTimeThread) | Node (存活检测) | Task 进程心跳 |
| **Engine 信息文件** | jobQueue/Engines/ | `<hostname>.bin/.json` | `EngineInfo_s` / `EngineFile` | Node (execEngineTimeThread) | App | Engine 注册/发现/状态监控 |
| **Lock 文件** | 与被保护文件同目录 | `<filename>.lock` | 空文件 (Windows deny-write sharing) | 所有进程 | 所有进程 | 互斥锁 |
| **TimeSum 文件** | 项目目录/Block_N/ | `timesum_<jobname>.json` 或 `TS_<jobname>.bin` | 时间统计信息 | Node | App | 任务耗时统计 |

---

## 三、文件命名与格式系统

### 3.1 宏定义体系

源码: [Types.h:37-181](Include/Core/Types.h#L37-L181)

```cpp
// 文件格式开关 (编译时宏)
const bool JOB_INFO_USE_BIN      = true;   // Job 文件使用 BIN 格式
const bool TASK_USE_BIN          = true;   // Task 文件使用 BIN 格式
const bool JOB_FEEDBACK_USE_BIN  = true;   // Feedback 文件使用 BIN 格式
const bool ENGINE_USE_BIN        = true;   // Engine 信息文件使用 BIN 格式

// 文件后缀
#define JSONFILE_POSTFIX   ".json"
#define BINFILE_POSTFIX    ".bin"
#define LOCKFILE_POSTFIX   ".lock"

// 文件名前缀
#define FEEDBACK_PREFIX     "feedback_"
#define FEEDBACK_BIN_PREFIX "JF_"
#define TASK_DEF_PREFIX     "task_def_"
#define TASK_DEF_BIN_PREFIX "TI_"

// 文件路径构造宏
#define MAKE_FEEDBAK_BIN_FILE(p,j)   (path(p) + "JF_" + j + ".bin")
#define MAKE_FEEDBAK_JSON_FILE(p,j)  (path(p) + "JF_" + j + ".json")
#define MAKE_TASK_BIN_FILE(p,j)      (path(p) + "TI_" + j + ".bin")
#define MAKE_TASK_JSON_FILE(p,j)     (path(p) + "TI_" + j + ".json")
```

### 3.2 双格式切换机制

整个系统在编译时通过 `USE_BIN` 宏开关选择 JSON 或 BIN 格式。运行时每处读写都要做条件分支:

```cpp
// 典型模式 (无处不在)
std::string feedback_file = "";
if (JOB_FEEDBACK_USE_BIN) {
    feedback_file = MAKE_FEEDBAK_BIN_FILE(parentDir, jobName);
} else {
    feedback_file = MAKE_FEEDBAK_JSON_FILE(parentDir, jobName);
}
```

### 3.3 BIN 格式: XOR 加密二进制

所有 BIN 文件使用固定密钥 **XOR 0xAB** 对内容进行简单混淆。每个文件类型有唯一的 magic header:

| 文件类型 | Magic Header | 数据结构 |
|---------|-------------|---------|
| Task 定义 | `TASKDEF-FILE-3MO` (15 bytes) | `SPTaskInfoFile` |
| Feedback | `FEED-FILE-3MO` (13 bytes) | `FeedBackFile` |
| Job 列表 | `JLIST-FILE-3MO` (14 bytes) | `JobListFile` |
| 时间统计 | `TTIME-FILE-3MO` (14 bytes) | `TaskTimeFile` |
| PID | 无 header (小文件) | `PIDFile` |

源码: [DataStruct.h:1695-1757](Include/Core/DataStruct.h#L1695-L1757)

```cpp
struct FeedBackData {
    int status;
    float percent;
    int taskRetVal;
    std::string msg;
    ByteCrypt byteCrypt;  // XOR 0xAB 加解密器

    bool Serialize(std::ofstream& out) const {
        byteCrypt.WriteByteDecrypted(out, (char*)&status, sizeof(status));
        byteCrypt.WriteByteDecrypted(out, (char*)&percent, sizeof(percent));
        byteCrypt.WriteByteDecrypted(out, (char*)&taskRetVal, sizeof(taskRetVal));
        // ... msg with length prefix
    }
};

struct FeedBackFile {
    FeedBackData feedBackData;
    ByteCrypt byteCrypt;

    bool Serialize(std::ofstream& out) const {
        const char SOURCE_HEADER_LABEL[] = "FEED-FILE-3MO";
        byteCrypt.WriteByteDecrypted(out, header, 13);  // header 也 XOR
        feedBackData.Serialize(out);
    }

    bool Deserialize(std::ifstream& in) {
        char header[13];
        byteCrypt.ReadByteDecrypted(in, header, sizeof(header));
        // 验证 header == "FEED-FILE-3MO"
        if (decrypted_header != std::string(SOURCE_HEADER_LABEL, 13))
            return false;
        feedBackData.Deserialize(in);
    }
};
```

### 3.4 JSON 格式

JSON 格式同时维护 **三套序列化代码**:
1. `nlohmann::json` (WriteToJson / CreateFromJson)
2. `RapidJSON string` (WriteToJsonV2 / CreateFromJsonV2)
3. `RapidJSON Value&` (WriteToJsonV3 / CreateFromJsonV3)

这是历史遗留问题——不同时期引入了不同的 JSON 库,而旧代码未被清理。

---

## 四、Lock 文件互斥协议

### 4.1 机制原理

Lock 不靠文件内容,而是利用 **Windows 文件共享模式**: 打开文件时指定拒绝写入共享 (`_SH_DENYWR`), 其他进程尝试以写入模式打开同一文件会失败。

源码: [File.h](Include/Core/File.h)

```cpp
static FILE* FopenDenyWriteLockUtf8(const std::string& path, const char* mode) {
    // Windows: 使用 _fsopen + _SH_DENYWR
    // 若文件已被其他进程以 deny-write 模式打开, 则返回 NULL
}
```

### 4.2 使用模式

```cpp
// 标准三段式
FILE* fp = AICORE::File::FopenDenyWriteLockUtf8(filePath + ".lock");
if (fp != NULL) {
    // === 临界区 ===
    // 读取或写入被保护的文件
    data.save(filePath);
    // === 临界区结束 ===
    fclose(fp);  // 释放锁
} else {
    // 获取锁失败, 其他进程正在操作此文件
    continue;  // 等待下次轮询
}
```

### 4.3 锁的粒度与作用域

| 锁文件 | 保护对象 | 持有者 | 持有时间 |
|-------|---------|-------|---------|
| `job.lock` (Pending/) | Job 文件不被并发调度 | Node | 从读取 pending 到移动到 Running/ |
| `job.lock` (Running/) | Job 状态不被并发修改 | Node | 临时,操作完即释放 |
| `task_def_N.lock` | Task 不被并发执行 | Node 全局 `fpTaskLock` | 从 task 开始到 task 完成 |
| `feedback.lock` | Feedback 文件不被并发读写 | 读写者 | save_with_retry / load_with_retry 期间 |
| `engineinfo.lock` | Engine 信息文件 | Node | execEngineTimeThread 写期间 |

### 4.4 孤儿锁清理

引擎启动时和运行中会清理"锁文件存在但被保护文件已不存在"的孤儿锁:

```cpp
// DoCleanupJobLockOnceWhileEngineStart() — 启动时
// 遍历 Pending/High/ 和 Running/
// 如果 .lock 对应的 .bin/.json 不存在 → 删除 .lock

// DoCleanupLockFiles() — 运行中周期清理线程
// 持续扫描 toBeCleanedJobMap 中的 job,超时后清除 .lock
```

---

## 五、Job 文件: 任务调度的核心载体

### 5.1 数据结构

源码: [TaskProcess.h:4719-4918](Include/Util/TaskProcess.h#L4719-L4918)

```cpp
struct JobFullInfo_s {
    std::string JobName;      // job 名称 (如 "job_20230316095227_AT")
    TaskGraph_s tg;           // 任务图
};

struct TaskGraph_s {
    JobInfo_s job;            // ProjectPath + ItemPath
    RunInfo_s runinfo;        // SubmitHostName, SubmitUser, SubmitTime, EndTime, Run_s
    JobFeedBack_s feedback;   // Status, Percent, Msg, TaskRetVal
    std::map<int, Task_s> tasksmap;  // 子任务映射表 (按 task ID)
};
```

### 5.2 Job 的状态机 = 目录迁移

**Job 没有状态字段, 状态由所在目录隐式表达:**

```
                    App 创建
                       │
                       ▼
                 ┌──────────┐
                 │ Pending/ │  ← 等待调度
                 └────┬─────┘
                      │ GetPendingJob() 抢到锁, 文件移动
                      ▼
                 ┌──────────┐
                 │ Running/ │  ← 正在执行
                 └────┬─────┘
          ┌───────────┼───────────┐
          ▼           ▼           ▼
    ┌──────────┐ ┌──────────┐ ┌──────────┐
    │Completed/│ │ Failed/  │ │Cancelled/│
    └──────────┘ └──────────┘ └──────────┘
```

对应的 `jobsta_e` 枚举 (用于 feedback, 不用于目录):

```cpp
enum jobsta_e {
    STATUS_PENDDING,  // Pending 目录
    STATUS_RUNNING,   // Running 目录
    STATUS_COMPLETE,  // Completed 目录
    STATUS_CANCLE,    // Cancelled 目录
    STATUS_FAILURE    // Failed 目录
};
```

### 5.3 优先级子目录

Pending 下还有按优先级分层的子目录:

```cpp
enum jobpriority_e {
    PRIORITY_URGENT,  // "Urgent"
    PRIORITY_HIGH,    // "High"
    PRIORITY_NORMAL,  // "Normal"
    PRIORITY_LOW,     // "Low"
    PRIORITY_PAUSE,   // "Pause"
};
```

调度器通过 `SortPendingJobFile()` / `SortJobFile()` 按名字中的时间戳排序,优先级高的子目录优先扫描。

---

## 六、Task 文件: 任务执行的算法定义

### 6.1 数据结构

```cpp
struct ATTaskInfo {
    std::string job_;           // job 名称
    std::string blockItem_;     // Block 名称
    std::string projectFile_;   // 项目文件路径
    std::string ATJson_;        // 空三 JSON 参数
    std::string GCPJson_;       // 控制点 JSON 参数
    TaskDescriptor task_;       // 任务描述符
    int tasktype_;              // 任务类型

    bool load(const std::string& file); // 加载 task_def_N 文件
};

struct TaskDescriptor {
    int id_;                    // 任务 ID
    int type_;                  // 类型: 4=主任务(拆分), 0=完成, 1=特征, 2=匹配, 3=SfM, ...
    int fatherId_;              // 父任务 ID (DAG)
    std::string fun_name_;      // 要执行的函数名 (如 "RunFeatureDetection")
    std::set<int> depends_;     // 依赖的前置 task ID
    std::vector<int> imgIds_;   // 处理的图像 ID 列表
};
```

### 6.2 Task 的 DAG 结构

Task 通过 `FatherId` + `Depends` 构建有向无环图:

```
task_def_0 (type=4, master)  ← 拆分任务生成子 task
    │
    ├──→ task_def_1 (type=1, FeatureDetection)  depends={}
    │        │
    │        ▼
    ├──→ task_def_2 (type=2, PairSelection)     depends={1}
    │        │
    │        ▼
    ├──→ task_def_3 (type=2/3, MatchPairs)      depends={2}
    │        │
    │        ▼
    ├──→ task_def_4 (type=3, SfM)               depends={3}
    │        │
    │        ▼
    └──→ task_def_5 (type=0, Complete)           depends={4}
```

### 6.3 Node 如何使用 Task 文件

Node 只在调度层面使用 Task 文件的**元数据**,不执行算法:

```cpp
// CallEngine.cpp:2505 — 读 function name 决定做什么
ATTaskInfo atparam;
atparam.load(fileName);
std::string function = atparam.task_.fun_name_;
int type = atparam.task_.type_;
int taskid = atparam.task_.id_;

// 建立 function 映射表 (用于进度描述)
maptaskfunction[task.task_.id_] = task.task_.fun_name_;
```

---

## 七、Feedback 文件: 进度与状态的实时反馈

### 7.1 数据结构

```cpp
struct JobFeedBack_s {
    jobsta_e Status;    // PENDDING → RUNNING → COMPLETE / FAILURE / CANCLE
    float Percent;      // 0.0 ~ 100.0
    std::string Msg;    // 描述信息
    int TaskRetVal;     // Task 进程返回值
};
```

### 7.2 完整生命周期

```
1. [Node] GetPendingJob() — 初次创建
   Status: PENDDING → RUNNING, Percent: 0.0
   写锁 - 获取 feedback_file.lock → 写入 → 释放锁

2. [Task 子进程] 算法执行中 — 持续更新
   Status: RUNNING, Percent: 递增 (5 → 15 → 30 → 60 → 90)
   写锁 - 获取 .lock → 写入 → 释放锁
   每步完成后将进度写入 feedback 文件

3. [Node] 监控循环 — 读取
   feadback.load_with_retry(feedback_file)
   .lock 锁保护读取过程不被 Task 的写入打断

4. [Node] Task 完成处理 — 最终状态
   Status: COMPLETE / FAILURE / CANCLE
   写锁 - 写入最终 feedback → 移动 job 文件到 Completed/Failed/Cancelled
```

### 7.3 带重试的原子读写

源码: [TaskProcess.h:1921-2072](Include/Util/TaskProcess.h#L1921-L2072)

```cpp
bool save_with_retry(const std::string& file, bool bInsideUI = true) {
    int retryTimes = 0;
    do {
        // 1. 先获取锁
        FILE* fpLock = AICORE::File::FopenDenyWriteLockUtf8(file + ".lock");
        if (fpLock == NULL) {
            retryTimes++;
            // UI 侧 200ms, 非 UI 侧 2000ms
            sleep(bInsideUI ? 200ms : 2000ms);
            continue;
        }
        // 2. 在锁保护下写入
        if (JOB_FEEDBACK_USE_BIN) {
            WriteToBin(file);     // binary XOR 0xAB
        } else {
            ofs << outjson.dump(4);  // JSON pretty-print
        }
        // 3. 释放锁
        fclose(fpLock);
        break;
    } while (retryTimes < 3);
    return retryTimes < 3;
}
```

重试最多 3 次, 每次间隔 200ms (UI 线程) 或 2000ms (非 UI 线程)。

### 7.4 LoadFeedbackFile 备用解析路径

源码: [CallEngine.cpp:657-748](App/Engine/CallEngine.cpp#L657-L748)

当 `JobFeedBack_s::load()` 的 nlohmann::json 解析失败时, 系统还有一条备用的 Qt QJsonDocument 解析路径:

```cpp
bool LoadFeedbackFile(std::string& feedback_file, JobFeedBack_s& job_feedback,
                       bool retry_more_times) {
    // 路径1: BIN → FeedBackFile::Deserialize
    // 路径2: JSON → QJsonDocument::fromJson
    //          从 QJsonObject 中读 "Status", "Percent", "Msg"
    // 最多重试 3 次, 间隔 2000ms
}
```

这提供了 **4 条独立的 feedback 读取路径**: nlohmann BIN, nlohmann JSON, Qt JSON, RapidJSON。

---

## 八、PID 文件: Task 进程心跳与存活检测

### 8.1 数据结构

```cpp
struct PIDFile {
    qint64 pid;                       // MoldAITask.exe 的进程 ID
    std::string taskFile;             // 正在执行的 task 文件路径
    std::string lastActivateTime;     // 最后活跃时间 (yyyyMMddhhmmss)
};
```

### 8.2 写入: execTaskTimeThread

源码: [CallEngine.cpp:4555-4663](App/Engine/CallEngine.cpp#L4555-L4663)

Node 的 `execTaskTimeThread` 线程每秒写入一次 PID 文件:

```cpp
void execTaskTimeThread() {
    while (true) {
        if (!gotNewPendingJobFile || NewFileForRun.isEmpty())
            continue;

        // 1. 先读现有 PID 文件 (如果存在)
        // 2. 判断: 同一个 pid → 如果 >3s 没更新则覆盖写入
        //         不同 pid → 如果 >5s 没更新则覆盖写入
        //         文件不存在 → 直接创建写入
        // 3. PIDFile::Serialize(out) → 写入二进制
        sleep(1000);
    }
}
```

### 8.3 读取: checkTaskInstanceStatus

源码: [CallEngine.cpp:4410-4483](App/Engine/CallEngine.cpp#L4410-L4483)

```cpp
bool checkTaskInstanceStatus(QString& sTmpNewFileForRun) {
    // 1. 检查 .pid 文件是否存在
    // 2. 读取 PIDFile → 获取 pid + lastActivateTime
    // 3. 同一 pid 且 >3s 未更新 → 认为 task 可以安全占用
    // 4. 不同 pid 且 >10s 未更新 → 可以抢占
    // 5. 仍在活跃 → 返回 false, 不抢占
}
```

这是一个简单但有效的 **进程存活检测协议**: 如果 Task 子进程崩溃而没有清理 PID 文件, 10 秒后 Node 可以安全地接管同一个任务。

---

## 九、Engine 信息文件: 引擎注册与发现

### 9.1 数据结构

```cpp
struct EngineInfo_s {
    int Status;           // 0=Ready, 1=Busy
    std::string Version, HostName, UserName, IPAddr;
    std::string ProjectName, StartTime, EndTime, TaskFile;
    int ProcessId, TotalMem, FreeMem;
};
```

### 9.2 写入: execEngineTimeThread

源码: [CallEngine.java:4347-4401](App/Engine/CallEngine.java#L4347-L4401)

每 1 秒刷新:

```cpp
void execEngineTimeThread() {
    while (true) {
        EngineInfo_s runinfo;
        runinfo.HostName = QHostInfo::localHostName();
        runinfo.UserName = getenv("USERNAME");
        runinfo.Status = gotNewPendingJobFile ? 1 : 0;  // Busy : Ready
        runinfo.TaskFile = NewFileForRun;                 // 当前执行的 task
        // ...
        runinfo.savebin(engineinfofile);  // C:\...\jobs\Engines\WIN-XXX.bin
        sleep(1000);
    }
}
```

### 9.3 读取: App 端

App 通过读取 `Engines/` 目录下的文件来发现可用的 Engine 节点, 检查其 `Status` 决定是否分配新任务。

### 9.4 退出时的清理

```cpp
void PostQuitProcess() {
    // 从 Engines/ 目录删除本机 Engine 信息文件
    std::filesystem::remove(engineinfofile);
}
```

---

## 十、完整调度生命周期 (端到端)

### 阶段 1: Engine 启动

```
main() [CallEngine.cpp:4816]
│
├── MakePath() [L:4328]
│   └── 创建 Pending/, Running/, Completed/, Failed/, Cancelled/
│
├── DoCleanupJobLockOnceWhileEngineStart() [L:175]
│   └── 清理孤儿 .lock 文件
│
├── 删除旧的 Engines/<hostname>.bin
│
└── 启动后台线程:
    ├── execEngineTimeThread     → 写 Engine 心跳
    ├── searchPendingJobThread2  → 主调度循环 ★
    ├── searchUnnormaldRunningJobThread → 异常检测
    ├── execTaskTimeThread       → 写 PID 文件
    └── DoCleanupLockFiles       → 锁清理
```

### 阶段 2: Job 提交 (由外部 App 完成)

```
App/GUI:
  1. 创建 job 目录: Block_N/job_YYYYMMDDHHMMSS_AT/
  2. 创建 task_def_0.bin/json (主任务, type=4)
  3. 创建 JobFullInfo_s 文件
  4. 写入 Pending/<jobname>.bin
```

### 阶段 3: Node 发现并调度 Job

```
searchPendingJobThread2() [L:2213]
│
├── 条件检查: !gotNewPendingJobFile && !taskrunning
│
├── GetRunningTaskInRunningJob() [L:1446]
│   └── 扫描 Running/ 目录, 检查是否有未完成的任务
│       有 → 恢复执行 (断点续传)
│       无 → 返回 AI3D_FAILURE
│
└── GetPendingJob() [L:1214]
    │
    ├── 1. JobMonitor::SortPendingJobFile() → 排序 pending 列表
    │
    ├── 2. foreach job in pending:
    │   │
    │   ├── 2a. 尝试锁 pending job: FopenDenyWriteLockUtf8(job.lock)
    │   │   失败 → 下一个 job
    │   │
    │   ├── 2b. 读取 JobFullInfo_s → blockpath, block, jobname
    │   │
    │   ├── 2c. 构造 feedback 路径:
    │   │   MAKE_FEEDBAK_BIN_FILE(blockpath/block/, jobname)
    │   │
    │   ├── 2d. 构造 task 路径:
    │   │   MAKE_TASK_BIN_FILE(blockpath/block/jobname/, "0")
    │   │
    │   ├── 2e. 检查 task_def_0 文件是否存在
    │   │   不存在 → 释放锁, continue
    │   │
    │   ├── 2f. 创建 Running/ 下的 job 文件
    │   │   jobpending_new.save(runningJobPath + jobFileName)
    │   │
    │   ├── 2g. 删除 Pending/ 下的原文件
    │   │   QFile(jobFilePath).remove()
    │   │
    │   ├── 2h. 写入初始 feedback (STATUS_RUNNING, Percent=0)
    │   │   feadback.save_with_retry(feedback_file)
    │   │
    │   ├── 2i. 锁住 task_def_0 文件 (全局 fpTaskLock)
    │   │   fpTaskLock = FopenDenyWriteLockUtf8(taskfile + ".lock")
    │   │
    │   ├── 2j. 设置全局状态
    │   │   gotNewPendingJobFile = true
    │   │   NewFileForRun = taskfile
    │   │   projectfilefullpath = blockpath
    │   │
    │   └── 2k. break (只取一个 job)
    │
    └── sleep(500ms)
```

### 阶段 4: Node 执行 Task

```
searchPendingJobThread2() 继续 [L:2287]
│
├── gotNewPendingJobFile && !NewFileForRun.isEmpty()
│
├── taskPid = qApp->applicationPid()
├── taskPidFile = NewFileForRun + ".pid"
│
└── ExecTaskFileV2() [L:2416]
    │
    ├── 1. 加载 task 文件: atparam.load(fileName)
    │   获取: function, type, taskid, blockpath, jobname
    │
    ├── 2. 读 feedback: feadback.load_with_retry(feedback_file)
    │   状态必须是 STATUS_RUNNING
    │
    ├── 3. 根据 task type 分支:
    │   │
    │   ├── type == ATLASTTASKTYPE (4): 主任务拆分
    │   │   加载所有子 task_def_N 文件
    │   │   建立 maptaskfunction 映射
    │   │   标记 job 完成 → 移到 Completed/
    │   │   更新 feedback (COMPLETE)
    │   │
    │   ├── type == ATCOMPLETETYPE (0): 所有任务完成
    │   │   更新 feedback → 移到 Completed/
    │   │
    │   └── 其他 type: 实际算法任务
    │       │
    │       ├── 3a. spawn MoldAITask.exe
    │       │   path = appDir + "/MoldAITask.exe"
    │       │   args = [taskfile]
    │       │   process.start(path, args)
    │       │
    │       ├── 3b. 监控循环: while process is running
    │       │   │
    │       │   ├── process.waitForFinished(timeout)
    │       │   ├── 读取 feedback: feadback.load_with_retry(feedback_file)
    │       │   ├── 检查异常: STATUS_CANCLE, STATUS_FAILURE
    │       │   └── 检查超时: Task 运行时间过长 → kill
    │       │
    │       └── 3c. 处理结果:
    │           │
    │           ├── CANCEL:  move Running/ → Cancelled/
    │           ├── FAILURE: move Running/ → Failed/
    │           └── COMPLETE: move Running/ → Completed/
    │
    └── 4. init() 清理全局状态
```

### 阶段 5: Task 子进程内部

```
MoldAITask.exe (由 Node spawn)
│
├── 读取 argv[1] = taskfile 路径
├── ATTaskInfo::load(taskfile) → 获取 fun_name, 参数
│
├── 执行对应算法函数:
│   RunFeatureDetection() / RunPairSelection() / RunMatchPairs()
│   / RunSfM() / RunOptimizeAT() / RunReconstruction()
│
├── 每个阶段完成后写 feedback:
│   JobFeedBack_s feedback;
│   feedback.Status = STATUS_RUNNING;
│   feedback.Percent = currentProgress;  // 递增
│   feedback.Msg = "Keypoint tasks completed";
│   feedback.save_with_retry(feedback_file);
│
└── 最终:
    feedback.Status = STATUS_COMPLETE;  // 或 FAILURE
    feedback.Percent = 100.0;
    feedback.save_with_retry(feedback_file);
    进程退出
```

---

## 十一、异常处理机制

### 11.1 孤儿 Job 恢复

`GetRunningTaskInRunningJob()` [CallEngine.cpp:1446](App/Engine/CallEngine.cpp#L1446)

Node 会扫描 Running/ 目录中的 job, 如果发现 task 状态为 RUNNING 但实际上没有进程在执行 (PID 检测), Node 会重新接管。

### 11.2 崩溃 Task 检测

`checkTaskInstanceStatus()` [CallEngine.java:4410](App/Engine/CallEngine.java#L4410)

通过 PID 文件检测:
- 同一 pid, >3s 未更新 → 认为 task 可抢占
- 不同 pid, >10s 未更新 → 认为原 task 已经崩溃

### 11.3 网络路径失效检测

```cpp
bool bNetworkPathAlreadyInvalid = false;
// 当检测到网络路径不可达时设置
// 调度线程检查此标志, 避免对不可达路径的不断重试
```

### 11.4 超时 Kill

Node 通过 `killTaskProcess()` 终止运行时间过长的 Task 子进程, 将 job 标记为 FAILURE。

### 11.5 重试与容错

- Feedback 文件: 读/写各重试 3 次
- Job 文件移动: save → sleep(1000ms) → 验证存在 → 删源文件
- BIN 文件: magic header 验证, 不匹配则返回 false

---

## 十二、数据流完整图

```
┌─────────────────────────────────────────────────────────────────────────┐
│  时间线                                                                    │
│                                                                          │
│  T0: App 创建                                                             │
│      Project/Block_N/job_xxx_AT/                                         │
│      └── task_def_0.bin  ← SPTaskInfoFile (BIN, XOR 0xAB)               │
│      JobQueue/Pending/job_xxx_AT.bin  ← JobListFile (BIN)               │
│                                                                          │
│  T1: Node 扫描 Pending/                                                   │
│      searchPendingJobThread2() → GetPendingJob()                         │
│      ① lock( Pending/job.lock )                                          │
│      ② read JobFullInfo_s                                                │
│      ③ write Running/job.bin                                             │
│      ④ delete Pending/job.bin                                            │
│      ⑤ write Project/Block_N/JF_job_xxx_AT.bin  (STATUS_RUNNING, 0%)    │
│      ⑥ lock( Project/Block_N/job_xxx_AT/TI_0.bin.lock )  ← fpTaskLock   │
│                                                                          │
│  T2: Node 执行 Task                                                       │
│      ExecTaskFileV2()                                                    │
│      ① read TI_0.bin → ATTaskInfo → fun_name = "RunFeatureDetection"     │
│      ② spawn MoldAITask.exe TI_0.bin                                     │
│                                                                          │
│  T3: Task 子进程运行                                                       │
│      while(processing):                                                  │
│          ① lock( JF_job.bin.lock )                                       │
│          ② write JF_job.bin  (Percent 递增)                              │
│          ③ unlock                                                         │
│      write JF_job.bin  (STATUS_COMPLETE, 100%)                           │
│      exit(0)                                                             │
│                                                                          │
│  T4: Node 检测完成                                                         │
│      ExecTaskFileV2() → 监控循环                                         │
│      process.waitForFinished() 返回                                       │
│      ① read JF_job.bin → STATUS_COMPLETE                                 │
│      ② lock( Running/job.lock )                                          │
│      ③ write Completed/job.bin                                           │
│      ④ delete Running/job.bin                                            │
│      ⑤ unlock( TI_0.bin.lock )  ← 释放 fpTaskLock                       │
│      ⑥ init() 清理全局状态                                                │
│                                                                          │
│  T5: App 读取结果                                                          │
│      ① read Completed/job.bin → JobFullInfo_s                            │
│      ② read JF_job.bin → 最终 feedback                                   │
│      ③ 展示结果给用户                                                      │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## 十三、关键设计特点与问题

### 设计特点

1. **无状态 Node**: Node 不持久化任何调度状态, 所有状态存于磁盘文件。Node 重启后可无缝恢复。
2. **文件系统即消息队列**: 目录就是队列 (Pending → Running → Completed), 文件移动就是 dequeue/enqueue。
3. **跨进程互斥仅靠文件锁**: Windows deny-write sharing 是整个系统的唯一互斥原语。
4. **Polling 驱动**: 没有事件通知机制, 所有线程都是轮询 (sleep + check)。
5. **编译时格式切换**: JSON/BIN 格式通过宏在编译时决定, 不是运行时选择。

### 已知问题

1. **三套 JSON 序列化代码**: nlohmann::json, RapidJSON string, RapidJSON Value& — 维护成本高, 容易出现同一字段在不同路径序列化结果不一致。
2. **XOR 0xAB 不是加密**: 简单的单字节异或仅是混淆, 不能提供真正的数据保护。
3. **GBK/UTF8 转换残留**: 大量 `field2_` 备份字段和注释掉的 `UTF82GBK/GBK2UTF8` 调用是历史遗留。
4. **文件写入不是原子的**: save→sleep→verify→delete 模式的窗口期可能导致不一致。
5. **轮询开销**: 多个线程独立轮询, sleep 间隔从 200ms 到 2000ms 不等, 延迟与 CPU 占用需要权衡。
6. **goto 使用**: `load_with_retry` 等函数使用 `goto` 进行重试控制流, 可读性较差。
