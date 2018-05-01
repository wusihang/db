#pragma once
#include<IO/WriteBufferFromString.h>
#include <Poco/LocalDateTime.h>
#include <Poco/DateTimeFormatter.h>
#include <Ext/likely.h>
#include<string>

#define WRITE_HELPERS_MAX_INT_WIDTH 20U

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

template<typename T>
Poco::UInt32 digits10(T x) {
    if (x < 10ULL)
        return 1;
    if (x < 100ULL)
        return 2;
    if (x < 1000ULL)
        return 3;

    if (x < 1000000000000ULL) {
        if (x < 100000000ULL) {
            if (x < 1000000ULL) {
                if (x < 10000ULL)
                    return 4;
                else
                    return 5 + (x >= 100000ULL);
            }

            return 7 + (x >= 10000000ULL);
        }

        if (x < 10000000000ULL)
            return 9 + (x >= 1000000000ULL);

        return 11 + (x >= 100000000000ULL);
    }
    return 12 + digits10(x / 1000000000000ULL);
}



template<typename T>
Poco::UInt32 writeUIntText(T x, char * dst) {
    static const char digits[201] = "00010203040506070809"
                                    "10111213141516171819"
                                    "20212223242526272829"
                                    "30313233343536373839"
                                    "40414243444546474849"
                                    "50515253545556575859"
                                    "60616263646566676869"
                                    "70717273747576777879"
                                    "80818283848586878889"
                                    "90919293949596979899";
    const Poco::UInt32 length = digits10(x);
    Poco::UInt32 next = length - 1;

    while (x >= 100) {
        const Poco::UInt32 i = (x % 100) * 2;
        x /= 100;
        dst[next] = digits[i + 1];
        dst[next - 1] = digits[i];
        next -= 2;
    }
    if (x < 10) {
        dst[next] = '0' + x;
    } else {
        const Poco::UInt32 i = x * 2;
        dst[next] = digits[i + 1];
        dst[next - 1] = digits[i];
    }
    return length;
}



template<typename T>
void writeUIntTextFallback(T x, WriteBuffer & buf) {
    if (x == 0) {
        buf.nextIfAtEnd();
        *buf.position() = '0';
        ++buf.position();

        return;
    }
    char tmp[WRITE_HELPERS_MAX_INT_WIDTH];
    char * pos;
    for (pos = tmp + WRITE_HELPERS_MAX_INT_WIDTH - 1; x != 0; --pos) {
        *pos = '0' + x % 10;
        x /= 10;
    }
    ++pos;
    buf.write(pos, tmp + WRITE_HELPERS_MAX_INT_WIDTH - pos);
}

template<typename T>
void writeUIntText(T x, WriteBuffer & buf) {
    if (likely(buf.position() + WRITE_HELPERS_MAX_INT_WIDTH < buf.buffer().end())) {
        buf.position() += writeUIntText(x, buf.position());
    } else {
        writeUIntTextFallback(x, buf);
    }
}


inline void writeString(const std::string& s, WriteBuffer & buf) {
    buf.write(s.data(), s.size());
}

}


#define writeCString(s, buf) \
    (buf).write((s), strlen(s))
