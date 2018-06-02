#pragma once
#include<Parser/IParserBase.h>

namespace DataBase {

class ParserCreateQuery : public IParserBase
{
protected:
    const char * getName() const {
        return "CREATE TABLE query";
    }
    bool parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected);
};

}
