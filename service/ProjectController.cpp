//-----------------------------------------------------------------------------
// 【ProjectController.cpp】
// 【单例控制器类实现】
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------

#include "ProjectController.hpp"

#include <string>

#include "IProjectExporter.hpp"
#include "IProjectImporter.hpp"
#include "ImportResult.hpp"
#include "PpmExporter.hpp"
#include "PpmImporter.hpp"
#include "Project.hpp"
#include "ProjectEditor.hpp"

//-----------------------------------------------------------------------------
// 【ProjectController::GetInstance】
// 【函数功能】返回全局唯一控制器实例（Meyer's Singleton，C++11 线程安全）
// 【参数】无
// 【返回值】ProjectController 单例引用
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
ProjectController& ProjectController::GetInstance()
{
    static ProjectController instance;
    return instance;
}

//-----------------------------------------------------------------------------
// 【ProjectController::ProjectController】
// 【函数功能】私有构造函数，初始化全部 Service 组件
// 【参数】无
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
ProjectController::ProjectController()
    : m_pProject(nullptr), m_pPpmImporter(std::make_unique<PpmImporter>()),
      m_pPpmExporter(std::make_unique<PpmExporter>())
{
}

//-----------------------------------------------------------------------------
// 【ProjectController::CreateEditor】
// 【函数功能】在栈上创建 ProjectEditor 并返回
// 【参数】无
// 【返回值】绑定当前 Project 和 Validator 的 ProjectEditor 实例
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
ProjectEditor ProjectController::CreateEditor()
{
    return ProjectEditor(*m_pProject, m_validator);
}

//=============================================================================
// 项目状态
//=============================================================================

//-----------------------------------------------------------------------------
// 【ProjectController::HasProject】
// 【函数功能】判断是否已加载项目
// 【参数】无
// 【返回值】true — 已加载有效项目
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
bool ProjectController::HasProject() const
{
    return (m_pProject != nullptr);
}

//-----------------------------------------------------------------------------
// 【ProjectController::GetProject】
// 【函数功能】获取当前项目的只读指针
// 【参数】无
// 【返回值】const Project*，未加载时返回 nullptr
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
const Project* ProjectController::GetProject() const
{
    return m_pProject.get();
}

//=============================================================================
// 导入导出
//=============================================================================

//-----------------------------------------------------------------------------
// 【ProjectController::ImportProject】
// 【函数功能】根据文件扩展名选择导入器，导入并替换当前项目（需求 1）
// 【参数】filePath — 输入参数，文件路径
//        errorMsg — 输出参数，错误信息
// 【返回值】true — 导入成功
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
bool ProjectController::ImportProject(const std::string&        filePath,
                                      std::string&              errorMsg,
                                      std::vector<std::string>* warnings)
{
    // 解析扩展名
    std::string extension;
    size_t      dotPos = filePath.rfind('.');

    if (dotPos != std::string::npos)
    {
        extension = filePath.substr(dotPos);

        // 转为小写
        for (char& ch : extension)
        {
            if ((ch >= 'A') && (ch <= 'Z'))
            {
                ch = static_cast<char>(ch + ('a' - 'A'));
            }
        }
    }

    ImportResult result(nullptr, {"不支持的文件格式"}, {});

    if (extension == ".ppm")
    {
        result = m_pPpmImporter->Import(filePath);
    }

    // 若调用方关心警告，转发 ImportResult 的警告信息
    if (warnings != nullptr)
    {
        *warnings = result.GetWarnings();
    }

    if (result.HasErrors() == true)
    {
        errorMsg.clear();

        for (const std::string& err : result.GetErrors())
        {
            if (errorMsg.empty() == false)
            {
                errorMsg += "; ";
            }

            errorMsg += err;
        }

        return false;
    }

    m_pProject = result.ReleaseProject();
    return true;
}

