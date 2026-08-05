//-----------------------------------------------------------------------------
// 【MilestoneBehavior.hpp】
// 【里程碑任务行为策略类声明】
// 【开发者及日期】QJQ 2026.7.29
// 【更改记录】 无
//-----------------------------------------------------------------------------

#ifndef MILESTONEBEHAVIOR_HPP
#define MILESTONEBEHAVIOR_HPP

#include "ITaskBehavior.hpp"

//-----------------------------------------------------------------------------
// 【MilestoneBehavior 类】
// 【功能】里程碑任务行为策略，始终禁止资源分配
// 【接口说明】CanAllocate() 始终返回 false
// 【开发者及日期】QJQ 2026.7.29
// 【更改记录】 无
//-----------------------------------------------------------------------------
class MilestoneBehavior : public ITaskBehavior
{
  public:
    MilestoneBehavior()                                    = default;
    MilestoneBehavior(const MilestoneBehavior&)            = default;
    MilestoneBehavior& operator=(const MilestoneBehavior&) = default;
    ~MilestoneBehavior() override                          = default;

    bool CanAllocate() const override;
};

#endif
