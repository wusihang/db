#include<DataBases/DataBaseFactory.h>
#include<DataBases/DataBaseOrdinary.h>
#include<Interpreter/Context.h>
#include <Poco/Exception.h>

namespace ErrorCodes
{
extern const int UNKNOWN_DATABASE_ENGINE;
}

std::shared_ptr< DataBase::IDataBase > DataBase::DatabaseFactory::get(const std::string& engine_name, const std::string& database_name, const std::string& path, DataBase::Context& context)
{
    if (engine_name == "Ordinary")
    {
        return std::make_shared<DataBase::DataBaseOrdinary>(database_name, path);
    }
    throw Poco::Exception("Unknown database engine: " + engine_name, ErrorCodes::UNKNOWN_DATABASE_ENGINE);
}
