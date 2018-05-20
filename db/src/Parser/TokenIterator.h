#pragma once
#include<Parser/Lexer.h>
#include<vector>


namespace DataBase {

class Tokens {
private:
	//保存已解析的token信息 (不包含空白字符和注释, 如果解析完毕,那么最后一个token类型应该是EndOfStream)
    std::vector<Token> data;
	//词法分析器
    Lexer lexer;

public:
    //tokens的构造是一个字符串的开始和结束指针, 构造当中初始化词法分析
    Tokens(const char* begin , const char* end)
        :lexer(begin,end) {
    }

    //重载[]操作符, 用于获取token列表中对应的token,如果token列表中不存在,那么尝试解析下一个token
    //获取指定index的token时,如果token还未在列表中,那么使用词法分析分析下一个token,并加入列表
    const Token& operator[](size_t index) {
        while (true)
        {
            //如果index小于当前已有的token,那么表示token已经解析过,就直接取出
            if (index < data.size())
                return data[index];

            //如果当前已解析token不为空,并且最后一个token是EndOfStream,那么就直接取出最后一个结束标记的token
            if (!data.empty() && data.back().isEnd())
                return data.back();

            //非以上两种情况时,使用词法分析器分析下一个token
            Token token = lexer.nextToken();
            //如果token是非注释,非空白字符,那么就将token加入到token列表中
            if (token.isSignificant())
                data.emplace_back(token);
        }
    }

    const Token & max()
    {
        //如果当前已解析的token为空,那么就解析token,并返回对应的token
        if (data.empty())
        {
            return (*this)[0];
        }
        //获取最后一个token
        return data.back();
    }

};


class TokenIterator {
private:
    Tokens * tokens;
    size_t index = 0;
public:
    TokenIterator(Tokens & tokens) : tokens(&tokens) {}

    //获取当前迭代的token
    const Token & get() {
        return (*tokens)[index];
    }

    //重载*操作符
    const Token & operator*() {
        return get();
    }

    //重载->操作符
    const Token * operator->() {
        return &get();
    }

    //这个是前缀++ , 而不是后缀++!!
    TokenIterator & operator++() {
        ++index;
        return *this;
    }
    
    //这个是前缀--, 而不是后缀--!!
    TokenIterator & operator--() {
        --index;
        return *this;
    }

    //大小比较实际上是Tokens中,Token位置的比较
    bool operator< (const TokenIterator & rhs) const {
        return index < rhs.index;
    }
    bool operator<= (const TokenIterator & rhs) const {
        return index <= rhs.index;
    }
    bool operator== (const TokenIterator & rhs) const {
        return index == rhs.index;
    }

    //有效token , 在EndOfStream之前的Token都是合法的
    bool isValid() {
        return get().type < TokenType::EndOfStream;
    }

    /// 获取已经分析的token中,最后一个token, 当然,如果已经分析结束,那么最后一个元素应该是EndOfStream
    const Token & max() {
        return tokens->max();
    }
};

//检查未匹配的圆括号
std::vector<Token> checkUnmatchedParentheses(TokenIterator begin, Token * last);

}

