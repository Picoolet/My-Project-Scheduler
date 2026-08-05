//-----------------------------------------------------------------------------
// 【ResourceListFormatter_test.cpp】
// 【资源列表格式化器单元测试】
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "ResourceDTO.hpp"
#include "ResourceListFormatter.hpp"

//-----------------------------------------------------------------------------
// 【main】
// 【函数功能】测试 ResourceListFormatter::Format 的名称与成本输出
// 【参数】无
// 【返回值】0 — 全部通过
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
int main()
{
    // 空列表
    std::string empty = ResourceListFormatter::Format({});
    assert(empty.find("暂无资源") != std::string::npos);

    std::vector<ResourceDTO> resources;

    ResourceDTO r1;
    r1.index    = 0;
    r1.idValue  = 101;
    r1.name     = "Architect";
    r1.unitCost = 100.0;

    ResourceDTO r2;
    r2.index    = 1;
    r2.idValue  = 102;
    r2.name     = "Tester";
    r2.unitCost = 60.0;

    resources.push_back(r1);
    resources.push_back(r2);

    std::string text = ResourceListFormatter::Format(resources);
    assert(text.find("Architect") != std::string::npos);
    assert(text.find("Tester") != std::string::npos);
    assert(text.find("100.00") != std::string::npos);
    assert(text.find("60.00") != std::string::npos);

    std::cout << "ResourceListFormatter test PASSED\n";
    return 0;
}
