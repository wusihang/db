#pragma once
#include <Ext/noncopyable.h>
#include<memory>
namespace IO {

class Block;

class IBlockOutputStream:private ext::noncopyable {
public:
    virtual void write(const Block & block) = 0;
    virtual void writePrefix() {}
    virtual void writeSuffix() {}
};

using BlockOutputStreamPtr = std::shared_ptr<IBlockOutputStream>;

}
