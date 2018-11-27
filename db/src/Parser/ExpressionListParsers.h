#pragma once
#include<Parser/IParserBase.h>
#include<memory>
#include<Ext/std_ext.h>
namespace DataBase {

class ListParser:public IParserBase {

public:
    ListParser(std::unique_ptr<IParser> && elem_parser_, std::unique_ptr<IParser>  && separator_parser_, bool allow_empty_ = true)
        : elem_parser(std::move(elem_parser_)), separator_parser(std::move(separator_parser_)), allow_empty(allow_empty_)
    {}

protected:
    const char * getName() const {
        return "list of elements";
    }
    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override;

private:
    std::unique_ptr<IParser>  elem_parser;
    std::unique_ptr<IParser>  separator_parser;
    bool allow_empty;
};

class ExpressionListParser : public IParserBase
{
public:
    ExpressionListParser(bool allow_alias_without_as_keyword_, bool prefer_alias_to_column_name_ = false)
    : allow_alias_without_as_keyword(allow_alias_without_as_keyword_), prefer_alias_to_column_name(prefer_alias_to_column_name_) {}

protected:
    bool allow_alias_without_as_keyword;
    bool prefer_alias_to_column_name;

    const char * getName() const { return "list of expressions"; }
   bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override;
};



class ExpressionWithOptionalAliasParser : public IParserBase
{
public:
    ExpressionWithOptionalAliasParser(bool allow_alias_without_as_keyword, bool prefer_alias_to_column_name_ = false);
protected:
    std::unique_ptr<IParser> impl;

