# Service 层总规划 (ServiceSchedule) — 原始版

> 本文档为业务层（Service 层）设计的**初始草稿**，待设计审查后定稿（参照 Model 层的
> ModelSchedule_original.md → ModelScheduleAudit.md → ModelSchedule.md 流程）。

## 1. 定位与职责

业务层是调度器的**业务逻辑层**，位于 Model 层之上，仅使用 C++ 标准库与 Model 层公开接口。它必须：

- 完成 Model 层不承担的**复杂业务校验**（引用完整性、名称唯一性、DAG 无环等）。
- 完成**关键路径计算（CPM）**等调度算法，产出调度结果。
- 承担**格式导入/导出**（IProjectImporter / IProjectExporter 及其派生类）。
- 承担**受控编辑**（ProjectEditor，后续阶段）。
- 不依赖任何界面；数据一律以参数与返回值传递。

**分层原则**：Model 层只保证数据完整性，业务规则全部上移；业务层仅通过 Model 层公开接口访问数据，不直接触碰私有成员，不使用 friend（见 ModelSchedule.md §4.5）。

## 2. 设计前提与已有成果

- Model 层已提供：Project 聚合根（含 `GetPredecessors` / `GetSuccessors` 邻接索引、`FindTask` / `FindResource` 查找、显式/自动 ID 插入）、`Id<Tag>` 强类型 ID、`DependencyType`、`Task::CanAllocateResource`。
- 已完成导入抽象：[IProjectImporter](ModelSchedule.md 无关联，位于 business/)、IProjectExporter 纯虚接口与 ManualImporter 测试桩（业务层文件见 `business/` 目录）。
- ScheduleResult / TaskScheduleInfo 属 Model 层纯数据类（设计见 ModelSchedule.md §3.7 更新版），本层负责**填充与消费**。

## 3. 校验组件：ProjectValidator

### 3.1 ValidationErrorCode（错误码枚举）

```cpp
enum class ValidationErrorCode
{
    TASK_NAME_DUPLICATE,     // 任务名称重复
    RESOURCE_NAME_DUPLICATE, // 资源名称重复
    TASK_NOT_FOUND,          // 依赖或分配引用了不存在的任务
    RESOURCE_NOT_FOUND,      // 分配引用了不存在的资源
    SELF_DEPENDENCY,         // 依赖关系自引用（pred == succ）
    CYCLIC_DEPENDENCY,       // 任务依赖图存在环
    MILESTONE_ALLOCATED,     // 里程碑被分配了资源
    NEGATIVE_DURATION        // 任务工期为负数
};
```

> 设计说明：错误码供调用方（GUI / CLI / 测试）做程序化处理；人类可读的描述由 ValidationIssue 提供。

### 3.2 ValidationIssue（单条校验问题）

```cpp
class ValidationIssue
{
  public:
    ValidationIssue(ValidationErrorCode code, const std::string& message);

    ValidationErrorCode GetCode() const;
    const std::string& GetMessage() const;

  private:
    ValidationErrorCode m_code;
    std::string         m_message;
};
```

- 语义上必须携带 code 与 message 才有意义，因此**不提供默认构造函数**（与 Task 一致）。
- 纯数据载体，构造后不可变。

### 3.3 ValidationResult（校验结果）

```cpp
class ValidationResult
{
  public:
    ValidationResult() = default; // 空结果 = 项目有效
    explicit ValidationResult(const std::vector<ValidationIssue>& issues);

    bool IsValid() const;
    size_t GetIssueCount() const;
    const std::vector<ValidationIssue>& GetIssues() const;

  private:
    std::vector<ValidationIssue> m_issues;
};
```

- 由 ProjectValidator 收集全部问题后一次性构造，构造后不可变（与 ScheduleResult 由 CPMCalculator 填充的构造模式一致）。
- **收集全部问题而非首错即停**：便于调用方一次修复。

### 3.4 ProjectValidator（校验器）

```cpp
class ProjectValidator
{
  public:
    ProjectValidator()                              = default;
    ProjectValidator(const ProjectValidator&)       = default;
    ProjectValidator& operator=(const ProjectValidator&) = default;
    ~ProjectValidator()                             = default;

    ValidationResult Validate(const Project& project) const;

  private:
    void CheckNameUniqueness(const Project& project,
                             std::vector<ValidationIssue>& issues) const;
    void CheckDuration(const Project& project,
                       std::vector<ValidationIssue>& issues) const;
    void CheckReferenceIntegrity(const Project& project,
                                 std::vector<ValidationIssue>& issues) const;
    void CheckMilestoneAllocation(const Project& project,
                                  std::vector<ValidationIssue>& issues) const;
    void CheckAcyclic(const Project& project,
                      std::vector<ValidationIssue>& issues) const;
};
```

**检查项与实现要点**（`Validate` 依次调用全部私有检查，全部通过才算有效）：

