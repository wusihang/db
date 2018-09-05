#include<Core/ColumnWithTypeAndName.h>
#include<Columns/IColumn.h>
#include<IO/WriteBufferFromString.h>
#include<IO/WriteBufferHelper.h>

DataBase::ColumnWithTypeAndName DataBase::ColumnWithTypeAndName::cloneEmpty() const
{
    ColumnWithTypeAndName res;
    res.name = name;
    res.type = type->clone();
    if (column)
    {
        res.column = column->cloneEmpty();
    }
    return res;
}


bool DataBase::ColumnWithTypeAndName::operator==(const DataBase::ColumnWithTypeAndName& other) const
{
    return name == other.name
           && ((!type && !other.type) || (type && other.type && type->getName() == other.type->getName()))
           && ((!column && !other.column) || (column && other.column && column->getName() == other.column->getName()));
}


DataBase::String DataBase::ColumnWithTypeAndName::prettyPrint() const
{
    IO::WriteBufferFromOwnString out;
    IO::writeString(name, out);
    if (type)
    {
        writeChar(' ', out);
        IO::writeString(type->getName(), out);
    }
    if (column)
    {
        writeChar(' ', out);
        IO::writeString(column->getName(), out);
    }
    return out.str();
}
