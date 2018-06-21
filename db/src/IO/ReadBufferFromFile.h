#include<IO/ReadBufferFromFileDescriptor.h>

namespace IO {


/** Accepts path to file and opens it, or pre-opened file descriptor.
  * Closes file by himself (thus "owns" a file descriptor).
  */
class ReadBufferFromFile : public ReadBufferFromFileDescriptor
{
protected:
    std::string file_name;
public:
    ReadBufferFromFile(const std::string & file_name_, size_t buf_size = DBMS_DEFAULT_BUFFER_SIZE, int flags = -1,
                       char * existing_memory = nullptr, size_t alignment = 0);

    /// Use pre-opened file descriptor.
    ReadBufferFromFile(int fd, const std::string & original_file_name, size_t buf_size = DBMS_DEFAULT_BUFFER_SIZE, int flags = -1,
                       char * existing_memory = nullptr, size_t alignment = 0);

    ~ReadBufferFromFile() override;

    /// Close file before destruction of object.
    void close();

    std::string getFileName() const override
    {
        return file_name;
    }
};

}
