//-----------------------------------------------------------------------------
// 【ScheduleResultFormatter.cpp】
// 【调度结果格式化器类实现】
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------

#include "ScheduleResultFormatter.hpp"

#include <algorithm>

#include "Id.hpp"
#include "TextUtil.hpp"

//-----------------------------------------------------------------------------
// 【ScheduleResultFormatter::Format】
// 【函数功能】将调度结果格式化为时间表格（序号/ID/名称/ES/EF/LS/LF/关键）
// 【参数】result — 输入参数，调度结果
//        tasks — 输入参数，任务 DTO 列表（用于名称与 ID 展示）
// 【返回值】格式化文本
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
std::string ScheduleResultFormatter::Format(const ScheduleResult&       result,
                                            const std::vector<TaskDTO>& tasks)
{
    const auto& criticalPath = result.GetCriticalPath();

    // 空结果判定：totalDuration==0 且关键路径为空
    if ((result.GetTotalDuration() == 0) && (criticalPath.empty() == true))
    {
        return "  无法计算调度（项目可能为空或存在环路）\n";
    }

    std::string text;
    text += "  " + std::string(56, '-') + "\n";
    text += "  调度结果 — 总工期: " + std::to_string(result.GetTotalDuration())
            + " 天\n";
    text += "  " + std::string(56, '-') + "\n";
    text += "  序号  ID  名称          ES   EF   LS   LF   关键\n";
    text += "  " + std::string(56, '-') + "\n";

    for (const TaskDTO& task : tasks)
    {
        TaskId taskId(task.idValue);

        bool isCritical = (std::find(criticalPath.begin(), criticalPath.end(),
                                     taskId)
                           != criticalPath.end());

        text += "  ";
        text += textutil::PadRight(std::to_string(task.index + 1), 6);
        text += textutil::PadRight(std::to_string(task.idValue), 5);
        text += textutil::PadRight(textutil::Truncate(task.name, 30), 14);
        text += textutil::PadRight(std::to_string(result.GetEarlyStart(taskId)),
                                   5);
        text += textutil::PadRight(
            std::to_string(result.GetEarlyFinish(taskId)), 5);
        text += textutil::PadRight(std::to_string(result.GetLateStart(taskId)),
                                   5);
        text += textutil::PadRight(std::to_string(result.GetLateFinish(taskId)),
                                   5);

        if (isCritical == true)
        {
            text += "*";
        }

        text += "\n";
    }

    text += "  " + std::string(56, '-') + "\n";
    text += "  关键路径: ";

    for (size_t i = 0; i < criticalPath.size(); ++i)
    {
        if (i > 0)
        {
            text += " → ";
        }

        text += std::to_string(criticalPath[i].Value());
    }

    text += "\n";
    text += "  " + std::string(56, '-') + "\n";
    return text;
}
