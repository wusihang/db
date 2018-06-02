#pragma once
#include<Parser/IParserBase.h>
namespace DataBase {

//建表语句中,列声明解析
class ColumnDeclarationListParser: public IParserBase {
protected:
    const char * getName() const {
        return "column declaration list";
    }
    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected);
};

template <class NameParser>
class IColumnDeclarationParser: public IParserBase {
protected:
    const char * getName() const {
        return "column declaration";
    }
    bool parseImpl(TokenIterator & pos, std::shared_ptr<IAST> & node, Expected & expected);
};
}
