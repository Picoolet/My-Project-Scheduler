# View 层实现 — Coding Agent 提示词

## 任务概述

基于已完成的 Model 层和 Service 层，实现 CLI 命令行式 View 层。完整设计文档见 [ViewSchedule.md](ViewSchedule.md)。

**核心设计决策：**
- 命令行式交互（如 `add task Design 5`），非菜单式
- Map 注册表分发命令（非 switch-case）
- 构造注入 Controller 引用（非单例调用）
- 严格遵守 MVC：View 不直接触碰 Model 层

## 已完成的依赖（只读使用，不修改）

这些是 Service/Model 层已经存在并验证通过的文件，View 层通过 `#include` 使用它们：

| 文件 | 关键接口 |
|---|---|
| `service/ProjectController.hpp` | `GetInstance()`, `ImportProject(path, err, warn*)`, `ExportProject(path, err)`, `HasProject()`, `ListTasks()`, `AddTask(name, dur, err)`, `RemoveTask(idx, err)`, `ModifyTask(idx, name, dur, err)`, `GetTaskRelations(idx)`, `ListDependencies()`, `AddDependency(pred,succ,type,lag,err)`, `RemoveDependency(idx, err)`, `RemoveDependency(predIdx, succIdx, err)`, `ListResources()`, `AddResource(name,cost,err)`, `AssignResource(task,res,qty,err)`, `GetStatistics()`, `Validate()`, `ComputeSchedule()` |
| `service/TaskDTO.hpp` | `struct TaskDTO { int index; int idValue; std::string name; int duration; std::vector<int> predecessorIndices; std::vector<int> successorIndices; }` |
| `service/DependencyDTO.hpp` | `struct DependencyDTO { int index; int predecessorIndex; int successorIndex; DependencyType type; int lag; }` |
| `service/ResourceDTO.hpp` | `struct ResourceDTO { int index; int idValue; std::string name; double unitCost; }` |
| `service/ProjectStatisticsDTO.hpp` | `struct ProjectStatisticsDTO { int taskCount; int dependencyCount; int resourceCount; bool isValid; int totalDuration; }` |
| `service/ValidationResult.hpp` | `class ValidationResult { bool IsValid() const; const std::vector<std::string>& GetErrors() const; }` |
| `model/ScheduleResult.hpp` | `class ScheduleResult { int GetTotalDuration() const; int GetEarlyStart(TaskId) const; int GetEarlyFinish(TaskId) const; int GetLateStart(TaskId) const; int GetLateFinish(TaskId) const; const std::vector<TaskId>& GetCriticalPath() const; }` |
| `model/DependencyType.hpp` | `enum class DependencyType { FS, SS, FF, SF }` |

## 需要创建的文件清单

```
view/
├── CommandRegistry.hpp / .cpp
├── CommandParser.hpp / .cpp
├── OutputWriter.hpp / .cpp
├── ConsoleView.hpp / .cpp
├── TaskListFormatter.hpp / .cpp
├── TaskRelationsFormatter.hpp / .cpp
├── DependencyListFormatter.hpp / .cpp
├── ResourceListFormatter.hpp / .cpp
├── StatisticsFormatter.hpp / .cpp
├── ValidationResultFormatter.hpp / .cpp
└── ScheduleResultFormatter.hpp / .cpp

main.cpp  # 修改（替换现有内容）

Test/
├── CommandRegistry_test.cpp
├── CommandParser_test.cpp
├── TaskListFormatter_test.cpp
├── TaskRelationsFormatter_test.cpp
├── DependencyListFormatter_test.cpp
├── ResourceListFormatter_test.cpp
├── StatisticsFormatter_test.cpp
├── ValidationResultFormatter_test.cpp
├── ScheduleResultFormatter_test.cpp
└── ConsoleView_integration_test.cpp
```

## 实现顺序

按 Phase 顺序实现，每个 Phase 内部可并行，Phase 间严格顺序：

### Phase 1：零依赖工具类（无 Service/Model 依赖）

#### 1a. `view/CommandRegistry.hpp` + `CommandRegistry.cpp`

头文件定义两个结构体 + 一个类：

