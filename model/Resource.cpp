//-----------------------------------------------------------------------------
// 【Resource.cpp】
// 【资源类实现】
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------

#include "Resource.hpp"

Resource::Resource(ResourceId id, const std::string& name, double unitCost)
    : m_resourceId(id), m_resourceName(name), m_rUnitCost(unitCost)
{
}

ResourceId Resource::GetId() const
{
    return m_resourceId;
}

const std::string& Resource::GetName() const
{
    return m_resourceName;
}

double Resource::GetUnitCost() const
{
    return m_rUnitCost;
}
