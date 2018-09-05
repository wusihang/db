#include<Parser/ParseQuery.h>
#include<Parser/ParserUseQuery.h>
#include<Parser/ParseQueryWithOutput.h>
#include<Parser/ParseInsertQuery.h>


bool DataBase::ParseQuery::parseImpl(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{
    //可添加新的类型解析, 一项项尝试解析,
    //如果所有都解析结果都不符合,那么认为传入的查询字符串是错误的
    DataBase::ParserUseQuery useQuery;
    DataBase::ParseQueryWithOutput queryWithOutput;
    DataBase::ParseInsertQuery  insertQuery(end);
    return queryWithOutput.parse(pos,node,expected)|| insertQuery.parse(pos,node,expected)||
           useQuery.parse(pos,node,expected);
}


