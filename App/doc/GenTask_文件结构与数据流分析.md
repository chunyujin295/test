# 生成式任务 — 文件结构与数据流分析

## 目录结构对照

```
重建式:
  Project/
    Block_N/
      JF_<AT_job>.bin                              ← AT job feedback (Block 根)
      JT_<AT_job>.bin                              ← AT job 时间统计
      <jobname>/                                   ← AT job 工作目录
        TI_0.bin, TI_1.bin                         ← DAG 子任务定义
      Reconstruction_M/                            ← 重建
        <jobname>/                                 ← 重建 job 工作目录
        JF_<rec_job>.bin                           ← 重建 job feedback
        Productions/                               ← PRODUCTION_DIR
          Production_P/                            ← PRODUCTION_PREFIX + id
            Tile_000/
              TI_0.bin                             ← Tile 子任务定义
              JF_<base>.bin                        ← Tile feedback
            JF_<base>.bin                          ← 生产总 job feedback
            result/                                ← 产出数据
      .blk / .bbin                                 ← Block 元数据

生成式:
  Project/
    Block_N/
      Generations/                                 ← 对标 Productions (GENERATION_DIR)
        Generation_1/                              ← 对标 Production_1 (GENERATION_PREFIX + id)
          JF_J_Block1_<timestamp>.bin              ← 生成任务 feedback
          result.glb                               ← 下载的结果
          preview.png                              ← 下载的预览图
        Generation_2/
          JF_J_Block1_<timestamp>.bin
      .blk / .bbin

jobs_gen/                                          ← 引擎调度队列 (对标 jobs/)
  Pending/J_Block1_<timestamp>.bin
  Running/...
  Completed/...
  Failed/...
  Cancelled/...
```

### 为什么 JF 不放在 Block 根下

重建式的 AT、重建、生产三级的 JF 是分层存放的。生成式一个任务对标一个 Production，所以 JF 应该放在对标 `Productions/Production_P/` 的 `Generations/Generation_<id>/` 下，而非 Block 根。Block 根下的 JF 对标的是 AT job，生成式没有 AT 阶段，不应在 Block 根下放 JF。

### 为什么用户输出不放在 Project 根下

重建式把输出放 `Project/Productions/` 是因为一个 Production 可能跨多个 Block（多 Block Tile 合并），输出范围是 Project 级别。生成式不跨 Block，一个任务只属于一个 Block，放 `Block/Generations/` 下即可。

---

## 命名规则对照

| 项目 | 生成式 | 重建式对标 |
|------|--------|-----------|
| job_name | `J_<BlockName>_<yyyyMMddhhmmss>` | 无固定规则 |
| job 文件 (队列) | `<job_name>.bin` → `J_Block1_20240604120000.bin` | 同 |
| JF 文件 | `JF_<job_name>.bin`, 放在 `result_dir/` 下 | `JF_<jobname>.bin` |
| task_uuid | `<user>_<timestamp>_<uuid4>` | 无 (生成式特有) |
| 结果目录 | `Generations/Generation_<id>/` | `Productions/Production_<id>/` |
| directory id | `generation_id`, `GetNextGenerationId()` | `production_t id_`, `GenerateValidProductionId()` |
| 目录常量 | `GENERATION_DIR "Generations"` / `GENERATION_PREFIX "Generation_"` | `PRODUCTION_DIR "Productions"` / `PRODUCTION_PREFIX "Production_"` |

### JF 文件如何区分

多个生成任务提交到同一 Block 时，JF 文件名含时间戳 `job_name`，且各自存放在独立的 `Generation_<id>/` 子目录中，互不冲突。前端用 `gen.result_dir + "/JF_" + gen.job_name` 定位。

### job_name 命名规则

`J_<BlockName>_<yyyyMMddhhmmss>`，例如 `J_Block1_20240604120000`。BlockName 标识归属，时间戳保证唯一性。

---

## 参数落盘对照

| 层 | 重建式 | 生成式 | 用途 |
|----|--------|--------|------|
| Block 元数据 | `Block.blk` → `production_infos_[].options_` | `Block.blk` → `generations_info_[]` (sub_type, generation_id, result_dir, ...) | 前端展示、恢复 UI |
| Job 文件 | `jobs/Pending/<name>.bin` → `JobFullInfo_s` | `jobs_gen/Pending/J_*.bin` → `GenJobFullInfo_s` (含 GenTaskParams) | 引擎调度 |
| Task 文件 | `Block/.../TI_*.bin` → `Task_s` (每个子任务) | 无 (生成式没有 DAG 拆分) | — |

---

## 完整数据流

### 提交阶段

