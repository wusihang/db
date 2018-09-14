#include<Storages/MergeTree/MergeTreeData.h>

namespace Storage {

void MergeTreeData::clearOldParts()
{

}

void MergeTreeData::clearOldTemporaryDirectories(ssize_t custom_directories_lifetime_seconds)
{

}

void MergeTreeData::delayInsertIfNeeded(Poco::Event* until)
{

}


MergeTreeData::MergeTreeData(const std::string& database_, const std::string& table_, const std::string& full_path_,const std::string& log_name_,DataBase::NamesAndTypesListPtr columns_)
    :database_name(database_),table_name(table_),full_path(full_path_),log_name(log_name_),columns(columns_)
{

}

}
