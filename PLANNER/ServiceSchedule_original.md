# Service 层总规划 (ServiceSchedule) — 实施计划

> 本文档是业务层（Service 层）的**详细设计计划**，供 Coding Agent 按章逐项实现。
> 设计依据：PLANNER/PLANNER.md（需求规格）、PLANNER/ModelSchedule.md（Model 层最终设计）。

## 1. 定位与架构

### 1.1 定位

Service 层是"没有界面的整个软件功能集合"，位于 Model 层之上、界面层之下。

**核心约定**：

- 只使用 C++ 标准库与 Model 层公开接口，不触碰 Model 私有成员。
- 不使用任何 GUI 类型（`QString` 等），所有数据以 `std::string` / STL 容器传递。
- 不含 `cout` / `cin` 或弹窗。输入以参数传入，结果以返回值或引用参数传出。
- **控制器 `ProjectController` 在整个程序生命周期中只能有 1 个实例（单例模式）。**

### 1.2 架构分解

```
界面层 (UI)
    │
    ▼
┌─────────────────────────────────────────┐
│  ProjectController (Singleton)          │  ← 唯一的对外入口
│    ├── 持有 unique_ptr<Project>          │
│    ├── 导入/导出 (polymorphic Importer)  │
│    ├── 人工编辑 (内联编辑逻辑)            │
│    ├── 统计 (委托 Model 计数 + CPM 结果) │
│    ├── 验证 (委托 ProjectValidator)      │
│    └── 调度 (委托 CPMCalculator)         │
├─────────────────────────────────────────┤
│  ProjectValidator     (stateless)        │  ← 可独立单测
│  CPMCalculator        (stateless)        │  ← 可独立单测
│  IProjectImporter/Exporter (polymorphic) │  ← 开放-封闭
│  PpmImporter / PpmExporter               │
├─────────────────────────────────────────┤
│  ValidationResult     (DTO, 本层定义)     │
│  展示信息 struct      (DTO, 本层定义)     │
└─────────────────────────────────────────┘
    │
    ▼
Model 层 (Project, Task, Dependency, Resource, ScheduleResult, …)
```

**职责分配原则**：

| 类 | 职责 | 为什么单独成类 |
| :--- | :--- | :--- |
| `ProjectController` | 统一的 Service 入口，持有当前 Project，协调所有操作 | 它是单例——界面层只知道这一个对象 |
| `ProjectValidator` | 对 `const Project&` 执行三条合理性检查并返回结果 | 独立的纯算法，可复用、可单独测试 |
| `CPMCalculator` | 对 `const Project&` 执行关键路径计算 | 独立的纯算法，可复用、可单独测试 |
| `IProjectImporter` + 派生类 | 多态导入体系，支持扩展新格式 | 开闭原则——加格式不改旧代码 |
| `IProjectExporter` + 派生类 | 多态导出体系 | 同上 |

> **为什么没有单独的 TaskEditor / DependencyEditor / ResourceEditor 类？**
>
> 编辑操作本质是"调用 Model 层受控修改接口 + 加一条业务规则检查"的薄层。拆分出三个只有 3~5 个方法的类会增加文件数和胶水代码，但收益极低——它们不会被独立复用或替换。
> 将这些方法作为 `ProjectController` 的私有成员函数，用命名前缀区分实体（`Task` / `Dependency` / `Resource`），
> 既保持代码组织清晰，又避免过度设计。若未来某一实体的编辑逻辑膨胀到需要独立测试，
> 再提取为独立类——这是一个**刻意延迟的设计决策**，不是疏忽。

---

## 2. 设计前提与已有资产

### 2.1 Model 层已提供（可直接使用）

| 接口 | 来源 | 用途 |
| :--- | :--- | :--- |
| `Project::AddTask(name, duration)` | `Project.hpp:81` | 添加任务 |
| `Project::AddTask(TaskId, name, duration)` | `Project.hpp:84` | 显式 ID 添加（PPM 导入用） |
| `Project::RemoveTask(TaskId)` | `Project.hpp:88` | 级联删除任务 |
| `Project::AddResource(name, unitCost)` | `Project.hpp:91` | 添加资源 |
| `Project::AddResource(ResourceId, name, unitCost)` | `Project.hpp:95` | 显式 ID 添加（PPM 导入用） |
| `Project::RemoveResource(ResourceId)` | `Project.hpp:99` | 级联删除资源 |
| `Project::AddDependency(pred, succ, type, lag)` | `Project.hpp:102` | 添加依赖（重复忽略） |
| `Project::AssignResource(taskId, resId, qty)` | `Project.hpp:105` | upsert 分配 |
| `Project::FindTask(TaskId)` | `Project.hpp:59` | 按 ID 查找任务 |
| `Project::FindResource(ResourceId)` | `Project.hpp:61` | 按 ID 查找资源 |
| `Project::FindDependency(pred, succ)` | `Project.hpp:63` | 查找依赖 |
| `Project::GetPredecessors(TaskId)` | `Project.hpp:66` | 前驱 ID 列表 |
| `Project::GetSuccessors(TaskId)` | `Project.hpp:68` | 后继 ID 列表 |
| `Project::GetAllocationsForTask(TaskId)` | `Project.hpp:71` | 任务分配记录 |
| `Project::TaskCount()` | `Project.hpp:50` | 任务总数 |
| `Project::DependencyCount()` | `Project.hpp:52` | 依赖总数 |
| `Project::ResourceCount()` | `Project.hpp:54` | 资源总数 |
| `Project::GetName()` / `SetName()` | `Project.hpp:48,78` | 项目名称 |
| `Task::GetId()` / `GetName()` / `GetDuration()` | `Task.hpp` | 任务属性 |
| `Task::SetName()` / `SetDuration()` | `Task.hpp` | 修改任务 |
| `Task::CanAllocateResource()` | `Task.hpp` | 里程碑判定 |
| `DependencyType { FS, SS, FF, SF }` | `DependencyType.hpp` | 依赖类型枚举 |
| `TaskId` / `ResourceId` / `Id<Tag>` | `Id.hpp` | 强类型 ID |

