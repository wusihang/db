#include<Parser/IdentifierParser.h>
#include<IO/ReadBufferFromMemory.h>
#include<Parser/ASTIdentifier.h>
#include<IO/ReadHelper.h>


bool DataBase::IdentifierParser::parseImpl(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{
    DataBase::TokenIterator begin = pos;

    /// 标识符为``或""包含的内容
    if (pos->type == TokenType::QuotedIdentifier)
    {
        IO::ReadBufferFromMemory buf(pos->begin, pos->size());
        std::string s;

        if (*pos->begin == '`')
            IO::readBackQuotedStringWithSQLStyle(s, buf);
        else
            IO::readDoubleQuotedStringWithSQLStyle(s, buf);

        //不允许空字符串的标识符
        if (s.empty())
            return false;
        ++pos;
        node = std::make_shared<ASTIdentifier>(StringRange(begin), s);
        return true;
    }
    else if (pos->type == TokenType::BareWord)
    {
        ++pos;
        node = std::make_shared<ASTIdentifier>(StringRange(begin), std::string(begin->begin, begin->end));
        return true;
    }
    return false;
}

