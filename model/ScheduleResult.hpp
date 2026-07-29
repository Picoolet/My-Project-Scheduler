//-----------------------------------------------------------------------------
// 【ScheduleResult.hpp】
// 【调度结果类声明，保存关键路径计算的完整结果】
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------

#ifndef SCHEDULERESULT_HPP
#define SCHEDULERESULT_HPP

#include <unordered_map>
#include <vector>

#include "Id.hpp"
#include "TaskScheduleInfo.hpp"

//-----------------------------------------------------------------------------
// 【ScheduleResult 类】
// 【功能】保存一次关键路径计算的完整结果，纯数据载体，不包含业务判断逻辑
// 【接口说明】构造函数接受数据映射和总工期；提供各时间值的只读访问
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
class ScheduleResult
{
  public:
    ScheduleResult(std::unordered_map<TaskId, TaskScheduleInfo> data,
                   int totalDuration, std::vector<TaskId> criticalPath);
    ScheduleResult()                                 = default;
    ~ScheduleResult()                                = default;
    ScheduleResult(const ScheduleResult&)            = default;
    ScheduleResult& operator=(const ScheduleResult&) = default;
    ScheduleResult(ScheduleResult&&)                 = default;
    ScheduleResult& operator=(ScheduleResult&&)      = default;

    // 获取项目总工期
    int GetTotalDuration() const;

    // 获取最早开始时间
    int GetEarlyStart(TaskId id) const;
    // 获取最早完成时间
    int GetEarlyFinish(TaskId id) const;
    // 获取最晚开始时间
    int GetLateStart(TaskId id) const;
    // 获取最晚完成时间
    int GetLateFinish(TaskId id) const;

    // 获取关键路径上的任务 ID 列表（按拓扑顺序）
    const std::vector<TaskId>& GetCriticalPath() const;

  private:
    std::unordered_map<TaskId, TaskScheduleInfo> m_dataMap; // 任务时间信息映射
    int                                          m_iTotalDuration; // 项目总工期
    std::vector<TaskId> m_criticalPathVec; // 关键路径任务 ID 列表
};

#endif
