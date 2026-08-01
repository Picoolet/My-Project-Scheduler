//-----------------------------------------------------------------------------
// 【Project.cpp】
// 【项目聚合根类实现】
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------

#include "Project.hpp"

#include <algorithm>
#include <utility>

//=============================================================================
// 只读接口
//=============================================================================

//-----------------------------------------------------------------------------
// 【Project::GetName】
// 【函数功能】获取项目名称
// 【参数】无
// 【返回值】项目名称的常量引用
// 【开发者及日期】 QJQ 2026.8.1
//-----------------------------------------------------------------------------
const std::string& Project::GetName() const
{
    return m_projectName;
}

//-----------------------------------------------------------------------------
// 【Project::TaskCount】
// 【函数功能】获取项目中的任务总数
// 【参数】无
// 【返回值】任务数量
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
size_t Project::TaskCount() const
{
    return m_tasks.size();
}

//-----------------------------------------------------------------------------
// 【Project::DependencyCount】
// 【函数功能】获取项目中的依赖关系总数
// 【参数】无
// 【返回值】依赖关系数量
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
size_t Project::DependencyCount() const
{
    return m_dependencies.size();
}

//-----------------------------------------------------------------------------
// 【Project::ResourceCount】
// 【函数功能】获取项目中的资源总数
// 【参数】无
// 【返回值】资源数量
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
size_t Project::ResourceCount() const
{
    return m_resources.size();
}

//-----------------------------------------------------------------------------
// 【Project::AllocationCount】
// 【函数功能】获取项目中的资源分配记录总数
// 【参数】无
// 【返回值】分配记录数量
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
size_t Project::AllocationCount() const
{
    return m_allocations.size();
}

