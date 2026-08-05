//-----------------------------------------------------------------------------
// 【ProjectController.hpp】
// 【单例控制器类声明，Service 层唯一对外入口】
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------

#ifndef PROJECTCONTROLLER_HPP
#define PROJECTCONTROLLER_HPP

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "CPMCalculator.hpp"
#include "DependencyDTO.hpp"
#include "DependencyType.hpp"
#include "ProjectDTOBuilder.hpp"
#include "ProjectStatisticsDTO.hpp"
#include "ProjectValidator.hpp"
#include "ResourceDTO.hpp"
#include "ScheduleResult.hpp"
#include "TaskDTO.hpp"
#include "ValidationResult.hpp"

class IProjectExporter;
class IProjectImporter;
class PpmExporter;
class PpmImporter;
class Project;
class ProjectEditor;

//-----------------------------------------------------------------------------
// 【ProjectController 类】
// 【功能】Service 层单例控制器，持有 Project 和全部 Service 组件，统一对外
//        提供所有服务操作
// 【接口说明】Meyer's Singleton，禁止拷贝/移动，薄层转发至下属组件
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
class ProjectController
{
  public:
    //------ 单例 ------
    static ProjectController& GetInstance();

    // 禁止拷贝和移动
    ProjectController(const ProjectController&)            = delete;
    ProjectController& operator=(const ProjectController&) = delete;

    //------ 项目导入（需求 1）------
    bool ImportProject(const std::string& filePath, std::string& errorMsg,
                       std::vector<std::string>* warnings = nullptr);

    //------ 项目导出（需求 2）------
    bool ExportProject(const std::string& filePath,
                       std::string&       errorMsg) const;

    //------ 项目状态 ------
    bool           HasProject() const;
    const Project* GetProject() const;

    //------ 任务管理（需求 3.1）------
    std::vector<TaskDTO> ListTasks() const;
    bool                 RemoveTask(int index, std::string& errorMsg);
    bool AddTask(const std::string& name, int duration, std::string& errorMsg);
    std::pair<std::vector<TaskDTO>, std::vector<TaskDTO>> GetTaskRelations(
        int index) const;
    bool ModifyTask(int index, const std::string& newName, int newDuration,
                    std::string& errorMsg);

    //------ 依赖管理（需求 3.2）------
    std::vector<DependencyDTO> ListDependencies() const;
    bool RemoveDependency(int index, std::string& errorMsg);
    bool RemoveDependency(int predIndex, int succIndex, std::string& errorMsg);
    bool AddDependency(int predIndex, int succIndex, DependencyType type,
                       int lag, std::string& errorMsg);

    //------ 资源管理（需求 3.3）------
    std::vector<ResourceDTO> ListResources() const;
    std::vector<ResourceDTO> GetTaskResources(int taskIndex) const;
    bool AddResource(const std::string& name, double unitCost,
                     std::string& errorMsg);
    bool AssignResource(int taskIndex, int resourceIndex, int quantity,
                        std::string& errorMsg);

    //------ 统计信息（需求 4）------
    ProjectStatisticsDTO GetStatistics() const;

    //------ 验证与调度（需求 5）------
    ValidationResult Validate() const;
    ScheduleResult   ComputeSchedule() const;

  private:
    ProjectController();
    ~ProjectController() = default;

    // 按需创建 Editor（每次编辑操作栈上构造，RAII 自动销毁）
    ProjectEditor CreateEditor();

    //------ 成员 ------
    std::unique_ptr<Project>     m_pProject;
    ProjectValidator             m_validator;
    CPMCalculator                m_calculator;
    ProjectDTOBuilder            m_dtoBuilder;
    std::unique_ptr<PpmImporter> m_pPpmImporter;
    std::unique_ptr<PpmExporter> m_pPpmExporter;
    // ManualImporter 不在 Controller 中（仅供测试用），直接构造调用即可
};

#endif
