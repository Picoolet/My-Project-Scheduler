# View 层设计方案

## 1. 定位与职责

View 层是 MVC 架构中的界面层，直接与用户交互。基于 **已完成并验证的 Service 层接口** 设计，职责如下：

- **交互入口**：接收用户所有操作指令并完成基础格式校验（数字合法性、范围检查、空名称拒绝）。
- **数据呈现**：将 `ProjectController` 返回的 DTO / 结果对象格式化为人类可读的表格和提示。
- **零业务逻辑**：**绝对不包含**任何业务逻辑（DAG 验证、CPM 计算、名称唯一性检查等），所有功能性请求委托给 `ProjectController`。
- **零 Model 直接依赖**：除 `ScheduleResult` 和 `ValidationResult`（纯数据载体）外，不对 Model 层对象直接操作。

---

## 2. 需求覆盖表

| View_Goal 需求 | ConsoleView 方法 | 依赖的 Controller 接口 | Formatter |
| :--- | :--- | :--- | :--- |
| 2.1 导入项目 | `ImportProject()` | `ImportProject(filePath, errorMsg)` | —（直接输出结果） |
| 2.1 导出项目 | `ExportProject()` | `ExportProject(filePath, errorMsg)` | — |
| 2.2 列出所有任务 | `ListTasks()` | `ListTasks()` | `TaskListFormatter` |
| 2.2 删除任务 | `RemoveTask()` | `RemoveTask(index, errorMsg)` | — |
| 2.2 添加任务 | `AddTask()` | `AddTask(name, duration, errorMsg)` | — |
| 2.2 查询前驱/后继 | `ShowTaskRelations()` | `GetTaskRelations(index)` | `TaskRelationsFormatter` |
| 2.2 修改任务 | `ModifyTask()` | `ModifyTask(index, name, dur, errorMsg)` | — |
| 2.3 列出所有依赖 | `ListDependencies()` | `ListDependencies()` | `DependencyListFormatter` |
| 2.3 删除依赖 | `RemoveDependency()` | `RemoveDependency(index, errorMsg)` | — |
| 2.3 添加依赖 | `AddDependency()` | `AddDependency(pred, succ, type, lag, errorMsg)` | — |
| 2.4 列出所有资源 | `ListResources()` | `ListResources()` | `ResourceListFormatter` |
| 2.4 添加资源 | `AddResource()` | `AddResource(name, cost, errorMsg)` | — |
| 2.4 分配资源 | `AssignResource()` | `AssignResource(taskIdx, resIdx, qty, errorMsg)` | — |
| 2.5 显示统计 | `ShowStatistics()` | `GetStatistics()` | `StatisticsFormatter` |
| 2.6 项目验证 | `ValidateProject()` | `Validate()` | `ValidationResultFormatter` |
| 2.6 调度计算 | `ComputeSchedule()` | `ComputeSchedule()` | `ScheduleResultFormatter` |

---

## 3. 核心设计理念

### 3.1 可注入的输入输出流

```cpp
class ConsoleView {
public:
    ConsoleView(std::istream& in, std::ostream& out);
    void Run();
private:
    std::istream& m_in;
    std::ostream& m_out;
};
```

- **生产环境**：`ConsoleView(std::cin, std::cout)`
- **单元测试**：注入 `std::istringstream` + `std::ostringstream`，验证完整交互流程，无需模拟标准输入输出。
- **可重用性**：未来可替换为网络流、文件流或 GUI 的事件流适配器。

### 3.2 Lambda 映射命令分发

采用 `std::map<int, std::function<void()>>` 存储菜单项到私有方法的映射：

```cpp
m_mainActions[1] = [this]() { ImportProject(); };
m_mainActions[2] = [this]() { ExportProject(); };
```

- 每个菜单项映射到一个私有成员函数，函数内部读取参数 → 调用 Controller → 输出。
- 满足"除 lambda 外不允许非成员函数"的规则要求。
- 避免大量 Command 子类导致的文件膨胀。