### 2.2 Service 层已有资产

| 文件 | 状态 | 说明 |
| :--- | :--- | :--- |
| `service/IProjectImporter.hpp` | 已实现 | 抽象导入接口，`Import()` 返回 `unique_ptr<Project>` |
| `service/IProjectExporter.hpp` | 已实现 | 抽象导出接口，`Export(const Project&)` 返回 `bool` |
| `service/ManualImporter.hpp/.cpp` | 已实现 | 测试桩，硬编码导入 PPM 样例，验证基准用 |

### 2.3 Model 层缺口（Service 层实现前必须先补齐）

Service 层依赖以下 Model 层接口，当前代码中**尚未实现**。

| 缺口 | 优先级 | 说明 |
| :--- | :--- | :--- |
| **`ScheduleResult` + `TaskScheduleInfo`** | **必须** | ModelSchedule.md §3.7 已完整设计。构造函数：`ScheduleResult(map<TaskId, TaskScheduleInfo>, int totalDuration, vector<TaskId> criticalPath)`。提供 `GetTotalDuration()` / `GetEarlyStart(TaskId)` / `GetEarlyFinish(TaskId)` / `GetLateStart(TaskId)` / `GetLateFinish(TaskId)` / `GetCriticalPath()` 六个 `const` 访问器，全部按值返回。**
| **`Project::GetTasks()` 等遍历接口** | **必须** | Service 层的 List 操作需要遍历全部 Task / Dependency / Resource / Allocation。建议接口：`const std::vector<Task>& GetTasks() const`、`const std::vector<Dependency>& GetDependencies() const`、`const std::vector<Resource>& GetResources() const`、`const std::vector<Allocation>& GetAllocations() const`——四个简单 const 引用返回。**
| **`Project::RemoveDependency(TaskId pred, TaskId succ)`** | **必须** | 需求 3.2.2 要求删除依赖。实现要点：移除后同步更新 `successors_` / `predecessors_` 索引。若不存在该依赖则静默忽略。 |
| `Dependency::GetType()` / `GetLag()` 等访问器 | 检查 | 确认依赖类是否已有 `GetPredecessorId()` / `GetSuccessorId()` / `GetType()` / `GetLag()` 四个访问器 |
| `Allocation::GetTaskId()` / `GetResourceId()` / `GetQuantity()` | 检查 | 确认分配类是否已有访问器 |
| `Resource::GetId()` / `GetName()` / `GetUnitCost()` | 检查 | 确认资源类是否已有访问器 |

---

## 3. 辅助数据结构（DTO——本层定义）

以下纯数据结构用于向界面层传递信息。它们都定义在 `service/` 目录下。

### 3.1 `TaskDisplayInfo` — 任务展示信息

```cpp
// 文件：service/TaskDisplayInfo.hpp
// 职责：封装单个任务的界面展示所需数据，纯数据载体

struct TaskDisplayInfo
{
    int         index;                  // 容器索引（0-based，同 vector 下标）
    int         idValue;                // TaskId 数值（供界面定位用）
    std::string name;                   // 任务名称
    int         duration;               // 工期
    std::vector<int> predecessorIndices; // 前驱任务的容器索引列表
    std::vector<int> successorIndices;   // 后继任务的容器索引列表
};
```

### 3.2 `DependencyDisplayInfo` — 依赖展示信息

```cpp
// 文件：service/DependencyDisplayInfo.hpp
// 职责：封装单条依赖的界面展示所需数据

struct DependencyDisplayInfo
{
    int            index;             // 序号（0-based）
    int            predecessorIndex;  // 前置任务容器索引
    int            successorIndex;    // 后置任务容器索引
    DependencyType type;              // FS / SS / FF / SF
    int            lag;               // 时差
};
```

### 3.3 `ResourceDisplayInfo` — 资源展示信息

```cpp
// 文件：service/ResourceDisplayInfo.hpp
// 职责：封装单个资源的界面展示所需数据

struct ResourceDisplayInfo
{
    int         index;     // 容器索引（0-based）
    int         idValue;   // ResourceId 数值
    std::string name;      // 资源名称
    double      unitCost;  // 单位成本
};
```

### 3.4 `ProjectStatistics` — 统计信息

```cpp
// 文件：service/ProjectStatistics.hpp
// 职责：封装项目宏观统计指标

struct ProjectStatistics
{
    int taskCount;        // Task 总数（含普通任务与里程碑）
    int dependencyCount;  // Dependency 总数
    int resourceCount;    // Resource 总数
    int totalDuration;    // 关键路径长度（天）
};
```

### 3.5 `ValidationResult` — 验证结果

