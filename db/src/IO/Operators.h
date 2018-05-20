#pragma once
#include <IO/WriteBufferHelper.h>

inline IO::WriteBuffer & operator<< (IO::WriteBuffer & buf, const char * x)     {
    writeCString(x, buf);
    return buf;
}

template <typename T>     IO::WriteBuffer & operator<< (IO::WriteBuffer & buf, const T & x)        {
    IO::writeText(x, buf);
    return buf;
}
template <> inline        IO::WriteBuffer & operator<< (IO::WriteBuffer & buf, const std::string & x)   {
    IO:: writeString(x, buf);
    return buf;
}
template <> inline        IO::WriteBuffer & operator<< (IO::WriteBuffer & buf, const char & x)     {
    IO::writeChar(x, buf);
    return buf;
}