**选用理由**（对照）：

| 方案 | 优点 | 缺点 | 本次 |
| :--- | ---: | :--- | :---: |
| 独立 Command 类 | OCP 完美 | 文件数暴增，每类逻辑极薄 | 不采用 |
| switch-case | 零开销 | 分支膨胀，难维护 | 不采用 |
| **lambda 映射** | 结构清晰，易扩展 | 需理解 `std::function` | **采用** |

### 3.3 Formatter 工具类集

Formatter 是**无状态的工具类**，全部提供**静态方法**，输入 DTO 或数据对象的 const 引用，输出 `std::string`。它们不依赖控制台特性，可被未来的 GUI 界面直接复用（替换为 Qt 的 Model/View 适配器即可）。

---

## 4. 主界面类：`ConsoleView`

### 4.1 类声明

```cpp
// view/ConsoleView.hpp
class ConsoleView {
public:
    ConsoleView(std::istream& in, std::ostream& out);
    void Run();

private:
    //------ 菜单 ------
    void ShowMainMenu();
    void HandleMainChoice(int choice);
    void ShowTaskMenu();
    void ShowDependencyMenu();
    void ShowResourceMenu();
    void ShowAnalysisMenu();

    //------ 项目操作 ------
    void ImportProject();
    void ExportProject();

    //------ 任务管理 ------
    void ListTasks();
    void RemoveTask();
    void AddTask();
    void ShowTaskRelations();
    void ModifyTask();

    //------ 依赖管理 ------
    void ListDependencies();
    void RemoveDependency();
    void AddDependency();

    //------ 资源管理 ------
    void ListResources();
    void AddResource();
    void AssignResource();

    //------ 分析与调度 ------
    void ShowStatistics();
    void ValidateProject();
    void ComputeSchedule();

    //------ 输入辅助 ------
    int         ReadInt(const std::string& prompt);
    std::string ReadLine(const std::string& prompt);
    double      ReadDouble(const std::string& prompt);
    bool        Confirm(const std::string& prompt);

    //------ 输出辅助 ------
    void PrintSuccess(const std::string& message);
    void PrintError(const std::string& message);
    void PrintWarning(const std::string& message);
    void PrintHeader(const std::string& title);

    //------ 菜单映射表 ------
    std::map<int, std::function<void()>> m_mainActions;
    std::map<int, std::function<void()>> m_taskActions;
    std::map<int, std::function<void()>> m_depActions;
    std::map<int, std::function<void()>> m_resActions;
    std::map<int, std::function<void()>> m_analysisActions;

    std::istream& m_in;
    std::ostream& m_out;
};
```

### 4.2 菜单结构树

```
═══════════════════════════════════════════
  项目管理器 (Project Scheduler)
  当前项目: [ProjectDemo]                  ← 导入后显示项目名，未导入显示 "无"
═══════════════════════════════════════════

主菜单
├── 1. 项目管理
│   ├── 1. 导入项目 (PPM 文件)
│   └── 2. 导出项目 (PPM 文件)
├── 2. 任务管理
│   ├── 1. 列出所有任务
│   ├── 2. 添加新任务
│   ├── 3. 修改任务
│   ├── 4. 删除任务
│   └── 5. 查看任务前后继
├── 3. 依赖管理
│   ├── 1. 列出所有依赖
│   ├── 2. 添加依赖
│   └── 3. 删除依赖
├── 4. 资源管理
│   ├── 1. 列出所有资源
│   ├── 2. 添加资源
│   └── 3. 分配资源给任务
├── 5. 分析与调度
│   ├── 1. 显示统计信息
│   ├── 2. 验证项目合理性
│   └── 3. 计算关键路径
└── 0. 退出
```

每个子菜单输入 `0` 返回上级。菜单标题栏动态显示当前项目名。

### 4.3 输入辅助方法

