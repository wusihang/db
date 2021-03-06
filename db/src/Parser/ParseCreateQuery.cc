#include<Parser/ParseCreateQuery.h>
#include<Parser/KeywordParser.h>
#include<Parser/TokenParser.h>
#include<Parser/IdentifierParser.h>
#include<Parser/ASTCreateQuery.h>
#include<Parser/ASTIdentifier.h>
#include<Parser/ColumnParser.h>
#include<Ext/typeid_cast.h>
#include<Parser/ASTFunction.h>
#include<Parser/ExpressionElementParsers.h>

namespace DataBase {
bool ParserCreateQuery::parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected)
{
    TokenIterator begin = pos;
    KeywordParser  key_create("CREATE");
    KeywordParser  key_database("DATABASE");
    KeywordParser key_table("TABLE");
    //左括号
    TokenParser s_lparen(TokenType::OpeningRoundBracket);
    //右括号
    TokenParser s_rparen(TokenType::ClosingRoundBracket);
    TokenParser s_dot(TokenType::Dot);

    //表名或库名标识符
    IdentifierParser name_parser;
    //字段列表声明
    ColumnDeclarationListParser column_list_parser;

    ParserEngine engine_parser;

    //以下是创建语句必要的语法结构
    std::shared_ptr<IAST> database;
    std::shared_ptr<IAST> table;
    std::shared_ptr<IAST> columns;
    std::shared_ptr<IAST> storage;
    if(!key_create.ignore(pos,expected))
    {
        return false;
    }
    if(key_database.ignore(pos,expected)) {
        if(!name_parser.parse(pos,database,expected)) {
            {
                return false;
            }
        }
    } else if(key_table.ignore(pos,expected)) {
        if (!name_parser.parse(pos, table, expected))
        {
            return false;
        }
        //如果是库名.表名的形式,那么需要进一步处理
        if(s_dot.ignore(pos,expected)) {
            database = table;
            if(!name_parser.parse(pos,table,expected)) {
                return false;
            }
        }
        //匹配左括号
        if(s_lparen.ignore(pos,expected)) {
            if(!column_list_parser.parse(pos,columns,expected)) {
                return false;
            }
            if(!s_rparen.ignore(pos,expected)) {
                return false;
            }
        }
        engine_parser.parse(pos,storage,expected);
    }

    auto query = std::make_shared<ASTCreateQuery>(StringRange(begin, pos));
    node = query;
    if(database) {
        query->database = typeid_cast<ASTIdentifier &>(*database).name;
    }
    if(table) {
        query->table  = typeid_cast<ASTIdentifier&>(*table).name;
    }
    if(columns) {
        query->columns = columns;
        query->children.push_back(columns);
    }
    query->storage = storage;
    if(storage) {
        query->children.push_back(storage);
    }
    return true;
}

bool ParserEngine::parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected)
{
    KeywordParser s_engine("ENGINE");
    TokenParser s_eq(TokenType::Equals);
    ParserIdentifierWithOptionalParameters storage_p;

    if (s_engine.ignore(pos, expected))
    {
        if (!s_eq.ignore(pos, expected))
            return false;

        if (!storage_p.parse(pos, node, expected))
            return false;
    }
    return true;
}

bool ParserIdentifierWithOptionalParameters::parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected)
{
    IdentifierParser non_parametric;
    ParserIdentifierWithParameters parametric;
    TokenIterator begin = pos;
    if (parametric.parse(pos, node, expected))
        return true;

    std::shared_ptr< IAST > ident;
    if (non_parametric.parse(pos, ident, expected))
    {
        auto func = std::make_shared<ASTFunction>(StringRange(begin));
        func->name = typeid_cast<ASTIdentifier &>(*ident).name;
        node = func;
        return true;
    }

    return false;
}

bool ParserIdentifierWithParameters::parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected)
{
    FunctionParser function_or_array;
    return function_or_array.parse(pos, node, expected);
}


}

