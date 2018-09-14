#pragma once
#include <Parser/ASTWithAlias.h>
#include<Core/Field.h>
namespace DataBase {
class ASTLiteral:public ASTWithAlias {
public:
    Field value;
    ASTLiteral() = default;
    ASTLiteral(const StringRange range_, const Field & value_);

protected:
    String getColumnNameImpl() const override;
};

}
