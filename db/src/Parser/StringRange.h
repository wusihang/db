#pragma once
#include<Parser/TokenIterator.h>

namespace DataBase {
class StringRange {
public:
    StringRange() = default;
    StringRange(const char* begin, const char* end)
        :first(begin),second(end) {  }
    StringRange(TokenIterator& token) : first(token->begin), second(token->end) {}
    StringRange(TokenIterator token_begin, TokenIterator token_end)
    {
        if (token_begin == token_end)
        {
            first = token_begin->begin;
            second = token_begin->begin;
        }
        TokenIterator token_last = token_end;
        --token_last;
        first = token_begin->begin;
        second = token_last->end;
    }
private:
    const char* first;
    const char* second;
};

}
