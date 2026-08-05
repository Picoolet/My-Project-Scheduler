//-----------------------------------------------------------------------------
// 【TaskRelationsFormatter.cpp】
// 【任务关系格式化器类实现】
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------

#include "TaskRelationsFormatter.hpp"

//-----------------------------------------------------------------------------
// 【TaskRelationsFormatter::Format】
// 【函数功能】将任务前驱/后继关系格式化为两区文本
// 【参数】relations — 输入参数，pair<前驱列表, 后继列表>
// 【返回值】格式化文本
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
std::string TaskRelationsFormatter::Format(
    const std::pair<std::vector<TaskDTO>, std::vector<TaskDTO>>& relations)
{
    std::string result;
    result += FormatSection("前驱任务 (Predecessors)", relations.first);
    result += FormatSection("后继任务 (Successors)", relations.second);
    return result;
}

//-----------------------------------------------------------------------------
// 【TaskRelationsFormatter::FormatSection】
// 【函数功能】格式化单个分区（标题 + 条目列表）
// 【参数】title — 输入参数，分区标题
//        items — 输入参数，任务 DTO 列表
// 【返回值】格式化文本
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
std::string TaskRelationsFormatter::FormatSection(
    const std::string& title, const std::vector<TaskDTO>& items)
{
    std::string result = "  " + title + ":\n";

    if (items.empty() == true)
    {
        result += "    (无)\n";
        return result;
    }

    for (const TaskDTO& task : items)
    {
        result += "    " + std::to_string(task.index + 1) + " - " + task.name
                  + "\n";
    }

    return result;
}
