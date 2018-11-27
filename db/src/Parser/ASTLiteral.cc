#include<Parser/ASTLiteral.h>
#include<Core/FieldVisitor.h>
DataBase::ASTLiteral::ASTLiteral(const DataBase::StringRange range_, const DataBase::Field& value_)
    : ASTWithAlias(range_) ,value(value_) {

}


DataBase::String DataBase::ASTLiteral::getColumnNameImpl() const
{
    return applyVisitor(FieldVisitorToString(),value);
}


std::shared_ptr< DataBase::IAST > DataBase::ASTLiteral::clone() const
{
    return std::make_shared<ASTLiteral>(*this);
}

std::string DataBase::ASTLiteral::getId() const
{
    return "Literal_" + applyVisitor(FieldVisitorDump(), value);
}
