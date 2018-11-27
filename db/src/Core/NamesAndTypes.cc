#include<Core/NamesAndTypes.h>
#include<IO/WriteBufferHelper.h>

std::vector< std::string > DataBase::NamesAndTypesList::getNames() const
{
    std::vector< std::string > res;
    res.reserve(size());
    for (const NameAndTypePair & column : *this)
    {
        res.push_back(column.name);
    }
    return res;
}

DataBase::NamesAndTypesList DataBase::NamesAndTypesList::filter(const std::vector< std::string >& names) const
{
    return filter(NameSet(names.begin(),names.end()));
}

DataBase::NamesAndTypesList DataBase::NamesAndTypesList::filter(const NameSet& names) const
{
    NamesAndTypesList res;
    for (const NameAndTypePair & column : *this)
    {
        if (names.count(column.name))
            res.push_back(column);
    }
    return res;
}


void DataBase::NamesAndTypesList::writeText(IO::WriteBuffer& buf) const
{
    IO::writeString("columns format version: 1\n", buf);
    IO::writeText(size(), buf);
    IO::writeString(" columns:\n", buf);
    for (const auto & it : *this)
    {
        IO::writeBackQuotedString(it.name, buf);
        IO::writeChar(' ', buf);
        IO::writeString(it.type->getName(), buf);
        IO:: writeChar('\n', buf);
    }
}
