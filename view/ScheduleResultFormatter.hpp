//-----------------------------------------------------------------------------
// 【ScheduleResultFormatter.hpp】
// 【调度结果格式化器类声明】
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------

#ifndef SCHEDULERESULTFORMATTER_HPP
#define SCHEDULERESULTFORMATTER_HPP

#include <string>
#include <vector>

#include "ScheduleResult.hpp"
#include "TaskDTO.hpp"

//-----------------------------------------------------------------------------
// 【ScheduleResultFormatter 类】
// 【功能】将调度结果与任务列表格式化为时间表格文本
// 【接口说明】纯静态方法类，禁止实例化
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
class ScheduleResultFormatter
{
  public:
    ScheduleResultFormatter() = delete;

    // 将调度结果格式化为表格；空结果返回无法计算提示
    static std::string Format(const ScheduleResult&       result,
                              const std::vector<TaskDTO>& tasks);
};

#endif
