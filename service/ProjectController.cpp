//-----------------------------------------------------------------------------
// 【ProjectController.cpp】
// 【单例控制器类实现】
// 【开发者及日期】 QJQ 2026.8.2
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
// 【开发者及日期】 QJQ 2026.8.2
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
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
ProjectController::ProjectController()
    : m_project(nullptr), m_pPpmImporter(std::make_unique<PpmImporter>()),
      m_pPpmExporter(std::make_unique<PpmExporter>())
{
}

//-----------------------------------------------------------------------------
// 【ProjectController::CreateEditor】
// 【函数功能】在栈上创建 ProjectEditor 并返回
// 【参数】无
// 【返回值】绑定当前 Project 和 Validator 的 ProjectEditor 实例
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
ProjectEditor ProjectController::CreateEditor()
{
    return ProjectEditor(*m_project, m_validator);
}

//=============================================================================
// 项目状态
//=============================================================================

//-----------------------------------------------------------------------------
// 【ProjectController::HasProject】
// 【函数功能】判断是否已加载项目
// 【参数】无
// 【返回值】true — 已加载有效项目
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
bool ProjectController::HasProject() const
{
    return (m_project != nullptr);
}

//-----------------------------------------------------------------------------
// 【ProjectController::GetProject】
// 【函数功能】获取当前项目的只读指针
// 【参数】无
// 【返回值】const Project*，未加载时返回 nullptr
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
const Project* ProjectController::GetProject() const
{
    return m_project.get();
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
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
bool ProjectController::ImportProject(const std::string& filePath,
                                      std::string&       errorMsg)
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

    m_project = result.ReleaseProject();
    return true;
}

//-----------------------------------------------------------------------------
// 【ProjectController::ExportProject】
// 【函数功能】根据文件扩展名选择导出器，将项目写出到文件（需求 2）
// 【参数】filePath — 输入参数，文件路径
//        errorMsg — 输出参数，错误信息
// 【返回值】true — 导出成功
// 【开发者及日期】 QJQ 2026.8.2
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
        success = m_pPpmExporter->Export(*m_project, filePath);
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

std::vector<TaskDTO> ProjectController::ListTasks() const
{
    if (HasProject() == false)
    {
        return {};
    }

    return m_dtoBuilder.BuildTaskDTOs(*m_project);
}

bool ProjectController::RemoveTask(int index, std::string& errorMsg)
{
    if (HasProject() == false)
    {
        errorMsg = "无项目";
        return false;
    }

    return CreateEditor().RemoveTask(index, errorMsg);
}

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

std::pair<std::vector<TaskDTO>, std::vector<TaskDTO>> ProjectController::
    GetTaskRelations(int index) const
{
    if (HasProject() == false)
    {
        return {};
    }

    return m_dtoBuilder.BuildTaskRelations(*m_project, index);
}

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

std::vector<DependencyDTO> ProjectController::ListDependencies() const
{
    if (HasProject() == false)
    {
        return {};
    }

    return m_dtoBuilder.BuildDependencyDTOs(*m_project);
}

bool ProjectController::RemoveDependency(int index, std::string& errorMsg)
{
    if (HasProject() == false)
    {
        errorMsg = "无项目";
        return false;
    }

    return CreateEditor().RemoveDependency(index, errorMsg);
}

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

std::vector<ResourceDTO> ProjectController::ListResources() const
{
    if (HasProject() == false)
    {
        return {};
    }

    return m_dtoBuilder.BuildResourceDTOs(*m_project);
}

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

    return m_dtoBuilder.BuildStatistics(*m_project, m_validator, m_calculator);
}

ValidationResult ProjectController::Validate() const
{
    if (HasProject() == false)
    {
        return ValidationResult({"无项目可验证"});
    }

    return m_validator.Validate(*m_project);
}

ScheduleResult ProjectController::ComputeSchedule() const
{
    if (HasProject() == false)
    {
        return ScheduleResult({}, 0, {});
    }

    return m_calculator.Calculate(*m_project);
}
