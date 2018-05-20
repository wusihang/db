#pragma once
#include<Parser/IParserBase.h>

namespace DataBase {

class IdentifierParser: public IParserBase {
protected:
    const char * getName() const {
        return "identifier";
    }
    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected);
};

}
