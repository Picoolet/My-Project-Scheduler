# Service 层最终设计 (ServiceSchedule)

> 本文档是业务层（Service 层）的**最终设计**，供 Coding Agent 按章逐项实现。
> 设计依据：PLANNER/PLANNER.md（需求规格）、PLANNER/ModelSchedule.md（Model 层最终设计）。
> 审计记录：PLANNER/ServiceSchedule_Audit.md → 所有审计结论已纳入本版。

---

## 1. 定位与架构

### 1.1 定位

Service 层是"没有界面的整个软件功能集合"，位于 Model 层之上、界面层之下。

**核心约定**：
- 只使用 C++ 标准库与 Model 层公开接口，不触碰 Model 私有成员。
- 不使用任何 GUI 类型（`QString` 等），所有数据以 `std::string` / STL 容器传递。
- 不含 `cout` / `cin` 或弹窗。输入以参数传入，结果以返回值或引用参数传出。
- **`ProjectController` 在整个程序生命周期中只能有 1 个实例（单例模式）。**

### 1.2 架构全景

```
                        View / UI
                           │
                           ▼
                 ┌─────────────────────┐
                 │ ProjectController   │  ← Singleton，唯一对外入口
                 │ (Singleton)         │
                 └────────┬────────────┘
                          │ 持有并协调
          ┌───────────────┼───────────────┬────────────────┐
          ▼               ▼               ▼                ▼
   ┌────────────┐  ┌────────────┐  ┌───────────┐  ┌──────────────┐
   │Project     │  │Project     │  │Project    │  │IProject      │
   │Editor      │  │DTOBuilder  │  │Validator  │  │Importer/     │
   │(修改+规则) │  │(数据转换)  │  │(图算法)   │  │Exporter      │
   └─────┬──────┘  └─────┬──────┘  └─────┬─────┘  └──────┬───────┘
         │               │               │               │
         ▼               ▼               ▼               ▼
   ┌──────────────────────────────────────────────────────────┐
   │                   Model 层 (Project)                      │
   └──────────────────────────────────────────────────────────┘
         │               │               │
         ▼               ▼               ▼
   ┌──────────┐  ┌──────────────┐  ┌──────────────────┐
   │CPM       │  │ImportResult  │  │TaskDTO           │
   │Calculator│  │Validation    │  │DependencyDTO     │
   │(调度计算)│  │Result (DTO)  │  │ResourceDTO ...   │
   └──────────┘  └──────────────┘  └──────────────────┘
```

### 1.3 职责分配

| 类 | 职责 | 一句话 |
| :--- | :--- | :--- |
| `ProjectController` | 持有 Project，接收请求，分发给下属组件 | "谁该处理这个请求？" |
| `ProjectEditor` | 对 `Project&` 执行修改 + 全部业务规则校验 | "这次修改合法吗？执行。" |
| `ProjectDTOBuilder` | 遍历 `const Project&`，组装 DTO | "Model 数据怎么变成 DTO？" |
| `ProjectValidator` | 完整验证 + 单边环检测（`WouldCreateCycle`） | "这个图有环吗？合理吗？" |
| `CPMCalculator` | 拓扑排序 + 前向/后向传播 + 关键路径 | "关键路径是什么？" |
| `IProjectImporter`+ 派生 | 格式→Project | "这个文件怎么变成 Project？" |
| `IProjectExporter`+ 派生 | Project→格式 | "Project 怎么写成文件？" |

---

## 2. 设计前提

### 2.1 Model 层已提供

| 接口 | 用途 |
| :--- | :--- |
| `Project::AddTask(name, duration)` / `AddTask(TaskId,...)` | 添加任务（自动 ID / 显式 ID） |
| `Project::RemoveTask(TaskId)` | 级联删除任务 |
| `Project::AddResource(name, cost)` / `AddResource(ResourceId,...)` | 添加资源 |
| `Project::RemoveResource(ResourceId)` | 级联删除资源 |
| `Project::AddDependency(pred, succ, type, lag)` | 添加依赖（重复忽略） |
| `Project::RemoveDependency(pred, succ)` | 移除依赖并更新索引 |
| `Project::AssignResource(taskId, resId, qty)` | upsert 分配 |
| `Project::FindTask(TaskId)` — const + non-const 重载 | O(1) 查任务（基于位置索引） |
| `Project::FindResource(ResourceId)` | 查资源 |
| `Project::FindDependency(pred, succ)` | 查依赖 |
| `Project::GetPredecessors(TaskId)` / `GetSuccessors(TaskId)` | 邻接查询 |
| `Project::GetTasks()` / `GetDependencies()` / `GetResources()` / `GetAllocations()` | const 引用遍历 |
| `Project::TaskCount()` / `DependencyCount()` / `ResourceCount()` | 计数 |
| `Project::GetName()` / `SetName()` | 项目名称 |
| `Task::GetId()` / `GetName()` / `GetDuration()` | 任务属性 |
| `Task::SetName()` / `SetDuration()` | 修改任务（内部自动切换策略） |
| `Task::CanAllocateResource()` | 里程碑判定 |
| `DependencyType { FS, SS, FF, SF }` | 依赖类型枚举 |
| `ScheduleResult` + `TaskScheduleInfo` | 调度结果载体 |

### 2.2 Service 层已有资产

| 文件 | 说明 |
| :--- | :--- |
| `service/IProjectImporter.hpp` | 抽象导入接口 |
| `service/IProjectExporter.hpp` | 抽象导出接口 |
| `service/ManualImporter.hpp/.cpp` | 测试桩，硬编码 PPM 样例，验证基准用 |

