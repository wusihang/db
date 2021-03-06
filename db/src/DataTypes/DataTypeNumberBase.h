#pragma once
#include <DataTypes/IDataType.h>
#include<Columns/IColumn.h>

namespace DataBase {

template<typename T>
class DataTypeNumberBase: public IDataType {
public:
    using FieldType = T;
    const std::string getName() const override {
        return TypeName<T>::get();
    }
    const char * getFamilyName() const override {
        return TypeName<T>::get();
    }

    std::shared_ptr<IColumn> createColumn() const override;
	
    void deserializeTextQuoted(IColumn& column, IO::ReadBuffer& istr) const override;
	
	void serializeBinary(const Field & field, IO::WriteBuffer & ostr) const override;
	
	void serializeBinary(const IColumn & column, size_t row_num, IO::WriteBuffer & ostr) const override;
	
	void serializeBinaryBulk(const IColumn & column, IO::WriteBuffer & ostr, size_t offset, size_t limit) const override;
};

template class DataTypeNumberBase<UInt8>;
template class DataTypeNumberBase<UInt16>;
template class DataTypeNumberBase<UInt32>;
template class DataTypeNumberBase<UInt64>;
template class DataTypeNumberBase<Int8>;
template class DataTypeNumberBase<Int16>;
template class DataTypeNumberBase<Int32>;
template class DataTypeNumberBase<Int64>;
template class DataTypeNumberBase<Float32>;
template class DataTypeNumberBase<Float64>;

}
