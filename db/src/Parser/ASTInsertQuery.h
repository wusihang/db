#pragma once
#include<Parser/IAST.h>
namespace DataBase {

class ASTInsertQuery:public IAST {
public:
    std::string  database;
    std::string  table;
    std::shared_ptr<IAST> columns;
    std::string format;
    /// Data to insert
    const char * data = nullptr;
    const char * end = nullptr;

    ASTInsertQuery() = default;
    ASTInsertQuery(const StringRange range_) : IAST(range_) {}

    /** Get the text that identifies this element. */
    std::string getId() const override {
        return "InsertQuery_" + database + "_" + table;
    };

    std::shared_ptr<IAST> clone() const override
    {
        auto res = std::make_shared<ASTInsertQuery>(*this);
        res->children.clear();

        if (columns) {
            res->columns = columns->clone();
            res->children.push_back(res->columns);
        }
        return res;
    }
};

}
