#pragma once
#include<Streams/Block.h>
#include<Core/NamesAndTypes.h>
namespace Storage {

class ITableDeclaration {
public:
    IO::Block getSampleBlock() const;
	const DataBase::NamesAndTypesList & getColumnsListNonMaterialized() const { return getColumnsListImpl(); }
private:
    const DataBase::NamesAndTypesList getColumnsListRange() const;
    virtual const DataBase::NamesAndTypesList & getColumnsListImpl() const = 0;
};

}
