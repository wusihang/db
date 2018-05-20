#include<Parser/TokenIterator.h>

std::vector< DataBase::Token > DataBase::checkUnmatchedParentheses(DataBase::TokenIterator begin, DataBase::Token* last)
{
	//只有两种括号, () [] , 检查结果是空集时,表示正常匹配,否则就是没有配对的括号出现
    std::vector< DataBase::Token > stack;
    for(DataBase::TokenIterator it = begin; it.isValid()&&  &it.get()<=last; ++it) {

        //如果token是 (  [
        if (it->type == TokenType::OpeningRoundBracket || it->type == TokenType::OpeningSquareBracket)
        {
            //压栈
            stack.push_back(*it);
        }
        // token是  ) ]
        else if (it->type == TokenType::ClosingRoundBracket || it->type == TokenType::ClosingSquareBracket)
        {
            //如果栈是空
            if (stack.empty())
            {
                /// Excessive closing bracket.
                stack.push_back(*it);
                return stack;
            }
            else if ((stack.back().type == TokenType::OpeningRoundBracket && it->type == TokenType::ClosingRoundBracket)
                     || (stack.back().type == TokenType::OpeningSquareBracket && it->type == TokenType::ClosingSquareBracket))
            {
                /// Valid match.
                stack.pop_back();
            }
            else
            {
                /// Closing bracket type doesn't match opening bracket type.
                stack.push_back(*it);
                return stack;
            }
        }
    }
    return stack;
}


