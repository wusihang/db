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
    virtual std::shared_ptr<Storage::IStorage> tryGetTable(const std::string & name) = 0;
    virtual void shutdown() = 0;
    virtual void loadTables(Context & context) = 0;
    virtual void attachTable(const String & table_name, const  std::shared_ptr< Storage::IStorage > & table) = 0 ;
};

}
