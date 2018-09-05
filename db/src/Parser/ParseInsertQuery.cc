#include<Parser/ParseInsertQuery.h>
#include<Parser/KeywordParser.h>
#include<Parser/TokenParser.h>
#include<Parser/IdentifierParser.h>
#include<Parser/ExpressionListParsers.h>
#include<Parser/ExpressionElementParsers.h>
#include<Ext/std_ext.h>
#include<Parser/ASTInsertQuery.h>
#include<Parser/ASTIdentifier.h>
#include<Ext/typeid_cast.h>
namespace ErrorCodes
{
extern const int SYNTAX_ERROR;
}
namespace DataBase {

bool ParseInsertQuery::parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected)
{
    auto begin = pos;
    KeywordParser s_insert_into("INSERT INTO");
    TokenParser s_dot(TokenType::Dot);
    KeywordParser s_values("VALUES");
    KeywordParser s_format("FORMAT");
    TokenParser s_lparen(TokenType::OpeningRoundBracket);
    TokenParser s_rparen(TokenType::ClosingRoundBracket);
    IdentifierParser name_p;
    ListParser column_p(std_ext::make_unique<IdentifierParser>(), std_ext::make_unique<TokenParser>(TokenType::Comma), false);


    std::shared_ptr< IAST > database;
    std::shared_ptr< IAST > table;
    std::shared_ptr< IAST > columns;
    std::shared_ptr< IAST > format;
    std::shared_ptr< IAST > select;
    const char* data = nullptr;
    if (!s_insert_into.ignore(pos, expected))
        return false;
    if (!name_p.parse(pos, table, expected))
        return false;
    if (s_dot.ignore(pos, expected))
    {
        database = table;
        if (!name_p.parse(pos, table, expected))
            return false;
    }
    /// Is there a list of columns
    if (s_lparen.ignore(pos, expected))
    {
        if (!column_p.parse(pos, columns, expected))
            return false;

        if (!s_rparen.ignore(pos, expected))
            return false;
    }
    if (s_values.ignore(pos, expected))
    {
        data = pos->begin;
    }
    else if (s_format.ignore(pos, expected))
    {
        auto name_pos = pos;
        if (!name_p.parse(pos, format, expected))
            return false;

        if (pos->type == TokenType::Semicolon)
            throw Poco::Exception("You have excessive ';' symbol before data for INSERT.\n"
                                  "Example:\n\n"
                                  "INSERT INTO t (x, y) FORMAT TabSeparated\n"
                                  "1\tHello\n"
                                  "2\tWorld\n"
                                  "\n"
                                  "Note that there is no ';' in first line.", ErrorCodes::SYNTAX_ERROR);

        /// Data starts after the first newline, if there is one, or after all the whitespace characters, otherwise.

        data = name_pos->end;

        while (data < end && (*data == ' ' || *data == '\t' || *data == '\f'))
            ++data;

        if (data < end && *data == '\r')
            ++data;

        if (data < end && *data == '\n')
            ++data;
    } else {
        return false;
    }
    auto query = std::make_shared<ASTInsertQuery>(StringRange(begin, pos));
    node = query;
    if (database)
    {
        query->database = typeid_cast<ASTIdentifier &>(*database).name;
    }
    query->table = typeid_cast<ASTIdentifier &>(*table).name;
    if (format)
    {
        query->format = typeid_cast<ASTIdentifier &>(*format).name;
    }
    query->columns = columns;
    query->data = data != end ? data : nullptr;
    query->end = end;
    if (columns)
    {
        query->children.push_back(columns);
    }
    return true;
}

}
