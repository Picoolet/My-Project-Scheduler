# View 层设计方案（ViewSchedule）

## 1. 定位与职责

View 层是 MVC 架构中的界面层，直接与用户交互，负责：

- 显示菜单、提示和格式化输出
- 接收用户输入并完成基础的输入校验（如数字格式、范围检查）
- 调用 `ProjectController` 单例的公开接口，获取 DTO 或操作结果
- 将 DTO 传递给对应的 Formatter，转化为字符串后输出

View 层**绝对不包含**：

- 任何业务逻辑（如判断任务是否为里程碑、计算关键路径）
- 对 Model 层对象的直接操作
- 对 Service 层除 `ProjectController` 外的其他组件的直接依赖

---

## 2. 核心设计理念

### 2.1 可配置的输入输出流

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

- 生产环境使用 `ConsoleView(std::cin, std::cout)`
- 单元测试可注入 `std::istringstream` 和 `std::ostringstream`，验证完整的交互流程
- 完全符合 C++ 标准，无全局变量，体现可重用性

### 2.2 轻量级命令分发（lambda 映射）

为避免引入过多的 Command 子类（违反“不过度设计”原则），同时仍然将菜单结构与具体操作解耦，采用 `std::map<int, std::function<void()>>` 存储菜单项。

每个菜单项映射到一个**私有成员函数**，该函数内部：

1. 从 `m_in` 读取必要参数
2. 调用 `ProjectController::GetInstance()` 的对应方法
3. 通过 Formatter 格式化结果，输出至 `m_out`

这种设计：

- 满足“除 lambda 外不允许非成员函数”的要求
- 避免了大量具体命令类的文件膨胀
- 保持了良好的可扩展性：新增菜单项只需新增一个私有函数并注册到 map 中

### 2.3 Formatter 工具类集

Formatter 是无状态的工具类，负责将 DTO 结构体转换为可展示的字符串。它们被设计为可重用的视图组件，不依赖 `std::cout` 或具体控制台特性，因此也可以被未来的 GUI 界面复用（例如直接将 DTO 填充到表格模型）。

---

## 3. 主界面类：`ConsoleView`

### 3.1 类声明框架

```cpp
// view/ConsoleView.hpp
class ConsoleView {
public:
    ConsoleView(std::istream& in, std::ostream& out);
    void Run();

private:
    // 主菜单与子菜单
    void ShowMainMenu();
    void HandleMainChoice(int choice);

    void ShowTaskMenu();
    void ShowDependencyMenu();
    void ShowResourceMenu();

    // 具体功能函数（对应需求 2.1 ~ 2.5）
    void ImportProject();
    void ExportProject();
    void ListTasks();
    void RemoveTask();
    void AddTask();
    void ShowTaskRelations();
    void ModifyTask();
    void ListDependencies();
    void RemoveDependency();
    void AddDependency();
    void ListResources();
    void AddResource();
    void AssignResource();
    void ShowStatistics();
    void ValidateProject();
    void ComputeSchedule();

    // 输入辅助
    int ReadInt(const std::string& prompt);
    std::string ReadLine(const std::string& prompt);
    double ReadDouble(const std::string& prompt);
    bool Confirm(const std::string& prompt);

    std::istream& m_in;
    std::ostream& m_out;
    // 菜单映射表
    std::map<int, std::function<void()>> m_mainActions;
    std::map<int, std::function<void()>> m_taskActions;
    std::map<int, std::function<void()>> m_depActions;
    std::map<int, std::function<void()>> m_resActions;
};
```

### 3.2 菜单结构树（符合用户操作习惯）

```
主菜单
├── 1. 项目导入/导出
│   ├── 1. 导入 PPM 文件
│   └── 2. 导出为 PPM 文件
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
├── 5. 项目统计与调度
│   ├── 1. 显示统计信息
│   ├── 2. 验证项目合理性
│   └── 3. 计算关键路径
└── 0. 退出
```

每个子菜单可返回上级，避免用户迷失。使用数字选择，输入 `0` 返回。

### 3.3 用户行为关怀要点

- **输入校验与重试**：读取索引、工期、成本等数值时，若输入非法（非数字、越界）则提示并允许重新输入，最多重试三次或让用户主动放弃。
- **危险操作确认**：删除任务、依赖等不可逆操作前，显示被删对象的关键信息并请求确认（`Confirm` 方法）。
- **操作反馈**：每次修改后显示简要成功信息；失败时通过 `errorMsg` 参数捕获并显示具体错误原因（如“任务名称已存在”）。
- **空项目保护**：若尚未导入项目（`HasProject() == false`），大部分功能应提示“当前无项目，请先导入”并直接返回。
- **结果显示格式化**：所有列表、统计、调度结果均通过 Formatter 输出为整齐的表格或结构化文本，提升可读性。

