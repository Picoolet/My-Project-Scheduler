//-----------------------------------------------------------------------------
// 【ScheduleResultFormatter_test.cpp】
// 【调度结果格式化器单元测试】
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------

#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "Id.hpp"
#include "ScheduleResult.hpp"
#include "ScheduleResultFormatter.hpp"
#include "TaskDTO.hpp"

//-----------------------------------------------------------------------------
// 【main】
// 【函数功能】测试 ScheduleResultFormatter::Format 的时间表与关键路径
// 【参数】无
// 【返回值】0 — 全部通过
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
int main()
{
    // 空结果 → 无法计算提示
    ScheduleResult emptyResult({}, 0, {});
    std::string    emptyText = ScheduleResultFormatter::Format(emptyResult, {});
    assert(emptyText.find("无法计算调度") != std::string::npos);

    // 非空结果
    std::unordered_map<TaskId, TaskScheduleInfo> data;

    TaskScheduleInfo info1;
    info1.earlyStart  = 0;
    info1.earlyFinish = 5;
    info1.lateStart   = 0;
    info1.lateFinish  = 5;
    data[TaskId(1)]   = info1;

    TaskScheduleInfo info2;
    info2.earlyStart  = 17;
    info2.earlyFinish = 17;
    info2.lateStart   = 22;
    info2.lateFinish  = 22;
    data[TaskId(6)]   = info2;

    std::vector<TaskId> path;
    path.push_back(TaskId(1));

    ScheduleResult result(std::move(data), 22, std::move(path));

    // 任务 DTO 列表
    std::vector<TaskDTO> tasks;

    TaskDTO t1;
    t1.index   = 0;
    t1.idValue = 1;
    t1.name    = "Requirement";
    tasks.push_back(t1);

    TaskDTO t6;
    t6.index   = 5;
    t6.idValue = 6;
    t6.name    = "Acceptance";
    tasks.push_back(t6);

    std::string text = ScheduleResultFormatter::Format(result, tasks);
    assert(text.find("总工期: 22") != std::string::npos);
    assert(text.find("Requirement") != std::string::npos);
    assert(text.find("Acceptance") != std::string::npos);
    assert(text.find("关键路径: 1") != std::string::npos);
    assert(text.find("→") != std::string::npos
           || text.find("关键路径") != std::string::npos);

    std::cout << "ScheduleResultFormatter test PASSED\n";
    return 0;
}
