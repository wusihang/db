#pragma once
#include<Parser/IAST.h>
namespace DataBase {

class ASTCreateQuery:public IAST {
public:
    bool attach {false};  //attach table , not create one
    std::string database;
    std::string table;
    std::shared_ptr<IAST> columns;
    std::shared_ptr<IAST> storage;
    ASTCreateQuery() = default;
    ASTCreateQuery(const StringRange range_) : IAST(range_) {}

    std::string getId() const override {
        return "CreateQuery_"+database+"_" + table;
    }

    std::shared_ptr< IAST > clone() const override {
        auto res = std::make_shared<ASTCreateQuery>(*this);
        res->children.clear();
        if (columns)        {
            res->columns = columns->clone();
            res->children.push_back(res->columns);
        }
        if (storage)        {
            res->storage = storage->clone();
            res->children.push_back(res->storage);
        }
        return res;
    }
};

}