```cpp
// 文件：service/ValidationResult.hpp
// 职责：封装项目合理性验证的完整结果。构造后不可变。

class ValidationResult
{
  public:
    // 默认构造 = 项目有效（空错误列表）
    ValidationResult();

    // 以错误信息列表构造（至少有一条错误时才用此构造函数）
    explicit ValidationResult(const std::vector<std::string>& errors);

    // 逐一追加错误（供 ProjectValidator 内部使用）
    void AddError(const std::string& error);

    bool IsValid() const;
    const std::vector<std::string>& GetErrors() const;

  private:
    std::vector<std::string> m_errors;
};
```

> **设计说明**：不引入 `ValidationErrorCode` 枚举。需求 5.1 只要求"返回具体错误信息"，
> 纯字符串列表已满足需求且扩展新检查项时无需改枚举定义，同时避免了多类联动的胶水代码。
> 若未来界面层需要错误码做差异化展示，可在不改变 `ValidationResult` 的前提下叠加一层映射。

---

## 4. `ProjectValidator` — 合理性验证器

### 4.1 职责与接口

```cpp
// 文件：service/ProjectValidator.hpp
// 职责：对 const Project& 执行需求 5.1 的三条合理性检查，返回 ValidationResult
// 特点：无状态；Validate() 为 const 方法；一个实例可反复使用

class ProjectValidator
{
  public:
    ProjectValidator()                              = default;
    ProjectValidator(const ProjectValidator&)       = default;
    ProjectValidator& operator=(const ProjectValidator&) = default;
    ~ProjectValidator()                             = default;

    // 执行全部三条检查，收集所有问题后一次性返回
    ValidationResult Validate(const Project& project) const;

  private:
    // ① 依赖图无环：Kahn 算法，处理数 < 任务总数 → 成环
    void CheckAcyclic(const Project& project,
                      std::vector<std::string>& errors) const;

    // ② 无悬挂节点：从入度为 0 的节点做正向 BFS/DFS，从出度为 0 的节点做反向
    //    BFS/DFS，两集合交集必须覆盖所有任务
    void CheckNoDangling(const Project& project,
                         std::vector<std::string>& errors) const;

    // ③ 引用完整性：每条 Dependency 的 pred / succ 均能 FindTask
    void CheckReferenceIntegrity(const Project& project,
                                 std::vector<std::string>& errors) const;
};
```

### 4.2 算法要点

**① CheckAcyclic（Kahn 算法）**：

```
1. 遍历所有任务，为每个 TaskId 统计入度（GetPredecessors().size()）
2. 将入度为 0 的任务加入队列
3. 反复取出队首任务，对其每个后继将其入度减 1；若减至 0 则入队
4. 最终若已处理的任务数 < TaskCount() → 存在环，追加错误信息
```

**② CheckNoDangling（双向可达性）**：

```
1. 找到所有入度为 0 的起始任务集合 S 和出度为 0 的终止任务集合 T
2. 若 S 为空且任务数 > 0 → 全部任务不可达（追加错误）
3. 从 S 中所有节点出发做正向 BFS/DFS，收集所有可达任务 R_forward
4. 从 T 中所有节点出发做反向 BFS/DFS（沿 GetPredecessors），收集 R_backward
5. R_forward ∩ R_backward 之外的任务即为悬挂节点，逐个追加错误信息
```

**③ CheckReferenceIntegrity**：

```
1. 遍历每条 Dependency：
   - 若 FindTask(pred) == nullptr → 追加 "前置任务 ID=X 不存在"
   - 若 FindTask(succ) == nullptr → 追加 "后置任务 ID=X 不存在"
```

> **依赖的 Model 接口**：`TaskCount()`、`GetPredecessors()`、`GetSuccessors()`、`FindTask()`、遍历全部依赖（需 §2.3 缺口补齐）。

---

## 5. `CPMCalculator` — 关键路径计算器

### 5.1 职责与接口

```cpp
// 文件：service/CPMCalculator.hpp
// 职责：对 const Project& 执行 CPM 前向/后向传播，产出 ScheduleResult
// 前置条件：project 已通过 ProjectValidator 校验（无环、引用完整、无悬挂节点）
//          违反前置条件时行为未定义（由调用方保证）
// 特点：无状态；Calculate() 为 const 方法；一个实例可反复使用

class CPMCalculator
{
  public:
    CPMCalculator()                            = default;
    CPMCalculator(const CPMCalculator&)        = default;
    CPMCalculator& operator=(const CPMCalculator&) = default;
    ~CPMCalculator()                           = default;

    // 执行 CPM 计算，按值返回 ScheduleResult（依赖移动语义）
    ScheduleResult Calculate(const Project& project) const;

    // 判断某任务是否在关键路径上（EF == LF），ScheduleResult 须已包含该任务
    bool IsCritical(const ScheduleResult& result, TaskId id) const;
};
```

### 5.2 算法完整描述

设 dur(X) 为任务 X 的工期（`GetDuration()`）。

**Step 1：拓扑排序**

```
以 Kahn 算法获取所有任务的拓扑序，同时得到入度信息。
```

**Step 2：前向传播（按拓扑序）**

```
初始化：对于所有无前驱（入度为 0）的任务，ES = 0，EF = dur(X)。

对于每个任务 X（按拓扑序），遍历其每一条后继依赖 (X → succ, type, lag)：

  初始化 candidateES = 0
  根据 type 计算 succ 的候选 ES：
    FS: candidateES = EF(X) + lag
    SS: candidateES = ES(X) + lag
    FF: candidateES = EF(X) + lag - dur(succ)
    SF: candidateES = ES(X) + lag - dur(succ)

  ES(succ) = max(ES(succ), candidateES)

处理完所有前驱后：EF(X) = ES(X) + dur(X)

总工期 = max(所有任务的 EF)
```

