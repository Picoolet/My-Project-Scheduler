//-----------------------------------------------------------------------------
// 【TaskListFormatter.hpp】
// 【任务列表格式化器类声明】
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------

#ifndef TASKLISTFORMATTER_HPP
#define TASKLISTFORMATTER_HPP

#include <string>
#include <vector>

#include "TaskDTO.hpp"

//-----------------------------------------------------------------------------
// 【TaskListFormatter 类】
// 【功能】将 TaskDTO 列表格式化为五列文本表格
// 【接口说明】纯静态方法类，禁止实例化
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
class TaskListFormatter
{
  public:
    TaskListFormatter() = delete;

    // 将任务列表格式化为表格文本；空列表返回 "  暂无任务\n"
    static std::string Format(const std::vector<TaskDTO>& tasks);

  private:
    // 将前驱/后继索引列表格式化为 "[1, 3, 5]"；空返回 ""
    static std::string FormatIndices(const std::vector<int>& indices);
};

#endif
