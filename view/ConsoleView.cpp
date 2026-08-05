//-----------------------------------------------------------------------------
// 【ConsoleView.cpp】
// 【命令行界面视图类实现】
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------

#include "ConsoleView.hpp"

#include <cctype>
#include <string>
#include <vector>

#include "DependencyListFormatter.hpp"
#include "DependencyType.hpp"
#include "ProjectController.hpp"
#include "ResourceListFormatter.hpp"
#include "ScheduleResult.hpp"
#include "ScheduleResultFormatter.hpp"
#include "StatisticsFormatter.hpp"
#include "TaskListFormatter.hpp"
#include "TaskRelationsFormatter.hpp"
#include "ValidationResult.hpp"
#include "ValidationResultFormatter.hpp"

//-----------------------------------------------------------------------------
// 【ConsoleView::ConsoleView】
// 【函数功能】构造函数，绑定输入输出流与控制器，初始化命令注册表
// 【参数】in — 输入参数，输入流
//        out — 输入参数，输出流
//        controller — 输入参数，Service 层控制器引用
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
ConsoleView::ConsoleView(std::istream& in, std::ostream& out,
                         ProjectController& controller)
    : m_in(in), m_parser(m_registry), m_output(out), m_controller(controller),
      m_bRunning(true)
{
    InitializeCommands();
}

