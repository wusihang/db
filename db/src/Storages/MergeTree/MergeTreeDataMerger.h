#pragma once
#include<Storages/MergeTree/MergeTreeData.h>
#include<functional>
namespace Storage {

class BackgroundProcessingPool;

class MergeTreeDataMerger {
public:
    using AllowedMergingPredicate = std::function<bool (const MergeTreeData::DataPartPtr &, const MergeTreeData::DataPartPtr &)>;

    MergeTreeDataMerger(MergeTreeData & data_, const BackgroundProcessingPool & pool_);

    /** Get maximum total size of parts to do merge, at current moment of time.
      * It depends on number of free threads in background_pool and amount of free space in disk.
      */
    size_t getMaxPartsSizeForMerge();

    /** For explicitly passed size of pool and number of used tasks.
      * This method could be used to calculate threshold depending on number of tasks in replication queue.
      */
    size_t getMaxPartsSizeForMerge(size_t pool_size, size_t pool_used);

    bool selectPartsToMerge(
        MergeTreeData::DataPartsVector & what,
        std::string & merged_name,
        bool aggressive,
        size_t max_total_size_to_merge,
        const AllowedMergingPredicate & can_merge);

    bool selectAllPartsToMergeWithinPartition(
        MergeTreeData::DataPartsVector & what,
        std::string & merged_name,
        size_t available_disk_space,
        const AllowedMergingPredicate & can_merge,
        const std::string & partition_id);

    static size_t estimateDiskSpaceForMerge(const MergeTreeData::DataPartsVector & parts);

private:
    MergeTreeData & data;
    const BackgroundProcessingPool & pool;
};
}
