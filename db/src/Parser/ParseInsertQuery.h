#pragma once
#include <Parser/IParserBase.h>


namespace DataBase {
class ParseInsertQuery:public IParserBase {
private:
    const char * end;

    const char * getName() const override {
        return "INSERT query";
    }
    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override;
public:
    ParseInsertQuery(const char * end) : end(end) {}
};
}
