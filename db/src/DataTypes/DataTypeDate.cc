#include<DataTypes/DataTypeDate.h>
#include<IO/ReadHelper.h>
#include<Columns/ColumnNumber.h>
#include<DataTypes/DataTypeFactory.h>
namespace DataBase {

void DataTypeDate::deserializeTextQuoted(IColumn& column, IO::ReadBuffer& istr) const
{
    DayNum_t x;
    IO::assertChar('\'', istr);
    IO::readDateText(x, istr);
    IO::assertChar('\'', istr);
    static_cast<ColumnUInt16 &>(column).getData().push_back(x);    /// It's important to do this at the end - for exception safety.
}


void registerDataTypeDate(DataTypeFactory & factory)
{
    factory.registerSimpleDataType("Date", [] { return DataTypePtr(std::make_shared<DataTypeDate>()); }, DataTypeFactory::CaseInsensitive);
}
}