//-----------------------------------------------------------------------------
// 【Project::FindTask】
// 【函数功能】按 ID 查找任务
// 【参数】id — 输入参数，要查找的任务 ID
// 【返回值】找到返回任务指针，未找到返回 nullptr
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
const Task* Project::FindTask(TaskId id) const
{
    for (const auto& task : m_tasks)
    {
        if (task.GetId() == id)
        {
            return &task;
        }
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
// 【Project::FindResource】
// 【函数功能】按 ID 查找资源
// 【参数】id — 输入参数，要查找的资源 ID
// 【返回值】找到返回资源指针，未找到返回 nullptr
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
const Resource* Project::FindResource(ResourceId id) const
{
    for (const auto& resource : m_resources)
    {
        if (resource.GetId() == id)
        {
            return &resource;
        }
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
// 【Project::FindDependency】
// 【函数功能】按前后置任务 ID 查找依赖关系
// 【参数】pred — 输入参数，前序任务 ID
//        succ — 输入参数，后继任务 ID
// 【返回值】找到返回依赖指针，未找到返回 nullptr
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
const Dependency* Project::FindDependency(TaskId pred, TaskId succ) const
{
    for (const auto& dep : m_dependencies)
    {
        if ((dep.GetPredecessorId() == pred) && (dep.GetSuccessorId() == succ))
        {
            return &dep;
        }
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
// 【Project::GetPredecessors】
// 【函数功能】获取指定任务的所有前驱任务 ID 列表，基于内部索引 O(1) 查找
// 【参数】id — 输入参数，目标任务 ID
// 【返回值】前驱任务 ID 列表，若任务无前驱则返回空列表
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
std::vector<TaskId> Project::GetPredecessors(TaskId id) const
{
    auto iter = m_predecessors.find(id);

    if (iter != m_predecessors.end())
    {
        return iter->second;
    }

    return {};
}

//-----------------------------------------------------------------------------
// 【Project::GetSuccessors】
// 【函数功能】获取指定任务的所有后继任务 ID 列表，基于内部索引 O(1) 查找
// 【参数】id — 输入参数，目标任务 ID
// 【返回值】后继任务 ID 列表，若任务无后继则返回空列表
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
std::vector<TaskId> Project::GetSuccessors(TaskId id) const
{
    auto iter = m_successors.find(id);

    if (iter != m_successors.end())
    {
        return iter->second;
    }

    return {};
}

//-----------------------------------------------------------------------------
// 【Project::GetAllocationsForTask】
// 【函数功能】获取指定任务的所有资源分配记录
// 【参数】id — 输入参数，目标任务 ID
// 【返回值】指向分配记录的 const 指针列表
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
std::vector<const Allocation*> Project::GetAllocationsForTask(TaskId id) const
{
    std::vector<const Allocation*> result;

    for (const auto& alloc : m_allocations)
    {
        if (alloc.GetTaskId() == id)
        {
            result.push_back(&alloc);
        }
    }

    return result;
}

//=============================================================================
// 修改接口
//=============================================================================

//-----------------------------------------------------------------------------
// 【Project::SetName】
// 【函数功能】修改项目名称
// 【参数】newName — 输入参数，新的项目名称
// 【返回值】无
// 【开发者及日期】 QJQ 2026.8.1
//-----------------------------------------------------------------------------
void Project::SetName(const std::string& newName)
{
    m_projectName = newName;
}

//-----------------------------------------------------------------------------
// 【Project::AddTask】
// 【函数功能】创建新任务并添加到项目中，自动生成唯一 TaskId
// 【参数】name — 输入参数，任务名称（唯一性由调用方保证）
//        duration — 输入参数，任务工期（>= 0，由调用方保证）
// 【返回值】新生成的任务 TaskId
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
TaskId Project::AddTask(const std::string& name, int duration)
{
    TaskId newId = GenerateTaskId();
    m_tasks.emplace_back(newId, name, duration);
    return newId;
}

//-----------------------------------------------------------------------------
// 【Project::AddTask（显式 ID 重载）】
// 【函数功能】以显式 ID 创建新任务，用于导入场景忠实保留文件中的 ID
// 【参数】id — 输入参数，任务 ID（-1 视为无效，且不得与现有任务重复）
//        name — 输入参数，任务名称（唯一性由调用方保证）
//        duration — 输入参数，任务工期（>= 0，由调用方保证）
// 【返回值】成功返回该 ID，失败（ID 为 Invalid() 或已被占用）返回 Invalid
// 【开发者及日期】 QJQ 2026.8.1
//-----------------------------------------------------------------------------
TaskId Project::AddTask(TaskId id, const std::string& name, int duration)
{
    // ID 为 Invalid()（-1 哨兵）或已被占用时忽略本次插入
    if ((id == TaskId::Invalid()) || (FindTask(id) != nullptr))
    {
        return TaskId::Invalid();
    }

    // 同步自增计数器，保证后续自动生成的 ID 不与显式 ID 冲突
    if (id.Value() > m_iNextTaskId)
    {
        m_iNextTaskId = id.Value();
    }

    m_tasks.emplace_back(id, name, duration);
    return id;
}

//-----------------------------------------------------------------------------
// 【Project::RemoveTask】
// 【函数功能】删除指定任务，级联删除关联 Dependency 与 Allocation，
//            并更新邻接索引
// 【参数】id — 输入参数，要删除的任务 ID
// 【返回值】无
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
void Project::RemoveTask(TaskId id)
{
    // 从邻接索引中移除该任务
    RemoveFromIndex(id);

    // 移除与该任务相关的所有依赖关系
    m_dependencies.erase(
        std::remove_if(m_dependencies.begin(), m_dependencies.end(),
                       [id](const Dependency& dep)
                       {
                           return (dep.GetPredecessorId() == id)
                                  || (dep.GetSuccessorId() == id);
                       }),
        m_dependencies.end());

    // 移除与该任务相关的所有资源分配记录
    m_allocations.erase(std::remove_if(m_allocations.begin(),
                                       m_allocations.end(),
                                       [id](const Allocation& alloc)
                                       { return (alloc.GetTaskId() == id); }),
                        m_allocations.end());

    // 移除任务本身
    m_tasks.erase(std::remove_if(m_tasks.begin(), m_tasks.end(),
                                 [id](const Task& task)
                                 { return (task.GetId() == id); }),
                  m_tasks.end());
}

//-----------------------------------------------------------------------------
// 【Project::AddResource】
// 【函数功能】创建新资源并添加到项目中，自动生成唯一 ResourceId
// 【参数】name — 输入参数，资源名称
//        unitCost — 输入参数，单位时间成本
// 【返回值】新生成的资源 ResourceId
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
ResourceId Project::AddResource(const std::string& name, double unitCost)
{
    ResourceId newId = GenerateResourceId();
    m_resources.emplace_back(newId, name, unitCost);
    return newId;
}

//-----------------------------------------------------------------------------
// 【Project::AddResource（显式 ID 重载）】
// 【函数功能】以显式 ID 创建资源，用于导入场景忠实保留文件中的 ID
// 【参数】id — 输入参数，资源 ID（-1 视为无效，且不得与现有资源重复）
//        name — 输入参数，资源名称（唯一性由调用方保证）
//        unitCost — 输入参数，单位时间成本
// 【返回值】成功返回该 ID，失败（ID 为 Invalid() 或已被占用）返回 Invalid
// 【开发者及日期】 QJQ 2026.8.1
//-----------------------------------------------------------------------------
ResourceId Project::AddResource(ResourceId id, const std::string& name,
                                double unitCost)
{
    // ID 为 Invalid()（-1 哨兵）或已被占用时忽略本次插入
    if ((id == ResourceId::Invalid()) || (FindResource(id) != nullptr))
    {
        return ResourceId::Invalid();
    }

    // 同步自增计数器，保证后续自动生成的 ID 不与显式 ID 冲突
    if (id.Value() > m_iNextResourceId)
    {
        m_iNextResourceId = id.Value();
    }

    m_resources.emplace_back(id, name, unitCost);
    return id;
}

//-----------------------------------------------------------------------------
// 【Project::RemoveResource】
// 【函数功能】删除指定资源，级联删除关联的 Allocation
// 【参数】id — 输入参数，要删除的资源 ID
// 【返回值】无
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
void Project::RemoveResource(ResourceId id)
{
    // 移除与该资源相关的所有分配记录
    m_allocations.erase(
        std::remove_if(m_allocations.begin(), m_allocations.end(),
                       [id](const Allocation& alloc)
                       { return (alloc.GetResourceId() == id); }),
        m_allocations.end());

    // 移除资源本身
    m_resources.erase(std::remove_if(m_resources.begin(), m_resources.end(),
                                     [id](const Resource& res)
                                     { return (res.GetId() == id); }),
                      m_resources.end());
}

//-----------------------------------------------------------------------------
// 【Project::AddDependency】
// 【函数功能】添加任务间的依赖关系，若相同 (pred, succ) 已存在则忽略
// 【参数】pred — 输入参数，前序任务 ID
//        succ — 输入参数，后继任务 ID
//        type — 输入参数，依赖类型
//        lag — 输入参数，时差
// 【返回值】无
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
void Project::AddDependency(TaskId pred, TaskId succ, DependencyType type,
                            int lag)
{
    // 若相同 (pred, succ) 已存在则忽略
    if (FindDependency(pred, succ) != nullptr)
    {
        return;
    }

    m_dependencies.emplace_back(pred, succ, type, lag);
    AddToIndex(pred, succ);
}

//-----------------------------------------------------------------------------
// 【Project::AssignResource】
// 【函数功能】为任务分配资源（upsert 语义）
// 【参数】taskId — 输入参数，目标任务 ID
//        resourceId — 输入参数，目标资源 ID
//        quantity — 输入参数，占用数量；若 <= 0 则删除该分配记录
// 【返回值】无
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
void Project::AssignResource(TaskId taskId, ResourceId resourceId, int quantity)
{
    // 查找已存在的分配记录
    for (auto& alloc : m_allocations)
    {
        if ((alloc.GetTaskId() == taskId)
            && (alloc.GetResourceId() == resourceId))
        {
            if (quantity <= 0)
            {
                // 删除该分配记录
                m_allocations.erase(
                    std::remove_if(m_allocations.begin(), m_allocations.end(),
                                   [taskId, resourceId](const Allocation& a)
                                   {
                                       return (a.GetTaskId() == taskId)
                                              && (a.GetResourceId()
                                                  == resourceId);
                                   }),
                    m_allocations.end());
            }
            else
            {
                // 更新数量（upsert）
                alloc.SetQuantity(quantity);
            }

            return;
        }
    }

    // 不存在且 quantity > 0，新增分配记录
    if (quantity > 0)
    {
        m_allocations.emplace_back(taskId, resourceId, quantity);
    }
}

//=============================================================================
// 私有辅助方法
//=============================================================================

//-----------------------------------------------------------------------------
// 【Project::AddToIndex】
// 【函数功能】在邻接索引中添加一条前驱-后继关系
// 【参数】pred — 输入参数，前序任务 ID
//        succ — 输入参数，后继任务 ID
// 【返回值】无
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
void Project::AddToIndex(TaskId pred, TaskId succ)
{
    m_successors[pred].push_back(succ);
    m_predecessors[succ].push_back(pred);
}

//-----------------------------------------------------------------------------
// 【Project::RemoveFromIndex】
// 【函数功能】从前驱/后继索引中移除与指定任务相关的所有条目
// 【参数】id — 输入参数，要移除的任务 ID
// 【返回值】无
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
void Project::RemoveFromIndex(TaskId id)
{
    // 从所有前驱的后继列表中移除此任务
    auto predIter = m_predecessors.find(id);

    if (predIter != m_predecessors.end())
    {
        for (TaskId pred : predIter->second)
        {
            auto& succList = m_successors[pred];
            succList.erase(std::remove(succList.begin(), succList.end(), id),
                           succList.end());
        }

        m_predecessors.erase(predIter);
    }

    // 从所有后继的前驱列表中移除此任务
    auto succIter = m_successors.find(id);

    if (succIter != m_successors.end())
    {
        for (TaskId succ : succIter->second)
        {
            auto& predList = m_predecessors[succ];
            predList.erase(std::remove(predList.begin(), predList.end(), id),
                           predList.end());
        }

        m_successors.erase(succIter);
    }
}

//-----------------------------------------------------------------------------
// 【Project::GenerateTaskId】
// 【函数功能】生成自增的唯一 TaskId
// 【参数】无
// 【返回值】新生成的 TaskId
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
TaskId Project::GenerateTaskId()
{
    int nextId    = m_iNextTaskId + 1;
    m_iNextTaskId = nextId;
    return TaskId(nextId);
}

//-----------------------------------------------------------------------------
// 【Project::GenerateResourceId】
// 【函数功能】生成自增的唯一 ResourceId
// 【参数】无
// 【返回值】新生成的 ResourceId
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
ResourceId Project::GenerateResourceId()
{
    int nextId        = m_iNextResourceId + 1;
    m_iNextResourceId = nextId;
    return ResourceId(nextId);
}
