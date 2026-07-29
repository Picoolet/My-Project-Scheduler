//-----------------------------------------------------------------------------
// 【Resource.hpp】
// 【资源类声明】
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------

#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include <string>

#include "Id.hpp"

//-----------------------------------------------------------------------------
// 【Resource 类】
// 【功能】表示项目中独立于任务存在的资源实体，可被任务占用
// 【接口说明】纯数据载体，提供 const 访问器
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
class Resource
{
  public:
    Resource(ResourceId id, const std::string& name, double unitCost);
    Resource()                           = default;
    ~Resource()                          = default;
    Resource(const Resource&)            = default;
    Resource& operator=(const Resource&) = default;
    Resource(Resource&&)                 = default;
    Resource& operator=(Resource&&)      = default;

    // 获取资源 ID
    ResourceId GetId() const;
    // 获取资源名称
    const std::string& GetName() const;
    // 获取单位时间成本
    double GetUnitCost() const;

  private:
    ResourceId  m_resourceId;   // 资源唯一标识
    std::string m_resourceName; // 资源名称
    double      m_rUnitCost;    // 单位时间成本
};

#endif