//-----------------------------------------------------------------------------
// 【ProjectController::ExportProject】
// 【函数功能】根据文件扩展名选择导出器，将项目写出到文件（需求 2）
// 【参数】filePath — 输入参数，文件路径
//        errorMsg — 输出参数，错误信息
// 【返回值】true — 导出成功
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
bool ProjectController::ExportProject(const std::string& filePath,
                                      std::string&       errorMsg) const
{
    if (HasProject() == false)
    {
        errorMsg = "无项目可导出";
        return false;
    }

    // 解析扩展名
    std::string extension;
    size_t      dotPos = filePath.rfind('.');

    if (dotPos != std::string::npos)
    {
        extension = filePath.substr(dotPos);

        for (char& ch : extension)
        {
            if ((ch >= 'A') && (ch <= 'Z'))
            {
                ch = static_cast<char>(ch + ('a' - 'A'));
            }
        }
    }

    bool success = false;

    if (extension == ".ppm")
    {
        success = m_pPpmExporter->Export(*m_pProject, filePath);
    }
    else
    {
        errorMsg = "不支持的文件格式";
        return false;
    }

    if (success == false)
    {
        errorMsg = "导出失败";
        return false;
    }

    return true;
}

//=============================================================================
// 任务管理
//=============================================================================

//-----------------------------------------------------------------------------
// 【ProjectController::ListTasks】
// 【函数功能】按容器索引顺序返回全部任务的 DTO 列表（需求 3.1.1）
// 【参数】无
// 【返回值】TaskDTO 列表；无项目时返回空列表
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
std::vector<TaskDTO> ProjectController::ListTasks() const
{
    if (HasProject() == false)
    {
        return {};
    }

    return m_dtoBuilder.BuildTaskDTOs(*m_pProject);
}

//-----------------------------------------------------------------------------
// 【ProjectController::RemoveTask】
// 【函数功能】删除指定索引的任务（需求 3.1.2），级联删除依赖与分配
// 【参数】index — 输入参数，任务容器索引
//        errorMsg — 输出参数，错误信息
// 【返回值】true — 删除成功
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
bool ProjectController::RemoveTask(int index, std::string& errorMsg)
{
    if (HasProject() == false)
    {
        errorMsg = "无项目";
        return false;
    }

    return CreateEditor().RemoveTask(index, errorMsg);
}

//-----------------------------------------------------------------------------
// 【ProjectController::AddTask】
// 【函数功能】添加新任务（需求 3.1.3），名称不可重复
// 【参数】name — 输入参数，任务名称
//        duration — 输入参数，工期
//        errorMsg — 输出参数，错误信息
// 【返回值】true — 添加成功
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
bool ProjectController::AddTask(const std::string& name, int duration,
                                std::string& errorMsg)
{
    if (HasProject() == false)
    {
        errorMsg = "无项目";
        return false;
    }

    return CreateEditor().AddTask(name, duration, errorMsg);
}

//-----------------------------------------------------------------------------
// 【ProjectController::GetTaskRelations】
// 【函数功能】返回指定任务的前驱与后继 DTO 列表（需求 3.1.4）
// 【参数】index — 输入参数，任务容器索引
// 【返回值】pair<前驱 DTO 列表, 后继 DTO 列表>；无项目时返回空 pair
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
std::pair<std::vector<TaskDTO>, std::vector<TaskDTO>> ProjectController::
    GetTaskRelations(int index) const
{
    if (HasProject() == false)
    {
        return {};
    }

    return m_dtoBuilder.BuildTaskRelations(*m_pProject, index);
}

//-----------------------------------------------------------------------------
// 【ProjectController::ModifyTask】
// 【函数功能】修改指定任务的名称或工期（需求 3.1.5）
// 【参数】index — 输入参数，任务容器索引
//        newName — 输入参数，新名称
//        newDuration — 输入参数，新工期
//        errorMsg — 输出参数，错误信息
// 【返回值】true — 修改成功
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
bool ProjectController::ModifyTask(int index, const std::string& newName,
                                   int newDuration, std::string& errorMsg)
{
    if (HasProject() == false)
    {
        errorMsg = "无项目";
        return false;
    }

    return CreateEditor().ModifyTask(index, newName, newDuration, errorMsg);
}

