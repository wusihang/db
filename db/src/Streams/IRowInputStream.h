#pragma once
#include <Ext/noncopyable.h>
#include<memory>
namespace IO {
class Block;
class IRowInputStream :private ext::noncopyable {
public:
    /** Read next row and append it to block.
      * If no more rows - return false.
      */
    virtual bool read(IO::Block & block) = 0;

    virtual void readPrefix() {};                /// delimiter before begin of result
    virtual void readSuffix() {};                /// delimiter after end of result
};

using RowInputStreamPtr = std::shared_ptr<IRowInputStream>;
}