### 2.3 实现前提

以下 Model 层接口是 Service 层各组件的前提，**实现前必须确认已存在**，详见 ModelSchedule.md §3.6：
- `ScheduleResult` 完整实现（ModelSchedule.md §3.7）
- Task 位置索引 + O(1) `FindTask`（ModelSchedule.md §4.8）
- `GetTasks()` / `GetDependencies()` / `GetResources()` / `GetAllocations()` 四个遍历器
- `RemoveDependency(TaskId pred, TaskId succ)`
- `FindTask(TaskId)` non-const 重载

---

## 3. 数据结构（DTO — 本层定义）

DTO（Data Transfer Object）是 Service 层向界面层传递数据的纯数据载体，不含任何业务逻辑。

### 3.1 `TaskDTO`

```cpp
// 文件：service/TaskDTO.hpp
struct TaskDTO
{
    int         index;                  // 容器索引（0-based）
    int         idValue;                // TaskId 数值
    std::string name;                   // 任务名称
    int         duration;               // 工期
    std::vector<int> predecessorIndices; // 前驱任务索引列表
    std::vector<int> successorIndices;   // 后继任务索引列表
};
```

### 3.2 `DependencyDTO`

```cpp
// 文件：service/DependencyDTO.hpp
struct DependencyDTO
{
    int            index;             // 序号（0-based）
    int            predecessorIndex;  // 前置任务索引
    int            successorIndex;    // 后置任务索引
    DependencyType type;              // FS / SS / FF / SF
    int            lag;               // 时差
};
```

### 3.3 `ResourceDTO`

```cpp
// 文件：service/ResourceDTO.hpp
struct ResourceDTO
{
    int         index;     // 容器索引（0-based）
    int         idValue;   // ResourceId 数值
    std::string name;      // 资源名称
    double      unitCost;  // 单位成本
};
```

### 3.4 `ProjectStatisticsDTO`

```cpp
// 文件：service/ProjectStatisticsDTO.hpp
struct ProjectStatisticsDTO
{
    int  taskCount;        // Task 总数（含普通与里程碑）
    int  dependencyCount;  // Dependency 总数
    int  resourceCount;    // Resource 总数
    bool isValid;          // 项目是否通过合理性验证
    int  totalDuration;    // 关键路径长度（天）；isValid==false 时取 -1
};
```

### 3.5 `ValidationResult`

```cpp
// 文件：service/ValidationResult.hpp
class ValidationResult
{
  public:
    ValidationResult();   // 默认 = 有效（空错误列表）
    explicit ValidationResult(const std::vector<std::string>& errors);

    void AddError(const std::string& error);  // 供 Validator 内部追加

    bool IsValid() const;
    const std::vector<std::string>& GetErrors() const;

  private:
    std::vector<std::string> m_errors;
};
```

### 3.6 `ImportResult`

```cpp
// 文件：service/ImportResult.hpp
// 职责：封装导入操作的完整结果，取代返回 nullptr 的模糊错误传递
class ImportResult
{
  public:
    ImportResult();   // 空结果 = 失败（无 Project）
    explicit ImportResult(std::unique_ptr<Project> project);
    ImportResult(std::unique_ptr<Project> project,
                 const std::vector<std::string>& errors,
                 const std::vector<std::string>& warnings);

    bool HasProject() const;
    Project* GetProject();             // 调用方接管所有权前检查
    std::unique_ptr<Project> ReleaseProject();  // 移交所有权

    bool HasErrors() const;
    bool HasWarnings() const;
    const std::vector<std::string>& GetErrors() const;
    const std::vector<std::string>& GetWarnings() const;

  private:
    std::unique_ptr<Project>  m_project;
    std::vector<std::string>  m_errors;
    std::vector<std::string>  m_warnings;
};
```

> **设计说明**：`Import()` 原返回 `unique_ptr<Project>`，失败只能返回 `nullptr`，无法区分"文件不存在"、"格式错误"、"ID 重复"等失败原因。`ImportResult` 将成功/失败/警告封装为一个值对象——Importer 负责填充，Controller 负责转发，UI 负责展示。

---

## 4. `ProjectValidator` — 合理性验证器

### 4.1 职责与接口

```cpp
// 文件：service/ProjectValidator.hpp
// 职责：对 const Project& 执行图分析——完整验证（需求 5.1）+ 单边环检测（供 Editor 复用）
// 特点：无状态；所有方法为 const；一个实例可反复使用

class ProjectValidator
{
  public:
    ProjectValidator()                              = default;
    ProjectValidator(const ProjectValidator&)       = default;
    ProjectValidator& operator=(const ProjectValidator&) = default;
    ~ProjectValidator()                             = default;

    // 执行全部三条检查，收集所有问题后一次性返回
    ValidationResult Validate(const Project& project) const;

    // 单边环检测：在现有依赖图上，若添加 (pred→succ) 是否会形成环
    // 从 succ 出发沿 GetSuccessors BFS/DFS，检查是否能到达 pred
    // 仅供 ProjectEditor::AddDependency 调用
    bool WouldCreateCycle(const Project& project,
                          TaskId pred, TaskId succ) const;

  private:
    // ① 依赖图无环：Kahn 算法
    void CheckAcyclic(const Project& project,
                      std::vector<std::string>& errors) const;

    // ② 无悬挂节点：正向可达 ∩ 反向可达 = 全部任务
    void CheckNoDangling(const Project& project,
                         std::vector<std::string>& errors) const;

    // ③ 引用完整性：每条 Dependency 的 pred/succ 均存在
    void CheckReferenceIntegrity(const Project& project,
                                 std::vector<std::string>& errors) const;
};
```

