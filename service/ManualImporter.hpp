//-----------------------------------------------------------------------------
// 【ManualImporter.hpp】
// 【手动导入器类声明，业务层测试桩】
// 【开发者及日期】 QJQ 2026.8.1
//-----------------------------------------------------------------------------

#ifndef MANUALIMPORTER_HPP
#define MANUALIMPORTER_HPP

#include "IProjectImporter.hpp"

//-----------------------------------------------------------------------------
// 【ManualImporter 类】
// 【功能】测试桩导入器，直接在代码中手动构建样例 Project，不依赖任何文件格式
// 【接口说明】Import() 返回按 ppm 样例手动构建的 Project
// 【开发者及日期】 QJQ 2026.8.1
//-----------------------------------------------------------------------------
class ManualImporter : public IProjectImporter
{
  public:
    ManualImporter()                                 = default;
    ManualImporter(const ManualImporter&)            = default;
    ManualImporter& operator=(const ManualImporter&) = default;
    ~ManualImporter() override                       = default;

    // 从代码手动构建并返回样例项目
    std::unique_ptr<Project> Import() override;
};

#endif
