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
};

}
