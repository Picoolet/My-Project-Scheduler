//-----------------------------------------------------------------------------
// 【CommandRegistry_test.cpp】
// 【命令注册表单元测试】
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------

#include <cassert>
#include <iostream>
#include <string>

#include "CommandRegistry.hpp"

//-----------------------------------------------------------------------------
// 【main】
// 【函数功能】测试 RegisterCommand / FindCommand / GetHelpText / GetCommands
// 【参数】无
// 【返回值】0 — 全部通过
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
int main()
{
    CommandRegistry registry;

    CommandDef addTask;
    addTask.action          = "add";
    addTask.target          = "task";
    addTask.signature       = "<name> <duration>";
    addTask.description     = "添加任务";
    addTask.minArgs         = 2;
    addTask.maxArgs         = 2;
    addTask.requiresProject = true;

    CommandDef addDep;
    addDep.action          = "add";
    addDep.target          = "dep";
    addDep.signature       = "";
    addDep.description     = ""; // 纯别名
    addDep.minArgs         = 4;
    addDep.maxArgs         = 4;
    addDep.requiresProject = true;

    CommandDef helpCmd;
    helpCmd.action          = "help";
    helpCmd.target          = "";
    helpCmd.signature       = "";
    helpCmd.description     = "显示帮助";
    helpCmd.minArgs         = 0;
    helpCmd.maxArgs         = 0;
    helpCmd.requiresProject = false;

    registry.RegisterCommand(addTask);
    registry.RegisterCommand(addDep);
    registry.RegisterCommand(helpCmd);

    // 查找存在的命令
    const CommandDef* found = registry.FindCommand("add", "task");
    assert(found != nullptr);
    assert(found->description == "添加任务");
    assert(found->requiresProject == true);

    // 查找别名
    assert(registry.FindCommand("add", "dep") != nullptr);

    // 查找不存在的命令
    assert(registry.FindCommand("add", "resource") == nullptr);
    assert(registry.FindCommand("unknown", "task") == nullptr);

    // GetHelpText 包含非别名条目，不含别名描述
    std::string help = registry.GetHelpText();
    assert(help.find("添加任务") != std::string::npos);
    assert(help.find("显示帮助") != std::string::npos);
    assert(help.find("可用命令") != std::string::npos);

    // GetCommands 返回 3 条
    assert(registry.GetCommands().size() == 3);

    std::cout << "CommandRegistry test PASSED\n";
    return 0;
}
