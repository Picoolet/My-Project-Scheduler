//-----------------------------------------------------------------------------
// 【ValidationResult.hpp】
// 【验证结果类声明，封装项目合理性校验的完整结果】
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------

#ifndef VALIDATIONRESULT_HPP
#define VALIDATIONRESULT_HPP

#include <string>
#include <vector>

//-----------------------------------------------------------------------------
// 【ValidationResult 类】
// 【功能】封装验证结果，收集所有错误信息后一次性返回
// 【接口说明】默认构造 = 有效；IsValid() 判定结果；GetErrors() 返回错误列表
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
class ValidationResult
{
  public:
    // 默认构造函数：空错误列表 = 有效
    ValidationResult();

    // 从已有错误列表构造
    explicit ValidationResult(const std::vector<std::string>& errors);

    ValidationResult(const ValidationResult&)            = default;
    ValidationResult& operator=(const ValidationResult&) = default;
    ValidationResult(ValidationResult&&)                 = default;
    ValidationResult& operator=(ValidationResult&&)      = default;
    ~ValidationResult()                                  = default;

    // 追加一条错误信息（供 ProjectValidator 内部使用）
    void AddError(const std::string& error);

    // 判断是否通过验证（错误列表为空）
    bool IsValid() const;

    // 获取全部错误信息
    const std::vector<std::string>& GetErrors() const;

  private:
    std::vector<std::string> m_errors; // 错误信息列表
};

#endif
