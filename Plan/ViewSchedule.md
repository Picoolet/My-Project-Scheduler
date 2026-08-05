# View 层实现计划

## Context

基于已完成的 Model 层和 Service 层（详见 [ModelSchedule.md](ModelSchedule.md) 和 [ServiceSchedule.md](ServiceSchedule.md)），实现 [View_Goal.md](View_Goal.md) 要求的 CLI 界面层。

分析 [ViewSchedule_original.md](ViewSchedule_original.md) 发现以下核心问题：

1. **上帝类**：ConsoleView 承担 5 类职责（菜单导航、业务操作、输入读取、输出渲染、菜单映射表），~25 个私有方法
2. **硬编码单例依赖**：无法注入 Controller，阻碍单元测试
3. **菜单式交互繁琐**：用户需嵌套导航，测试输入为晦涩的数字序列

**设计决策：改为命令行式交互**（如 `add task Design 5`），同步解决以上三个问题。

Service 层已确认就绪（含 `ImportProject` warnings 参数和 `RemoveDependency` 的 (pred,succ) 重载）。

## 架构设计

### View 层结构

```
ConsoleView (主循环 + 命令注册 + 命令分发)
  │  构造注入: std::istream&, std::ostream&, ProjectController&
  │
  ├── CommandRegistry (命令定义存储 + 查找 + 帮助文本)
  │     - 注册命令定义（含别名）
  │     - 按 "action:target" 查找
  │     - 生成帮助文本（遍历所有注册的命令）
  │     - 依赖：无（仅用标准库）
  │
  ├── CommandParser (字符串 → ParsedCommand)
  │     - 分词（支持双引号字符串）
  │     - 通过 Registry 查找命令
  │     - 参数个数校验（基于 CommandDef 声明的 minArgs/maxArgs）
  │     - 依赖：CommandRegistry（const 引用）
  │
  ├── OutputWriter (统一输出格式)
  │     - PrintSuccess/PrintError/PrintWarning/PrintHeader/Print
  │     - 不依赖 cout/cerr，全部通过 m_out 引用
  │
  └── Formatters × 7 (数据 → 字符串，全部静态方法)
        TaskListFormatter, TaskRelationsFormatter, DependencyListFormatter,
        ResourceListFormatter, StatisticsFormatter, ValidationResultFormatter,
        ScheduleResultFormatter
```

### 命令语法

```
add task <name> <duration>
remove task <index>
list tasks
modify task <index> <name> <duration>    # name用引号包裹可含空格
show task <index>                        # 查看前驱/后继
add dependency <predIdx> <succIdx> <FS|SS|FF|SF> <lag>
remove dependency <index>
remove dependency <predIdx> <succIdx>
list dependencies
add resource <name> <unitCost>
list resources
assign resource <taskIdx> <resIdx> <quantity>
import <filePath>
export <filePath>
stats
validate
schedule
help
quit / exit
```

### 命令注册与分发

`ConsoleView` 构造函数中注册所有命令（action + target → handler），存入 `std::unordered_map<std::string, CommandHandler>`。

```cpp
void RegisterCommand(const std::string& action, const std::string& target,
                     const std::string& signature,
                     const std::string& description,
                     int minArgs, int maxArgs,
                     CommandHandler handler);
```

- Key = `"action:target"`（如 `"add:task"`, `"list:tasks"`）
- 支持别名：同一 handler 注册多个 key（如 `"list:task"`, `"list:tasks"`, `"ls:task"`）
- `help` 命令自动遍历注册表生成帮助文本
- 运行时 `ExecuteCommand` → `m_handlerMap.find(key)` → O(1) 分发

### 校验分层

| 层级 | 校验内容 | 负责类 |
| --- | --- | --- |
| Parser | 参数个数（minArgs/maxArgs）、参数类型（int/double） | `CommandParser` |
| View handler | —（薄层转发，不做校验） | `ConsoleView` 各 Handle 方法 |
| Controller/Editor | 名称唯一性、DAG 约束、里程碑资源限制 | Service 层 |

### 依赖规则

**View 层允许依赖：**

- `ProjectController`（唯一 Controller 依赖）
- `TaskDTO`, `DependencyDTO`, `ResourceDTO`, `ProjectStatisticsDTO`（DTO）
- `ScheduleResult`, `TaskScheduleInfo`（Model 层纯数据载体）
- `ValidationResult`（Service 层数据载体）
- `DependencyType`（枚举，用于 Formatter 渲染和 Parser 依赖类型校验）

**View 层禁止依赖：**

- `Project`, `Task`, `Dependency`, `Resource`, `Allocation`（Model 领域类）
- `IProjectImporter`, `IProjectExporter`, `PpmImporter`, `PpmExporter`
- `CPMCalculator`, `ProjectValidator`, `ProjectEditor`

