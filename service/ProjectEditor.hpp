//-----------------------------------------------------------------------------
// 【ProjectEditor.hpp】
// 【项目编辑器类声明，封装对 Project 的全部人工修改操作及其业务规则】
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------

#ifndef PROJECTEDITOR_HPP
#define PROJECTEDITOR_HPP

#include <string>

#include "DependencyType.hpp"
#include "Id.hpp"

class Project;
class ProjectValidator;

//-----------------------------------------------------------------------------
// 【ProjectEditor 类】
// 【功能】对 Project& 执行受控修改，保证每次修改都符合业务规则
// 【接口说明】构造时绑定 Project& 和 Validator&，不持有所有权，栈上使用
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
class ProjectEditor
{
  public:
    // 构造时绑定目标项目与验证器
    ProjectEditor(Project& project, const ProjectValidator& validator);

    ProjectEditor(const ProjectEditor&)            = default;
    ProjectEditor& operator=(const ProjectEditor&) = default;
    ~ProjectEditor()                               = default;

    //------ 任务管理（需求 3.1）------
    bool RemoveTask(int index, std::string& errorMsg);
    bool AddTask(const std::string& name, int duration, std::string& errorMsg);
    bool ModifyTask(int index, const std::string& newName, int newDuration,
                    std::string& errorMsg);

    //------ 依赖管理（需求 3.2）------
    bool RemoveDependency(int index, std::string& errorMsg);
    bool AddDependency(int predIndex, int succIndex, DependencyType type,
                       int lag, std::string& errorMsg);

    //------ 资源管理（需求 3.3）------
    bool AddResource(const std::string& name, double unitCost,
                     std::string& errorMsg);
    bool AssignResource(int taskIndex, int resourceIndex, int quantity,
                        std::string& errorMsg);

    //------ 索引 ↔ ID 映射（public — 供 Controller 等调用方按需使用）------
    TaskId     IndexToTaskId(int index) const;
    int        TaskIdToIndex(TaskId id) const;
    ResourceId IndexToResourceId(int index) const;

  private:
    //------ 名称唯一性 ------
    bool IsTaskNameDuplicate(const std::string& name,
                             TaskId excludeId = TaskId::Invalid()) const;
    bool IsResourceNameDuplicate(const std::string& name) const;

    Project& m_project; // 目标项目（非 const 引用，不持有所有权）
    const ProjectValidator& m_validator; // 用于环检测
};

#endif
