//-----------------------------------------------------------------------------
// 【TaskRelationsFormatter.hpp】
// 【任务关系格式化器类声明】
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------

#ifndef TASKRELATIONSFORMATTER_HPP
#define TASKRELATIONSFORMATTER_HPP

#include <string>
#include <utility>
#include <vector>

#include "TaskDTO.hpp"

//-----------------------------------------------------------------------------
// 【TaskRelationsFormatter 类】
// 【功能】将任务的前驱/后继关系格式化为分区列表文本
// 【接口说明】纯静态方法类，禁止实例化
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
class TaskRelationsFormatter
{
  public:
    TaskRelationsFormatter() = delete;

    // 分两区显示前驱和后继的序号+名称；空列表显示 "(无)"
    static std::string Format(
        const std::pair<std::vector<TaskDTO>, std::vector<TaskDTO>>& relations);

  private:
    // 格式化单个分区（标题 + 条目列表）
    static std::string FormatSection(const std::string&          title,
                                     const std::vector<TaskDTO>& items);
};

#endif
