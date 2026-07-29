//-----------------------------------------------------------------------------
// 【ITaskBehavior.cpp】
// 【任务行为抽象接口之静态工厂实现】
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------

#include "ITaskBehavior.hpp"
#include "MilestoneBehavior.hpp"
#include "NormalBehavior.hpp"

//-----------------------------------------------------------------------------
// 【ITaskBehavior::Create】
// 【函数功能】根据工期创建对应的任务行为策略
// 【参数】duration — 任务工期，调用方保证 >= 0
// 【返回值】duration == 0 返回 MilestoneBehavior
//          duration > 0 返回 NormalBehavior
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
std::unique_ptr<ITaskBehavior> ITaskBehavior::Create(int duration)
{
    if (duration == 0)
    {
        return std::make_unique<MilestoneBehavior>();
    }

    return std::make_unique<NormalBehavior>();
}
