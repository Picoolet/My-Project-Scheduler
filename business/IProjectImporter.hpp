//-----------------------------------------------------------------------------
// 【IProjectImporter.hpp】
// 【项目导入器抽象接口声明，业务层基础抽象】
// 【开发者及日期】 QJQ 2026.8.1
//-----------------------------------------------------------------------------

#ifndef IPROJECTIMPORTER_HPP
#define IPROJECTIMPORTER_HPP

#include <memory>

#include "Project.hpp"

//-----------------------------------------------------------------------------
// 【IProjectImporter 类】
// 【功能】项目导入器的抽象接口，将外部格式数据转换为 Project 对象
// 【接口说明】未来各格式导入器（如 PpmImporter）均由此接口派生
// 【开发者及日期】 QJQ 2026.8.1
//-----------------------------------------------------------------------------
class IProjectImporter
{
  public:
    IProjectImporter()                                   = default;
    IProjectImporter(const IProjectImporter&)            = default;
    IProjectImporter& operator=(const IProjectImporter&) = default;
    virtual ~IProjectImporter()                          = default;

    // 从外部数据导入项目，返回新构建的 Project 对象
    virtual std::unique_ptr<Project> Import() = 0;
};

#endif
