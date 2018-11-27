#pragma once
#include<DataBases/IDataBase.h>
#include<mutex>
#include <map>

namespace Poco{
	class Logger;
}

namespace DataBase {

class DataBaseMemory: public IDataBase {
protected:
    const std::string name;
    mutable std::mutex mutex;
    std::map<std::string,std::shared_ptr<Storage::IStorage> > tables;
	
	Poco::Logger * log;

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
    std::shared_ptr< Storage::IStorage > tryGetTable(const std::string& name) override;
	void shutdown() override;
	void loadTables(Context & context) override;
	void attachTable(const String & table_name, const  std::shared_ptr< Storage::IStorage > & table) override;
    ~DataBaseMemory()  {}
};

}
