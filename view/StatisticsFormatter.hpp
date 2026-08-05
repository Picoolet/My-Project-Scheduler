//-----------------------------------------------------------------------------
// 【StatisticsFormatter.hpp】
// 【统计信息格式化器类声明】
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------

#ifndef STATISTICSFORMATTER_HPP
#define STATISTICSFORMATTER_HPP

#include <string>

#include "ProjectStatisticsDTO.hpp"

//-----------------------------------------------------------------------------
// 【StatisticsFormatter 类】
// 【功能】将项目统计 DTO 格式化为文本块
// 【接口说明】纯静态方法类，禁止实例化
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
class StatisticsFormatter
{
  public:
    StatisticsFormatter() = delete;

    // 将统计信息格式化为文本；isValid==false 时状态显示未通过验证
    static std::string Format(const ProjectStatisticsDTO& stats);
};

#endif
