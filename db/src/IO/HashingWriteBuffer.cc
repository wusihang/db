#include<IO/HashingWriteBuffer.h>
#include<IO/ReadBuffer.h>
#include<cstring>

namespace IO {

template<typename T>
void IHashingBuffer<T>::calculateHash(Position data, size_t len)
{
    if (len)
    {
        /// if the data is less than `block_size`, then put them into buffer and calculate hash later
        if (block_pos + len < block_size)
        {
            ::memcpy(&BufferWithOwnMemory<T>::memory[block_pos], data, len);
            block_pos += len;
        }
        else
        {
            /// if something is already written to the buffer, then we'll add it
            if (block_pos)
            {
                size_t n = block_size - block_pos;
                ::memcpy(&BufferWithOwnMemory<T>::memory[block_pos], data, n);
                append(&BufferWithOwnMemory<T>::memory[0]);
                len -= n;
                data += n;
                block_pos = 0;
            }

            while (len >= block_size)
            {
                append(data);
                len -= block_size;
                data += block_size;
            }

            /// write the remainder to its buffer
            if (len)
            {
                ::memcpy(&BufferWithOwnMemory<T>::memory[0], data, len);
                block_pos = len;
            }
        }
    }
}

template class IHashingBuffer<IO::ReadBuffer>;
template class IHashingBuffer<IO::WriteBuffer>;
}