```cpp
// 读取整数，含重试机制（最多 3 次），返回 -1 表示用户放弃
int ReadInt(const std::string& prompt);

// 读取一行字符串，自动去除首尾空格，不可为空（返回空串表示放弃）
std::string ReadLine(const std::string& prompt);

// 读取浮点数，含重试和范围校验
double ReadDouble(const std::string& prompt);

// 确认操作（Y/N），默认返回 false
bool Confirm(const std::string& prompt);
```

- `ReadInt` 和 `ReadDouble` 内部循环直至合法或用户主动放弃（输入 `q` 或重试 3 次失败）。
- `ReadLine` 去除首尾空格后若为空 → 提示并重试。

### 4.4 输出辅助方法

```cpp
void PrintSuccess(const std::string& message); // 输出 "  [OK] message"
void PrintError(const std::string& message);   // 输出 "  [FAIL] message"
void PrintWarning(const std::string& message); // 输出 "  [WARN] message"
void PrintHeader(const std::string& title);    // 输出带分隔线的标题
```

统一操作反馈格式，用户一眼可识别操作结果。不依赖终端颜色（纯文本结构），跨平台兼容。

---

## 5. 用户交互设计（友好性增强）

### 5.1 空项目保护

每个需要项目存在的操作前，统一检查 `ProjectController::GetInstance().HasProject()`：

```cpp
if (controller.HasProject() == false) {
    PrintWarning("当前无项目，请先导入 PPM 文件或手动创建任务。");
    return;
}
```

### 5.2 索引转换（1-based ↔ 0-based）

用户输入和显示均使用 **1-based 序号**（人类直觉），内部传递给 Controller 时转换为 **0-based**：

```cpp
int userIndex = ReadInt("请输入序号（1, 2, 3...）: ");
int internalIndex = userIndex - 1; // 转换
// 调用 Controller 使用 internalIndex
```

- 所有列表显示时，序号列从 1 开始（index + 1）。
- 若输入序号 ≤ 0 → 提示"序号必须为正整数"，允许重试。

### 5.3 危险操作确认

删除任务时：

1. 先调用 `ListTasks()` 确认待删任务存在
2. 显示任务的关键信息（名称、工期）
3. 提示"删除该任务将同时删除其所有关联依赖和资源分配，确认？(Y/N)"

删除依赖时同理，显示依赖的关键信息。

### 5.4 修改操作的现有值回显

修改任务时：

1. 先显示当前任务名称和工期
2. 提示输入新名称（直接回车 = 保持原名不变）
3. 提示输入新工期（直接回车/输入 `-1` = 保持原工期不变）

避免用户每次都要重新输入两个字段。

### 5.5 表格列宽自适应策略

- 各列最小宽度：
  - 序号：4 字符
  - 名称：最长任务名 + 2
  - 工期：6 字符
  - 类型：4 字符
  - 成本：10 字符（含小数）
- 超出终端预期宽度的名称截断为 30 字符 + `...`
- 分隔线长度取实际内容宽度

### 5.6 依赖类型友好输入

用户输入 `FS`/`SS`/`FF`/`SF` 时：

- 不区分大小写（内部 `toupper` 转换）
- 输入非法类型时列出可选值并要求重试

### 5.7 操作反馈示例

```
  [OK] 任务 "Design" 添加成功（自动创建为普通任务）。
  [OK] 任务 "Acceptance" 添加成功（工期=0，自动创建为里程碑）。
  [FAIL] 添加任务失败：名称 "Design" 已存在。
  [FAIL] 添加依赖失败：该依赖将导致循环（1 → 2 → 4 → 1）。
  [WARN] 项目已导入，但有 2 条警告（详见上方信息）。
```

---

## 6. Formatter 类集设计

### 6.1 类列表与接口

