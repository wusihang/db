#pragma once
#include<IO/BufferWithOwnMemory.h>
#include<IO/WriteBuffer.h>
#include <vector>
#include<IO/CompressStreams.h>

namespace IO {

class CompressedWriteBuffer : public BufferWithOwnMemory<WriteBuffer>
{
private:
    WriteBuffer & out;
    CompressionMethod method;

    std::vector<char> compressed_buffer;

    void nextImpl() override;

public:
    CompressedWriteBuffer(
        WriteBuffer & out_,
        CompressionMethod method_ = CompressionMethod::NONE,
        size_t buf_size = DBMS_DEFAULT_BUFFER_SIZE);

    /// The amount of compressed data
    size_t getCompressedBytes()
    {
        nextIfAtEnd();
        return out.count();
    }

    /// How many uncompressed bytes were written to the buffer
    size_t getUncompressedBytes()
    {
        return count();
    }

    /// How many bytes are in the buffer (not yet compressed)
    size_t getRemainingBytes()
    {
        nextIfAtEnd();
        return offset();
    }

    ~CompressedWriteBuffer() override;
};

}
