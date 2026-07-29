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

size_t Project::TaskCount() const
{
    return m_tasks.size();
}

size_t Project::DependencyCount() const
{
    return m_dependencies.size();
}

size_t Project::ResourceCount() const
{
    return m_resources.size();
}

size_t Project::AllocationCount() const
{
    return m_allocations.size();
}

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
TaskId Project::AddTask(const std::string& name, int duration)
{
    TaskId newId = GenerateTaskId();
    m_tasks.emplace_back(newId, name, duration);
    return newId;
}

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
ResourceId Project::AddResource(const std::string& name, double unitCost)
{
    ResourceId newId = GenerateResourceId();
    m_resources.emplace_back(newId, name, unitCost);
    return newId;
}

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
void Project::AddToIndex(TaskId pred, TaskId succ)
{
    m_successors[pred].push_back(succ);
    m_predecessors[succ].push_back(pred);
}

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
TaskId Project::GenerateTaskId()
{
    unsigned int nextId = m_uNextTaskId + 1U;
    m_uNextTaskId       = nextId;
    return TaskId(nextId);
}

//-----------------------------------------------------------------------------
ResourceId Project::GenerateResourceId()
{
    unsigned int nextId = m_uNextResourceId + 1U;
    m_uNextResourceId   = nextId;
    return ResourceId(nextId);
}
