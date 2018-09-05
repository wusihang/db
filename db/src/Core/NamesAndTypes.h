#pragma once
#include<tuple>
#include<memory>
#include <list>
#include<DataTypes/IDataType.h>
namespace DataBase {

struct NameAndTypePair
{
    std::string name;
    std::shared_ptr<IDataType> type;

    NameAndTypePair() {}
    NameAndTypePair(const  std::string & name_, const std::shared_ptr<IDataType>  & type_) : name(name_), type(type_) {}

    bool operator<(const NameAndTypePair & rhs) const
    {
        return std::forward_as_tuple(name, type->getName()) < std::forward_as_tuple(rhs.name, rhs.type->getName());
    }

    bool operator==(const NameAndTypePair & rhs) const
    {
        return name == rhs.name && type->getName() == rhs.type->getName();
    }
};


class NamesAndTypesList: public std::list<NameAndTypePair> {
public:
    NamesAndTypesList() {}
};

using NamesAndTypesListPtr = std::shared_ptr<NamesAndTypesList>;

}
