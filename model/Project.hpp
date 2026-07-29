//-----------------------------------------------------------------------------
// 【Project.hpp】
// 【项目聚合根类声明，管理所有领域对象及其关系】
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------

#ifndef PROJECT_HPP
#define PROJECT_HPP

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "Allocation.hpp"
#include "Dependency.hpp"
#include "DependencyType.hpp"
#include "Id.hpp"
#include "Resource.hpp"
#include "Task.hpp"

//-----------------------------------------------------------------------------
// 【Project 类】
// 【功能】管理所有 Task、Dependency、Resource、Allocation 及其关系，
//        维护内部邻接索引，提供只读访问和受控的公开修改接口。
// 【接口说明】只读接口返回 const 指针/引用；修改接口负责维护数据完整性
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
class Project
{
  public:
    Project()  = default;
    ~Project() = default;

    // 禁止拷贝
    Project(const Project&)            = delete;
    Project& operator=(const Project&) = delete;

    // 支持移动
    Project(Project&&)            = default;
    Project& operator=(Project&&) = default;

    //-------------------------------------------------------------------------
    // 只读接口
    //-------------------------------------------------------------------------

    // 获取任务总数
    size_t TaskCount() const;
    // 获取依赖总数
    size_t DependencyCount() const;
    // 获取资源总数
    size_t ResourceCount() const;
    // 获取分配记录总数
    size_t AllocationCount() const;

    // 按 ID 查找任务，未找到返回 nullptr
    const Task* FindTask(TaskId id) const;
    // 按 ID 查找资源，未找到返回 nullptr
    const Resource* FindResource(ResourceId id) const;
    // 按前后置任务查找依赖，未找到返回 nullptr
    const Dependency* FindDependency(TaskId pred, TaskId succ) const;

    // 获取某任务的前驱任务 ID 列表（基于内部索引）
    std::vector<TaskId> GetPredecessors(TaskId id) const;
    // 获取某任务的后继任务 ID 列表（基于内部索引）
    std::vector<TaskId> GetSuccessors(TaskId id) const;

    // 获取某任务的所有资源分配记录
    std::vector<const Allocation*> GetAllocationsForTask(TaskId id) const;

    //-------------------------------------------------------------------------
    // 修改接口（受控，负责维护数据完整性）
    //-------------------------------------------------------------------------

    // 添加任务，返回新生成的 TaskId；名称唯一性由调用方保证
    TaskId AddTask(const std::string& name, int duration);

    // 删除任务，级联删除关联的 Dependency 和 Allocation，并更新索引
    void RemoveTask(TaskId id);

    // 添加资源，返回新生成的 ResourceId
    ResourceId AddResource(const std::string& name, double unitCost);

    // 删除资源，级联删除关联的 Allocation
    void RemoveResource(ResourceId id);

    // 添加依赖关系，若 (pred, succ) 已存在则忽略，同时更新邻接索引
    void AddDependency(TaskId pred, TaskId succ, DependencyType type, int lag);

    // 分配资源（upsert），若 quantity <= 0 则删除该分配记录
    void AssignResource(TaskId taskId, ResourceId resourceId, int quantity);

  private:
    // 更新前驱/后继索引：在 succ 的前驱列表中添加 pred
    void AddToIndex(TaskId pred, TaskId succ);
    // 从前驱/后继索引中移除与指定任务相关的所有条目
    void RemoveFromIndex(TaskId id);

    // 内部 ID 自增
    TaskId     GenerateTaskId();
    ResourceId GenerateResourceId();

    std::vector<Task>       m_tasks;        // 任务集合
    std::vector<Dependency> m_dependencies; // 依赖关系集合
    std::vector<Resource>   m_resources;    // 资源集合
    std::vector<Allocation> m_allocations;  // 资源分配记录集合

    std::unordered_map<TaskId, std::vector<TaskId>> m_successors; // 后继邻接表
    std::unordered_map<TaskId, std::vector<TaskId>>
        m_predecessors; // 前驱邻接表

    unsigned int m_uNextTaskId;     // 任务 ID 自增计数器
    unsigned int m_uNextResourceId; // 资源 ID 自增计数器
};

#endif
