#pragma once
#include<Parser/IParserBase.h>

namespace DataBase {

class ParseQueryWithOutput :public IParserBase {
protected:
    const char* getName() const override {
        return "Query with output";
    }
    bool parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected) override;
};

}
