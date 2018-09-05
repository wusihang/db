#pragma once
#include<string>
#include<city.h>
#include<IO/ReadBuffer.h>
#include<IO/WriteBuffer.h>
#include<map>
#include<Storages/MergeTree/MergeTreePartInfo.h>
class SipHash;

namespace Storage {

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

struct MergeTreeDataPartChecksums{
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
    void summaryDataChecksum(SipHash & hash) const;

    std::string toString() const;
    static MergeTreeDataPartChecksums parse(const std::string & s);
};

struct MergeTreeDataPart
{
	 MergeTreePartInfo info;
	 std::string name;
};

}
