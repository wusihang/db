#pragma once
#include<Core/Field.h>
#include<Core/Types.h>

namespace DataBase {

template <typename R = void>
struct StaticVisitor
{
    using ResultType = R;
};

template <typename Visitor, typename F>
typename std::decay<Visitor>::type::ResultType applyVisitor(Visitor && visitor, F && field)
{
    switch (field.getType())
    {
    case Field::Types::Null:
        return visitor(field.template get<Null>());
    case Field::Types::UInt64:
        return visitor(field.template get<UInt64>());
    case Field::Types::Int64:
        return visitor(field.template get<Int64>());
    case Field::Types::Float64:
        return visitor(field.template get<Float64>());
    case Field::Types::String:
        return visitor(field.template get<String>());
    default:
        throw Poco::Exception("Bad type of Field", ErrorCodes::BAD_TYPE_OF_FIELD);
    }
}

/** Prints Field as literal in SQL query */
class FieldVisitorToString : public StaticVisitor<String>
{
public:
    String operator() (const Null & x) const;
    String operator() (const UInt64 & x) const;
    String operator() (const Int64 & x) const;
    String operator() (const Float64 & x) const;
    String operator() (const String & x) const;
};

/** Print readable and unique text dump of field type and value. */
class FieldVisitorDump : public StaticVisitor<String>
{
public:
    String operator() (const Null & x) const;
    String operator() (const UInt64 & x) const;
    String operator() (const Int64 & x) const;
    String operator() (const Float64 & x) const;
    String operator() (const String & x) const;
};

}