//=============================================================================
// 依赖管理
//=============================================================================

//-----------------------------------------------------------------------------
// 【ProjectController::ListDependencies】
// 【函数功能】按序号返回全部依赖的 DTO 列表（需求 3.2.1）
// 【参数】无
// 【返回值】DependencyDTO 列表；无项目时返回空列表
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
std::vector<DependencyDTO> ProjectController::ListDependencies() const
{
    if (HasProject() == false)
    {
        return {};
    }

    return m_dtoBuilder.BuildDependencyDTOs(*m_pProject);
}

//-----------------------------------------------------------------------------
// 【ProjectController::RemoveDependency】
// 【函数功能】删除指定序号的依赖（需求 3.2.2）
// 【参数】index — 输入参数，依赖序号
//        errorMsg — 输出参数，错误信息
// 【返回值】true — 删除成功
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
bool ProjectController::RemoveDependency(int index, std::string& errorMsg)
{
    if (HasProject() == false)
    {
        errorMsg = "无项目";
        return false;
    }

    return CreateEditor().RemoveDependency(index, errorMsg);
}

//-----------------------------------------------------------------------------
// 【ProjectController::RemoveDependency（按前后继索引）】
// 【函数功能】按前驱/后继任务索引删除匹配的依赖（需求 3.2.2，View_Goal §2.3）
// 【参数】predIndex — 输入参数，前序任务索引
//        succIndex — 输入参数，后继任务索引
//        errorMsg — 输出参数，错误信息
// 【返回值】true — 删除成功
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
bool ProjectController::RemoveDependency(int predIndex, int succIndex,
                                         std::string& errorMsg)
{
    if (HasProject() == false)
    {
        errorMsg = "无项目";
        return false;
    }

    ProjectEditor editor = CreateEditor();
    TaskId        predId = editor.IndexToTaskId(predIndex);
    TaskId        succId = editor.IndexToTaskId(succIndex);

    if ((predId == TaskId::Invalid()) || (succId == TaskId::Invalid()))
    {
        errorMsg = "任务索引无效";
        return false;
    }

    const auto& deps = m_pProject->GetDependencies();

    for (size_t i = 0; i < deps.size(); ++i)
    {
        if ((deps[i].GetPredecessorId() == predId)
            && (deps[i].GetSuccessorId() == succId))
        {
            return editor.RemoveDependency(static_cast<int>(i), errorMsg);
        }
    }

    errorMsg = "未找到匹配的依赖关系";
    return false;
}

//-----------------------------------------------------------------------------
// 【ProjectController::AddDependency】
// 【函数功能】添加新依赖（需求 3.2.3），包含唯一性与无环检测
// 【参数】predIndex — 输入参数，前序任务索引
//        succIndex — 输入参数，后继任务索引
//        type — 输入参数，依赖类型
//        lag — 输入参数，时差
//        errorMsg — 输出参数，错误信息
// 【返回值】true — 添加成功
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
bool ProjectController::AddDependency(int predIndex, int succIndex,
                                      DependencyType type, int lag,
                                      std::string& errorMsg)
{
    if (HasProject() == false)
    {
        errorMsg = "无项目";
        return false;
    }

    return CreateEditor().AddDependency(predIndex, succIndex, type, lag,
                                        errorMsg);
}

//=============================================================================
// 资源管理
//=============================================================================

//-----------------------------------------------------------------------------
// 【ProjectController::ListResources】
// 【函数功能】按容器索引顺序返回全部资源的 DTO 列表（需求 3.3.1）
// 【参数】无
// 【返回值】ResourceDTO 列表；无项目时返回空列表
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
std::vector<ResourceDTO> ProjectController::ListResources() const
{
    if (HasProject() == false)
    {
        return {};
    }

    return m_dtoBuilder.BuildResourceDTOs(*m_pProject);
}

