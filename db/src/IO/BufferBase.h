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
    /** The constructor takes a range of memory to use for the buffer.
     * offset - the starting point of the cursor. ReadBuffer must set it to the end of the range, and WriteBuffer - to the beginning.
     */
    BufferBase(Position ptr, size_t size, size_t offset)
        : internal_buffer(ptr, ptr + size), working_buffer(ptr, ptr + size), pos(ptr + offset) {
    }

    void set(Position ptr, size_t size, size_t offset) {
        internal_buffer = Buffer(ptr, ptr + size);
        working_buffer = Buffer(ptr, ptr + size);
        pos = ptr + offset;
    }

    /// get buffer
    inline Buffer & internalBuffer() {
        return internal_buffer;
    }

    /// get the part of the buffer from which you can read / write data
    inline Buffer & buffer() {
        return working_buffer;
    }

    /// get (for reading and modifying) the position in the buffer
    inline Position & position() {
        return pos;
    }
    ;

    /// offset in bytes of the cursor from the beginning of the buffer
    inline size_t offset() const {
        return pos - working_buffer.begin();
    }

    /** How many bytes have been read/written, counting those that are still in the buffer. */
    size_t count() const {
        return bytes + offset();
    }

    /** Check that there is more bytes in buffer after cursor. */
    bool __attribute__((__always_inline__))  hasPendingData() const {
        return pos != working_buffer.end();
    }

protected:
    /// A reference to a piece of memory for the buffer.
    Buffer internal_buffer;

    /** A piece of memory that you can use.
     * For example, if internal_buffer is 1MB, and from a file for reading it was loaded into the buffer
     *  only 10 bytes, then working_buffer will be 10 bytes in size
     *  (working_buffer.end() will point to the position immediately after the 10 bytes that can be read).
     */
    Buffer working_buffer;

    /// Read/write position.
    Position pos;

    /** How many bytes have been read/written, not counting those that are now in the buffer.
     * (counting those that were already used and "removed" from the buffer)
     */
    size_t bytes = 0;
};
}
