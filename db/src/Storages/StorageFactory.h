#pragma once
#include<memory>
#include<Storages/IStorage.h>

namespace DataBase{
	class Context;
}

namespace StorageFactory {
std::shared_ptr<Storage::IStorage> getStorage(const std::string& name,const std::string & data_path,
        const std::string & table_name,
        const std::string & database_name,DataBase::Context& context,DataBase::NamesAndTypesListPtr columns);
}
