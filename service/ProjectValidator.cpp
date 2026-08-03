//-----------------------------------------------------------------------------
// 【ProjectValidator.cpp】
// 【项目合理性验证器类实现】
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------

#include "ProjectValidator.hpp"

#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "Dependency.hpp"
#include "Project.hpp"
#include "Task.hpp"

//-----------------------------------------------------------------------------
// 【ProjectValidator::Validate】
// 【函数功能】依次执行三条检查，收集所有问题后一次性返回
// 【参数】project — 输入参数，待验证的项目
// 【返回值】ValidationResult，IsValid()==true 表示全部通过
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
ValidationResult ProjectValidator::Validate(const Project& project) const
{
    std::vector<std::string> errors;

    CheckAcyclic(project, errors);
    CheckNoDangling(project, errors);
    CheckReferenceIntegrity(project, errors);

    if (errors.empty() == true)
    {
        return ValidationResult();
    }

    return ValidationResult(errors);
}

//-----------------------------------------------------------------------------
// 【ProjectValidator::CheckAcyclic】
// 【函数功能】使用 Kahn 算法检测依赖图是否存在环路
// 【参数】project — 输入参数，待检查的项目
//        errors — 输出参数，错误信息列表
// 【返回值】无，若有环则追加错误到 errors
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
void ProjectValidator::CheckAcyclic(const Project&            project,
                                    std::vector<std::string>& errors) const
{
    size_t taskCount = project.TaskCount();

    if (taskCount == 0)
    {
        return;
    }

    // 统计每个 TaskId 的入度
    std::unordered_map<TaskId, int> indegree;

    for (const Task& task : project.GetTasks())
    {
        indegree[task.GetId()] = static_cast<int>(
            project.GetPredecessors(task.GetId()).size());
    }

    // Kahn 算法：入度为零者入队
    std::queue<TaskId> taskQueue;

    for (const auto& pair : indegree)
    {
        if (pair.second == 0)
        {
            taskQueue.push(pair.first);
        }
    }

    size_t processedCount = 0;

    while (taskQueue.empty() == false)
    {
        TaskId current = taskQueue.front();
        taskQueue.pop();
        ++processedCount;

        for (TaskId succ : project.GetSuccessors(current))
        {
            auto iter = indegree.find(succ);

            if (iter != indegree.end())
            {
                --(iter->second);

                if (iter->second == 0)
                {
                    taskQueue.push(succ);
                }
            }
        }
    }

    if (processedCount < taskCount)
    {
        errors.push_back("依赖图中存在循环依赖");
    }
}

