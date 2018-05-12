#include<IO/Allocator.h>
#include<Poco/Exception.h>
#include<IO/WriteBufferHelper.h>
#include<string.h>
#include<sys/mman.h>
#include<stdlib.h>


/** Many modern allocators (for example, tcmalloc) do not do a mremap for realloc,
  *  even in case of large enough chunks of memory.
  * Although this allows you to increase performance and reduce memory consumption during realloc.
  * To fix this, we do mremap manually if the chunk of memory is large enough.
  * The threshold (64 MB) is chosen quite large, since changing the address space is
  *  very slow, especially in the case of a large number of threads.
  * We expect that the set of operations mmap/something to do/mremap can only be performed about 1000 times per second.
  *
  * PS. This is also required, because tcmalloc can not allocate a chunk of memory greater than 16 GB.
  */
static constexpr size_t MMAP_THRESHOLD = 64 * (1ULL << 20);
static constexpr size_t MMAP_MIN_ALIGNMENT = 4096;
static constexpr size_t MALLOC_MIN_ALIGNMENT = 8;


template <bool clear_memory_>
void * IO::Allocator<clear_memory_>::alloc(size_t size, size_t alignment)
{
//     CurrentMemoryTracker::alloc(size);

    void * buf;

    //如果尺寸超过了mmap分配阈值,那么就使用mmap来进行内存地址映射
    if (size >= MMAP_THRESHOLD)
    {
        //mmap分配时,地址对齐参数要求<=4KB , 也就是不能超过内存页尺寸
        if (alignment > MMAP_MIN_ALIGNMENT)
        {
            throw Poco::Exception("Too large alignment: more than page size.");
        }
        //内容可被读写,    写入时拷贝, 匿名映射, 映射起点为0 , length为size,起始地址未指定
        buf = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        //返回映射内存地址, 如果不能正常mmap,抛出异常
        if (MAP_FAILED == buf)
        {
            throw Poco::Exception("Allocator: Cannot mmap.");
        }
    }
    else
    {
        //如果内存对齐要求<=8个字节,那么可以直接使用calloc和malloc来分配内存
        //这时候是自动内存对齐的,因为C库透明地帮处理了
        if (alignment <= MALLOC_MIN_ALIGNMENT)
        {
            //clear_memory由模板传入,含义就是是否要初始化清0内存区域
            if (clear_memory)
            {
                //从堆内存分配 size个1字节空间,也就是分配size个字节
                //分配的内存都是经过初始化的,初始化为0
                buf = ::calloc(size, 1);
            }
            else
            {
                //从堆上获取指定字节的内存空间,参数为要分配的字节数,结果为内存空间首地址
                //得到的内存是未初始化的,可使用memset来清0
                buf = ::malloc(size);
            }
            //如果内存分配失败,那么抛出异常
            if (nullptr == buf)
            {
                throw Poco::Exception("Allocator: Cannot malloc.");
            }
        }
        else
        {
            //内存对齐>8个字节,那么就要手动维护字节对齐
            buf = nullptr;
            //成功时返回size字节的动态内存,这块内存是alignment的倍数,
            //参数alignment必须是2的次幂并且是void*的倍数,返回的内存块地址放在了buf指针
            //函数原型:int posix_memalign (void **memptr,size_t alignment,size_t size);
            int res = posix_memalign(&buf, alignment, size);

            if (0 != res)
            {
                throw Poco::Exception("Cannot allocate memory (posix_memalign)");
            }

            if (clear_memory)
            {
                memset(buf, 0, size);
            }
        }
    }

    return buf;
}


template <bool clear_memory_>
void IO::Allocator<clear_memory_>::free(void * buf, size_t size)
{
    if (size >= MMAP_THRESHOLD)
    {
        //如果大小超过了mmap分配阈值,那么就munmap解除映射
        if (0 != munmap(buf, size))
        {
            throw Poco::Exception("Allocator: Cannot munmap.");
        }
    }
    else
    {
        //释放堆空间
        ::free(buf);
        buf = nullptr;
    }

//     CurrentMemoryTracker::free(size);
}



template <bool clear_memory_>
void * IO::Allocator<clear_memory_>::realloc(void * buf, size_t old_size, size_t new_size, size_t alignment)
{
#if !defined(__APPLE__) && !defined(__FreeBSD__)
    if (old_size < MMAP_THRESHOLD && new_size < MMAP_THRESHOLD && alignment <= MALLOC_MIN_ALIGNMENT)
    {
//         CurrentMemoryTracker::realloc(old_size, new_size);

        //buf为已分配堆内存空间指针, realloc函数将buf指向的内存块改变为new_size字节
        //如果new_size小于原来的空间,那么保持原样.
        //如果n>原来的空间,那么系统将重新为buf从堆上分配一块大小为n的内存空间
        //同时将原来指向空间的内容依次复制到新的内存空间上,p之前指向的空间被释放
        //该函数分配的空间也是未初始化的.
        // 额外说明的是: realloc传入的new_size如果是0,那么就相当于释放空间
        buf = ::realloc(buf, new_size);

        if (nullptr == buf)
        {
            throw Poco::Exception("Allocator: Cannot realloc.");
        }

        //如果clear_memory模板是true,那么每次分配内存都重新初始化(新分配部分)
        if (clear_memory)
        {
            memset(reinterpret_cast<char *>(buf) + old_size, 0, new_size - old_size);
        }
    }
    else if (old_size >= MMAP_THRESHOLD && new_size >= MMAP_THRESHOLD)
    {
//         CurrentMemoryTracker::realloc(old_size, new_size);

        buf = mremap(buf, old_size, new_size, MREMAP_MAYMOVE);
        if (MAP_FAILED == buf)
        {
            throw Poco::Exception("Allocator: Cannot mremap memory chunk from " + IO::toString(old_size) + " to " + IO::toString(new_size) + " bytes.");
        }
    }
#else
    if ((old_size < MMAP_THRESHOLD && new_size < MMAP_THRESHOLD && alignment <= MALLOC_MIN_ALIGNMENT) ||
            (old_size >= MMAP_THRESHOLD && new_size >= MMAP_THRESHOLD))
    {
//         CurrentMemoryTracker::realloc(old_size, new_size);
        buf = ::realloc(buf, new_size);
        if (nullptr == buf)
        {
            throw Poco::Exception("Allocator: Cannot realloc.");
        }
        if (clear_memory)
            memset(reinterpret_cast<char *>(buf) + old_size, 0, new_size - old_size);
    }
#endif
    else
    {
        void * new_buf = alloc(new_size, alignment);
        memcpy(new_buf, buf, old_size);
        free(buf, old_size);
        buf = new_buf;
    }
    return buf;
}


/// Explicit template instantiations.
template class IO::Allocator<true>;
template class IO::Allocator<false>;
