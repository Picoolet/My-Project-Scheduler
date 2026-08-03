//-----------------------------------------------------------------------------
// 【ProjectEditor.cpp】
// 【项目编辑器类实现】
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------

#include "ProjectEditor.hpp"

#include "Dependency.hpp"
#include "Project.hpp"
#include "ProjectValidator.hpp"
#include "Resource.hpp"
#include "Task.hpp"

//-----------------------------------------------------------------------------
// 【ProjectEditor::ProjectEditor】
// 【函数功能】构造函数，绑定目标项目与验证器（栈上非所有权绑定）
// 【参数】project — 输入参数，目标项目的非 const 引用
//        validator — 输入参数，验证器的 const 引用
// 【返回值】无
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
ProjectEditor::ProjectEditor(Project&                project,
                             const ProjectValidator& validator)
    : m_project(project), m_validator(validator)
{
}

//=============================================================================
// 私有辅助：索引 ↔ ID 映射
//=============================================================================

//-----------------------------------------------------------------------------
// 【ProjectEditor::IndexToTaskId】
// 【函数功能】将容器索引转换为 TaskId
// 【参数】index — 输入参数，容器索引（0-based）
// 【返回值】对应 TaskId；越界返回 TaskId::Invalid()
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
TaskId ProjectEditor::IndexToTaskId(int index) const
{
    const auto& tasks = m_project.GetTasks();

    if ((index < 0) || (static_cast<size_t>(index) >= tasks.size()))
    {
        return TaskId::Invalid();
    }

    return tasks[static_cast<size_t>(index)].GetId();
}

//-----------------------------------------------------------------------------
// 【ProjectEditor::TaskIdToIndex】
// 【函数功能】将 TaskId 转换为容器索引
// 【参数】id — 输入参数，任务 ID
// 【返回值】对应的容器索引，未找到返回 -1
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
int ProjectEditor::TaskIdToIndex(TaskId id) const
{
    const auto& tasks = m_project.GetTasks();

    for (int i = 0; static_cast<size_t>(i) < tasks.size(); ++i)
    {
        if (tasks[static_cast<size_t>(i)].GetId() == id)
        {
            return i;
        }
    }

    return -1;
}

//-----------------------------------------------------------------------------
// 【ProjectEditor::IndexToResourceId】
// 【函数功能】将容器索引转换为 ResourceId
// 【参数】index — 输入参数，容器索引（0-based）
// 【返回值】对应 ResourceId；越界返回 ResourceId::Invalid()
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
ResourceId ProjectEditor::IndexToResourceId(int index) const
{
    const auto& resources = m_project.GetResources();

    if ((index < 0) || (static_cast<size_t>(index) >= resources.size()))
    {
        return ResourceId::Invalid();
    }

    return resources[static_cast<size_t>(index)].GetId();
}

//=============================================================================
// 私有辅助：名称唯一性
//=============================================================================

//-----------------------------------------------------------------------------
// 【ProjectEditor::IsTaskNameDuplicate】
// 【函数功能】检查任务名称是否已存在
// 【参数】name — 输入参数，待检查的名称
//        excludeId — 输入参数，排除的任务 ID（修改场景不与自己比较）
// 【返回值】true — 名称重复
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
bool ProjectEditor::IsTaskNameDuplicate(const std::string& name,
                                        TaskId             excludeId) const
{
    for (const Task& task : m_project.GetTasks())
    {
        if (task.GetId() == excludeId)
        {
            continue;
        }

        if (task.GetName() == name)
        {
            return true;
        }
    }

    return false;
}

//-----------------------------------------------------------------------------
// 【ProjectEditor::IsResourceNameDuplicate】
// 【函数功能】检查资源名称是否已存在
// 【参数】name — 输入参数，待检查的名称
// 【返回值】true — 名称重复
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
bool ProjectEditor::IsResourceNameDuplicate(const std::string& name) const
{
    for (const Resource& res : m_project.GetResources())
    {
        if (res.GetName() == name)
        {
            return true;
        }
    }

    return false;
}

//=============================================================================
// 任务管理
//=============================================================================

//-----------------------------------------------------------------------------
// 【ProjectEditor::RemoveTask】
// 【函数功能】删除指定索引的任务（需求 3.1.2），Model 层级联删除依赖与分配
// 【参数】index — 输入参数，任务容器索引
//        errorMsg — 输出参数，错误信息
// 【返回值】true — 删除成功
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
bool ProjectEditor::RemoveTask(int index, std::string& errorMsg)
{
    TaskId id = IndexToTaskId(index);

    if (id == TaskId::Invalid())
    {
        errorMsg = "索引无效";
        return false;
    }

    m_project.RemoveTask(id);
    return true;
}

//-----------------------------------------------------------------------------
// 【ProjectEditor::AddTask】
// 【函数功能】添加新任务（需求 3.1.3），名称不可重复，duration==0 自动里程碑
// 【参数】name — 输入参数，任务名称
//        duration — 输入参数，工期（>= 0）
//        errorMsg — 输出参数，错误信息
// 【返回值】true — 添加成功
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
bool ProjectEditor::AddTask(const std::string& name, int duration,
                            std::string& errorMsg)
{
    if (name.empty() == true)
    {
        errorMsg = "任务名称不能为空";
        return false;
    }

    if (IsTaskNameDuplicate(name) == true)
    {
        errorMsg = "任务名称已存在";
        return false;
    }

    m_project.AddTask(name, duration);
    return true;
}

