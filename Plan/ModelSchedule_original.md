# Model 层总规划 (ModelSchedule)

## 1. 定位与职责

Model 层是整个项目调度器的**领域模型**，仅包含可重用的核心业务对象。它必须：

- 完全独立于界面、控制器和具体文件格式，**仅使用 C++ 标准库**。
- 表达任务、依赖、资源及它们之间的关系，并维护必要的内部索引以支持高效查询。
- 不包含任何业务逻辑（如调度算法、项目修改、导入导出），只提供数据的**存储和只读访问**。
- 所有公开成员函数中，不改变对象状态的必须标注 `const`。

## 2. 核心类及其职责

### 2.1 `Task` (任务)

- **职责**：描述项目中需要完成的具体工作。
- **属性**：
  - `TaskId id`（唯一标识，由 Project 管理）
  - `std::string name`
  - `int duration`（天数，≥0）
- **行为**：
  - `bool canAllocateResource() const`  →  委托给内部的资源分配策略。
  - 提供 `id()`, `name()`, `duration()` 等 const 访问器。
  - `setName(const std::string&)` 修改名称。
  - `setDuration(int)` 修改工期，**且会自动切换内部的资源分配策略**（见 3.1）。
- **关键设计**：使用**策略模式**代替继承来区分里程碑和普通任务。`Task` 内部持有 `std::unique_ptr<IResourceAllocationPolicy>`，`setDuration()` 根据新工期创建对应的策略对象并替换，从而在保持 `Task` 对象内存地址（以及它在容器中的位置）不变的情况下，动态改变“是否允许分配资源”的行为。

### 2.2 `IResourceAllocationPolicy` (资源分配策略接口)

- **职责**：抽象“任务是否允许被分配资源”的判断。
- **接口**：

  ```cpp
  class IResourceAllocationPolicy {
  public:
      virtual ~IResourceAllocationPolicy() = default;
      virtual bool canAllocate() const = 0;
  };
  ```

- **实现子类**：
  - `NormalPolicy`：`canAllocate()` 返回 `true`。
  - `MilestonePolicy`：`canAllocate()` 返回 `false`。
- 这两个实现类本身也是可重用的领域概念，置于 Model 层。

### 2.3 `Dependency` (依赖)

- **职责**：描述两个任务之间的时序约束。
- **属性**：
  - `TaskId predecessorId`
  - `TaskId successorId`
  - `DependencyType type`（枚举：FS, SS, FF, SF）
  - `int lag`（整数，正为滞后，负为提前）
- **行为**：纯数据载体，可提供简单的 `type()`、`lag()` 等访问器。不存储指针，仅存储 TaskId，以解耦并保证容器重分配安全。

### 2.4 `Resource` (资源)

- **职责**：独立于任务存在的实体，可被任务占用。
- **属性**：
  - `ResourceId id`
  - `std::string name`
  - `double unitCost`（单位时间成本）
- **行为**：纯数据载体，提供访问器。

### 2.5 `ResourceAllocation` (资源分配)

- **职责**：记录某个任务对某种资源的占用数量（关联类）。
- **属性**：
  - `TaskId taskId`
  - `ResourceId resourceId`
  - `int quantity`（正整数）
- **行为**：纯数据载体。该类独立存在，不依附于 Task 或 Resource，由 Project 统一管理。

### 2.6 `Project` (项目聚合根)

- **职责**：管理一次调度模型中的所有 Task、Dependency、Resource 及其分配关系，并维护内部索引。
- **存储的集合**：
  - `std::vector<Task>` （或 `std::vector<std::unique_ptr<Task>>`，若需要多态则必须用指针，但采用策略模式后可按值存储 `Task`，推荐按值存储以简化内存管理）
  - `std::vector<Dependency>`
  - `std::vector<Resource>`
  - `std::vector<ResourceAllocation>`
- **内部索引**（完全私有，惰性构建或增量维护）：
  - `successors_`：`unordered_map<TaskId, vector<Dependency*>>` 或存储 DependencyId
  - `predecessors_`：同上
  - 该索引在 `Dependency` 集合变化时同步更新，由 Project 的**修改接口**维护。  
    *注：修改接口仅由 `ProjectEditor`（业务层）调用，Model 层的 `Project` 可提供有限制的 `addDependency` 等内部方法，并将它们设为 `private`，然后声明 `ProjectEditor` 为友元；或者另设包级可见的修改函数。为保持 Model 纯粹，强烈建议采用**友元**方式，让 Editor 可以直接操作内部集合并维护索引。*
- **公开查询接口**（均 `const`）：
  - 获取 Task、Dependency、Resource、Allocation 的总数和遍历器（返回 `const` 迭代器范围）
  - 根据 Id 查找 Task / Resource
  - 获取某 Task 的前驱列表、后继列表（基于内部索引高效实现）
  - 判断项目是否为空等。
- **不提供任何公开的修改方法**，所有修改必须通过 Editor 或导入器借助友元机制完成。

### 2.7 `ScheduleResult` (调度结果)

- **职责**：保存一次关键路径计算的完整结果，是纯数据载体。
- **包含数据**：
  - `int totalDuration` （项目总工期）
  - 每个任务的四个时间值：`ES`, `EF`, `LS`, `LF`
  - 关键路径上的任务 ID 列表（按拓扑顺序）
- **公开接口**（均 `const`）：
  - `int getTotalDuration() const`
  - `int getEarlyStart(TaskId) const`
  - `int getEarlyFinish(TaskId) const`
  - `int getLateStart(TaskId) const`
  - `int getLateFinish(TaskId) const`
  - `bool isCritical(TaskId) const`  （可通过比较 EF==LF 实时判断，或由计算器预置标志；从职责清晰出发，`ScheduleResult` 可仅存储原始时间，由外部通过比较函数判断。如果提供 `isCritical` 也是合理的便利方法，不违背数据载体原则）
  - `std::vector<TaskId> getCriticalPath() const`
