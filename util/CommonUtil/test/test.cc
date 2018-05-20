#include<CommonUtil/FindSymbol.h>
#include<iostream>
#include<cstring>

int main() {
    const char* constChar = "abcdefghijklmnop";
    const char* result = find_first_symbols<'o','h','p','m','n','f'>(constChar, constChar + strlen(constChar));
	std::cout<< __builtin_ctz(0x789) << std::endl;
    std::cout << result << std::endl;
    return 0;
}