```cpp
#ifndef COMMANDREGISTRY_HPP
#define COMMANDREGISTRY_HPP

#include <string>
#include <unordered_map>
#include <vector>

// 命令定义（注册时填写）
struct CommandDef
{
    std::string action;           // 动作：add, remove, list, show, modify, import, export, stats, validate, schedule, help, quit
    std::string target;           // 目标：task, dependency, resource, tasks, dependencies, resources, ""（无目标）
    std::string signature;        // 参数签名："<name> <duration>"（用于 help 显示）
    std::string description;      // 功能描述（用于 help 显示）
    int         minArgs;          // 最少参数个数
    int         maxArgs;          // 最多参数个数（-1 表示不限）
    bool        requiresProject;  // 是否需要已加载项目
};

// 解析结果
struct ParsedCommand
{
    std::string              action;
    std::string              target;
    std::vector<std::string> args;
    bool                     isValid;
    std::string              errorMsg;
};

class CommandRegistry
{
public:
    CommandRegistry()  = default;
    ~CommandRegistry() = default;

    // 注册一条命令定义。不同 action/target 组合可指向同一逻辑（别名）
    // 如果同一 key 已存在则覆盖（别名后注册的优先）
    void RegisterCommand(const CommandDef& def);

    // 按 "action:target" 精确查找，未找到返回 nullptr
    const CommandDef* FindCommand(const std::string& action,
                                  const std::string& target) const;

    // 生成帮助文本：按 action 分组，每行 "  action target signature  — description"
    std::string GetHelpText() const;

    // 获取全部命令定义（供遍历，如 ConsoleView 检查注册完整性）
    const std::vector<CommandDef>& GetCommands() const;

private:
    std::vector<CommandDef> m_commandDefs;
    std::unordered_map<std::string, const CommandDef*> m_commandIndex;
    // key = action + ":" + target（如 "add:task", "help:"）
};

#endif
```

**实现要点：**
- `RegisterCommand`：`m_commandDefs.push_back(def)`，然后 `m_commandIndex[def.action + ":" + def.target] = &m_commandDefs.back()`
- `FindCommand`：构造 key = `action + ":" + target`，`m_commandIndex.find(key)`，未找到返回 nullptr
- `GetHelpText`：遍历 `m_commandDefs`，跳过 description 为空的条目（纯别名），按 action 分组输出。格式：
  ```
  可用命令：
    add task <name> <duration>           — 添加任务（duration=0 自动里程碑）
    add dependency <pred> <succ> <type> <lag> — 添加依赖
    ...
  ```

#### 1b. `view/CommandParser.hpp` + `CommandParser.cpp`

```cpp
#ifndef COMMANDPARSER_HPP
#define COMMANDPARSER_HPP

#include "CommandRegistry.hpp"
#include <string>
#include <vector>

class CommandParser
{
public:
    explicit CommandParser(const CommandRegistry& registry);

    // 解析一行用户输入，返回 ParsedCommand
    // - 空行/纯空格 → {.isValid=false, .errorMsg=""}
    // - 未知 action:target → {.isValid=false, .errorMsg="未知命令..."}
    // - 参数个数不匹配 → {.isValid=false, .errorMsg="参数个数错误..."}
    // - 成功 → {.isValid=true, .action=..., .target=..., .args=...}
    ParsedCommand ParseLine(const std::string& line) const;

private:
    // 分词：空格分隔，双引号内为一个 token（不含引号本身）
    // 例如：add task "Design Phase" 5 → ["add", "task", "Design Phase", "5"]
    std::vector<std::string> Tokenize(const std::string& line) const;

    const CommandRegistry& m_registry;
};

#endif
```

**实现要点：**
- `Tokenize`：
  1. 遍历字符，跳过前导空格
  2. 遇到 `"` → 收集到下一个 `"`（不含引号），作为一个 token
  3. 遇到普通字符 → 收集到下一个空格，作为一个 token
  4. 返回 token 列表
