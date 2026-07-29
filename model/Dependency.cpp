//-----------------------------------------------------------------------------
// 【Dependency.cpp】
// 【任务依赖关系类实现】
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------

#include "Dependency.hpp"

//-----------------------------------------------------------------------------
// 【Dependency::Dependency】
// 【函数功能】构造依赖关系对象
// 【参数】predecessorId — 前序任务 ID
//        successorId — 后继任务 ID
//        type — 依赖类型
//        lag — 时差
// 【返回值】无
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
Dependency::Dependency(TaskId predecessorId, TaskId successorId,
                       DependencyType type, int lag)
    : m_predecessorId(predecessorId), m_successorId(successorId), m_type(type),
      m_iLag(lag)
{
}

TaskId Dependency::GetPredecessorId() const
{
    return m_predecessorId;
}

TaskId Dependency::GetSuccessorId() const
{
    return m_successorId;
}

DependencyType Dependency::GetType() const
{
    return m_type;
}

int Dependency::GetLag() const
{
    return m_iLag;
}
