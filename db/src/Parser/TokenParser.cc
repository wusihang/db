#include<Parser/TokenParser.h>

bool DataBase::TokenParser::parseImpl(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{
    if (pos->type != token_type)
    {
        expected.add(pos, getTokenName(token_type));
        return false;
    }
    ++pos;
    return true;
}
