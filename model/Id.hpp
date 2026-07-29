//-----------------------------------------------------------------------------
// 【Id.hpp】
// 【强类型 ID 模板，提供编译期类型安全的标识符】
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------

#ifndef ID_HPP
#define ID_HPP

#include <functional>

//-----------------------------------------------------------------------------
// 【Id 类模板】
// 【功能】通过标签类型 Tag 在编译期区分不同实体的 ID，防止误传
// 【接口说明】提供值访问、相等比较、哈希支持及无效值哨兵
// 【开发者及日期】 QJQ 2026.7.29
//-----------------------------------------------------------------------------
template <typename Tag> class Id
{
  public:
    // 默认构造为无效值
    Id() : m_value(0U)
    {
    }

    // 从 unsigned int 显式构造
    explicit Id(unsigned int value) : m_value(value)
    {
    }

    Id(const Id&)            = default;
    Id& operator=(const Id&) = default;
    ~Id()                    = default;

    // 获取原始数值
    unsigned int Value() const
    {
        return m_value;
    }

    // 相等比较
    bool operator==(const Id& other) const
    {
        return (m_value == other.m_value);
    }

    // 不等比较
    bool operator!=(const Id& other) const
    {
        return (m_value != other.m_value);
    }

    // 返回无效值哨兵（Value 为 0）
    static Id Invalid()
    {
        return Id(0U);
    }

  private:
    unsigned int m_value; // 内部存储的 ID 数值
};

// std::hash 特化，支持用作 unordered_map 的 key
namespace std
{
template <typename Tag> struct hash<Id<Tag>>
{
    size_t operator()(const Id<Tag>& id) const
    {
        return hash<unsigned int>()(id.Value());
    }
};
} // namespace std

// 具体 ID 类型别名
using TaskId     = Id<struct TaskIdTag>;
using ResourceId = Id<struct ResourceIdTag>;

#endif
