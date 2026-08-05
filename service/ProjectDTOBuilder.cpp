//-----------------------------------------------------------------------------
// 【ProjectDTOBuilder.cpp】
// 【DTO 构建器类实现】
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------

#include "ProjectDTOBuilder.hpp"

#include <unordered_map>

#include "Allocation.hpp"
#include "CPMCalculator.hpp"
#include "Dependency.hpp"
#include "Id.hpp"
#include "Project.hpp"
#include "ProjectValidator.hpp"
#include "Resource.hpp"
#include "ScheduleResult.hpp"
#include "Task.hpp"

//-----------------------------------------------------------------------------
// 【ProjectDTOBuilder::BuildTaskDTOs】
// 【函数功能】按容器索引顺序构建全部 TaskDTO
// 【参数】project — 输入参数，目标项目
// 【返回值】TaskDTO 列表（下标即容器索引）
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
std::vector<TaskDTO> ProjectDTOBuilder::BuildTaskDTOs(
    const Project& project) const
{
    std::vector<TaskDTO> result;
    const auto&          tasks = project.GetTasks();

    for (size_t i = 0; i < tasks.size(); ++i)
    {
        int index = static_cast<int>(i);
        result.push_back(BuildSingleTaskDTO(project, index, tasks[i]));
    }

    return result;
}

//-----------------------------------------------------------------------------
// 【ProjectDTOBuilder::BuildSingleTaskDTO】
// 【函数功能】构建单个任务的 TaskDTO（含前驱/后继索引列表）
// 【参数】project — 输入参数，目标项目
//        index — 输入参数，该任务在容器中的索引
//        task — 输入参数，任务引用
// 【返回值】填充完整的 TaskDTO
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
TaskDTO ProjectDTOBuilder::BuildSingleTaskDTO(const Project& project, int index,
                                              const Task& task) const
{
    TaskDTO dto;
    dto.index    = index;
    dto.idValue  = task.GetId().Value();
    dto.name     = task.GetName();
    dto.duration = task.GetDuration();

    // 建立 TaskId → index 反向映射
    std::unordered_map<TaskId, int> idToIndex;
    const auto&                     tasks = project.GetTasks();

    for (size_t i = 0; i < tasks.size(); ++i)
    {
        idToIndex[tasks[i].GetId()] = static_cast<int>(i);
    }

    // 填充前驱索引列表
    for (TaskId predId : project.GetPredecessors(task.GetId()))
    {
        auto iter = idToIndex.find(predId);

        if (iter != idToIndex.end())
        {
            dto.predecessorIndices.push_back(iter->second);
        }
    }

    // 填充后继索引列表
    for (TaskId succId : project.GetSuccessors(task.GetId()))
    {
        auto iter = idToIndex.find(succId);

        if (iter != idToIndex.end())
        {
            dto.successorIndices.push_back(iter->second);
        }
    }

    return dto;
}