- `ParseLine`：
  1. `tokens = Tokenize(line)`，若为空返回 `{.isValid=false}`
  2. `action = tokens[0]`（转为小写）
  3. 若 `tokens.size() >= 2` 且 `tokens[1]` 匹配已知 target 关键词（task/tasks/dependency/dependencies/dep/resource/resources/res）→ `target = tokens[1]`（转为小写/标准化），`args = tokens[2..]`
  4. 否则 `target = ""`，`args = tokens[1..]`
  5. `def = m_registry.FindCommand(action, target)`，未找到返回 `{.isValid=false, .errorMsg="未知命令..."}`
  6. 检查 `args.size()` 在 `[def->minArgs, def->maxArgs]` 范围内，不在则返回错误
  7. 返回成功 ParsedCommand

#### 1c. `view/OutputWriter.hpp` + `OutputWriter.cpp`

```cpp
#ifndef OUTPUTWRITER_HPP
#define OUTPUTWRITER_HPP

#include <ostream>
#include <string>

class OutputWriter
{
public:
    explicit OutputWriter(std::ostream& out);

    void Print(const std::string& text);
    void PrintLine(const std::string& text);             // Print + '\n'
    void PrintSuccess(const std::string& message);        // "  [OK] message\n"
    void PrintError(const std::string& message);          // "  [FAIL] message\n"
    void PrintWarning(const std::string& message);        // "  [WARN] message\n"
    void PrintHeader(const std::string& title);           // 带分隔线标题
    void PrintSeparator();                                // 一行 "─" 分隔线
    std::ostream& GetStream();

private:
    std::ostream& m_out;
};

#endif
```

**实现要点：**
- `PrintHeader`：输出 `\n══════ title ══════\n`（分隔线长度取 title 长度 + 8，最少 40 字符宽）
- 全部输出通过 `m_out`，不使用 `std::cout`/`std::cerr`

### Phase 2：Formatter 类（依赖 DTO + Model 数据载体）

**共同特征：**
- 全部纯静态方法类
- 禁止实例化（`= delete` 构造函数或全部私有 + 静态方法）
- 每个类的唯一公共方法：`static std::string Format(...)`
- 格式化为人类可读的文本表格

#### 2a. `view/TaskListFormatter.hpp` + `.cpp`

```cpp
class TaskListFormatter
{
public:
    TaskListFormatter() = delete;

    // 输入：BuildTaskDTOs 的返回值
    // 输出：五列表格（序号/名称/工期/前驱/后继），1-based 序号
    // 空列表 → 返回 "  暂无任务\n"
    static std::string Format(const std::vector<TaskDTO>& tasks);
};
```

**表格格式：**
```
  ─────────────────────────────────────────────────────────────
  序号  名称              工期      前驱任务      后继任务
  ─────────────────────────────────────────────────────────────
  1     Requirement        5                        [2]
  2     Design             3         [1]            [3]
  6     Acceptance         0 (里程碑) [3]
  ─────────────────────────────────────────────────────────────
```

- 工期列的宽度固定为 10（含 "(里程碑)" 文本的可能宽度）
- 前驱/后继列：`[1, 3, 5]` 格式
- 序号从 1 开始（DTO 的 index + 1）
- 名称截断为 30 字符 + `...`

#### 2b. `view/TaskRelationsFormatter.hpp` + `.cpp`

```cpp
class TaskRelationsFormatter
{
public:
    TaskRelationsFormatter() = delete;

    // 输入：GetTaskRelations 的返回值（pair<前驱列表, 后继列表>）
    // 输出：分两区显示前驱和后继的序号+名称
    static std::string Format(
        const std::pair<std::vector<TaskDTO>, std::vector<TaskDTO>>& relations);
};
```

**格式：**
```
  前驱任务 (Predecessors):
    1 - Requirement
  后继任务 (Successors):
    3 - Coding
    6 - Acceptance
```
空列表显示 "    (无)"

#### 2c. `view/DependencyListFormatter.hpp` + `.cpp`

```cpp
class DependencyListFormatter
{
public:
    DependencyListFormatter() = delete;

    static std::string Format(const std::vector<DependencyDTO>& dependencies);
};
```

**表格：序号/前置序号/后置序号/类型/Lag（五列），1-based 序号**

DependencyType → 字符串：FS/SS/FF/SF（内部 static 辅助函数）

#### 2d. `view/ResourceListFormatter.hpp` + `.cpp`

