#include<IO/WriteBufferFromFileDescriptor.h>
#include<IO/WriteBufferHelper.h>
#include<unistd.h>
#include<Poco/Exception.h>

IO::WriteBufferFromFileDescriptor::WriteBufferFromFileDescriptor(int fd_, size_t buf_size, char* existing_memory, size_t alignment)
    : WriteBufferFromFileBase(buf_size, existing_memory, alignment),fd(fd_)
{

}
IO::WriteBufferFromFileDescriptor::~WriteBufferFromFileDescriptor()
{
    try
    {
        if (fd >= 0)
            next();
    }
    catch (...)
    {
    }
}



off_t IO::WriteBufferFromFileDescriptor::doSeek(off_t offset, int whence)
{
    off_t res = lseek(fd, offset, whence);
    if (-1 == res)
        throw Poco::Exception("Cannot seek through file " + getFileName());
    return res;
}

void IO::WriteBufferFromFileDescriptor::doTruncate(off_t length)
{
    int res = ftruncate(fd, length);
    if (-1 == res)
        throw Poco::Exception("Cannot truncate file " + getFileName());
}

std::string IO::WriteBufferFromFileDescriptor::getFileName() const
{
    return "(fd = " + IO::toString(fd) + ")";
}

void IO::WriteBufferFromFileDescriptor::sync()
{
    next();
    int res = ::fsync(fd);
    if (-1 == res)
        throw Poco::Exception("Cannot fsync " + getFileName());
}


off_t IO::WriteBufferFromFileDescriptor::getPositionInFile()
{
    return seek(0, SEEK_CUR);
}


void IO::WriteBufferFromFileDescriptor::nextImpl()
{
    if (!offset())
        return;

    size_t bytes_written = 0;
    while (bytes_written != offset())
    {
        ssize_t res = 0;
        {
            res = ::write(fd, working_buffer.begin() + bytes_written, offset() - bytes_written);
        }

        if ((-1 == res || 0 == res) && errno != EINTR)
        {
            throw Poco::Exception("Cannot write to file " + getFileName());
        }
        if (res > 0)
            bytes_written += res;
    }
}






