#pragma once
#include<string>
#include<utility>
#include<memory>
namespace Storage {
class IStorage;
}
namespace DataBase {
class Context;
std::pair<std::string, std::shared_ptr<Storage::IStorage>> createTableFromDefinition(
            const std::string & definition,
            const std::string & database_name,
            const std::string & database_data_path,
            Context & context,
            const std::string & description_for_error_message);

}
