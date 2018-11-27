#include<Streams/BlockInputStreamFromRowInputStream.h>


namespace IO {

BlockInputStreamFromRowInputStream::BlockInputStreamFromRowInputStream(RowInputStreamPtr row_input_, const Block& sample_, size_t max_block_size_)
    :row_input(row_input_),sample(sample_),max_block_size(max_block_size_) {

}

Block BlockInputStreamFromRowInputStream::read()
{
    Block res = sample.cloneEmpty();
    for(size_t row = 0 ; row < max_block_size; row++) {
        if(!row_input->read(res)) {
            break;
        }
    }
    if (res.rows() == 0)
    {
        res.clear();
    }
    return res;
}


}
