//-----------------------------------------------------------------------------
// 【CPMCalculator.hpp】
// 【关键路径计算器类声明，对 const Project& 执行 CPM 调度计算】
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------

#ifndef CPMCALCULATOR_HPP
#define CPMCALCULATOR_HPP

#include "Id.hpp"
#include "ScheduleResult.hpp"

class Project;

//-----------------------------------------------------------------------------
// 【CPMCalculator 类】
// 【功能】对 const Project& 执行 CPM 计算（拓扑排序+前向+后向+关键路径）
// 【接口说明】无状态，Calculate() 自防御（有环/引用断裂→空 ScheduleResult）
//            一个实例可反复用于不同 Project
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
class CPMCalculator
{
  public:
    CPMCalculator()                                = default;
    CPMCalculator(const CPMCalculator&)            = default;
    CPMCalculator& operator=(const CPMCalculator&) = default;
    ~CPMCalculator()                               = default;

    // 执行 CPM 计算，返回 ScheduleResult（自防御，遇环/断裂返回空结果）
    ScheduleResult Calculate(const Project& project) const;

    // 判断某任务是否在关键路径上（EF == LF）
    bool IsCritical(const ScheduleResult& result, TaskId id) const;
};

#endif
