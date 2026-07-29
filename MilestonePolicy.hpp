//-----------------------------------------------------------------------------
// 【MilestonePolicy.hpp】
// 【里程碑资源分配策略类声明】
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------

#ifndef MILESTONEPOLICY_HPP
#define MILESTONEPOLICY_HPP

#include "IResourceAllocationPolicy.hpp"

//-----------------------------------------------------------------------------
// 【MilestonePolicy 类】
// 【功能】里程碑资源分配策略，始终禁止资源分配
// 【接口说明】canAllocate() 始终返回 false
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
class MilestonePolicy : public IResourceAllocationPolicy
{
  public:
    MilestonePolicy()                                  = default;
    MilestonePolicy(const MilestonePolicy&)            = default;
    MilestonePolicy& operator=(const MilestonePolicy&) = default;
    ~MilestonePolicy() override                        = default;

    bool canAllocate() const override;
};

#endif
