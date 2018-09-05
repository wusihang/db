#include<Storages/ITableDeclaration.h>

IO::Block Storage::ITableDeclaration::getSampleBlock() const
{
    IO::Block res;
    for (const auto & col : getColumnsListRange())
    {
        res.insert( { col.type->createColumn(), col.type, col.name });
    }
    return res;
}


const DataBase::NamesAndTypesList Storage::ITableDeclaration::getColumnsListRange() const
{
    return getColumnsListImpl();
}
