#pragma once

#include <iostream>
#include <Core/Types.h>
#include <IO/ReadBuffer.h>
#include <IO/WriteBuffer.h>

namespace ErrorCodes {
extern const int ATTEMPT_TO_READ_AFTER_EOF;
}

namespace IO
{


/** Write UInt64 in variable length format (base128) NOTE Only up to 2^63 - 1 are supported. */
void writeVarUInt(DataBase::UInt64 x, std::ostream & ostr);
void writeVarUInt(DataBase::UInt64 x, IO::WriteBuffer & ostr);
char * writeVarUInt(DataBase::UInt64 x, char * ostr);


/** Read UInt64, written in variable length format (base128) */
void readVarUInt(DataBase::UInt64 & x, std::istream & istr);
void readVarUInt(DataBase::UInt64 & x, IO::ReadBuffer & istr);
const char * readVarUInt(DataBase::UInt64 & x, const char * istr, size_t size);


/** Get the length of UInt64 in VarUInt format */
size_t getLengthOfVarUInt(DataBase::UInt64 x);

/** Get the Int64 length in VarInt format */
size_t getLengthOfVarInt(DataBase::Int64 x);


/** Write Int64 in variable length format (base128) */
template <typename OUT>
inline void writeVarInt(DataBase::Int64 x, OUT & ostr)
{
    writeVarUInt(static_cast<DataBase::UInt64>((x << 1) ^ (x >> 63)), ostr);
}

inline char * writeVarInt(DataBase::Int64 x, char * ostr)
{
    return writeVarUInt(static_cast<DataBase::UInt64>((x << 1) ^ (x >> 63)), ostr);
}


/** Read Int64, written in variable length format (base128) */
template <typename IN>
inline void readVarInt(DataBase::Int64 & x, IN & istr)
{
    readVarUInt(*reinterpret_cast<DataBase::UInt64*>(&x), istr);
    x = (static_cast<DataBase::UInt64>(x) >> 1) ^ -(x & 1);
}

inline const char * readVarInt(DataBase::Int64 & x, const char * istr, size_t size)
{
    const char * res = readVarUInt(*reinterpret_cast<DataBase::UInt64*>(&x), istr, size);
    x = (static_cast<DataBase::UInt64>(x) >> 1) ^ -(x & 1);
    return res;
}


inline void writeVarT(DataBase::UInt64 x, std::ostream & ostr) {
    writeVarUInt(x, ostr);
}
inline void writeVarT(DataBase::Int64 x, std::ostream & ostr) {
    writeVarInt(x, ostr);
}
inline void writeVarT(DataBase::UInt64 x, IO::WriteBuffer & ostr) {
    writeVarUInt(x, ostr);
}
inline void writeVarT(DataBase::Int64 x, IO::WriteBuffer & ostr) {
    writeVarInt(x, ostr);
}
inline char * writeVarT(DataBase::UInt64 x, char * & ostr) {
    return writeVarUInt(x, ostr);
}
inline char * writeVarT(DataBase::Int64 x, char * & ostr) {
    return writeVarInt(x, ostr);
}

inline void readVarT(DataBase::UInt64 & x, std::istream & istr) {
    readVarUInt(x, istr);
}
inline void readVarT(DataBase::Int64 & x, std::istream & istr) {
    readVarInt(x, istr);
}
inline void readVarT(DataBase::UInt64 & x, IO::ReadBuffer & istr) {
    readVarUInt(x, istr);
}
inline void readVarT(DataBase::Int64 & x, IO::ReadBuffer & istr) {
    readVarInt(x, istr);
}
inline const char * readVarT(DataBase::UInt64 & x, const char * istr, size_t size) {
    return readVarUInt(x, istr, size);
}
inline const char * readVarT(DataBase::Int64 & x, const char * istr, size_t size) {
    return readVarInt(x, istr, size);
}


/// For [U]Int32, [U]Int16.

inline void readVarUInt(DataBase::UInt32 & x, IO::ReadBuffer & istr)
{
    DataBase::UInt64 tmp;
    readVarUInt(tmp, istr);
    x = tmp;
}

inline void readVarInt(DataBase::Int32 & x, IO::ReadBuffer & istr)
{
    DataBase::Int64 tmp;
    readVarInt(tmp, istr);
    x = tmp;
}

inline void readVarUInt(DataBase::UInt16 & x, IO::ReadBuffer & istr)
{
    DataBase::UInt64 tmp;
    readVarUInt(tmp, istr);
    x = tmp;
}

inline void readVarInt(DataBase::Int16 & x, IO::ReadBuffer & istr)
{
    DataBase::Int64 tmp;
    readVarInt(tmp, istr);
    x = tmp;
}


inline void throwReadAfterEOF()
{
    throw Poco::Exception("Attempt to read after eof", ErrorCodes::ATTEMPT_TO_READ_AFTER_EOF);
}

inline void readVarUInt(DataBase::UInt64 & x, IO::ReadBuffer & istr)
{
    x = 0;
    for (size_t i = 0; i < 9; ++i)
    {
        if (istr.eof())
            throwReadAfterEOF();

        DataBase::UInt64 byte = *istr.position();
        ++istr.position();
        x |= (byte & 0x7F) << (7 * i);

        if (!(byte & 0x80))
            return;
    }
}


inline void readVarUInt(DataBase::UInt64 & x, std::istream & istr)
{
    x = 0;
    for (size_t i = 0; i < 9; ++i)
    {
        DataBase:: UInt64 byte = istr.get();
        x |= (byte & 0x7F) << (7 * i);

        if (!(byte & 0x80))
            return;
    }
}

inline const char * readVarUInt(DataBase::UInt64 & x, const char * istr, size_t size)
{
    const char * end = istr + size;

    x = 0;
    for (size_t i = 0; i < 9; ++i)
    {
        if (istr == end)
            throwReadAfterEOF();

        DataBase::UInt64 byte = *istr;
        ++istr;
        x |= (byte & 0x7F) << (7 * i);

        if (!(byte & 0x80))
            return istr;
    }

    return istr;
}


inline void writeVarUInt(DataBase::UInt64 x, IO::WriteBuffer & ostr)
{
    for (size_t i = 0; i < 9; ++i)
    {
        uint8_t byte = x & 0x7F;
        if (x > 0x7F)
            byte |= 0x80;

        ostr.nextIfAtEnd();
        *ostr.position() = byte;
        ++ostr.position();

        x >>= 7;
        if (!x)
            return;
    }
}


inline void writeVarUInt(DataBase::UInt64 x, std::ostream & ostr)
{
    for (size_t i = 0; i < 9; ++i)
    {
        uint8_t byte = x & 0x7F;
        if (x > 0x7F)
            byte |= 0x80;

        ostr.put(byte);

        x >>= 7;
        if (!x)
            return;
    }
}


inline char * writeVarUInt(DataBase::UInt64 x, char * ostr)
{
    for (size_t i = 0; i < 9; ++i)
    {
        uint8_t byte = x & 0x7F;
        if (x > 0x7F)
            byte |= 0x80;

        *ostr = byte;
        ++ostr;

        x >>= 7;
        if (!x)
            return ostr;
    }

    return ostr;
}


inline size_t getLengthOfVarUInt(DataBase::UInt64 x)
{
    return x < (1ULL << 7) ? 1
           : (x < (1ULL << 14) ? 2
              : (x < (1ULL << 21) ? 3
                 : (x < (1ULL << 28) ? 4
                    : (x < (1ULL << 35) ? 5
                       : (x < (1ULL << 42) ? 6
                          : (x < (1ULL << 49) ? 7
                             : (x < (1ULL << 56) ? 8
                                : 9)))))));
}


inline size_t getLengthOfVarInt(DataBase::Int64 x)
{
    return getLengthOfVarUInt(static_cast<DataBase::UInt64>((x << 1) ^ (x >> 63)));
}

}
