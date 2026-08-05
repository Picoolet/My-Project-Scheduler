//-----------------------------------------------------------------------------
// 【ResourceListFormatter.hpp】
// 【资源列表格式化器类声明】
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------

#ifndef RESOURCELISTFORMATTER_HPP
#define RESOURCELISTFORMATTER_HPP

#include <string>
#include <vector>

#include "ResourceDTO.hpp"

//-----------------------------------------------------------------------------
// 【ResourceListFormatter 类】
// 【功能】将 ResourceDTO 列表格式化为三列文本表格
// 【接口说明】纯静态方法类，禁止实例化
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
class ResourceListFormatter
{
  public:
    ResourceListFormatter() = delete;

    // 将资源列表格式化为表格文本（成本保留 2 位小数）
    static std::string Format(const std::vector<ResourceDTO>& resources);
};

#endif
