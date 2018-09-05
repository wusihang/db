#pragma once
#include<Ext/noncopyable.h>
#include<memory>
#include<Streams/Block.h>
namespace IO {

class IBlockInputStream: private ext::noncopyable {

public:
    virtual void readPrefix() {}
    virtual void readSuffix() {}
     virtual Block read() = 0;

};

using BlockInputStreamPtr = std::shared_ptr<IBlockInputStream>;

}
