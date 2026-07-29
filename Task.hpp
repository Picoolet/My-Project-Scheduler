//-----------------------------------------------------------------------------
// 【Task.hpp】
// 【任务类声明，管理任务基本信息与行为策略】
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------

#ifndef TASK_HPP
#define TASK_HPP

#include <memory>
#include <string>

#include "ITaskBehavior.hpp"

//-----------------------------------------------------------------------------
// 【Task 类】
// 【功能】表示项目调度中的任务，包含任务标识、名称、工期及行为策略
// 【接口说明】通过策略模式组合 ITaskBehavior，实现任务行为的多态
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
class Task
{
  public:
    Task()  = default;
    ~Task() = default;

    // 禁止拷贝（unique_ptr 成员不可拷贝）
    Task(const Task&)            = delete;
    Task& operator=(const Task&) = delete;

    // 支持移动
    Task(Task&&)            = default;
    Task& operator=(Task&&) = default;

  private:
    int                            m_iTaskID;       // 任务唯一标识
    std::string                    m_taskName;      // 任务名称
    int                            m_iTaskDuration; // 任务工期
    std::unique_ptr<ITaskBehavior> m_policy;        // 任务行为策略
};

#endif
