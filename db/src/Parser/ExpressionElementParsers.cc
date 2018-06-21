#include<Parser/ExpressionElementParsers.h>
#include<Parser/ASTIdentifier.h>
#include<Parser/KeywordParser.h>
#include<Ext/typeid_cast.h>
#include<cstring>

bool DataBase::UnsignedIntegerParser::parseImpl(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{
    return false;
}

bool DataBase::NullParser::parseImpl(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{
    return  false;
}

bool DataBase::NumberParser::parseImpl(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{
    return true;
}


bool DataBase::StringLiteralParser::parseImpl(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{
    return false;
}

bool DataBase::LiteralParser::parseImpl(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{
    NullParser null_p;
    NumberParser num_p;
    StringLiteralParser str_p;

    if (null_p.parse(pos, node, expected))
        return true;
    if (num_p.parse(pos, node, expected))
        return true;
    if (str_p.parse(pos, node, expected))
        return true;
    return false;
}


bool DataBase::ExpressionElementParser::parseImpl(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{
    return false;
}


//显示实例化该模板
template class DataBase::WithOptionalAliasParserImpl<DataBase::AliasParserImpl>; 
template<typename T>
bool DataBase::WithOptionalAliasParserImpl<T>::parseImpl(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{
    if(!elem_parser->parse(pos,node,expected)) {
        return false;
    }

    bool allow_alias_without_as_keyword_now = allow_alias_without_as_keyword;
    if (allow_alias_without_as_keyword)
        if (const ASTIdentifier * id = typeid_cast<const ASTIdentifier *>(node.get()))
            if (0 == strcasecmp(id->name.data(), "FROM"))
                allow_alias_without_as_keyword_now = false;

    std::shared_ptr<IAST> alias_node;
    if (T(allow_alias_without_as_keyword_now).parse(pos, alias_node, expected))
    {
        std::string alias_name = typeid_cast<ASTIdentifier &>(*alias_node).name;

        if (ASTWithAlias * ast_with_alias = dynamic_cast<ASTWithAlias *>(node.get()))
        {
            ast_with_alias->alias = alias_name;
            ast_with_alias->prefer_alias_to_column_name = prefer_alias_to_column_name;
        }
        else
        {
            expected.add(pos, "alias cannot be here");
            return false;
        }
    }
    return true;
}

const char * DataBase::AliasParserBase::restricted_keywords[] =
{
    "FROM",
    "FINAL",
    "SAMPLE",
    "ARRAY",
    "LEFT",
    "RIGHT",
    "INNER",
    "FULL",
    "CROSS",
    "JOIN",
    "GLOBAL",
    "ANY",
    "ALL",
    "ON",
    "USING",
    "PREWHERE",
    "WHERE",
    "GROUP",
    "WITH",
    "HAVING",
    "ORDER",
    "LIMIT",
    "SETTINGS",
    "FORMAT",
    "UNION",
    "INTO",
    nullptr
};

bool DataBase::AliasParserImpl::parseImpl(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{

    KeywordParser s_as("AS");
    IdentifierParser id_p;

    bool has_as_word = s_as.parse(pos, node, expected);
    if (!allow_alias_without_as_keyword && !has_as_word)
        return false;

    if (!id_p.parse(pos, node, expected))
        return false;

    if (!has_as_word)
    {
        /** In this case, the alias can not match the keyword -
          *  so that in the query "SELECT x FROM t", the word FROM was not considered an alias,
          *  and in the query "SELECT x FRO FROM t", the word FRO was considered an alias.
          */
        const std::string & name = static_cast<const ASTIdentifier &>(*node.get()).name;

        for (const char ** keyword = restricted_keywords; *keyword != nullptr; ++keyword)
            if (0 == strcasecmp(name.data(), *keyword))
                return false;
    }
    return true;
}