## 实现文件清单

```
view/
├── CommandRegistry.hpp / .cpp             # 新建 — 命令注册表（定义存储+查找+帮助）
├── CommandParser.hpp / .cpp               # 新建 — 命令行解析器（分词+校验）
├── OutputWriter.hpp / .cpp                # 新建 — 统一输出工具
├── ConsoleView.hpp / .cpp                 # 新建 — 主界面类（命令注册+分发+主循环）
├── TaskListFormatter.hpp / .cpp           # 新建 — 任务列表格式化
├── TaskRelationsFormatter.hpp / .cpp      # 新建 — 前后继关系格式化
├── DependencyListFormatter.hpp / .cpp     # 新建 — 依赖列表格式化
├── ResourceListFormatter.hpp / .cpp       # 新建 — 资源列表格式化
├── StatisticsFormatter.hpp / .cpp         # 新建 — 统计信息格式化
├── ValidationResultFormatter.hpp / .cpp   # 新建 — 验证结果格式化
├── ScheduleResultFormatter.hpp / .cpp     # 新建 — 调度结果格式化

Test/
├── CommandRegistry_test.cpp                       # 新建
├── CommandParser_test.cpp                         # 新建
├── TaskListFormatter_test.cpp                     # 新建
├── TaskRelationsFormatter_test.cpp                # 新建
├── DependencyListFormatter_test.cpp               # 新建
├── ResourceListFormatter_test.cpp                 # 新建
├── StatisticsFormatter_test.cpp                   # 新建
├── ValidationResultFormatter_test.cpp             # 新建
├── ScheduleResultFormatter_test.cpp               # 新建
└── ConsoleView_integration_test.cpp               # 新建

main.cpp  # 修改 — 构造 ConsoleView 并注入 Controller
```

**共计：1 个 Registry + 1 个 Parser + 1 个 Writer + 1 个 View + 7 个 Formatter + 10 个测试 + 1 个 main = 40 个文件（20 .hpp + 20 .cpp）**

## 关键类接口

### CommandDef 与 ParsedCommand

```cpp
// 命令定义（注册时填写）
struct CommandDef
{
    std::string action;           // 动作：add, remove, list, ...
    std::string target;           // 目标：task, dependency, resource, ""（无目标）
    std::string signature;        // 参数签名："<name> <duration>"（用于 help）
    std::string description;      // 功能描述（用于 help）
    int         minArgs;          // 最少参数个数
    int         maxArgs;          // 最多参数个数（-1 表示不限）
    bool        requiresProject;  // 是否需要已加载项目
};

// 解析结果
struct ParsedCommand
{
    std::string              action;   // 识别到的动作
    std::string              target;   // 识别到的目标（可为空）
    std::vector<std::string> args;     // 提取的参数列表
    bool                     isValid;  // 是否解析成功
    std::string              errorMsg; // 失败时的错误信息
};
```

### CommandRegistry

```cpp
class CommandRegistry
{
  public:
    CommandRegistry()  = default;
    ~CommandRegistry() = default;

    // 注册一条命令定义（含别名：不同 action/target → 同一 handler 的 key）
    void RegisterCommand(const CommandDef& def);

    // 按 key 查找命令定义，未找到返回 nullptr
    const CommandDef* FindCommand(const std::string& action,
                                  const std::string& target) const;

    // 生成帮助文本（遍历 m_commandDefs，按 action 分组）
    std::string GetHelpText() const;

    // 获取全部命令定义（供 ConsoleView 在注册阶段访问 handler 映射）
    const std::vector<CommandDef>& GetCommands() const;

  private:
    std::vector<CommandDef> m_commandDefs; // 所有命令定义
    std::unordered_map<std::string, const CommandDef*>
        m_commandIndex; // "add:task" → 指针
};
```

### CommandParser

```cpp
class CommandParser
{
  public:
    // 构造时绑定命令注册表（const 引用，不持有所有权）
    explicit CommandParser(const CommandRegistry& registry);

    // 解析一行用户输入，返回 ParsedCommand
    ParsedCommand ParseLine(const std::string& line) const;

  private:
    // 分词：空格分隔，双引号内为一个 token
    std::vector<std::string> Tokenize(const std::string& line) const;

    const CommandRegistry& m_registry;
};
```

### OutputWriter

```cpp
class OutputWriter {
public:
    explicit OutputWriter(std::ostream& out);

    void Print(const std::string& text);               // 原始输出
    void PrintSuccess(const std::string& message);      // "[OK] message"
    void PrintError(const std::string& message);        // "[FAIL] message"
    void PrintWarning(const std::string& message);      // "[WARN] message"
    void PrintHeader(const std::string& title);         // 带分隔线的标题
    std::ostream& GetStream();                          // 供 Formatter 直接输出

private:
    std::ostream& m_out;
};
```

