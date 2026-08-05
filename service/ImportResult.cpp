//-----------------------------------------------------------------------------
// 【ImportResult.cpp】
// 【导入结果类实现】
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------

#include "ImportResult.hpp"

#include "Project.hpp"

//-----------------------------------------------------------------------------
// 【ImportResult::ImportResult（默认构造）】
// 【函数功能】构造一个失败的导入结果（无 Project，无 errors/warnings）
// 【参数】无
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
ImportResult::ImportResult()
{
}

//-----------------------------------------------------------------------------
// 【ImportResult::ImportResult（单参）】
// 【函数功能】构造一个成功的导入结果（仅含 Project，无 errors/warnings）
// 【参数】project — 输入参数，导入成功的 Project（转移所有权）
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
ImportResult::ImportResult(std::unique_ptr<Project> project)
    : m_pProject(std::move(project))
{
}

//-----------------------------------------------------------------------------
// 【ImportResult::ImportResult（三参）】
// 【函数功能】构造一个含 errors/warnings 的导入结果
// 【参数】project — 输入参数，导入的 Project（转移所有权，可为空）
//        errors — 输入参数，错误信息列表
//        warnings — 输入参数，警告信息列表
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
ImportResult::ImportResult(std::unique_ptr<Project>        project,
                           const std::vector<std::string>& errors,
                           const std::vector<std::string>& warnings)
    : m_pProject(std::move(project)), m_errors(errors), m_warnings(warnings)
{
}

//-----------------------------------------------------------------------------
// 【ImportResult::ReleaseProject】
// 【函数功能】移交 Project 所有权，调用后 ImportResult 不再持有 Project
// 【参数】无
// 【返回值】Project 的 unique_ptr
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
std::unique_ptr<Project> ImportResult::ReleaseProject()
{
    return std::move(m_pProject);
}

//-----------------------------------------------------------------------------
// 【ImportResult::HasErrors】
// 【函数功能】判断导入是否产生错误
// 【参数】无
// 【返回值】true — 存在至少一条错误
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
bool ImportResult::HasErrors() const
{
    return (m_errors.empty() == false);
}

//-----------------------------------------------------------------------------
// 【ImportResult::HasWarnings】
// 【函数功能】判断导入是否产生警告
// 【参数】无
// 【返回值】true — 存在至少一条警告
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
bool ImportResult::HasWarnings() const
{
    return (m_warnings.empty() == false);
}

//-----------------------------------------------------------------------------
// 【ImportResult::GetErrors】
// 【函数功能】获取全部错误信息
// 【参数】无
// 【返回值】错误信息列表的 const 引用
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
const std::vector<std::string>& ImportResult::GetErrors() const
{
    return m_errors;
}

//-----------------------------------------------------------------------------
// 【ImportResult::GetWarnings】
// 【函数功能】获取全部警告信息
// 【参数】无
// 【返回值】警告信息列表的 const 引用
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
const std::vector<std::string>& ImportResult::GetWarnings() const
{
    return m_warnings;
}
