#pragma once
#include<DataBases/IDataBase.h>
#include<mutex>
#include <map>

namespace DataBase {

class DataBaseMemory: public IDataBase {
protected:
    const std::string name;
    mutable std::mutex mutex;
    std::map<std::string,std::shared_ptr<Storage::IStorage> > tables;

public:
    DataBaseMemory(const std::string& _name)
        :name(_name) {
    }

    std::string getEngineName() const override {
        return "Memory";
    }
    bool empty() const override;
    bool isTableExists(const std::string& table_name) const override;
    void createTable(const std::string& table_name,std::shared_ptr<Storage::IStorage>& storage,const std::shared_ptr<IAST>& query)  override;

    ~DataBaseMemory()  {}
};

}