**Step 3：后向传播（按逆拓扑序）**

```
初始化：对于所有无后继（出度为 0）的任务，LF = 总工期，LS = LF - dur(X)。

对于每个任务 X（按逆拓扑序），遍历其每一条前驱依赖 (pred → X, type, lag)：

  根据 type 计算 pred 的候选 LF 或 LS：
    FS: 对 pred 的 LF 约束 = LS(X) - lag
    SS: 对 pred 的 LS 约束 = LS(X) - lag
    FF: 对 pred 的 LF 约束 = LF(X) - lag
    SF: 对 pred 的 LS 约束 = LF(X) - lag

  对于 FS/FF 类型：LF(pred) = min(LF(pred), constraint)，然后 LS(pred) = LF(pred) - dur(pred)
  对于 SS/SF 类型：LS(pred) = min(LS(pred), constraint)，然后 LF(pred) = LS(pred) + dur(pred)
```

**Step 4：提取关键路径**

```
遍历所有任务：若 ES == LS（等价于 EF == LF），则该任务在关键路径上。
按 ES 升序排列（即拓扑序），得到关键路径任务 ID 列表。
```

**Step 5：构造 ScheduleResult**

```
将每个 TaskId → TaskScheduleInfo{ES, EF, LS, LF} 填入 map，
与 totalDuration、criticalPath 一起送入 ScheduleResult 构造函数，按值返回。
```

> **验证基准**：PLANNER/ImportFormat 样例 ProjectDemo（6 个任务）应得出：
>
> - 总工期 = 22 天
> - 关键路径 = [1, 2, 3, 4, 5]

### 5.3 空项目处理

```
若 TaskCount() == 0：
  - totalDuration = 0
  - criticalPath 为空
  - data_ 为空 map
```

---

## 6. `ProjectController` — 单例控制器

### 6.1 职责

`ProjectController` 是 Service 层的**唯一对外入口**，也是整个程序生命周期中**只能有 1 个实例**的单例对象。

它持有当前 Project、持有 ProjectValidator / CPMCalculator 实例，对外提供与 PLANNER.md 需求一一对应的方法。

### 6.2 完整接口

```cpp
// 文件：service/ProjectController.hpp
// 职责：Service 层单例控制器，统一对外提供所有服务操作

class ProjectController
{
  public:
    //------ 单例 ------
    static ProjectController& GetInstance();

    // 禁止拷贝和移动
    ProjectController(const ProjectController&)            = delete;
    ProjectController& operator=(const ProjectController&) = delete;

    //------ 项目导入（需求 1）------
    // 根据文件扩展名选择对应导入器，导入成功则替换当前项目
    // 返回 false 时 errorMsg 包含失败原因
    bool ImportProject(const std::string& filePath,
                       std::string&       errorMsg);

    //------ 项目导出（需求 2）------
    // 根据文件扩展名选择对应导出器
    // 返回 false 时 errorMsg 包含失败原因
    bool ExportProject(const std::string& filePath,
                       std::string&       errorMsg) const;

    //------ 项目状态 ------
    bool HasProject() const;  // 当前是否持有项目

    //------ 任务管理（需求 3.1）------
    std::vector<TaskDisplayInfo> ListTasks() const;
    bool RemoveTask(int index, std::string& errorMsg);
    bool AddTask(const std::string& name, int duration,
                 std::string& errorMsg);
    // 返回指定任务的前驱与后继信息
    // first = 前驱列表, second = 后继列表
    std::pair<std::vector<TaskDisplayInfo>, std::vector<TaskDisplayInfo>>
        GetTaskRelations(int index) const;
    bool ModifyTask(int index, const std::string& newName,
                    int newDuration, std::string& errorMsg);

    //------ 依赖管理（需求 3.2）------
    std::vector<DependencyDisplayInfo> ListDependencies() const;
    bool RemoveDependency(int index, std::string& errorMsg);
    bool AddDependency(int predIndex, int succIndex,
                       DependencyType type, int lag,
                       std::string& errorMsg);

    //------ 资源管理（需求 3.3）------
    std::vector<ResourceDisplayInfo> ListResources() const;
    bool AddResource(const std::string& name, double unitCost,
                     std::string& errorMsg);
    bool AssignResource(int taskIndex, int resourceIndex,
                        int quantity, std::string& errorMsg);

    //------ 统计信息（需求 4）------
    ProjectStatistics GetStatistics() const;

    //------ 验证与调度（需求 5）------
    ValidationResult Validate() const;
    ScheduleResult   ComputeSchedule() const;

  private:
    ProjectController();   // 私有一致构造
    ~ProjectController() = default;

    //------ 内部辅助：索引 ↔ ID 映射 ------
    // 将容器索引（0-based）映射为 TaskId
    // 索引越界返回 TaskId::Invalid()
    TaskId IndexToTaskId(int index) const;
    // 将 TaskId 映射为容器索引，未找到返回 -1
    int    TaskIdToIndex(TaskId id) const;
    // 将容器索引映射为 ResourceId，越界返回 ResourceId::Invalid()
    ResourceId IndexToResourceId(int index) const;

    //------ 内部辅助：名称唯一性 ------
    // 检查任务名是否重复，可选排除指定 ID（修改场景：不与自己比较）
    bool IsTaskNameDuplicate(const std::string& name,
                             TaskId excludeId = TaskId::Invalid()) const;
    bool IsResourceNameDuplicate(const std::string& name) const;

    //------ 内部辅助：展示信息构建 ------
    TaskDisplayInfo       BuildTaskDisplayInfo(int index, const Task& task) const;
    DependencyDisplayInfo BuildDependencyDisplayInfo(int index,
                            const Dependency& dep) const;
    ResourceDisplayInfo   BuildResourceDisplayInfo(int index,
                            const Resource& res) const;

    //------ 内部辅助：DAG 快速检查（添加依赖时复用）------
    // 在现有 project 上模拟添加一条依赖后，检查是否产生环
    // 比完整 Validate 更轻量，仅做无环检查
    bool WouldCreateCycle(TaskId pred, TaskId succ) const;

    //------ 成员 ------
    std::unique_ptr<Project> m_project;
    ProjectValidator         m_validator;
    CPMCalculator            m_calculator;
};
```