    const char * getName() const {
        return "expression with optional alias";
    }

    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override
    {
        return impl->parse(pos, node, expected);
    }
};


//可变数量的操作符解析,中间操作符
class VariableArityOperatorListParser : public IParserBase
{
private:
    const char * infix;  //插入词
    const char * function_name;  //函数名
    std::unique_ptr<IParser> elem_parser;  //元素解析器

public:
    VariableArityOperatorListParser(const char * infix_, const char * function_,  std::unique_ptr<IParser> && elem_parser_)
        : infix(infix_), function_name(function_), elem_parser(std::move(elem_parser_))
    {
    }

protected:
    const char * getName() const {
        return "list, delimited by operator of variable arity";
    }
    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override;
};

//前缀表达式操作符解析
class PrefixUnaryOperatorExpressionParser : public IParserBase
{
private:
    const char** operators;
    std::unique_ptr<IParser> elem_parser;

public:
    PrefixUnaryOperatorExpressionParser(const char** operators_, std::unique_ptr<IParser> && elem_parser_)
        : operators(operators_), elem_parser(std::move(elem_parser_))
    {
    }

protected:
    const char * getName() const {
        return "expression with prefix unary operator";
    }
    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override;
};

//向左结合二元操作符
class LeftAssociativeBinaryOperatorListParser : public IParserBase
{
private:
    const char** operators;
    std::unique_ptr<IParser> first_elem_parser;
    std::unique_ptr<IParser> remaining_elem_parser;

public:
    /** `operators_` - allowed operators and their corresponding functions
      */
    LeftAssociativeBinaryOperatorListParser(const char** operators_, std::unique_ptr<IParser> && first_elem_parser_)
        : operators(operators_), first_elem_parser(std::move(first_elem_parser_))
    {
    }
    LeftAssociativeBinaryOperatorListParser(const char** operators_, std::unique_ptr<IParser> && first_elem_parser_,
                                            std::unique_ptr<IParser>  && remaining_elem_parser_)
        : operators(operators_), first_elem_parser(std::move(first_elem_parser_)),
          remaining_elem_parser(std::move(remaining_elem_parser_))
    {
    }

protected:
    const char * getName() const {
        return "list, delimited by binary operators";
    }
    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override;
};

class ArrayElementExpressionParser : public IParserBase
{
private:
    static const char * operators[];
protected:
    const char * getName() const {
        return "array element expression";
    }
    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override;
};

class TupleElementExpressionParser : public IParserBase
{
private:
    static const char * operators[];
protected:
    const char * getName() const {
        return "tuple element expression";
    }

    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override;
};

//一元操作符 -
class UnaryMinusExpressionParser : public IParserBase
{
private:
    static const char * operators[];
    PrefixUnaryOperatorExpressionParser operator_parser {operators, std_ext::make_unique<TupleElementExpressionParser>()};

protected:
    const char * getName() const {
        return "unary minus expression";
    }

    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override;
};

class MultiplicativeExpressionParser : public IParserBase
{
private:
    static const char * operators[];
    LeftAssociativeBinaryOperatorListParser operator_parser {operators, std_ext::make_unique<UnaryMinusExpressionParser>()};

protected:
    const char * getName() const {
        return "multiplicative expression";
    }

    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override
    {
        return operator_parser.parse(pos, node, expected);
    }
};

class AdditiveExpressionParser : public IParserBase
{
private:
    static const char * operators[];
    LeftAssociativeBinaryOperatorListParser operator_parser {operators, std_ext::make_unique<MultiplicativeExpressionParser>()};

protected:
    const char * getName() const {
        return "additive expression";
    }

    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override
    {
        return operator_parser.parse(pos, node, expected);
    }
};

//连字符表达式
class ConcatExpressionParser : public IParserBase
{
private:
    VariableArityOperatorListParser operator_parser {"||", "concat", std_ext::make_unique<AdditiveExpressionParser>()};

protected:
    const char * getName() const {
        return "string concatenation expression";
    }

    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override
    {
        return operator_parser.parse(pos, node, expected);
    }
};

//between  and 表达式
class BetweenExpressionParser : public IParserBase
{
private:
    ConcatExpressionParser elem_parser;

protected:
    const char * getName() const {
        return "BETWEEN expression";
    }
    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override;
};


//比较表达式,二元操作
class ComparisonExpressionParser : public IParserBase
{
private:
    static const char * operators[];
    LeftAssociativeBinaryOperatorListParser operator_parser {operators, std_ext::make_unique<BetweenExpressionParser>()};

protected:
    const char * getName() const {
        return "comparison expression";
    }

    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override
    {
        return operator_parser.parse(pos, node, expected);
    }
};

class ParserNullityChecking : public IParserBase
{
protected:
    const char * getName() const override {
        return "nullity checking";
    }
    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override;
};


//逻辑非操作符
class LogicalNotExpressionParser : public IParserBase
{
private:
    static const char * operators[];
    PrefixUnaryOperatorExpressionParser operator_parser {operators,std_ext::make_unique<ParserNullityChecking>()};

protected:
    const char * getName() const {
        return "logical-NOT expression";
    }

    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override
    {
        return operator_parser.parse(pos, node, expected);
    }
};


//逻辑and操作符
class LogicalAndExpressionParser : public IParserBase
{
private:
    VariableArityOperatorListParser operator_parser {"AND", "and", std_ext::make_unique<LogicalNotExpressionParser>()};

protected:
    const char * getName() const {
        return "logical-AND expression";
    }

    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override
    {
        return operator_parser.parse(pos, node, expected);
    }
};


//逻辑操作符 or 解析
class LogicalOrExpressionParser : public IParserBase
{
private:
    VariableArityOperatorListParser operator_parser {"OR", "or", std_ext::make_unique<LogicalAndExpressionParser>()};

protected:
    const char * getName() const {
        return "logical-OR expression";
    }

    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override
    {
        return operator_parser.parse(pos, node, expected);
    }
};

class TernaryOperatorExpressionParser:public IParserBase {
private:
    LogicalOrExpressionParser elem_parser;

protected:
    const char * getName() const {
        return "expression with ternary operator";
    }
    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override;
};


class LambdaExpressionParser : public IParserBase
{
private:
    TernaryOperatorExpressionParser elem_parser;

protected:
    const char * getName() const {
        return "lambda expression";
    }
    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override;
};

}
