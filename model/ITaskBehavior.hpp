//-----------------------------------------------------------------------------
// 【ITaskBehavior.hpp】
// 【任务行为抽象接口声明】
// 【开发者及日期】QJQ 2026.7.29
// 【更改记录】 无
//-----------------------------------------------------------------------------

#ifndef ITASKBEHAVIOR_HPP
#define ITASKBEHAVIOR_HPP

#include <memory>

//-----------------------------------------------------------------------------
// 【ITaskBehavior 类】
// 【功能】任务行为的抽象接口，定义 CanAllocate 纯虚函数供子类实现
// 【接口说明】CanAllocate() 返回任务是否可分配资源
// 【开发者及日期】QJQ 2026.7.29
// 【更改记录】 无
//-----------------------------------------------------------------------------
class ITaskBehavior
{
  public:
    ITaskBehavior()                                = default;
    ITaskBehavior(const ITaskBehavior&)            = default;
    ITaskBehavior& operator=(const ITaskBehavior&) = default;
    virtual ~ITaskBehavior()                       = default;

    // 判断当前是否可以分配资源
    virtual bool CanAllocate() const = 0;

    // 静态工厂：根据工期创建对应行为策略
    // 前置条件：duration >= 0（由调用方保证）
    static std::unique_ptr<ITaskBehavior> Create(int duration);
};

#endif
