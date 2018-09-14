#include<Parser/ExpressionListParsers.h>
#include<Parser/ASTExpressionList.h>
#include<CommonUtil/StringUtils.h>
#include<Parser/KeywordParser.h>
#include<Parser/ASTFunction.h>
#include<Parser/TokenParser.h>
#include<Parser/ExpressionElementParsers.h>
#include<Ext/std_ext.h>

namespace DataBase {

//解析操作符
static bool parseOperator(TokenIterator & pos, const char * op, Expected & expected)
{
    //如果是ASCII字符,那么直接跳过该关键字
    if (StringUtils::isWordCharASCII(*op))
    {
        return KeywordParser(op).ignore(pos, expected);
    }
    else
    {
        //如果当前token的长度和op相同,并且完全匹配,那么就返回true
        if (strlen(op) == pos->size() && 0 == memcmp(op, pos->begin, pos->size()))
        {
            ++pos;
            return true;
        }
        return false;
    }
}

const char * MultiplicativeExpressionParser::operators[] =
{
    "*",     "multiply",
    "/",     "divide",
    "%",     "modulo",
    nullptr
};

const char * UnaryMinusExpressionParser::operators[] =
{
    "-",     "negate",
    nullptr
};

const char * AdditiveExpressionParser::operators[] =
{
    "+",     "plus",
    "-",     "minus",
    nullptr
};

const char * ComparisonExpressionParser::operators[] =
{
    "==",            "equals",
    "!=",            "notEquals",
    "<>",            "notEquals",
    "<=",            "lessOrEquals",
    ">=",            "greaterOrEquals",
    "<",             "less",
    ">",             "greater",
    "=",             "equals",
    "LIKE",          "like",
    "NOT LIKE",      "notLike",
    "IN",            "in",
    "NOT IN",        "notIn",
    "GLOBAL IN",     "globalIn",
    "GLOBAL NOT IN", "globalNotIn",
    nullptr
};

const char * LogicalNotExpressionParser::operators[] =
{
    "NOT", "not",
    nullptr
};

const char* TupleElementExpressionParser::operators[]= {
    ".", "tupleElement",
    nullptr
};

const char* ArrayElementExpressionParser::operators[]= {
    "[", "arrayElement",
    nullptr
};


bool ListParser::parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected)
{
    bool first = true;
    auto list = std::make_shared<ASTExpressionList>();
    node = list;
    while (1)
    {
        if (first)
        {
            std::shared_ptr< IAST > elem;
            if (!elem_parser->parse(pos, elem, expected))
                break;
            list->children.push_back(elem);
            first = false;
        }
        else
        {
            TokenIterator prev_pos = pos;
            if (!separator_parser->ignore(pos, expected))
                break;
            std::shared_ptr< IAST > elem;
            if (!elem_parser->parse(pos, elem, expected))
            {
                pos = prev_pos;
                break;
            }
            list->children.push_back(elem);
        }
    }
    return (allow_empty || !first);
}

bool ExpressionListParser::parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected)
{
    return ListParser(
               std_ext::make_unique<ExpressionWithOptionalAliasParser>(allow_alias_without_as_keyword, prefer_alias_to_column_name),
               std_ext::make_unique<TokenParser>(TokenType::Comma))
           .parse(pos, node, expected);
}


//三元表达式  (condtion) ? result1 : result2s
bool TernaryOperatorExpressionParser::parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected)
{
    TokenParser symbol1(TokenType::QuestionMark);
    TokenParser symbol2(TokenType::Colon);
    std::shared_ptr< IAST > elem_cond;
    std::shared_ptr< IAST > elem_then;
    std::shared_ptr< IAST > elem_else;
    DataBase::TokenIterator begin = pos;
    //逻辑判断部分解析
    if (!elem_parser.parse(pos, elem_cond, expected))
        return false;

    //问号解析,如果没有问号,那就是个表达式
    if (!symbol1.ignore(pos, expected))
    {
        node = elem_cond;
    }
    else
    {
        //then部分解析
        if (!elem_parser.parse(pos, elem_then, expected))
            return false;
        //冒号
        if (!symbol2.ignore(pos, expected))
            return false;
        //else部分
        if (!elem_parser.parse(pos, elem_else, expected))
            return false;

        //三元表达式的行为和if函数一致
        /// the function corresponding to the operator
        auto function = std::make_shared<ASTFunction>();
        /// function arguments
        auto exp_list = std::make_shared<ASTExpressionList>();
        function->range.first = begin->begin;
        function->range.second = pos->begin;
        function->name = "if";
        function->arguments = exp_list;
        function->children.push_back(exp_list);
        exp_list->children.push_back(elem_cond);
        exp_list->children.push_back(elem_then);
        exp_list->children.push_back(elem_else);
        exp_list->range.first = begin->begin;
        exp_list->range.second = pos->begin;
        node = function;
    }
    return true;
}

