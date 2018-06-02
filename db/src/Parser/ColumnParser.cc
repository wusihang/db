#include<Parser/ColumnParser.h>
#include<Parser/ExpressionListParsers.h>
#include<Parser/IdentifierParser.h>
#include<Parser/TokenParser.h>
#include<Parser/ASTColumnDeclaration.h>
#include<Parser/ASTIdentifier.h>
#include<Ext/std_ext.h>
#include<Ext/typeid_cast.h>

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
    const auto begin = pos;
    std::shared_ptr< IAST > name;
    if(!n_parser.parse(pos,name,expected))
    {
        return false;
    }

    std::shared_ptr<IAST> type;
    if(!type_parser.parse(pos,type,expected)) {
        return false;
    }
    const auto column_declaration = std::make_shared<ASTColumnDeclaration>(StringRange(begin,pos));
    node = column_declaration;
    column_declaration->name = typeid_cast<ASTIdentifier&>(*name).name;
    if(type) {
        column_declaration->type = type;
        column_declaration->children.push_back(std::move(type));
    }
    return true;
}
}
