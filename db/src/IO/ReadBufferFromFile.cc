#include<IO/ReadBufferFromFile.h>
#include<IO/WriteBufferHelper.h>

namespace ErrorCodes {
extern const int CANNOT_CLOSE_FILE;
extern const int CANNOT_OPEN_FILE;
extern const int FILE_DOESNT_EXIST;
}


void IO::ReadBufferFromFile::close()
{
    if (0 != ::close(fd))
    {
        throw Poco::Exception("Cannot close file", ErrorCodes::CANNOT_CLOSE_FILE);
    }
    fd = -1;
}


IO::ReadBufferFromFile::ReadBufferFromFile(const std::string& file_name_, size_t buf_size, int flags, char* existing_memory, size_t alignment)
    : ReadBufferFromFileDescriptor(-1, buf_size, existing_memory, alignment),file_name(file_name_)
{
    fd = open(file_name.c_str(), flags == -1 ? O_RDONLY : flags);
    if (-1 == fd)
    {
        throw Poco::Exception("Cannot open file " + file_name, errno == ENOENT ? ErrorCodes::FILE_DOESNT_EXIST : ErrorCodes::CANNOT_OPEN_FILE);
    }
}


IO::ReadBufferFromFile::ReadBufferFromFile(int fd, const std::string& original_file_name, size_t buf_size, int flags, char* existing_memory, size_t alignment)
    : ReadBufferFromFileDescriptor(flags, buf_size, existing_memory, alignment),
      file_name(original_file_name.empty() ? "(fd = " + toString(fd) + ")" : original_file_name)
{

}


IO::ReadBufferFromFile::~ReadBufferFromFile()
{
    if (fd < 0)
        return;
    ::close(fd);
}
