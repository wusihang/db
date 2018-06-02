#include<IO/WriteBufferFromFile.h>
#include<IO/WriteBufferHelper.h>
#include<CommonUtil/LoggerUtil.h>
#include<sys/unistd.h>
namespace ErrorCodes
{
extern const int FILE_DOESNT_EXIST;
extern const int CANNOT_OPEN_FILE;
extern const int CANNOT_CLOSE_FILE;
}



namespace IO {

void WriteBufferFromFile::close()
{
    next();
    if (0 != ::close(fd))
    {
        throw Poco::Exception("Cannot close file", ErrorCodes::CANNOT_CLOSE_FILE);
    }
    fd = -1;
}

WriteBufferFromFile::WriteBufferFromFile(int fd, const std::string& original_file_name, size_t buf_size, char* existing_memory, size_t alignment)
    : WriteBufferFromFileDescriptor(fd, buf_size, existing_memory, alignment),
      filename(original_file_name.empty() ? "(fd = " +toString(fd) + ")" : original_file_name)
{

}

WriteBufferFromFile::WriteBufferFromFile(const std::string& file_name_, size_t buf_size, int flags, mode_t mode, char* existing_memory, size_t alignment)
    : WriteBufferFromFileDescriptor(-1, buf_size, existing_memory, alignment),filename(file_name_)
{
    fd = open(filename.c_str(), flags == -1 ? O_WRONLY | O_TRUNC | O_CREAT : flags, mode);
    if (-1 == fd)
    {
        throw Poco::Exception("Cannot open file " + filename, errno == ENOENT ? ErrorCodes::FILE_DOESNT_EXIST : ErrorCodes::CANNOT_OPEN_FILE);
    }
}

WriteBufferFromFile::~WriteBufferFromFile()
{
    if (fd >= 0) {
        try
        {
            next();
        }
        catch (...)
        {
            DataBase::currentExceptionLog();
        }
        ::close(fd);
    }
}


}



