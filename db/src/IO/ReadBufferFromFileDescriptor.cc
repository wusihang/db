#include<IO/ReadBufferFromFileDescriptor.h>
#include<IO/WriteBufferHelper.h>
#include<Poco/Exception.h>
#include<errno.h>
#include<time.h>
#include<sys/types.h>
#include<sys/time.h>
#include<sys/select.h>

std::string IO::ReadBufferFromFileDescriptor::getFileName() const
{
    return "(fd = " + IO::toString(fd) + ")";
}

off_t IO::ReadBufferFromFileDescriptor::doSeek(off_t offset, int whence)
{
    off_t new_pos = offset;
    if(whence == SEEK_CUR) {
        //查找当前读写位置, 文件位置 - buffer剩余空间 + 偏移
        new_pos = pos_in_file - (working_buffer.end() - pos) + offset;
    } else if(whence!=SEEK_SET) {
        throw Poco::Exception("ReadBufferFromFileDescriptor::seek expects SEEK_SET or SEEK_CUR as whence");
    }

    //如果new_pos + buffer剩余空间 == 文件位置
    if(new_pos+ (working_buffer.end() - pos) == pos_in_file) {
        return new_pos;
    }
    //如果buffer还有空间,并且新的位置不足以填满buffer,那么改变当前buffer的pos,返回new_pos
    if(hasPendingData() && new_pos <=pos_in_file&& new_pos>= pos_in_file-static_cast<off_t>(working_buffer.size())) {
        pos = working_buffer.begin() + (new_pos - (pos_in_file - working_buffer.size()));
        return new_pos;
    } else {
        pos = working_buffer.end();
        //SEEK_SET表示参数new_pos即为新的读写位置
        off_t res = lseek(fd,new_pos,SEEK_SET);
        if(-1 == res) {
            throw Poco::Exception("cannot seek through file" + getFileName());
        }
        pos_in_file = new_pos;
        return res;
    }
}

bool IO::ReadBufferFromFileDescriptor::nextImpl()
{
    size_t bytes_read = 0;
    while(!bytes_read) {

        ssize_t res = 0;
        {
            res = ::read(fd,internal_buffer.begin(),internal_buffer.size());
        }
        if(!res) {
            break;
        }
        if(-1 == res && errno!=EINTR) {
            throw Poco::Exception("cannot read file from" + getFileName());
        }
        if(res> 0 ) {
            bytes_read += res;
        }
    }

    pos_in_file += bytes_read;
    if(bytes_read) {
        working_buffer.resize(bytes_read);
        return true;
    } else {
        return false;
    }
}

bool IO::ReadBufferFromFileDescriptor::poll(size_t timeout_microseconds)
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    struct timeval timeout = {time_t(timeout_microseconds/1000000), suseconds_t(timeout_microseconds%100000)};

    int res = select(1, &fds, 0, 0, &timeout);

    if (-1 == res)
        throw Poco::Exception("Cannot call linux select");

    return res > 0;
}




