#pragma once
#include<Parser/IParserBase.h>
#include<Parser/IdentifierParser.h>
namespace DataBase {
	
class UnsignedIntegerParser : public IParserBase
{
protected:
    const char * getName() const {
        return "unsigned integer";
    }
    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST>& node, Expected & expected) override;
};

class NullParser : public IParserBase
{
protected:
    const char * getName() const {
        return "NULL";
    }
    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override;
};

class NumberParser : public IParserBase
{
protected:
    const char * getName() const {
        return "number";
    }
    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override;
};
class StringLiteralParser : public IParserBase
{
protected:
    const char * getName() const {
        return "string literal";
    }
    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override;
};

/** The literal is one of: NULL, UInt64, Int64, Float64, String.
*/
class LiteralParser : public IParserBase
{
protected:
    const char * getName() const {
        return "literal";
    }
    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override;
};

/** The expression element is one of: an expression in parentheses, an array, a literal, a function, an identifier, an asterisk.
  */
class ExpressionElementParser : public IParserBase
{
protected:
    const char * getName() const {
        return "element of expression";
    }
    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override;
};


class AliasParserBase
{
public:
    static const char * restricted_keywords[];
};

class AliasParserImpl : public IParserBase, AliasParserBase
{
public:
    AliasParserImpl(bool allow_alias_without_as_keyword_)
        : allow_alias_without_as_keyword(allow_alias_without_as_keyword_) {}
protected:
    bool allow_alias_without_as_keyword;

    const char * getName() const {
        return "alias";
    }
    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override;
};

template <typename T>
class WithOptionalAliasParserImpl : public IParserBase
{
public:
    WithOptionalAliasParserImpl(std::unique_ptr<IParser>&& elem_parser_, bool allow_alias_without_as_keyword_, bool prefer_alias_to_column_name_ = false)
        : elem_parser(std::move(elem_parser_)), allow_alias_without_as_keyword(allow_alias_without_as_keyword_),
          prefer_alias_to_column_name(prefer_alias_to_column_name_) {}
protected:
    std::unique_ptr<IParser> elem_parser;
    bool allow_alias_without_as_keyword;
    bool prefer_alias_to_column_name;

    const char * getName() const {
        return "element of expression with optional alias";
    }
    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override;
};


//声明外部模板,加快编译
extern template class WithOptionalAliasParserImpl<AliasParserImpl>;
using WithOptionalAliasParser = WithOptionalAliasParserImpl<AliasParserImpl>;

}
