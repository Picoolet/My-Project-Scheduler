//-----------------------------------------------------------------------------
// 【PpmImporter.cpp】
// 【PPM 格式导入器类实现】
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------

#include "PpmImporter.hpp"

#include <cctype>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "DependencyType.hpp"
#include "Id.hpp"
#include "ImportResult.hpp"
#include "Project.hpp"

namespace
{

//-----------------------------------------------------------------------------
// 【Trim】
// 【函数功能】去除字符串首尾空格
// 【参数】s — 输入参数，原始字符串
// 【返回值】去除首尾空格后的字符串
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
std::string Trim(const std::string& s)
{
    size_t start = 0;

    while ((start < s.size())
           && (std::isspace(static_cast<unsigned char>(s[start])) != 0))
    {
        ++start;
    }

    size_t end = s.size();

    while ((end > start)
           && (std::isspace(static_cast<unsigned char>(s[end - 1])) != 0))
    {
        --end;
    }

    return s.substr(start, (end - start));
}

//-----------------------------------------------------------------------------
// 【Split】
// 【函数功能】按空格拆分字符串
// 【参数】s — 输入参数，待拆分字符串
// 【返回值】拆分后的词列表
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
std::vector<std::string> Split(const std::string& s)
{
    std::vector<std::string> result;
    std::istringstream       stream(s);
    std::string              token;

    while ((stream >> token).operator bool())
    {
        result.push_back(token);
    }

    return result;
}

//-----------------------------------------------------------------------------
// 【ParseDependencyType】
// 【函数功能】解析依赖类型字符串
// 【参数】s — 输入参数，依赖类型字符串（"FS"/"SS"/"FF"/"SF"）
// 【返回值】对应的 DependencyType；非法输入返回 FS
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
DependencyType ParseDependencyType(const std::string& s)
{
    if (s == "SS")
    {
        return DependencyType::SS;
    }

    if (s == "FF")
    {
        return DependencyType::FF;
    }

    if (s == "SF")
    {
        return DependencyType::SF;
    }

    return DependencyType::FS;
}

} // anonymous namespace

//-----------------------------------------------------------------------------
// 【PpmImporter::Import】
// 【函数功能】读取 PPM 文件，逐行解析并构建 Project
// 【参数】path — 输入参数，PPM 文件路径
// 【返回值】ImportResult（含 Project + errors + warnings）
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
ImportResult PpmImporter::Import(const std::string& path)
{
    std::ifstream file(path);

    if (file.is_open() == false)
    {
        return ImportResult(nullptr, {"无法打开文件: " + path}, {});
    }

    std::unique_ptr<Project> project = std::make_unique<Project>();
    std::vector<std::string> errors;
    std::vector<std::string> warnings;

    std::string line;
    int         lineNumber = 0;

    while (std::getline(file, line).operator bool())
    {
        ++lineNumber;

        // 去除首尾空格
        line = Trim(line);

        // 跳过空行和注释行
        if ((line.empty() == true) || (line[0] == '#'))
        {
            continue;
        }

        std::vector<std::string> tokens = Split(line);

        if (tokens.empty() == true)
        {
            continue;
        }

        char prefix = tokens[0][0];

        switch (prefix)
        {
        case 'P':
        {
            // 项目名称
            if (tokens.size() >= 2)
            {
                project->SetName(tokens[1]);
            }

            break;
        }

        case 'T':
        {
            // T ID Name Duration
            if (tokens.size() < 4)
            {
                errors.push_back("第 " + std::to_string(lineNumber)
                                 + " 行格式错误：字段数不足");
                break;
            }

            int taskId   = std::stoi(tokens[1]);
            int duration = std::stoi(tokens[3]);

            if (project->AddTask(TaskId(taskId), tokens[2], duration)
                == TaskId::Invalid())
            {
                warnings.push_back("第 " + std::to_string(lineNumber)
                                   + " 行：任务 ID " + tokens[1]
                                   + " 重复，已跳过");
            }

            break;
        }

        case 'M':
        {
            // M ID Name 0 （里程碑）
            if (tokens.size() < 3)
            {
                errors.push_back("第 " + std::to_string(lineNumber)
                                 + " 行格式错误：字段数不足");
                break;
            }

            int taskId = std::stoi(tokens[1]);

            if (project->AddTask(TaskId(taskId), tokens[2], 0)
                == TaskId::Invalid())
            {
                warnings.push_back("第 " + std::to_string(lineNumber)
                                   + " 行：里程碑 ID " + tokens[1]
                                   + " 重复，已跳过");
            }

            break;
        }

        case 'R':
        {
            // R ID Name UnitCost
            if (tokens.size() < 4)
            {
                errors.push_back("第 " + std::to_string(lineNumber)
                                 + " 行格式错误：字段数不足");
                break;
            }

            int    resId = std::stoi(tokens[1]);
            double cost  = std::stod(tokens[3]);

            if (project->AddResource(ResourceId(resId), tokens[2], cost)
                == ResourceId::Invalid())
            {
                warnings.push_back("第 " + std::to_string(lineNumber)
                                   + " 行：资源 ID " + tokens[1]
                                   + " 重复，已跳过");
            }

            break;
        }

        case 'D':
        {
            // D PredID SuccID Type Lag
            if (tokens.size() < 5)
            {
                errors.push_back("第 " + std::to_string(lineNumber)
                                 + " 行格式错误：字段数不足");
                break;
            }

            int            predId = std::stoi(tokens[1]);
            int            succId = std::stoi(tokens[2]);
            DependencyType type   = ParseDependencyType(tokens[3]);
            int            lag    = std::stoi(tokens[4]);

            project->AddDependency(TaskId(predId), TaskId(succId), type, lag);
            break;
        }

        case 'A':
        {
            // A TaskID ResourceID Quantity
            if (tokens.size() < 4)
            {
                errors.push_back("第 " + std::to_string(lineNumber)
                                 + " 行格式错误：字段数不足");
                break;
            }

            int taskId = std::stoi(tokens[1]);
            int resId  = std::stoi(tokens[2]);
            int qty    = std::stoi(tokens[3]);

            project->AssignResource(TaskId(taskId), ResourceId(resId), qty);
            break;
        }

        default:
        {
            errors.push_back("第 " + std::to_string(lineNumber)
                             + " 行：非法行前缀 '" + std::string(1, prefix)
                             + "'");
            break;
        }
        }
    }

    return ImportResult(std::move(project), errors, warnings);
}
