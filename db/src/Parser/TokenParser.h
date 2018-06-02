#pragma once
#include<Parser/IParserBase.h>

namespace DataBase {
class TokenParser : public IParserBase
{
private:
    TokenType token_type;
public:
    TokenParser(TokenType token_type) : token_type(token_type) {}
protected:
    const char * getName() const override {
        return "token";
    }
    bool parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected) override;
};


}
