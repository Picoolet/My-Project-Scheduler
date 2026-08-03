//-----------------------------------------------------------------------------
// 【IProjectExporter.hpp】
// 【项目导出器抽象接口声明，业务层基础抽象】
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------

#ifndef IPROJECTEXPORTER_HPP
#define IPROJECTEXPORTER_HPP

#include <string>

class Project;

//-----------------------------------------------------------------------------
// 【IProjectExporter 类】
// 【功能】项目导出器的抽象接口——Export(project, path) 无状态，可复用于多文件
// 【接口说明】接受 Project 和文件路径，成功返回 true
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
class IProjectExporter
{
  public:
    IProjectExporter()                                   = default;
    IProjectExporter(const IProjectExporter&)            = default;
    IProjectExporter& operator=(const IProjectExporter&) = default;
    virtual ~IProjectExporter()                          = default;

    // 将 project 导出到指定路径，成功返回 true
    virtual bool Export(const Project& project, const std::string& path) = 0;
};

#endif