### 6.3 各方法实现要点

#### 6.3.1 导入导出

```
ImportProject(filePath, errorMsg):
  1. 解析 filePath 扩展名 → 选择导入器
     - ".ppm" → PpmImporter(filePath)
     - 未知扩展名 → errorMsg = "不支持的文件格式", return false
  2. importer.Import() → unique_ptr<Project>
     - 返回 nullptr → errorMsg = "导入失败: ...", return false
  3. m_project = std::move(result)
  4. return true

ExportProject(filePath, errorMsg):
  1. 若 !HasProject() → errorMsg = "无项目可导出", return false
  2. 解析扩展名 → 选择导出器
  3. exporter.Export(*m_project) → bool
  4. return result
```

#### 6.3.2 任务管理

```
ListTasks():
  1. 遍历 m_project->GetTasks()（顺序即容器顺序）
  2. 为每个 Task 构建 TaskDisplayInfo：
     - index = 遍历下标
     - idValue = task.GetId().Value()
     - name = task.GetName()
     - duration = task.GetDuration()
     - predecessorIndices = 对每个 GetPredecessors(id)，查 TaskIdToIndex
     - successorIndices   = 对每个 GetSuccessors(id)，查 TaskIdToIndex
  3. 返回 vector<TaskDisplayInfo>

RemoveTask(index, errorMsg):
  1. TaskId id = IndexToTaskId(index)
  2. 若 id == Invalid() → errorMsg = "索引无效", return false
  3. m_project->RemoveTask(id)  // Model 层级联删除依赖与分配
  4. return true

AddTask(name, duration, errorMsg):
  1. 若 name 为空 → errorMsg = "任务名称不能为空", return false
  2. 若 IsTaskNameDuplicate(name) → errorMsg = "任务名称已存在", return false
  3. m_project->AddTask(name, duration)
     - duration == 0 → Model 层自动创建 MilestoneBehavior
     - duration  > 0 → Model 层自动创建 NormalBehavior
  4. return true

GetTaskRelations(index):
  1. TaskId id = IndexToTaskId(index)，若无效返回空 pair
  2. 遍历 GetPredecessors(id)，为每个 pred 构建 TaskDisplayInfo
  3. 遍历 GetSuccessors(id)，为每个 succ 构建 TaskDisplayInfo
  4. 返回 {predecessors, successors}

ModifyTask(index, newName, newDuration, errorMsg):
  1. TaskId id = IndexToTaskId(index)，若无效 → errorMsg, return false
  2. 若 newName != task.GetName()：
     - 检查 IsTaskNameDuplicate(newName, excludeId=id) → 重复则拒绝
     - task.SetName(newName)  // 注意获取 non-const Task*（见 §9 缺口）
  3. 若 newDuration != task.GetDuration()：
     - task.SetDuration(newDuration)  // 内部自动切换策略
  4. return true
  注意：修改前后容器索引不变——Task 对象本身未被替换，仅调用 setter。
```

> **ModifyTask 的 non-const Task 访问**：`Project::FindTask` 返回 `const Task*`，
> 无法调用 `SetName` / `SetDuration`。需要 Model 层提供 `FindTask` 的 non-const 重载
> 或 `GetTaskByIndex` 等接口。详见 §9。

#### 6.3.3 依赖管理

```
ListDependencies():
  1. 遍历 m_project->GetDependencies()
  2. 为每个 Dependency 构建 DependencyDisplayInfo：
     - index = 遍历下标
     - predecessorIndex = TaskIdToIndex(dep.GetPredecessorId())
     - successorIndex   = TaskIdToIndex(dep.GetSuccessorId())
     - type = dep.GetType()
     - lag  = dep.GetLag()
  3. 返回 vector

RemoveDependency(index, errorMsg):
  1. 通过 GetDependencies()[index] 获取依赖
  2. 若 index 越界 → errorMsg, return false
  3. TaskId pred = dep.GetPredecessorId()
  4. TaskId succ = dep.GetSuccessorId()
  5. m_project->RemoveDependency(pred, succ)  // 需 Model 层实现（§2.3）
  6. return true

AddDependency(predIndex, succIndex, type, lag, errorMsg):
  1. TaskId predId = IndexToTaskId(predIndex)，若无效 → errorMsg
  2. TaskId succId = IndexToTaskId(succIndex)，若无效 → errorMsg
  3. 若 predId == succId → errorMsg = "不能创建自引用依赖", return false
  4. 若 m_project->FindDependency(predId, succId) != nullptr
     → errorMsg = "该依赖已存在", return false
  5. 若 WouldCreateCycle(predId, succId)
     → errorMsg = "添加此依赖会产生循环", return false
  6. m_project->AddDependency(predId, succId, type, lag)
  7. return true
```