### 4.2 算法要点

**① CheckAcyclic（Kahn 算法）**：

```
1. 遍历所有任务，为每个 TaskId 统计入度 = GetPredecessors().size()
2. 将入度为 0 的任务加入队列
3. 反复取出队首任务，对其每个后继将其入度减 1；若减至 0 则入队
4. 若已处理的任务数 < TaskCount() → 存在环，追加错误信息
```

**② CheckNoDangling（双向可达性）**：

```
1. 找到所有入度=0 的起始集 S 和出度=0 的终止集 T
2. 若 S 为空且任务数 > 0 → 全部任务不可达，追加错误
3. 从 S 出发做正向 BFS/DFS → 收集可达集 R_forward
4. 从 T 出发沿 GetPredecessors 做反向 BFS/DFS → 收集 R_backward
5. R_forward ∩ R_backward 之外的任务为悬挂节点，逐个追加错误
```

**③ CheckReferenceIntegrity**：

```
遍历每条 Dependency：
  - FindTask(pred) == nullptr → 追加错误
  - FindTask(succ) == nullptr → 追加错误
```

**WouldCreateCycle**：

```
从 succId 出发沿 GetSuccessors 做 BFS/DFS：
  - 若能到达 predId → 添加 (pred→succ) 会成环 → return true
  - 否则 return false
使用 visited 集合防止无限循环（防守编程，虽然已知当前无环）。
```

---

## 5. `CPMCalculator` — 关键路径计算器

### 5.1 职责与接口

```cpp
// 文件：service/CPMCalculator.hpp
// 职责：对 const Project& 执行 CPM 计算，产出 ScheduleResult
// 前置条件：project 已通过 ProjectValidator 校验（无环、引用完整、无悬挂）
//          违反前置条件时行为未定义（由调用方保证）
// 特点：无状态；Calculate() 为 const；一个实例可反复使用

class CPMCalculator
{
  public:
    CPMCalculator()                            = default;
    CPMCalculator(const CPMCalculator&)        = default;
    CPMCalculator& operator=(const CPMCalculator&) = default;
    ~CPMCalculator()                           = default;

    ScheduleResult Calculate(const Project& project) const;

    // 判断某任务是否在关键路径上（EF == LF），ScheduleResult 须已包含该任务
    bool IsCritical(const ScheduleResult& result, TaskId id) const;
};
```

### 5.2 算法完整描述

设 dur(X) 为任务 X 的工期（`GetDuration()`）。

**Step 1 — 拓扑排序**：以 Kahn 算法获取所有任务的拓扑序，同时得到入度信息。

**Step 2 — 前向传播（按拓扑序）**：

```
初始化：无前驱（入度=0）的任务 ES=0，EF=dur(X)

对于每个任务 X（按拓扑序），遍历其后继依赖 (X → succ, type, lag)：

  根据 type 计算 succ 的候选 ES：
    FS: candidateES = EF(X) + lag
    SS: candidateES = ES(X) + lag
    FF: candidateES = EF(X) + lag - dur(succ)
    SF: candidateES = ES(X) + lag - dur(succ)

  ES(succ) = max(ES(succ), candidateES)
  EF(X) = ES(X) + dur(X)

总工期 = max(所有任务的 EF)
```

**Step 3 — 后向传播（按逆拓扑序）**：

```
初始化：无后继（出度=0）的任务 LF=总工期，LS=LF-dur(X)

对于每个任务 X（按逆拓扑序），遍历其前驱依赖 (pred → X, type, lag)：

  根据 type 计算 pred 的候选值：
    FS: 对 pred 的 LF 约束 = LS(X) - lag → LF(pred)=min(LF(pred), constraint)
                                              LS(pred)=LF(pred)-dur(pred)
    SS: 对 pred 的 LS 约束 = LS(X) - lag → LS(pred)=min(LS(pred), constraint)
                                              LF(pred)=LS(pred)+dur(pred)
    FF: 对 pred 的 LF 约束 = LF(X) - lag → （同 FS）
    SF: 对 pred 的 LS 约束 = LF(X) - lag → （同 SS）
```

**Step 4 — 提取关键路径**：

```
遍历所有任务：ES == LS（等价 EF == LF）→ 在关键路径上
按 ES 升序（即拓扑序）排列，得到关键路径 TaskId 列表
```

**Step 5 — 构造 ScheduleResult**：

```
将 TaskId → TaskScheduleInfo{ES, EF, LS, LF} 填入 map，
与 totalDuration、criticalPath 一起送入 ScheduleResult 构造函数，按值返回。
```

**空项目处理**：

```
若 TaskCount() == 0：totalDuration=0，criticalPath 为空，data_ 为空 map
```

> **验证基准**：PLANNER/ImportFormat 样例 ProjectDemo（6 个任务）应得出：
> 总工期 = 22 天，关键路径 = [1, 2, 3, 4, 5]

---

## 6. `ProjectEditor` — 项目编辑器

### 6.1 职责

`ProjectEditor` 封装**对 Project 的全部人工修改操作及其业务规则**（需求 3.1 ~ 3.3）。

与 Controller 的分工：
- Controller：接收请求，决定"谁来处理"
- Editor：执行修改，判断"这次修改是否合法"

