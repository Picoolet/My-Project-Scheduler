//-----------------------------------------------------------------------------
// 【ManualImporter.cpp】
// 【手动导入器类实现，业务层测试桩】
// 【开发者及日期】 QJQ 2026.8.1
//-----------------------------------------------------------------------------

#include "ManualImporter.hpp"

#include <memory>

#include "DependencyType.hpp"
#include "Id.hpp"

//-----------------------------------------------------------------------------
// 【ManualImporter::Import】
// 【函数功能】在代码中手动构建 PLANNER/ImportFormat 中 ppm 样例的 Project
// 【参数】无
// 【返回值】指向新构建的 Project 对象的 unique_ptr
// 【开发者及日期】 QJQ 2026.8.1
//-----------------------------------------------------------------------------
std::unique_ptr<Project> ManualImporter::Import()
{
    std::unique_ptr<Project> project = std::make_unique<Project>();

    // 添加任务，对应样例 T/M 行；TaskId 由 Project 自动生成，
    // 按添加顺序依次为 1~6，与样例文件中的任务 ID 一致
    TaskId requirementId = project->AddTask("Requirement", 5);
    TaskId designId      = project->AddTask("Design", 3);
    TaskId codingId      = project->AddTask("Coding", 7);
    TaskId testingId     = project->AddTask("Testing", 4);
    TaskId deploymentId  = project->AddTask("Deployment", 2);
    // 工期 0 自动识别为里程碑
    TaskId acceptanceId = project->AddTask("Acceptance", 0);

    // 添加资源，对应样例 R 行；ResourceId 由 Project 独立自增，
    // 内存中为 1~5，与样例文件中的资源 ID（101~105）不同
    ResourceId architectId   = project->AddResource("Architect", 100.0);
    ResourceId seniorDevId   = project->AddResource("SeniorDev", 80.0);
    ResourceId juniorDevId   = project->AddResource("JuniorDev", 50.0);
    ResourceId testerId      = project->AddResource("Tester", 60.0);
    ResourceId meetingRoomId = project->AddResource("MeetingRoom", 400.0);

    // 添加依赖关系，对应样例 D 行
    project->AddDependency(requirementId, designId, DependencyType::FS, 0);
    project->AddDependency(designId, codingId, DependencyType::FS, 2);
    project->AddDependency(codingId, testingId, DependencyType::FS, -1);
    project->AddDependency(testingId, deploymentId, DependencyType::FS, 0);
    project->AddDependency(codingId, acceptanceId, DependencyType::FS, 0);

    // 添加资源分配，对应样例 A 行
    project->AssignResource(requirementId, architectId, 1);
    project->AssignResource(requirementId, meetingRoomId, 3);
    project->AssignResource(designId, seniorDevId, 2);
    project->AssignResource(codingId, seniorDevId, 2);
    project->AssignResource(codingId, juniorDevId, 1);
    project->AssignResource(testingId, testerId, 2);
    project->AssignResource(deploymentId, juniorDevId, 1);

    return project;
}
