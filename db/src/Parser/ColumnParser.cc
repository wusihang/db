#include<Parser/ColumnParser.h>
#include<Parser/ExpressionListParsers.h>
#include<Parser/IdentifierParser.h>
#include<Parser/KeywordParser.h>
#include<Parser/TokenParser.h>
#include<Parser/ASTColumnDeclaration.h>
#include<Parser/ASTIdentifier.h>
#include<Ext/std_ext.h>
#include<Ext/typeid_cast.h>
#include<Poco/String.h>

namespace DataBase {

bool ColumnDeclarationListParser::parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected)
{
    return ListParser(std_ext::make_unique<IColumnDeclarationParser<IdentifierParser> >(),std_ext::make_unique<TokenParser>(TokenType::Comma),false).parse(pos,node,expected);
}

//注意,类模板声明的方法,需要在类名称前面加上模板类名<>
template<class NameParser>
bool IColumnDeclarationParser<NameParser>::parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected)
{
    NameParser n_parser;
    IdentifierParser type_parser;
    KeywordParser s_default("DEFAULT");
    KeywordParser s_materialized("MATERIALIZED");
    KeywordParser s_alias("ALIAS");
    const auto begin = pos;
    std::shared_ptr< IAST > name;
    //三元操作符表达式解析,如  (s+2) ? 2*s:  s+1
    TernaryOperatorExpressionParser expr_parser;
    if(!n_parser.parse(pos,name,expected))
    {
        return false;
    }

    std::shared_ptr<IAST> type;
    //解析类型
    type_parser.parse(pos, type, expected);

    //解析 {ALIAS, MATERIALIZED, DEFAULT}
    std::string default_specifier;
    std::shared_ptr<IAST> default_expression;
    TokenIterator pos_before_specifier = pos;
    if (s_default.ignore(pos, expected) ||
            s_materialized.ignore(pos, expected) ||
            s_alias.ignore(pos, expected))
    {
        //表达式转为大写
        default_specifier = Poco::toUpper(std::string (pos_before_specifier->begin, pos_before_specifier->end));
        //默认值之后必须是一个表达式
        /// should be followed by an expression
        if (!expr_parser.parse(pos, default_expression, expected))
        {
            return false;
        }
    }
    //不允许只有列名而没有类型名称
    else if (!type)
    {
        return false;
    }
    const auto column_declaration = std::make_shared<ASTColumnDeclaration>(StringRange(begin,pos));
    node = column_declaration;
    //字段名
    column_declaration->name = typeid_cast<ASTIdentifier&>(*name).name;
    //类型
    if(type) {
        column_declaration->type = type;
        column_declaration->children.push_back(std::move(type));
    }
    //默认值表达式
    if(default_expression)
    {
        column_declaration->default_specifier  = default_specifier;
        column_declaration->default_expression = default_expression;
        column_declaration->children.push_back(std::move(default_expression));
    }
    return true;
}
}
