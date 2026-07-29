//-----------------------------------------------------------------------------
// 【Allocation.cpp】
// 【资源分配记录类实现】
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------

#include "Allocation.hpp"

Allocation::Allocation(TaskId taskId, ResourceId resourceId, int quantity)
    : m_taskId(taskId), m_resourceId(resourceId), m_iQuantity(quantity)
{
}

TaskId Allocation::GetTaskId() const
{
    return m_taskId;
}

ResourceId Allocation::GetResourceId() const
{
    return m_resourceId;
}

int Allocation::GetQuantity() const
{
    return m_iQuantity;
}

void Allocation::SetQuantity(int quantity)
{
    m_iQuantity = quantity;
}
