//-----------------------------------------------------------------------------
// 【ResourceListFormatter.cpp】
// 【资源列表格式化器类实现】
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------

#include "ResourceListFormatter.hpp"

#include <iomanip>
#include <sstream>

#include "TextUtil.hpp"

//-----------------------------------------------------------------------------
// 【ResourceListFormatter::Format】
// 【函数功能】将资源列表格式化为三列表格（序号/名称/单位成本）
// 【参数】resources — 输入参数，ResourceDTO 列表
// 【返回值】格式化文本；空列表返回 "  暂无资源\n"
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
std::string ResourceListFormatter::Format(
    const std::vector<ResourceDTO>& resources)
{
    if (resources.empty() == true)
    {
        return "  暂无资源\n";
    }

    std::string result;
    result += "  " + std::string(48, '-') + "\n";
    result += "  索引  ID   名称            单位成本\n";
    result += "  " + std::string(48, '-') + "\n";

    for (const ResourceDTO& res : resources)
    {
        std::ostringstream costStream;
        costStream << std::fixed << std::setprecision(2) << res.unitCost;

        result += "  ";
        result += TextUtil::PadRight(std::to_string(res.index + 1), 6);
        result += TextUtil::PadRight(std::to_string(res.idValue), 5);
        result += TextUtil::PadRight(TextUtil::Truncate(res.name, 30), 16);
        result += costStream.str();
        result += "\n";
    }

    result += "  " + std::string(48, '-') + "\n";
    return result;
}