| Formatter 类 | 输入类型 | 输出格式 |
| :--- | :--- | :--- |
| `TaskListFormatter` | `std::vector<TaskDTO>` | 五列表格：序号 / 名称 / 工期 / 前驱列表 / 后继列表 |
| `TaskRelationsFormatter` | `std::pair<std::vector<TaskDTO>, std::vector<TaskDTO>>` | 分两区：前驱列表 + 后继列表（序号 + 名称） |
| `DependencyListFormatter` | `std::vector<DependencyDTO>` | 五列表格：序号 / 前置序号 / 后置序号 / 类型 / Lag |
| `ResourceListFormatter` | `std::vector<ResourceDTO>` | 三列表格：序号 / 名称 / 单位成本 |
| `StatisticsFormatter` | `ProjectStatisticsDTO` | 五条字段：任务数 / 依赖数 / 资源数 / 项目状态 / 总工期 |
| `ValidationResultFormatter` | `ValidationResult` | 通过：`[OK]` 提示；不通过：逐条列出错误信息 |
| `ScheduleResultFormatter` | `ScheduleResult` | 总工期 + 每任务时间参数表 (ES/EF/LS/LF) + 关键路径列表 |

**注意**：`ImportResult` 在 View 层不可见 — `ProjectController::ImportProject` 内部消化 `ImportResult`，仅暴露 `bool + errorMsg`。因此不设 `ImportResultFormatter`。

### 6.2 接口示例

```cpp
// view/TaskListFormatter.hpp
class TaskListFormatter {
public:
    static std::string Format(const std::vector<TaskDTO>& tasks);
};

// view/DependencyListFormatter.hpp
class DependencyListFormatter {
public:
    static std::string Format(const std::vector<DependencyDTO>& dependencies);
};
```

所有 Formatter 均为纯静态方法类，不允许实例化。这是比"默认构造 + 普通方法"更彻底的"无状态"表达。

### 6.3 关键转换

#### DependencyType → 字符串

统一在 `DependencyListFormatter` 内部完成，不另设工具函数（避免过度设计）：

```cpp
// 内部辅助函数（静态自由函数，.cpp 文件内 anonymous namespace）
namespace {
    std::string DependencyTypeToString(DependencyType type)
    {
        switch (type)
        {
        case DependencyType::FS: return "FS";
        case DependencyType::SS: return "SS";
        case DependencyType::FF: return "FF";
        case DependencyType::SF: return "SF";
        default:                 return "??";
        }
    }
}
```

#### 前驱/后继索引格式化

`TaskListFormatter` 中 `predecessorIndices` / `successorIndices` 以 `[1,3,5]` 格式显示（区间短于 3 个则展开，长于 3 个则打省略）。

#### 关键路径格式化

`ScheduleResultFormatter` 格式化关键路径为 `1 → 2 → 3 → 4 → 5`（TaskId 的 value），同时标注哪些任务在关键路径上（时间参数表中用 `*` 标记）。

---

## 7. 代码测试设计（新增）

### 7.1 测试基础设施

利用流注入特性，所有测试无需修改被测代码：

```cpp
// 测试示例框架
class ViewTestFixture {
protected:
    std::istringstream m_testInput;   // 模拟用户输入
    std::ostringstream m_testOutput;  // 捕获 View 输出
    ConsoleView        m_view;        // 注入测试流

    ViewTestFixture(const std::string& inputScript)
        : m_testInput(inputScript)
        , m_view(m_testInput, m_testOutput)
    {
    }

    // 断言输出包含指定文本
    bool OutputContains(const std::string& text) const;
    // 获取完整输出
    std::string GetOutput() const;
};
```

### 7.2 Formatter 单元测试

每类 Formatter 对应一个测试源文件 `Test/<Name>Formatter_test.cpp`：

#### TaskListFormatter 测试

| 测试用例 | 输入 | 预期输出特征 |
| :--- | :--- | :--- |
| 空任务列表 | `{}` | 输出含 "No tasks" 或空表格 |
| 单个普通任务 | `[{0, 1, "Design", 3, {}, {}}]` | 输出含 "Design" / "3" / "普通" |
| 里程碑任务 | `[{0, 1, "Accept", 0, {}, {}}]` | 输出含 "Accept" / "0 (里程碑)" |
| 含前后继的任务 | 6-task Demo 数据 | 输出含完整前驱/后继列表 `[2]` / `[3,6]` |
| 长任务名 | 名称 40 字符 | 名称被截断为 30+`...` |

