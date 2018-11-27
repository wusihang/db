#include<Parser/ExpressionElementParsers.h>
#include<Parser/ASTIdentifier.h>
#include<Parser/KeywordParser.h>
#include<Ext/typeid_cast.h>
#include<Parser/ExpressionListParsers.h>
#include<Parser/ASTFunction.h>
#include<Parser/ASTLiteral.h>
#include<cstring>
#include<cstdlib>
#include<IO/ReadHelper.h>
#include<Parser/ASTAsterisk.h>
#include<Parser/TokenParser.h>

namespace ErrorCodes {
extern const int LOGICAL_ERROR;
}

bool DataBase::UnsignedIntegerParser::parseImpl(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{
    Field res;

    DataBase::TokenIterator begin = pos;
    if (!pos.isValid())
        return false;

    UInt64 x = 0;
    IO::ReadBufferFromMemory in(pos->begin, pos->size());
    if (!IO::tryReadIntText(x, in) || in.count() != pos->size())
    {
        expected.add(pos, "unsigned integer");
        return false;
    }

    res = x;
    ++pos;
    node = std::make_shared<ASTLiteral>(StringRange(begin, pos), res);
    return true;
}

bool DataBase::NullParser::parseImpl(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{
    DataBase::TokenIterator begin = pos;
    KeywordParser nested_parser("NULL");
    if (nested_parser.parse(pos, node, expected))
    {
        node = std::make_shared<DataBase::ASTLiteral>(StringRange(StringRange(begin, pos)), Null());
        return true;
    }
    else
        return false;
}

bool DataBase::NumberParser::parseImpl(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{
    bool negative = false;
    if (pos->type == TokenType::Minus)
    {
        ++pos;
        negative = true;
    }
    else if (pos->type == TokenType::Plus)  /// Leading plus is simply ignored.
        ++pos;
    Field res;
    DataBase::TokenIterator begin = pos;
    if (!pos.isValid())
        return false;
    /** Maximum length of number. 319 symbols is enough to write maximum double in decimal form.
    * Copy is needed to use strto* functions, which require 0-terminated string.
    */
    static constexpr size_t MAX_LENGTH_OF_NUMBER = 319;
    if (pos->size() > MAX_LENGTH_OF_NUMBER)
    {
        expected.add(pos, "number");
        return false;
    }
    char buf[MAX_LENGTH_OF_NUMBER + 1];

    memcpy(buf, pos->begin, pos->size());
    buf[pos->size()] = 0;

    char * pos_double = buf;
    errno = 0;    /// Functions strto* don't clear errno.
    Float64 float_value = std::strtod(buf, &pos_double);
    if (pos_double != buf + pos->size() || errno == ERANGE)
    {
        expected.add(pos, "number");
        return false;
    }

    if (float_value < 0)
        throw Poco::Exception("Logical error: token number cannot begin with minus, but parsed float number is less than zero.", ErrorCodes::LOGICAL_ERROR);

    if (negative)
        float_value = -float_value;

    res = float_value;

    /// try to use more exact type: UInt64

    char * pos_integer = buf;

    errno = 0;
    UInt64 uint_value = std::strtoull(buf, &pos_integer, 0);
    if (pos_integer == pos_double && errno != ERANGE && (!negative || uint_value <= (1ULL << 63)))
    {
        if (negative)
            res = -static_cast<Int64>(uint_value);
        else
            res = uint_value;
    }

    ++pos;
    node = std::make_shared<ASTLiteral>(StringRange(begin, pos), res);
    return true;
}


bool DataBase::StringLiteralParser::parseImpl(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{
    if (pos->type != TokenType::StringLiteral)
        return false;

    String s;
    IO::ReadBufferFromMemory in(pos->begin, pos->size());

    try
    {
        IO::readQuotedStringWithSQLStyle(s, in);
    }
    catch (const Poco::Exception & e)
    {
        expected.add(pos, "string literal");
        return false;
    }

    if (in.count() != pos->size())
    {
        expected.add(pos, "string literal");
        return false;
    }

    ++pos;
    node = std::make_shared<ASTLiteral>(StringRange(pos->begin, pos->end), s);
    return true;
}

bool DataBase::CompoundIdentifierParser::parseImpl(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{
    DataBase::TokenIterator begin = pos;
    std::shared_ptr< DataBase::IAST > id_list;
    if (!ListParser(std_ext::make_unique<IdentifierParser>(), std_ext::make_unique<TokenParser>(TokenType::Dot), false)
            .parse(pos, id_list, expected))
        return false;

    String name;
    const ASTExpressionList & list = static_cast<const ASTExpressionList &>(*id_list.get());
    for (const auto & child : list.children)
    {
        if (!name.empty())
            name += '.';
        name += static_cast<const ASTIdentifier &>(*child.get()).name;
    }

    node = std::make_shared<ASTIdentifier>(StringRange(begin, pos), name);

    /// In `children`, remember the identifiers-components, if there are more than one.
    if (list.children.size() > 1)
        node->children.insert(node->children.end(), list.children.begin(), list.children.end());

    return true;
}


bool DataBase::LiteralParser::parseImpl(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{
    NullParser null_p;
    NumberParser num_p;
    StringLiteralParser str_p;
    return null_p.parse(pos, node, expected)||num_p.parse(pos, node, expected)||str_p.parse(pos, node, expected);
}

bool DataBase::AsteriskParser::parseImpl(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{
    DataBase::TokenIterator begin = pos;
    if (pos->type == TokenType::Asterisk)
    {
        ++pos;
        node = std::make_shared<ASTAsterisk>(StringRange(begin, pos));
        return true;
    }
    return false;
}



bool DataBase::ExpressionElementParser::parseImpl(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{
    LiteralParser literal_p;
    AsteriskParser asterisk_p;
    CompoundIdentifierParser ci_p;
    return literal_p.parse(pos,node,expected) || asterisk_p.parse(pos,node,expected)||ci_p.parse(pos,node,expected);
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


bool DataBase::FunctionParser::parseImpl(DataBase::TokenIterator& pos, std::shared_ptr< DataBase::IAST >& node, DataBase::Expected& expected)
{
    DataBase::TokenIterator begin = pos;
    IdentifierParser id_parser;
    ExpressionListParser contents(false);
    std::shared_ptr< DataBase::IAST > identifier;
    std::shared_ptr< DataBase::IAST > expr_list_args;
    if (!id_parser.parse(pos, identifier, expected))
        return false;

    if (pos->type != TokenType::OpeningRoundBracket)
        return false;
    ++pos;

    if (!contents.parse(pos, expr_list_args, expected))
        return false;
    if (pos->type != TokenType::ClosingRoundBracket)
        return false;
    ++pos;

    auto function_node = std::make_shared<ASTFunction>(StringRange(begin, pos));
    function_node->name = typeid_cast<ASTIdentifier &>(*identifier).name;
    function_node->arguments = expr_list_args;
    function_node->children.push_back(function_node->arguments);
    node = function_node;
    return true;
}