### 6.2 完整接口

```cpp
// 文件：service/ProjectEditor.hpp
// 职责：对 Project& 执行受控修改，保证每次修改都符合业务规则
// 特点：构造时绑定 Project& 和 ProjectValidator&，不持有所有权

class ProjectEditor
{
  public:
    // 构造时绑定目标项目和验证器（验证器用于 AddDependency 的环检测）
    ProjectEditor(Project& project, const ProjectValidator& validator);

    ProjectEditor(const ProjectEditor&)            = default;
    ProjectEditor& operator=(const ProjectEditor&) = default;
    ~ProjectEditor()                               = default;

    //------ 任务管理（需求 3.1）------
    // 3.1.2 删除指定任务（级联由 Model 层保证）
    bool RemoveTask(int index, std::string& errorMsg);

    // 3.1.3 添加新任务：名称不可重复；duration==0 → 自动里程碑
    bool AddTask(const std::string& name, int duration,
                 std::string& errorMsg);

    // 3.1.5 修改指定任务：名称可改（不可重复），工期可改（0↔>0 自动切换策略）
    bool ModifyTask(int index, const std::string& newName,
                    int newDuration, std::string& errorMsg);

    //------ 依赖管理（需求 3.2）------
    // 3.2.2 删除指定依赖
    bool RemoveDependency(int index, std::string& errorMsg);

    // 3.2.3 添加新依赖：唯一性约束 + DAG 约束
    //       内部调用 validator.WouldCreateCycle 检查环
    bool AddDependency(int predIndex, int succIndex,
                       DependencyType type, int lag,
                       std::string& errorMsg);

    //------ 资源管理（需求 3.3）------
    // 3.3.2 添加新资源：名称不可重复
    bool AddResource(const std::string& name, double unitCost,
                     std::string& errorMsg);

    // 3.3.3 为任务分配资源：里程碑不可分配
    bool AssignResource(int taskIndex, int resourceIndex,
                        int quantity, std::string& errorMsg);

  private:
    //------ 索引 ↔ ID 映射 ------
    TaskId     IndexToTaskId(int index) const;
    int        TaskIdToIndex(TaskId id) const;
    ResourceId IndexToResourceId(int index) const;

    //------ 名称唯一性 ------
    bool IsTaskNameDuplicate(const std::string& name,
                             TaskId excludeId = TaskId::Invalid()) const;
    bool IsResourceNameDuplicate(const std::string& name) const;

    Project&               m_project;    // 目标项目（非 const 引用，不持有所有权）
    const ProjectValidator& m_validator; // 用于环检测
};
```

### 6.3 方法实现要点

```
RemoveTask(index, errorMsg):
  1. TaskId id = IndexToTaskId(index)，若无效 → errorMsg="索引无效", return false
  2. m_project.RemoveTask(id)  // Model 层级联删除依赖与分配
  3. return true

AddTask(name, duration, errorMsg):
  1. name 为空 → errorMsg, return false
  2. IsTaskNameDuplicate(name) → errorMsg="任务名称已存在", return false
  3. m_project.AddTask(name, duration)
     - duration==0 → Model 层自动创建 MilestoneBehavior
     - duration >0 → Model 层自动创建 NormalBehavior
  4. return true

ModifyTask(index, newName, newDuration, errorMsg):
  1. TaskId id = IndexToTaskId(index)，若无效 → errorMsg, return false
  2. Task* task = m_project.FindTask(id)  // non-const 重载
  3. 若 newName != task->GetName()：
     - IsTaskNameDuplicate(newName, excludeId=id) → 重复则拒绝
     - task->SetName(newName)
  4. 若 newDuration != task->GetDuration()：
     - task->SetDuration(newDuration)  // 内部自动切换策略
  5. return true
  注意：Task 对象在容器中位置不变，索引保持不变

RemoveDependency(index, errorMsg):
  1. 通过 GetDependencies()[index] 获取依赖，若越界 → errorMsg, return false
  2. TaskId pred = dep.GetPredecessorId()
  3. TaskId succ = dep.GetSuccessorId()
  4. m_project.RemoveDependency(pred, succ)
  5. return true

AddDependency(predIndex, succIndex, type, lag, errorMsg):
  1. TaskId predId = IndexToTaskId(predIndex)，若无效 → errorMsg
  2. TaskId succId = IndexToTaskId(succIndex)，若无效 → errorMsg
  3. 若 predId == succId → errorMsg="不能创建自引用依赖", return false
  4. 若 m_project.FindDependency(predId, succId) != nullptr
     → errorMsg="该依赖已存在", return false
  5. 若 m_validator.WouldCreateCycle(m_project, predId, succId)
     → errorMsg="添加此依赖会产生循环", return false
  6. m_project.AddDependency(predId, succId, type, lag)
  7. return true

AddResource(name, unitCost, errorMsg):
  1. name 为空 → errorMsg, return false
  2. IsResourceNameDuplicate(name) → errorMsg, return false
  3. m_project.AddResource(name, unitCost)
  4. return true

AssignResource(taskIndex, resourceIndex, quantity, errorMsg):
  1. TaskId taskId = IndexToTaskId(taskIndex)，若无效 → errorMsg
  2. ResourceId resId = IndexToResourceId(resourceIndex)，若无效 → errorMsg
  3. 若 quantity <= 0 → errorMsg="分配数量必须为正整数", return false
  4. const Task* task = m_project.FindTask(taskId)
  5. 若 task->CanAllocateResource() == false
     → errorMsg="里程碑任务不可分配资源", return false
  6. m_project.AssignResource(taskId, resId, quantity)
  7. return true
```

