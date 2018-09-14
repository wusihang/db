#include<Storages/MergeTree/MergeTreeDataWriter.h>
#include<Columns/ColumnNumber.h>
#include<Ext/typeid_cast.h>

namespace Storage {
BlocksWithDateIntervals MergeTreeDataWriter::splitBlockIntoParts(const IO::Block& block)
{
	return {};
//     size_t rows = block.rows();
//     size_t columns = block.columns();
//     const DataBase::ColumnUInt16::Container_t & dates =
//         typeid_cast<const DataBase::ColumnUInt16 &>(*block.getByName(data.date_column_name).column).getData();
}


MergeTreeData::MutableDataPartPtr MergeTreeDataWriter::writeTempPart(BlockWithDateInterval& block)
{
	return nullptr;
}

}
