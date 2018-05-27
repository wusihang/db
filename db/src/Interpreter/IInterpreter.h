#pragma once
#include<Streams/BlockIO.h>

namespace DataBase {

class IInterpreter {
public:
    virtual IO::BlockIO execute() = 0;
    virtual ~IInterpreter() {}

};

}
