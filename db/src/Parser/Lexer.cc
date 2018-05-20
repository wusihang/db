#include<Parser/Lexer.h>
#include<CommonUtil/FindSymbol.h>
#include<CommonUtil/StringUtils.h>

namespace DataBase {

// 单引号, 双引号
template <char quote, TokenType success_token, TokenType error_token>
Token quotedString(const char *& pos, const char * const token_begin, const char * const end)
{
    ++pos;
    while(true) {
        //找下一个对应的字符,例如是单引号,那么找到单引号或\所在位置
        pos = find_first_symbols<quote,'\\'>(pos,end);
        if (pos >= end)
        {
            return Token(error_token, token_begin, end);
        }
        //如果找到的是对应的quote,那么判断下一个字符是否也是quote, 也就是说(以"号为例) "a""b"  这样属于连字ab
        if (*pos == quote)
        {
            ++pos;
            if (pos < end && *pos == quote)
            {
                ++pos;
                continue;
            }
            return Token(success_token, token_begin, pos);
        }

        //如果找到的是\, 那么就继续搜索
        if (*pos == '\\')
        {
            ++pos;
            if (pos >= end)
            {
                return Token(error_token, token_begin, end);
            }
            ++pos;
            continue;
        }
        __builtin_unreachable();
    }
}

//获取下一个token, 如果获取到的token不为注释或空白字符,那么保存获得的token的类型
Token Lexer::nextToken()
{
    Token res = nextTokenImpl();
    if (res.isSignificant())
        prev_significant_token_type = res.type;
    return res;
}

Token Lexer::nextTokenImpl()
{
    //如果当前位置到达结束,那么就返回EndOfStream类型Token
    if(pos >= end) {
        return Token(TokenType::EndOfStream,end,end);
    }
    const char* const token_begin = pos;

    auto commentUtilEndOfLine = [&]() mutable {
        //含义是,跳过单行注释部分的内容解析
        pos = find_first_symbols<'\n'>(pos,end);
        return Token(TokenType::Comment,token_begin,end);
    };

    //解析token
    switch(*pos) {
        //开头是空白字符
    case ' ':
    case '\t':
    case '\n':
    case '\r':
    case '\f':
    case '\v':
    {
        ++pos;
        //跳过所有空格,所有空格认为是一个token
        while (pos < end && StringUtils::isWhitespaceASCII(*pos))
            ++pos;
        return Token(TokenType::Whitespace, token_begin, pos);
    }

    //开头如果是字符,那么必须以 下划线或字母开头,中间可以包含数字
    case 'a'...'z':
    case 'A'...'Z':
    case '_':
    {
        ++pos;
        while (pos < end && StringUtils::isWordCharASCII(*pos))
            ++pos;
        return Token(TokenType::BareWord, token_begin, pos);
    }
    //如果是数字开头,那么认为是数字
    case '0'...'9': {
        //如果上一个token的类型是.
        if(prev_significant_token_type  == TokenType::Dot)
        {
            ++pos;
            while (pos < end && StringUtils::isNumericASCII(*pos))
            {
                ++pos;
            }
        } else {
            // 解析0x , 0X
            bool hex = false;
            if (pos + 2 < end && *pos == '0' && (pos[1] == 'x' ||  pos[1] == 'X'))
            {
                hex = true;
                pos += 2;
            }
            else
            {
                ++pos;
            }
            //如果是16进制,那么需要判断是否是16进制的字符
            while (pos < end && (hex ? StringUtils::isHexDigit(*pos) : StringUtils::isNumericASCII(*pos)))
            {
                ++pos;
            }
            //如果是. 那么继续解析,数字
            if (pos < end && *pos == '.')
            {
                ++pos;
                while (pos < end && (hex ? StringUtils::isHexDigit(*pos) : StringUtils::isNumericASCII(*pos)))
                {
                    ++pos;
                }
            }

            //指数 , 16进制的指数标记为p或P, 10进制的指数为E或e
            if (pos + 1 < end && (hex ? (*pos == 'p' || *pos == 'P') : (*pos == 'e' || *pos == 'E')))
            {
                ++pos;
                /// 指数部分,可以带有符号位,
                if (pos + 1 < end && (*pos == '-' || *pos == '+'))
                {
                    ++pos;
                }
                //指数部分始终是数字
                while (pos < end && StringUtils::isNumericASCII(*pos))
                {
                    ++pos;
                }
            }
        }

        //这里的判断是为了避免出现 select 123XXX  这样的字符, 数字之后不能立马跟着字符
        if (pos < end && StringUtils::isWordCharASCII(*pos))
        {
            ++pos;
            while (pos < end &&  StringUtils::isWordCharASCII(*pos))
                ++pos;
            //错误格式的数字
            return Token(TokenType::ErrorWrongNumber, token_begin, pos);
        }
        return Token(TokenType::Number, token_begin, pos);
    }

    //单引号,认为是字符串常量
    case '\'':
        return quotedString<'\'', TokenType::StringLiteral, TokenType::ErrorSingleQuoteIsNotClosed>(pos, token_begin, end);
        //双引号和`认为是标识符
    case '"':
        return quotedString<'"', TokenType::QuotedIdentifier, TokenType::ErrorDoubleQuoteIsNotClosed>(pos, token_begin, end);
    case '`':
        return quotedString<'`', TokenType::QuotedIdentifier, TokenType::ErrorBackQuoteIsNotClosed>(pos, token_begin, end);

    case '(':
        return Token(TokenType::OpeningRoundBracket, token_begin, ++pos);
    case ')':
        return Token(TokenType::ClosingRoundBracket, token_begin, ++pos);
    case '[':
        return Token(TokenType::OpeningSquareBracket, token_begin, ++pos);
    case ']':
        return Token(TokenType::ClosingSquareBracket, token_begin, ++pos);
    case ',':
        return Token(TokenType::Comma, token_begin, ++pos);
    case ';':
        return Token(TokenType::Semicolon, token_begin, ++pos);

        //如果是.  那么可能是数字,也有可能是属性解析
    case '.': {
		//如果上一个token类型是 ) 或  ] 或 标识符 或`  或数字 那么token类型标记为Dot
        if (pos > begin && (!(pos + 1 < end &&StringUtils::isNumericASCII(pos[1]))
                            || prev_significant_token_type == TokenType::ClosingRoundBracket
                            || prev_significant_token_type == TokenType::ClosingSquareBracket
                            || prev_significant_token_type == TokenType::BareWord
                            || prev_significant_token_type == TokenType::QuotedIdentifier
                            || prev_significant_token_type == TokenType::Number))
            return Token(TokenType::Dot, token_begin, ++pos);
        ++pos;
        while (pos < end && StringUtils::isNumericASCII(*pos))
        {
            ++pos;
        }

        /// 指数标识符
        if (pos + 1 < end && (*pos == 'e' || *pos == 'E'))
        {
            ++pos;
            /// 指数部分可以包含符号
            if (pos + 1 < end && (*pos == '-' || *pos == '+'))
                ++pos;

            while (pos < end && StringUtils::isNumericASCII(*pos))
            {
                ++pos;
            }
        }
        return Token(TokenType::Number, token_begin, pos);
    }

    case '+':
        return Token(TokenType::Plus, token_begin, ++pos);
    case '-':   /// minus (-), arrow (->) or start of comment (--)
    {
        ++pos;
        if (pos < end && *pos == '>')
            return Token(TokenType::Arrow, token_begin, ++pos);
        if (pos < end && *pos == '-')
        {
            ++pos;
            return commentUtilEndOfLine();
        }
        return Token(TokenType::Minus, token_begin, pos);
    }

    case '*':
        ++pos;
        return Token(TokenType::Asterisk, token_begin, pos);
    case '/':   /// division (/) or start of comment (//, /*)
    {
        ++pos;
        if (pos < end && (*pos == '/' || *pos == '*'))
        {
            //单行注释
            if (*pos == '/')
            {
                ++pos;
                return commentUtilEndOfLine();
            }
            else
            {
                ++pos;
                while (pos + 2 <= end)
                {
                    /// */注释块 , 多行注释,  这里这样写意味着不支持嵌套注释
                    if (pos[0] == '*' && pos[1] == '/')
                    {
                        pos += 2;
                        return Token(TokenType::Comment, token_begin, pos);
                    }
                    ++pos;
                }
                return Token(TokenType::ErrorMultilineCommentIsNotClosed, token_begin, end);
            }
        }
        //除法
        return Token(TokenType::Slash, token_begin, pos);
    }

    case '%':
        return Token(TokenType::Percent, token_begin, ++pos);
    case '=':   /// =, ==
    {
        ++pos;
        if (pos < end && *pos == '=')
            ++pos;
        return Token(TokenType::Equals, token_begin, pos);
    }
    case '!':   /// !=
    {
        ++pos;
        if (pos < end && *pos == '=')
            return Token(TokenType::NotEquals, token_begin, ++pos);
        return Token(TokenType::ErrorSingleExclamationMark, token_begin, pos);
    }
    case '<':   /// <, <=, <>
    {
        ++pos;
        if (pos < end && *pos == '=')
            return Token(TokenType::LessOrEquals, token_begin, ++pos);
        if (pos < end && *pos == '>')
            return Token(TokenType::NotEquals, token_begin, ++pos);
        return Token(TokenType::Less, token_begin, pos);
    }
    case '>':   /// >, >=
    {
        ++pos;
        if (pos < end && *pos == '=')
            return Token(TokenType::GreaterOrEquals, token_begin, ++pos);
        return Token(TokenType::Greater, token_begin, pos);
    }
    case '?':
        return Token(TokenType::QuestionMark, token_begin, ++pos);
    case ':':
        return Token(TokenType::Colon, token_begin, ++pos);
    case '|': // ||连接符
    {
        ++pos;
        if (pos < end && *pos == '|')
            return Token(TokenType::Concatenation, token_begin, ++pos);
        return Token(TokenType::ErrorSinglePipeMark, token_begin, pos);
    }

    default:
        return Token(TokenType::Error,token_begin,++pos);
    }
}


const char* getErrorTokenDescription(TokenType type)
{
    switch (type)
    {
    case TokenType::Error:
        return "Unrecognized token";
    case TokenType::ErrorMultilineCommentIsNotClosed:
        return "Multiline comment is not closed";
    case TokenType::ErrorSingleQuoteIsNotClosed:
        return "Single quoted string is not closed";
    case TokenType::ErrorDoubleQuoteIsNotClosed:
        return "Double quoted string is not closed";
    case TokenType::ErrorBackQuoteIsNotClosed:
        return "Back quoted string is not closed";
    case TokenType::ErrorSingleExclamationMark:
        return "Exclamation mark can only occur in != operator";
    case TokenType::ErrorSinglePipeMark:
        return "Pipe symbol could only occur in || operator";
    case TokenType::ErrorWrongNumber:
        return "Wrong number";
    default:
        return "Not an error";
    }
}


const char* getTokenName(TokenType type)
{
    switch (type)
    {
#define M(TOKEN) \
        case TokenType::TOKEN: return #TOKEN;
        APPLY_FOR_TOKENS(M)
#undef M
    default:
        __builtin_unreachable();
    }
}


}





