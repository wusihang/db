#pragma once
#include<tuple>
#include<memory>
#include <list>
#include<DataTypes/IDataType.h>
#include<unordered_set>
namespace IO{
	class WriteBuffer;
}

namespace DataBase {
using Names = std::vector<std::string>;
using NameSet = std::unordered_set<std::string>;
	
	
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
    NamesAndTypesList(std::initializer_list<NameAndTypePair> init) : std::list<NameAndTypePair>(init) {}
    template <typename Iterator>
    NamesAndTypesList(Iterator begin, Iterator end) : std::list<NameAndTypePair>(begin, end) {}

    Names getNames() const;

    NamesAndTypesList filter(const Names& names) const;
    NamesAndTypesList filter(const NameSet& names) const;
	
	 void writeText(IO::WriteBuffer & buf) const;
};

using NamesAndTypesListPtr = std::shared_ptr<NamesAndTypesList>;

}
