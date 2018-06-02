#include<Parser/ParseQueryWithOutput.h>
#include<Parser/ParseCreateQuery.h>
#include<Parser/KeywordParser.h>


bool DataBase::ParseQueryWithOutput::parseImpl(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{
    DataBase::ParserCreateQuery create_query;
    bool res = create_query.parse(pos,node,expected);
    return res;
}
