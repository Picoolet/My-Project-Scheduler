//-----------------------------------------------------------------------------
// 【ScheduleResult.cpp】
// 【调度结果类实现】
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------

#include "ScheduleResult.hpp"

#include <utility>

ScheduleResult::ScheduleResult(
    std::unordered_map<TaskId, TaskScheduleInfo> data, int totalDuration,
    std::vector<TaskId> criticalPath)
    : m_dataMap(std::move(data)), m_iTotalDuration(totalDuration),
      m_criticalPathVec(std::move(criticalPath))
{
}

int ScheduleResult::GetTotalDuration() const
{
    return m_iTotalDuration;
}

int ScheduleResult::GetEarlyStart(TaskId id) const
{
    auto iter = m_dataMap.find(id);

    if (iter != m_dataMap.end())
    {
        return iter->second.EarlyStart;
    }

    return 0;
}

int ScheduleResult::GetEarlyFinish(TaskId id) const
{
    auto iter = m_dataMap.find(id);

    if (iter != m_dataMap.end())
    {
        return iter->second.EarlyFinish;
    }

    return 0;
}

int ScheduleResult::GetLateStart(TaskId id) const
{
    auto iter = m_dataMap.find(id);

    if (iter != m_dataMap.end())
    {
        return iter->second.LateStart;
    }

    return 0;
}

int ScheduleResult::GetLateFinish(TaskId id) const
{
    auto iter = m_dataMap.find(id);

    if (iter != m_dataMap.end())
    {
        return iter->second.LateFinish;
    }

    return 0;
}

const std::vector<TaskId>& ScheduleResult::GetCriticalPath() const
{
    return m_criticalPathVec;
}