---

## 7. `ProjectDTOBuilder` — DTO 构建器

### 7.1 职责

将 Model 层原始数据（`const Project&`）转换为界面层所需的 DTO。
Controller 不应同时承担"协调"和"数据格式转换"——DTOBuilder 将转换逻辑从 Controller 剥离。

### 7.2 接口

```cpp
// 文件：service/ProjectDTOBuilder.hpp
// 职责：遍历 const Project&，组装界面层所需的全部 DTO
// 特点：无状态；所有方法为 const；一个实例可反复用于不同 Project

class ProjectDTOBuilder
{
  public:
    ProjectDTOBuilder()                               = default;
    ProjectDTOBuilder(const ProjectDTOBuilder&)       = default;
    ProjectDTOBuilder& operator=(const ProjectDTOBuilder&) = default;
    ~ProjectDTOBuilder()                              = default;

    // 需求 3.1.1：按容器索引顺序返回全部 TaskDTO
    std::vector<TaskDTO> BuildTaskDTOs(const Project& project) const;

    // 需求 3.1.4：返回指定任务的前驱与后继信息
    // first=前驱列表, second=后继列表
    std::pair<std::vector<TaskDTO>, std::vector<TaskDTO>>
        BuildTaskRelations(const Project& project, int index) const;

    // 需求 3.2.1：返回全部 DependencyDTO
    std::vector<DependencyDTO> BuildDependencyDTOs(const Project& project) const;

    // 需求 3.3.1：返回全部 ResourceDTO
    std::vector<ResourceDTO> BuildResourceDTOs(const Project& project) const;

    // 需求 4：返回统计信息。内部执行 Validate + 若通过则 CPM Calculate，
    //        一次调用返回全部统计指标，调用方无需关心顺序
    ProjectStatisticsDTO BuildStatistics(const Project& project,
                                         const ProjectValidator& validator,
                                         const CPMCalculator& calculator) const;

  private:
    // 构建单个 TaskDTO（供 BuildTaskDTOs 和 BuildTaskRelations 复用）
    TaskDTO BuildSingleTaskDTO(const Project& project,
                               int index, const Task& task) const;
};
```

### 7.3 实现要点

```
BuildTaskDTOs(project):
  1. 遍历 project.GetTasks()（下标即容器索引）
  2. 对每个 Task 调用 BuildSingleTaskDTO：
     - predecessorIndices：对 GetPredecessors(id) 的每个 ID 查 TaskIdToIndex
     - successorIndices：对 GetSuccessors(id) 的每个 ID 查 TaskIdToIndex
     - TaskIdToIndex 实现：遍历 GetTasks() 做 ID→下标映射

BuildTaskRelations(project, index):
  1. 通过 index 取 task（GetTasks()[index]），若越界返回空 pair
  2. 遍历 GetPredecessors(id)：为每个 pred 构建 TaskDTO（取 pred 对应的 index）
  3. 遍历 GetSuccessors(id)：同理
  4. 返回 {predecessorDTOs, successorDTOs}

BuildDependencyDTOs(project):
  1. 遍历 project.GetDependencies()，下标即序号
  2. 对每条 dep：predecessorIndex/successorIndex = TaskIdToIndex

BuildResourceDTOs(project):
  1. 遍历 project.GetResources()，下标即索引

BuildStatistics(project, validator, calculator):
  1. stats.taskCount       = project.TaskCount()
  2. stats.dependencyCount = project.DependencyCount()
  3. stats.resourceCount   = project.ResourceCount()
  4. ValidationResult vr = validator.Validate(project)
  5. stats.isValid = vr.IsValid()
  6. 若 isValid：stats.totalDuration = calculator.Calculate(project).GetTotalDuration()
     否则：stats.totalDuration = -1
  7. 返回 stats
```

---

## 8. `ProjectController` — 单例控制器

### 8.1 职责

`ProjectController` 是 Service 层的**唯一对外入口**，整个程序生命周期中**只能有 1 个实例**。

它的职责是**协调**——持有当前 Project 和全部 Service 组件，将界面层的请求路由到正确的处理者。