### ConsoleView

```cpp
class ConsoleView {
public:
    // 构造注入
    ConsoleView(std::istream& in, std::ostream& out,
                ProjectController& controller);
    void Run();

private:
    // 初始化命令注册表（构造函数中调用，向 m_registry 注册所有命令）
    void InitializeCommands();

    // 注册单条命令及别名
    void RegisterCommand(const std::string& action, const std::string& target,
                         const std::string& signature,
                         const std::string& description,
                         int minArgs, int maxArgs, bool requiresProject,
                         CommandHandler handler);

    // 执行命令（统一检查 requiresProject + 空项目保护）
    void ExecuteCommand(const ParsedCommand& cmd);

    // 显示欢迎信息和当前项目名
    void ShowPrompt();

    // 索引转换（1-based → 0-based），非法输入返回 -1 并输出错误
    int ToZeroBasedIndex(const std::string& arg) const;

    //------ 命令处理方法（每个命令对应一个私有方法）------
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

    //------ 成员 ------
    using CommandHandler =
        std::function<void(const std::vector<std::string>&)>;

    CommandRegistry                           m_registry;
    CommandParser                             m_parser;
    OutputWriter                              m_output;
    ProjectController&                        m_controller;
    std::unordered_map<std::string, CommandHandler>
        m_handlerMap; // "add:task" → handler
};
```

### 命令注册示例

```cpp
void ConsoleView::InitializeCommands()
{
    // 任务管理
    RegisterCommand("add", "task", "<name> <duration>",
                    "添加任务（duration=0 自动里程碑）", 2, 2, true,
                    [this](auto& a) { HandleAddTask(a); });

    RegisterCommand("remove", "task", "<index>",
                    "删除指定任务（级联删除依赖与分配）", 1, 1, true,
                    [this](auto& a) { HandleRemoveTask(a); });
    // 别名
    RegisterCommand("rm", "task", "<index>", "", 1, 1, true,
                    [this](auto& a) { HandleRemoveTask(a); });

    // ...

    // 导入导出（不需要项目）
    RegisterCommand("import", "", "<filePath>",
                    "导入 PPM 项目文件", 1, 1, false,
                    [this](auto& a) { HandleImport(a); });

    RegisterCommand("export", "", "<filePath>",
                    "导出项目为 PPM 格式", 1, 1, true,
                    [this](auto& a) { HandleExport(a); });

    // 无 target 命令
    RegisterCommand("help", "", "",
                    "显示此帮助信息", 0, 0, false,
                    [this](auto& a) { HandleHelp(a); });
    RegisterCommand("quit", "", "",
                    "退出程序", 0, 0, false,
                    [this](auto& a) { HandleQuit(a); });
    RegisterCommand("exit", "", "", "", 0, 0, false,
                    [this](auto& a) { HandleQuit(a); });  // 别名
}
```

### ExecuteCommand（统一空项目保护）

```cpp
void ConsoleView::ExecuteCommand(const ParsedCommand& cmd)
{
    // 查找命令定义
    const CommandDef* def = m_registry.FindCommand(cmd.action, cmd.target);

    if ((def == nullptr) || (cmd.isValid == false))
    {
        m_output.PrintError(cmd.errorMsg.empty()
                                ? "未知命令，输入 help 查看可用命令"
                                : cmd.errorMsg);
        return;
    }

    // 统一空项目检查
    if ((def->requiresProject == true)
        && (m_controller.HasProject() == false))
    {
        m_output.PrintWarning("当前无项目，请先 import 文件");
        return;
    }

    // 查找并调用 handler（ConsoleView 内部维护 handler map）
    std::string key = cmd.action + ":" + cmd.target;
    auto        it  = m_handlerMap.find(key);

    if (it != m_handlerMap.end())
    {
        it->second(cmd.args);
    }
}
```

### 命令处理示例（薄层转发 + 统一索引转换）

```cpp
void ConsoleView::HandleAddTask(const std::vector<std::string>& args)
{
    std::string name     = args[0];
    int         duration = std::stoi(args[1]);

    std::string errorMsg;
    if (m_controller.AddTask(name, duration, errorMsg) == true)
    {
        std::string typeInfo = (duration == 0) ? "（自动创建为里程碑）" : "";
        m_output.PrintSuccess("任务 \"" + name + "\" 添加成功" + typeInfo);
    }
    else
    {
        m_output.PrintError("添加任务失败：" + errorMsg);
    }
}

void ConsoleView::HandleRemoveTask(const std::vector<std::string>& args)
{
    int index = ToZeroBasedIndex(args[0]);
    if (index < 0) { return; }  // ToZeroBasedIndex 已输出错误

    // 显示任务信息并确认
    // ...确认逻辑...

    std::string errorMsg;
    if (m_controller.RemoveTask(index, errorMsg) == true)
    {
        m_output.PrintSuccess("任务删除成功");
    }
    else
    {
        m_output.PrintError("删除任务失败：" + errorMsg);
    }
}
```

