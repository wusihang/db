#include<DataBases/DataBaseMemory.h>
#include<Poco/Exception.h>
#include<Interpreter/Context.h>
#include<Poco/Logger.h>

namespace ErrorCodes {
extern const int TABLE_ALREADY_EXISTS;
}

namespace DataBase {

bool DataBaseMemory::empty() const {
    std::lock_guard<std::mutex> lock(mutex);
    return false;
}

bool DataBaseMemory::isTableExists(const std::string& table_name) const
{
    std::lock_guard<std::mutex> lock(mutex);
    //tables是map结构,根据table_name查询,如果存在,返回1,否则返回0
    return tables.count(table_name);
}

void DataBaseMemory::createTable(const std::string& table_name,std::shared_ptr<Storage::IStorage>& storage,const std::shared_ptr<IAST>& /*query*/)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (!tables.emplace(table_name, storage).second) {
        throw Poco::Exception("Table " + name + "." + table_name + " already exists.", ErrorCodes::TABLE_ALREADY_EXISTS);
    }
}

void DataBaseMemory::loadTables(Context& context)
{
    log = &Poco::Logger::get("DatabaseMemory(" + name + ")");
    //do nothing
}

void DataBaseMemory::attachTable(const String& table_name, const std::shared_ptr< Storage::IStorage >& table)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (!tables.emplace(table_name, table).second)
        throw Poco::Exception("Table " + name + "." + table_name + " already exists.", ErrorCodes::TABLE_ALREADY_EXISTS);
}


std::shared_ptr< Storage::IStorage > DataBaseMemory::tryGetTable(const std::string& name)
{
    std::lock_guard<std::mutex> lock(mutex);
    auto it = tables.find(name);
    if (it == tables.end())
        return {};
    return it->second;
}

void DataBaseMemory::shutdown()
{
    std::lock_guard<std::mutex> lock(mutex);
    tables.clear();
}


}
