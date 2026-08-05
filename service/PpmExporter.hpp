//-----------------------------------------------------------------------------
// 【PpmExporter.hpp】
// 【PPM 格式导出器类声明】
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------

#ifndef PPMEXPORTER_HPP
#define PPMEXPORTER_HPP

#include "IProjectExporter.hpp"

//-----------------------------------------------------------------------------
// 【PpmExporter 类】
// 【功能】将 Project 对象写入 PPM 格式文件
// 【接口说明】无状态——Export(project, path) 可反复调用，同一实例可处理多个文件
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
class PpmExporter : public IProjectExporter
{
  public:
    PpmExporter()                              = default;
    PpmExporter(const PpmExporter&)            = default;
    PpmExporter& operator=(const PpmExporter&) = default;
    ~PpmExporter() override                    = default;

    bool Export(const Project& project, const std::string& path) override;
};

#endif