### 8.2 完整接口

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
    // 接受具体 Importer 实例（由 UI 层构造），导入成功则替换当前项目
    ImportResult ImportProject(IProjectImporter& importer);

    //------ 项目导出（需求 2）------
    // 接受具体 Exporter 实例（由 UI 层构造）
    bool ExportProject(IProjectExporter& exporter) const;

    //------ 项目状态 ------
    bool HasProject() const;
    const Project* GetProject() const;  // 只读访问（DTOBuilder 用）

    //------ 任务管理（需求 3.1）------
    std::vector<TaskDTO> ListTasks() const;
    bool RemoveTask(int index, std::string& errorMsg);
    bool AddTask(const std::string& name, int duration,
                 std::string& errorMsg);
    std::pair<std::vector<TaskDTO>, std::vector<TaskDTO>>
        GetTaskRelations(int index) const;
    bool ModifyTask(int index, const std::string& newName,
                    int newDuration, std::string& errorMsg);

    //------ 依赖管理（需求 3.2）------
    std::vector<DependencyDTO> ListDependencies() const;
    bool RemoveDependency(int index, std::string& errorMsg);
    bool AddDependency(int predIndex, int succIndex,
                       DependencyType type, int lag,
                       std::string& errorMsg);

    //------ 资源管理（需求 3.3）------
    std::vector<ResourceDTO> ListResources() const;
    bool AddResource(const std::string& name, double unitCost,
                     std::string& errorMsg);
    bool AssignResource(int taskIndex, int resourceIndex,
                        int quantity, std::string& errorMsg);

    //------ 统计信息（需求 4）------
    ProjectStatisticsDTO GetStatistics() const;

    //------ 验证与调度（需求 5）------
    ValidationResult Validate() const;
    ScheduleResult   ComputeSchedule() const;

  private:
    ProjectController();
    ~ProjectController() = default;

    // 按需创建 Editor（每次编辑操作时构造，绑定当前 Project 和 Validator）
    ProjectEditor CreateEditor();

    //------ 成员 ------
    std::unique_ptr<Project> m_project;
    ProjectValidator         m_validator;
    CPMCalculator            m_calculator;
    ProjectDTOBuilder        m_dtoBuilder;
};
```

### 8.3 各方法实现要点

Controller 的方法都是**薄层转发**——只做前置检查，然后委托给对应组件：

```
ImportProject(importer):
  1. ImportResult result = importer.Import()
  2. 若 result.HasProject()：m_project = result.ReleaseProject()
  3. return result

ExportProject(exporter):
  1. 若 !HasProject() → return false
  2. return exporter.Export(*m_project)

ListTasks():
  return m_dtoBuilder.BuildTaskDTOs(*m_project)

RemoveTask(index, errorMsg):
  若 !HasProject() → errorMsg, return false
  return CreateEditor().RemoveTask(index, errorMsg)

AddTask(name, duration, errorMsg):
  若 !HasProject() → errorMsg, return false
  return CreateEditor().AddTask(name, duration, errorMsg)

GetTaskRelations(index):
  若 !HasProject() → 返回空 pair
  return m_dtoBuilder.BuildTaskRelations(*m_project, index)

ModifyTask(index, newName, newDuration, errorMsg):
  若 !HasProject() → errorMsg, return false
  return CreateEditor().ModifyTask(index, newName, newDuration, errorMsg)

// ListDependencies / RemoveDependency / AddDependency 同理转发
// ListResources / AddResource / AssignResource 同理转发

GetStatistics():
  若 !HasProject() → 返回空统计（全部为 0, isValid=false, totalDuration=-1）
  return m_dtoBuilder.BuildStatistics(*m_project, m_validator, m_calculator)

Validate():
  若 !HasProject() → ValidationResult({"无项目可验证"})
  return m_validator.Validate(*m_project)

ComputeSchedule():
  若 !HasProject() → 返回空 ScheduleResult
  return m_calculator.Calculate(*m_project)
```

> **`CreateEditor()` 说明**：`ProjectEditor` 构造时绑定 `Project&` 和 `Validator&`。
> Controller 在每次编辑操作时调用 `CreateEditor()` 创建一个栈上的 Editor 实例——轻量、无堆分配、RAII 自动销毁。

---

## 9. 导入导出继承体系

### 9.1 现有接口（需微调）

**IProjectImporter** — 返回值改为 `ImportResult`：

```cpp
// 文件：service/IProjectImporter.hpp （修改：Import 返回类型）
class IProjectImporter
{
  public:
    virtual ~IProjectImporter() = default;
    virtual ImportResult Import() = 0;  // 原: unique_ptr<Project>
};
```

**IProjectExporter** — 保持不变：

```cpp
// 文件：service/IProjectExporter.hpp （不变）
class IProjectExporter
{
  public:
    virtual ~IProjectExporter() = default;
    virtual bool Export(const Project& project) = 0;
};
```

**ManualImporter** — 适配 `ImportResult` 返回类型：

```cpp
// 文件：service/ManualImporter.hpp （修改：Import 返回类型）
class ManualImporter : public IProjectImporter
{
  public:
    ImportResult Import() override;  // 原: unique_ptr<Project>
};
```

### 9.2 `PpmImporter`

```cpp
// 文件：service/PpmImporter.hpp + service/PpmImporter.cpp
// 职责：读取 PPM 格式文件，解析为 Project 对象
// 构造时绑定文件路径；Import() 返回 ImportResult（含 errors/warnings）

class PpmImporter : public IProjectImporter
{
  public:
    explicit PpmImporter(const std::string& filePath);
    ImportResult Import() override;

  private:
    std::string m_filePath;
};
```

**Import 实现要点**：

```
1. 打开文件，若失败 → ImportResult(nullptr, {"无法打开文件: ..."}, {})
2. 创建 Project，逐行解析（跳过空行、注释 '#', 行首空白）
3. 每行按空格分词：
   'P' → SetName
   'T' → AddTask(TaskId(id), name, duration)    // 显式 ID
   'M' → AddTask(TaskId(id), name, 0)
   'R' → AddResource(ResourceId(id), name, cost) // 显式 ID
   'D' → AddDependency(pred, succ, ParseType(s), lag)
   'A' → AssignResource(taskId, resId, qty)
4. 解析异常（行前缀非法、字段数不足、ID 非数字）→ errors.push_back
5. AddTask/AddResource 返回 Invalid（ID 重复）→ warnings.push_back，跳过该行
6. 返回 ImportResult(project, errors, warnings)

辅助函数：
  Trim(s)       — 去首尾空格
  Split(s)      — 按空格分词
  ParseDependencyType("FS") → DependencyType::FS 等
