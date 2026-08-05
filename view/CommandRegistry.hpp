//-----------------------------------------------------------------------------
// 【CommandRegistry.hpp】
// 【命令注册表类声明，管理命令定义与查找】
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------

#ifndef COMMANDREGISTRY_HPP
#define COMMANDREGISTRY_HPP

#include <string>
#include <unordered_map>
#include <vector>

//-----------------------------------------------------------------------------
// 【CommandDef 结构体】
// 【功能】描述一条命令的定义（动作、目标、参数签名、所需参数个数等）
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
struct CommandDef
{
    std::string action; // 动作：add, remove, list, show, modify, import, ...
    std::string target; // 目标：task, dependency, resource, ...，""（无目标）
    std::string signature;   // 参数签名："<name> <duration>"（用于 help 显示）
    std::string description; // 功能描述（用于 help 显示）
    int         minArgs;     // 最少参数个数
    int         maxArgs;     // 最多参数个数（-1 表示不限）
    bool        requiresProject; // 是否需要已加载项目
};

//-----------------------------------------------------------------------------
// 【ParsedCommand 结构体】
// 【功能】命令解析结果
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
struct ParsedCommand
{
    std::string              action;
    std::string              target;
    std::vector<std::string> args;
    bool                     isValid;
    std::string              errorMsg;
};

//-----------------------------------------------------------------------------
// 【CommandRegistry 类】
// 【功能】维护命令定义注册表，支持按 "action:target" 查找与生成帮助文本
// 【接口说明】同一 action/target 组合可重复注册（别名覆盖，后注册者优先）
// 【开发者及日期】 QJQ 2026.8.5
//-----------------------------------------------------------------------------
class CommandRegistry
{
  public:
    CommandRegistry()  = default;
    ~CommandRegistry() = default;

    // 注册一条命令定义；同一 key 已存在则覆盖（别名后注册优先）
    void RegisterCommand(const CommandDef& def);

    // 按 "action:target" 精确查找，未找到返回 nullptr
    const CommandDef* FindCommand(const std::string& action,
                                  const std::string& target) const;

    // 生成帮助文本：按注册顺序输出，跳过 description 为空的别名条目
    std::string GetHelpText() const;

    // 获取全部命令定义
    const std::vector<CommandDef>& GetCommands() const;

  private:
    std::vector<CommandDef> m_commandDefs;
    // key = action + ":" + target → m_commandDefs 下标
    // （存下标而非指针，避免 vector 扩容导致指针失效）
    std::unordered_map<std::string, int> m_commandIndex;
};

#endif
