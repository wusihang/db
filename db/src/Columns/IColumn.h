#pragma once
#include<Ext/noncopyable.h>
#include<Poco/Exception.h>
#include<memory>

namespace ErrorCodes{
	extern const int NOT_IMPLEMENTED;
}

namespace DataBase {
class IColumn;
using ColumnPtr = std::shared_ptr<IColumn>;
class IColumn : private ext::noncopyable {

public:
    virtual std::string getName() const = 0;

    /// Creates empty column with the same type.
    virtual ColumnPtr cloneEmpty() const {
        return cloneResized(0);
    }

     virtual size_t size() const = 0;
    
    /// If size is greater, than default values are appended.
    virtual ColumnPtr cloneResized(size_t size) const {
        throw Poco::Exception("Cannot cloneResized() column " + getName(), ErrorCodes::NOT_IMPLEMENTED);
    }

};

}
