#include<Core/FieldVisitor.h>
#include<IO/WriteBufferHelper.h>

namespace DataBase {

template <typename T>
static inline String formatQuoted(T x)
{
    IO::WriteBufferFromOwnString wb;
    IO::writeQuoted(x, wb);
    return wb.str();
}

static inline String formatFloat(Float64 x)
{
    IO::WriteBufferFromOwnString wb;
    IO::writeFloatText(x, wb);
    return wb.str();
}



String FieldVisitorToString::operator()(const String& x) const
{
    return formatQuoted(x);
}

String FieldVisitorToString::operator()(const Float64& x) const
{
    return formatFloat(x);
}

String FieldVisitorToString::operator()(const Int64& x) const
{
    return formatQuoted(x);
}
String FieldVisitorToString::operator()(const UInt64& x) const
{
    return formatQuoted(x);
}

String FieldVisitorToString::operator()(const Null& x) const
{
    return "NULL";
}

}
