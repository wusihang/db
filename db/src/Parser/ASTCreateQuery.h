#pragma once
#include<Parser/IAST.h>
namespace DataBase {

class ASTCreateQuery:public IAST {
public:
	bool attach{false};   //attach table , not create one
    std::string database;
    std::string table;
	std::shared_ptr<IAST> columns;
	std::shared_ptr<IAST> storage;
    ASTCreateQuery() = default;
    ASTCreateQuery(const StringRange range_) : IAST(range_) {}

    std::string getId() const override {
        return "CreateQuery_"+database+"_" + table;
    }
};

}
