#pragma once
#include <type_traits>
#include<utility>

/** https://svn.boost.org/trac/boost/ticket/5182
  */

template <class T, class Tag>
struct StrongTypedef
{
    using Self = StrongTypedef<T, Tag>;
    T t;

    template <class Enable = typename std::is_copy_constructible<T>::type>
    explicit StrongTypedef(const T & t_) : t(t_) {};
    template <class Enable = typename std::is_move_constructible<T>::type>
    explicit StrongTypedef(T && t_) : t(std::move(t_)) {};

    template <class Enable = typename std::is_default_constructible<T>::type>
    StrongTypedef(): t() {};

    StrongTypedef(const Self &) = default;
    StrongTypedef(Self &&) = default;

    Self & operator=(const Self &) = default;
    Self & operator=(Self &&) = default;

    template <class Enable = typename std::is_copy_assignable<T>::type>
    Self & operator=(const T & rhs) {
        t = rhs;
        return *this;
    }

    template <class Enable = typename std::is_move_assignable<T>::type>
    Self & operator=(T && rhs) {
        t = std::move(rhs);
        return *this;
    }

    operator const T & () const {
        return t;
    }
    operator T & () {
        return t;
    }

    bool operator==(const Self & rhs) const {
        return t == rhs.t;
    }
    bool operator<(const Self & rhs) const {
        return t < rhs.t;
    }

    T & toUnderType() {
        return t;
    }
    const T & toUnderType() const {
        return t;
    }
};



#define STRONG_TYPEDEF(T, D) \
    struct D ## Tag {}; \
    using D = StrongTypedef<T, D ## Tag>; 
