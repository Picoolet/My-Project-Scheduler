# Model 层总规划 (ModelSchedule) — 最终版

## 1. 定位与职责

Model 层是整个项目调度器的**领域模型**，仅包含可重用的核心业务对象。它必须：

- 完全独立于界面、控制器和具体文件格式，**仅使用 C++ 标准库**。
- 表达任务、依赖、资源及它们之间的关系，并维护必要的内部索引以支持高效查询。
- 不包含复杂业务校验逻辑（如 DAG 无环验证），只提供数据的**存储、受控修改和只读访问**。复杂校验由上层 `ProjectValidator` 完成。
- 所有公开成员函数中，不改变对象状态的必须标注 `const`。

---

## 2. 基础设施： ID

### `Id<Tag>` 值对象模板

- **职责**：为不同实体提供编译期区分的 ID 类型，防止 `TaskId` 与 `ResourceId` 互相误传。
- **接口**：
  - `int value() const` — 获取原始数值（有效 ID 为非负整数，-1 为无效哨兵）。
  - `bool operator==(const Id&) const`
  - `bool operator!=(const Id&) const`
  - `static Id Invalid()` — 返回无效值（value = -1），用于表示"未找到"等哨兵语义。
  - `std::hash<Id<Tag>>` 特化 — 支持用作 `unordered_map` 的 key。

- **具体别名**：

  ```cpp
  using TaskId     = Id<struct TaskIdTag>;
  using ResourceId = Id<struct ResourceIdTag>;
  ```

---

## 3. 核心类及其职责

### 3.1 `ITaskBehavior` (任务行为接口)

- **职责**：抽象任务在资源分配等场景下的行为判断。
- **接口**：

  ```cpp
  class ITaskBehavior
  {
  public:
      virtual ~ITaskBehavior() = default;
      virtual bool CanAllocate() const = 0;

      // 静态工厂：根据工期创建对应行为策略
      // 前置条件：duration >= 0（由调用方保证）
      // duration == 0 → MilestoneBehavior
      // duration  > 0 → NormalBehavior
      static std::unique_ptr<ITaskBehavior> Create(int duration);
  };
  ```

- **实现子类**：
  - `NormalBehavior`：`CanAllocate()` 返回 `true`。
  - `MilestoneBehavior`：`CanAllocate()` 返回 `false`。
- 工厂方法将创建逻辑集中在接口上，`Task` 仅依赖抽象接口，遵守开闭原则。

### 3.2 `Task` (任务)

- **职责**：描述项目中需要完成的具体工作。
- **属性**：
  - `TaskId id`
  - `std::string name`
  - `int duration`（≥ 0，由调用方保证）
- **行为**：
  - `TaskId GetId() const`
  - `const std::string& GetName() const`
  - `int GetDuration() const`
  - `void SetName(const std::string& newName)`
  - `void SetDuration(int newDuration)` — 内部调用 `ITaskBehavior::Create()` 替换策略。
  - `bool CanAllocateResource() const` — 委托给内部的任务行为对象。
- **关键设计**：使用**策略模式**代替继承。`Task` 持有 `std::unique_ptr<ITaskBehavior>`，`SetDuration()` 通过静态工厂获取新策略并替换旧策略。对象在内存中的位置不发生改变，满足"修改工期索引不变"的约束。`Task` 不依赖任何具体策略类。

### 3.3 `Dependency` (依赖)

- **职责**：描述两个任务之间的时序约束。
- **属性**：
  - `TaskId predecessorId`
  - `TaskId successorId`
  - `DependencyType type`（`enum class DependencyType { FS, SS, FF, SF };`）
  - `int lag`（正数为滞后，负数为提前/超前）
- **行为**：纯数据载体，提供 `type()`、`lag()`、`predecessorId()`、`successorId()` 等 const 访问器。仅存储 TaskId，解耦并保证容器重分配安全。
- **前提（依赖上层保证）**：`predecessorId ≠ successorId`，且整个依赖图不构成环路。

### 3.4 `Resource` (资源)

- **职责**：独立于任务存在的实体，可被任务占用。
- **属性**：
  - `ResourceId id`
  - `std::string name`
  - `double unitCost`
- **行为**：纯数据载体，提供 const 访问器。

### 3.5 `Allocation` (资源分配)

- **职责**：记录某个任务对某种资源的占用数量。
- **属性**：
  - `TaskId taskId`
  - `ResourceId resourceId`
  - `int quantity`（> 0，由调用方保证）
- **行为**：纯数据载体。唯一性约束（同一 task + resource 只能有一条记录）由 `Project::assignResource` 的 upsert 语义保证。

### 3.6 `Project` (项目聚合根)

- **职责**：管理所有 Task、Dependency、Resource、Allocation 及其关系，维护内部索引，提供只读访问和**受控的公开修改接口**。
- **属性与存储的集合**：
  - `std::string projectName` — 项目名称
  - `std::vector<Task> tasks_`
  - `std::vector<Dependency> dependencies_`
  - `std::vector<Resource> resources_`
  - `std::vector<Allocation> allocations_`
