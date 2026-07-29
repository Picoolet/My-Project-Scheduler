//-----------------------------------------------------------------------------
// 【NormalBehavior.hpp】
// 【普通任务行为策略类声明】
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------

#ifndef NORMALBEHAVIOR_HPP
#define NORMALBEHAVIOR_HPP

#include "ITaskBehavior.hpp"

//-----------------------------------------------------------------------------
// 【NormalBehavior 类】
// 【功能】普通任务行为策略，始终允许资源分配
// 【接口说明】CanAllocate() 始终返回 true
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
class NormalBehavior : public ITaskBehavior
{
  public:
    NormalBehavior()                                 = default;
    NormalBehavior(const NormalBehavior&)            = default;
    NormalBehavior& operator=(const NormalBehavior&) = default;
    ~NormalBehavior() override                       = default;

    bool CanAllocate() const override;
};

#endif
