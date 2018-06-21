#pragma once
#include<Parser/ASTWithAlias.h>
#include<Parser/ASTExpressionList.h>
namespace DataBase {

class ASTFunction : public ASTWithAlias {
public:
    std::string name;
    std::shared_ptr<IAST> arguments;
    std::shared_ptr<IAST> parameters;

    ASTFunction() = default;
    ASTFunction(const StringRange range_) : ASTWithAlias(range_) {}

    std::string getId() const override {
        return "Function_" + name;
    }
protected:
    std::string getColumnNameImpl() const override {
        return name;
    }
};


template <typename... Args>
std::shared_ptr<IAST> makeASTFunction(const std::string & name, Args &&... args)
{
    const auto function = std::make_shared<ASTFunction>();
    std::shared_ptr<IAST> result(function);
    function->name = name;
    function->arguments = std::make_shared<ASTExpressionList>();
    function->children.push_back(function->arguments);
    function->arguments->children = { std::forward<Args>(args)... };
    return result;
}

}
