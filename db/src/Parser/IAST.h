#pragma once
#include<vector>
#include<memory>
#include<Parser/StringRange.h>
#include<Poco/Exception.h>

namespace DataBase {

//抽象语法树
class IAST {

public:
    //子节点
    std::vector<IAST> children;
    StringRange range;
    std::shared_ptr<std::string> query_string;

    IAST() = default;
    IAST(const StringRange _range)
        :range(_range) {
    }

    virtual ~IAST() = default;

    virtual std::string getColumnName() const {
        throw Poco::Exception("trying to get name of not a column:" + getId());
    }

    virtual std::string getAliasOrColumnName() const {
        getColumnName();
    }

    virtual std::string tryGetAlias() const {
        return std::string();
    }

    virtual void setAlias(const std::string & to)
    {
        throw Poco::Exception("cannot set alias of " + getColumnName());
    }

    virtual std::string getId() const {
        throw Poco::Exception("IAST cannot be instaniation directly");
    }

    virtual std::shared_ptr<IAST> clone() const {
        throw Poco::Exception("IAST cannot be instaniation directly");
    }

};


}