//-----------------------------------------------------------------------------
// 【ConsoleView::InitializeCommands】
// 【函数功能】注册全部命令（含别名），并绑定对应 handler
// 【参数】无
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::InitializeCommands()
{
    //------ 添加命令 ------
    RegisterCommand("add", "task", "<name> <duration>",
                    "添加任务（duration=0 自动里程碑）", 2, 2, true,
                    [this](const std::vector<std::string>& args)
                    { HandleAddTask(args); });

    RegisterCommand("add", "dependency", "<pred> <succ> <FS|SS|FF|SF> <lag>",
                    "添加依赖", 4, 4, true,
                    [this](const std::vector<std::string>& args)
                    { HandleAddDependency(args); });

    RegisterCommand("add", "dep", "", "", 4, 4, true,
                    [this](const std::vector<std::string>& args)
                    { HandleAddDependency(args); });

    RegisterCommand("add", "resource", "<name> <unitCost>", "添加资源", 2, 2,
                    true, [this](const std::vector<std::string>& args)
                    { HandleAddResource(args); });

    RegisterCommand("add", "res", "", "", 2, 2, true,
                    [this](const std::vector<std::string>& args)
                    { HandleAddResource(args); });

    //------ 删除命令 ------
    RegisterCommand("remove", "task", "<index>", "删除任务", 1, 1, true,
                    [this](const std::vector<std::string>& args)
                    { HandleRemoveTask(args); });

    RegisterCommand("rm", "task", "", "", 1, 1, true,
                    [this](const std::vector<std::string>& args)
                    { HandleRemoveTask(args); });

    RegisterCommand("remove", "dependency", "<index> 或 <predIdx> <succIdx>",
                    "删除依赖", 1, 2, true,
                    [this](const std::vector<std::string>& args)
                    { HandleRemoveDependency(args); });

    RegisterCommand("remove", "dep", "", "", 1, 2, true,
                    [this](const std::vector<std::string>& args)
                    { HandleRemoveDependency(args); });

    RegisterCommand("rm", "dep", "", "", 1, 2, true,
                    [this](const std::vector<std::string>& args)
                    { HandleRemoveDependency(args); });

    //------ 列表命令 ------
    RegisterCommand("list", "tasks", "", "列出所有任务", 0, 0, true,
                    [this](const std::vector<std::string>& args)
                    { HandleListTasks(args); });

    RegisterCommand("list", "task", "", "", 0, 0, true,
                    [this](const std::vector<std::string>& args)
                    { HandleListTasks(args); });

    RegisterCommand("ls", "tasks", "", "", 0, 0, true,
                    [this](const std::vector<std::string>& args)
                    { HandleListTasks(args); });

    RegisterCommand("ls", "task", "", "", 0, 0, true,
                    [this](const std::vector<std::string>& args)
                    { HandleListTasks(args); });

    RegisterCommand("list", "dependencies", "", "列出所有依赖", 0, 0, true,
                    [this](const std::vector<std::string>& args)
                    { HandleListDependencies(args); });

    RegisterCommand("list", "deps", "", "", 0, 0, true,
                    [this](const std::vector<std::string>& args)
                    { HandleListDependencies(args); });

    RegisterCommand("ls", "deps", "", "", 0, 0, true,
                    [this](const std::vector<std::string>& args)
                    { HandleListDependencies(args); });

    RegisterCommand("list", "resources", "", "列出所有资源", 0, 0, true,
                    [this](const std::vector<std::string>& args)
                    { HandleListResources(args); });

    RegisterCommand("list", "res", "", "", 0, 0, true,
                    [this](const std::vector<std::string>& args)
                    { HandleListResources(args); });

    RegisterCommand("ls", "res", "", "", 0, 0, true,
                    [this](const std::vector<std::string>& args)
                    { HandleListResources(args); });

    //------ 查询/修改命令 ------
    RegisterCommand("show", "task", "<index>", "查看任务的前驱与后继", 1, 1,
                    true, [this](const std::vector<std::string>& args)
                    { HandleShowTask(args); });

    RegisterCommand("modify", "task", "<index> <name> <duration>", "修改任务",
                    3, 3, true, [this](const std::vector<std::string>& args)
                    { HandleModifyTask(args); });

    RegisterCommand("assign", "resource", "<taskIdx> <resIdx> <quantity>",
                    "为任务分配资源", 3, 3, true,
                    [this](const std::vector<std::string>& args)
                    { HandleAssignResource(args); });

    RegisterCommand("assign", "res", "", "", 3, 3, true,
                    [this](const std::vector<std::string>& args)
                    { HandleAssignResource(args); });

    //------ 文件与统计命令 ------
    RegisterCommand("import", "", "<filePath>", "导入项目文件", 1, 1, false,
                    [this](const std::vector<std::string>& args)
                    { HandleImport(args); });

    RegisterCommand("export", "", "<filePath>", "导出项目文件", 1, 1, true,
                    [this](const std::vector<std::string>& args)
                    { HandleExport(args); });

    RegisterCommand("stats", "", "", "显示项目统计信息", 0, 0, true,
                    [this](const std::vector<std::string>& args)
                    { HandleStats(args); });

    RegisterCommand("validate", "", "", "执行项目合理性验证", 0, 0, true,
                    [this](const std::vector<std::string>& args)
                    { HandleValidate(args); });

    RegisterCommand("schedule", "", "", "执行关键路径调度计算", 0, 0, true,
                    [this](const std::vector<std::string>& args)
                    { HandleSchedule(args); });

    RegisterCommand("help", "", "", "显示帮助", 0, 0, false,
                    [this](const std::vector<std::string>& args)
                    { HandleHelp(args); });

    RegisterCommand("quit", "", "", "退出程序", 0, 0, false,
                    [this](const std::vector<std::string>& args)
                    { HandleQuit(args); });

    RegisterCommand("exit", "", "", "", 0, 0, false,
                    [this](const std::vector<std::string>& args)
                    { HandleQuit(args); });
}

//-----------------------------------------------------------------------------
// 【ConsoleView::RegisterCommand】
// 【函数功能】注册一条命令（注册表定义 + handler 映射）
// 【参数】action — 输入参数，动作
//        target — 输入参数，目标
//        signature — 输入参数，参数签名
//        description — 输入参数，功能描述
//        minArgs — 输入参数，最少参数个数
//        maxArgs — 输入参数，最多参数个数
//        requiresProject — 输入参数，是否需要已加载项目
//        handler — 输入参数，命令处理函数
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::RegisterCommand(const std::string& action,
                                  const std::string& target,
                                  const std::string& signature,
                                  const std::string& description, int minArgs,
                                  int maxArgs, bool requiresProject,
                                  CommandHandler handler)
{
    CommandDef def;
    def.action          = action;
    def.target          = target;
    def.signature       = signature;
    def.description     = description;
    def.minArgs         = minArgs;
    def.maxArgs         = maxArgs;
    def.requiresProject = requiresProject;

    m_registry.RegisterCommand(def);
    m_handlerMap[action + ":" + target] = handler;
}

