//-----------------------------------------------------------------------------
// 【TaskListFormatter.cpp】
// 【任务列表格式化器类实现】
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------

#include "TaskListFormatter.hpp"

#include "TextUtil.hpp"

//-----------------------------------------------------------------------------
// 【TaskListFormatter::Format】
// 【函数功能】将任务列表格式化为五列表格（序号/名称/工期/前驱/后继）
// 【参数】tasks — 输入参数，TaskDTO 列表（DTO 内 index 为 0-based）
// 【返回值】格式化文本；空列表返回 "  暂无任务\n"
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
std::string TaskListFormatter::Format(const std::vector<TaskDTO>& tasks)
{
    if (tasks.empty() == true)
    {
        return "  暂无任务\n";
    }

    std::string result;
    result += "  " + std::string(60, '-') + "\n";
    result += "  索引  ID   名称            工期        前驱任务    后继任务\n";
    result += "  " + std::string(60, '-') + "\n";

    for (const TaskDTO& task : tasks)
    {
        result += "  ";
        result += TextUtil::PadRight(std::to_string(task.index + 1), 6);
        result += TextUtil::PadRight(std::to_string(task.idValue), 5);
        result += TextUtil::PadRight(TextUtil::Truncate(task.name, 30), 16);

        std::string dur = std::to_string(task.duration);

        if (task.duration == 0)
        {
            dur += " (里程碑)";
        }

        result += TextUtil::PadRight(dur, 12);
        result += TextUtil::PadRight(FormatIndices(task.predecessorIndices),
                                     12);
        result += FormatIndices(task.successorIndices);
        result += "\n";
    }

    result += "  " + std::string(60, '-') + "\n";
    return result;
}

//-----------------------------------------------------------------------------
// 【TaskListFormatter::FormatIndices】
// 【函数功能】将 0-based 索引列表格式化为 "[1, 3, 5]"（1-based 显示）
// 【参数】indices — 输入参数，0-based 索引列表
// 【返回值】格式化字符串；空列表返回 ""
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
std::string TaskListFormatter::FormatIndices(const std::vector<int>& indices)
{
    if (indices.empty() == true)
    {
        return "";
    }

    std::string s = "[";

    for (size_t i = 0; i < indices.size(); ++i)
    {
        if (i > 0)
        {
            s += ", ";
        }

        s += std::to_string(indices[i] + 1);
    }

    s += "]";
    return s;
}