```cpp
class ResourceListFormatter
{
public:
    ResourceListFormatter() = delete;

    static std::string Format(const std::vector<ResourceDTO>& resources);
};
```

**表格：序号/名称/单位成本（三列），成本保留 2 位小数**

#### 2e. `view/StatisticsFormatter.hpp` + `.cpp`

```cpp
class StatisticsFormatter
{
public:
    StatisticsFormatter() = delete;

    static std::string Format(const ProjectStatisticsDTO& stats);
};
```

**格式：**
```
  ─────────────────────────────
  项目统计
  ─────────────────────────────
  Task 总数:       6
  Dependency 总数:  5
  Resource 总数:    5
  项目状态:         通过验证
  总工期:           22 天
  ─────────────────────────────
```
`isValid==false` → 状态显示 "未通过验证"，总工期显示 "N/A"

#### 2f. `view/ValidationResultFormatter.hpp` + `.cpp`

```cpp
class ValidationResultFormatter
{
public:
    ValidationResultFormatter() = delete;

    static std::string Format(const ValidationResult& result);
};
```

- `IsValid() == true` → `"  [OK] 项目验证通过，无错误。\n"`
- `IsValid() == false` → 逐条列出错误，编号 `  #1: <错误信息>\n`

#### 2g. `view/ScheduleResultFormatter.hpp` + `.cpp`

```cpp
class ScheduleResultFormatter
{
public:
    ScheduleResultFormatter() = delete;

    static std::string Format(const ScheduleResult& result);
};
```

**格式：**
```
  ────────────────────────────────────────────────────
  调度结果 — 总工期: 22 天
  ────────────────────────────────────────────────────
  序号  ID  名称          ES   EF   LS   LF   关键
  ────────────────────────────────────────────────────
  1     1   Requirement    0    5    0    5    *
  2     2   Design         5    8    5    8    *
  ...
  6     6   Acceptance    17   17   22   22
  ────────────────────────────────────────────────────
  关键路径: 1 → 2 → 3 → 4 → 5
  ────────────────────────────────────────────────────
```

- 通过 `Controller::ListTasks()` 获取名称，通过 `TaskId::Value()` 获取 ID 数值
- 关键路径用 `*` 标记（`GetCriticalPath()` 判断是否包含该 TaskId）
- 空结果（totalDuration==0 且 criticalPath 为空）→ "  无法计算调度（项目可能为空或存在环路）\n"
- **注意**：这个 Formatter 需要的输入不只是 ScheduleResult。你需要同时传递 Task 名称列表。建议签名：
  `static std::string Format(const ScheduleResult& result, const std::vector<TaskDTO>& tasks);`

### Phase 3：ConsoleView（依赖 Phase 1 + 2）

#### `view/ConsoleView.hpp` + `ConsoleView.cpp`

这是最大的类，但结构清晰：

