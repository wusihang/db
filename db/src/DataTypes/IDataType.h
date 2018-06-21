#pragma once
#include <string>
#include<memory>
#include<Core/Types.h>
namespace DataBase {
class IColumn;

class IDataType {
public:
    virtual const std::string getName() const = 0;
    virtual const char* getFamilyName() const=0;
    virtual std::shared_ptr<IColumn> createColumn() const = 0;
    virtual std::shared_ptr<IDataType> clone() const = 0;
    virtual ~IDataType() {};
};

}
