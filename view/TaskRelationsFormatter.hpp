//-----------------------------------------------------------------------------
// 【TaskRelationsFormatter.hpp】
// 【任务详情格式化器类声明】
// 【开发者及日期】 QJQ 2026.8.6
//-----------------------------------------------------------------------------

#ifndef TASKRELATIONSFORMATTER_HPP
#define TASKRELATIONSFORMATTER_HPP

#include <string>
#include <utility>
#include <vector>

#include "ResourceDTO.hpp"
#include "TaskDTO.hpp"

//-----------------------------------------------------------------------------
// 【TaskRelationsFormatter 类】
// 【功能】将任务详情（自身信息 + 前驱/后继 + 资源列表）格式化为分区文本
// 【接口说明】纯静态方法类，禁止实例化
// 【开发者及日期】 QJQ 2026.8.6
//-----------------------------------------------------------------------------
class TaskRelationsFormatter
{
  public:
    TaskRelationsFormatter() = delete;

    // 格式化任务详情：先展示任务自身索引/ID/名称，再展示前驱、后继与资源列表
    static std::string Format(
        const TaskDTO&                                               task,
        const std::pair<std::vector<TaskDTO>, std::vector<TaskDTO>>& relations,
        const std::vector<ResourceDTO>&                              resources);

  private:
    // 格式化单个任务分区（标题 + 条目列表），空列表显示 "(无)"
    static std::string FormatSection(const std::string&          title,
                                     const std::vector<TaskDTO>& items);
};

#endif
