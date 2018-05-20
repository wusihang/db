#pragma once
#include<Parser/IParserBase.h>

namespace DataBase {

class KeywordParser : public IParserBase
{
private:
	//关键字指针
    const char * s;

public:
    KeywordParser(const char * s_): s(s_) {

    }

protected:
	//名称也是关键字
    const char * getName() const override {
        return s;
    }

    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override;
};
}

