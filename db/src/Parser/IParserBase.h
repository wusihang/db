#pragma once

#include <Parser/IParser.h>


namespace DataBase {

/** Base class for most parsers
  */
class IParserBase : public IParser
{
public:
    bool parse(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected);

protected:
    virtual bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) = 0;
};

}
