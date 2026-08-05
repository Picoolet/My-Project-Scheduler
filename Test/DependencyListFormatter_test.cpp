//-----------------------------------------------------------------------------
// 【DependencyListFormatter_test.cpp】
// 【依赖列表格式化器单元测试】
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "DependencyDTO.hpp"
#include "DependencyListFormatter.hpp"
#include "DependencyType.hpp"

//-----------------------------------------------------------------------------
// 【main】
// 【函数功能】测试 DependencyListFormatter::Format 的类型字符串输出
// 【参数】无
// 【返回值】0 — 全部通过
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
int main()
{
    // 空列表
    std::string empty = DependencyListFormatter::Format({});
    assert(empty.find("暂无依赖") != std::string::npos);

    std::vector<DependencyDTO> deps;

    DependencyDTO d1;
    d1.index            = 0;
    d1.predecessorIndex = 0;
    d1.successorIndex   = 1;
    d1.type             = DependencyType::FS;
    d1.lag              = 0;

    DependencyDTO d2;
    d2.index            = 1;
    d2.predecessorIndex = 2;
    d2.successorIndex   = 3;
    d2.type             = DependencyType::SS;
    d2.lag              = -2;

    deps.push_back(d1);
    deps.push_back(d2);

    std::string text = DependencyListFormatter::Format(deps);
    assert(text.find("FS") != std::string::npos);
    assert(text.find("SS") != std::string::npos);
    assert(text.find("-2") != std::string::npos);

    std::cout << "DependencyListFormatter test PASSED\n";
    return 0;
}
