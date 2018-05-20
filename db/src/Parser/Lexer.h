#pragma once
#include<cstddef>

namespace DataBase {


#define APPLY_FOR_TOKENS(M) \
    M(Whitespace) \
    M(Comment) \
    \
    M(BareWord)               /** Either keyword (SELECT) or identifier (column) */ \
    \
    M(Number)                 /** Always non-negative. No leading plus. 123 or something like 123.456e12, 0x123p12 */ \
    M(StringLiteral)          /** 'hello word', 'hello''word', 'hello\'word\\' */ \
    \
    M(QuotedIdentifier)       /** "x", `x` */ \
    \
    M(OpeningRoundBracket) \
    M(ClosingRoundBracket) \
    \
    M(OpeningSquareBracket) \
    M(ClosingSquareBracket) \
    \
    M(Comma) \
    M(Semicolon) \
    M(Dot)                    /** Compound identifiers, like a.b or tuple access operator a.1, (x, y).2. */ \
                              /** Need to be distinguished from floating point number with omitted integer part: .1 */ \
    \
    M(Asterisk)               /** Could be used as multiplication operator or on it's own: "SELECT *" */ \
    \
    M(Plus) \
    M(Minus) \
    M(Slash) \
    M(Percent) \
    M(Arrow)                  /** ->. Should be distinguished from minus operator. */ \
    M(QuestionMark) \
    M(Colon) \
    M(Equals) \
    M(NotEquals) \
    M(Less) \
    M(Greater) \
    M(LessOrEquals) \
    M(GreaterOrEquals) \
    M(Concatenation)          /** String concatenation operator: || */ \
    \
    /** Order is important. EndOfStream goes after all usual tokens, and special error tokens goes after EndOfStream. */ \
    \
    M(EndOfStream) \
    \
    /** Something unrecognized. */ \
    M(Error) \
    /** Something is wrong and we have more information. */ \
    M(ErrorMultilineCommentIsNotClosed) \
    M(ErrorSingleQuoteIsNotClosed) \
    M(ErrorDoubleQuoteIsNotClosed) \
    M(ErrorBackQuoteIsNotClosed) \
    M(ErrorSingleExclamationMark) \
    M(ErrorSinglePipeMark) \
    M(ErrorWrongNumber) \
 
enum class TokenType
{
#define M(TOKEN) TOKEN,
    APPLY_FOR_TOKENS(M)
#undef M
};

class Token {
public:
    TokenType type;
    const char * begin;
    const char * end;

	//token的长度,比如 select , 那么token长度就是6
    size_t size() const {
        return end - begin;
    }

    Token() = default;
    Token(TokenType type, const char * begin, const char * end) : type(type), begin(begin), end(end) {}

    //非空格和注释
    bool isSignificant() const {
        return type != TokenType::Whitespace && type != TokenType::Comment;
    }
    //上述枚举定义中,大于EndOfStream的枚举值都是错误
    bool isError() const {
        return type > TokenType::EndOfStream;
    }
    //EndOfStream表示token解析到末尾了
    bool isEnd() const {
        return type == TokenType::EndOfStream;
    }
};


//词法分析器
class Lexer
{
public:
	//词法分析器的构造是字符串的开始和结束位置, pos表示当前解析的位置,每分析一个token,pos都会向前推进
    Lexer(const char * begin, const char * end) : begin(begin), pos(begin), end(end) {}
    Token nextToken();

private:
    const char * const begin;
    const char * pos;
    const char * const end;

	//获取下一个token
    Token nextTokenImpl();

	//保存上一个token的类型,用于解析时判断字符存在的二义性
    TokenType prev_significant_token_type = TokenType::Whitespace;
};

//获取对应类型的token名称
const char * getTokenName(TokenType type);

//获取异常token的错误描述信息,不同的错误token有不同的描述
const char * getErrorTokenDescription(TokenType type);

}

