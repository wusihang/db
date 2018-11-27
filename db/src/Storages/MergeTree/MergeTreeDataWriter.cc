#include<Storages/MergeTree/MergeTreeDataWriter.h>
#include<Columns/ColumnNumber.h>
#include<Ext/typeid_cast.h>
#include<Core/DateLUT.h>
#include<Poco/File.h>
#include<CommonUtil/LoggerUtil.h>
#include<Interpreter/SortBlock.h>
#include<Storages/MergeTree/MergedBlockOutputStream.h>
#include<IO/WriteBufferHelper.h>
#include<ctime>
namespace Storage {
BlocksWithDateIntervals MergeTreeDataWriter::splitBlockIntoParts(const IO::Block& block)
{
    const auto & date_lut = DataBase::DateLUT::instance();
    size_t rows = block.rows();
    size_t columns = block.columns();
    //根据日期列名称，获取对应的日期数据，日期是16位存储的数字
    const DataBase::ColumnUInt16::Container_t & dates =
        typeid_cast<const DataBase::ColumnUInt16 &>(*block.getByName(data.date_column_name).column).getData();
    //以下计算分别求得最大日期和最小日期
    DataBase::UInt16 min_date = std::numeric_limits< DataBase::UInt16>::max();
    DataBase::UInt16 max_date = std::numeric_limits< DataBase::UInt16>::min();
    for (auto it = dates.begin(); it != dates.end(); ++it)
    {
        if (*it < min_date)
            min_date = *it;
        if (*it > max_date)
            max_date = *it;
    }

    BlocksWithDateIntervals res;

    //求得最小日期、最大日期所在月份的第一天
    DataBase::UInt16 min_month = date_lut.toFirstDayNumOfMonth(DataBase::DayNum_t(min_date));
    DataBase::UInt16 max_month = date_lut.toFirstDayNumOfMonth(DataBase::DayNum_t(max_date));
    //如果最大日期和最小日期同处一个月，那么就不继续拆分
    if (min_month == max_month)
    {
        res.push_back(BlockWithDateInterval(block, min_date, max_date));
        return res;
    }
    using BlocksByMonth = std::map<DataBase::UInt16, BlockWithDateInterval *>;
    BlocksByMonth blocks_by_month;

    std::vector<DataBase::IColumn *> src_columns(columns);
    for (size_t i = 0; i < columns; ++i)
        src_columns[i] = block.safeGetByPosition(i).column.get();

    for (size_t i = 0; i < rows; ++i)
    {
        DataBase::UInt16 month = date_lut.toFirstDayNumOfMonth(DataBase::DayNum_t(dates[i]));

        BlockWithDateInterval *& block_for_month = blocks_by_month[month];
        if (!block_for_month)
        {
            block_for_month = &*res.insert(res.end(), BlockWithDateInterval());
            block_for_month->block = block.cloneEmpty();
        }
        block_for_month->updateDates(dates[i]);
        for (size_t j = 0; j < columns; ++j)
            block_for_month->block.getByPosition(j).column->insertFrom(*src_columns[j], i);
    }
    return res;
}


MergeTreeData::MutableDataPartPtr MergeTreeDataWriter::writeTempPart(BlockWithDateInterval& block_with_date)
{
    IO::Block & block = block_with_date.block;
    DataBase::UInt16 min_date = block_with_date.min_date;
    DataBase::UInt16 max_date = block_with_date.max_date;

    const auto & date_lut = DataBase::DateLUT::instance();
    DataBase::DayNum_t min_month = date_lut.toFirstDayNumOfMonth(DataBase::DayNum_t(min_date));
    DataBase::DayNum_t max_month = date_lut.toFirstDayNumOfMonth(DataBase::DayNum_t(max_date));
    if (min_month != max_month)
        throw Poco::Exception("Logical error: part spans more than one month.");

    //按粒度计算part个数，向上取整
    size_t part_size = (block.rows() + data.index_granularity - 1) / data.index_granularity;
    static const std::string TMP_PREFIX = "tmp_insert_";
    DataBase::Int64 temp_index = data.insert_increment.get();
    std::string part_name = MergeTreePartInfo::getPartName(DataBase::DayNum_t(min_date), DataBase::DayNum_t(max_date), temp_index, temp_index, 0);

    MergeTreeData::MutableDataPartPtr new_data_part = std::make_shared<MergeTreeData::DataPart>(data);
    new_data_part->name = part_name;
    new_data_part->relative_path = TMP_PREFIX + part_name;
    new_data_part->is_temp = true;

    /// The name could be non-unique in case of stale files from previous runs.
    std::string full_path = new_data_part->getFullPath();
    Poco::File dir(full_path);
    if (dir.exists())
    {
        LOG_WARNING(log, "Removing old temporary directory " + full_path);
        dir.remove(true);
    }
    dir.createDirectories();
    data.getPrimaryExpression()->execute(block);
    DataBase::SortDescription sort_descr = data.getSortDescription();

    DataBase::IColumn::Permutation * perm_ptr = nullptr;
    DataBase::IColumn::Permutation perm;
    if (!DataBase::isAlreadySorted(block, sort_descr))
    {
        DataBase::stableGetPermutation(block, sort_descr, perm);
        perm_ptr = &perm;
    }
    auto columns = data.getColumnsList().filter(block.getColumnsList().getNames());
    IO::MergedBlockOutputStream out(data, new_data_part->getFullPath(), columns,IO::CompressionMethod::NONE);
    out.writePrefix();
    out.writeWithPermutation(block,perm_ptr);
    MergeTreeData::DataPart::Checksums checksums = out.writeSuffixAndGetChecksums();
    new_data_part->info.partition_id = IO::toString(date_lut.toNumYYYYMM(min_month));
    new_data_part->info.min_block = temp_index;
    new_data_part->info.max_block = temp_index;
    new_data_part->info.level = 0;
    new_data_part->min_date = DataBase::DayNum_t(min_date);
    new_data_part->max_date = DataBase::DayNum_t(max_date);
    new_data_part->size = part_size;
    new_data_part->modification_time = ::time(nullptr);
    new_data_part->columns = columns;
    new_data_part->checksums = checksums;
    new_data_part->index.swap(out.getIndex());
    new_data_part->size_in_bytes = MergeTreeData::DataPart::calcTotalSize(new_data_part->getFullPath());
    return new_data_part;
}

}