```cpp
#ifndef CONSOLEVIEW_HPP
#define CONSOLEVIEW_HPP

#include "CommandParser.hpp"
#include "CommandRegistry.hpp"
#include "OutputWriter.hpp"
#include <functional>
#include <istream>
#include <string>
#include <unordered_map>
#include <vector>

class ProjectController;

class ConsoleView
{
public:
    ConsoleView(std::istream& in, std::ostream& out,
                ProjectController& controller);
    void Run();

private:
    using CommandHandler =
        std::function<void(const std::vector<std::string>&)>;

    // 初始化命令注册表（构造函数中调用）
    void InitializeCommands();

    // 注册单条命令。内部同时向 m_registry 注册 CommandDef 和向 m_handlerMap 注册 handler
    void RegisterCommand(const std::string& action, const std::string& target,
                         const std::string& signature,
                         const std::string& description, int minArgs,
                         int maxArgs, bool requiresProject,
                         CommandHandler handler);

    // 执行命令：查定义 → requiresProject 检查 → 查 handler → 调用
    void ExecuteCommand(const ParsedCommand& cmd);

    void ShowWelcome();
    void ShowPrompt();

    // 索引转换（1-based → 0-based），非法输入输出错误并返回 -1
    int ToZeroBasedIndex(const std::string& arg) const;

    // 确认操作（y/n），返回 true 表示确认
    bool Confirm(const std::string& prompt) const;

    //------ 命令处理方法（每个对应一个私有方法）------
    void HandleImport(const std::vector<std::string>& args);
    void HandleExport(const std::vector<std::string>& args);
    void HandleListTasks(const std::vector<std::string>& args);
    void HandleAddTask(const std::vector<std::string>& args);
    void HandleRemoveTask(const std::vector<std::string>& args);
    void HandleShowTask(const std::vector<std::string>& args);
    void HandleModifyTask(const std::vector<std::string>& args);
    void HandleListDependencies(const std::vector<std::string>& args);
    void HandleAddDependency(const std::vector<std::string>& args);
    void HandleRemoveDependency(const std::vector<std::string>& args);
    void HandleListResources(const std::vector<std::string>& args);
    void HandleAddResource(const std::vector<std::string>& args);
    void HandleAssignResource(const std::vector<std::string>& args);
    void HandleStats(const std::vector<std::string>& args);
    void HandleValidate(const std::vector<std::string>& args);
    void HandleSchedule(const std::vector<std::string>& args);
    void HandleHelp(const std::vector<std::string>& args);
    void HandleQuit(const std::vector<std::string>& args);

    std::istream& m_in;
    CommandRegistry                           m_registry;
    CommandParser                             m_parser;
    OutputWriter                              m_output;
    ProjectController&                        m_controller;
    std::unordered_map<std::string, CommandHandler> m_handlerMap;
    bool m_bRunning;
};

#endif
```

**构造函数实现要点：**
1. 初始化列表：`m_in(in)`, `m_output(out)`, `m_controller(controller)`, `m_parser(m_registry)`, `m_bRunning(true)`
2. 调用 `InitializeCommands()`

**`InitializeCommands()` 必须注册以下全部命令（含别名）：**

```
action  target       signature                        requiresProject   → handler
------  ------       ---------                        ---------------   --------
add     task         <name> <duration>                true              HandleAddTask
add     dependency   <pred> <succ> <FS|SS|FF|SF> <lag> true             HandleAddDependency
add     dep          <pred> <succ> <FS|SS|FF|SF> <lag> true             HandleAddDependency (alias)
add     resource     <name> <unitCost>                true              HandleAddResource
add     res          <name> <unitCost>                true              HandleAddResource (alias)
remove  task         <index>                          true              HandleRemoveTask
rm      task         <index>                          true              HandleRemoveTask (alias)
remove  dependency   <index>                          true              HandleRemoveDependency
remove  dependency   <predIdx> <succIdx>              true              HandleRemoveDependency (overload)
remove  dep          <index>                          true              HandleRemoveDependency (alias)
remove  dep          <predIdx> <succIdx>              true              HandleRemoveDependency (alias)
rm      dep          <index>                          true              HandleRemoveDependency (alias)
list    tasks                                          true              HandleListTasks
list    task                                           true              HandleListTasks (alias)
ls      tasks                                          true              HandleListTasks (alias)
ls      task                                           true              HandleListTasks (alias)
list    dependencies                                   true              HandleListDependencies
list    deps                                           true              HandleListDependencies (alias)
ls      deps                                           true              HandleListDependencies (alias)
list    resources                                      true              HandleListResources
list    res                                            true              HandleListResources (alias)
ls      res                                            true              HandleListResources (alias)
show    task         <index>                          true              HandleShowTask
modify  task         <index> <name> <duration>        true              HandleModifyTask
assign  resource     <taskIdx> <resIdx> <quantity>    true              HandleAssignResource
assign  res          <taskIdx> <resIdx> <quantity>    true              HandleAssignResource (alias)
import  ""           <filePath>                       false             HandleImport
export  ""           <filePath>                       true              HandleExport
stats   ""                                            true              HandleStats
validate ""                                           true              HandleValidate
schedule ""                                           true              HandleSchedule
help    ""                                            false             HandleHelp
quit    ""                                            false             HandleQuit
exit    ""                                            false             HandleQuit (alias)
```

