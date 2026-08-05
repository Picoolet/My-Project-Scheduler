//-----------------------------------------------------------------------------
// 【CommandParser.hpp】
// 【命令解析器类声明，将一行用户输入解析为 ParsedCommand】
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------

#ifndef COMMANDPARSER_HPP
#define COMMANDPARSER_HPP

#include <string>
#include <vector>

#include "CommandRegistry.hpp"

//-----------------------------------------------------------------------------
// 【CommandParser 类】
// 【功能】将一行用户输入分词并匹配注册表，产生 ParsedCommand
// 【接口说明】绑定 CommandRegistry 引用；支持双引号内多词 token
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
class CommandParser
{
  public:
    explicit CommandParser(const CommandRegistry& registry);

    CommandParser(const CommandParser&)            = default;
    CommandParser& operator=(const CommandParser&) = default;
    ~CommandParser()                               = default;

    // 解析一行用户输入，返回 ParsedCommand
    ParsedCommand ParseLine(const std::string& line) const;

  private:
    // 分词：空格分隔，双引号内为一个 token（不含引号本身）
    std::vector<std::string> Tokenize(const std::string& line) const;

    // 判断某词是否为已知 target 关键词
    bool IsTargetKeyword(const std::string& token) const;

    const CommandRegistry& m_registry;
};

#endif