//-----------------------------------------------------------------------------
// 【ProjectValidator::CheckNoDangling】
// 【函数功能】检测依赖图中是否存在悬挂节点（不可从起始到达或无法到达终止）
// 【参数】project — 输入参数，待检查的项目
//        errors — 输出参数，错误信息列表
// 【返回值】无，若有悬挂节点则追加错误到 errors
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
void ProjectValidator::CheckNoDangling(const Project&            project,
                                       std::vector<std::string>& errors) const
{
    size_t taskCount = project.TaskCount();

    if (taskCount == 0)
    {
        return;
    }

    // 找起始集 S（入度=0）和终止集 T（出度=0）
    std::unordered_set<TaskId> startSet;
    std::unordered_set<TaskId> endSet;

    for (const Task& task : project.GetTasks())
    {
        TaskId id = task.GetId();

        if (project.GetPredecessors(id).empty() == true)
        {
            startSet.insert(id);
        }

        if (project.GetSuccessors(id).empty() == true)
        {
            endSet.insert(id);
        }
    }

    // 若存在任务但起始集为空，全部任务均为悬挂节点
    if (startSet.empty() == true)
    {
        for (const Task& task : project.GetTasks())
        {
            errors.push_back("任务 \"" + task.GetName()
                             + "\" 为悬挂节点（不可达）");
        }

        return;
    }

    // 正向 BFS：从起始集沿 GetSuccessors 收集可达集
    std::unordered_set<TaskId> forwardReachable;
    std::queue<TaskId>         forwardQueue;

    for (TaskId start : startSet)
    {
        forwardQueue.push(start);
        forwardReachable.insert(start);
    }

    while (forwardQueue.empty() == false)
    {
        TaskId current = forwardQueue.front();
        forwardQueue.pop();

        for (TaskId succ : project.GetSuccessors(current))
        {
            if (forwardReachable.find(succ) == forwardReachable.end())
            {
                forwardReachable.insert(succ);
                forwardQueue.push(succ);
            }
        }
    }

    // 反向 BFS：从终止集沿 GetPredecessors 收集可达集
    std::unordered_set<TaskId> backwardReachable;
    std::queue<TaskId>         backwardQueue;

    for (TaskId end : endSet)
    {
        backwardQueue.push(end);
        backwardReachable.insert(end);
    }

    while (backwardQueue.empty() == false)
    {
        TaskId current = backwardQueue.front();
        backwardQueue.pop();

        for (TaskId pred : project.GetPredecessors(current))
        {
            if (backwardReachable.find(pred) == backwardReachable.end())
            {
                backwardReachable.insert(pred);
                backwardQueue.push(pred);
            }
        }
    }

    // 不在交集内的任务为悬挂节点
    for (const Task& task : project.GetTasks())
    {
        TaskId id              = task.GetId();
        bool   inForwardRange  = (forwardReachable.find(id)
                                  != forwardReachable.end());
        bool   inBackwardRange = (backwardReachable.find(id)
                                  != backwardReachable.end());

        if ((inForwardRange == false) || (inBackwardRange == false))
        {
            errors.push_back("任务 \"" + task.GetName() + "\" 为悬挂节点");
        }
    }
}

//-----------------------------------------------------------------------------
// 【ProjectValidator::CheckReferenceIntegrity】
// 【函数功能】检测每条依赖的 pred/succ 是否对应存在的任务
// 【参数】project — 输入参数，待检查的项目
//        errors — 输出参数，错误信息列表
// 【返回值】无，若有断引用则追加错误到 errors
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
void ProjectValidator::CheckReferenceIntegrity(
    const Project& project, std::vector<std::string>& errors) const
{
    for (const Dependency& dep : project.GetDependencies())
    {
        if (project.FindTask(dep.GetPredecessorId()) == nullptr)
        {
            errors.push_back("依赖引用的前置任务不存在 (ID="
                             + std::to_string(dep.GetPredecessorId().Value())
                             + ")");
        }

        if (project.FindTask(dep.GetSuccessorId()) == nullptr)
        {
            errors.push_back("依赖引用的后继任务不存在 (ID="
                             + std::to_string(dep.GetSuccessorId().Value())
                             + ")");
        }
    }
}

//-----------------------------------------------------------------------------
// 【ProjectValidator::WouldCreateCycle】
// 【函数功能】判断在现有依赖图上添加 (pred→succ) 是否会形成环
// 【参数】project — 输入参数，当前项目
//        pred — 输入参数，拟添加依赖的前序任务 ID
//        succ — 输入参数，拟添加依赖的后继任务 ID
// 【返回值】true — 添加后会成环；false — 安全
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
bool ProjectValidator::WouldCreateCycle(const Project& project, TaskId pred,
                                        TaskId succ) const
{
    // 从 succ 出发 BFS，若可达 pred 则添加 (pred→succ) 会成环
    std::unordered_set<TaskId> visited;
    std::queue<TaskId>         bfsQueue;

    bfsQueue.push(succ);
    visited.insert(succ);

    while (bfsQueue.empty() == false)
    {
        TaskId current = bfsQueue.front();
        bfsQueue.pop();

        for (TaskId next : project.GetSuccessors(current))
        {
            if (next == pred)
            {
                return true;
            }

            if (visited.find(next) == visited.end())
            {
                visited.insert(next);
                bfsQueue.push(next);
            }
        }
    }

    return false;
}
