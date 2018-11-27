#include<IO/CompressedWriteBuffer.h>
#include<Exception/ExceptionHelper.h>
#include <Core/Types.h>
#include<Ext/unaligned.h>
#include<city.h>

namespace ErrorCodes {
extern const int UNKNOWN_COMPRESSION_METHOD;
}

namespace IO {

CompressedWriteBuffer::CompressedWriteBuffer(WriteBuffer& out_, CompressionMethod method_, size_t buf_size)
    : BufferWithOwnMemory<WriteBuffer>(buf_size), out(out_), method(method_)
{

}


void CompressedWriteBuffer::nextImpl()
{
    size_t uncompressed_size = offset();
    if(uncompressed_size) {
        size_t compressed_size = 0;
        char * compressed_buffer_ptr = nullptr;
        switch(method) {
        case CompressionMethod::NONE:
        {
            static constexpr size_t header_size = 1 + sizeof (DataBase::UInt32) + sizeof (DataBase::UInt32);

            compressed_size = header_size + uncompressed_size;
            DataBase:: UInt32 uncompressed_size_32 = uncompressed_size;
            DataBase::UInt32 compressed_size_32 = compressed_size;

            compressed_buffer.resize(compressed_size);

            compressed_buffer[0] = static_cast<DataBase::UInt8>(CompressionMethodByte::NONE);

            unalignedStore(&compressed_buffer[1], compressed_size_32);
            unalignedStore(&compressed_buffer[5], uncompressed_size_32);
            ::memcpy(&compressed_buffer[9], working_buffer.begin(), uncompressed_size);
            compressed_buffer_ptr = &compressed_buffer[0];
            break;
        }
        default:
            throw Poco::Exception("Unknown compression method", ErrorCodes::UNKNOWN_COMPRESSION_METHOD);
        }
        CityHash_v1_0_2::uint128 checksum = CityHash_v1_0_2::CityHash128(compressed_buffer_ptr, compressed_size);
        out.write(reinterpret_cast<const char *>(&checksum), sizeof(checksum));
        out.write(compressed_buffer_ptr, compressed_size);
    }
}

CompressedWriteBuffer::~CompressedWriteBuffer()
{
    try
    {
        next();
    }
    catch (...)
    {
        DataBase::currentExceptionLog();
    }
}



}
