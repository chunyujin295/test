# Doc 文档目录

## 现有系统分析

| 文档 | 说明 |
|------|------|
| [Engine_Task_Analysis.md](Engine_Task_Analysis.md) | Engine 与 Task 模块的架构分析。MoldAINode.exe（调度守护进程）与 MoldAITask.exe（算法执行进程）的分离设计、进程通信方式和职责划分。 |
| [Engine_DataStructure_Analysis.md](Engine_DataStructure_Analysis.md) | Engine 层用到的所有数据结构。JobInfo_s、RunInfo_s、JobFeedBack_s、Task_s、TaskGraph_s 等核心结构体及其序列化方式。 |
| [Core_Analysis.md](Core_Analysis.md) | Core 层（MoldAIData.dll）完整分析。类型系统、文件 I/O、数学算法、域数据模型（ATData、BlockObject、Camera 等）、任务系统、坐标系统、45+ 头文件与 46 源文件的依赖关系。 |
| [Engine_Core_Integration.md](Engine_Core_Integration.md) | Engine 对 Core 的调用关系。三层引用路径（直接 include、Util 桥接、文件系统 Task 子进程），约 90 处 File.h 调用、120 处 Logging.h 调用、100 处 Types.h 引用的完整统计与生命周期追踪。 |
| [Engine_FileSystem_IPC.md](Engine_FileSystem_IPC.md) | 文件系统 IPC 机制的完整分析。五类文件（Job、Task 定义、Feedback、PID、Engine 心跳）、Windows deny-write 文件锁互斥协议、XOR 0xAB BIN 格式、Job 状态机 = 目录迁移的调度模型。 |
| [详细分析文档.md](详细分析文档.md) | 8 章综合学习文档，覆盖任务调度全流程。 |

## 生成式任务设计

| 文档 | 说明 |
|------|------|
| [可能用到的数据结构.md](可能用到的数据结构.md) | 生成式任务的三方数据结构定义。GenTaskCategory、GenTaskSubType、GenTaskStatus 枚举，AssetRef、GenerationParams、GenTaskRequest、GenTaskResponse 结构体。 |
| [GenTask_Design.md](GenTask_Design.md) | 生成式模型任务系统的总体设计方案。可行性评估、风险分析（重复提交、状态不一致、网络分区）、HTTP 通信协议约定、generateThread 伪代码、SDK 接口定义、分阶段实施计划。 |
| [GenTask_Integration.md](GenTask_Integration.md) | 生成式任务如何整合到现有代码体系。Block 层 type 字段分叉、BlockObject 内部分化、重建式与生成式目录结构对比、JobInfo_s/JobFeedBack_s 扩展、GenJobInfo 数据结构、完整数据流、文件改动清单。 |
| [GenTask_Checklist.md](GenTask_Checklist.md) | 分阶段执行清单。6 个 Phase 按依赖关系排列，每项可勾选，含文件改动汇总表。 |

## 阅读顺序建议

```
现有系统理解                     生成式任务设计
─────────────                  ─────────────
Core_Analysis.md              可能用到的数据结构.md
    ↓                              ↓
Engine_Core_Integration.md     GenTask_Design.md
    ↓                              ↓
Engine_DataStructure_Analysis.md  GenTask_Integration.md
    ↓
Engine_FileSystem_IPC.md
    ↓
Engine_Task_Analysis.md
    ↓
详细分析文档.md (综合)
```
