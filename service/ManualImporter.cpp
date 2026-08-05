//-----------------------------------------------------------------------------
// 【ManualImporter.cpp】
// 【手动导入器类实现，业务层测试桩】
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------

#include "ManualImporter.hpp"

#include <memory>

#include "DependencyType.hpp"
#include "Id.hpp"
#include "ImportResult.hpp"
#include "Project.hpp"

//-----------------------------------------------------------------------------
// 【ManualImporter::Import】
// 【函数功能】在代码中手动构建 PLANNER/ImportFormat 中 ppm 样例的 Project
// 【参数】path — 输入参数，文件路径（测试桩忽略此参数）
// 【返回值】包含 ProjectDemo 的 ImportResult
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
ImportResult ManualImporter::Import(const std::string& /* path */)
{
    std::unique_ptr<Project> project = std::make_unique<Project>();
    project->SetName("ProjectDemo");

    // 添加任务，对应样例 T/M 行，显式 ID 与样例文件中的 ID 一致（1~6）
    project->AddTask(TaskId(1), "Requirement", 5);
    project->AddTask(TaskId(2), "Design", 3);
    project->AddTask(TaskId(3), "Coding", 7);
    project->AddTask(TaskId(4), "Testing", 4);
    project->AddTask(TaskId(5), "Deployment", 2);
    // 工期 0 自动识别为里程碑（对应样例 M 行）
    project->AddTask(TaskId(6), "Acceptance", 0);

    // 添加资源，对应样例 R 行，显式 ID 与样例文件中的 ID 一致（101~105）
    project->AddResource(ResourceId(101), "Architect", 100.0);
    project->AddResource(ResourceId(102), "SeniorDev", 80.0);
    project->AddResource(ResourceId(103), "JuniorDev", 50.0);
    project->AddResource(ResourceId(104), "Tester", 60.0);
    project->AddResource(ResourceId(105), "MeetingRoom", 400.0);

    // 添加依赖关系，对应样例 D 行
    project->AddDependency(TaskId(1), TaskId(2), DependencyType::FS, 0);
    project->AddDependency(TaskId(2), TaskId(3), DependencyType::FS, 2);
    project->AddDependency(TaskId(3), TaskId(4), DependencyType::FS, -1);
    project->AddDependency(TaskId(4), TaskId(5), DependencyType::FS, 0);
    project->AddDependency(TaskId(3), TaskId(6), DependencyType::FS, 0);

    // 添加资源分配，对应样例 A 行
    project->AssignResource(TaskId(1), ResourceId(101), 1);
    project->AssignResource(TaskId(1), ResourceId(105), 3);
    project->AssignResource(TaskId(2), ResourceId(102), 2);
    project->AssignResource(TaskId(3), ResourceId(102), 2);
    project->AssignResource(TaskId(3), ResourceId(103), 1);
    project->AssignResource(TaskId(4), ResourceId(104), 2);
    project->AssignResource(TaskId(5), ResourceId(103), 1);

    return ImportResult(std::move(project));
}
