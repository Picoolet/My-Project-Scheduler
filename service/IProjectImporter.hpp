//-----------------------------------------------------------------------------
// 【IProjectImporter.hpp】
// 【项目导入器抽象接口声明，业务层基础抽象】
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------

#ifndef IPROJECTIMPORTER_HPP
#define IPROJECTIMPORTER_HPP

#include <string>

#include "ImportResult.hpp"

//-----------------------------------------------------------------------------
// 【IProjectImporter 类】
// 【功能】项目导入器的抽象接口——Import(path) 无状态，可复用于多文件
// 【接口说明】接受文件路径，返回 ImportResult（含 Project + errors + warnings）
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
class IProjectImporter
{
  public:
    IProjectImporter()                                   = default;
    IProjectImporter(const IProjectImporter&)            = default;
    IProjectImporter& operator=(const IProjectImporter&) = default;
    virtual ~IProjectImporter()                          = default;

    // 从指定路径导入项目，返回 ImportResult
    virtual ImportResult Import(const std::string& path) = 0;
};

#endif
