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

    //同构造函数
    void set(Position ptr, size_t size) {
        BufferBase::set(ptr, size, 0);
    }

    // 如果当前位置偏移不为0,也就是buffer中读取了相关内容,那么执行nextImpl,并重置pos为起点
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

    //写入n个字节
    void write(const char * from, size_t n) {
        size_t bytes_copied = 0;
        while (bytes_copied < n) {
			//如果buffer写满了,那么执行next,否则继续
            nextIfAtEnd();
			//取当前剩余buffer和待写入剩余字节的最小值
            size_t bytes_to_copy = std::min(static_cast<size_t>(working_buffer.end() - pos), n - bytes_copied);
			//将待写入字节拷贝到对应的buffer内存区域
            std::memcpy(pos, from + bytes_copied, bytes_to_copy);
			//记录buffer偏移
            pos += bytes_to_copy;
            bytes_copied += bytes_to_copy;
        }
    }

    virtual ~WriteBuffer() = default;

	//判断buffer是否已经满了,满了就调用next
    inline void nextIfAtEnd() {
        if (!hasPendingData())
        {
            next();
        }
    }

    //单个字节写入
    inline void write(char x) {
        nextIfAtEnd();
        *pos = x;
        ++pos;
    }

private:
	//WriteBuffer的子类都必须重写该方法
    virtual void nextImpl() {
        throw Poco::Exception("Cannot write after end of buffer.");
    }

};

}