**`Run()` 主循环：**
```cpp
void ConsoleView::Run()
{
    ShowWelcome();
    while (m_bRunning == true)
    {
        ShowPrompt();
        std::string line;
        if (std::getline(m_in, line) == false)
        {
            break;  // EOF
        }
        // 跳过纯空行
        if (line.find_first_not_of(" \t") == std::string::npos)
        {
            continue;
        }
        ParsedCommand cmd = m_parser.ParseLine(line);
        ExecuteCommand(cmd);
    }
}
```

**`ShowPrompt()`：**
```
  提示符格式："> "（当前项目名显示在欢迎信息和 stats 中即可）
```

**`ExecuteCommand()`：**
```cpp
void ConsoleView::ExecuteCommand(const ParsedCommand& cmd)
{
    // 1. 查找命令定义
    const CommandDef* def = m_registry.FindCommand(cmd.action, cmd.target);
    if ((def == nullptr) || (cmd.isValid == false))
    {
        m_output.PrintError(cmd.errorMsg.empty()
                                ? "未知命令，输入 help 查看可用命令"
                                : cmd.errorMsg);
        return;
    }

    // 2. 统一空项目检查
    if ((def->requiresProject == true) && (m_controller.HasProject() == false))
    {
        m_output.PrintWarning("当前无项目，请先 import 文件");
        return;
    }

    // 3. 查找 handler
    std::string key = cmd.action + ":" + cmd.target;
    auto        it  = m_handlerMap.find(key);
    if (it != m_handlerMap.end())
    {
        it->second(cmd.args);
    }
}
```

**`ToZeroBasedIndex()`：**
```cpp
int ConsoleView::ToZeroBasedIndex(const std::string& arg) const
{
    try
    {
        int index = std::stoi(arg);
        if (index <= 0)
        {
            m_output.PrintError("序号必须为正整数");
            return -1;
        }
        return (index - 1);
    }
    catch (...)
    {
        m_output.PrintError("序号必须为正整数");
        return -1;
    }
}
```

**`Confirm()`：**
```cpp
bool ConsoleView::Confirm(const std::string& prompt) const
{
    m_output.PrintWarning(prompt + " (y/n): ");
    std::string line;
    if (std::getline(m_in, line) == false) { return false; }
    return (line == "y" || line == "Y" || line == "yes" || line == "Yes");
}
```

**Handler 实现要点（薄层转发）：**

- `HandleImport`：调 `m_controller.ImportProject(args[0], errorMsg, &warnings)`，成功且无警告 → `[OK]`，成功有警告 → `[OK] 导入成功（含N条警告）` + 逐条输出，失败 → `[FAIL]`
- `HandleExport`：调 `m_controller.ExportProject(args[0], errorMsg)`
- `HandleListTasks`：调 `m_controller.ListTasks()` → `TaskListFormatter::Format()` → `m_output.Print()`
- `HandleAddTask`：调 `m_controller.AddTask(args[0], stoi(args[1]), errorMsg)`，duration==0 时成功消息含 "(自动创建为里程碑)"
- `HandleRemoveTask`：1) `ToZeroBasedIndex(args[0])`，失败 return；2) 查任务名并显示；3) `Confirm("确认删除此任务？")`；4) 调 `m_controller.RemoveTask(index, errorMsg)`
- `HandleShowTask`：`ToZeroBasedIndex(args[0])` → `m_controller.GetTaskRelations(index)` → `TaskRelationsFormatter::Format()` → 输出
- `HandleModifyTask`：1) `ToZeroBasedIndex(args[0])`；2) 获取当前任务信息显示；3) name 来自 args[1]（若 args[1] 为 `-` 表示不改名，需先获取旧名）；4) duration 来自 args[2]；5) 调 `m_controller.ModifyTask(index, newName, newDuration, errorMsg)`
- `HandleListDependencies`：`ListDependencies()` → `DependencyListFormatter::Format()` → 输出
- `HandleAddDependency`：1) `ToZeroBasedIndex(args[0])` + `ToZeroBasedIndex(args[1])`；2) 解析 DependencyType（args[2]，不区分大小写）；3) `stoi(args[3])` 解析 lag；4) 调 `m_controller.AddDependency(predIdx, succIdx, type, lag, errorMsg)`
- `HandleRemoveDependency`：若 args 个数=1 → `ToZeroBasedIndex(args[0])` → `m_controller.RemoveDependency(index, errorMsg)`；若 args 个数=2 → 两个 `ToZeroBasedIndex` → `m_controller.RemoveDependency(predIdx, succIdx, errorMsg)`
- `HandleListResources`：`ListResources()` → `ResourceListFormatter::Format()` → 输出
- `HandleAddResource`：`m_controller.AddResource(args[0], stod(args[1]), errorMsg)`
- `HandleAssignResource`：1) 两个 `ToZeroBasedIndex` + `stoi(args[2])` 解析 quantity；2) `m_controller.AssignResource(taskIdx, resIdx, qty, errorMsg)`
- `HandleStats`：`GetStatistics()` → `StatisticsFormatter::Format()` → 输出
- `HandleValidate`：`m_controller.Validate()` → `ValidationResultFormatter::Format()` → 输出
- `HandleSchedule`：`m_controller.ListTasks()` + `m_controller.ComputeSchedule()` → `ScheduleResultFormatter::Format(result, tasks)` → 输出
- `HandleHelp`：`m_output.Print(m_registry.GetHelpText())`
- `HandleQuit`：`m_output.Print("Goodbye!\n")`，`m_bRunning = false`

