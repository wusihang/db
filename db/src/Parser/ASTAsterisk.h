#pragma once
#include<Parser/IAST.h>
namespace DataBase {
class ASTAsterisk : public IAST
{
public:
    ASTAsterisk() = default;
    ASTAsterisk(StringRange range_) : IAST(range_) {}
    std::string getColumnName() const override {
        return "*";
    }
};
}
