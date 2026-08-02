//-----------------------------------------------------------------------------
// 【ScheduleResult.hpp】
// 【调度结果类声明，保存 CPM 关键路径计算的完整结果】
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------

#ifndef SCHEDULERESULT_HPP
#define SCHEDULERESULT_HPP

#include <unordered_map>
#include <vector>

#include "Id.hpp"

//-----------------------------------------------------------------------------
// 【TaskScheduleInfo 结构体】
// 【功能】保存单个任务的关键路径时间信息，纯数据载体
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
struct TaskScheduleInfo
{
    int earlyStart;  // ES：最早开始时间
    int earlyFinish; // EF：最早完成时间
    int lateStart;   // LS：最晚开始时间
    int lateFinish;  // LF：最晚完成时间
};

//-----------------------------------------------------------------------------
// 【ScheduleResult 类】
// 【功能】保存一次 CPM 计算的完整结果，由 CPMCalculator 填充后按值返回
// 【接口说明】构造后不可变，提供六个 const 访问器
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
class ScheduleResult
{
  public:
    // 构造函数：接受三次计算结果，构造后不可变
    ScheduleResult(std::unordered_map<TaskId, TaskScheduleInfo> data,
                   int totalDuration, std::vector<TaskId> criticalPath);

    // 禁止拷贝
    ScheduleResult(const ScheduleResult&)            = delete;
    ScheduleResult& operator=(const ScheduleResult&) = delete;

    // 支持移动
    ScheduleResult(ScheduleResult&&)            = default;
    ScheduleResult& operator=(ScheduleResult&&) = default;

    ~ScheduleResult() = default;

    // 获取项目总工期
    int GetTotalDuration() const;
    // 获取指定任务的最早开始时间，不存在返回 0
    int GetEarlyStart(TaskId id) const;
    // 获取指定任务的最早完成时间，不存在返回 0
    int GetEarlyFinish(TaskId id) const;
    // 获取指定任务的最晚开始时间，不存在返回 0
    int GetLateStart(TaskId id) const;
    // 获取指定任务的最晚完成时间，不存在返回 0
    int GetLateFinish(TaskId id) const;
    // 获取关键路径任务 ID 列表（按拓扑序）
    const std::vector<TaskId>& GetCriticalPath() const;

  private:
    // 按 ID 取 TaskScheduleInfo 中指定字段值，ID 不存在时返回 0
    int GetFieldValue(TaskId id, int TaskScheduleInfo::* field) const;

    std::unordered_map<TaskId, TaskScheduleInfo> m_data; // 各任务时间信息
    int                                          m_iTotalDuration; // 项目总工期
    std::vector<TaskId>                          m_criticalPath;   // 关键路径
};

#endif
