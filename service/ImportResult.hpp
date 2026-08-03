//-----------------------------------------------------------------------------
// 【ImportResult.hpp】
// 【导入结果类声明，封装导入操作的完整结果（Project + errors + warnings）】
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------

#ifndef IMPORTRESULT_HPP
#define IMPORTRESULT_HPP

#include <memory>
#include <string>
#include <vector>

class Project;

//-----------------------------------------------------------------------------
// 【ImportResult 类】
// 【功能】封装导入操作的完整结果：Project（可空） + errors + warnings
// 【接口说明】Project 访问有且仅有一条路径 — ReleaseProject() 转移所有权
//            无 GetProject() 借阅方法；错误检查通过 GetErrors/GetWarnings 完成
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
class ImportResult
{
  public:
    // 默认构造：失败结果（无 Project，无 errors/warnings）
    ImportResult();

    // 成功构造：仅含 Project（无 errors/warnings）
    explicit ImportResult(std::unique_ptr<Project> project);

    // 完整构造：Project + errors + warnings
    ImportResult(std::unique_ptr<Project>        project,
                 const std::vector<std::string>& errors,
                 const std::vector<std::string>& warnings);

    ImportResult(const ImportResult&)            = delete;
    ImportResult& operator=(const ImportResult&) = delete;
    ImportResult(ImportResult&&)                 = default;
    ImportResult& operator=(ImportResult&&)      = default;
    ~ImportResult()                              = default;

    // 移交 Project 所有权（唯一访问路径）
    std::unique_ptr<Project> ReleaseProject();

    // 是否含有错误（true 表示导入失败或部分失败）
    bool HasErrors() const;

    // 是否含有警告（true 表示部分行跳过但仍成功）
    bool HasWarnings() const;

    // 获取全部错误信息
    const std::vector<std::string>& GetErrors() const;

    // 获取全部警告信息
    const std::vector<std::string>& GetWarnings() const;

  private:
    std::unique_ptr<Project> m_project;  // 导入的 Project（可空）
    std::vector<std::string> m_errors;   // 错误列表
    std::vector<std::string> m_warnings; // 警告列表
};

#endif
