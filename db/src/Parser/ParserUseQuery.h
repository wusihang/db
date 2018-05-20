#pragma once

#include <Parser/IParserBase.h>


namespace DataBase
{
class ParserUseQuery : public IParserBase
{
protected:
    const char * getName() const {
        return "USE query";
    }
    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected);
};

}