```
1. 前端获取 Block → GetTaskInfoMutual()
2. 前端填入 gen_options.gen_params (prompt, polygon_limit, ...)
3. 前端: genId = task.GetNextGenerationId()  // 分配递增 id
4. 前端: SubmitGenTask(task, user, pendingPath, genId)
   → 创建 Generations/Generation_<genId>/
   → 设置 job.result_dir = Project/Block/Generations/Generation_<genId>/
   → 写入 J_*.bin 到 jobs_gen/Pending/
   → 写入 JF_*.bin 到 result_dir/
   → 返回 result.generation_id
5. 前端: generations_info_.push_back({generation_id, task_uuid, job_name, result_dir, ...})
6. 前端: WriteBlockInfoToBin → Block.blk
```

### 引擎处理阶段

```
7. GenTaskThread::ProcessPendingJobs()
   → 扫描 jobs_gen/Pending/J_*.bin
   → load_with_retry(jobFilePath) → GenJobFullInfo_s (含 result_dir)
   → BuildFeedbackPath(info) → info.job.result_dir + "/JF_" + info.job_name
   → load feedback
   → HTTP SubmitTask → server_task_id
   → PointManager::CreatePointTask → freeze_no
   → 更新 feedback + 注册 Block → 移到 Running/

8. GenTaskThread::ProcessRunningJobs()
   → 扫描 jobs_gen/Running/J_*.bin
   → load job + feedback (同上, result_dir 拼接路径)
   → HTTP QueryTaskStatus
   → IN_PROGRESS: ApplyResponse + UpdateFeedback (progress)
   → COMPLETED:
     → PointManager::SettlePoints (先结算, 失败不动状态)
     → ApplyResponse + UpdateFeedback + 更新 Block + 移到 Completed/
```

### 前端轮询阶段

```
9. 前端定时读取 Block.blk → generations_info_[]
10. 遍历每个 gen:
    → PENDING/IN_PROGRESS:
      fbPath = gen.result_dir + "/JF_" + gen.job_name + ".bin"
      读 JobFeedBack_s → Percent → UI 进度条
    → COMPLETED:
      gen.result_url → DownloadResult(gen.result_url, gen.result_dir + "/result.glb")
      gen.preview_url → 预览图
11. 更新 UI 树节点 (ITGeneration)
```

### 崩溃恢复

| 崩溃时机 | 状态 | 恢复行为 |
|----------|------|---------|
| SubmitTask 成功, FreezePoints 前 | Pending/, server_task_id 有, freeze_no 空 | 下轮跳过 submit 直接重试 FreezePoints |
| FreezePoints 成功, 移 Running 前 | Pending/, server_task_id 有, freeze_no 有 | 崩溃恢复路径 → 移回 Running |
| SettlePoints 成功, points_settled 落盘后 | Running/, points_settled=true | 下轮跳过 settle 直接完成 |
| SettlePoints 成功, points_settled 落盘前 | Running/, points_settled=false | 下轮重试 settle (服务端须幂等) |
| 状态更新中 | Completed/ 或 Running/ | Completed/ 则无事; Running/ 下轮继续 |

---

## 关键字段链路: result_dir

`result_dir` 是整个流程中连接各阶段的枢纽：

```
SubmitGenTask 创建
  → job.result_dir = Project/Block/Generations/Generation_<id>/
  → J_*.bin (GenJobTaskData.Serialize/Deserialize)
  → Block.blk (generations_info_[i].result_dir)
        │
        ├── Engine: BuildFeedbackPath → result_dir/JF_xxx.bin
        ├── Engine: ProcessPendingJobs 注册 Block 时写 generations_info_
        └── 前端: 轮询 feedback → gen.result_dir + "/JF_" + gen.job_name
             前端: 下载结果 → gen.result_dir + "/result.glb"
```

三个存储位置都承载同一个值：
1. J_*.bin inside GenJobInfo_s (引擎操作)
2. Block.blk inside generations_info_ (前端读取)
3. JF_*.bin path (独立 feedback, 引擎写 / 前端读)

---

## 与 JT 文件的关系澄清

| 文件 | 前缀 | 含义 | 生成式是否需要 |
|------|------|------|:---:|
| JF | `JF_` | Job Feedback — 进度反馈 | 是 |
| JT | `JT_` | Job Time — 时间统计 (time_ / JT_ prefix) | 不需要 (无子任务耗时统计) |
| TI | `TI_` | Task Item — 子任务定义 | 不需要 (无 DAG 拆分) |

`JT_` 不是 "JobTask"，是 "Job Time" (时间统计)。`JI_` 是 "Job Info" (job queue entry)。

---

## 相关文档

| 文档 | 内容 |
|------|------|
| [GenTask_Checklist.md](GenTask_Checklist.md) | 分 Phase 执行清单 |
| [GenTask_Design.md](GenTask_Design.md) | 总体设计方案 |
| [GenTask_Integration.md](GenTask_Integration.md) | 整合方案 |
| [GenTask_Frontend_API.md](GenTask_Frontend_API.md) | 前端 API |
| [积分接口集成方案.md](积分接口集成方案.md) | 积分系统设计 |
| [GenTask_总体设计梳理.md](GenTask_总体设计梳理.md) | 设计梳理 |
