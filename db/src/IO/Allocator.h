#pragma once
#include<string>

namespace IO {

template <bool clear_memory_>
class Allocator
{
protected:
    static constexpr bool clear_memory = clear_memory_;

public:
    /// Allocate memory range.
    void* alloc(size_t size, size_t alignment = 0);

    /// Free memory range.
    void free(void * buf, size_t size);

    /** Enlarge memory range.
      * Data from old range is moved to the beginning of new range.
      * Address of memory range could change.
      */
    void * realloc(void * buf, size_t old_size, size_t new_size, size_t alignment = 0);

protected:
    static constexpr size_t getStackThreshold()
    {
        return 0;
    }
};

}