#### TaskRelationsFormatter 测试

| 测试用例 | 预期 |
| :--- | :--- |
| 空 pair（无前后继） | "No predecessors" + "No successors" |
| 有前后继的任务 | 分别列出序号和名称 |

#### DependencyListFormatter 测试

| 测试用例 | 预期 |
| :--- | :--- |
| 空列表 | "No dependencies" |
| 各类型依赖（FS/SS/FF/SF） | 类型列正确显示 FS/SS/FF/SF |
| 正/负/零 Lag | Lag 列显示对应数值 |

#### ResourceListFormatter 测试

| 测试用例 | 预期 |
| :--- | :--- |
| 空列表 | "No resources" |
| 含成本资源 | 成本显示为 "100.00"（固定 2 位小数） |

#### StatisticsFormatter 测试

| 测试用例 | 输入 | 预期 |
| :--- | :--- | :--- |
| 有效项目 | `{6, 5, 5, true, 22}` | 显示全部统计 + "总工期: 22 天" |
| 无效项目 | `{0, 0, 0, false, -1}` | 显示 "项目状态: 未通过验证" + "总工期: N/A" |

#### ValidationResultFormatter 测试

| 测试用例 | 输入 | 预期 |
| :--- | :--- | :--- |
| 通过 | 空 errors | `[OK] 项目验证通过，无错误。` |
| 有 1 个错误 | `{"Cyclic dependency..."}` | 逐条列出错误 |
| 有多个错误 | 3 条错误 | 逐条列出，编号清晰 |

#### ScheduleResultFormatter 测试

| 测试用例 | 输入 | 预期 |
| :--- | :--- | :--- |
| 空结果 | totalDuration=0, 空 criticalPath | "总工期: 0 天" + "无关键路径（项目可能为空或存在环路）" |
| 完整结果 | ProjectDemo CPM 结果 (22天) | 表格中各任务 ES/EF/LS/LF 正确，关键路径标记 `*` |

> **关键验证数据（ProjectDemo）**：`ManualImporter` 构建的 6-task 项目 CPM 结果：
>
> - totalDuration = 22
> - criticalPath = `[TaskId(1), TaskId(2), TaskId(3), TaskId(4), TaskId(5)]`
> - Task 1 (Requirement): ES=0, EF=5, LS=0, LF=5
> - Task 6 (Acceptance, 里程碑): ES=17, EF=17, LS=22, LF=22

### 7.3 ConsoleView 集成测试

通过注入 `istringstream` 模拟完整用户操作序列，验证交互流程的正确性。

#### 测试场景清单

| # | 场景 | 输入序列 | 验证点 |
| :--- | :--- | :--- | :--- |
| 1 | 无项目时列出任务 | `2` `1` `0` `0` | 输出含 "无项目，请先导入" |
| 2 | 退出程序 | `0` | 程序正常退出，输出含 "Goodbye" |
| 3 | 无效菜单选项 | `99` `0` | 提示 "无效选项"，允许重新输入 |
| 4 | 添加任务流程 | `2` `2` `NewTask` `5` `0` `0` `0` | 输出含 `[OK] 任务 "NewTask" 添加成功` |
| 5 | 添加重名任务 | `ManualImport→2→2→Requirement→5→0→0→0` | 输出含 `[FAIL]` 和 "已存在" |
| 6 | 删除任务确认 | `2` `4` `1` `N` `0` `0` `0` | 拒绝时任务未被删除 |
| 7 | 验证通过 | `ManualImport→5→2` `0` `0` | 输出含 `[OK] 项目验证通过` |
| 8 | 调度计算 | `ManualImport→5→3` `0` `0` | 输出含 "总工期: 22 天" + 关键路径 |
| 9 | 统计信息 | `ManualImport→5→1` `0` `0` | 输出含 taskCount=6, depCount=5, resCount=5 |
| 10 | 导入非法文件 | `1` `1` `/nonexistent.ppm` `1` `1` `0` | 输出含 `[FAIL]` + 文件错误信息 |
| 11 | 列出依赖 | `ManualImport→3→1` `0` `0` `0` | 表格含全部 5 条依赖 |
| 12 | 列出资源 | `ManualImport→4→1` `0` `0` `0` | 表格含全部 5 个资源 |
| 13 | 添加依赖（成环拒绝） | `ManualImport→3→2→3→1→FS→0→0→0→0` | 输出含 "循环" 拒绝提示 |
| 14 | 修改任务 | `ManualImport→2→3→1→NewName→10→0→0→0` | 输出含修改成功确认 |
| 15 | 非数字输入 | `abc`（在期望整数处） | 提示 "请输入有效数字"，允许重试 |

