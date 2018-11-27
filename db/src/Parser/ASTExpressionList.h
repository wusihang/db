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
    std::shared_ptr< IAST > clone() const override {
        const auto res = std::make_shared<ASTExpressionList>(*this);
        res->children.clear();
        for (const auto & child : children)
            res->children.emplace_back(child->clone());
        return res;
    }
};

}
