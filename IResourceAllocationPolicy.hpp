//-----------------------------------------------------------------------------
// 【IResourceAllocationPolicy.hpp】
// 【资源分配策略抽象接口声明】
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------

#ifndef IRESOURCEALLOCATIONPOLICY_HPP
#define IRESOURCEALLOCATIONPOLICY_HPP

//-----------------------------------------------------------------------------
// 【IResourceAllocationPolicy 类】
// 【功能】资源分配策略的抽象接口，定义 canAllocate 纯虚函数供子类实现
// 【接口说明】canAllocate() 返回资源是否可分配
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
class IResourceAllocationPolicy
{
  public:
    IResourceAllocationPolicy()                                 = default;
    IResourceAllocationPolicy(const IResourceAllocationPolicy&) = default;
    IResourceAllocationPolicy& operator=(const IResourceAllocationPolicy&)
        = default;
    virtual ~IResourceAllocationPolicy() = default;

    // 判断当前是否可以分配资源
    virtual bool canAllocate() const = 0;
};

#endif
