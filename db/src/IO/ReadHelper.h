#pragma once

#include<string>
#include<type_traits>
#include<Poco/Exception.h>
#include<IO/ReadBufferFromMemory.h>
#include<IO/ReadBuffer.h>
#include<CommonUtil/StringUtils.h>
#include<Ext/likely.h>
#include<Core/DateLUT.h>
#define DEFAULT_MAX_STRING_SIZE 0x00FFFFFFULL
#include<IO/VarInt.h>

namespace ErrorCodes {
extern const int CANNOT_PARSE_DATE;
}

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
inline typename std::enable_if<!std::is_integral<T>::value, void>::type
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


template <typename T, bool throw_on_error = true>
void readIntTextUnsafe(T & x, ReadBuffer & buf)
{
    bool negative = false;
    x = 0;

    auto on_error = []
    {
        if (throw_on_error)
        {
            throw Poco::Exception("try read eof");
        }
    };

    if (unlikely(buf.eof()))
        return on_error();

    if (std::is_signed<T>::value && *buf.position() == '-')
    {
        ++buf.position();
        negative = true;
        if (unlikely(buf.eof()))
            return on_error();
    }

    if (*buf.position() == '0')                    /// There are many zeros in real datasets.
    {
        ++buf.position();
        return;
    }

    while (!buf.eof())
    {
        if ((*buf.position() & 0xF0) == 0x30)    /// It makes sense to have this condition inside loop.
        {
            x *= 10;
            x += *buf.position() & 0x0F;
            ++buf.position();
        }
        else
            break;
    }

    if (std::is_signed<T>::value && negative)
        x = -x;
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


void readQuotedStringWithSQLStyle(std::string & s, ReadBuffer & buf);


inline void skipWhitespaceIfAny(ReadBuffer & buf)
{
    while (!buf.eof() && StringUtils::isWhitespaceASCII(*buf.position()))
        ++buf.position();
}

void assertChar(char symbol, ReadBuffer & buf);


/// In YYYY-MM-DD format
inline void readDateText(DataBase::DayNum_t & date, ReadBuffer & buf)
{
    char s[10];
    size_t size = buf.read(s, 10);
    if (10 != size)
    {
        s[size] = 0;
        throw Poco::Exception(std::string("Cannot parse date ") + s, ErrorCodes::CANNOT_PARSE_DATE);
    }

    Poco::UInt16 year = (s[0] - '0') * 1000 + (s[1] - '0') * 100 + (s[2] - '0') * 10 + (s[3] - '0');
    Poco::UInt8 month = (s[5] - '0') * 10 + (s[6] - '0');
    Poco::UInt8 day = (s[8] - '0') * 10 + (s[9] - '0');

    date = DataBase::DateLUT::instance().makeDayNum(year, month, day);
}

inline void readStringBinary(std::string & s, ReadBuffer & buf, size_t MAX_STRING_SIZE = DEFAULT_MAX_STRING_SIZE)
{
    size_t size = 0;
    IO::readVarUInt(size, buf);

    if (size > MAX_STRING_SIZE)
        throw Poco::Exception("Too large string size.");

    s.resize(size);
    buf.readStrict(&s[0], size);
}

template <typename T>
inline typename std::enable_if<std::is_arithmetic<T>::value, void>::type
readBinary(T & x, ReadBuffer & buf) { readPODBinary(x, buf); }

inline void readBinary(std::string & x, ReadBuffer & buf) { readStringBinary(x, buf); }

}
