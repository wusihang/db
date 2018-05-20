#pragma once
#include<Parser/ASTWithAlias.h>
#include<Parser/StringRange.h>

namespace DataBase {

class ASTIdentifier: public ASTWithAlias {

public:
    enum Kind {
        Column,
        Database,
        Table,
        Format
    };

    std::string name;

    Kind kind;
    ASTIdentifier() = default;
    ASTIdentifier(const StringRange range_, const std::string & name_, const Kind kind_ = Column)
        : ASTWithAlias(range_), name(name_), kind(kind_) {

    }

    std::string getId() const override {
        return "Identifier_" + name;
    }

    std::shared_ptr<IAST> clone() const override {
        return std::make_shared<ASTIdentifier>(*this);
    }

protected:
    std::string getColumnNameImpl() const override {
        return name;
    }
};

}
