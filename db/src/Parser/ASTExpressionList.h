#pragma once
#include<Parser/IAST.h>

namespace DataBase {

class ASTExpressionList:public IAST {
public:
    ASTExpressionList() = default;
    ASTExpressionList(const StringRange range_) : IAST(range_) {}
    std::string getId() const override {
        return "ExpressionList";
    }
};

}