### Phase 4：main.cpp（替换现有内容）

```cpp
#include "ConsoleView.hpp"
#include "ProjectController.hpp"
#include <iostream>

int main()
{
    ConsoleView view(std::cin, std::cout, ProjectController::GetInstance());
    view.Run();
    return 0;
}
```

### Phase 5：测试

每个测试文件用简洁的 assert 宏即可（允许手写 `assert` 或 `doctest`）。

**测试要点：**
- `CommandRegistry_test`：注册 3 条命令 → 查找存在/不存在的 → 验证 GetHelpText 含预期文本
- `CommandParser_test`：有效命令 → 验证 ParsedCommand 各字段；无效命令 → 验证 isValid==false；双引号字符串 → 验证 token 正确；参数不足 → 验证错误
- Formatter 测试：构造 DTO 列表/Result 对象 → 调用 Format → 验证输出字符串含关键文本（名称、数字、"里程碑"、"*"等）
- `ConsoleView_integration_test`：注入 `istringstream` 模拟输入序列，验证输出关键文本

## 编码规则速查

这些规则在 `FormatRule/rule_latest.md` 中定义，必须遵守：

### 格式
- 4 空格缩进，Allman 风格（`{` 独占一行）
- 文件头注释 + 类注释 + 函数注释（详细格式见 Phase 各节的注释模板）
- 私有成员前缀 `m_`，指针前缀 `m_p`
- 函数名大写开头，动词性；布尔函数 `Is`/`Has` 开头
- 显式布尔比较：`if (isValid == true)` 而非 `if (isValid)`
- 控制语句必须用大括号
- 开发者标注：`QJQ 2026.8.5`

### 操作
- 每次修改 `.cpp`/`.hpp` 后立即运行 `clang-format -i <文件>`
- View 层禁止 `#include` Model 层头文件，但以下例外允许：`ScheduleResult.hpp`、`DependencyType.hpp`（纯数据/枚举）
- View 层使用 `ProjectController.hpp` 时，不需要也不应该 include `Project.hpp`、`ProjectEditor.hpp`、`ProjectValidator.hpp`、`CPMCalculator.hpp`、`PpmImporter.hpp`、`PpmExporter.hpp`

## 编译验证

```bash
# 编译主程序
g++ -std=c++17 -o ProjectScheduler main.cpp view/*.cpp service/*.cpp model/*.cpp

# 编译并运行单个测试（以 CommandRegistry 为例）
g++ -std=c++17 -o test_Registry Test/CommandRegistry_test.cpp view/CommandRegistry.cpp
./test_Registry
```

## 验证基准

以 ManualImporter 的 ProjectDemo（6 任务、5 依赖、5 资源）为基准：
- `stats` → 显示 taskCount=6, depCount=5, resCount=5, isValid=true, totalDuration=22
- `schedule` → 总工期 22 天，关键路径 `1 → 2 → 3 → 4 → 5`
- `validate` → `[OK] 项目验证通过`
