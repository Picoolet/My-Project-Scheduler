//-----------------------------------------------------------------------------
// 【Dependency.hpp】
// 【任务依赖关系类声明】
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------

#ifndef DEPENDENCY_HPP
#define DEPENDENCY_HPP

#include "DependencyType.hpp"
#include "Id.hpp"

//-----------------------------------------------------------------------------
// 【Dependency 类】
// 【功能】描述两个任务之间的时序约束，存储前后置任务 ID、依赖类型和时差
// 【接口说明】纯数据载体，提供 const 访问器
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
class Dependency
{
  public:
    Dependency(TaskId predecessorId, TaskId successorId, DependencyType type,
               int lag);
    Dependency()                             = default;
    ~Dependency()                            = default;
    Dependency(const Dependency&)            = default;
    Dependency& operator=(const Dependency&) = default;
    Dependency(Dependency&&)                 = default;
    Dependency& operator=(Dependency&&)      = default;

    // 获取前序任务 ID
    TaskId GetPredecessorId() const;
    // 获取后继任务 ID
    TaskId GetSuccessorId() const;
    // 获取依赖类型
    DependencyType GetType() const;
    // 获取时差
    int GetLag() const;

  private:
    TaskId         m_predecessorId; // 前序任务 ID
    TaskId         m_successorId;   // 后继任务 ID
    DependencyType m_type;          // 依赖类型
    int            m_iLag;          // 时差（正数为滞后，负数为提前）
};

#endif