```

### 9.3 `PpmExporter`

```cpp
// 文件：service/PpmExporter.hpp + service/PpmExporter.cpp
// 职责：将 Project 对象写入 PPM 格式文件

class PpmExporter : public IProjectExporter
{
  public:
    explicit PpmExporter(const std::string& filePath);
    bool Export(const Project& project) override;

  private:
    std::string m_filePath;
};
```

**Export 实现要点**：

```
1. 创建/覆盖文件，若无法创建 → return false
2. 按 PPM 块顺序写入：
   - "# <ProjectName>" （注释行）
   - 'P' 行：project.GetName()
   - 'T' 行：GetTasks() 中 duration>0 者 → "T ID Name Duration"
   - 'M' 行：GetTasks() 中 duration==0 者 → "M ID Name 0"
   - 'R' 行：GetResources() → "R ID Name UnitCost"
   - 'D' 行：GetDependencies() → "D PredID SuccID Type Lag"
     DependencyType→字符串："FS"/"SS"/"FF"/"SF"
   - 'A' 行：GetAllocations() → "A TaskID ResourceID Quantity"
3. 关闭文件，return true
```

---

## 10. 类关系图

```text
                        View / UI
                           │
                           ▼
              ┌─────────────────────────┐
              │  ProjectController      │  ← Singleton
              │  (Singleton)            │
              │                         │
              │  + GetInstance()        │
              │  + ImportProject()      │
              │  + ExportProject()      │
              │  + ListTasks()          │
              │  + AddTask()            │
              │  + RemoveTask()         │
              │  + ModifyTask()         │
              │  + GetTaskRelations()   │
              │  + ListDependencies()   │
              │  + AddDependency()      │
              │  + RemoveDependency()   │
              │  + ListResources()      │
              │  + AddResource()        │
              │  + AssignResource()     │
              │  + GetStatistics()      │
              │  + Validate()           │
              │  + ComputeSchedule()    │
              └──────────┬──────────────┘
                         │ 持有
          ┌──────────────┼──────────────┬──────────────┐
          ▼              ▼              ▼              ▼
   ┌────────────┐ ┌────────────┐ ┌───────────┐ ┌─────────────┐
   │ unique_ptr │ │Project     │ │Project    │ │Project      │
   │ <Project>  │ │Validator   │ │DTOBuilder │ │Calculator   │
   │            │ │(stateless) │ │(stateless)│ │(stateless)  │
   └─────┬──────┘ └──────┬─────┘ └─────┬─────┘ └──────┬──────┘
         │               │             │              │
         │               │        只读 │              │ 只读
         ▼               ▼             ▼              ▼
   ┌──────────────────────────────────────────────────────┐
   │              const Project& (Model 层)               │
   └──────────────────────────────────────────────────────┘
         ▲
         │ 读写
   ┌─────┴──────┐
   │Project     │  ← 每次编辑操作栈上创建，绑定 Project& 和 Validator&
   │Editor      │
   │(stateless) │
   └────────────┘

   IProjectImporter         IProjectExporter
   (纯虚接口)               (纯虚接口)
        △                        △
        │                        │
   ┌────┴────┐             ┌────┴────┐
   │Ppm      │             │Ppm      │
   │Importer │             │Exporter │
   └─────────┘             └─────────┘

   ┌──────────────┐  ┌──────────────┐  ┌──────────────┐
   │ ImportResult │  │ Validation   │  │ TaskDTO      │
   │              │  │ Result       │  │ DependencyDTO│
   └──────────────┘  └──────────────┘  │ ResourceDTO  │
                                       │ ProjectStats │
                                       │    DTO       │
                                       └──────────────┘
```

---

## 11. 文件清单与实现顺序

### 11.1 新建文件

| # | 文件 | 说明 | Phase |
| :--- | :--- | :--- | :--- |
| 1 | `service/ValidationResult.hpp/.cpp` | 验证结果 DTO | 1 |
| 2 | `service/ImportResult.hpp/.cpp` | 导入结果 DTO | 1 |
| 3 | `service/TaskDTO.hpp` | 任务 DTO | 1 |
| 4 | `service/DependencyDTO.hpp` | 依赖 DTO | 1 |
| 5 | `service/ResourceDTO.hpp` | 资源 DTO | 1 |
| 6 | `service/ProjectStatisticsDTO.hpp` | 统计 DTO | 1 |
| 7 | `service/ProjectValidator.hpp/.cpp` | 验证器（含 WouldCreateCycle） | 2 |
| 8 | `service/CPMCalculator.hpp/.cpp` | 关键路径计算器 | 2 |
| 9 | `service/ProjectDTOBuilder.hpp/.cpp` | DTO 构建器 | 3 |
| 10 | `service/ProjectEditor.hpp/.cpp` | 项目编辑器 | 3 |
| 11 | `service/ProjectController.hpp/.cpp` | 单例控制器 | 4 |
| 12 | `service/PpmImporter.hpp/.cpp` | PPM 导入器 | 5 |
| 13 | `service/PpmExporter.hpp/.cpp` | PPM 导出器 | 5 |

### 11.2 需修改的已有文件

| 文件 | 修改内容 |
| :--- | :--- |
| `service/IProjectImporter.hpp` | `Import()` 返回类型改为 `ImportResult` |
| `service/ManualImporter.hpp` | `Import()` 返回类型改为 `ImportResult` |
| `service/ManualImporter.cpp` | 适配 `ImportResult` 返回 |

### 11.3 实现顺序

```
Phase 1：零依赖的 DTO（可并行）
  └── ValidationResult, ImportResult, TaskDTO, DependencyDTO,
      ResourceDTO, ProjectStatisticsDTO

