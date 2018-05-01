#pragma once
#include<IO/BufferBase.h>

#include<Poco/Exception.h>

#include<algorithm>
#include<cstring>
#include<memory>
#include<iostream>


namespace IO {

class WriteBuffer: public BufferBase {
public:
    //字符串起始地址，字符串长度
    WriteBuffer(Position ptr, size_t size)
        : BufferBase(ptr, size, 0) {
    }

    void set(Position ptr, size_t size) {
        BufferBase::set(ptr, size, 0);
    }

    inline void next() {
        size_t off = offset();
        if (off) {
            bytes += off;
            try {
                nextImpl();
            } catch (...) {
                pos = working_buffer.begin();
                throw;
            }
            pos = working_buffer.begin();
        }
    }

    void write(const char * from, size_t n) {
        size_t bytes_copied = 0;
        while (bytes_copied < n) {
            nextIfAtEnd();
            size_t bytes_to_copy = std::min(static_cast<size_t>(working_buffer.end() - pos), n - bytes_copied);
            std::memcpy(pos, from + bytes_copied, bytes_to_copy);
            pos += bytes_to_copy;
            bytes_copied += bytes_to_copy;
        }
    }

    virtual ~WriteBuffer() = default;

    inline void nextIfAtEnd() {
        if (!hasPendingData())
        {
            next();
        }
    }

    inline void write(char x) {
        nextIfAtEnd();
        *pos = x;
        ++pos;
    }

private:
    virtual void nextImpl() {
        throw Poco::Exception("Cannot write after end of buffer.");
    }

};

}