//-----------------------------------------------------------------------------
// 【ProjectEditor::ModifyTask】
// 【函数功能】修改指定任务（需求 3.1.5），名称可改（不可重复），工期可改
// 【参数】index — 输入参数，任务容器索引
//        newName — 输入参数，新名称
//        newDuration — 输入参数，新工期
//        errorMsg — 输出参数，错误信息
// 【返回值】true — 修改成功
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
bool ProjectEditor::ModifyTask(int index, const std::string& newName,
                               int newDuration, std::string& errorMsg)
{
    TaskId id = IndexToTaskId(index);

    if (id == TaskId::Invalid())
    {
        errorMsg = "索引无效";
        return false;
    }

    Task* task = m_project.FindTask(id);

    if (task == nullptr)
    {
        errorMsg = "任务不存在";
        return false;
    }

    // 名称变更检查
    if (newName != task->GetName())
    {
        if (IsTaskNameDuplicate(newName, id) == true)
        {
            errorMsg = "任务名称已存在";
            return false;
        }

        task->SetName(newName);
    }

    // 工期变更检查
    if (newDuration != task->GetDuration())
    {
        task->SetDuration(newDuration);
    }

    return true;
}

//=============================================================================
// 依赖管理
//=============================================================================

//-----------------------------------------------------------------------------
// 【ProjectEditor::RemoveDependency】
// 【函数功能】删除指定索引的依赖（需求 3.2.2）
// 【参数】index — 输入参数，依赖容器索引
//        errorMsg — 输出参数，错误信息
// 【返回值】true — 删除成功
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
bool ProjectEditor::RemoveDependency(int index, std::string& errorMsg)
{
    const auto& deps = m_project.GetDependencies();

    if ((index < 0) || (static_cast<size_t>(index) >= deps.size()))
    {
        errorMsg = "索引无效";
        return false;
    }

    const Dependency& dep  = deps[static_cast<size_t>(index)];
    TaskId            pred = dep.GetPredecessorId();
    TaskId            succ = dep.GetSuccessorId();

    m_project.RemoveDependency(pred, succ);
    return true;
}

//-----------------------------------------------------------------------------
// 【ProjectEditor::AddDependency】
// 【函数功能】添加新依赖（需求 3.2.3），唯一性约束 + DAG 环检测
// 【参数】predIndex — 输入参数，前序任务索引
//        succIndex — 输入参数，后继任务索引
//        type — 输入参数，依赖类型
//        lag — 输入参数，时差
//        errorMsg — 输出参数，错误信息
// 【返回值】true — 添加成功
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
bool ProjectEditor::AddDependency(int predIndex, int succIndex,
                                  DependencyType type, int lag,
                                  std::string& errorMsg)
{
    TaskId predId = IndexToTaskId(predIndex);

    if (predId == TaskId::Invalid())
    {
        errorMsg = "前置任务索引无效";
        return false;
    }

    TaskId succId = IndexToTaskId(succIndex);

    if (succId == TaskId::Invalid())
    {
        errorMsg = "后置任务索引无效";
        return false;
    }

    if (predId == succId)
    {
        errorMsg = "不能创建自引用依赖";
        return false;
    }

    if (m_project.FindDependency(predId, succId) != nullptr)
    {
        errorMsg = "该依赖已存在";
        return false;
    }

    if (m_validator.WouldCreateCycle(m_project, predId, succId) == true)
    {
        errorMsg = "添加此依赖会产生循环";
        return false;
    }

    m_project.AddDependency(predId, succId, type, lag);
    return true;
}

//=============================================================================
// 资源管理
//=============================================================================

//-----------------------------------------------------------------------------
// 【ProjectEditor::AddResource】
// 【函数功能】添加新资源（需求 3.3.2），名称不可重复
// 【参数】name — 输入参数，资源名称
//        unitCost — 输入参数，单位成本
//        errorMsg — 输出参数，错误信息
// 【返回值】true — 添加成功
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
bool ProjectEditor::AddResource(const std::string& name, double unitCost,
                                std::string& errorMsg)
{
    if (name.empty() == true)
    {
        errorMsg = "资源名称不能为空";
        return false;
    }

    if (IsResourceNameDuplicate(name) == true)
    {
        errorMsg = "资源名称已存在";
        return false;
    }

    m_project.AddResource(name, unitCost);
    return true;
}

//-----------------------------------------------------------------------------
// 【ProjectEditor::AssignResource】
// 【函数功能】为任务分配资源（需求 3.3.3），里程碑不可分配
// 【参数】taskIndex — 输入参数，任务容器索引
//        resourceIndex — 输入参数，资源容器索引
//        quantity — 输入参数，占用数量（必须 > 0）
//        errorMsg — 输出参数，错误信息
// 【返回值】true — 分配成功
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
bool ProjectEditor::AssignResource(int taskIndex, int resourceIndex,
                                   int quantity, std::string& errorMsg)
{
    TaskId taskId = IndexToTaskId(taskIndex);

    if (taskId == TaskId::Invalid())
    {
        errorMsg = "任务索引无效";
        return false;
    }

    ResourceId resId = IndexToResourceId(resourceIndex);

    if (resId == ResourceId::Invalid())
    {
        errorMsg = "资源索引无效";
        return false;
    }

    if (quantity <= 0)
    {
        errorMsg = "分配数量必须为正整数";
        return false;
    }

    const Task* task = m_project.FindTask(taskId);

    if (task == nullptr)
    {
        errorMsg = "任务不存在";
        return false;
    }

    if (task->CanAllocateResource() == false)
    {
        errorMsg = "里程碑任务不可分配资源";
        return false;
    }

    m_project.AssignResource(taskId, resId, quantity);
    return true;
}
