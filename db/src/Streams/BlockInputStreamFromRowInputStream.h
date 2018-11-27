#pragma once
#include<Streams/IBlockInputStream.h>
#include<Streams/IRowInputStream.h>
namespace IO {

class BlockInputStreamFromRowInputStream:public IBlockInputStream {
public:
    BlockInputStreamFromRowInputStream( RowInputStreamPtr row_input_, const Block & sample_,size_t max_block_size_);
    Block read() override;
private:
    RowInputStreamPtr row_input;
    const Block sample;
    size_t max_block_size;
};

}
