#include<Parser/IParserBase.h>

bool DataBase::IParserBase::parse(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{
	//保留当前位置, 用于解析失败时将pos重置
    DataBase::TokenIterator begin  = pos;
	//expected中添加解析器的名称
    expected.add(begin,getName());
	//尝试解析
    bool res = parseImpl(pos,node,expected);
    if (!res)
    {
		//解析失败后,语法树清空, pos指向解析前的位置
        node = nullptr;
        pos = begin;
    }
    return res;
}
