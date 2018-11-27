#pragma once
#include<IO/WriteBufferFromString.h>
#include <Poco/LocalDateTime.h>
#include <Poco/DateTimeFormatter.h>
#include <Ext/likely.h>
#include<IO/WriteIntText.h>
#include<string>
#include<CommonUtil/FindSymbol.h>
#include<IO/DoubleConvertor.h>
#include<Core/DateLUT.h>
#include<IO/VarInt.h>

namespace ErrorCodes
{
extern const int CANNOT_PRINT_FLOAT_OR_DOUBLE_NUMBER;
}

namespace IO {

inline void writeChar(char x, IO::WriteBuffer & buf) {
    buf.nextIfAtEnd();
    *buf.position() = x;
    ++buf.position();
}

inline void writeTimestampText(Poco::LocalDateTime localDateTime, const std::string& formatter, WriteBuffer& buf) {
    std::string formattedStr = Poco::DateTimeFormatter::format(localDateTime, formatter);
    buf.write(formattedStr.c_str(), formattedStr.length());
}

template <char date_delimeter = '-', char time_delimeter = ':'>
inline void writeDateTimeText(time_t datetime, WriteBuffer & buf, const DataBase::DateLUTImpl & date_lut = DataBase::DateLUT::instance())
{
    char s[19] = {'0', '0', '0', '0', date_delimeter, '0', '0', date_delimeter, '0', '0', ' ', '0', '0', time_delimeter, '0', '0', time_delimeter, '0', '0'};

    if (unlikely(datetime > DATE_LUT_MAX || datetime == 0))
    {
        buf.write(s, 19);
        return;
    }

    const auto & values = date_lut.getValues(datetime);

    s[0] += values.year / 1000;
    s[1] += (values.year / 100) % 10;
    s[2] += (values.year / 10) % 10;
    s[3] += values.year % 10;
    s[5] += values.month / 10;
    s[6] += values.month % 10;
    s[8] += values.day_of_month / 10;
    s[9] += values.day_of_month % 10;

    Poco::UInt8 hour = date_lut.toHour(datetime);
    Poco::UInt8 minute = date_lut.toMinuteInaccurate(datetime);
    Poco::UInt8 second = date_lut.toSecondInaccurate(datetime);

    s[11] += hour / 10;
    s[12] += hour % 10;
    s[14] += minute / 10;
    s[15] += minute % 10;
    s[17] += second / 10;
    s[18] += second % 10;

    buf.write(s, 19);
}


template <char c>
void writeAnyEscapedString(const char * begin, const char * end, WriteBuffer & buf)
{
    const char * pos = begin;
    while (true)
    {
        /// On purpose we will escape more characters than minimally necessary.
        const char * next_pos = find_first_symbols<'\b', '\f', '\n', '\r', '\t', '\0', '\\', c>(pos, end);

        if (next_pos == end)
        {
            buf.write(pos, next_pos - pos);
            break;
        }
        else
        {
            buf.write(pos, next_pos - pos);
            pos = next_pos;
            switch (*pos)
            {
            case '\b':
                writeChar('\\', buf);
                writeChar('b', buf);
                break;
            case '\f':
                writeChar('\\', buf);
                writeChar('f', buf);
                break;
            case '\n':
                writeChar('\\', buf);
                writeChar('n', buf);
                break;
            case '\r':
                writeChar('\\', buf);
                writeChar('r', buf);
                break;
            case '\t':
                writeChar('\\', buf);
                writeChar('t', buf);
                break;
            case '\0':
                writeChar('\\', buf);
                writeChar('0', buf);
                break;
            case '\\':
                writeChar('\\', buf);
                writeChar('\\', buf);
                break;
            case c:
                writeChar('\\', buf);
                writeChar(c, buf);
                break;
            default:
                writeChar(*pos, buf);
            }
            ++pos;
        }
    }
}

template <char c>
void writeAnyQuotedString(const char * begin, const char * end, WriteBuffer & buf)
{
    writeChar(c, buf);
    writeAnyEscapedString<c>(begin, end, buf);
    writeChar(c, buf);
}

template <char c>
void writeAnyQuotedString(const std::string & s, WriteBuffer & buf)
{
    writeAnyQuotedString<c>(s.data(), s.data() + s.size(), buf);
}


inline void writeString(const std::string& s, WriteBuffer & buf) {
    buf.write(s.data(), s.size());
}

inline void writeString(const char * data, size_t size, WriteBuffer & buf)
{
    buf.write(data, size);
}

/// Outputs a string in backquotes, as an identifier in MySQL.
inline void writeBackQuotedString(const std::string & s, WriteBuffer & buf)
{
    writeAnyQuotedString<'`'>(s, buf);
}


template <typename T>
inline typename std::enable_if<std::is_integral<T>::value, void>::type
writeText(const T & x, WriteBuffer & buf) {
    writeIntText(x, buf);
}

template <typename T>
inline typename std::enable_if<std::is_floating_point<T>::value, void>::type
writeText(const T & x, WriteBuffer & buf) { writeFloatText(x, buf); }


inline void writeEscapedString(const char * str, size_t size, WriteBuffer & buf)
{
    writeAnyEscapedString<'\''>(str, str + size, buf);
}
inline void writeEscapedString(const std::string & s, WriteBuffer & buf)
{
    writeEscapedString(s.data(), s.size(), buf);
}

inline void writeText(const std::string & x,WriteBuffer & buf)
{ writeEscapedString(x, buf); }

template <typename T>
inline std::string toString(const T & x)
{
    WriteBufferFromOwnString buf;
    writeText(x, buf);
    return buf.str();
}

inline void writeQuotedString(const std::string & s, WriteBuffer & buf)
{
    writeAnyQuotedString<'\''>(s, buf);
}

//如果是算术类型
template <typename T>
inline typename std::enable_if<std::is_arithmetic<T>::value, void>::type
writeQuoted(const T & x, WriteBuffer & buf) {
    writeText(x, buf);
}

inline void writeQuoted(const std::string & x,    WriteBuffer & buf) {
    writeQuotedString(x, buf);
}



inline void writeFloatText(double x, WriteBuffer & buf)
{
    DoubleConverter<false>::BufferType buffer;
    double_conversion::StringBuilder builder {buffer, sizeof(buffer)};

    const auto result = DoubleConverter<false>::instance().ToShortest(x, &builder);

    if (!result)
        throw Poco::Exception("Cannot print double number", ErrorCodes::CANNOT_PRINT_FLOAT_OR_DOUBLE_NUMBER);

    buf.write(buffer, builder.position());
}

inline void writeFloatText(float x, WriteBuffer & buf)
{
    DoubleConverter<false>::BufferType buffer;
    double_conversion::StringBuilder builder {buffer, sizeof(buffer)};

    const auto result = DoubleConverter<false>::instance().ToShortestSingle(x, &builder);

    if (!result)
        throw Poco::Exception("Cannot print float number", ErrorCodes::CANNOT_PRINT_FLOAT_OR_DOUBLE_NUMBER);

    buf.write(buffer, builder.position());
}


template <typename T>
static inline std::string formatQuotedWithPrefix(T x, const char * prefix)
{
    WriteBufferFromOwnString wb;
    wb.write(prefix, strlen(prefix));
    writeQuoted(x, wb);
    return wb.str();
}

template <typename T>
inline void writePODBinary(const T & x, WriteBuffer & buf)
{
    buf.write(reinterpret_cast<const char *>(&x), sizeof(x));
}

template <typename T>
inline void writeIntBinary(const T & x, WriteBuffer & buf)
{
    writePODBinary(x, buf);
}

template <typename T>
inline void writeFloatBinary(const T & x, WriteBuffer & buf)
{
    writePODBinary(x, buf);
}

inline void writeStringBinary(const char * s, WriteBuffer & buf)
{
    writeVarUInt(strlen(s), buf);
    buf.write(s, strlen(s));
}

inline void writeStringBinary(const std::string & s, WriteBuffer & buf)
{
    IO::writeVarUInt(s.size(), buf);
    buf.write(s.data(), s.size());
}

template <typename T>
inline typename std::enable_if<std::is_arithmetic<T>::value, void>::type
writeBinary(const T & x, WriteBuffer & buf) { writePODBinary(x, buf); }

inline void writeBinary(const  std::string & x, WriteBuffer & buf) { writeStringBinary(x, buf); }

}


#define writeCString(s, buf) \
    (buf).write((s), strlen(s))