//可变数量的操作符解析
//内容注释以 or为例
bool VariableArityOperatorListParser::parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected)
{
    DataBase::TokenIterator begin = pos;
    std::shared_ptr<IAST> arguments;
    //or表达式传入的内容为and表达式解析,先解析and部分
    if(!elem_parser->parse(pos,node,expected)) {
        return false;
    }
    while(1) {
        //当表达式为or时, 插入词 infix = or   , 也就是解析or关键字
        if(!parseOperator(pos,infix,expected)) {
            break;
        }
        //第一次循环时, 用传入的函数名构建ASTFunction语法树
        if(!arguments) {
            node = makeASTFunction(function_name,node);
            arguments = static_cast<ASTFunction &>(*node).arguments;
        }
        std::shared_ptr<IAST> elem;
        //解析连接词的另一部分
        if (!elem_parser->parse(pos, elem, expected))
        {
            return false;
        }
        arguments->children.push_back(elem);
    }
    if (arguments)
    {
        arguments->range = node->range = StringRange(begin, pos);
    }
    return true;
}

//前缀表达式
bool PrefixUnaryOperatorExpressionParser::parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected)
{
    DataBase::TokenIterator begin = pos;
    const char ** it;
    for (it = operators; *it; it += 2)
    {
        if (parseOperator(pos, *it, expected))
        {
            break;
        }
    }

    if (it[0] && 0 == strncmp(it[0], "NOT", 3))
    {
        /// Was there an even number of NOTs.
        bool even = false;
        const char ** jt;
        while (true)
        {
            for (jt = operators; *jt; jt += 2)
                if (parseOperator(pos, *jt, expected))
                {
                    break;
                }

            if (!*jt)
            {
                break;
            }
            even = !even;
        }

        if (even)
        {
            it = jt;   /// Zero the result of parsing the first NOT. It turns out, as if there is no `NOT` chain at all.
        }
    }

    std::shared_ptr<IAST> elem;
    if (!elem_parser->parse(pos, elem, expected))
    {
        return false;
    }

    if (!*it)
    {
        node = elem;
    }
    else
    {
        /// the function corresponding to the operator
        auto function = std::make_shared<ASTFunction>();
        /// function arguments
        auto exp_list = std::make_shared<ASTExpressionList>();
        function->range.first = begin->begin;
        function->range.second = pos->begin;
        function->name = it[1];
        function->arguments = exp_list;
        function->children.push_back(exp_list);
        exp_list->children.push_back(elem);
        exp_list->range.first = begin->begin;
        exp_list->range.second = pos->begin;
        node = function;
    }
    return true;
}

bool LeftAssociativeBinaryOperatorListParser::parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected)
{
    bool first = true;
    DataBase::TokenIterator begin = pos;
    while (1)
    {
        //首次解左侧表达式
        if (first)
        {
            std::shared_ptr< IAST > elem;
            if (!first_elem_parser->parse(pos, elem, expected))
                return false;
            node = elem;
            first = false;
        }
        else
        {
            /// try to find any of the valid operators
            const char ** it;
            for (it = operators; *it; it += 2)
                if (parseOperator(pos, *it, expected))
                    break;
            if (!*it)
                break;

            /// the function corresponding to the operator
            auto function = std::make_shared<ASTFunction>();
            /// function arguments
            auto exp_list = std::make_shared<ASTExpressionList>();

            std::shared_ptr< IAST > elem;
            //如果指定了右侧表达式解析器,那么就使用,否则认为左右两侧解析规则是一样的
            if (!(remaining_elem_parser ? remaining_elem_parser : first_elem_parser)->parse(pos, elem, expected))
                return false;

            /// the first argument of the function is the previous element, the second is the next one
            function->range.first = begin->begin;
            function->range.second = pos->begin;
            function->name = it[1];
            function->arguments = exp_list;
            function->children.push_back(exp_list);

            exp_list->children.push_back(node);
            exp_list->children.push_back(elem);
            exp_list->range.first = begin->begin;
            exp_list->range.second = pos->begin;

            /** special exception for the access operator to the element of the array `x[y]`, which
              * contains the infix part '[' and the suffix ''] '(specified as' [')
              */
            if (0 == strcmp(it[0], "["))
            {
                if (pos->type != TokenType::ClosingSquareBracket)
                    return false;
                ++pos;
            }
            node = function;
        }
    }
    return true;
}


bool ParserNullityChecking::parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected)
{
    std::shared_ptr< IAST > node_comp;
    if (!ComparisonExpressionParser().parse(pos, node_comp, expected))
    {
        return false;
    }
    DataBase::TokenIterator begin = pos;
    KeywordParser s_is("IS");
    KeywordParser s_not("NOT");
    KeywordParser s_null("NULL");
    //跳过is
    if (s_is.ignore(pos, expected))
    {
        bool is_not = false;
        //解析not
        if (s_not.ignore(pos, expected))
            is_not = true;
        //解析null
        if (!s_null.ignore(pos, expected))
            return false;

        auto args = std::make_shared<ASTExpressionList>();
        args->children.push_back(node_comp);

        auto function = std::make_shared<ASTFunction>(StringRange {begin, pos});
        function->name = is_not ? "isNotNull" : "isNull";
        function->arguments = args;
        function->children.push_back(function->arguments);
        node = function;
    }
    else
    {
        node = node_comp;
    }
    return true;
}

