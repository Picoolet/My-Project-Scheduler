//-----------------------------------------------------------------------------
// 【ConsoleView_integration_test.cpp】
// 【ConsoleView 集成测试：注入模拟输入流，验证输出关键文本】
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

#include "ConsoleView.hpp"
#include "ImportResult.hpp"
#include "ManualImporter.hpp"
#include "PpmExporter.hpp"
#include "Project.hpp"
#include "ProjectController.hpp"

//-----------------------------------------------------------------------------
// 【main】
// 【函数功能】先用 PpmExporter 写出 ProjectDemo 临时文件，再通过 import 命令
//            装载，验证 stats/validate/schedule/list 的关键输出
// 【参数】无
// 【返回值】0 — 全部通过
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
int main()
{
    // 准备临时 ppm 文件（用 PpmExporter 从 ManualImporter 的项目写出）
    const std::string tempFile = "d:/tmp/hw3_integration.ppm";

    ManualImporter importer;
    ImportResult   ir      = importer.Import("");
    auto           project = ir.ReleaseProject();

    PpmExporter exporter;
    assert(exporter.Export(*project, tempFile) == true);

    ProjectController& controller = ProjectController::GetInstance();

    std::string input;
    input += "import " + tempFile + "\n";
    input += "stats\n";
    input += "validate\n";
    input += "schedule\n";
    input += "list tasks\n";
    input += "quit\n";

    std::istringstream in(input);
    std::ostringstream out;

    ConsoleView view(in, out, controller);
    view.Run();

    std::string output = out.str();

    // 导入成功
    assert(output.find("[OK] 导入成功") != std::string::npos);

    // 统计信息
    assert(output.find("项目统计") != std::string::npos);
    assert(output.find("22 天") != std::string::npos);
    assert(output.find("通过验证") != std::string::npos);

    // 验证通过
    assert(output.find("验证通过") != std::string::npos);

    // 调度结果
    assert(output.find("总工期: 22") != std::string::npos);
    assert(output.find("关键路径: 1 → 2 → 3 → 4 → 5") != std::string::npos);

    // 任务列表
    assert(output.find("Requirement") != std::string::npos);
    assert(output.find("Acceptance") != std::string::npos);

    // 退出
    assert(output.find("Goodbye") != std::string::npos);

    std::cout << "ConsoleView integration test PASSED\n";
    return 0;
}
