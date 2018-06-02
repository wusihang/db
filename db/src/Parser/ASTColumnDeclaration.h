#pragma once
#include<Parser/IAST.h>
namespace DataBase {

class ASTColumnDeclaration:public IAST {
public:
    std::string name;
    std::shared_ptr<IAST> type;

    ASTColumnDeclaration() = default;
    ASTColumnDeclaration(const StringRange range) : IAST {range} {}
    std::string getId() const override {
        return "ColumnDeclaration_" + name;
    }
};

}