Phase 2：纯算法类（依赖 Phase 1 DTO）
  ├── ProjectValidator（三条检查 + WouldCreateCycle）
  └── CPMCalculator（拓扑 + 前向 + 后向 + 关键路径）

Phase 3：构建器与编辑器（依赖 Phase 2）
  ├── ProjectDTOBuilder（遍历 Project → DTO，依赖 Validator + Calculator）
  └── ProjectEditor（修改 Project + 业务规则，依赖 Validator）

Phase 4：控制器（依赖 Phase 3）
  └── ProjectController（协调全部组件，薄层转发）

Phase 5：导入导出（依赖 Phase 1 ImportResult + Controller 可并行开发）
  ├── PpmImporter
  └── PpmExporter

Phase 6：联调验证
  └── ManualImporter → ProjectController → Validate → ComputeSchedule
      → 核对 22 天 + 关键路径 [1,2,3,4,5]
```

---

## 12. 设计决策记录

### 12.1 为什么 ProjectEditor 从 Controller 剥离

编辑操作不是简单的 Model setter 调用——`AddDependency` 包含参数校验、存在性检查、重复检查、环检测、Project 修改五个步骤，是独立的复合业务逻辑。若放在 Controller 内，Controller 将同时承担"请求协调"和"修改规则"两种职责。剥离后 Controller 只做转发，Editor 只做修改，各司其职。

### 12.2 为什么引入 ProjectDTOBuilder

遍历 Model → 查询关系 → ID/index 转换 → 组合 DTO 是一条独立的职责链，与"请求协调"无关。Controller 若承担数据转换，其职责变为协调+转换+UI 适配三者混合。DTOBuilder 作为无状态工具类（与 Validator 模式一致），可独立单测，且方便未来替换展示逻辑。

### 12.3 为什么 WouldCreateCycle 在 Validator 而非 Controller

该函数不依赖 Controller 状态、是纯图算法、只读 Project。它与 `CheckAcyclic` 共享同一套图遍历基础设施（BFS/DFS），放在 Validator 内比散落在 Controller 中更内聚。Editor 通过 `Validator&` 调用它。

### 12.4 为什么不拆 TaskEditor / DependencyEditor / ResourceEditor

`ProjectEditor` 当前总共 7 个公开方法，按实体拆分为三个类会引入 6 个文件和不必要的胶水代码。方法以命名前缀区分实体（`AddTask` / `AddDependency` / `AddResource`），清晰但不碎片化。这是**有意不拆**——若未来某类实体编辑规则膨胀到需要独立测试和维护，随时可提取而不影响 Controller 公开接口（Controller 持有的 Editor 类型变为组合即可）。

### 12.5 为什么不引入 IValidator / ImporterFactory

IValidator：当前只有一种验证规则（作业的 DAG + 悬挂 + 引用完整性）。引入接口是提前抽象——只有当多种验证体系同时存在或用户需动态选择时才需要。ImporterFactory：格式选择（文件扩展名→Importer）是 UI 层职责（文件对话框、命令行参数解析），不是 Service 层职责。Controller 接受 `IProjectImporter&`，UI 构造具体实例传入，各层边界清晰。

### 12.6 为什么 ImportResult 替代裸指针

`Import()` 失败只返回 `nullptr` 时，错误信息（文件不存在/格式错误/ID 重复/字段缺失）全部丢失。`ImportResult` 封装成功/失败/警告——Importer 负责填充，Controller 负责转发，UI 负责展示。每层只处理自己该处理的信息。

### 12.7 为什么 GetStatistics 内部执行 Validate + CPM

原设计要求调用方"先 Validate，再 ComputeSchedule，再 GetStatistics"，引入了隐式调用顺序依赖。现设计 `BuildStatistics` 内部自包含——一次调用完成统计，通过 `isValid` 字段告知项目状态。调用方无需关心内部顺序。

### 12.8 为什么 ProjectEditor 每次编辑操作栈上创建

`ProjectEditor` 构造时绑定 `Project&` 和 `Validator&`——它不持有所有权，不缓存状态，构造成本为零。每次编辑操作通过 `CreateEditor()` 在栈上创建一个临时实例，RAII 自动销毁。无需堆分配，无需存储在 Controller 成员中。这比持有一个长期存在的 Editor 对象更安全——每次编辑操作面对的都是最新的 Project 和 Validator 状态。

---

## 13. 验证基准

以 `ManualImporter` 构造的 ProjectDemo（6 任务、5 依赖、5 资源、7 分配）为基准：

| 验证项 | 预期结果 |
| :--- | :--- |
| `Validate().IsValid()` | `true`，错误列表为空 |
| `GetStatistics()` | taskCount=6, depCount=5, resCount=5, isValid=true, totalDuration=22 |
| `ComputeSchedule().GetTotalDuration()` | 22 |
| `ComputeSchedule().GetCriticalPath()` | `[TaskId(1), TaskId(2), TaskId(3), TaskId(4), TaskId(5)]` |
| 任务 1 ES/EF/LS/LF | 0 / 5 / 0 / 5 |
| 任务 5 ES/EF/LS/LF | 20 / 22 / 20 / 22 |
| 任务 6（里程碑）ES/EF/LS/LF | 17 / 17 / 22 / 22 |
