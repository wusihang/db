#include<IO/WriteBufferFromFileDescriptor.h>
#include <sys/types.h>
namespace IO {

class WriteBufferFromFile:public WriteBufferFromFileDescriptor {

protected:
    std::string filename;
public:
    WriteBufferFromFile(
        const std::string & file_name_,
        size_t buf_size = DBMS_DEFAULT_BUFFER_SIZE,
        int flags = -1,
        mode_t mode = 0666,
        char * existing_memory = nullptr,
        size_t alignment = 0);

    /// Use pre-opened file descriptor.
    WriteBufferFromFile(
        int fd,
        const std::string & original_file_name,
        size_t buf_size = DBMS_DEFAULT_BUFFER_SIZE,
        char * existing_memory = nullptr,
        size_t alignment = 0);

    ~WriteBufferFromFile() override;

    void close();

    std::string getFileName() const override
    {
        return filename;
    }

};

}
