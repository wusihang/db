#include<Storages/StorageFactory.h>
#include<CommonUtil/StringUtils.h>
#include<Storages/StorageMergeTree.h>
#include <Poco/Exception.h>
#include <Interpreter/Context.h>

namespace ErrorCodes {
extern const int  UNKNOWN_DATABASE_ENGINE;
}

std::shared_ptr< Storage::IStorage > StorageFactory::getStorage(const std::string& name,const std::string & data_path,
        const std::string & table_name,
        const std::string & database_name,DataBase::Context& context,DataBase::NamesAndTypesListPtr columns)
{
    if(name == "MergeTree") {
        return std::make_shared<Storage::MergeTreeStorage>(context,data_path,database_name,table_name,columns);
    }
    throw Poco::Exception("Unknow Engine: " + name,ErrorCodes::UNKNOWN_DATABASE_ENGINE);
}
