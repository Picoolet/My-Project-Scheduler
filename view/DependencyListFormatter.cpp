//-----------------------------------------------------------------------------
// 【DependencyListFormatter.cpp】
// 【依赖列表格式化器类实现】
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------

#include "DependencyListFormatter.hpp"

#include "DependencyType.hpp"
#include "TextUtil.hpp"

//-----------------------------------------------------------------------------
// 【DependencyListFormatter::Format】
// 【函数功能】将依赖列表格式化为五列表格（序号/前置/后置/类型/Lag）
// 【参数】dependencies — 输入参数，DependencyDTO 列表
// 【返回值】格式化文本；空列表返回 "  暂无依赖\n"
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
std::string DependencyListFormatter::Format(
    const std::vector<DependencyDTO>& dependencies)
{
    if (dependencies.empty() == true)
    {
        return "  暂无依赖\n";
    }

    std::string result;
    result += "  " + std::string(48, '-') + "\n";
    result += "  序号  前置序号    后置序号    类型    Lag\n";
    result += "  " + std::string(48, '-') + "\n";

    for (const DependencyDTO& dep : dependencies)
    {
        result += "  ";
        result += textutil::PadRight(std::to_string(dep.index + 1), 6);
        result += textutil::PadRight(std::to_string(dep.predecessorIndex + 1),
                                     12);
        result += textutil::PadRight(std::to_string(dep.successorIndex + 1),
                                     12);
        result += textutil::PadRight(TypeToString(dep.type), 8);
        result += std::to_string(dep.lag);
        result += "\n";
    }

    result += "  " + std::string(48, '-') + "\n";
    return result;
}

//-----------------------------------------------------------------------------
// 【DependencyListFormatter::TypeToString】
// 【函数功能】将 DependencyType 转为字符串
// 【参数】type — 输入参数，依赖类型
// 【返回值】"FS" / "SS" / "FF" / "SF"
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
std::string DependencyListFormatter::TypeToString(DependencyType type)
{
    switch (type)
    {
    case DependencyType::SS:
        return "SS";

    case DependencyType::FF:
        return "FF";

    case DependencyType::SF:
        return "SF";

    default:
        return "FS";
    }
}
