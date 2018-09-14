#include<Parser/ASTLiteral.h>
#include<Core/FieldVisitor.h>
DataBase::ASTLiteral::ASTLiteral(const DataBase::StringRange range_, const DataBase::Field& value_)
    : ASTWithAlias(range_) ,value(value_) {

}


DataBase::String DataBase::ASTLiteral::getColumnNameImpl() const
{
    return applyVisitor(FieldVisitorToString(),value);
}
