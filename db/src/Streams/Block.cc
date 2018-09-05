#include<Streams/Block.h>
#include<Poco/Exception.h>
#include<IO/WriteBufferHelper.h>

namespace ErrorCodes {
extern const int POSITION_OUT_OF_BOUND;
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

