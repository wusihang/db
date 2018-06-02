#pragma once
#include<Parser/IAST.h>
namespace DataBase {

class ASTCreateQuery:public IAST {
public:
    std::string database;
    std::string table;
	std::shared_ptr<IAST> columns;
    ASTCreateQuery() = default;
    ASTCreateQuery(const StringRange range_) : IAST(range_) {}

    std::string getId() const override {
        return "CreateQuery_"+database+"_" + table;
    }
};

}
