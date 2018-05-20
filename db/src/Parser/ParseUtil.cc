#include<Parser/ParseUtil.h>
#include<Parser/IAST.h>
#include<Parser/IParser.h>
#include<Parser/TokenIterator.h>
#include<Parser/Lexer.h>
#include<Poco/Exception.h>
#include<IO/WriteBufferFromString.h>
#include<IO/Operators.h>

std::shared_ptr< DataBase::IAST > ParseUtil::parseQuery(DataBase::IParser& parser, const char* begin, const char* end, const std::string& description)
{
    auto pos = begin;
    std::string out_error_message;
	//解析查询语句,如果错误,返回空指针,并且将out_error_message填充为相应的错误信息
    std::shared_ptr< DataBase::IAST > res  = tryParseQuery(parser,pos,end,out_error_message,false,description);
    if(res) {
        return res;
    }
    throw Poco::Exception(out_error_message);
}

std::shared_ptr< DataBase::IAST > ParseUtil::tryParseQuery(DataBase::IParser& parser, const char*& pos, const char* end, std::string& out_error_message, bool hilite, const std::string& description)
{
	//用查询字符串的开始和结束初始化词法分析
	const char* begin = pos;
    DataBase::Tokens tokens(pos,end);
	//tokens迭代器
    DataBase::TokenIterator it(tokens);
	//如果当前token已经指向EndOfStream或者 ; 那么就表示查询结束, 也就是说查询字符串没内容或者只有一个;
    if (it->isEnd()
            || it->type == DataBase::TokenType::Semicolon)
    {
        out_error_message = "Empty query";
        return nullptr;
    }
    
    
    DataBase::Expected expected;
    std::shared_ptr<DataBase::IAST> res;
	//尝试使用传入的解析器解析tokens, 生成语法树
    bool parse_res = parser.parse(it, res, expected);
	//获得解析后的最后一个token
    DataBase::Token last_token = it.max();
	//如果最后一个token是错误的,那么就获取对应错误token的描述信息,并写入out_error_message返回
    if (last_token.isError())
    {
        out_error_message = getLexicalErrorMessage(pos, end, last_token, hilite, description);
        return nullptr;
    }
    if(!parse_res){
		throw Poco::Exception("Unrecognized SQL ===>" + std::string(begin,end));
    }
    return res;
}


std::string ParseUtil::getLexicalErrorMessage(
    const char * begin,
    const char * end,
    DataBase::Token last_token,
    bool hilite,
    const std::string & query_description)
{
    IO::WriteBufferFromOwnString out;
    writeCommonErrorMessage(out, begin, end, last_token, query_description);
    out << DataBase::getErrorTokenDescription(last_token.type);
    return out.str();
}


void ParseUtil::writeCommonErrorMessage(
    IO::WriteBuffer & out,
    const char * begin,
    const char * end,
    DataBase:: Token last_token,
    const std::string & query_description)
{
    out << "Syntax error";
    if (!query_description.empty())
        out << " (" << query_description << ")";

    out << ": failed at position " << (last_token.begin - begin + 1) << " ;ErrorInfo: ";

    if (last_token.type == DataBase::TokenType::EndOfStream || last_token.type ==  DataBase::TokenType::Semicolon)
        out << " (end of query)";

    const char * nl = reinterpret_cast<const char *>(memchr(begin, '\n', end - begin));
    if (nullptr != nl && nl + 1 != end)
    {
        size_t line = 0;
        size_t col = 0;
        std::tie(line, col) = getLineAndCol(begin, last_token.begin);
        out << " (line " << line << ", col " << col << ")";
    }
}


std::pair<size_t, size_t>  ParseUtil::getLineAndCol(const char * begin, const char * pos)
{
    size_t line = 0;
    const char * nl;
    while (nullptr != (nl = reinterpret_cast<const char *>(memchr(begin, '\n', pos - begin))))
    {
        ++line;
        begin = nl + 1;
    }
    /// Lines numbered from 1.
    return { line + 1, pos - begin + 1 };
}
