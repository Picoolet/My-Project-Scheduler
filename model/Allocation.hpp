//-----------------------------------------------------------------------------
// 【Allocation.hpp】
// 【资源分配记录类声明】
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------

#ifndef ALLOCATION_HPP
#define ALLOCATION_HPP

#include "Id.hpp"

//-----------------------------------------------------------------------------
// 【Allocation 类】
// 【功能】记录某个任务对某种资源的占用数量
// 【接口说明】纯数据载体，提供 const 访问器
//           唯一性约束（同一 task + resource 只能有一条记录）
//           由 Project::AssignResource 的 upsert 语义保证
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
class Allocation
{
  public:
    Allocation(TaskId taskId, ResourceId resourceId, int quantity);
    Allocation()                             = default;
    ~Allocation()                            = default;
    Allocation(const Allocation&)            = default;
    Allocation& operator=(const Allocation&) = default;
    Allocation(Allocation&&)                 = default;
    Allocation& operator=(Allocation&&)      = default;

    // 获取关联的任务 ID
    TaskId GetTaskId() const;
    // 获取关联的资源 ID
    ResourceId GetResourceId() const;
    // 获取占用数量
    int GetQuantity() const;

    // 修改占用数量（供 Project::AssignResource 使用）
    void SetQuantity(int quantity);

  private:
    TaskId     m_taskId;     // 任务 ID
    ResourceId m_resourceId; // 资源 ID
    int        m_iQuantity;  // 占用数量（> 0）
};

#endif
