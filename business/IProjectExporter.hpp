//-----------------------------------------------------------------------------
// 【IProjectExporter.hpp】
// 【项目导出器抽象接口声明，业务层基础抽象】
// 【开发者及日期】 QJQ 2026.8.1
//-----------------------------------------------------------------------------

#ifndef IPROJECTEXPORTER_HPP
#define IPROJECTEXPORTER_HPP

#include "Project.hpp"

//-----------------------------------------------------------------------------
// 【IProjectExporter 类】
// 【功能】项目导出器的抽象接口，将 Project 输出为外部格式数据
// 【接口说明】未来各格式导出器（如 PpmExporter）均由此接口派生
// 【开发者及日期】 QJQ 2026.8.1
//-----------------------------------------------------------------------------
class IProjectExporter
{
  public:
    IProjectExporter()                                   = default;
    IProjectExporter(const IProjectExporter&)            = default;
    IProjectExporter& operator=(const IProjectExporter&) = default;
    virtual ~IProjectExporter()                          = default;

    // 导出项目到外部目标，返回是否导出成功
    virtual bool Export(const Project& project) = 0;
};

#endif
