//-----------------------------------------------------------------------------
// 【StatisticsFormatter_test.cpp】
// 【统计信息格式化器单元测试】
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------

#include <cassert>
#include <iostream>
#include <string>

#include "ProjectStatisticsDTO.hpp"
#include "StatisticsFormatter.hpp"

//-----------------------------------------------------------------------------
// 【main】
// 【函数功能】测试 StatisticsFormatter::Format 的有效/无效状态分支
// 【参数】无
// 【返回值】0 — 全部通过
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
int main()
{
    // 有效统计
    ProjectStatisticsDTO stats;
    stats.taskCount       = 6;
    stats.dependencyCount = 5;
    stats.resourceCount   = 5;
    stats.isValid         = true;
    stats.totalDuration   = 22;

    std::string text = StatisticsFormatter::Format(stats);
    assert(text.find("6") != std::string::npos);
    assert(text.find("5") != std::string::npos);
    assert(text.find("通过验证") != std::string::npos);
    assert(text.find("22 天") != std::string::npos);

    // 无效统计
    ProjectStatisticsDTO invalidStats;
    invalidStats.taskCount       = 0;
    invalidStats.dependencyCount = 0;
    invalidStats.resourceCount   = 0;
    invalidStats.isValid         = false;
    invalidStats.totalDuration   = -1;

    std::string invalidText = StatisticsFormatter::Format(invalidStats);
    assert(invalidText.find("未通过验证") != std::string::npos);
    assert(invalidText.find("N/A") != std::string::npos);

    std::cout << "StatisticsFormatter test PASSED\n";
    return 0;
}
