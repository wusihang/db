#pragma once
#include <string>
#include <cstring>
#include <cstddef>

namespace detail {
bool startsWith(const std::string& s, const char* prefix, size_t prefix_size);
bool endsWith(const std::string& s, const char* suffix, size_t suffix_size);
}

namespace StringUtils {

inline bool startsWith(const std::string& s, const std::string& prefix) {
    return detail::startsWith(s, prefix.data() , prefix.size());
}
inline bool endsWith(const std::string& s, const std::string& suffix) {
    return detail::endsWith(s, suffix.data(), suffix.size());
}


//对于GCC来说，如果传递的string是一个常量，那么strlen会在编译期间计算完成
inline bool startsWith(const std::string& s, const char* prefix) {
    return detail::startsWith(s, prefix, strlen(prefix));
}
inline bool endsWith(const std::string& s, const char* suffix) {
    return detail::endsWith(s, suffix, strlen(suffix));
}

inline bool isWhitespaceASCII(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}


inline bool isAlphaASCII(char c)
{
    return (c >= 'a' && c <= 'z')
           || (c >= 'A' && c <= 'Z');
}

inline bool isNumericASCII(char c)
{
    return (c >= '0' && c <= '9');
}

inline bool isWordCharASCII(char c)
{
    return c == '_' || isAlphaASCII(c) || isNumericASCII(c);
}

inline bool isHexDigit(char c) {
    return (c>='a'&&c<='f') || (c>='A'&&c<='Z') || isNumericASCII(c);
}


}
