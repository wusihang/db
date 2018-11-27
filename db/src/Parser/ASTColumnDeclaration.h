#pragma once
#include<Parser/IAST.h>
namespace DataBase {

class ASTColumnDeclaration:public IAST {
public:
    std::string name;
    std::shared_ptr<IAST> type;
	std::string default_specifier;
    std::shared_ptr<IAST> default_expression;

    ASTColumnDeclaration() = default;
    ASTColumnDeclaration(const StringRange range) : IAST {range} {}
    std::string getId() const override {
        return "ColumnDeclaration_" + name;
    }
    
    std::shared_ptr< IAST > clone() const override{
		const auto res = std::make_shared<ASTColumnDeclaration>(*this);
        std::shared_ptr< IAST > ptr{res};
        res->children.clear();
        if (type) {
            res->type = type->clone();
            res->children.push_back(res->type);
        }
        if (default_expression) {
            res->default_expression = default_expression->clone();
            res->children.push_back(res->default_expression);
        }
        return ptr;
    }
};

}
