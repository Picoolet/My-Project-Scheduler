//-----------------------------------------------------------------------------
// 【CommandParser_test.cpp】
// 【命令解析器单元测试】
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------

#include <cassert>
#include <iostream>
#include <string>

#include "CommandParser.hpp"
#include "CommandRegistry.hpp"

//-----------------------------------------------------------------------------
// 【main】
// 【函数功能】测试 Tokenize / ParseLine 的各分支
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

    CommandDef importCmd;
    importCmd.action          = "import";
    importCmd.target          = "";
    importCmd.signature       = "<filePath>";
    importCmd.description     = "导入";
    importCmd.minArgs         = 1;
    importCmd.maxArgs         = 1;
    importCmd.requiresProject = false;

    registry.RegisterCommand(addTask);
    registry.RegisterCommand(importCmd);

    CommandParser parser(registry);

    // 有效命令
    ParsedCommand cmd = parser.ParseLine("add task Design 5");
    assert(cmd.isValid == true);
    assert(cmd.action == "add");
    assert(cmd.target == "task");
    assert(cmd.args.size() == 2);
    assert(cmd.args[0] == "Design");
    assert(cmd.args[1] == "5");

    // 双引号 token
    cmd = parser.ParseLine("add task \"Design Phase\" 5");
    assert(cmd.isValid == true);
    assert(cmd.args.size() == 2);
    assert(cmd.args[0] == "Design Phase");

    // 无 target 命令（import）
    cmd = parser.ParseLine("import demo.ppm");
    assert(cmd.isValid == true);
    assert(cmd.action == "import");
    assert(cmd.target == "");
    assert(cmd.args.size() == 1);
    assert(cmd.args[0] == "demo.ppm");

    // 未知命令
    cmd = parser.ParseLine("frobnicate task 1");
    assert(cmd.isValid == false);
    assert(cmd.errorMsg.find("未知命令") != std::string::npos);

    // 参数不足
    cmd = parser.ParseLine("add task onlyName");
    assert(cmd.isValid == false);
    assert(cmd.errorMsg.find("参数个数错误") != std::string::npos);

    // 空行
    cmd = parser.ParseLine("   ");
    assert(cmd.isValid == false);

    std::cout << "CommandParser test PASSED\n";
    return 0;
}
