#include<Storages/StorageFactory.h>
#include<CommonUtil/StringUtils.h>
#include<Storages/StorageMergeTree.h>
#include <Poco/Exception.h>

namespace ErrorCodes {
extern const int  UNKNOWN_DATABASE_ENGINE;
}

std::shared_ptr< Storage::IStorage > StorageFactory::getStorage(const std::string& name)
{
    if(name == "MergeTree") {
        return std::make_shared<Storage::MergeTreeStorage>();
    }
    throw Poco::Exception("Unknow Engine: " + name,ErrorCodes::UNKNOWN_DATABASE_ENGINE);
}
