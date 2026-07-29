//-----------------------------------------------------------------------------
// 【NormalPolicy.hpp】
// 【普通资源分配策略类声明】
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------

#ifndef NORMALPOLICY_HPP
#define NORMALPOLICY_HPP

#include "IResourceAllocationPolicy.hpp"

//-----------------------------------------------------------------------------
// 【NormalPolicy 类】
// 【功能】普通资源分配策略，始终允许资源分配
// 【接口说明】canAllocate() 始终返回 true
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
class NormalPolicy : public IResourceAllocationPolicy
{
  public:
    NormalPolicy()                               = default;
    NormalPolicy(const NormalPolicy&)            = default;
    NormalPolicy& operator=(const NormalPolicy&) = default;
    ~NormalPolicy() override                     = default;

    bool canAllocate() const override;
};

#endif