1. **名称唯一性** `CheckNameUniqueness`：分别遍历任务与资源，用 `unordered_set<std::string>` 检测重复（任务名与资源名各自独立）。
2. **工期合法性** `CheckDuration`：任意任务 `GetDuration() < 0` → `NEGATIVE_DURATION`。
3. **引用完整性** `CheckReferenceIntegrity`：每条依赖的 pred / succ 必须存在（`FindTask`）；`pred == succ` → `SELF_DEPENDENCY`；每条分配引用的任务与资源必须存在（`FindTask` / `FindResource`）。
4. **里程碑分配** `CheckMilestoneAllocation`：任务 `CanAllocateResource() == false` 且存在分配记录 → `MILESTONE_ALLOCATED`。
5. **无环性** `CheckAcyclic`：Kahn 算法——基于 `GetPredecessors` / `GetSuccessors` 统计入度，反复移除入度为 0 的任务；若最终处理数 < 任务总数 → `CYCLIC_DEPENDENCY`。

> 设计说明：
> - 与 Model 层分工：Model 保证 ID 唯一、依赖不重复、级联清理；Validator 检查 Model 不承担的业务规则。
> - 无状态类；`Validate` 为 const 方法，一个实例可反复使用。

## 4. 计算组件：CPMCalculator

### 4.1 CPMCalculator（关键路径计算器）

```cpp
class CPMCalculator
{
  public:
    CPMCalculator()                          = default;
    CPMCalculator(const CPMCalculator&)      = default;
    CPMCalculator& operator=(const CPMCalculator&) = default;
    ~CPMCalculator()                         = default;

    ScheduleResult Calculate(const Project& project) const;
    bool IsCritical(const ScheduleResult& result, TaskId id) const;
};
```

- `Calculate`：**只读依赖** `const Project&`，按值返回 ScheduleResult（依赖移动语义）。**前置条件**：project 已通过 ProjectValidator 校验（无环、引用完整）；违反前置条件的行为未定义。
- `IsCritical`：业务层的关键性判断（EF == LF）。Model 层不提供该语义（见 ModelSchedule.md §3.7 设计说明）。

### 4.2 计算算法（CPM）

设任务 X 的 ES / EF / LS / LF 分别表示最早开始 / 最早完成 / 最晚开始 / 最晚完成，dur(X) 为工期。

**前向计算（按拓扑序）**：

- 无前驱的任务：ES = 0；否则对每条前驱依赖 (pred → X, type, lag) 求约束，取最大值：

  | 依赖类型 | 约束（对 X 的 ES） |
  | :--- | :--- |
  | FS | EF(pred) + lag |
  | SS | ES(pred) + lag |
  | FF | EF(pred) + lag − dur(X) |
  | SF | ES(pred) + lag − dur(X) |

- EF = ES + dur(X)
- **总工期** = 所有任务 EF 的最大值

**后向计算（按逆拓扑序）**：

- 无后继的任务：LF = 总工期；否则对每条后继依赖 (X → succ, type, lag) 求约束，取最小值：

  | 依赖类型 | 约束（对 X 的 LF） |
  | :--- | :--- |
  | FS | LS(succ) − lag |
  | SS | LS(succ) − lag |
  | FF | LF(succ) − lag |
  | SF | LF(succ) − lag |

- LS = LF − dur(X)

**关键路径**：满足 EF == LF 的任务集合，按 ES 升序（即拓扑序）排列。

> 公式来源：ImportFormat 文档第 3 节（FS/SS/FF/SF 定义与 Lag 公式）。
> 验证基准：样例 ProjectDemo 应得出 总工期 22 天、关键路径 1 → 2 → 3 → 4 → 5。

### 4.3 ScheduleResult 契约

ScheduleResult 属 Model 层纯数据类（完整设计见 ModelSchedule.md §3.7 更新版），本层契约：

- CPMCalculator 负责填充并**按值返回**（移动语义）。
- 消费方（UI、导出器）只读访问；`IsCritical` 由本层 CPMCalculator 提供。

## 5. 类关系图

```text
ProjectValidator ──依赖──→ const Project&（只读）
CPMCalculator   ──依赖──→ const Project&（只读）
CPMCalculator   ──产出──→ ScheduleResult（按值，移动语义）
ValidationResult ──包含──→ vector<ValidationIssue> ──包含──→ ValidationErrorCode

IProjectImporter / IProjectExporter（已实现）
 └─ ManualImporter（测试桩）

ProjectEditor（后续阶段）──依赖──→ Project + ProjectValidator
```

## 6. 待定问题与后续规划

**待定问题**（待审查讨论）：

1. 校验消息的语言（中文 / 英文）与格式（是否内嵌 TaskId 数值）。
2. 空项目（0 任务）Calculate 的结果定义：约定为 totalDuration = 0、空 criticalPath、空 data_。
3. 并行关键路径的 path 输出约定（按 ES 升序；任务间无依赖时相对顺序任意）。
4. IsCritical 前置条件：result 中必须已包含 id 对应任务的调度信息（由调用方保证）。
5. ManualImporter 与 ProjectValidator / CPMCalculator 的联调测试方式（临时 main 或正式测试框架）。

**后续规划**：

- ProjectEditor（依赖 Project + ProjectValidator，编辑操作的业务封装）。
- PpmImporter / PpmExporter（基于 IProjectImporter / IProjectExporter 实现）。
- 多项目支持与否待定（P 行名称目前存储于 Model 的 projectName）。
