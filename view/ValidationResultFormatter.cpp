//-----------------------------------------------------------------------------
// 【ValidationResultFormatter.cpp】
// 【验证结果格式化器类实现】
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------

#include "ValidationResultFormatter.hpp"

//-----------------------------------------------------------------------------
// 【ValidationResultFormatter::Format】
// 【函数功能】将验证结果格式化为文本
// 【参数】result — 输入参数，验证结果
// 【返回值】通过 → "  [OK] 项目验证通过，无错误。\n"；否则逐条列出错误
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
std::string ValidationResultFormatter::Format(const ValidationResult& result)
{
    if (result.IsValid() == true)
    {
        return "  [OK] 项目验证通过，无错误。\n";
    }

    std::string text;
    int         count = 1;

    for (const std::string& error : result.GetErrors())
    {
        text += "  #" + std::to_string(count) + ": " + error + "\n";
        ++count;
    }

    return text;
}
