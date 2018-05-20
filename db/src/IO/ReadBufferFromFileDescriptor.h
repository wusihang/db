#pragma once
#include<IO/ReadBufferFromFileBase.h>

namespace IO {

class ReadBufferFromFileDescriptor: public ReadBufferFromFileBase {

protected:
	//文件描述符
    int fd;
	//文件偏移, 与worker_buffer.end()对应
	//初始偏移为0
    off_t pos_in_file; 

    bool nextImpl() override;

    /// 文件描述符名称
    std::string getFileName() const override;

public:
    ReadBufferFromFileDescriptor(int fd_, size_t buf_size = DBMS_DEFAULT_BUFFER_SIZE, char * existing_memory = nullptr, size_t alignment = 0)
        : ReadBufferFromFileBase(buf_size, existing_memory, alignment), fd(fd_), pos_in_file(0) {}

    int getFD() const override
    {
        return fd;
    }

    off_t getPositionInFile() override
    {
        return pos_in_file - (working_buffer.end() - pos);
    }
    
    virtual ~ReadBufferFromFileDescriptor() = default;

private:
    /// If 'offset' is small enough to stay in buffer after seek, then true seek in file does not happen.
    off_t doSeek(off_t offset, int whence) override;

    /// Assuming file descriptor supports 'select', check that we have data to read or wait until timeout.
    bool poll(size_t timeout_microseconds);

};

}
