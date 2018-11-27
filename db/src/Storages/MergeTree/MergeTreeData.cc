#include<Storages/MergeTree/MergeTreeData.h>
#include<Parser/IAST.h>
#include<Ext/typeid_cast.h>
#include<DataTypes/DataTypeDate.h>
#include<Poco/File.h>
#include<CommonUtil/LoggerUtil.h>
#include<Poco/Logger.h>

namespace ErrorCodes {
extern const int NO_SUCH_COLUMN_IN_TABLE;
extern const int BAD_ARGUMENTS;
extern const int LOGICAL_ERROR;
extern const int DUPLICATE_DATA_PART;
}

namespace Storage {

static inline void logSubtract(size_t & from, size_t value, Logger * log, const std::string & variable)
{
    if (value > from)
        LOG_ERROR(log, "Possibly incorrect subtraction: " << from << " - " << value << " = " << from - value << ", variable " << variable);

    from -= value;
}

void MergeTreeData::clearOldParts()
{

}

void MergeTreeData::clearOldTemporaryDirectories(ssize_t custom_directories_lifetime_seconds)
{

}

void MergeTreeData::delayInsertIfNeeded(Poco::Event* until)
{

}

void MergeTreeData::renameTempPartAndAdd(MergeTreeData::MutableDataPartPtr& part, DataBase::SimpleIncrement* increment)
{
    auto removed = renameTempPartAndReplace(part, increment);
    if (!removed.empty())
    {
        throw Poco::Exception("Added part " + part->name + " covers " + IO::toString(removed.size())
                              + " existing part(s) (including " + removed[0]->name + ")", ErrorCodes::LOGICAL_ERROR);
    }
}

MergeTreeData::DataPartsVector MergeTreeData::renameTempPartAndReplace(MergeTreeData::MutableDataPartPtr& part, DataBase::SimpleIncrement* increment)
{
    DataPartsVector replaced;
    {
        std::lock_guard<std::mutex> lock(data_parts_mutex);
        if(increment) {
            part->info.min_block = part->info.max_block = increment->get();
        }
        std::string old_name = part->name;
        std::string new_name = MergeTreePartInfo::getPartName(part->min_date, part->max_date, part->info.min_block, part->info.max_block, part->info.level);
        LOG_TRACE(log, "Renaming temporary part " << part->relative_path << " to " << new_name << ".");

        /// Check that new part doesn't exist yet.
        {
            part->is_temp = false;
            part->name = new_name;
            bool duplicate = data_parts.count(part);
            part->name = old_name;
            part->is_temp = true;
            if (duplicate)
                throw Poco::Exception("Part " + new_name + " already exists", ErrorCodes::DUPLICATE_DATA_PART);
        }

        bool in_all_data_parts;
        {
            std::lock_guard<std::mutex> lock_all(all_data_parts_mutex);
            in_all_data_parts = all_data_parts.count(part) != 0;
        }
        /// New part can be removed from data_parts but not from filesystem and ZooKeeper
        if (in_all_data_parts)
            clearOldPartsAndRemove();

        part->renameTo(new_name);
        part->is_temp = false;
        part->name = new_name;

        bool obsolete = false;
        auto it = data_parts.lower_bound(part);
        /// Go to the left.
        while (it != data_parts.begin())
        {
            --it;
            if (!part->contains(**it))
            {
                if ((*it)->contains(*part))
                    obsolete = true;
                ++it;
                break;
            }
            replaced.push_back(*it);
            (*it)->remove_time = time(nullptr);
            removePartContributionToColumnSizes(*it);
            data_parts.erase(it++); /// Yes, ++, not --.
        }
        std::reverse(replaced.begin(), replaced.end()); /// Parts must be in ascending order.
        while (it != data_parts.end())
        {
            if (!part->contains(**it))
            {
                if ((*it)->name == part->name || (*it)->contains(*part))
                    obsolete = true;
                break;
            }
            replaced.push_back(*it);
            (*it)->remove_time = time(nullptr);
            removePartContributionToColumnSizes(*it);
            data_parts.erase(it++);
        }

        if (obsolete)
        {
            LOG_WARNING(log, "Obsolete part " << part->name << " added");
            part->remove_time = time(nullptr);
        }
        else
        {
            data_parts.insert(part);
            addPartContributionToColumnSizes(part);
        }

        {
            std::lock_guard<std::mutex> lock_all(all_data_parts_mutex);
            all_data_parts.insert(part);
        }
    }
    return replaced;
}

void MergeTreeData::addPartContributionToColumnSizes(const MergeTreeData::DataPartPtr& part)
{
    const auto & files = part->checksums.files;

    ///  This method doesn't take into account columns with multiple files.
    for (const auto & column : getColumnsList())
    {
        const auto escaped_name = column.name;
        const auto bin_file_name = escaped_name + ".bin";
        const auto mrk_file_name = escaped_name + ".mrk";

        ColumnSize & column_size = column_sizes[column.name];

        if (files.count(bin_file_name))
        {
            const auto & bin_file_checksums = files.at(bin_file_name);
            column_size.data_compressed += bin_file_checksums.file_size;
            column_size.data_uncompressed += bin_file_checksums.uncompressed_size;
        }

        if (files.count(mrk_file_name))
            column_size.marks += files.at(mrk_file_name).file_size;
    }
}


void MergeTreeData::removePartContributionToColumnSizes(const MergeTreeData::DataPartPtr& part)
{
    const auto & files = part->checksums.files;
    ///  This method doesn't take into account columns with multiple files.
    for (const auto & column : *columns)
    {
        const auto escaped_name = column.name;
        const auto bin_file_name = escaped_name + ".bin";
        const auto mrk_file_name = escaped_name + ".mrk";

        auto & column_size = column_sizes[column.name];

        if (files.count(bin_file_name))
        {
            const auto & bin_file_checksums = files.at(bin_file_name);
            logSubtract(column_size.data_compressed, bin_file_checksums.file_size, log, bin_file_name + ".file_size");
            logSubtract(column_size.data_uncompressed, bin_file_checksums.uncompressed_size, log, bin_file_name + ".uncompressed_size");
        }

        if (files.count(mrk_file_name))
            logSubtract(column_size.marks, files.at(mrk_file_name).file_size, log, mrk_file_name + ".file_size");
    }
}



MergeTreeData::MergeTreeData(const std::string& database_, const std::string& table_, const std::string& full_path_,const std::string& log_name_, DataBase::NamesAndTypesListPtr columns_,const std::string& date_column_name_,std::shared_ptr<DataBase::IAST> & primary_expr_ast_,DataBase::UInt64 index_granularity_,DataBase::Context& context_)
    :context(context_),database_name(database_),table_name(table_),full_path(full_path_),log_name(log_name_), log(&Poco::Logger::get(log_name + " (Data)")),columns(columns_),date_column_name(date_column_name_),primary_expr_ast(primary_expr_ast_?primary_expr_ast_->clone():nullptr),index_granularity(index_granularity_)
{

    const auto check_date_exists = [this] (const DataBase::NamesAndTypesList & columns)
    {
        for (const auto & column : columns)
        {
            if (column.name == date_column_name)
            {
                if (!typeid_cast<const DataBase::DataTypeDate *>(column.type.get()))
                    throw Poco::Exception("Date column (" + date_column_name + ") for storage of MergeTree family must have type Date."
                                          " Provided column of type " + column.type->getName() + "."
                                          " You may have separate column with type " + column.type->getName() + ".", ErrorCodes::BAD_TYPE_OF_FIELD);
                return true;
            }
        }
        return false;
    };

    if (!check_date_exists(*columns))
        throw Poco::Exception {"Date column (" + date_column_name + ") does not exist in table declaration.", ErrorCodes::NO_SUCH_COLUMN_IN_TABLE};
    if (!primary_expr_ast )
        throw Poco::Exception("Primary key could be empty only for UnsortedMergeTree", ErrorCodes::BAD_ARGUMENTS);

    initPrimaryKey();

    /// Creating directories, if not exist.
    Poco::File(full_path).createDirectories();
    Poco::File(full_path + "detached").createDirectory();
}


void MergeTreeData::initPrimaryKey()
{
    if (primary_expr_ast)
    {
        sort_descr.clear();
        sort_descr.reserve(primary_expr_ast->children.size());
        for (const std::shared_ptr<DataBase::IAST> & ast : primary_expr_ast->children)
        {
            std::string name = ast->getColumnName();
            //默认情况下，排序顺序升序，Null认为是更小
            sort_descr.emplace_back(name, 1, 1);
        }
    }
}


}

