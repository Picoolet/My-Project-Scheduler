//-----------------------------------------------------------------------------
// 【TaskRelationsFormatter_test.cpp】
// 【任务关系格式化器单元测试】
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------

#include <cassert>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "TaskDTO.hpp"
#include "TaskRelationsFormatter.hpp"

//-----------------------------------------------------------------------------
// 【main】
// 【函数功能】测试 TaskRelationsFormatter::Format 的分区输出
// 【参数】无
// 【返回值】0 — 全部通过
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
int main()
{
    // 空关系
    std::pair<std::vector<TaskDTO>, std::vector<TaskDTO>> emptyRel;
    std::string emptyText = TaskRelationsFormatter::Format(emptyRel);
    assert(emptyText.find("前驱任务") != std::string::npos);
    assert(emptyText.find("后继任务") != std::string::npos);
    assert(emptyText.find("(无)") != std::string::npos);

    // 非空关系
    TaskDTO pred;
    pred.index = 0;
    pred.name  = "Design";

    TaskDTO succ1;
    succ1.index = 2;
    succ1.name  = "Coding";

    TaskDTO succ2;
    succ2.index = 5;
    succ2.name  = "Acceptance";

    std::pair<std::vector<TaskDTO>, std::vector<TaskDTO>> relations;
    relations.first.push_back(pred);
    relations.second.push_back(succ1);
    relations.second.push_back(succ2);

    std::string text = TaskRelationsFormatter::Format(relations);
    assert(text.find("前驱任务") != std::string::npos);
    assert(text.find("1 - Design") != std::string::npos);
    assert(text.find("后继任务") != std::string::npos);
    assert(text.find("3 - Coding") != std::string::npos);
    assert(text.find("6 - Acceptance") != std::string::npos);

    std::cout << "TaskRelationsFormatter test PASSED\n";
    return 0;
}