//-----------------------------------------------------------------------------
// 【ProjectDTOBuilder::BuildTaskRelations】
// 【函数功能】返回指定任务的前驱与后继信息
// 【参数】project — 输入参数，目标项目
//        index — 输入参数，目标任务容器索引
// 【返回值】pair<前驱 DTO 列表, 后继 DTO 列表>；越界返回空 pair
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
std::pair<std::vector<TaskDTO>, std::vector<TaskDTO>> ProjectDTOBuilder::
    BuildTaskRelations(const Project& project, int index) const
{
    std::pair<std::vector<TaskDTO>, std::vector<TaskDTO>> result;

    const auto& tasks = project.GetTasks();

    if ((index < 0) || (static_cast<size_t>(index) >= tasks.size()))
    {
        return result;
    }

    const Task& task = tasks[static_cast<size_t>(index)];

    // 建立 TaskId → index 反向映射
    std::unordered_map<TaskId, int> idToIndex;

    for (size_t i = 0; i < tasks.size(); ++i)
    {
        idToIndex[tasks[i].GetId()] = static_cast<int>(i);
    }

    // 填充前驱 DTO 列表
    for (TaskId predId : project.GetPredecessors(task.GetId()))
    {
        auto iter = idToIndex.find(predId);

        if (iter != idToIndex.end())
        {
            const Task* predTask = project.FindTask(predId);

            if (predTask != nullptr)
            {
                result.first.push_back(
                    BuildSingleTaskDTO(project, iter->second, *predTask));
            }
        }
    }

    // 填充后继 DTO 列表
    for (TaskId succId : project.GetSuccessors(task.GetId()))
    {
        auto iter = idToIndex.find(succId);

        if (iter != idToIndex.end())
        {
            const Task* succTask = project.FindTask(succId);

            if (succTask != nullptr)
            {
                result.second.push_back(
                    BuildSingleTaskDTO(project, iter->second, *succTask));
            }
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
// 【ProjectDTOBuilder::BuildDependencyDTOs】
// 【函数功能】按序号构建全部 DependencyDTO
// 【参数】project — 输入参数，目标项目
// 【返回值】DependencyDTO 列表（下标即序号）
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
std::vector<DependencyDTO> ProjectDTOBuilder::BuildDependencyDTOs(
    const Project& project) const
{
    std::vector<DependencyDTO> result;
    const auto&                deps = project.GetDependencies();

    // 建立 TaskId → index 反向映射
    std::unordered_map<TaskId, int> idToIndex;
    const auto&                     tasks = project.GetTasks();

    for (size_t i = 0; i < tasks.size(); ++i)
    {
        idToIndex[tasks[i].GetId()] = static_cast<int>(i);
    }

    for (size_t i = 0; i < deps.size(); ++i)
    {
        const Dependency& dep = deps[i];

        DependencyDTO dto;
        dto.index = static_cast<int>(i);
        dto.type  = dep.GetType();
        dto.lag   = dep.GetLag();

        auto predIter = idToIndex.find(dep.GetPredecessorId());

        if (predIter != idToIndex.end())
        {
            dto.predecessorIndex = predIter->second;
        }
        else
        {
            dto.predecessorIndex = -1;
        }

        auto succIter = idToIndex.find(dep.GetSuccessorId());

        if (succIter != idToIndex.end())
        {
            dto.successorIndex = succIter->second;
        }
        else
        {
            dto.successorIndex = -1;
        }

        result.push_back(dto);
    }

    return result;
}

//-----------------------------------------------------------------------------
// 【ProjectDTOBuilder::BuildResourceDTOs】
// 【函数功能】按容器索引顺序构建全部 ResourceDTO
// 【参数】project — 输入参数，目标项目
// 【返回值】ResourceDTO 列表（下标即索引）
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
std::vector<ResourceDTO> ProjectDTOBuilder::BuildResourceDTOs(
    const Project& project) const
{
    std::vector<ResourceDTO> result;
    const auto&              resources = project.GetResources();

    for (size_t i = 0; i < resources.size(); ++i)
    {
        const Resource& res = resources[i];

        ResourceDTO dto;
        dto.index    = static_cast<int>(i);
        dto.idValue  = res.GetId().Value();
        dto.name     = res.GetName();
        dto.unitCost = res.GetUnitCost();
        result.push_back(dto);
    }

    return result;
}

//-----------------------------------------------------------------------------
// 【ProjectDTOBuilder::BuildTaskResources】
// 【函数功能】返回指定任务已分配的资源 DTO 列表
// 【参数】project — 输入参数，目标项目
//        taskIndex — 输入参数，任务容器索引
// 【返回值】ResourceDTO 列表；越界或任务无分配时返回空列表
// 【开发者及日期】QJQ 2026.8.6
// 【更改记录】 无
//-----------------------------------------------------------------------------
std::vector<ResourceDTO> ProjectDTOBuilder::BuildTaskResources(
    const Project& project, int taskIndex) const
{
    std::vector<ResourceDTO> result;
    const auto&              tasks = project.GetTasks();

    if ((taskIndex < 0) || (static_cast<size_t>(taskIndex) >= tasks.size()))
    {
        return result;
    }

    TaskId taskId = tasks[static_cast<size_t>(taskIndex)].GetId();

    // 建立 ResourceId → index 映射
    std::unordered_map<ResourceId, int> resIdToIndex;
    const auto&                         resources = project.GetResources();

    for (size_t i = 0; i < resources.size(); ++i)
    {
        resIdToIndex[resources[i].GetId()] = static_cast<int>(i);
    }

    for (const Allocation* alloc : project.GetAllocationsForTask(taskId))
    {
        const Resource* res = project.FindResource(alloc->GetResourceId());

        if (res == nullptr)
        {
            continue;
        }

        ResourceDTO dto;
        dto.index    = resIdToIndex[res->GetId()];
        dto.idValue  = res->GetId().Value();
        dto.name     = res->GetName();
        dto.unitCost = res->GetUnitCost();
        result.push_back(dto);
    }

    return result;
}

//-----------------------------------------------------------------------------
// 【ProjectDTOBuilder::BuildStatistics】
// 【函数功能】构建项目统计信息（内部执行 Validate 并视结果执行 CPM Calculate）
// 【参数】project — 输入参数，目标项目
//        validator — 输入参数，验证器引用
//        calculator — 输入参数，CPM 计算器引用
// 【返回值】ProjectStatisticsDTO（含 isValid 标志和 totalDuration）
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
ProjectStatisticsDTO ProjectDTOBuilder::BuildStatistics(
    const Project& project, const ProjectValidator& validator,
    const CPMCalculator& calculator) const
{
    ProjectStatisticsDTO stats;
    stats.taskCount       = static_cast<int>(project.TaskCount());
    stats.dependencyCount = static_cast<int>(project.DependencyCount());
    stats.resourceCount   = static_cast<int>(project.ResourceCount());

    ValidationResult vr = validator.Validate(project);
    stats.isValid       = vr.IsValid();

    if (stats.isValid == true)
    {
        stats.totalDuration = calculator.Calculate(project).GetTotalDuration();
    }
    else
    {
        stats.totalDuration = -1;
    }

    return stats;
}
