#pragma once
#include <string>
#include<memory>
#include<vector>
#include<Core/Types.h>
#include<Columns/IColumn.h>
namespace IO {
class ReadBuffer;
class WriteBuffer;
}

namespace DataBase {
class IDataType;
using ColumnPtr = std::shared_ptr<IColumn>;
using DataTypePtr = std::shared_ptr<IDataType>;
using DataTypes = std::vector<DataTypePtr>;
class IDataType {
public:
    virtual const std::string getName() const = 0;
    virtual const char* getFamilyName() const=0;
    virtual std::shared_ptr<IColumn> createColumn() const = 0;
    virtual std::shared_ptr<IDataType> clone() const = 0;
    virtual void deserializeTextQuoted(IColumn & column, IO::ReadBuffer & istr) const = 0;
	
	 virtual void serializeBinaryBulk(const IColumn & column, IO::WriteBuffer & ostr, size_t offset, size_t limit) const = 0;
	 virtual void serializeBinary(const Field & field, IO::WriteBuffer & ostr) const = 0;
	 virtual void serializeBinary(const IColumn & column, size_t row_num, IO::WriteBuffer & ostr) const = 0;
	 
	 
    virtual ~IDataType() {};
};

}
