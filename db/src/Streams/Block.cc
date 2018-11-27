#include<Streams/Block.h>
#include<Poco/Exception.h>
#include<IO/WriteBufferHelper.h>
#include<IO/Operators.h>

namespace ErrorCodes {
extern const int POSITION_OUT_OF_BOUND;
extern const int NOT_FOUND_COLUMN_IN_BLOCK;
}

IO::Block IO::Block::cloneEmpty() const
{
    Block res;
    for (const auto & elem : data)
    {
        res.insert(elem.cloneEmpty());
    }
    return res;
}

void IO::Block::insert(size_t position, const DataBase::ColumnWithTypeAndName& elem)
{
    if (position > data.size())
    {
        throw Poco::Exception("Position out of bound in Block::insert(), max position = "
                              + IO::toString(data.size()), ErrorCodes::POSITION_OUT_OF_BOUND);
    }
    for (auto& name_pos : index_by_name)
    {
        if (name_pos.second >= position)
        {
            ++name_pos.second;
        }
    }
    index_by_name[elem.name] = position;
    data.emplace(data.begin() + position, elem);
}


void IO::Block::insert(const DataBase::ColumnWithTypeAndName& elem)
{
    index_by_name[elem.name] = data.size();
    data.emplace_back(elem);
}

void IO::Block::insert(const DataBase::ColumnWithTypeAndName&& elem)
{
    index_by_name[elem.name] = data.size();
    data.emplace_back(std::move(elem));
}


void IO::Block::clear()
{
    data.clear();
    index_by_name.clear();
}


size_t IO::Block::rows() const
{
    for (const auto & elem : data)
        if (elem.column)
            return elem.column->size();
    return 0;
}


DataBase::ColumnWithTypeAndName& IO::Block::getByName(const std::string& name)
{
    auto it = index_by_name.find(name);
    if (index_by_name.end() == it)
        throw Poco::Exception("Not found column " + name + " in block. There are only columns: " + dumpNames()
                              , ErrorCodes::NOT_FOUND_COLUMN_IN_BLOCK);

    return data[it->second];
}

const DataBase::ColumnWithTypeAndName& IO::Block::getByName(const std::string& name) const
{
    auto it = index_by_name.find(name);
    if (index_by_name.end() == it)
        throw Poco::Exception("Not found column " + name + " in block. There are only columns: " + dumpNames()
                              , ErrorCodes::NOT_FOUND_COLUMN_IN_BLOCK);

    return data[it->second];
}


std::string IO::Block::dumpNames() const
{
    IO::WriteBufferFromOwnString out;
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        if (it != data.begin())
            out << ", ";
        out << it->name;
    }
    return out.str();
}

DataBase::ColumnWithTypeAndName& IO::Block::safeGetByPosition(size_t position)
{
    if (data.empty())
        throw Poco::Exception("Block is empty", ErrorCodes::POSITION_OUT_OF_BOUND);

    if (position >= data.size())
        throw Poco::Exception("Position " + toString(position)
                              + " is out of bound in Block::safeGetByPosition(), max position = "
                              + toString(data.size() - 1)
                              + ", there are columns: " + dumpNames(), ErrorCodes::POSITION_OUT_OF_BOUND);

    return data[position];
}

const DataBase::ColumnWithTypeAndName& IO::Block::safeGetByPosition(size_t position) const
{
    if (data.empty())
        throw Poco::Exception("Block is empty", ErrorCodes::POSITION_OUT_OF_BOUND);
    if (position >= data.size())
        throw Poco::Exception("Position " + toString(position)
                              + " is out of bound in Block::safeGetByPosition(), max position = "
                              + toString(data.size() - 1)
                              + ", there are columns: " + dumpNames(), ErrorCodes::POSITION_OUT_OF_BOUND);
    return data[position];
}


DataBase::NamesAndTypesList IO::Block::getColumnsList() const
{
    DataBase::NamesAndTypesList res;
    for (const auto & elem : data)
        res.push_back(DataBase::NameAndTypePair(elem.name, elem.type));
    return res;
}

