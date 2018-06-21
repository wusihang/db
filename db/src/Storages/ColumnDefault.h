#pragma once
#include <string>
#include <unordered_map>
#include <Parser/IAST.h>

namespace DataBase {

enum class ColumnDefaultType
{
    Default,
    Materialized,
    Alias
};

ColumnDefaultType columnDefaultTypeFromString(const std::string & str);
std::string toString(const ColumnDefaultType type);


struct ColumnDefault
{
    ColumnDefaultType type;
    std::shared_ptr<IAST> expression;
};

bool operator==(const ColumnDefault & lhs, const ColumnDefault & rhs);
using ColumnDefaults = std::unordered_map<std::string, ColumnDefault>;


}


namespace std
{
template<> struct hash<DataBase::ColumnDefaultType>
{
    size_t operator()(const DataBase::ColumnDefaultType type) const
    {
        return hash<int> {}(static_cast<int>(type));
    }
};
}