---

## 4. 命令体系的取舍结论

**采用「lambda 映射」而非独立 Command 类**。

| 方案 | 优点 | 缺点 | 本次选择 |
| ------ | ------ | ------ | ---------- |
| 独立 Command 类（每个操作一个类） | 完美满足 OCP，便于扩展 | 文件数暴增，逻辑薄，过度设计 | 不采用 |
| switch-case | 零开销，简单直接 | 分支膨胀，不易维护，扩展性差 | 不采用 |
| **lambda 映射** | 分离菜单结构与实现，易于维护，符合作业规则 | 需要理解 `std::function` | **采用** |

这种轻量级命令模式正是“优雅设计”与“不过度设计”的平衡点。

---

## 5. Formatter 类集设计

所有 Formatter 均为**无状态的工具类**，提供静态或普通方法，输入为 DTO 的 const 引用，输出为 `std::string`。

| Formatter 类 | 输入 DTO | 输出格式说明 |
| :--- | :--- | :--- |
| `TaskListFormatter` | `std::vector<TaskDTO>` | 表格：索引、ID、名称、工期、前驱、后继 |
| `TaskRelationsFormatter` | `std::pair<std::vector<TaskDTO>, std::vector<TaskDTO>>` | 分别列出前驱列表和后继列表（索引+名称） |
| `DependencyListFormatter` | `std::vector<DependencyDTO>` | 表格：序号、前置索引、后置索引、类型、Lag |
| `ResourceListFormatter` | `std::vector<ResourceDTO>` | 表格：索引、ID、名称、单位成本 |
| `StatisticsFormatter` | `ProjectStatisticsDTO` | 任务数/依赖数/资源数/是否合理/总工期 |
| `ScheduleResultFormatter` | `ScheduleResult` (Model 层) | 总工期 + 各任务时间参数表 + 关键路径 ID 列表 |
| `ValidationResultFormatter` | `ValidationResult` | 错误列表，或显示“项目合理” |
| `ImportResultFormatter` | `ImportResult` | 导入成功提示 + 警告列表，或错误列表 |

`ScheduleResult` 和 `ValidationResult` 虽然定义在 Model 层，但它们是纯数据载体，可以被 View 层直接使用（不违反 MVC，因为 View 层依赖它们只是为了展示）。如果希望完全隔离，可以在 Service 层定义对应的 DTO，但此时 `ScheduleResult` 本身已足够纯粹，可直接使用。

**示例接口声明**：

```cpp
class TaskListFormatter {
public:
    static std::string Format(const std::vector<TaskDTO>& tasks);
};
```

---

## 6. 改进点与主流方案对标

### 6.1 输入/输出抽象化

如上所述，注入流对象使 View 可测试，这是工业级 CLI 工具的标准实践（如使用 `Console.ReadKey` 抽象）。额外收益：未来可替换为网络流、文件流，或用于自动化测试脚本。

### 6.2 用户交互的鲁棒性

- 在所有需要索引的地方，将用户输入的“1-based 显示序号”转换为内部的 0-based 索引，并调用 `Validate` 确保在合法范围。
- 在任务、资源名称输入时，去除首尾空格，并检查空字符串。
- 工期和成本输入时，确保为正整数或正浮点数，否则要求重输。

### 6.3 国际化准备（非必需，但体现可重用性）

Formatter 内部的所有字符串常量可以集中管理，但本次作业只要求纯英文，不强制。

### 6.4 与未来 GUI 的兼容性

由于所有业务逻辑都由 Controller 返回 DTO，且 Formatter 只负责 DTO → string 转换，将来编写 Qt 界面时：

- Controller 和 DTO 直接复用
- Formatter 可被替换为 Qt 的 Model/View 数据适配器
- `ConsoleView` 整体被替换为 `QMainWindow` 子类

MVC 三层完全解耦，界面替换成本极低。

---

## 7. 文件清单

```
view/
├── ConsoleView.hpp / .cpp        # 主界面类
├── TaskListFormatter.hpp / .cpp
├── TaskRelationsFormatter.hpp / .cpp
├── DependencyListFormatter.hpp / .cpp
├── ResourceListFormatter.hpp / .cpp
├── StatisticsFormatter.hpp / .cpp
├── ScheduleResultFormatter.hpp / .cpp
├── ValidationResultFormatter.hpp / .cpp
└── ImportResultFormatter.hpp / .cpp
```

共计 **1 个主界面类 + 8 个 Formatter 类**，结构清晰，职责单一。

---

这套 View 层设计完全基于已经完成的 Service 层契约，保证了界面与逻辑的彻底分离，同时兼顾了用户的实际操作体验。