> 注：`ManualImport→` 表示先通过 `ManualImporter` 构造测试 Project 并导入（测试辅助，在测试套件中直接调 `Controller::ImportProject` 的桩路径或注入预构建 Project）。

### 7.4 测试文件清单

```
Test/
├── TaskListFormatter_test.cpp
├── TaskRelationsFormatter_test.cpp
├── DependencyListFormatter_test.cpp
├── ResourceListFormatter_test.cpp
├── StatisticsFormatter_test.cpp
├── ValidationResultFormatter_test.cpp
├── ScheduleResultFormatter_test.cpp
└── ConsoleView_integration_test.cpp
```

- 每个 Formatter 测试源文件 ≤ 200 行（含注释）。
- 集成测试按上述 15 个场景组织为 15 个独立的 `TEST_CASE`（或按习惯使用宏展开的测试函数）。
- 测试框架不强制（可使用手工 assert 宏或轻量级测试框架如 `doctest`），但必须保持与 Model/Service 层一致的代码规范。

### 7.5 测试基准数据

所有涉及 ProjectDemo 的测试，基准数据来源于 `ManualImporter::Import("")` 构建的项目：

```
ProjectDemo
├── Task(1) "Requirement"  dur=5
├── Task(2) "Design"       dur=3
├── Task(3) "Coding"       dur=7
├── Task(4) "Testing"      dur=4
├── Task(5) "Deployment"   dur=2
├── Task(6) "Acceptance"   dur=0 (里程碑)
├── Dep(1→2) FS Lag=0
├── Dep(2→3) FS Lag=2
├── Dep(3→4) FS Lag=-1
├── Dep(4→5) FS Lag=0
├── Dep(3→6) FS Lag=0
├── Res(101) "Architect"    100.0
├── Res(102) "SeniorDev"    80.0
├── Res(103) "JuniorDev"    50.0
├── Res(104) "Tester"       60.0
└── Res(105) "MeetingRoom"  400.0
```

> 关键路径: 1 → 2 → 3 → 4 → 5，总工期 22 天。

---

## 8. main.cpp 设计

```cpp
// main.cpp
#include "ConsoleView.hpp"
#include <iostream>

int main()
{
    ConsoleView view(std::cin, std::cout);
    view.Run();
    return 0;
}
```

`main` 函数仅负责实例化 View 并启动其运行，符合大作业"不可直接将界面功能写在 main 函数中"的要求。代码不超过 15 行（含注释）。

---

## 9. 实现顺序与文件清单

### 9.1 文件清单

