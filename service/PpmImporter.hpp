//-----------------------------------------------------------------------------
// 【PpmImporter.hpp】
// 【PPM 格式导入器类声明】
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------

#ifndef PPMIMPORTER_HPP
#define PPMIMPORTER_HPP

#include "IProjectImporter.hpp"

//-----------------------------------------------------------------------------
// 【PpmImporter 类】
// 【功能】读取 PPM 格式文件，解析为 Project 对象
// 【接口说明】无状态——Import(path) 可反复调用，同一实例可处理多个文件
// 【开发者及日期】QJQ 2026.8.2
// 【更改记录】 无
//-----------------------------------------------------------------------------
class PpmImporter : public IProjectImporter
{
  public:
    PpmImporter()                              = default;
    PpmImporter(const PpmImporter&)            = default;
    PpmImporter& operator=(const PpmImporter&) = default;
    ~PpmImporter() override                    = default;

    ImportResult Import(const std::string& path) override;
};

#endif
