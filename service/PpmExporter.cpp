//-----------------------------------------------------------------------------
// 【PpmExporter.cpp】
// 【PPM 格式导出器类实现】
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------

#include "PpmExporter.hpp"

#include <fstream>
#include <string>

#include "Allocation.hpp"
#include "Dependency.hpp"
#include "DependencyType.hpp"
#include "Project.hpp"
#include "Resource.hpp"
#include "Task.hpp"

namespace
{

//-----------------------------------------------------------------------------
// 【DependencyTypeToString】
// 【函数功能】将 DependencyType 转为字符串
// 【参数】type — 输入参数，依赖类型
// 【返回值】"FS" / "SS" / "FF" / "SF"
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
std::string DependencyTypeToString(DependencyType type)
{
    switch (type)
    {
    case DependencyType::SS:
        return "SS";

    case DependencyType::FF:
        return "FF";

    case DependencyType::SF:
        return "SF";

    default:
        return "FS";
    }
}

} // anonymous namespace

//-----------------------------------------------------------------------------
// 【PpmExporter::Export】
// 【函数功能】将 Project 按 PPM 格式写入指定路径文件
// 【参数】project — 输入参数，待导出的项目
//        path — 输入参数，输出文件路径
// 【返回值】true — 导出成功
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
bool PpmExporter::Export(const Project& project, const std::string& path)
{
    std::ofstream file(path);

    if (file.is_open() == false)
    {
        return false;
    }

    // 注释行：项目名称
    file << "# " << project.GetName() << "\n";

    // P 行：项目名称
    file << "P " << project.GetName() << "\n";

    // T 行：duration > 0 的任务
    for (const Task& task : project.GetTasks())
    {
        if (task.GetDuration() > 0)
        {
            file << "T " << task.GetId().Value() << " " << task.GetName() << " "
                 << task.GetDuration() << "\n";
        }
    }

    // M 行：duration == 0 的里程碑
    for (const Task& task : project.GetTasks())
    {
        if (task.GetDuration() == 0)
        {
            file << "M " << task.GetId().Value() << " " << task.GetName()
                 << " 0\n";
        }
    }

    // R 行：资源列表
    for (const Resource& res : project.GetResources())
    {
        file << "R " << res.GetId().Value() << " " << res.GetName() << " "
             << res.GetUnitCost() << "\n";
    }

    // D 行：依赖列表
    for (const Dependency& dep : project.GetDependencies())
    {
        file << "D " << dep.GetPredecessorId().Value() << " "
             << dep.GetSuccessorId().Value() << " "
             << DependencyTypeToString(dep.GetType()) << " " << dep.GetLag()
             << "\n";
    }

    // A 行：分配记录
    for (const Allocation& alloc : project.GetAllocations())
    {
        file << "A " << alloc.GetTaskId().Value() << " "
             << alloc.GetResourceId().Value() << " " << alloc.GetQuantity()
             << "\n";
    }

    return true;
}
