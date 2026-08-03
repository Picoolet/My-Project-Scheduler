//-----------------------------------------------------------------------------
// 【ProjectStatisticsDTO.hpp】
// 【项目统计信息数据传输对象，纯数据载体，不含业务逻辑】
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------

#ifndef PROJECTSTATISTICSDTO_HPP
#define PROJECTSTATISTICSDTO_HPP

//-----------------------------------------------------------------------------
// 【ProjectStatisticsDTO 结构体】
// 【功能】承载项目宏观统计指标（需求 4），供界面层使用
// 【字段说明】isValid 为 false 时 totalDuration 取 -1
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
struct ProjectStatisticsDTO
{
    int  taskCount;       // Task 总数（含普通与里程碑）
    int  dependencyCount; // Dependency 总数
    int  resourceCount;   // Resource 总数
    bool isValid;         // 项目是否通过合理性验证
    int  totalDuration;   // 关键路径总工期；isValid==false 时取 -1
};

#endif
