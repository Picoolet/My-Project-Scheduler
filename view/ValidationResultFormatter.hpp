//-----------------------------------------------------------------------------
// 【ValidationResultFormatter.hpp】
// 【验证结果格式化器类声明】
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------

#ifndef VALIDATIONRESULTFORMATTER_HPP
#define VALIDATIONRESULTFORMATTER_HPP

#include <string>

#include "ValidationResult.hpp"

//-----------------------------------------------------------------------------
// 【ValidationResultFormatter 类】
// 【功能】将验证结果格式化为文本
// 【接口说明】纯静态方法类，禁止实例化
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
class ValidationResultFormatter
{
  public:
    ValidationResultFormatter() = delete;

    // 验证通过 → [OK]；未通过 → 逐条列出错误
    static std::string Format(const ValidationResult& result);
};

#endif
