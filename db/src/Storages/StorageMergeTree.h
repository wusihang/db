#pragma once
#include<mutex>
#include<Storages/IStorage.h>
#include<Storages/BackgroundProcessingPool.h>
#include<Storages/MergeTree/MergeTreeData.h>
#include<Storages/MergeTree/MergeTreeDataMerger.h>
#include<Storages/MergeTree/MergeTreeDataWriter.h>
namespace Poco {
class Logger;
}

namespace Storage {

class MergeTreeStorage: public IStorage {
public:
    std::string getName() const override {
        return "MergeTree";
    }
    void startup() override;
    void shutdown() override;
    BlockOutputStreamPtr write(const std::shared_ptr< DataBase::IAST >& query) override;
    BlockInputStreams read(const Names& column_names, const DataBase::Context& context, QueryProcessingStage::Enum& processed_stage, size_t max_block_size, unsigned int num_streams) override;
    ~MergeTreeStorage() override;

    MergeTreeStorage(DataBase::Context& _context,const std::string & path_,
                     const std::string & database_name_,
                     const std::string & table_name_,DataBase::NamesAndTypesListPtr columns_);
	
	 const DataBase::NamesAndTypesList & getColumnsListImpl() const override { return data.getColumnsListNonMaterialized(); }

private:
	friend struct CurrentlyMergingPartsTagger;
	
    BackgroundProcessingPool& pool;
    DataBase::Context& context;
    BackgroundProcessingPool::TaskHandle merge_task_handle;
    std::atomic<bool> shutdown_called {false};
    std::string path;
    std::string database_name;
    std::string table_name;
    Poco::Logger* log;
    MergeTreeData data;
    std::mutex currently_merging_mutex;
    MergeTreeData::DataParts currently_merging;
	
	MergeTreeDataMerger merger;
	
	MergeTreeDataWriter writer;


    bool mergeTask();
    bool merge(bool aggressive,const std::string & partition);
};

}