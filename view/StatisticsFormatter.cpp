//-----------------------------------------------------------------------------
// 【StatisticsFormatter.cpp】
// 【统计信息格式化器类实现】
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------

#include "StatisticsFormatter.hpp"

#include <string>

#include "TextUtil.hpp"

//-----------------------------------------------------------------------------
// 【StatisticsFormatter::Format】
// 【函数功能】将统计信息格式化为文本块
// 【参数】stats — 输入参数，项目统计 DTO
// 【返回值】格式化文本
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
std::string StatisticsFormatter::Format(const ProjectStatisticsDTO& stats)
{
    std::string result;
    result += "  " + std::string(36, '-') + "\n";
    result += "  项目统计\n";
    result += "  " + std::string(36, '-') + "\n";
    result += "  Task 总数:       " + std::to_string(stats.taskCount) + "\n";
    result += "  Dependency 总数: " + std::to_string(stats.dependencyCount)
              + "\n";
    result += "  Resource 总数:   " + std::to_string(stats.resourceCount)
              + "\n";

    std::string status;

    if (stats.isValid == true)
    {
        status = "通过验证";
    }
    else
    {
        status = "未通过验证";
    }

    result += "  项目状态:        " + status + "\n";

    std::string duration;

    if ((stats.isValid == true) && (stats.totalDuration >= 0))
    {
        duration = std::to_string(stats.totalDuration) + " 天";
    }
    else
    {
        duration = "N/A";
    }

    result += "  总工期:          " + duration + "\n";
    result += "  " + std::string(36, '-') + "\n";
    return result;
}
