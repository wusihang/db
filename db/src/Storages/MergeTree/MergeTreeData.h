#pragma once
#include<set>
#include<unordered_map>
#include<vector>
#include<Storages/ITableDeclaration.h>
#include<Storages/MergeTree/MergeTreeDataPart.h>
#include<Core/NamesAndTypes.h>
namespace Storage {

class MergeTreeData:public ITableDeclaration {

public:
    using DataPart = MergeTreeDataPart;
    using MutableDataPartPtr = std::shared_ptr<DataPart>;
    /// After the DataPart is added to the working set, it cannot be changed.
    using DataPartPtr = std::shared_ptr<const DataPart>;

    struct DataPartPtrLess
    {
        using is_transparent = void;
        bool operator()(const DataPartPtr & lhs, const MergeTreePartInfo & rhs) const {
            return lhs->info < rhs;
        }
        bool operator()(const MergeTreePartInfo & lhs, const DataPartPtr & rhs) const {
            return lhs < rhs->info;
        }
        bool operator()(const DataPartPtr & lhs, const DataPartPtr & rhs) const {
            return lhs->info < rhs->info;
        }
    };

    using DataParts = std::set<DataPartPtr, DataPartPtrLess>;
    using DataPartsVector = std::vector<DataPartPtr>;

    /// For resharding.
    using MutableDataParts = std::set<MutableDataPartPtr, DataPartPtrLess>;
    using PerShardDataParts = std::unordered_map<size_t, MutableDataPartPtr>;

    /// Delete irrelevant parts.
    void clearOldParts();

    /// Deleate all directories which names begin with "tmp"
    /// Set non-negative parameter value to override MergeTreeSettings temporary_directories_lifetime
    void clearOldTemporaryDirectories(ssize_t custom_directories_lifetime_seconds = -1);


    MergeTreeData(const std::string & database_, const std::string & table_, const std::string & full_path_,const std::string& log_name_,DataBase::NamesAndTypesListPtr columns_);

    std::string getLogName() const {
        return log_name;
    }

    const DataBase::NamesAndTypesList& getColumnsListImpl() const override {
		return *columns;
    }

private:
    std::string database_name;
    std::string table_name;
    std::string full_path;
    std::string log_name;

    DataBase::NamesAndTypesListPtr columns;
};

}
