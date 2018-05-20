#pragma once
#include<IO/WriteBufferFromString.h>
#include <Poco/LocalDateTime.h>
#include <Poco/DateTimeFormatter.h>
#include <Ext/likely.h>
#include<IO/WriteIntText.h>
#include<string>


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


inline void writeString(const std::string& s, WriteBuffer & buf) {
    buf.write(s.data(), s.size());
}

inline void writeString(const char * data, size_t size, WriteBuffer & buf)
{
    buf.write(data, size);
}


template <typename T>
inline typename std::enable_if<std::is_integral<T>::value, void>::type
writeText(const T & x, WriteBuffer & buf) {
    writeIntText(x, buf);
}

template <typename T>
inline std::string toString(const T & x)
{
    WriteBufferFromOwnString buf;
    writeText(x, buf);
    return buf.str();
}

}


#define writeCString(s, buf) \
    (buf).write((s), strlen(s))
