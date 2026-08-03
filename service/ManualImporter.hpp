//-----------------------------------------------------------------------------
// 【ManualImporter.hpp】
// 【手动导入器类声明，业务层测试桩】
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------

#ifndef MANUALIMPORTER_HPP
#define MANUALIMPORTER_HPP

#include "IProjectImporter.hpp"

//-----------------------------------------------------------------------------
// 【ManualImporter 类】
// 【功能】测试桩导入器，直接在代码中手动构建样例 Project
// 【接口说明】Import(path) 忽略 path 参数，硬编码返回 ProjectDemo
// 【开发者及日期】 QJQ 2026.8.2
//-----------------------------------------------------------------------------
class ManualImporter : public IProjectImporter
{
  public:
    ManualImporter()                                 = default;
    ManualImporter(const ManualImporter&)            = default;
    ManualImporter& operator=(const ManualImporter&) = default;
    ~ManualImporter() override                       = default;

    // 从代码手动构建并返回样例项目（忽略 path 参数）
    ImportResult Import(const std::string& path) override;
};

#endif
