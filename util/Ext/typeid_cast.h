#pragma once

#include <type_traits>
#include <typeinfo>
#include <string>
#include<Poco/Exception.h>

template <typename To, typename From>
typename std::enable_if<std::is_reference<To>::value, To>::type typeid_cast(From & from)
{
    if (typeid(from) == typeid(To))
        return static_cast<To>(from);
    else
        throw Poco::Exception("Bad cast from type " + std::string(typeid(from).name()) + " to " + std::string(typeid(To).name()));
}


template <typename To, typename From>
To typeid_cast(From * from)
{
    if (typeid(*from) == typeid(typename std::remove_pointer<To>::type))
        return static_cast<To>(from);
    else
        return nullptr;
}
