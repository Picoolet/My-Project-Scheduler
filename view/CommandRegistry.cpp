//-----------------------------------------------------------------------------
// 【CommandRegistry.cpp】
// 【命令注册表类实现】
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------

#include "CommandRegistry.hpp"

//-----------------------------------------------------------------------------
// 【CommandRegistry::RegisterCommand】
// 【函数功能】注册一条命令定义；同一 key 存在则覆盖（别名后注册优先）
// 【参数】def — 输入参数，命令定义
// 【返回值】无
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
void CommandRegistry::RegisterCommand(const CommandDef& def)
{
    m_commandDefs.push_back(def);
    m_commandIndex[def.action + ":" + def.target] = static_cast<int>(
        m_commandDefs.size() - 1);
}

//-----------------------------------------------------------------------------
// 【CommandRegistry::FindCommand】
// 【函数功能】按 "action:target" 精确查找命令定义
// 【参数】action — 输入参数，命令动作
//        target — 输入参数，命令目标
// 【返回值】找到返回 CommandDef 指针，未找到返回 nullptr
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
const CommandDef* CommandRegistry::FindCommand(const std::string& action,
                                               const std::string& target) const
{
    auto iter = m_commandIndex.find(action + ":" + target);

    if (iter != m_commandIndex.end())
    {
        return &m_commandDefs[iter->second];
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
// 【CommandRegistry::GetHelpText】
// 【函数功能】生成帮助文本，按注册顺序输出，跳过别名条目
// 【参数】无
// 【返回值】帮助文本字符串
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
std::string CommandRegistry::GetHelpText() const
{
    std::string text = "可用命令：\n";

    for (const CommandDef& def : m_commandDefs)
    {
        // 跳过纯别名条目（description 为空）
        if (def.description.empty() == true)
        {
            continue;
        }

        text += "  ";
        text += def.action;
        text += " ";

        if (def.target.empty() == false)
        {
            text += def.target;
            text += " ";
        }

        text += def.signature;
        text += "  — ";
        text += def.description;
        text += "\n";
    }

    return text;
}

//-----------------------------------------------------------------------------
// 【CommandRegistry::GetCommands】
// 【函数功能】获取全部命令定义（供遍历检查）
// 【参数】无
// 【返回值】命令定义列表的 const 引用
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
const std::vector<CommandDef>& CommandRegistry::GetCommands() const
{
    return m_commandDefs;
}
