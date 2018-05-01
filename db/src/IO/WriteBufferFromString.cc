#include<IO/WriteBufferFromString.h>
#include<string>

const static int WRITE_BUFFER_FROM_STRING_INITIAL_SIZE_IF_EMPTY = 32;

IO::WriteBufferFromString::WriteBufferFromString(std::string& s_)
    : WriteBuffer(reinterpret_cast<Position>(&s_[0]), s_.size()), s(s_) {
    if (s.empty()) {
        s.resize(WRITE_BUFFER_FROM_STRING_INITIAL_SIZE_IF_EMPTY);
        set(reinterpret_cast<Position>(&s[0]), s.size());
    }
}

void IO::WriteBufferFromString::nextImpl()
{
    size_t old_size = s.size();
    s.resize(old_size * 2);
    internal_buffer = Buffer(reinterpret_cast<Position>(&s[old_size]), reinterpret_cast<Position>(&*s.end()));
    working_buffer = internal_buffer;
}

void IO::WriteBufferFromString::finish()
{
    s.resize(count());
}

IO::WriteBufferFromString::~WriteBufferFromString()
{
    finish();
}



IO::WriteBufferFromOwnString::WriteBufferFromOwnString()
    : IO::WriteBufferFromString(value) {
}

std::string& IO::WriteBufferFromOwnString::str()
{
    finish();
    return value;
}


