#pragma once

#include<Core/Types.h>
#include<IO/WriteBufferHelper.h>
#include<Core/Field.h>
#include<IO/ReadHelper.h>
#include<IO/VarInt.h>

namespace ErrorCodes {
	extern const int TYPE_MISMATCH;
}


namespace DataBase {
template <typename IntType>
struct SettingInt
{
    IntType value;
    bool changed = false;

    SettingInt(IntType x = 0) : value(x) {}

    operator IntType() const {
        return value;
    }
    SettingInt & operator= (IntType x) {
        set(x);
        return *this;
    }

    String toString() const
    {
        return IO::toString(value);
    }

    void set(IntType x)
    {
        value = x;
        changed = true;
    }

    void set(const Field & x)
    {
        set(safeGet<IntType>(x));
    }

    void set(const String & x)
    {
        set(IO::parse<IntType>(x));
    }

    void set(IO::ReadBuffer & buf)
    {
        IntType x = 0;
        IO::readVarT(x, buf);
        set(x);
    }

    void write(IO::WriteBuffer & buf) const
    {
        IO::writeVarT(value, buf);
    }
};

using SettingUInt64 = SettingInt<UInt64>;
using SettingInt64 = SettingInt<Int64>;
using SettingBool = SettingUInt64;



struct SettingFloat
{
    float value;
    bool changed = false;

    SettingFloat(float x = 0) : value(x) {}

    operator float() const { return value; }
    SettingFloat & operator= (float x) { set(x); return *this; }

    String toString() const
    {
        return IO::toString(value);
    }

    void set(float x)
    {
        value = x;
        changed = true;
    }

    void set(const Field & x)
    {
        if (x.getType() == Field::Types::UInt64)
        {
            set(safeGet<UInt64>(x));
        }
        else if (x.getType() == Field::Types::Int64)
        {
            set(safeGet<Int64>(x));
        }
        else if (x.getType() == Field::Types::Float64)
        {
            set(safeGet<Float64>(x));
        }
        else
            throw Poco::Exception(std::string("Bad type of setting. Expected UInt64, Int64 or Float64, got ") + x.getTypeName(), ErrorCodes::TYPE_MISMATCH);
    }

    void set(const String & x)
    {
        set(IO::parse<float>(x));
    }

    void set(IO::ReadBuffer & buf)
    {
        String x;
        IO::readBinary(x, buf);
        set(x);
    }

    void write(IO::WriteBuffer & buf) const
    {
        IO::writeBinary(toString(), buf);
    }
};

struct SettingString
{
    String value;
    bool changed = false;

    SettingString(const String & x = String{}) : value(x) {}

    operator String() const { return value; }
    SettingString & operator= (const String & x) { set(x); return *this; }

    String toString() const
    {
        return value;
    }

    void set(const String & x)
    {
        value = x;
        changed = true;
    }

    void set(const Field & x)
    {
        set(safeGet<const String &>(x));
    }

    void set(IO::ReadBuffer & buf)
    {
        String x;
        IO::readBinary(x, buf);
        set(x);
    }

    void write(IO::WriteBuffer & buf) const
    {
        IO::writeBinary(value, buf);
    }
};
}
