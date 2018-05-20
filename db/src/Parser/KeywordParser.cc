#include<Parser/KeywordParser.h>
#include<cstring>
#include<CommonUtil/FindSymbol.h>
#include<Poco/Exception.h>

bool DataBase::KeywordParser::parseImpl(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{
	//关键字解析的token如果不是标识符,那么就肯定不是关键字
    if (pos->type != TokenType::BareWord)
        return false;

	//保存当前字符指针
    const char * current_word = s;

	//计算关键字长度
    size_t s_length = strlen(s);
	
	//关键字不允许为空字符串
    if (!s_length)
        throw Poco::Exception("Logical error: keyword cannot be empty string");

	//字符结尾
    const char * s_end = s + s_length;

    while (true)
    {
		//保存当前关键字信息
        expected.add(pos, current_word);
		//如果字符不是标识符,返回false ==> 后面继续推进
		if (pos->type != TokenType::BareWord)
            return false;

		//查找下一个\0 或 空格
        const char * next_whitespace = find_first_symbols<' ', '\0'>(current_word, s_end);
		//获得当前词到\0或空格的长度
        size_t word_length = next_whitespace - current_word;

		//如果长度不等于当前token的长度 , 那么返回false
        if (word_length != pos->size())
            return false;

		//比较字符(忽略大小写) , 将当前字符和关键字进行比较, 如果不相同,那么返回false
        if (strncasecmp(pos->begin, current_word, word_length))
            return false;

		//推进到下一个token
        ++pos;

		//如果是\0,那么跳出循环
        if (!*next_whitespace)
            break;
		//否则当前字符 = 下一个空白+1, 查找下一个关键字
        current_word = next_whitespace + 1;
    }

    return true;
}
