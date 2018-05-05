#pragma once
#include<string>
#include<type_traits>
#include<Poco/Exception.h>
#include<IO/ReadBufferFromMemory.h>
#include<IO/ReadBuffer.h>

namespace IO {
template <typename T, typename ReturnType = void>
ReturnType readIntTextImpl(T & x, ReadBuffer & buf)
{
    static constexpr bool throw_exception = std::is_same<ReturnType, void>::value;

    bool negative = false;
    x = 0;
    if (buf.eof())
    {
        if (throw_exception)
            throw Poco::Exception("attempt to read after EOF");
        else
            return ReturnType(false);
    }

    while (!buf.eof())
    {
        switch (*buf.position())
        {
        case '+':
            break;
        case '-':
            if (std::is_signed<T>::value)
                negative = true;
            else
            {
                if (throw_exception)
                    throw Poco::Exception("Unsigned type must not contain '-' symbol");
                else
                    return ReturnType(false);
            }
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            x *= 10;
            x += *buf.position() - '0';
            break;
        default:
            if (negative)
                x = -x;
            return ReturnType(true);
        }
        ++buf.position();
    }
    if (negative)
        x = -x;

    return ReturnType(true);
}


template <typename T>
void readIntText(T & x, ReadBuffer & buf)
{
    readIntTextImpl<T, void>(x, buf);
}

template <typename T>
inline typename std::enable_if<std::is_integral<T>::value, void>::type
readText(T & x, ReadBuffer & buf) {
    readIntText(x, buf);
}


template <typename T>
inline T parse(const char * data, size_t size)
{
    T res;
    IO::ReadBufferFromMemory buf(data, size);
    readText(res, buf);
    return res;
}

template <typename T>
inline T parse(const std::string & s)
{
    return parse<T>(s.data(), s.size());
}



}