- **构建方式**：由 `CPMCalculator` 直接填充数据，通过公开的构建方法或构造函数传入所有必要数据。不需要友元，也不需要使用 `unique_ptr` 返回，通常按值返回（依赖移动语义）。

### 2.8 `DependencyType` 枚举

```cpp
enum class DependencyType { FS, SS, FF, SF };
```

定义于全局命名空间或 Dependency 内部，Model 层的一部分。

## 3. 核心设计决策与动机

### 3.1 策略模式代替里程碑子类

- **问题**：作业要求“修改工期为0则变为里程碑，修改工期>0则变回普通任务”，且“索引不能变”。如果用 `BasicTask` 和 `MilestoneTask` 继承体系，修改工期时必须删除旧对象并创建新对象，会破坏容器索引。
- **方案**：`Task` 固定存在，内部使用 `IResourceAllocationPolicy` 接口。`setDuration(0)` 时将策略替换为 `MilestonePolicy`，`setDuration(>0)` 时替换为 `NormalPolicy`。对象在内存中的位置不发生改变，彻底满足要求。
- **优点**：符合“组合优于继承”，职责更单一，更易扩展（如未来增加“汇总任务”只需增加新策略）。

### 3.2 Dependency 与 ResourceAllocation 存储 ID 而非指针

- **原因**：
  - 避免因容器内存重分配（如 `vector` 扩容）导致的指针/引用失效。
  - 使模型更容易序列化。
  - 通过 `Project` 提供的查找方法间接获取对象引用，保持解耦。
- **代价**：每次访问需要一次查找，但 `unordered_map` 索引使得开销极小，且调度计算时通常一次加载全部关系。

### 3.3 Project 内部维护图索引

- **目的**：支持对前驱/后继的高效查询（O(1)），这对关键路径算法和依赖修改至关重要。
- **维护者**：仅由 `Project` 的私有修改方法（被 `ProjectEditor` 通过友元调用）负责更新索引，保证数据一致性。对外部来说，索引是不可见的。

### 3.4 纯数据 Project + 外部 Editor

- `Project` 不包含业务逻辑，仅提供只读数据访问和一个受控的修改窗口。
- 所有增删改操作封装在 `ProjectEditor`（业务层）中，它可细分为 `TaskEditor`、`DependencyEditor`、`ResourceEditor`，实现职责分离。
- 这符合 CQRS 思想（读写分离），也使 Model 层可以被任意数量的业务逻辑类安全操作，重用性极高。

### 3.5 Model 层绝不涉及任何界面和文件

- 不使用 `QString`、操作系统 API 或任何第三方库。
- 字符串全部使用 `std::string`，集合使用 STL 容器。
- 这使得 Model 层可以被命令行调度器、GUI 项目工具、单元测试等直接引用，无需任何修改。

## 4. 类关系图

```Markdown
Project (聚合根)
 ├── contains: vector<Task>
 ├── contains: vector<Dependency>
 ├── contains: vector<Resource>
 ├── contains: vector<ResourceAllocation>
 ├── maintains: predecessors_ / successors_ (private)
 └── provides: const accessors

Task
 └── owns: unique_ptr<IResourceAllocationPolicy>

IResourceAllocationPolicy <|.. NormalPolicy
IResourceAllocationPolicy <|.. MilestonePolicy

Dependency  -->  TaskId (predecessor, successor)
ResourceAllocation  -->  TaskId, ResourceId

ScheduleResult  (独立数据类，由 CPMCalculator 生成)
```

*注：箭头代表依赖或拥有关系，虚线三角为实现关系。*

## 5. 接口草案（仅示意关键部分）

```cpp
// Task.h
class Task {
public:
    Task(TaskId id, const std::string& name, int duration);
    TaskId id() const;
    const std::string& name() const;
    int duration() const;
    void setName(const std::string& newName);
    void setDuration(int newDuration);   // 内部切换策略
    bool canAllocateResource() const;    // 委托给策略
private:
    std::unique_ptr<IResourceAllocationPolicy> policy_;
    // ... 其他成员
};

// Project.h
class Project {
public:
    // 遍历器
    auto tasks() const { /* 返回 const 视图 */ }
    const Task* findTask(TaskId id) const;
    std::vector<TaskId> predecessors(TaskId id) const;  // 基于索引
    // ... 其他只读接口
private:
    friend class ProjectEditor;   // 允许 Editor 调用私有修改方法
    friend class PpmImporter;     // 允许导入器直接构建模型
    void addTaskInternal(Task task);
    void removeTaskInternal(TaskId id);  // 同时更新索引并级联删除依赖/分配
    // ... 其他内部修改方法
    std::vector<Task> tasks_;
    std::vector<Dependency> dependencies_;
    // ... 以及索引映射
};

// ScheduleResult.h
class ScheduleResult {
public:
    int totalDuration() const;
    int earlyStart(TaskId id) const;
    int earlyFinish(TaskId id) const;
    // ... 其他 getter
    std::vector<TaskId> criticalPath() const;
private:
    friend class CPMCalculator;  // 可选，若需要高效批量填充
    // 或者直接提供 set 方法，不设友元
    void setEarlyStart(TaskId, int es);
    // 内部存储映射
};
```

**注意**：该规划确保了 Model 层的纯净、可重用和高度封装，同时为上层业务逻辑（编辑器、验证器、调度器）提供了稳固的基石。后续开发所有上层功能时，均基于此 Model 定义进行扩展。
