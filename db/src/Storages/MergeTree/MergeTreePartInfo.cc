#include<Storages/MergeTree/MergeTreePartInfo.h>
#include<IO/WriteBufferHelper.h>

namespace Storage {

std::string MergeTreePartInfo::getPartName(DataBase::DayNum_t left_date, DataBase::DayNum_t right_date, DataBase::Int64 left_id, DataBase::Int64 right_id, DataBase::UInt64 level)
{
    const auto & date_lut = DataBase::DateLUT::instance();
    /// Directory name for the part has form: `YYYYMMDD_YYYYMMDD_N_N_L`.
    unsigned left_date_id = date_lut.toNumYYYYMMDD(left_date);
    unsigned right_date_id = date_lut.toNumYYYYMMDD(right_date);
    IO::WriteBufferFromOwnString wb;
    IO::writeIntText(left_date_id, wb);
    IO::writeChar('_', wb);
    IO::writeIntText(right_date_id, wb);
    IO::writeChar('_', wb);
    IO::writeIntText(left_id, wb);
    IO::writeChar('_', wb);
    IO:: writeIntText(right_id, wb);
    IO::writeChar('_', wb);
    IO::writeIntText(level, wb);
    return wb.str();
}

}
