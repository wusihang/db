#pragma once


namespace ext {

class noncopyable {
protected:
    constexpr noncopyable() = default;
    ~noncopyable() = default;
    noncopyable( const noncopyable& ) = delete;
    noncopyable& operator=( const noncopyable& ) = delete;
};

}
