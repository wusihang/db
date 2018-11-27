#pragma once
#include<DataTypes/DataTypeNumberBase.h>

namespace IO{
	class ReadBuffer;
}

namespace DataBase {

class DataTypeDate:public DataTypeNumberBase<UInt16> {
public:
    const std::string getName() const override {
        return "Date";
    }
    const char * getFamilyName() const override {
        return "Date";
    }
    void deserializeTextQuoted(IColumn& column, IO::ReadBuffer& istr) const override;
	 DataTypePtr clone() const override { return std::make_shared<DataTypeDate>(); }
};

}
