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
bool tryReadIntText(T & x, ReadBuffer & buf)
{
    return readIntTextImpl<T, bool>(x, buf);
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


template <typename T>
static void appendToStringOrVector(T & s, const char * begin, const char * end)
{
    s.append(begin, end - begin);
}

template <typename Vector>
void readStringUntilEOFInto(Vector & s, ReadBuffer & buf)
{
    while (!buf.eof())
    {
        size_t bytes = buf.buffer().end() - buf.position();

        appendToStringOrVector(s, buf.position(), buf.position() + bytes);
        buf.position() += bytes;

        if (buf.hasPendingData())
            return;
    }
}

void readStringUntilEOF(std::string & s, ReadBuffer & buf);

template <typename T>
inline void readPODBinary(T & x, ReadBuffer & buf)
{
    buf.readStrict(reinterpret_cast<char *>(&x), sizeof(x));
}

inline char parseEscapeSequence(char c)
{
    switch(c)
    {
    case 'a':
        return '\a';
    case 'b':
        return '\b';
    case 'f':
        return '\f';
    case 'n':
        return '\n';
    case 'r':
        return '\r';
    case 't':
        return '\t';
    case 'v':
        return '\v';
    case '0':
        return '\0';
    default:
        return c;
    }
}


template <bool enable_sql_style_quoting, typename Vector>
void readQuotedStringInto(Vector & s, ReadBuffer & buf);


template <bool enable_sql_style_quoting, typename Vector>
void readBackQuotedStringInto(Vector & s, ReadBuffer & buf);

void readBackQuotedStringWithSQLStyle(std::string & s, ReadBuffer & buf);


// void readQuotedString(std::string & s, ReadBuffer & buf);
// void readQuotedStringWithSQLStyle(std::string & s, ReadBuffer & buf);

template <bool enable_sql_style_quoting, typename Vector>
void readDoubleQuotedStringInto(Vector & s, ReadBuffer & buf);

// void readDoubleQuotedString(std::string & s, ReadBuffer & buf);
void readDoubleQuotedStringWithSQLStyle(std::string & s, ReadBuffer & buf);

}
