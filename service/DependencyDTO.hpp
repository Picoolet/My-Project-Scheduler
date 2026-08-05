//-----------------------------------------------------------------------------
// 【DependencyDTO.hpp】
// 【依赖数据传输对象，纯数据载体，不含业务逻辑】
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------

#ifndef DEPENDENCYDTO_HPP
#define DEPENDENCYDTO_HPP

#include "DependencyType.hpp"

//-----------------------------------------------------------------------------
// 【DependencyDTO 结构体】
// 【功能】承载单条依赖的展示信息（需求 3.2.1），供界面层使用
// 【字段说明】index 为瞬时展示编号，predecessorIndex/successorIndex 为任务索引
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
struct DependencyDTO
{
    int            index;            // 序号（0-based，瞬时展示编号）
    int            predecessorIndex; // 前置任务索引
    int            successorIndex;   // 后置任务索引
    DependencyType type;             // FS / SS / FF / SF
    int            lag;              // 时差（正数为滞后，负数为提前）
};

#endif