- **内部索引**（完全私有，由修改方法增量维护）：
  - `std::unordered_map<TaskId, std::vector<TaskId>> successors_`
  - `std::unordered_map<TaskId, std::vector<TaskId>> predecessors_`
  - 索引存储 TaskId 而非指针，彻底消除 vector 扩容导致的悬空引用风险。
- **ID 生成**：内部维护自增计数器（`int` 类型，初值为 0），`addTask` 和 `addResource` 自动分配唯一 ID（从 1 开始递增）。
- **显式 ID 插入**：`AddTask(TaskId, name, duration)` / `AddResource(ResourceId, name, unitCost)` 重载支持导入场景下忠实保留文件中的显式 ID。语义：
  - 有效 ID 为非负整数（**0 合法**）；-1 是 `Id::Invalid()` 哨兵，显式插入 -1 时忽略本次插入。
  - 若 ID 已被占用则忽略本次插入，返回 `Id::Invalid()`（与 `addDependency` 跳过重复的语义一致，由调用方判定成功与否）。
  - 插入成功后同步自增计数器（取 `max(计数器, 显式 ID)`），保证后续自动生成的 ID 永不与显式 ID 冲突。

#### 公开只读接口（均 `const`）

- `const std::string& GetName() const`
- `size_t taskCount() const`、`size_t dependencyCount() const` 等
- `const Task* findTask(TaskId id) const`
- `const Resource* findResource(ResourceId id) const`
- `std::vector<TaskId> getPredecessors(TaskId id) const` — 基于索引 O(1) 查找
- `std::vector<TaskId> getSuccessors(TaskId id) const` — 基于索引 O(1) 查找
- `const Dependency* findDependency(TaskId pred, TaskId succ) const`
- 遍历器（返回 `const` 迭代器范围或 span）

#### 公开修改接口（受控）

- `void SetName(const std::string& newName)` — 修改项目名称。
- `TaskId addTask(const std::string& name, int duration)` — 创建 Task 并返回新生成的 TaskId。名称唯一性由调用方保证。
- `TaskId addTask(TaskId id, const std::string& name, int duration)` — 显式指定 ID 创建 Task（导入用），失败时返回 `Invalid`，语义见上文"显式 ID 插入"。
- `void removeTask(TaskId id)` — 级联删除与该 Task 关联的所有 Dependency 和 Allocation，并更新邻接索引。传入无效 ID 时静默忽略（或不执行任何操作）。
- `ResourceId addResource(const std::string& name, double unitCost)` — 创建 Resource 并返回新生成的 ResourceId。
- `ResourceId addResource(ResourceId id, const std::string& name, double unitCost)` — 显式指定 ID 创建 Resource（导入用），失败时返回 `Invalid`，语义见上文"显式 ID 插入"。
- `void addDependency(TaskId pred, TaskId succ, DependencyType type, int lag)` — 若相同的 (pred, succ) 组合已存在依赖则忽略（不重复添加），同时更新 `successors_` 和 `predecessors_` 索引。
- `void assignResource(TaskId task, ResourceId res, int quantity)` — 分配资源。若该 (task, res) 已存在分配记录则覆盖数量（upsert）；若 quantity ≤ 0 则视为取消分配（删除记录）。
- `void removeResource(ResourceId id)` — 级联删除关联的 Allocation。

> **设计说明**：级联删除放在 Project 内部，是因为删除一个实体后清理其关联记录属于**数据完整性**范畴（避免悬空引用），而非业务逻辑。这是聚合根的职责。

### 3.7 `ScheduleResult` (调度结果)

- **职责**：保存一次关键路径计算的完整结果，纯数据载体，不包含任何业务判断逻辑。
- **内部存储**：`std::unordered_map<TaskId, TaskScheduleInfo> data_`、`int totalDuration_` 和 `std::vector<TaskId> criticalPath_`（关键路径上的任务 ID，按拓扑序）。
- **`TaskScheduleInfo` 结构**：

  ```cpp
  struct TaskScheduleInfo
  {
      int earlyStart;   // ES
      int earlyFinish;  // EF
      int lateStart;    // LS
      int lateFinish;   // LF
  };
  ```

- **公开接口**（均 `const`）：
  - `int GetTotalDuration() const`
  - `int GetEarlyStart(TaskId) const`
  - `int GetEarlyFinish(TaskId) const`
  - `int GetLateStart(TaskId) const`
  - `int GetLateFinish(TaskId) const`
  - `std::vector<TaskId> GetCriticalPath() const` — 返回存储的关键路径任务 ID 列表（按拓扑顺序排列，由 CPMCalculator 计算后存入）。
- **构建方式**：构造函数接受 `std::unordered_map<TaskId, TaskScheduleInfo>`、`int totalDuration` 与 `std::vector<TaskId> criticalPath` 三项，由业务层 `CPMCalculator` 填充后按值返回（依赖移动语义，见 ServiceSchedule_original.md §4）。

