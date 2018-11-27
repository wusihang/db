#pragma once
#include<set>
#include<unordered_map>
#include<vector>
#include<Storages/ITableDeclaration.h>
#include<Storages/MergeTree/MergeTreeDataPart.h>
#include<Core/NamesAndTypes.h>
#include<Poco/Event.h>
#include<Common/SimpleIncrement.h>
#include<Interpreter/ExpressionActions.h>
#include<Core/SortDescription.h>
#include <Interpreter/Context.h>
namespace DataBase {
class IAST;
}
namespace Poco {
class Logger;
}

namespace Storage {

class MergeTreeData:public ITableDeclaration {
    friend class  MergeTreeDataWriter;
    friend class MergeTreeDataPart;
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

    void clearOldPartsAndRemove() {

    }


    MergeTreeData(const std::string & database_, const std::string & table_, const std::string & full_path_,const std::string& log_name_,DataBase::NamesAndTypesListPtr columns_,const std::string& date_column_name_,std::shared_ptr<DataBase::IAST> & primary_expr_ast_,DataBase::UInt64 index_granularity_,DataBase::Context& context_);

    std::string getLogName() const {
        return log_name;
    }

    const DataBase::NamesAndTypesList& getColumnsListImpl() const override {
        return *columns;
    }

    void delayInsertIfNeeded(Poco::Event * until = nullptr);

    void renameTempPartAndAdd(MutableDataPartPtr & part, DataBase::SimpleIncrement * increment = nullptr);

    DataPartsVector renameTempPartAndReplace(
        MutableDataPartPtr & part, DataBase::SimpleIncrement * increment = nullptr);

    DataBase::ExpressionActionsPtr getPrimaryExpression() const {
        return primary_expr;
    }

    DataBase::SortDescription getSortDescription() const {
        return sort_descr;
    }

    DataBase::Context& context;

    struct ColumnSize
    {
        size_t marks = 0;
        size_t data_compressed = 0;
        size_t data_uncompressed = 0;

        size_t getTotalCompressedSize() const
        {
            return marks + data_compressed;
        }
    };

    using ColumnSizes = std::unordered_map<std::string, ColumnSize>;

private:
    std::string database_name;
    std::string table_name;
    std::string full_path;
    std::string log_name;

    Poco::Logger * log;

    DataParts data_parts;
    mutable std::mutex data_parts_mutex;

    DataParts all_data_parts;
    mutable std::mutex all_data_parts_mutex;

    DataBase::NamesAndTypesListPtr columns;

    ColumnSizes column_sizes;

    const std::string date_column_name;

    DataBase::SimpleIncrement insert_increment;

    std::shared_ptr<DataBase::IAST> primary_expr_ast;

    DataBase::ExpressionActionsPtr primary_expr;

    DataBase::SortDescription sort_descr;

public:
    const size_t index_granularity;

private:
    void initPrimaryKey();
    void addPartContributionToColumnSizes(const DataPartPtr & part);
    void removePartContributionToColumnSizes(const DataPartPtr & part);
};

}

