#pragma once
#include<utility>
#include<memory>

namespace std_ext {

//extend std
template<typename T, typename ... Ts>
std::unique_ptr<T> make_unique(Ts&&... params) {
#if (__cplusplus >= 201402L) || (defined(_MSC_VER) && _MSC_VER >= 1800 )
	return std::make_unique<T>(std::forward<Ts>(params)...);
#else
	return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
#endif
}

}
