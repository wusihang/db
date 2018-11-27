#pragma once
#include<Core/DateLUT.h>
#include<tuple>
namespace Storage {
struct MergeTreePartInfo
{
    std::string partition_id;
    Poco::Int64 min_block;
    Poco::Int64 max_block;
    Poco::UInt32 level;

    bool operator<(const MergeTreePartInfo & rhs) const
    {
        return std::forward_as_tuple(partition_id, min_block, max_block, level) < std::forward_as_tuple(rhs.partition_id, rhs.min_block, rhs.max_block, rhs.level);
    }

    /// Contains another part (obtained after merging another part with some other)
    bool contains(const MergeTreePartInfo & rhs) const
    {
        return partition_id == rhs.partition_id        /// Parts for different partitions are not merged
               && min_block <= rhs.min_block
               && max_block >= rhs.max_block
               && level >= rhs.level;
    }
    
    static std::string getPartName(DataBase::DayNum_t left_date, DataBase::DayNum_t right_date, DataBase::Int64 left_id, DataBase::Int64 right_id, DataBase::UInt64 level);
};
}
