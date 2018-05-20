#pragma once
#include<Parser/IAST.h>
#include<string>

namespace DataBase {

class ASTWithAlias : public IAST {

public:
    ASTWithAlias() = default;
    ASTWithAlias(const StringRange _range):IAST(_range) {
    }

    std::string alias;
    bool prefer_alias_to_column_name = false;

    std::string getColumnName() const override final {
        return prefer_alias_to_column_name && !alias.empty() ? alias : getColumnNameImpl();
    }
    std::string getAliasOrColumnName() const override {
        return alias.empty() ? getColumnNameImpl() : alias;
    }
    std::string tryGetAlias() const override {
        return alias;
    }
    void setAlias(const std::string & to) override {
        alias = to;
    }

protected:
    virtual  std::string getColumnNameImpl() const = 0;

};

inline std::shared_ptr<IAST> setAlias(std::shared_ptr<IAST>  ast, const std::string & alias)
{
    ast->setAlias(alias);
    return ast;
};


}