**`WouldCreateCycle` 轻量实现**：

```
不修改真实数据，仅基于当前邻接索引模拟：
  1. 从 succId 出发，沿 GetSuccessors 做 BFS/DFS
  2. 若能到达 predId → 添加 (pred→succ) 会形成环
  3. 注意：需要用 visited 集合避免无限循环（虽然已知当前无环，但防守编程）
```

#### 6.3.4 资源管理

```
ListResources():
  1. 遍历 m_project->GetResources()
  2. 为每个 Resource 构建 ResourceDisplayInfo
  3. 返回 vector

AddResource(name, unitCost, errorMsg):
  1. 若 name 为空 → errorMsg, return false
  2. 若 IsResourceNameDuplicate(name) → errorMsg, return false
  3. m_project->AddResource(name, unitCost)
  4. return true

AssignResource(taskIndex, resourceIndex, quantity, errorMsg):
  1. TaskId taskId = IndexToTaskId(taskIndex)，若无效 → errorMsg
  2. ResourceId resId = IndexToResourceId(resourceIndex)，若无效 → errorMsg
  3. 若 quantity <= 0 → errorMsg = "分配数量必须为正整数", return false
  4. const Task* task = m_project->FindTask(taskId)
  5. 若 task->CanAllocateResource() == false
     → errorMsg = "里程碑任务不可分配资源", return false
  6. m_project->AssignResource(taskId, resId, quantity)
  7. return true
```

#### 6.3.5 统计信息

```
GetStatistics():
  1. stats.taskCount       = m_project->TaskCount()
  2. stats.dependencyCount = m_project->DependencyCount()
  3. stats.resourceCount   = m_project->ResourceCount()
  4. 若 TaskCount() == 0：stats.totalDuration = 0
     否则：调用 m_calculator.Calculate(*m_project).GetTotalDuration()
  5. return stats
```

> **totalDuration 依赖 CPM 计算**：需求 4 的"总工期 = 关键路径长度"。
> 若项目尚未验证为合理，`ComputeSchedule` 前置条件不满足，此处的行为由调用方保证。
> 一种防御策略：先调用 Validate()，若不通过则 totalDuration = -1（未定义）。

#### 6.3.6 验证与调度

```
Validate():
  1. 若 !HasProject() → 返回 ValidationResult({"无项目可验证"})
  2. return m_validator.Validate(*m_project)

ComputeSchedule():
  1. 若 !HasProject() → 返回空 ScheduleResult（totalDuration=0, 空 map, 空 criticalPath）
  2. return m_calculator.Calculate(*m_project)
  // 前置条件（IsValid）由调用方保证
```

---

## 7. 导入导出继承体系

### 7.1 已有接口（保持不变）

- `IProjectImporter` — `service/IProjectImporter.hpp`，纯虚 `Import() → unique_ptr<Project>`
- `IProjectExporter` — `service/IProjectExporter.hpp`，纯虚 `Export(const Project&) → bool`
- `ManualImporter` — `service/ManualImporter.hpp/.cpp`，硬编码样例，测试验证基准

### 7.2 待实现：`PpmImporter`

```cpp
// 文件：service/PpmImporter.hpp + service/PpmImporter.cpp
// 职责：读取 PPM 格式文件，解析为 Project 对象
// 继承：IProjectImporter

class PpmImporter : public IProjectImporter
{
  public:
    // 构造时绑定文件路径
    explicit PpmImporter(const std::string& filePath);

    PpmImporter(const PpmImporter&)            = default;
    PpmImporter& operator=(const PpmImporter&) = default;
    ~PpmImporter() override                    = default;

    // 读取文件内容，逐行解析，构建并返回 Project
    std::unique_ptr<Project> Import() override;

  private:
    std::string m_filePath;
};
```

**Import 实现要点**：

```
1. 打开文件，若失败返回 nullptr
2. 创建空的 unique_ptr<Project>
3. 逐行读取：
   - 跳过空行、注释行（'#' 开头）
   - 'P' 行：project->SetName(项目名称)
   - 'T' 行：解析 ID、名称、工期 → project->AddTask(TaskId(id), name, duration)
             若返回 Invalid → 记录警告，跳过此行
   - 'M' 行：解析 ID、名称 → project->AddTask(TaskId(id), name, 0)
   - 'R' 行：解析 ID、名称、单位成本 → project->AddResource(ResourceId(id), name, cost)
   - 'D' 行：解析 predId, succId, type 字符串→枚举, lag → project->AddDependency(...)
   - 'A' 行：解析 taskId, resourceId, quantity → project->AssignResource(...)
4. 文件格式错误（如行前缀非法、字段数不足、ID 非数字）→ 返回 nullptr
5. 返回 project

辅助函数：
  - Trim(s) — 去除首尾空格
  - Split(s) — 按空格分割字符串
  - ParseDependencyType("FS") → DependencyType::FS 等
```

> **容错策略**：P 行缺失时 project 名称为空串（合法）；显式 ID 重复时 `AddTask(TaskId)` 返回 Invalid，
> 导入器可选择记录警告并跳过该行（使导入尽量成功），或直接返回 nullptr。
> 此处采用**严格模式**——任何解析异常返回 nullptr，由调用方 `ImportProject` 提供友好错误信息。

### 7.3 待实现：`PpmExporter`

