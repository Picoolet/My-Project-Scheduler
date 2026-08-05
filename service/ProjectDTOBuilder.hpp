//-----------------------------------------------------------------------------
// 【ProjectDTOBuilder.hpp】
// 【DTO 构建器类声明，将 Model 层数据转换为界面层 DTO】
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------

#ifndef PROJECTDTOBUILDER_HPP
#define PROJECTDTOBUILDER_HPP

#include <utility>
#include <vector>

#include "DependencyDTO.hpp"
#include "ProjectStatisticsDTO.hpp"
#include "ResourceDTO.hpp"
#include "TaskDTO.hpp"

class CPMCalculator;
class Project;
class ProjectValidator;
class Task;

//-----------------------------------------------------------------------------
// 【ProjectDTOBuilder 类】
// 【功能】遍历 const Project&，组装界面层所需的全部 DTO
// 【接口说明】无状态，所有方法为 const，一个实例可反复用于不同 Project
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
class ProjectDTOBuilder
{
  public:
    ProjectDTOBuilder()                                    = default;
    ProjectDTOBuilder(const ProjectDTOBuilder&)            = default;
    ProjectDTOBuilder& operator=(const ProjectDTOBuilder&) = default;
    ~ProjectDTOBuilder()                                   = default;

    // 需求 3.1.1：按容器索引顺序返回全部 TaskDTO
    std::vector<TaskDTO> BuildTaskDTOs(const Project& project) const;

    // 需求 3.1.4：返回指定任务的前驱与后继 DTO
    std::pair<std::vector<TaskDTO>, std::vector<TaskDTO>> BuildTaskRelations(
        const Project& project, int index) const;

    // 需求 3.2.1：返回全部 DependencyDTO
    std::vector<DependencyDTO> BuildDependencyDTOs(
        const Project& project) const;

    // 需求 3.3.1：返回全部 ResourceDTO
    std::vector<ResourceDTO> BuildResourceDTOs(const Project& project) const;

    // 需求 3.3.3 扩展：返回指定任务已分配的资源 DTO 列表
    std::vector<ResourceDTO> BuildTaskResources(const Project& project,
                                                int            taskIndex) const;

    // 需求 4：返回统计信息（内部执行 Validate + 若通过则 CPM Calculate）
    ProjectStatisticsDTO BuildStatistics(const Project&          project,
                                         const ProjectValidator& validator,
                                         const CPMCalculator& calculator) const;

  private:
    // 构建单个 TaskDTO（供 BuildTaskDTOs 和 BuildTaskRelations 复用）
    TaskDTO BuildSingleTaskDTO(const Project& project, int index,
                               const Task& task) const;
};

#endif
