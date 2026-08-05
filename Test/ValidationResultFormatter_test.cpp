//-----------------------------------------------------------------------------
// 【ValidationResultFormatter_test.cpp】
// 【验证结果格式化器单元测试】
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "ValidationResult.hpp"
#include "ValidationResultFormatter.hpp"

//-----------------------------------------------------------------------------
// 【main】
// 【函数功能】测试 ValidationResultFormatter::Format 的有效/无效分支
// 【参数】无
// 【返回值】0 — 全部通过
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
int main()
{
    // 有效
    ValidationResult valid;
    std::string      validText = ValidationResultFormatter::Format(valid);
    assert(validText.find("验证通过") != std::string::npos);
    assert(validText.find("OK") != std::string::npos);

    // 无效（含错误列表）
    std::vector<std::string> errors;
    errors.push_back("依赖图中存在循环依赖");
    errors.push_back("任务 X 为悬挂节点");

    ValidationResult invalid(errors);
    std::string      invalidText = ValidationResultFormatter::Format(invalid);
    assert(invalidText.find("#1") != std::string::npos);
    assert(invalidText.find("循环依赖") != std::string::npos);
    assert(invalidText.find("#2") != std::string::npos);
    assert(invalidText.find("悬挂节点") != std::string::npos);

    std::cout << "ValidationResultFormatter test PASSED\n";
    return 0;
}
