#pragma once
#include <Streams/IBlockOutputStream.h>
#include<Streams/Block.h>
namespace IO {

struct BlockIO {
    BlockOutputStreamPtr out;
    Block out_sample;   /// Example of a block to be written to `out`.
};
}
