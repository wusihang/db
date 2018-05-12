#pragma once
#include<algorithm>

namespace IO {
// buffer中的游标，指示读写位置
using Position = char *;

class Buffer {
public:
    Buffer(Position begin_pos_, Position end_pos_)
        : begin_pos(begin_pos_), end_pos(end_pos_) {
    }

    inline Position begin() const {
        return begin_pos;
    }
    inline Position end() const {
        return end_pos;
    }
    inline size_t size() const {
        return end_pos - begin_pos;
    }
    inline void resize(size_t size) {
        end_pos = begin_pos + size;
    }

    //交换两个buffer,只要简单交换buffer的起始指针即可
    inline void swap(Buffer & other) {
        std::swap(begin_pos, other.begin_pos);
        std::swap(end_pos, other.end_pos);
    }

private:
    Position begin_pos;
    Position end_pos;        /// 1 byte after the end of the buffer
};

class BufferBase {
public:
    /** 
	 *  构造函数参数为,ptr: 内存起始地址,size: buffer尺寸,offset: 位置相对起始地址的偏移量
	 *  对于读buffer来说,offset设置为结尾,  对于写buffer来说, offset设置为开头
     */
    BufferBase(Position ptr, size_t size, size_t offset)
        : internal_buffer(ptr, ptr + size), working_buffer(ptr, ptr + size), pos(ptr + offset) {
    }

    //和构造函数参数含义一致
    void set(Position ptr, size_t size, size_t offset) {
        internal_buffer = Buffer(ptr, ptr + size);
        working_buffer = Buffer(ptr, ptr + size);
        pos = ptr + offset;
    }

    ///  获取buffer自身
    inline Buffer & internalBuffer() {
        return internal_buffer;
    }

    /// 获取可自由读写的buffer
    inline Buffer & buffer() {
        return working_buffer;
    }

    /// 获取buffer的当前读写指针位置
    inline Position & position() {
        return pos;
    }
    ;

    // 当前读写位置相对起始位置的偏移量
    inline size_t offset() const {
        return pos - working_buffer.begin();
    }

    //计算总共读取的字节数,包括已经读写的,还有当前buffer中正在读写的字节数
    size_t count() const {
        return bytes + offset();
    }

    //检查当前指针位置之后是否还有buffer空间,也就是判断buffer是否还有空余空间
    bool __attribute__((__always_inline__))  hasPendingData() const {
        return pos != working_buffer.end();
    }

protected:
    /// buffer内存区域
    Buffer internal_buffer;

    /** 
	 *  可用内存buffer区域
     * For example, if internal_buffer is 1MB, and from a file for reading it was loaded into the buffer
     *  only 10 bytes, then working_buffer will be 10 bytes in size
     *  (working_buffer.end() will point to the position immediately after the 10 bytes that can be read).
     */
    Buffer working_buffer;

    /// 读写内存位置
    Position pos;

    /** 
	 * 已经读写的字节数,不包括正在当前buffer中的字节
     */
    size_t bytes = 0;
};
}
