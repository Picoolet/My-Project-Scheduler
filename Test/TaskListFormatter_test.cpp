//-----------------------------------------------------------------------------
// 【TaskListFormatter_test.cpp】
// 【任务列表格式化器单元测试】
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "TaskDTO.hpp"
#include "TaskListFormatter.hpp"

//-----------------------------------------------------------------------------
// 【main】
// 【函数功能】测试 TaskListFormatter::Format 的关键文本
// 【参数】无
// 【返回值】0 — 全部通过
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
int main()
{
    // 空列表
    std::string empty = TaskListFormatter::Format({});
    assert(empty.find("暂无任务") != std::string::npos);

    // 构造两个任务
    std::vector<TaskDTO> tasks;

    TaskDTO t1;
    t1.index              = 0;
    t1.idValue            = 1;
    t1.name               = "Requirement";
    t1.duration           = 5;
    t1.predecessorIndices = {};
    t1.successorIndices   = {1};

    TaskDTO t2;
    t2.index              = 1;
    t2.idValue            = 2;
    t2.name               = "Acceptance";
    t2.duration           = 0; // 里程碑
    t2.predecessorIndices = {0};
    t2.successorIndices   = {};

    tasks.push_back(t1);
    tasks.push_back(t2);

    std::string text = TaskListFormatter::Format(tasks);

    // 名称、索引、工期、里程碑标记、ID 列
    assert(text.find("Requirement") != std::string::npos);
    assert(text.find("Acceptance") != std::string::npos);
    assert(text.find("索引") != std::string::npos);
    assert(text.find("ID") != std::string::npos);
    assert(text.find("1") != std::string::npos);
    assert(text.find("里程碑") != std::string::npos);
    assert(text.find("2") != std::string::npos); // 前驱索引 [2] 显示 1-based

    std::cout << "TaskListFormatter test PASSED\n";
    return 0;
}
