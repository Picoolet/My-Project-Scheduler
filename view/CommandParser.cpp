//-----------------------------------------------------------------------------
// 【CommandParser.cpp】
// 【命令解析器类实现】
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------

#include "CommandParser.hpp"

#include <cctype>

//-----------------------------------------------------------------------------
// 【CommandParser::CommandParser】
// 【函数功能】构造函数，绑定命令注册表引用
// 【参数】registry — 输入参数，命令注册表
// 【返回值】无
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
CommandParser::CommandParser(const CommandRegistry& registry)
    : m_registry(registry)
{
}

//-----------------------------------------------------------------------------
// 【CommandParser::Tokenize】
// 【函数功能】将一行输入按空格分词，双引号内整体为一个 token
// 【参数】line — 输入参数，用户输入行
// 【返回值】token 列表（双引号内不含引号本身）
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
std::vector<std::string> CommandParser::Tokenize(const std::string& line) const
{
    std::vector<std::string> tokens;
    size_t                   i = 0;

    while (i < line.size())
    {
        // 跳过空白
        if (std::isspace(static_cast<unsigned char>(line[i])) != 0)
        {
            ++i;
            continue;
        }

        // 双引号 token
        if (line[i] == '"')
        {
            ++i;
            std::string token;

            while ((i < line.size()) && (line[i] != '"'))
            {
                token += line[i];
                ++i;
            }

            // 跳过结尾引号（若存在）
            if (i < line.size())
            {
                ++i;
            }

            tokens.push_back(token);
            continue;
        }

        // 普通 token：收集到下一个空白
        std::string token;

        while ((i < line.size())
               && (std::isspace(static_cast<unsigned char>(line[i])) == 0))
        {
            token += line[i];
            ++i;
        }

        tokens.push_back(token);
    }

    return tokens;
}

//-----------------------------------------------------------------------------
// 【CommandParser::IsTargetKeyword】
// 【函数功能】判断某词是否为已知 target 关键词
// 【参数】token — 输入参数，待判断的词（已转小写）
// 【返回值】true — 是 target 关键词
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
bool CommandParser::IsTargetKeyword(const std::string& token) const
{
    if ((token == "task") || (token == "tasks") || (token == "dependency")
        || (token == "dependencies") || (token == "dep")
        || (token == "resource") || (token == "resources") || (token == "res"))
    {
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
// 【CommandParser::ParseLine】
// 【函数功能】解析一行用户输入，匹配注册表并校验参数个数
// 【参数】line — 输入参数，用户输入行
// 【返回值】ParsedCommand（isValid 标志 + 错误信息）
// 【开发者及日期】QJQ 2026.8.5
// 【更改记录】 无
//-----------------------------------------------------------------------------
ParsedCommand CommandParser::ParseLine(const std::string& line) const
{
    ParsedCommand result;
    result.isValid = false;

    std::vector<std::string> tokens = Tokenize(line);

    if (tokens.empty() == true)
    {
        return result; // 空行/纯空格：isValid=false，无错误信息
    }

    // 动作：转小写
    result.action = tokens[0];

    for (char& ch : result.action)
    {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }

    // 目标识别：若 tokens[1] 是已知 target 关键词则作为 target
    if ((tokens.size() >= 2) && (IsTargetKeyword(tokens[1]) == true))
    {
        result.target = tokens[1];

        for (char& ch : result.target)
        {
            ch = static_cast<char>(
                std::tolower(static_cast<unsigned char>(ch)));
        }

        result.args.assign(tokens.begin() + 2, tokens.end());
    }
    else
    {
        result.target = "";
        result.args.assign(tokens.begin() + 1, tokens.end());
    }

    // 查找命令定义
    const CommandDef* def = m_registry.FindCommand(result.action,
                                                   result.target);

    if (def == nullptr)
    {
        result.errorMsg = "未知命令: " + result.action + " " + result.target;
        return result;
    }

    // 参数个数校验
    int argCount = static_cast<int>(result.args.size());

    if ((argCount < def->minArgs)
        || ((def->maxArgs != -1) && (argCount > def->maxArgs)))
    {
        result.errorMsg = "参数个数错误：需要 " + def->signature;
        return result;
    }

    result.isValid = true;
    return result;
}
