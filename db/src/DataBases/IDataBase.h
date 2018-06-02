#pragma once
#include<memory>
#include<string>
#include<Storages/IStorage.h>
#include <Parser/IAST.h>

namespace DataBase {

class IDataBase :public std::enable_shared_from_this<IDataBase> {

public:
    virtual bool empty() const = 0;
    virtual std::string getEngineName() const = 0;
    virtual bool isTableExists(const std::string& table_name) const = 0;
    virtual void createTable(const std::string& table_name,std::shared_ptr<Storage::IStorage>& storage,const std::shared_ptr<IAST>& query)  = 0;
};

}
