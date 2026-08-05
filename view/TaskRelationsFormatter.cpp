//-----------------------------------------------------------------------------
// 【TaskRelationsFormatter.cpp】
// 【任务详情格式化器类实现】
// 【开发者及日期】 QJQ 2026.8.6
//-----------------------------------------------------------------------------

#include "TaskRelationsFormatter.hpp"

//-----------------------------------------------------------------------------
// 【TaskRelationsFormatter::Format】
// 【函数功能】格式化任务详情：自身信息 + 前驱/后继 + 资源列表
// 【参数】task — 输入参数，任务自身 DTO
//        relations — 输入参数，pair<前驱列表, 后继列表>
//        resources — 输入参数，任务已分配的资源 DTO 列表
// 【返回值】格式化文本
// 【开发者及日期】 QJQ 2026.8.6
//-----------------------------------------------------------------------------
std::string TaskRelationsFormatter::Format(
    const TaskDTO&                                               task,
    const std::pair<std::vector<TaskDTO>, std::vector<TaskDTO>>& relations,
    const std::vector<ResourceDTO>&                              resources)
{
    std::string result;

    // 任务自身信息（依赖列表之前展示）
    result += "  索引: " + std::to_string(task.index + 1) + "    ID: "
              + std::to_string(task.idValue) + "    名称: " + task.name + "\n";

    result += FormatSection("前驱任务 (Predecessors)", relations.first);
    result += FormatSection("后继任务 (Successors)", relations.second);

    // 资源列表（依赖列表之后展示）
    result += "  资源列表 (Resources):\n";

    if (resources.empty() == true)
    {
        result += "    (无)\n";
    }
    else
    {
        for (const ResourceDTO& res : resources)
        {
            result += "    " + std::to_string(res.index + 1) + " - " + res.name
                      + "\n";
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
// 【TaskRelationsFormatter::FormatSection】
// 【函数功能】格式化单个任务分区（标题 + 条目列表）
// 【参数】title — 输入参数，分区标题
//        items — 输入参数，任务 DTO 列表
// 【返回值】格式化文本
// 【开发者及日期】 QJQ 2026.8.6
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
