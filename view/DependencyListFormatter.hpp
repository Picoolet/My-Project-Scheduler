//-----------------------------------------------------------------------------
// 【DependencyListFormatter.hpp】
// 【依赖列表格式化器类声明】
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------

#ifndef DEPENDENCYLISTFORMATTER_HPP
#define DEPENDENCYLISTFORMATTER_HPP

#include <string>
#include <vector>

#include "DependencyDTO.hpp"

//-----------------------------------------------------------------------------
// 【DependencyListFormatter 类】
// 【功能】将 DependencyDTO 列表格式化为五列文本表格
// 【接口说明】纯静态方法类，禁止实例化
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
class DependencyListFormatter
{
  public:
    DependencyListFormatter() = delete;

    // 将依赖列表格式化为表格文本
    static std::string Format(const std::vector<DependencyDTO>& dependencies);

  private:
    // 将 DependencyType 转为字符串 "FS"/"SS"/"FF"/"SF"
    static std::string TypeToString(DependencyType type);
};

#endif
