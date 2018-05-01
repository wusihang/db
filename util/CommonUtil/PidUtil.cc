#include<CommonUtil/PidUtil.h>
#include<Poco/File.h>
#include<Poco/Path.h>
#include<Poco/Exception.h>

#include<sstream>

#include <sys/types.h>
#include <sys/fcntl.h>
#include<unistd.h>

void PidUtil::Pid::seed(const std::string& file_)
{
    file = Poco::Path(file_).absolute().toString();
    //if pid file not exist, then create it with mode 666 (of couse , if you set a mask , then the final result would be 0666^mask)
    int fd = open(file.c_str(),
                  O_CREAT | O_EXCL | O_WRONLY,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (-1 == fd) {
        file.clear();
        //if pid file exists , throw exception
        if (EEXIST == errno)
            throw Poco::Exception("Pid file exists, should not start daemon.");
        throw Poco::CreateFileException("Cannot create pid file.");
    }
    try {
        std::stringstream s;
        s << getpid();
        if (static_cast<ssize_t>(s.str().size()) != write(fd, s.str().c_str(), s.str().size()))
            throw Poco::Exception("Cannot write to pid file.");
    } catch (...) {
        close(fd);
        throw;
    }
    close(fd);
}

PidUtil::Pid::~Pid()
{
    if (!file.empty()) {
        Poco::File(file).remove();
        file.clear();
    }
}
