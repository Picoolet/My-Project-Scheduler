//-----------------------------------------------------------------------------
// 【ScheduleResult.cpp】
// 【调度结果类实现】
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------

#include "ScheduleResult.hpp"

#include <utility>

//-----------------------------------------------------------------------------
// 【ScheduleResult::ScheduleResult】
// 【函数功能】构造函数，保存 CPM 计算的三项结果
// 【参数】data — 输入参数，各任务时间信息映射（移动语义）
//        totalDuration — 输入参数，项目总工期
//        criticalPath — 输入参数，关键路径任务 ID 列表（移动语义）
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
ScheduleResult::ScheduleResult(
    std::unordered_map<TaskId, TaskScheduleInfo> data, int totalDuration,
    std::vector<TaskId> criticalPath)
    : m_data(std::move(data)), m_iTotalDuration(totalDuration),
      m_criticalPath(std::move(criticalPath))
{
}

//-----------------------------------------------------------------------------
// 【ScheduleResult::GetFieldValue】
// 【函数功能】按 ID 查找任务信息并提取指定字段值，ID 不存在时返回 0
// 【参数】id — 输入参数，目标任务 ID
//        field — 输入参数，指向 TaskScheduleInfo 成员字段的指针
// 【返回值】字段值；若 id 不在 m_data 中则返回 0
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
int ScheduleResult::GetFieldValue(TaskId id,
                                  int TaskScheduleInfo::* field) const
{
    auto iter = m_data.find(id);

    if (iter != m_data.end())
    {
        return (iter->second.*field);
    }

    return 0;
}

//-----------------------------------------------------------------------------
// 【ScheduleResult::GetTotalDuration】
// 【函数功能】获取项目总工期
// 【参数】无
// 【返回值】总工期（int）
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
int ScheduleResult::GetTotalDuration() const
{
    return m_iTotalDuration;
}

//-----------------------------------------------------------------------------
// 【ScheduleResult::GetEarlyStart】
// 【函数功能】获取指定任务的最早开始时间（ES）
// 【参数】id — 输入参数，目标任务 ID
// 【返回值】ES 值；若 id 不在结果中则返回 0
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
int ScheduleResult::GetEarlyStart(TaskId id) const
{
    return GetFieldValue(id, &TaskScheduleInfo::earlyStart);
}

//-----------------------------------------------------------------------------
// 【ScheduleResult::GetEarlyFinish】
// 【函数功能】获取指定任务的最早完成时间（EF）
// 【参数】id — 输入参数，目标任务 ID
// 【返回值】EF 值；若 id 不在结果中则返回 0
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
int ScheduleResult::GetEarlyFinish(TaskId id) const
{
    return GetFieldValue(id, &TaskScheduleInfo::earlyFinish);
}

//-----------------------------------------------------------------------------
// 【ScheduleResult::GetLateStart】
// 【函数功能】获取指定任务的最晚开始时间（LS）
// 【参数】id — 输入参数，目标任务 ID
// 【返回值】LS 值；若 id 不在结果中则返回 0
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
int ScheduleResult::GetLateStart(TaskId id) const
{
    return GetFieldValue(id, &TaskScheduleInfo::lateStart);
}

//-----------------------------------------------------------------------------
// 【ScheduleResult::GetLateFinish】
// 【函数功能】获取指定任务的最晚完成时间（LF）
// 【参数】id — 输入参数，目标任务 ID
// 【返回值】LF 值；若 id 不在结果中则返回 0
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
int ScheduleResult::GetLateFinish(TaskId id) const
{
    return GetFieldValue(id, &TaskScheduleInfo::lateFinish);
}

//-----------------------------------------------------------------------------
// 【ScheduleResult::GetCriticalPath】
// 【函数功能】获取关键路径任务 ID 列表
// 【参数】无
// 【返回值】关键路径 ID 列表的 const 引用（按拓扑序排列）
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
const std::vector<TaskId>& ScheduleResult::GetCriticalPath() const
{
    return m_criticalPath;
}
