//-----------------------------------------------------------------------------
// 【ConsoleView.hpp】
// 【命令行界面视图类声明，界面层唯一入口】
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------

#ifndef CONSOLEVIEW_HPP
#define CONSOLEVIEW_HPP

#include <functional>
#include <istream>
#include <string>
#include <unordered_map>
#include <vector>

#include "CommandParser.hpp"
#include "CommandRegistry.hpp"
#include "OutputWriter.hpp"

class ProjectController;

//-----------------------------------------------------------------------------
// 【ConsoleView 类】
// 【功能】命令行式界面：读取用户命令，分发给 Controller 并格式化输出
// 【接口说明】构造注入 Controller 引用；Map 注册表分发命令，零业务逻辑
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
class ConsoleView
{
  public:
    ConsoleView(std::istream& in, std::ostream& out,
                ProjectController& controller);

    ConsoleView(const ConsoleView&)            = delete;
    ConsoleView& operator=(const ConsoleView&) = delete;
    ~ConsoleView()                             = default;

    // 运行主循环，直到 quit/exit 或 EOF
    void Run();

  private:
    using CommandHandler = std::function<void(const std::vector<std::string>&)>;

    // 初始化命令注册表与 handler 映射（构造函数中调用）
    void InitializeCommands();

    // 注册单条命令（注册表 + handler 映射）
    void RegisterCommand(const std::string& action, const std::string& target,
                         const std::string& signature,
                         const std::string& description, int minArgs,
                         int maxArgs, bool requiresProject,
                         CommandHandler handler);

    // 执行命令：查定义 → requiresProject 检查 → 调 handler
    void ExecuteCommand(const ParsedCommand& cmd);

    void ShowWelcome();
    void ShowPrompt();

    // 索引转换（1-based → 0-based），非法输入输出错误并返回 -1
    int ToZeroBasedIndex(const std::string& arg) const;

    // 确认操作（y/n），返回 true 表示确认
    bool Confirm(const std::string& prompt) const;

    //------ 命令处理方法 ------
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

    std::istream&                                   m_in;
    CommandRegistry                                 m_registry;
    CommandParser                                   m_parser;
    OutputWriter                                    m_output;
    ProjectController&                              m_controller;
    std::unordered_map<std::string, CommandHandler> m_handlerMap;
    bool                                            m_bRunning;
};

#endif
