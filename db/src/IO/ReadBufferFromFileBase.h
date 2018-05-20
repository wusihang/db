#pragma once
#include<IO/BufferWithOwnMemory.h>
#include<IO/ReadBuffer.h>
#include<ctime>
#include<unistd.h>
#include<string>
#include<functional>
#include<fcntl.h>
namespace IO {

class ReadBufferFromFileBase: public BufferWithOwnMemory<ReadBuffer> {
public:
    ReadBufferFromFileBase(size_t buf_size, char * existing_memory, size_t alignment);
    virtual ~ReadBufferFromFileBase();
    off_t seek(off_t off, int whence = SEEK_SET);
    virtual off_t getPositionInFile() = 0;
    virtual std::string getFileName() const = 0;
    virtual int getFD() const = 0;

protected:
    clockid_t clock_type;

    virtual off_t doSeek(off_t off, int whence) = 0;
};


}
