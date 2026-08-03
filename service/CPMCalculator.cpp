//-----------------------------------------------------------------------------
// 【CPMCalculator.cpp】
// 【关键路径计算器类实现】
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------

#include "CPMCalculator.hpp"

#include <queue>
#include <unordered_map>
#include <vector>

#include "Dependency.hpp"
#include "DependencyType.hpp"
#include "Project.hpp"
#include "ScheduleResult.hpp"
#include "Task.hpp"

//-----------------------------------------------------------------------------
// 【CPMCalculator::Calculate】
// 【函数功能】执行 CPM 五步算法：拓扑排序 + 前向 + 后向 + 关键路径 + 构造结果
// 【参数】project — 输入参数，待计算的项目
// 【返回值】ScheduleResult，自防御（有环或引用断裂时返回空结果）
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
ScheduleResult CPMCalculator::Calculate(const Project& project) const
{
    size_t taskCount = project.TaskCount();

    // 空项目：返回空结果
    if (taskCount == 0)
    {
        return ScheduleResult({}, 0, {});
    }

    //=====================================================================
    // Step 1 — 拓扑排序（Kahn 算法，含自防御）
    //=====================================================================

    std::unordered_map<TaskId, int> indegree;
    std::vector<TaskId>             topoOrder;

    for (const Task& task : project.GetTasks())
    {
        indegree[task.GetId()] = static_cast<int>(
            project.GetPredecessors(task.GetId()).size());
    }

    std::queue<TaskId> taskQueue;

    for (const auto& pair : indegree)
    {
        if (pair.second == 0)
        {
            taskQueue.push(pair.first);
        }
    }

    while (taskQueue.empty() == false)
    {
        TaskId current = taskQueue.front();
        taskQueue.pop();
        topoOrder.push_back(current);

        for (TaskId succ : project.GetSuccessors(current))
        {
            // 自防御：后继任务不存在 → 引用断裂
            if (project.FindTask(succ) == nullptr)
            {
                return ScheduleResult({}, 0, {});
            }

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

    // 已处理节点 < 总节点 → 有环
    if (topoOrder.size() < taskCount)
    {
        return ScheduleResult({}, 0, {});
    }

    //=====================================================================
    // Step 2 — 前向传播（按拓扑序）
    //=====================================================================

    std::unordered_map<TaskId, int> es; // 最早开始
    std::unordered_map<TaskId, int> ef; // 最早完成

    for (TaskId id : topoOrder)
    {
        int dur = project.FindTask(id)->GetDuration();

        // 无前驱的任务 ES = 0（es[id] 默认值即为 0）
        auto esIter = es.find(id);

        if (esIter == es.end())
        {
            es[id] = 0;
        }

        ef[id] = es[id] + dur;

        // 向后继传播候选 ES
        for (TaskId succ : project.GetSuccessors(id))
        {
            const Dependency* dep = project.FindDependency(id, succ);

            if (dep == nullptr)
            {
                continue;
            }

            int succDur     = project.FindTask(succ)->GetDuration();
            int candidateES = 0;

            switch (dep->GetType())
            {
            case DependencyType::FS:
                candidateES = ef[id] + dep->GetLag();
                break;

            case DependencyType::SS:
                candidateES = es[id] + dep->GetLag();
                break;

            case DependencyType::FF:
                candidateES = ef[id] + dep->GetLag() - succDur;
                break;

            case DependencyType::SF:
                candidateES = es[id] + dep->GetLag() - succDur;
                break;
            }

            auto succEsIter = es.find(succ);

            if ((succEsIter == es.end()) || (candidateES > succEsIter->second))
            {
                es[succ] = candidateES;
            }
        }
    }

    // 总工期 = max(所有 EF)
    int totalDuration = 0;

    for (const auto& pair : ef)
    {
        if (pair.second > totalDuration)
        {
            totalDuration = pair.second;
        }
    }

    //=====================================================================
    // Step 3 — 后向传播（按逆拓扑序）
    //=====================================================================

    std::unordered_map<TaskId, int> lf; // 最晚完成
    std::unordered_map<TaskId, int> ls; // 最晚开始

    for (auto rit = topoOrder.rbegin(); rit != topoOrder.rend(); ++rit)
    {
        TaskId id  = *rit;
        int    dur = project.FindTask(id)->GetDuration();

        // 无后继的任务 LF = totalDuration
        auto lfIter = lf.find(id);

        if (lfIter == lf.end())
        {
            lf[id] = totalDuration;
        }

        ls[id] = lf[id] - dur;

        // 向前驱传播约束
        for (TaskId pred : project.GetPredecessors(id))
        {
            const Dependency* dep = project.FindDependency(pred, id);

            if (dep == nullptr)
            {
                continue;
            }

            int  predDur    = project.FindTask(pred)->GetDuration();
            int  predLF     = 0;
            int  predLS     = 0;
            auto predLfIter = lf.find(pred);

            if (predLfIter != lf.end())
            {
                predLF = predLfIter->second;
                predLS = ls[pred];
            }
            else
            {
                // 初始化为大值
                predLF = totalDuration;
                predLS = totalDuration;
            }

            switch (dep->GetType())
            {
            case DependencyType::FS:
            {
                int constraint = ls[id] - dep->GetLag();

                if (constraint < predLF)
                {
                    lf[pred] = constraint;
                    ls[pred] = lf[pred] - predDur;
                }

                break;
            }

            case DependencyType::SS:
            {
                int constraint = ls[id] - dep->GetLag();

                if (constraint < predLS)
                {
                    ls[pred] = constraint;
                    lf[pred] = ls[pred] + predDur;
                }

                break;
            }

            case DependencyType::FF:
            {
                int constraint = lf[id] - dep->GetLag();

                if (constraint < predLF)
                {
                    lf[pred] = constraint;
                    ls[pred] = lf[pred] - predDur;
                }

                break;
            }

            case DependencyType::SF:
            {
                int constraint = lf[id] - dep->GetLag();

                if (constraint < predLS)
                {
                    ls[pred] = constraint;
                    lf[pred] = ls[pred] + predDur;
                }

                break;
            }
            }
        }
    }

    //=====================================================================
    // Step 4 — 提取关键路径（ES == LS 的任务按 ES 升序排列）
    //=====================================================================

    std::vector<TaskId> criticalPath;

    for (TaskId id : topoOrder)
    {
        if (es[id] == ls[id])
        {
            criticalPath.push_back(id);
        }
    }

    //=====================================================================
    // Step 5 — 构造 ScheduleResult
    //=====================================================================

    std::unordered_map<TaskId, TaskScheduleInfo> scheduleData;

    for (TaskId id : topoOrder)
    {
        TaskScheduleInfo info;
        info.earlyStart  = es[id];
        info.earlyFinish = ef[id];
        info.lateStart   = ls[id];
        info.lateFinish  = lf[id];
        scheduleData[id] = info;
    }

    return ScheduleResult(std::move(scheduleData), totalDuration,
                          std::move(criticalPath));
}

//-----------------------------------------------------------------------------
// 【CPMCalculator::IsCritical】
// 【函数功能】判断指定任务是否在关键路径上（EF == LF）
// 【参数】result — 输入参数，已计算的调度结果
//        id — 输入参数，待判断的任务 ID
// 【返回值】true — 在关键路径上；false — 不在
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
bool CPMCalculator::IsCritical(const ScheduleResult& result, TaskId id) const
{
    return (result.GetEarlyFinish(id) == result.GetLateFinish(id));
}
