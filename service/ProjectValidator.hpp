//-----------------------------------------------------------------------------
// 【ProjectValidator.hpp】
// 【项目合理性验证器类声明，对 const Project& 执行图分析】
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------

#ifndef PROJECTVALIDATOR_HPP
#define PROJECTVALIDATOR_HPP

#include <string>
#include <vector>

#include "Id.hpp"
#include "ValidationResult.hpp"

class Project;

//-----------------------------------------------------------------------------
// 【ProjectValidator 类】
// 【功能】对 const Project& 执行三条图检查 + 单边环检测
// 【接口说明】无状态，所有方法为 const，一个实例可反复用于不同 Project
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
class ProjectValidator
{
  public:
    ProjectValidator()                                   = default;
    ProjectValidator(const ProjectValidator&)            = default;
    ProjectValidator& operator=(const ProjectValidator&) = default;
    ~ProjectValidator()                                  = default;

    // 执行全部三条检查，收集所有问题后一次性返回
    ValidationResult Validate(const Project& project) const;

    // 单边环检测：若添加 (pred→succ) 是否会形成环，供 Editor 调用
    bool WouldCreateCycle(const Project& project, TaskId pred,
                          TaskId succ) const;

  private:
    // ① 依赖图无环：Kahn 算法
    void CheckAcyclic(const Project&            project,
                      std::vector<std::string>& errors) const;

    // ② 无悬挂节点：正向可达 ∩ 反向可达 = 全部任务
    void CheckNoDangling(const Project&            project,
                         std::vector<std::string>& errors) const;

    // ③ 引用完整性：每条 Dependency 的 pred/succ 均存在
    void CheckReferenceIntegrity(const Project&            project,
                                 std::vector<std::string>& errors) const;
};

#endif