```cpp
// 文件：service/PpmExporter.hpp + service/PpmExporter.cpp
// 职责：将 Project 对象写入 PPM 格式文件
// 继承：IProjectExporter

class PpmExporter : public IProjectExporter
{
  public:
    explicit PpmExporter(const std::string& filePath);

    PpmExporter(const PpmExporter&)            = default;
    PpmExporter& operator=(const PpmExporter&) = default;
    ~PpmExporter() override                    = default;

    bool Export(const Project& project) override;

  private:
    std::string m_filePath;
};
```

**Export 实现要点**：

```
1. 创建/覆盖文件，若无法创建返回 false
2. 写入：
   - P 行：project.GetName()
   - T 行：遍历 GetTasks()，duration > 0 者写 "T ID Name Duration"
   - M 行：遍历 GetTasks()，duration == 0 者写 "M ID Name 0"
   - R 行：遍历 GetResources()，写 "R ID Name UnitCost"
   - D 行：遍历 GetDependencies()，写 "D PredID SuccID Type Lag"
     - DependencyType → 字符串："FS"/"SS"/"FF"/"SF"
   - A 行：遍历 GetAllocations()，写 "A TaskID ResourceID Quantity"
3. 关闭文件，返回 true

注意：PPM 格式要求块顺序（T/M → R → D → A），写入时按此顺序组织。
```

---

## 8. 类关系图

```text
                    ┌──────────────────────┐
                    │  ProjectController    │
                    │  (Singleton)          │
                    │                       │
                    │  + GetInstance()      │
                    │  + ImportProject()    │
                    │  + ExportProject()    │
                    │  + ListTasks()        │
                    │  + RemoveTask()       │
                    │  + AddTask()          │
                    │  + GetTaskRelations() │
                    │  + ModifyTask()       │
                    │  + ListDependencies() │
                    │  + RemoveDependency() │
                    │  + AddDependency()    │
                    │  + ListResources()    │
                    │  + AddResource()      │
                    │  + AssignResource()   │
                    │  + GetStatistics()    │
                    │  + Validate()         │
                    │  + ComputeSchedule()  │
                    └──────┬───────────────┘
                           │ 持有
              ┌────────────┼────────────┐
              ▼            ▼            ▼
     ┌──────────────┐ ┌──────────┐ ┌──────────┐
     │ unique_ptr   │ │Project   │ │CPM       │
     │ <Project>    │ │Validator │ │Calculator│
     └──────┬───────┘ └────┬─────┘ └────┬─────┘
            │              │            │
            │         只读 │            │ 只读
            ▼              ▼            ▼
     ┌──────────┐  ┌──────────────────────────┐
     │  Model   │  │      const Project&       │
     │  Project │  └──────────────────────────┘
     └──────────┘

    IProjectImporter          IProjectExporter
    (纯虚接口)                (纯虚接口)
         △                         △
         │                         │
    ┌────┴────┐              ┌────┴────┐
    │Ppm      │              │Ppm      │
    │Importer │              │Exporter │
    └─────────┘              └─────────┘

    ┌─────────────────┐     ┌─────────────────┐
    │ ValidationResult│     │ TaskDisplayInfo │
    │ (DTO, service/) │     │ DependencyDisp..│
    └─────────────────┘     │ ResourceDisp... │
                            │ ProjectStats    │
                            │ (DTO, service/) │
                            └─────────────────┘
```

> Model 层 `ScheduleResult` 是被 `CPMCalculator` 产出的返回值，不属于 Service 层。

---

## 9. Model 层缺口清单（实现前必须补齐）

| # | 缺口 | 影响范围 | 建议接口 |
| :--- | :--- | :--- | :--- |
| 1 | **`ScheduleResult` 类** | CPMCalculator、GetStatistics、ComputeSchedule | 按 ModelSchedule.md §3.7 完整实现：`ScheduleResult(map<TaskId, TaskScheduleInfo>, int totalDuration, vector<TaskId> criticalPath)` + 六个 const 访问器 |
| 2 | **`Project::GetTasks()` 等遍历接口** | 全部 List 方法、索引映射、导入导出 | `const vector<Task>& GetTasks() const`、`const vector<Dependency>& GetDependencies() const`、`const vector<Resource>& GetResources() const`、`const vector<Allocation>& GetAllocations() const` |
| 3 | **`Project::RemoveDependency(pred, succ)`** | RemoveDependency | `void RemoveDependency(TaskId pred, TaskId succ)`——移除后更新 `successors_`/`predecessors_`，不存在则静默忽略 |
| 4 | **`Project::FindTask(TaskId)` 的 non-const 重载** | ModifyTask（需要调用 Task::SetName/SetDuration） | `Task* FindTask(TaskId id)`——或提供 `GetTaskByIndex(int)` 返回 non-const 指针/引用 |
| 5 | **`Dependency` 访问器确认** | ListDependencies、导入导出 | `GetPredecessorId()`、`GetSuccessorId()`、`GetType()`、`GetLag()` |
| 6 | **`Allocation` 访问器确认** | ListResources（分配展示）、导入导出 | `GetTaskId()`、`GetResourceId()`、`GetQuantity()` |

---

## 10. 文件清单与实现顺序

### 10.1 新建文件