bool BetweenExpressionParser::parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected)
{
    KeywordParser s_between("BETWEEN");
    KeywordParser s_and("AND");
    //比较对象
    std::shared_ptr< IAST > subject;
    std::shared_ptr< IAST > left;
    std::shared_ptr< IAST > right;
    DataBase::TokenIterator begin = pos;
    if (!elem_parser.parse(pos, subject, expected))
        return false;
    if (!s_between.ignore(pos, expected))
        node = subject;
    else
    {
        if (!elem_parser.parse(pos, left, expected))
            return false;

        if (!s_and.ignore(pos, expected))
            return false;

        if (!elem_parser.parse(pos, right, expected))
            return false;

        /// AND function
        auto f_and = std::make_shared<ASTFunction>();
        auto args_and = std::make_shared<ASTExpressionList>();

        /// >=
        auto f_ge = std::make_shared<ASTFunction>();
        auto args_ge = std::make_shared<ASTExpressionList>();

        /// <=
        auto f_le = std::make_shared<ASTFunction>();
        auto args_le = std::make_shared<ASTExpressionList>();

        args_ge->children.emplace_back(subject);
        args_ge->children.emplace_back(left);

        args_le->children.emplace_back(subject);
        args_le->children.emplace_back(right);

        f_ge->range.first = begin->begin;
        f_ge->range.second = pos->begin;
        f_ge->name = "greaterOrEquals";
        f_ge->arguments = args_ge;
        f_ge->children.emplace_back(f_ge->arguments);

        f_le->range.first = begin->begin;
        f_le->range.second = pos->begin;
        f_le->name = "lessOrEquals";
        f_le->arguments = args_le;
        f_le->children.emplace_back(f_le->arguments);

        args_and->children.emplace_back(f_ge);
        args_and->children.emplace_back(f_le);

        f_and->range.first = begin->begin;
        f_and->range.second = pos->begin;
        f_and->name = "and";
        f_and->arguments = args_and;
        f_and->children.emplace_back(f_and->arguments);
        node = f_and;
    }
    return true;
}

bool UnaryMinusExpressionParser::parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected)
{
    if (pos->type == TokenType::Minus)
    {
        LiteralParser lit_p;
        DataBase::TokenIterator begin = pos;

        if (lit_p.parse(pos, node, expected))
            return true;
        pos = begin;
    }
    return operator_parser.parse(pos, node, expected);
}

bool TupleElementExpressionParser::parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected)
{
    return LeftAssociativeBinaryOperatorListParser {
        operators,
        std_ext::make_unique<ArrayElementExpressionParser>(),
        std_ext::make_unique<UnsignedIntegerParser>()
    } .parse(pos, node, expected);
}

bool ArrayElementExpressionParser::parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected)
{
    return LeftAssociativeBinaryOperatorListParser {
        operators,
        std_ext::make_unique<ExpressionElementParser>(),
        std_ext::make_unique<ExpressionWithOptionalAliasParser>(false)
    } .parse(pos, node, expected);
}


ExpressionWithOptionalAliasParser::ExpressionWithOptionalAliasParser(bool allow_alias_without_as_keyword, bool prefer_alias_to_column_name_)
    : impl(std_ext::make_unique<WithOptionalAliasParser>(std_ext::make_unique<LambdaExpressionParser>(),
            allow_alias_without_as_keyword, prefer_alias_to_column_name_))
{

}


bool LambdaExpressionParser::parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected)
{
    TokenParser arrow(TokenType::Arrow);
    TokenParser open(TokenType::OpeningRoundBracket);
    TokenParser close(TokenType::ClosingRoundBracket);
    DataBase::TokenIterator begin = pos;
    do
    {
        std::shared_ptr<IAST> inner_arguments;
        std::shared_ptr<IAST> expression;
        bool was_open = false;
        if (open.ignore(pos, expected))
        {
            was_open = true;
        }

        if (!ListParser(std_ext::make_unique<IdentifierParser>(), std_ext::make_unique<TokenParser>(TokenType::Comma)).parse(pos, inner_arguments, expected))
            break;

        if (was_open)
        {
            if (!close.ignore(pos, expected))
                break;
        }

        if (!arrow.ignore(pos, expected))
            break;

        if (!elem_parser.parse(pos, expression, expected))
            return false;
        /// lambda(tuple(inner_arguments), expression)
        auto lambda = std::make_shared<ASTFunction>();
        node = lambda;
        lambda->name = "lambda";

        auto outer_arguments = std::make_shared<ASTExpressionList>();
        lambda->arguments = outer_arguments;
        lambda->children.push_back(lambda->arguments);

        auto tuple = std::make_shared<ASTFunction>();
        outer_arguments->children.push_back(tuple);
        tuple->name = "tuple";
        tuple->arguments = inner_arguments;
        tuple->children.push_back(inner_arguments);

        outer_arguments->children.push_back(expression);
        return true;
    }  while (false);
    pos = begin;
    return elem_parser.parse(pos, node, expected);
}


}