//-----------------------------------------------------------------------------
// 【ProjectController::GetTaskResources】
// 【函数功能】返回指定任务已分配的资源 DTO 列表（需求 3.3.3 展示辅助）
// 【参数】taskIndex — 输入参数，任务容器索引
// 【返回值】ResourceDTO 列表；无项目或越界时返回空列表
// 【开发者及日期】QJQ 2026.8.6
// 【更改记录】 无
//-----------------------------------------------------------------------------
std::vector<ResourceDTO> ProjectController::GetTaskResources(
    int taskIndex) const
{
    if (HasProject() == false)
    {
        return {};
    }

    return m_dtoBuilder.BuildTaskResources(*m_pProject, taskIndex);
}

//-----------------------------------------------------------------------------
// 【ProjectController::AddResource】
// 【函数功能】添加新资源（需求 3.3.2），名称不可重复
// 【参数】name — 输入参数，资源名称
//        unitCost — 输入参数，单位成本
//        errorMsg — 输出参数，错误信息
// 【返回值】true — 添加成功
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
bool ProjectController::AddResource(const std::string& name, double unitCost,
                                    std::string& errorMsg)
{
    if (HasProject() == false)
    {
        errorMsg = "无项目";
        return false;
    }

    return CreateEditor().AddResource(name, unitCost, errorMsg);
}

//-----------------------------------------------------------------------------
// 【ProjectController::AssignResource】
// 【函数功能】为任务分配资源（需求 3.3.3），里程碑不可分配
// 【参数】taskIndex — 输入参数，任务容器索引
//        resourceIndex — 输入参数，资源容器索引
//        quantity — 输入参数，占用数量
//        errorMsg — 输出参数，错误信息
// 【返回值】true — 分配成功
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
bool ProjectController::AssignResource(int taskIndex, int resourceIndex,
                                       int quantity, std::string& errorMsg)
{
    if (HasProject() == false)
    {
        errorMsg = "无项目";
        return false;
    }

    return CreateEditor().AssignResource(taskIndex, resourceIndex, quantity,
                                         errorMsg);
}

//=============================================================================
// 统计与调度
//=============================================================================

//-----------------------------------------------------------------------------
// 【ProjectController::GetStatistics】
// 【函数功能】构建项目统计信息（需求 4），内部执行验证与 CPM 调度
// 【参数】无
// 【返回值】ProjectStatisticsDTO（含 isValid 标志和 totalDuration）
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
ProjectStatisticsDTO ProjectController::GetStatistics() const
{
    if (HasProject() == false)
    {
        ProjectStatisticsDTO empty;
        empty.taskCount       = 0;
        empty.dependencyCount = 0;
        empty.resourceCount   = 0;
        empty.isValid         = false;
        empty.totalDuration   = -1;
        return empty;
    }

    return m_dtoBuilder.BuildStatistics(*m_pProject, m_validator, m_calculator);
}

//-----------------------------------------------------------------------------
// 【ProjectController::Validate】
// 【函数功能】执行项目合理性验证（需求 5）：无环、无悬挂、引用完整
// 【参数】无
// 【返回值】ValidationResult，IsValid()==true 表示全部通过
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
ValidationResult ProjectController::Validate() const
{
    if (HasProject() == false)
    {
        return ValidationResult({"无项目可验证"});
    }

    return m_validator.Validate(*m_pProject);
}

//-----------------------------------------------------------------------------
// 【ProjectController::ComputeSchedule】
// 【函数功能】执行 CPM 调度计算（需求 5），返回 ES/EF/LS/LF + 关键路径
// 【参数】无
// 【返回值】ScheduleResult；无项目时返回空结果
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
ScheduleResult ProjectController::ComputeSchedule() const
{
    if (HasProject() == false)
    {
        return ScheduleResult({}, 0, {});
    }

    return m_calculator.Calculate(*m_pProject);
}
