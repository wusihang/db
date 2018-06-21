#pragma once
#include<Ext/noncopyable.h>

namespace DataBase {

class IColumn : private ext::noncopyable {

public:
    virtual std::string getName() const = 0;

};

}