> **设计说明**：`IsCritical(TaskId)` 不在 Model 层提供。Model 层只提供原始时间数据，关键性判断（EF == LF）属于业务语义，由业务层 `CPMCalculator::IsCritical` 实现（见 ServiceSchedule_original.md §4.1）。

### 3.8 `DependencyType` 枚举

```cpp
enum class DependencyType { FS, SS, FF, SF };
```

定义于 Model 层，可放在独立的 `DependencyType.hpp` 或 `Dependency.hpp` 中。

---

## 4. 核心设计决策与动机

### 4.1 策略模式代替里程碑子类

- **问题**：作业要求"修改工期为 0 则变为里程碑，修改工期 > 0 则变回普通任务"，且"索引不能变"。如果用 `BasicTask` 和 `MilestoneTask` 继承体系，修改工期时必须删除旧对象并创建新对象，会破坏容器索引。
- **方案**：`Task` 固定存在，内部使用 `ITaskBehavior` 接口。`SetDuration(0)` 时通过工厂创建 `MilestoneBehavior`，`SetDuration(>0)` 时创建 `NormalBehavior`。对象在内存中的位置不发生改变，彻底满足要求。
- **优点**：符合"组合优于继承"，职责更单一，更易扩展。

### 4.2 静态工厂置于接口

- `ITaskBehavior::Create(int duration)` 将策略创建逻辑与接口放在一起。`Task` 仅依赖抽象接口，不感知具体策略类，遵守开闭原则。若未来策略种类增多，可单独抽出 `TaskBehaviorFactory` 类。

### 4.3 Dependency 与 Allocation 存储 ID 而非指针

- **原因**：
  - 避免因容器内存重分配（如 `vector` 扩容）导致的指针/引用失效。
  - 使模型更容易序列化。
  - 通过 `Project` 提供的查找方法间接获取对象引用，保持解耦。
- **代价**：每次访问需要一次查找，但 `unordered_map` 索引使得开销极小。

### 4.4 Project 内部维护图索引

- **目的**：支持对前驱/后继的高效查询（O(1)），这对关键路径算法和依赖修改至关重要。
- **维护者**：由 `Project` 的公开修改方法（`addDependency`、`removeTask`）负责同步更新索引。索引存储 `TaskId`，无悬空风险。
- **对外部透明**：外部调用方无需关心索引的存在。

### 4.5 公开受控修改接口（无友元）

- `Project` 的修改方法全部为 `public`，`ProjectEditor` 作为普通调用方使用这些接口。这消除了旧设计中 `friend` 带来的反向依赖——现在是 Editor 依赖 Project，而非 Project 知道 Editor。
- Model 层可以被任意数量的业务逻辑类安全操作，重用性极高。

### 4.6 Model 层不做复杂业务校验

- 以下校验由上层 `ProjectValidator` + `ProjectEditor` 负责：
  - DAG 无环验证
  - 任务名称唯一性
  - 依赖引用完整性（predecessor/successor 对应的 Task 存在）
- Model 层仅保证**数据完整性**：ID 唯一（自动生成与显式插入均保证不冲突）、索引与数据同步、级联清理避免悬空引用。

### 4.7 Model 层绝不涉及任何界面和文件

- 不使用 `QString`、操作系统 API 或任何第三方库。
- 字符串全部使用 `std::string`，集合使用 STL 容器。
- 这使得 Model 层可以被命令行调度器、GUI 项目工具、单元测试等直接引用，无需任何修改。

---

## 5. 类关系图

```text
Project (聚合根)
 ├── contains: vector<Task>
 ├── contains: vector<Dependency>
 ├── contains: vector<Resource>
 ├── contains: vector<Allocation>
 ├── maintains: predecessors_ / successors_ (private, unordered_map<TaskId, vector<TaskId>>)
 └── provides: const accessors + controlled public mutators

Task
 └── owns: unique_ptr<ITaskBehavior>

ITaskBehavior <|.. NormalBehavior
ITaskBehavior <|.. MilestoneBehavior
ITaskBehavior : static Create(duration) → unique_ptr<ITaskBehavior>

Id<TaskIdTag>     → alias TaskId
Id<ResourceIdTag> → alias ResourceId

Dependency       -->  TaskId (predecessor, successor)
Allocation       -->  TaskId, ResourceId

ScheduleResult  (独立数据类，由 CPMCalculator 构造并填充，按值返回)
```

---

## 6. 上层依赖关系

```text
ProjectEditor   ──依赖──→ Project (公开修改 + 只读接口)
ProjectEditor   ──依赖──→ ProjectValidator
ProjectValidator──依赖──→ const Project& (只读)
CPMCalculator   ──依赖──→ const Project& (只读)
CPMCalculator   ──产出──→ ScheduleResult
```

无反向依赖（Model 层不依赖任何上层类）。
