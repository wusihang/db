#pragma once

namespace ext {
template <typename T>
class singleton
{
public:
    static T & instance()
    {
        /// C++11 has thread safe statics. GCC and Clang have thread safe statics by default even before C++11.
        static T instance;
        return instance;
    }
protected:
    singleton() {};
private:
    singleton(const singleton &);
    singleton & operator=(const singleton &);
};

}
