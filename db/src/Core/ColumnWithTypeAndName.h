#pragma once
#include<DataTypes/IDataType.h>

namespace DataBase {

class ColumnWithTypeAndName {
public:
    ColumnPtr column;
    DataTypePtr type;
    String name;

    ColumnWithTypeAndName() {}
    ColumnWithTypeAndName(const ColumnPtr & column_, const DataTypePtr & type_, const String name_)
        : column(column_), type(type_), name(name_) {}

    /// Uses type->createColumn() to create column
    ColumnWithTypeAndName(const DataTypePtr & type_, const String name_)
        : column(type_->createColumn()), type(type_), name(name_) {}

    ColumnWithTypeAndName cloneEmpty() const;
    bool operator==(const ColumnWithTypeAndName & other) const;
    String prettyPrint() const;
};

}
