#pragma once
#include<Parser/IAST.h>
#include<Parser/StringRange.h>

namespace DataBase {

class ASTUseQuery: public IAST {

public :
    std::string database;

    ASTUseQuery() = default;

    ASTUseQuery(const StringRange _range):IAST(_range) {

    }

    std::string getId() const override {
        return "UseQuery_" + database;
    };

    std::shared_ptr<IAST> clone() const override {
        return std::make_shared<ASTUseQuery>(*this);
    }

};

}
