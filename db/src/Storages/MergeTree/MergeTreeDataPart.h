#pragma once
#include<string>
#include<city.h>
#include<IO/ReadBuffer.h>
#include<IO/WriteBuffer.h>
#include<map>
#include<Storages/MergeTree/MergeTreePartInfo.h>
#include<Columns/IColumn.h>
#include<Core/NamesAndTypes.h>
#include<atomic>
// class SipHash;

namespace Storage {
class MergeTreeData;
struct MergeTreeDataPartChecksum {
    using uint128 = CityHash_v1_0_2::uint128;
    size_t file_size {};
    uint128 file_hash {};

    bool is_compressed = false;
    size_t uncompressed_size {};
    uint128 uncompressed_hash {};

    MergeTreeDataPartChecksum() {}
    MergeTreeDataPartChecksum(size_t file_size_, uint128 file_hash_) : file_size(file_size_), file_hash(file_hash_) {}
    MergeTreeDataPartChecksum(size_t file_size_, uint128 file_hash_, size_t uncompressed_size_, uint128 uncompressed_hash_)
        : file_size(file_size_), file_hash(file_hash_), is_compressed(true),
          uncompressed_size(uncompressed_size_), uncompressed_hash(uncompressed_hash_) {}

    void checkEqual(const MergeTreeDataPartChecksum & rhs, bool have_uncompressed, const std::string & name) const;
    void checkSize(const std::string & path) const;
};

struct MergeTreeDataPartChecksums {
    using Checksum = MergeTreeDataPartChecksum;

    /// The order is important.
    using FileChecksums = std::map<std::string, Checksum>;
    FileChecksums files;

    void addFile(const std::string & file_name, size_t file_size, Checksum::uint128 file_hash);

    void add(MergeTreeDataPartChecksums && rhs_checksums);

    /// Checks that the set of columns and their checksums are the same. If not, throws an exception.
    /// If have_uncompressed, for compressed files it compares the checksums of the decompressed data. Otherwise, it compares only the checksums of the files.
    void checkEqual(const MergeTreeDataPartChecksums & rhs, bool have_uncompressed) const;

    /// Checks that the directory contains all the needed files of the correct size. Does not check the checksum.
    void checkSizes(const std::string & path) const;

    /// Serializes and deserializes in human readable form.
    bool read(IO::ReadBuffer & in); /// Returns false if the checksum is too old.
    bool read_v2(IO::ReadBuffer & in);
    bool read_v3(IO::ReadBuffer & in);
    bool read_v4(IO::ReadBuffer & in);
    void write(IO::WriteBuffer & out) const;

    bool empty() const
    {
        return files.empty();
    }

    /// Checksum from the set of checksums of .bin files.
//     void summaryDataChecksum(SipHash & hash) const;

    std::string toString() const;
    static MergeTreeDataPartChecksums parse(const std::string & s);
};

struct MergeTreeDataPart
{
    using Checksums = MergeTreeDataPartChecksums;
    using Checksum = MergeTreeDataPartChecksums::Checksum;

    MergeTreeDataPart(MergeTreeData & storage_) : storage(storage_) {}
    MergeTreePartInfo info;
    std::string name;
    mutable std::string relative_path;
    bool is_temp = false;
    MergeTreeData& storage;

    using Index = DataBase::Columns;
    Index index;
	
	DataBase::DayNum_t min_date;
    DataBase::DayNum_t max_date;
	
	size_t size = 0;            
	
	DataBase::NamesAndTypesList columns;
	
	time_t modification_time = 0;

	using ColumnToSize = std::map<std::string, size_t>;
	
	Checksums checksums;
	
	 std::atomic<size_t> size_in_bytes {0};  /// size in bytes, 0 - if not counted;
                                            ///  is used from several threads without locks (it is changed with ALTER).
	 
	 mutable time_t remove_time = std::numeric_limits<time_t>::max(); /// When the part is removed from the working set.
	
    std::string getFullPath() const;
	
	/// Makes checks and move part to new directory
    /// Changes only relative_dir_name, you need to update other metadata (name, is_temp) explicitly
    void renameTo(const std::string & new_relative_path, bool remove_new_dir_if_exists = true) const;
	
	bool contains(const MergeTreeDataPart & other) const { return info.contains(other.info); }
	
	static size_t calcTotalSize(const std::string & from);
};

}
