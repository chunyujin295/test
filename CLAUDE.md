# CLAUDE.md — MoldAI 项目配置

## 核心规则

### 代码修改策略：文档先行，不直接改源码

**所有对代码的改动，禁止直接修改源代码文件。** 任何修改方案只能写到 `App/doc/` 目录下对应的文档中。

具体规则：

1. **需要修改现有代码时** — 找到 `App/doc/` 下对应的文档，在文档中记录修改方案（改哪个文件、哪一行、怎么改）。不要直接编辑 `.cpp`/`.h`/`.cmake` 等源文件。
2. **新增功能/模块时** — 在 `App/doc/` 下新建或更新设计文档，描述新增内容。源码由用户自行根据文档实施。
3. **修复 Bug 时** — 在 `App/doc/` 相关文档中记录问题原因和修复方案。
4. **唯一例外** — `App/doc/` 目录下的 `.md` 文件本身可以被直接修改，这是记录变更的主要载体。

### 文档目录结构

```
App/doc/
├── README.md                  — 文档索引
├── GenTask_Checklist.md       — 分阶段执行清单（主要变更目标）
├── GenTask_Design.md          — 总体设计
├── GenTask_Integration.md     — 集成方案
├── GenTask_Frontend_API.md    — 前端 API
├── 可能用到的数据结构.md        — 三方数据结构
├── Engine_Task_Analysis.md    — 现有系统分析
├── Engine_DataStructure_Analysis.md
├── Engine_Core_Integration.md
├── Engine_FileSystem_IPC.md
├── Core_Analysis.md
├── FileSystemArchitecture.md
├── 详细分析文档.md
└── 服务端接口安全.md
```

### 其他

- 项目是 C++17 + CMake + Qt6 的摄影测量/三维重建桌面软件
- 生成式任务 (GenTask) 是新功能，核心代码在 `App/Engine/GenTaskThread.*`、`App/Engine/GenHttpClient.*`、`Include/Core/GenTaskOptions.h`、`Include/Core/GenTaskAPI.*`
- GenTaskProcess.h 缺失待补齐