## 实现顺序

### Phase 1：零依赖工具类

- `CommandRegistry` — 无外部依赖（仅标准库）：管理 `CommandDef` 注册、查找、帮助文本
- `CommandParser` — 依赖 `CommandRegistry`（const 引用）：分词 + 命令查找 + 参数个数校验
- `OutputWriter` — 无外部依赖（仅 `<iostream>`, `<string>`）
- `CommandDef`, `ParsedCommand` — 定义于 `CommandRegistry` 头文件内

### Phase 2：Formatter（依赖 DTO + Model 数据载体）

- 全部 7 个 Formatter — 依赖：DTO 类型 + `ScheduleResult` + `ValidationResult` + `DependencyType`
- 全部纯静态方法，不实例化

### Phase 3：ConsoleView（依赖 Phase 1 + 2）

- 构造函数：注册全部命令 + 别名
- `Run()`：显示提示符 → 读行 → ParseLine → ExecuteCommand → 循环
- `ExecuteCommand()`：查表 → 调用 handler
- 各 `Handle*()` 方法：薄层转发到 Controller

### Phase 4：main.cpp

- 构造 ConsoleView，注入 `std::cin`, `std::cout`, `ProjectController::GetInstance()`
- 调用 `view.Run()`

### Phase 5：测试

- Formatter 单元测试 (7 个) — 注入预构建 DTO/Result 对象，验证输出字符串
- CommandParser 单元测试 — 注入字符串，验证 ParsedCommand 各字段
- ConsoleView 集成测试 — 注入 `istringstream` + `ostringstream`，验证完整交互流程

## 用户交互细节

### 索引约定

- 用户输入和显示均用 **1-based** 序号
- 传递给 Controller 前转换为 0-based
- 通过 `ConsoleView::ToZeroBasedIndex(arg)` 统一处理：
  - 解析失败（非数字）→ 输出 `[FAIL] 索引必须为正整数`，返回 -1
  - 解析成功 → 返回 `value - 1`（0-based）

### 空项目保护

- 不再由每个 Handler 分散检查，改为 `ExecuteCommand` 统一处理
- `CommandDef::requiresProject` 标志位控制：
  - `true`（大多数命令）：`ExecuteCommand` 调用前检查 `HasProject()`，无项目时输出 `[WARN]` 并返回
  - `false`（`import`、`help`、`quit`）：跳过检查，始终可执行
- Handler 方法不再包含 `HasProject()` 判断，保持薄层转发

### 危险操作确认

- `remove task <idx>`：先显示任务信息 → `确认删除? (y/n)`
- `remove dependency <idx>`：同上

### 导入结果展示

- 成功但有警告：`[OK] 导入成功（含 N 条警告）` + 逐条显示警告
- 失败：`[FAIL] 导入失败: <错误信息>`
- 利用 Controller 的 warnings 参数

### 删除依赖的双模式

- `remove dependency <index>` → Controller::RemoveDependency(index)
- `remove dependency <predIdx> <succIdx>` → Controller::RemoveDependency(predIdx, succIdx)
- CommandParser 注册时声明两种参数个数模式（分别注册两条命令定义）

## 验证方法

### 编译验证

```bash
g++ -std=c++17 main.cpp view/*.cpp service/*.cpp model/*.cpp -o ProjectScheduler
```

### 功能验证

1. 启动程序 → 显示欢迎信息和 `>` 提示符
2. `import <test.ppm>` → 验证导入成功/失败信息
3. `list tasks` → 验证表格显示完整（序号/名称/工期/前驱/后继）
4. `add task TestTask 5` → 验证添加成功
5. `add task Milestone1 0` → 验证里程碑提示
6. `remove task 1` → 确认 y → 验证删除成功
7. `modify task 1 "New Name" 10` → 验证修改成功
8. `add dependency 2 3 FS 1` → 验证依赖添加
9. `validate` → 验证通过/不通过信息
10. `schedule` → 验证总工期 + 关键路径
11. `help` → 验证所有命令的帮助文本
12. `quit` → 验证退出

### 基准数据验证

以 ManualImporter 的 ProjectDemo 为基准：

- `stats` → taskCount=6, depCount=5, resCount=5, isValid=true, totalDuration=22
- `schedule` → 总工期 22, 关键路径 [1,2,3,4,5]
- `validate` → `[OK] 项目验证通过`
