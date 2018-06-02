#include<Parser/ExpressionListParsers.h>
#include<Parser/ASTExpressionList.h>

namespace DataBase {
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
            auto prev_pos = pos;
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
}
