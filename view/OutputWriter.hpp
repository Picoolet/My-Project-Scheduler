//-----------------------------------------------------------------------------
// 【OutputWriter.hpp】
// 【输出写入器类声明，统一封装界面输出格式】
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------

#ifndef OUTPUTWRITER_HPP
#define OUTPUTWRITER_HPP

#include <ostream>
#include <string>

//-----------------------------------------------------------------------------
// 【OutputWriter 类】
// 【功能】封装向指定流输出的各类格式化方法（成功/失败/警告/标题等）
// 【接口说明】绑定 std::ostream&，全部输出经 m_out，不使用 cout/cerr
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
class OutputWriter
{
  public:
    explicit OutputWriter(std::ostream& out);

    OutputWriter(const OutputWriter&)            = default;
    OutputWriter& operator=(const OutputWriter&) = default;
    ~OutputWriter()                              = default;

    void          Print(const std::string& text) const;
    void          PrintLine(const std::string& text) const;
    void          PrintSuccess(const std::string& message) const;
    void          PrintError(const std::string& message) const;
    void          PrintWarning(const std::string& message) const;
    void          PrintHeader(const std::string& title) const;
    void          PrintSeparator() const;
    std::ostream& GetStream() const;

  private:
    std::ostream& m_out;
};

#endif
