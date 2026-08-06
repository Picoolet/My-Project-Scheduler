//-----------------------------------------------------------------------------
// 【TaskRelationsFormatter_test.cpp】
// 【任务详情格式化器单元测试】
// 【开发者及日期】 QJQ 2026.8.6
//-----------------------------------------------------------------------------

#include <cassert>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "ResourceDTO.hpp"
#include "TaskDTO.hpp"
#include "TaskRelationsFormatter.hpp"

//-----------------------------------------------------------------------------
// 【main】
// 【函数功能】测试 TaskRelationsFormatter::Format 的分区输出
// 【参数】无
// 【返回值】0 — 全部通过
// 【开发者及日期】 QJQ 2026.8.6
//-----------------------------------------------------------------------------
int main()
{
    // 任务自身信息
    TaskDTO task;
    task.index   = 2;
    task.idValue = 3;
    task.name    = "Coding";

    // 前驱/后继
    TaskDTO pred;
    pred.index = 1;
    pred.name  = "Design";

    TaskDTO succ1;
    succ1.index = 3;
    succ1.name  = "Testing";

    TaskDTO succ2;
    succ2.index = 5;
    succ2.name  = "Acceptance";

    std::pair<std::vector<TaskDTO>, std::vector<TaskDTO>> relations;
    relations.first.push_back(pred);
    relations.second.push_back(succ1);
    relations.second.push_back(succ2);

    // 资源列表
    std::vector<ResourceDTO> resources;

    ResourceDTO res1;
    res1.index    = 1;
    res1.idValue  = 102;
    res1.name     = "SeniorDev";
    res1.quantity = 2;
    resources.push_back(res1);

    ResourceDTO res2;
    res2.index    = 2;
    res2.idValue  = 103;
    res2.name     = "JuniorDev";
    res2.quantity = 1;
    resources.push_back(res2);

    std::string text = TaskRelationsFormatter::Format(task, relations,
                                                      resources);

    // 任务自身信息（索引/ID/名称）
    assert(text.find("索引: 3") != std::string::npos);
    assert(text.find("ID: 3") != std::string::npos);
    assert(text.find("名称: Coding") != std::string::npos);

    // 前驱/后继
    assert(text.find("前驱任务") != std::string::npos);
    assert(text.find("2 - Design") != std::string::npos);
    assert(text.find("后继任务") != std::string::npos);
    assert(text.find("4 - Testing") != std::string::npos);
    assert(text.find("6 - Acceptance") != std::string::npos);

    // 资源列表
    assert(text.find("资源列表") != std::string::npos);
    assert(text.find("2 - SeniorDev (×2)") != std::string::npos);
    assert(text.find("3 - JuniorDev (×1)") != std::string::npos);

    // 空关系/资源 → (无)
    std::pair<std::vector<TaskDTO>, std::vector<TaskDTO>> emptyRel;
    std::string emptyText = TaskRelationsFormatter::Format(task, emptyRel, {});
    assert(emptyText.find("(无)") != std::string::npos);

    std::cout << "TaskRelationsFormatter test PASSED\n";
    return 0;
}