| 文件 | 说明 | 优先级 | 实现顺序 |
| :--- | :--- | :--- | :--- |
| `service/ValidationResult.hpp` | 验证结果 DTO（仅头文件，无 .cpp） | P0 | 1 |
| `service/ValidationResult.cpp` | （函数体简单，可全部内联于 .hpp，也可按规范分离） | P0 | 1 |
| `service/ProjectValidator.hpp` | 验证器声明 | P0 | 2 |
| `service/ProjectValidator.cpp` | 验证器实现（三条检查） | P0 | 2 |
| `service/CPMCalculator.hpp` | 计算器声明 | P0 | 3 |
| `service/CPMCalculator.cpp` | 计算器实现（拓扑+前向+后向） | P0 | 3 |
| `service/TaskDisplayInfo.hpp` | 任务展示 DTO（纯头文件） | P1 | 4 |
| `service/DependencyDisplayInfo.hpp` | 依赖展示 DTO（纯头文件） | P1 | 4 |
| `service/ResourceDisplayInfo.hpp` | 资源展示 DTO（纯头文件） | P1 | 4 |
| `service/ProjectStatistics.hpp` | 统计信息 DTO（纯头文件） | P1 | 4 |
| `service/ProjectController.hpp` | 控制器声明 | P1 | 5 |
| `service/ProjectController.cpp` | 控制器实现 | P1 | 5 |
| `service/PpmImporter.hpp` | PPM 导入器声明 | P2 | 6 |
| `service/PpmImporter.cpp` | PPM 导入器实现 | P2 | 6 |
| `service/PpmExporter.hpp` | PPM 导出器声明 | P2 | 7 |
| `service/PpmExporter.cpp` | PPM 导出器实现 | P2 | 7 |

### 10.2 已有文件（保持不变或仅引用路径调整）

- `service/IProjectImporter.hpp`
- `service/IProjectExporter.hpp`
- `service/ManualImporter.hpp`
- `service/ManualImporter.cpp`

### 10.3 推荐实现顺序

```
Phase 0：Model 层缺口补齐
  ├── ScheduleResult + TaskScheduleInfo
  ├── Project::GetTasks() / GetDependencies() / GetResources() / GetAllocations()
  ├── Project::RemoveDependency(pred, succ)
  └── Project::FindTask() non-const 重载

Phase 1：无依赖的 DTO 与纯算法类（可并行的独立工作）
  ├── 1. ValidationResult
  ├── 2. ProjectValidator
  └── 3. CPMCalculator

Phase 2：DTO 与控制器
  ├── 4. TaskDisplayInfo / DependencyDisplayInfo / ResourceDisplayInfo / ProjectStatistics
  └── 5. ProjectController（依赖 Phase 0 缺口 + Phase 1 全部）

Phase 3：导入导出
  ├── 6. PpmImporter
  └── 7. PpmExporter

Phase 4：验证
  └── 8. 以 ManualImporter 构造 ProjectDemo，联调 Validator → CPMCalculator → 核对 22 天 + [1,2,3,4,5]
```

---

## 11. 设计决策记录

### 11.1 为什么编辑器逻辑不拆成独立类

PLANNER.md 简略类图中 `ProjectEditor → TaskEditor / DependencyEditor / ResourceEditor` 是一种**概念性分解**。落实到代码时，三个编辑器的每个方法都是"参数校验 + 调一个 Model 层方法"的薄层（合计约 11 个方法），独立成类会引入 6 个文件和不必要的胶水代码。将这些方法作为 `ProjectController` 的私有成员，既保持代码内聚又满足简洁原则。**这是一个刻意延迟的决策——若未来编辑逻辑膨胀，随时可提取，无需修改公开接口。**

### 11.2 为什么不引入 ValidationErrorCode 枚举

需求 5.1 只要求"返回具体错误信息"，字符串列表已完全满足。枚举会带来同步维护成本且扩展新检查时需改定义。`ValidationResult` 接口保持最小——`bool IsValid()` + `vector<string> GetErrors()`。

### 11.3 为什么容器索引映射放在 Service 层

需求文档以"容器索引"作为界面层定位手段，而 Model 层以 `TaskId` / `ResourceId` 定位。Service 层处于两套体系之间，天然负责翻译。`IndexToTaskId` / `TaskIdToIndex` 通过 `GetTasks()` 遍历构建映射，简单可靠。

### 11.4 为什么 ProjectValidator 和 CPMCalculator 是无状态的

二者都只依赖 `const Project&` 输入，不持有可变状态。无状态意味着：

- 可被 `ProjectController` 在构造时创建一次并反复调用（高效，无需重复分配）
- 可独立单元测试（只需构造一个 Project 即可验证全部三条检查，或验证 CPM 结果）

### 11.5 为什么 IProjectImporter::Import() 不接受参数

每个具体导入器的数据源（文件路径、网络地址等）在**构造函数**中绑定，`Import()` 无参调用。这是 Strategy 模式的标准用法——调用方不需要知道数据源的细节。同理 `IProjectExporter` 在构造时绑定输出目标。

---

## 12. 验证基准

以 `ManualImporter` 构造的 ProjectDemo（6 任务、5 依赖、5 资源、7 分配）为基准：

| 验证项 | 预期结果 |
| :--- | :--- |
| `Validate()` | `IsValid() == true`，错误列表为空 |
| `GetStatistics()` | taskCount=6, depCount=5, resCount=5, totalDuration=22 |
| `ComputeSchedule().GetTotalDuration()` | 22 |
| `ComputeSchedule().GetCriticalPath()` | `[TaskId(1), TaskId(2), TaskId(3), TaskId(4), TaskId(5)]` |
| 任务 1 ES/EF/LS/LF | 0 / 5 / 0 / 5 |
| 任务 5 ES/EF/LS/LF | 20 / 22 / 20 / 22 |
| 任务 6（里程碑）ES/EF/LS/LF | 17 / 17 / 22 / 22 |
