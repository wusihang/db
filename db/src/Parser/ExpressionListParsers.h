#pragma once
#include<Parser/IParserBase.h>
#include<memory>
namespace DataBase {

class ListParser:public IParserBase {

public:
    ListParser(std::unique_ptr<IParser> && elem_parser_, std::unique_ptr<IParser>  && separator_parser_, bool allow_empty_ = true)
        : elem_parser(std::move(elem_parser_)), separator_parser(std::move(separator_parser_)), allow_empty(allow_empty_)
    {}

protected:
    const char * getName() const {
        return "list of elements";
    }
    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected) override;

private:
    std::unique_ptr<IParser>  elem_parser;
    std::unique_ptr<IParser>  separator_parser;
    bool allow_empty;
};
}
