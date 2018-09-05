#pragma once
#include<Parser/IParserBase.h>
namespace DataBase {

class ParseQuery: public IParserBase {
private:
	const char* end;
	
protected:
    const char* getName() const override{
        return "Query";
    }
    bool parseImpl(TokenIterator& pos, std::shared_ptr< IAST >& node, Expected& expected) override;

public:
    ParseQuery(const char* end_) :end(end_){
    }

};

}
