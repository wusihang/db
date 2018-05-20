#include<Parser/ParseQuery.h>
#include<Parser/ParserUseQuery.h>


bool DataBase::ParseQuery::parseImpl(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{
    //这里当前只实现了use查询解析,
    //之后可以添加新的类型解析, 一项项尝试解析,
    //如果所有都解析结果都不符合,那么认为传入的查询字符串是错误的
    DataBase::ParserUseQuery useQuery;
    return useQuery.parse(pos,node,expected);
}
