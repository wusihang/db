#pragma once
#include<utility>
#include<memory>

namespace std_ext {

//extend std
template<typename T, typename ... Ts>
std::unique_ptr<T> make_unique(Ts&&... params) {
    return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
}

}
