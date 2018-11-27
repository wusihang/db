#pragma once
#include<limits>
#include<Poco/Types.h>
#include<Streams/Block.h>
#include<list>
#include<Storages/MergeTree/MergeTreeData.h>
#include<Poco/Logger.h>

namespace Storage {

struct BlockWithDateInterval
{
    IO::Block block;
    Poco::UInt16 min_date = std::numeric_limits<Poco::UInt16>::max(); /// For further updating, see updateDates method.
    Poco::UInt16 max_date = std::numeric_limits<Poco::UInt16>::min();

    BlockWithDateInterval() = default;
    BlockWithDateInterval(const IO::Block & block_, Poco::UInt16 min_date_, Poco::UInt16 max_date_)
        : block(block_), min_date(min_date_), max_date(max_date_) {}

    void updateDates(Poco::UInt16 date)
    {
        if (date < min_date)
            min_date = date;

        if (date > max_date)
            max_date = date;
    }
};

using BlocksWithDateIntervals = std::list<BlockWithDateInterval>;

class MergeTreeDataWriter {
public:
    MergeTreeDataWriter(MergeTreeData & data_) : data(data_), log(&Poco::Logger::get(data.getLogName() + " (Writer)")) {}

    /** Split the block to blocks, each of them must be written as separate part.
      *  (split rows by partition)
      * Works deterministically: if same block was passed, function will return same result in same order.
      */
    BlocksWithDateIntervals splitBlockIntoParts(const IO::Block & block);

    /** All rows must correspond to same partition.
      * Returns part with unique name starting with 'tmp_', yet not added to MergeTreeData.
      */
    MergeTreeData::MutableDataPartPtr writeTempPart(BlockWithDateInterval & block_with_date);

private:
    MergeTreeData & data;

    Poco::Logger * log;
};
}
