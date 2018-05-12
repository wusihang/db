#pragma once

#include <iostream>
#include <Poco/Exception.h>
#include <IO/WriteBuffer.h>
#include <IO/BufferWithOwnMemory.h>


namespace IO
{

//BufferWithOwnMemory继承自WriteBuffer
class WriteBufferFromOStream : public BufferWithOwnMemory<WriteBuffer>
{
private:
    std::ostream & ostr;

    void nextImpl() override
    {
		//如果buffer中有内容,那么就将内容写入到ostr
        if (offset()) {
            ostr.write(working_buffer.begin(), offset());
            ostr.flush();
			//ostr写入异常时抛出异常信息
            if (!ostr.good())
            {
                throw Poco::Exception("Cannot write to ostream");
            }
        }
    }

public:
    WriteBufferFromOStream(
        std::ostream & ostr_,
        size_t size = DBMS_DEFAULT_BUFFER_SIZE,
        char * existing_memory = nullptr,
        size_t alignment = 0)
        : BufferWithOwnMemory<WriteBuffer>(size, existing_memory, alignment), ostr(ostr_) {}

    ~WriteBufferFromOStream() override
    {
        try
        {
			//析构时,将buffer中的内容写入ostr
            next();
        }
        catch (...)
        {

        }
    }
};

}