//-----------------------------------------------------------------------------
// 【ConsoleView::Run】
// 【函数功能】主循环：读入命令、解析、执行，直到退出或 EOF
// 【参数】无
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::Run()
{
    ShowWelcome();

    while (m_bRunning == true)
    {
        ShowPrompt();

        std::string line;

        if (static_cast<bool>(std::getline(m_in, line)) == false)
        {
            break; // EOF
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

//-----------------------------------------------------------------------------
// 【ConsoleView::ExecuteCommand】
// 【函数功能】执行命令：查定义、统一空项目检查、调 handler
// 【参数】cmd — 输入参数，解析后的命令
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::ExecuteCommand(const ParsedCommand& cmd)
{
    const CommandDef* def = m_registry.FindCommand(cmd.action, cmd.target);

    if ((def == nullptr) || (cmd.isValid == false))
    {
        m_output.PrintError(cmd.errorMsg.empty() == true
                                ? "未知命令，输入 help 查看可用命令"
                                : cmd.errorMsg);
        return;
    }

    // 统一空项目检查
    if ((def->requiresProject == true) && (m_controller.HasProject() == false))
    {
        m_output.PrintWarning("当前无项目，请先 import 文件");
        return;
    }

    // 查找并调用 handler
    std::string key = cmd.action + ":" + cmd.target;
    auto        it  = m_handlerMap.find(key);

    if (it != m_handlerMap.end())
    {
        it->second(cmd.args);
    }
}

//-----------------------------------------------------------------------------
// 【ConsoleView::ShowWelcome】
// 【函数功能】显示欢迎信息
// 【参数】无
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::ShowWelcome()
{
    m_output.PrintLine("项目调度器 (Project Scheduler)");
    m_output.PrintLine("输入 help 查看可用命令，输入 quit 退出。");
}

//-----------------------------------------------------------------------------
// 【ConsoleView::ShowPrompt】
// 【函数功能】显示提示符
// 【参数】无
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::ShowPrompt()
{
    m_output.Print("> ");
}

//-----------------------------------------------------------------------------
// 【ConsoleView::ToZeroBasedIndex】
// 【函数功能】将 1-based 序号转换为 0-based 索引
// 【参数】arg — 输入参数，用户输入的序号字符串
// 【返回值】0-based 索引；非法输入输出错误并返回 -1
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
int ConsoleView::ToZeroBasedIndex(const std::string& arg) const
{
    try
    {
        int index = std::stoi(arg);

        if (index <= 0)
        {
            m_output.PrintError("索引必须为正整数");
            return -1;
        }

        return (index - 1);
    }
    catch (...)
    {
        m_output.PrintError("索引必须为正整数");
        return -1;
    }
}

//-----------------------------------------------------------------------------
// 【ConsoleView::Confirm】
// 【函数功能】请求用户确认（y/n）
// 【参数】prompt — 输入参数，确认提示文本
// 【返回值】true — 用户确认
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
bool ConsoleView::Confirm(const std::string& prompt) const
{
    m_output.PrintWarning(prompt + " (y/n): ");

    std::string line;

    if (static_cast<bool>(std::getline(m_in, line)) == false)
    {
        return false;
    }

    if ((line == "y") || (line == "Y") || (line == "yes") || (line == "Yes"))
    {
        return true;
    }

    return false;
}

//=============================================================================
// 命令处理方法
//=============================================================================

//-----------------------------------------------------------------------------
// 【ConsoleView::HandleImport】
// 【函数功能】导入项目文件，反馈成功/失败与警告
// 【参数】args — 输入参数，参数列表（含文件路径）
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::HandleImport(const std::vector<std::string>& args)
{
    std::string              errorMsg;
    std::vector<std::string> warnings;

    bool success = m_controller.ImportProject(args[0], errorMsg, &warnings);

    if (success == false)
    {
        m_output.PrintError("导入失败: " + errorMsg);
        return;
    }

    if (warnings.empty() == true)
    {
        m_output.PrintSuccess("导入成功");
    }
    else
    {
        m_output.PrintSuccess("导入成功（含 " + std::to_string(warnings.size())
                              + " 条警告）");

        for (const std::string& warning : warnings)
        {
            m_output.PrintWarning(warning);
        }
    }
}

//-----------------------------------------------------------------------------
// 【ConsoleView::HandleExport】
// 【函数功能】导出项目文件
// 【参数】args — 输入参数，参数列表（含文件路径）
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::HandleExport(const std::vector<std::string>& args)
{
    std::string errorMsg;

    if (m_controller.ExportProject(args[0], errorMsg) == true)
    {
        m_output.PrintSuccess("导出成功");
    }
    else
    {
        m_output.PrintError("导出失败: " + errorMsg);
    }
}

//-----------------------------------------------------------------------------
// 【ConsoleView::HandleListTasks】
// 【函数功能】列出所有任务
// 【参数】args — 输入参数，参数列表（忽略）
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::HandleListTasks(const std::vector<std::string>& /* args */)
{
    m_output.Print(TaskListFormatter::Format(m_controller.ListTasks()));
}

//-----------------------------------------------------------------------------
// 【ConsoleView::HandleAddTask】
// 【函数功能】添加任务
// 【参数】args — 输入参数，[名称, 工期]
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::HandleAddTask(const std::vector<std::string>& args)
{
    int duration = 0;

    try
    {
        duration = std::stoi(args[1]);
    }
    catch (...)
    {
        m_output.PrintError("工期必须为正整数");
        return;
    }

    std::string errorMsg;

    if (m_controller.AddTask(args[0], duration, errorMsg) == true)
    {
        if (duration == 0)
        {
            m_output.PrintSuccess("添加成功（自动创建为里程碑）");
        }
        else
        {
            m_output.PrintSuccess("添加成功");
        }
    }
    else
    {
        m_output.PrintError("添加失败: " + errorMsg);
    }
}

//-----------------------------------------------------------------------------
// 【ConsoleView::HandleRemoveTask】
// 【函数功能】删除任务（含确认）
// 【参数】args — 输入参数，[索引]
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::HandleRemoveTask(const std::vector<std::string>& args)
{
    int index = ToZeroBasedIndex(args[0]);

    if (index < 0)
    {
        return;
    }

    // 显示待删除任务名称
    const auto& tasks = m_controller.ListTasks();

    if (static_cast<size_t>(index) >= tasks.size())
    {
        m_output.PrintError("索引无效");
        return;
    }

    m_output.PrintLine("  待删除任务: "
                       + tasks[static_cast<size_t>(index)].name);

    if (Confirm("确认删除此任务？") == false)
    {
        m_output.PrintLine("  已取消。");
        return;
    }

    std::string errorMsg;

    if (m_controller.RemoveTask(index, errorMsg) == true)
    {
        m_output.PrintSuccess("删除成功");
    }
    else
    {
        m_output.PrintError("删除失败: " + errorMsg);
    }
}

//-----------------------------------------------------------------------------
// 【ConsoleView::HandleShowTask】
// 【函数功能】查看指定任务的详情：自身信息 + 前驱/后继 + 资源列表
// 【参数】args — 输入参数，[索引]
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.6
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::HandleShowTask(const std::vector<std::string>& args)
{
    int index = ToZeroBasedIndex(args[0]);

    if (index < 0)
    {
        return;
    }

    // 获取任务自身信息
    const auto& tasks = m_controller.ListTasks();

    if (static_cast<size_t>(index) >= tasks.size())
    {
        m_output.PrintError("索引无效");
        return;
    }

    const TaskDTO& task = tasks[static_cast<size_t>(index)];

    m_output.Print(TaskRelationsFormatter::Format(
        task, m_controller.GetTaskRelations(index),
        m_controller.GetTaskResources(index)));
}

//-----------------------------------------------------------------------------
// 【ConsoleView::HandleModifyTask】
// 【函数功能】修改任务名称与工期（- 表示不改名）
// 【参数】args — 输入参数，[索引, 新名称或 -, 新工期]
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::HandleModifyTask(const std::vector<std::string>& args)
{
    int index = ToZeroBasedIndex(args[0]);

    if (index < 0)
    {
        return;
    }

    // 获取当前任务信息
    const auto& tasks = m_controller.ListTasks();

    if (static_cast<size_t>(index) >= tasks.size())
    {
        m_output.PrintError("索引无效");
        return;
    }

    std::string newName = args[1];

    if (newName == "-")
    {
        newName = tasks[static_cast<size_t>(index)].name;
    }

    int newDuration = 0;

    try
    {
        newDuration = std::stoi(args[2]);
    }
    catch (...)
    {
        m_output.PrintError("工期必须为非负整数");
        return;
    }

    std::string errorMsg;

    if (m_controller.ModifyTask(index, newName, newDuration, errorMsg) == true)
    {
        m_output.PrintSuccess("修改成功");
    }
    else
    {
        m_output.PrintError("修改失败: " + errorMsg);
    }
}

//-----------------------------------------------------------------------------
// 【ConsoleView::HandleListDependencies】
// 【函数功能】列出所有依赖
// 【参数】args — 输入参数，参数列表（忽略）
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::HandleListDependencies(
    const std::vector<std::string>& /* args */)
{
    m_output.Print(
        DependencyListFormatter::Format(m_controller.ListDependencies()));
}

//-----------------------------------------------------------------------------
// 【ConsoleView::HandleAddDependency】
// 【函数功能】添加依赖
// 【参数】args — 输入参数，[前驱索引, 后继索引, 类型, Lag]
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::HandleAddDependency(const std::vector<std::string>& args)
{
    int predIndex = ToZeroBasedIndex(args[0]);

    if (predIndex < 0)
    {
        return;
    }

    int succIndex = ToZeroBasedIndex(args[1]);

    if (succIndex < 0)
    {
        return;
    }

    // 解析依赖类型（不区分大小写）
    std::string typeStr = args[2];

    for (char& ch : typeStr)
    {
        ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }

    DependencyType type = DependencyType::FS;

    if (typeStr == "SS")
    {
        type = DependencyType::SS;
    }
    else if (typeStr == "FF")
    {
        type = DependencyType::FF;
    }
    else if (typeStr == "SF")
    {
        type = DependencyType::SF;
    }

    int lag = 0;

    try
    {
        lag = std::stoi(args[3]);
    }
    catch (...)
    {
        m_output.PrintError("Lag 必须为整数");
        return;
    }

    std::string errorMsg;

    if (m_controller.AddDependency(predIndex, succIndex, type, lag, errorMsg)
        == true)
    {
        m_output.PrintSuccess("添加成功");
    }
    else
    {
        m_output.PrintError("添加失败: " + errorMsg);
    }
}

//-----------------------------------------------------------------------------
// 【ConsoleView::HandleRemoveDependency】
// 【函数功能】删除依赖（支持索引或前驱+后继两种方式）
// 【参数】args — 输入参数，[索引] 或 [前驱索引, 后继索引]
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::HandleRemoveDependency(const std::vector<std::string>& args)
{
    std::string errorMsg;
    bool        success = false;

    if (args.size() == 1)
    {
        int index = ToZeroBasedIndex(args[0]);

        if (index < 0)
        {
            return;
        }

        success = m_controller.RemoveDependency(index, errorMsg);
    }
    else
    {
        int predIndex = ToZeroBasedIndex(args[0]);

        if (predIndex < 0)
        {
            return;
        }

        int succIndex = ToZeroBasedIndex(args[1]);

        if (succIndex < 0)
        {
            return;
        }

        success = m_controller.RemoveDependency(predIndex, succIndex, errorMsg);
    }

    if (success == true)
    {
        m_output.PrintSuccess("删除成功");
    }
    else
    {
        m_output.PrintError("删除失败: " + errorMsg);
    }
}

//-----------------------------------------------------------------------------
// 【ConsoleView::HandleListResources】
// 【函数功能】列出所有资源
// 【参数】args — 输入参数，参数列表（忽略）
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::HandleListResources(
    const std::vector<std::string>& /* args */)
{
    m_output.Print(ResourceListFormatter::Format(m_controller.ListResources()));
}

//-----------------------------------------------------------------------------
// 【ConsoleView::HandleAddResource】
// 【函数功能】添加资源
// 【参数】args — 输入参数，[名称, 单位成本]
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::HandleAddResource(const std::vector<std::string>& args)
{
    double unitCost = 0.0;

    try
    {
        unitCost = std::stod(args[1]);
    }
    catch (...)
    {
        m_output.PrintError("单位成本必须为数字");
        return;
    }

    std::string errorMsg;

    if (m_controller.AddResource(args[0], unitCost, errorMsg) == true)
    {
        m_output.PrintSuccess("添加成功");
    }
    else
    {
        m_output.PrintError("添加失败: " + errorMsg);
    }
}

//-----------------------------------------------------------------------------
// 【ConsoleView::HandleAssignResource】
// 【函数功能】为任务分配资源
// 【参数】args — 输入参数，[任务索引, 资源索引, 数量]
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::HandleAssignResource(const std::vector<std::string>& args)
{
    int taskIndex = ToZeroBasedIndex(args[0]);

    if (taskIndex < 0)
    {
        return;
    }

    int resourceIndex = ToZeroBasedIndex(args[1]);

    if (resourceIndex < 0)
    {
        return;
    }

    int quantity = 0;

    try
    {
        quantity = std::stoi(args[2]);
    }
    catch (...)
    {
        m_output.PrintError("分配数量必须为正整数");
        return;
    }

    std::string errorMsg;

    if (m_controller.AssignResource(taskIndex, resourceIndex, quantity,
                                    errorMsg)
        == true)
    {
        m_output.PrintSuccess("分配成功");
    }
    else
    {
        m_output.PrintError("分配失败: " + errorMsg);
    }
}

//-----------------------------------------------------------------------------
// 【ConsoleView::HandleStats】
// 【函数功能】显示项目统计信息
// 【参数】args — 输入参数，参数列表（忽略）
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::HandleStats(const std::vector<std::string>& /* args */)
{
    m_output.Print(StatisticsFormatter::Format(m_controller.GetStatistics()));
}

//-----------------------------------------------------------------------------
// 【ConsoleView::HandleValidate】
// 【函数功能】执行项目合理性验证
// 【参数】args — 输入参数，参数列表（忽略）
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::HandleValidate(const std::vector<std::string>& /* args */)
{
    m_output.Print(ValidationResultFormatter::Format(m_controller.Validate()));
}

//-----------------------------------------------------------------------------
// 【ConsoleView::HandleSchedule】
// 【函数功能】执行关键路径调度计算
// 【参数】args — 输入参数，参数列表（忽略）
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::HandleSchedule(const std::vector<std::string>& /* args */)
{
    std::vector<TaskDTO> tasks    = m_controller.ListTasks();
    ScheduleResult       schedule = m_controller.ComputeSchedule();

    m_output.Print(ScheduleResultFormatter::Format(schedule, tasks));
}

//-----------------------------------------------------------------------------
// 【ConsoleView::HandleHelp】
// 【函数功能】显示帮助文本
// 【参数】args — 输入参数，参数列表（忽略）
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::HandleHelp(const std::vector<std::string>& /* args */)
{
    m_output.Print(m_registry.GetHelpText());
}

//-----------------------------------------------------------------------------
// 【ConsoleView::HandleQuit】
// 【函数功能】退出程序
// 【参数】args — 输入参数，参数列表（忽略）
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
void ConsoleView::HandleQuit(const std::vector<std::string>& /* args */)
{
    m_output.PrintLine("Goodbye!");
    m_bRunning = false;
}
