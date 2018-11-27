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

class ParserEngine:public IParserBase {
protected:
    const char * getName() const {
        return "ENGINE";
    }
    bool parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected);
};


class ParserIdentifierWithOptionalParameters : public IParserBase
{
protected:
    const char * getName() const {
        return "identifier with optional parameters";
    }
    bool parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected);
};

class ParserIdentifierWithParameters : public IParserBase
{
protected:
    const char * getName() const { return "identifier with parameters"; }
     bool parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected);
};


}
