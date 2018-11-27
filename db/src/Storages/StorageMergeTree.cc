#include<Storages/StorageMergeTree.h>
#include<Interpreter/Context.h>
#include<CommonUtil/LoggerUtil.h>
#include<Poco/Exception.h>
#include<Poco/Logger.h>
#include<Storages/DiskSpaceMonitor.h>
#include<Storages/MergeTree//MergeTreeBlockOutputStream.h>
#include <experimental/optional>
#include<Parser/IAST.h>
namespace ErrorCodes
{
extern const int ABORTED;
extern const int LOGICAL_ERROR;
}


namespace Storage {
MergeTreeStorage::MergeTreeStorage(DataBase::Context& _context,const std::string & path_,
                                   const std::string & database_name_,
                                   const std::string & table_name_,DataBase::NamesAndTypesListPtr columns_,const std::string& date_column_name_,std::shared_ptr<DataBase::IAST> & primary_expr_ast_,DataBase::UInt64 index_granularity)
    :pool(_context.getBackgroundPool()),context(_context),path(path_),full_path(path_+table_name_+"/"),database_name(database_name_),table_name(table_name_)
    ,log(&Logger::get(database_name_+"."+table_name_+"(StorageMergeTree)")),
    data(database_name,table_name,full_path,database_name_ + "." + table_name_,columns_,date_column_name_,primary_expr_ast_,index_granularity,_context)
	,merger(data,context.getBackgroundPool()),writer(data) {

}
void MergeTreeStorage::startup()
{
    merge_task_handle =  pool.addTask([this] {
        return mergeTask();
    });
}

void MergeTreeStorage::shutdown()
{
    if(!shutdown_called) {
        shutdown_called = true;
//         merger.cancelForever();
        if (merge_task_handle)
        {
            pool.removeTask(merge_task_handle);
        }
    }
}
MergeTreeStorage::~MergeTreeStorage()
{
    shutdown();
}

bool MergeTreeStorage::mergeTask()
{
    if (shutdown_called)
        return false;
    try
    {
        return merge( false/*aggressive*/, {}/*partition*/);
    }
    catch (Poco::Exception & e)
    {
        if (e.code() == ErrorCodes::ABORTED)
        {
            LOG_INFO(log, e.message());
            return false;
        }
        throw;
    }
}

struct CurrentlyMergingPartsTagger
{
    MergeTreeData::DataPartsVector parts;
    DiskSpaceMonitor::ReservationPtr reserved_space;
    MergeTreeStorage * storage = nullptr;

    CurrentlyMergingPartsTagger() = default;

    CurrentlyMergingPartsTagger(const MergeTreeData::DataPartsVector & parts_, size_t total_size, MergeTreeStorage & storage_)
        : parts(parts_), storage(&storage_)
    {
        /// Assume mutex is already locked, because this method is called from mergeTask.
        reserved_space = DiskSpaceMonitor::reserve(storage->path, total_size); /// May throw.
        for (const auto & part : parts)
        {
            if (storage->currently_merging.count(part))
                throw Poco::Exception("Tagging alreagy tagged part " + part->name + ". This is a bug.", ErrorCodes::LOGICAL_ERROR);
        }
        storage->currently_merging.insert(parts.begin(), parts.end());
    }

    ~CurrentlyMergingPartsTagger()
    {
        std::lock_guard<std::mutex> lock(storage->currently_merging_mutex);

        for (const auto & part : parts)
        {
            if (!storage->currently_merging.count(part))
                std::terminate();
            storage->currently_merging.erase(part);
        }
    }
};


bool MergeTreeStorage::merge(bool aggressive,const std::string & partition)
{
    data.clearOldParts();
    data.clearOldTemporaryDirectories();
    auto structure_lock = lockStructure(true);
    size_t disk_space = DiskSpaceMonitor::getUnreservedFreeSpace(path);

    std::string merged_name;
    std::experimental::optional<CurrentlyMergingPartsTagger> merging_tagger;
    {
        std::lock_guard<std::mutex> lock(currently_merging_mutex);
        MergeTreeData::DataPartsVector parts;
        //两个MergeTreeData都不为空时才可以merge
        auto can_merge = [this] (const MergeTreeData::DataPartPtr & left, const MergeTreeData::DataPartPtr & right)
        {
            return !currently_merging.count(left) && !currently_merging.count(right);
        };

        bool selected = false;

        if (partition.empty())
        {
            size_t max_parts_size_for_merge = merger.getMaxPartsSizeForMerge();
            if (max_parts_size_for_merge > 0)
            {
                selected = merger.selectPartsToMerge(parts, merged_name, aggressive, max_parts_size_for_merge, can_merge);
            }
        } else
        {
            selected = merger.selectAllPartsToMergeWithinPartition(parts, merged_name, disk_space, can_merge, partition);
        }

        if (!selected)
            return false;
        merging_tagger.emplace(parts, MergeTreeDataMerger::estimateDiskSpaceForMerge(parts), *this);
    }
    return true;
}


BlockInputStreams MergeTreeStorage::read(const Names& column_names, const DataBase::Context& context, QueryProcessingStage::Enum& processed_stage, size_t max_block_size, unsigned int num_streams)
{
    return Storage::IStorage::read(column_names, context, processed_stage, max_block_size, num_streams);
}

BlockOutputStreamPtr MergeTreeStorage::write(const std::shared_ptr< DataBase::IAST >& query)
{
    return std::make_shared<IO::MergeTreeBlockOutputStream>(*this);
}


}




