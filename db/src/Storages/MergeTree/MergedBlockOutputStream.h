#pragma once
#include<Streams/IBlockOutputStream.h>
#include<IO/WriteBufferFromFile.h>
#include<Storages/MergeTree/MergeTreeData.h>
#include<IO/HashingWriteBuffer.h>
#include<IO/CompressedWriteBuffer.h>

namespace IO {

class IMergedBlockOutputStream : public IBlockOutputStream
{
public:
    IMergedBlockOutputStream(
        Storage::MergeTreeData & storage_,
        size_t min_compress_block_size_,
        size_t max_compress_block_size_,
        size_t aio_threshold_,
        CompressionMethod compression_method_	);

protected:
    using OffsetColumns = std::set<std::string>;

    struct ColumnStream
    {
        ColumnStream(
            const std::string & escaped_column_name_,
            const  std::string & data_path,
            const std::string & data_file_extension_,
            const std::string & marks_path,
            const std::string & marks_file_extension_,
            size_t max_compress_block_size,
            CompressionMethod compression_method,
            size_t estimated_size,
            size_t aio_threshold);

        std::string escaped_column_name;
        std::string data_file_extension;
        std::string marks_file_extension;

        /// compressed -> compressed_buf -> plain_hashing -> plain_file
        std::unique_ptr<IO::WriteBufferFromFileBase> plain_file;
        HashingWriteBuffer plain_hashing;
        CompressedWriteBuffer compressed_buf;
        HashingWriteBuffer compressed;

        /// marks -> marks_file
        IO::WriteBufferFromFile marks_file;
        HashingWriteBuffer marks;

        void finalize();

        void sync();

        void addToChecksums(Storage::MergeTreeData::DataPart::Checksums & checksums);
    };

    using ColumnStreams = std::map<std::string, std::unique_ptr<ColumnStream>>;

    void addStream(const std::string & path, const std::string & name, const DataBase::IDataType & type, size_t estimated_size,
                   size_t level, const std::string & filename, bool skip_offsets);

    /// Write data of one column.
    void writeData(const std::string & name, const DataBase::IDataType & type, const DataBase::IColumn & column, OffsetColumns & offset_columns,
                   size_t level, bool skip_offsets);

    Storage::MergeTreeData & storage;

    ColumnStreams column_streams;

    /// The offset to the first row of the block for which you want to write the index.
    size_t index_offset = 0;

    size_t min_compress_block_size;
    size_t max_compress_block_size;

    size_t aio_threshold;

    CompressionMethod compression_method;

private:
    /// Internal version of writeData.
    void writeDataImpl(const std::string & name, const DataBase::IDataType & type, const DataBase::IColumn & column,
                       OffsetColumns & offset_columns, size_t level, bool write_array_data, bool skip_offsets);
};


/** To write one part.
  * The data refers to one partition, and is written in one part.
  */
class MergedBlockOutputStream : public IMergedBlockOutputStream
{
public:
    MergedBlockOutputStream(
        Storage::MergeTreeData & storage_,
        std::string part_path_,
        const DataBase::NamesAndTypesList & columns_list_,
        CompressionMethod compression_method);

    MergedBlockOutputStream(
        Storage::MergeTreeData & storage_,
        std::string part_path_,
        const DataBase::NamesAndTypesList & columns_list_,
        CompressionMethod compression_method,
        const Storage::MergeTreeData::DataPart::ColumnToSize & merged_column_to_size_,
        size_t aio_threshold_);

    std::string getPartPath() const;

    /// If the data is pre-sorted.
    void write(const Block & block) override;

    /** If the data is not sorted, but we have previously calculated the permutation, after which they will be sorted.
      * This method is used to save RAM, since you do not need to keep two blocks at once - the original one and the sorted one.
      */
    void writeWithPermutation(const Block & block, const DataBase::IColumn::Permutation * permutation);

    void writeSuffix() override;

    Storage::MergeTreeData::DataPart::Checksums writeSuffixAndGetChecksums(
        const DataBase::NamesAndTypesList & total_column_list,
        Storage::MergeTreeData::DataPart::Checksums * additional_column_checksums = nullptr);

    Storage:: MergeTreeData::DataPart::Checksums writeSuffixAndGetChecksums();

    Storage::MergeTreeData::DataPart::Index & getIndex();

    /// How many marks are already written.
    size_t marksCount();

private:
    void init();

    /** If `permutation` is given, it rearranges the values in the columns when writing.
      * This is necessary to not keep the whole block in the RAM to sort it.
      */
    void writeImpl(const Block & block, const DataBase::IColumn::Permutation * permutation);

private:
    DataBase::NamesAndTypesList columns_list;
    std::string part_path;

    size_t marks_count = 0;

    std::unique_ptr<WriteBufferFromFile> index_file_stream;
//     std::unique_ptr<HashingWriteBuffer> index_stream;
    Storage::MergeTreeData::DataPart::Index index_columns;

    std::unique_ptr<HashingWriteBuffer> index_stream;
};
}
