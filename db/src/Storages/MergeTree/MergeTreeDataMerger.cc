#include<Storages/MergeTree/MergeTreeDataMerger.h>
#include<Storages/BackgroundProcessingPool.h>


namespace Storage {

MergeTreeDataMerger::MergeTreeDataMerger(MergeTreeData& data_, const BackgroundProcessingPool& pool_)
    :data(data_),pool(pool_)
{

}

size_t MergeTreeDataMerger::estimateDiskSpaceForMerge(const MergeTreeData::DataPartsVector& parts)
{
	return 0;
}

size_t MergeTreeDataMerger::getMaxPartsSizeForMerge()
{
	return 0;
}

size_t MergeTreeDataMerger::getMaxPartsSizeForMerge(size_t pool_size, size_t pool_used)
{
	return 0;
}

bool MergeTreeDataMerger::selectAllPartsToMergeWithinPartition(MergeTreeData::DataPartsVector& what, std::string& merged_name, size_t available_disk_space, const AllowedMergingPredicate& can_merge, const std::string& partition_id)
{
	return true;
}
bool MergeTreeDataMerger::selectPartsToMerge(MergeTreeData::DataPartsVector& what, std::string& merged_name, bool aggressive, size_t max_total_size_to_merge, const AllowedMergingPredicate& can_merge)
{
	return true;
}
}
