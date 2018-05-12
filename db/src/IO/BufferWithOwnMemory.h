#pragma once
#include<Common/CommonDefines.h>
#include <Ext/noncopyable.h>
#include<IO/Allocator.h>
#include<functional>

namespace IO {

//内存, 不可拷贝构造,不可赋值构造 , 内存分配时不初始化内存区域
struct Memory :ext::noncopyable, Allocator<false>
{
	//内存容量
    size_t m_capacity = 0;
	//内存大小
    size_t m_size = 0;
	//内存数据
    char * m_data = nullptr;
	//对齐
    size_t alignment = 0;

    Memory() {}

    /// 构造函数,如果alignment不为0 ,那么就要对其字节
    Memory(size_t size_, size_t alignment_ = 0) : m_capacity(size_), m_size(m_capacity), alignment(alignment_)
    {
        alloc();
    }

    ~Memory()
    {
        dealloc();
    }

    Memory(Memory && rhs)
    {
        *this = std::move(rhs);
    }

    Memory & operator=(Memory && rhs)
    {
        std::swap(m_capacity, rhs.m_capacity);
        std::swap(m_size, rhs.m_size);
        std::swap(m_data, rhs.m_data);
        std::swap(alignment, rhs.alignment);

        return *this;
    }

    size_t size() const {
        return m_size;
    }
    const char & operator[](size_t i) const {
        return m_data[i];
    }
    char & operator[](size_t i) {
        return m_data[i];
    }
    const char * data() const {
        return m_data;
    }
    char * data() {
        return m_data;
    }

    //重新分配内存大小
    void resize(size_t new_size)
    {
        if (0 == m_capacity)
        {
            m_size = m_capacity = new_size;
            alloc();
        }
        else if (new_size < m_capacity)
        {
            m_size = new_size;
            return;
        }
        else
        {
            new_size = align(new_size, alignment);
            m_data = static_cast<char *>(Allocator::realloc(m_data, m_capacity, new_size, alignment));
            m_capacity = new_size;
            m_size = m_capacity;
        }
    }

    //内存对齐,向上取整
    static size_t align(const size_t value, const size_t alignment)
    {
        if (!alignment)
            return value;

        return (value + alignment - 1) / alignment * alignment;
    }

private:
    void alloc()
    {
        if (!m_capacity)
        {
            m_data = nullptr;
            return;
        }
        //字节对齐
        size_t new_capacity = align(m_capacity, alignment);
		//分配内存
        m_data = static_cast<char *>(Allocator::alloc(new_capacity, alignment));
        m_capacity = new_capacity;
		//默认内存大小和内存容量是一致的
        m_size = m_capacity;
    }

    void dealloc()
    {
		//如果内存已分配
        if (m_data)
        {
			//释放内存
            Allocator::free(m_data, m_capacity);
			//指针清0 ,防止出现野指针, 否则可能会导致下次内存分配失败
            m_data = nullptr;
        }
    }
};



template <typename Base>
class BufferWithOwnMemory : public Base
{
protected:
    Memory memory;
public:
    /// If non-nullptr 'existing_memory' is passed, then buffer will not create its own memory and will use existing_memory without ownership.
    BufferWithOwnMemory(size_t size = DBMS_DEFAULT_BUFFER_SIZE, char * existing_memory = nullptr, size_t alignment = 0)
        : Base(nullptr, 0), memory(existing_memory ? 0 : size, alignment)
    {
		//如果existing_memory=nullptr,那么就使用memory,默认分配1M 作为缓冲区
        Base::set(existing_memory ? existing_memory : memory.data(), size);
    }
};

}