```
view/
├── ConsoleView.hpp / .cpp              # 主界面类
├── TaskListFormatter.hpp / .cpp
├── TaskRelationsFormatter.hpp / .cpp
├── DependencyListFormatter.hpp / .cpp
├── ResourceListFormatter.hpp / .cpp
├── StatisticsFormatter.hpp / .cpp
├── ValidationResultFormatter.hpp / .cpp
├── ScheduleResultFormatter.hpp / .cpp
Test/
├── TaskListFormatter_test.cpp
├── TaskRelationsFormatter_test.cpp
├── DependencyListFormatter_test.cpp
├── ResourceListFormatter_test.cpp
├── StatisticsFormatter_test.cpp
├── ValidationResultFormatter_test.cpp
├── ScheduleResultFormatter_test.cpp
└── ConsoleView_integration_test.cpp

main.cpp                                 # 程序入口（项目根目录）
```

共计：**1 个主界面类 (2 文件) + 7 个 Formatter 类 (14 文件) + 8 个测试文件 + 1 个 main.cpp**。

### 9.2 推荐实现顺序

| Phase | 内容 | 依赖 |
| :--- | :--- | :--- |
| 1 | 全部 7 个 Formatter (.hpp + .cpp) | DTO 类型、ScheduleResult、ValidationResult |
| 2 | ConsoleView 输入/输出辅助方法 | — |
| 3 | ConsoleView 各功能函数 + 菜单映射 | Controller、Formatters |
| 4 | main.cpp | ConsoleView |
| 5 | Formatter 单元测试 (7 个) | 对应 Formatter |
| 6 | ConsoleView 集成测试 | ConsoleView + ManualImporter |

### 9.3 编译验证

```
# 编译主程序
g++ -std=c++17 main.cpp view/*.cpp service/*.cpp model/*.cpp -o ProjectScheduler

# 编译测试
g++ -std=c++17 Test/*.cpp view/*.cpp service/*.cpp model/*.cpp -o ViewTests
```

---

## 10. 编码规则速查

- 私有成员前缀 `m_`；指针前缀 `m_p`；非简单类型无类型前缀。
- 函数名大写开头，动词性；布尔函数 `Is`/`Has` 开头。
- 每个 `.hpp`/`.cpp` 必须有文件头注释（格式见 `FormatRule/rule_latest.md` §2.1）。
- 每个类、函数必须有注释（格式见规则文档 §2.2、§2.3）。
- 控制语句必须使用大括号，`{` 单独占一行（Allman 风格）。
- 显式布尔比较：`if (isValid == true)` 而非 `if (isValid)`。
- 禁止 `cout`/`cin` 直接使用——统一通过 `m_out`/`m_in` 引用。
- 所有错误提示以 `[FAIL]` 前缀开始，成功以 `[OK]` 开始，警告以 `[WARN]` 开始。
- 修改任何 `.cpp`/`.hpp` 后立即执行 `clang-format -i <文件>`。

---

## 11. 与未来 GUI 的兼容性

由于所有业务逻辑都由 `ProjectController` 返回 DTO 驱动，且 Formatter 只负责 DTO → string 转换：

- `ProjectController` + DTO 可直接复用于 Qt 界面。
- Formatter 可替换为 Qt 的 `QAbstractTableModel` 子类。
- `ConsoleView` 可整体替换为 `QMainWindow` 子类。

MVC 三层完全解耦，界面替换成本极低。

---

## 12. 设计决策摘要

| 决策 | 结论 | 理由 |
| :--- | :--- | :--- |
| 命令分发方式 | lambda 映射 | 平衡简洁性与可维护性，避免过度设计 |
| Formatter 数量 | 7 个（无 ImportResultFormatter） | ImportResult 在 View 层不可见，Controller 已内部消化 |
| 是否引入菜单基类 | 否 | 唯一一种交互形式，无抽象必要 |
| 表格列宽 | 自适应 + 截断 | 适应不同终端宽度，保障可读性 |
| 输入重试次数 | 3 次 | 避免死循环，给用户放弃路径 |
| 索引风格 | 1-based 展示 + 输入，0-based 传参 | 符合人类直觉，内部接口保持一致 |
| Formatter 方法类型 | 纯静态方法 | 无状态工具类的最纯粹表达形式 |
| 测试基础设施 | 流注入 | 被测代码零修改，覆盖完整交互流程 |
