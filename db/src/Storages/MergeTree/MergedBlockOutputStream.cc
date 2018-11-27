#include<Storages/MergeTree/MergedBlockOutputStream.h>
#include<Poco/File.h>
#include<Ext/std_ext.h>
#include<IO/WriteBufferHelper.h>
namespace ErrorCodes {
extern const int BAD_ARGUMENTS;
extern const int NOT_IMPLEMENTED;
}

namespace IO {
namespace
{
constexpr auto DATA_FILE_EXTENSION = ".bin";
constexpr auto MARKS_FILE_EXTENSION = ".mrk";
}

static WriteBufferFromFileBase * createWriteBufferFromFileBase(const std::string & filename_, size_t estimated_size,
        size_t aio_threshold,  size_t buffer_size_ = DBMS_DEFAULT_BUFFER_SIZE,
        int flags_ = -1,
        mode_t mode = 0666,
        char * existing_memory_ = nullptr,
        size_t alignment = 0)
{
    if ((aio_threshold == 0) || (estimated_size < aio_threshold))
    {
        return new WriteBufferFromFile(filename_, buffer_size_, flags_, mode, existing_memory_, alignment);
    }
    else
    {
        throw Poco::Exception("AIO is not implemented yet ", ErrorCodes::NOT_IMPLEMENTED);
    }
}

IMergedBlockOutputStream::IMergedBlockOutputStream(Storage::MergeTreeData& storage_, size_t min_compress_block_size_, size_t max_compress_block_size_, size_t aio_threshold_,CompressionMethod compression_method_)
    : storage(storage_), min_compress_block_size(min_compress_block_size_),max_compress_block_size(max_compress_block_size_),aio_threshold(aio_threshold_),compression_method(compression_method_)
{

}

void IMergedBlockOutputStream::addStream(const std::string& path, const std::string& name, const DataBase::IDataType& type, size_t estimated_size, size_t level, const std::string& filename, bool skip_offsets)
{
    std::string escaped_column_name;
    if (filename.size())
        escaped_column_name = filename;
    else
        escaped_column_name = name;
    column_streams[name] = std_ext::make_unique<ColumnStream>(
                               escaped_column_name,
                               path + escaped_column_name, DATA_FILE_EXTENSION,
                               path + escaped_column_name, MARKS_FILE_EXTENSION,
                               max_compress_block_size,
                               compression_method,
                               estimated_size,
                               aio_threshold);
}

void IMergedBlockOutputStream::ColumnStream::addToChecksums(Storage::MergeTreeDataPart::Checksums& checksums)
{
    std::string name = escaped_column_name;
    checksums.files[name + data_file_extension].is_compressed = true;
    checksums.files[name + data_file_extension].uncompressed_size = compressed.count();
    checksums.files[name + data_file_extension].uncompressed_hash = compressed.getHash();
    checksums.files[name + data_file_extension].file_size = plain_hashing.count();
    checksums.files[name + data_file_extension].file_hash = plain_hashing.getHash();
    checksums.files[name + marks_file_extension].file_size = marks.count();
    checksums.files[name + marks_file_extension].file_hash = marks.getHash();
}

IMergedBlockOutputStream::ColumnStream::ColumnStream(const std::string& escaped_column_name_, const std::string& data_path, const std::string& data_file_extension_, const std::string& marks_path, const std::string& marks_file_extension_, size_t max_compress_block_size,
        CompressionMethod compression_method,size_t estimated_size, size_t aio_threshold)
    :escaped_column_name(escaped_column_name_),data_file_extension {data_file_extension_},marks_file_extension {marks_file_extension_},
plain_file(createWriteBufferFromFileBase(data_path + data_file_extension, estimated_size, aio_threshold, max_compress_block_size)),
plain_hashing(*plain_file), compressed_buf(plain_hashing, compression_method), compressed(compressed_buf),
marks_file(marks_path + marks_file_extension, 4096, O_TRUNC | O_CREAT | O_WRONLY),marks(marks_file)
{

}

void IMergedBlockOutputStream::ColumnStream::finalize()
{
    compressed.next();
    plain_file->next();
    marks.next();
}

void IMergedBlockOutputStream::ColumnStream::sync()
{
    plain_file->sync();
    marks_file.sync();
}

void IMergedBlockOutputStream::writeData(const std::string& name, const DataBase::IDataType& type, const DataBase::IColumn& column, IMergedBlockOutputStream::OffsetColumns& offset_columns, size_t level, bool skip_offsets)
{
    writeDataImpl(name, type, column, offset_columns, level, false, skip_offsets);
}
void IMergedBlockOutputStream::writeDataImpl(const std::string& name, const DataBase::IDataType& type, const DataBase::IColumn& column, IMergedBlockOutputStream::OffsetColumns& offset_columns, size_t level, bool write_array_data, bool skip_offsets)
{
    size_t size = column.size();
    ColumnStream & stream = *column_streams[name];
    size_t prev_mark = 0;
    while (prev_mark < size)
    {
        size_t limit = 0;

        /// If there is `index_offset`, then the first mark goes not immediately, but after this number of rows.
        if (prev_mark == 0 && index_offset != 0)
            limit = index_offset;
        else
        {
            limit = storage.index_granularity;

            /// There could already be enough data to compress into the new block.
            if (stream.compressed.offset() >= min_compress_block_size)
                stream.compressed.next();

            writeIntBinary(stream.plain_hashing.count(), stream.marks);
            writeIntBinary(stream.compressed.offset(), stream.marks);
        }

        type.serializeBinaryBulk(column, stream.compressed, prev_mark, limit);

        /// So that instead of the marks pointing to the end of the compressed block, there were marks pointing to the beginning of the next one.
        stream.compressed.nextIfAtEnd();

        prev_mark += limit;
    }

}



// MergedBlockOutputStream part
MergedBlockOutputStream::MergedBlockOutputStream(Storage::MergeTreeData& storage_, std::string part_path_, const DataBase::NamesAndTypesList& columns_list_, CompressionMethod compression_method,const Storage::MergeTreeDataPart::ColumnToSize& merged_column_to_size_, size_t aio_threshold_)
    :IMergedBlockOutputStream(storage_,storage_.context.getSettings().min_compress_block_size,storage_.context.getSettings().max_compress_block_size,storage_.context.getSettings().min_bytes_to_use_direct_io,compression_method)
    ,columns_list(columns_list_), part_path(part_path_)
{
    init();
    for (const auto & it : columns_list)
    {
        size_t estimated_size = 0;
        if (aio_threshold > 0)
        {
            auto it2 = merged_column_to_size_.find(it.name);
            if (it2 != merged_column_to_size_.end())
                estimated_size = it2->second;
        }
        addStream(part_path, it.name, *(it.type), estimated_size, 0, "", false);
    }
}

MergedBlockOutputStream::MergedBlockOutputStream(Storage::MergeTreeData& storage_, std::string part_path_, const DataBase::NamesAndTypesList& columns_list_,CompressionMethod compression_method)
    :IMergedBlockOutputStream(storage_,storage_.context.getSettings().min_compress_block_size,storage_.context.getSettings().max_compress_block_size,storage_.context.getSettings().min_bytes_to_use_direct_io,compression_method),
     columns_list(columns_list_), part_path(part_path_)
{
    init();
    for (const auto & it : columns_list)
    {
        addStream(part_path, it.name, *(it.type), 0, 0, "", false);
    }
}

void MergedBlockOutputStream::write(const Block& block)
{
    writeImpl(block, nullptr);
}

Storage::MergeTreeDataPart::Index& MergedBlockOutputStream::getIndex()
{
    return index_columns;
}

std::string MergedBlockOutputStream::getPartPath() const
{
    return part_path;
}

void MergedBlockOutputStream::init()
{
    Poco::File(part_path).createDirectories();
    index_file_stream = std_ext::make_unique<WriteBufferFromFile>(
                            part_path + "primary.idx", DBMS_DEFAULT_BUFFER_SIZE, O_TRUNC | O_CREAT | O_WRONLY);
    index_stream = std_ext::make_unique<HashingWriteBuffer>(*index_file_stream);
}


size_t MergedBlockOutputStream::marksCount()
{
    return marks_count;
}

void MergedBlockOutputStream::writeImpl(const Block& block, const DataBase::IColumn::Permutation* permutation)
{
    size_t rows = block.rows();
    OffsetColumns offset_columns;
    auto sort_description = storage.getSortDescription();
    /// Here we will add the columns related to the Primary Key, then write the index.
    std::vector<DataBase::ColumnWithTypeAndName> primary_columns(sort_description.size());
    std::map<std::string, size_t> primary_columns_name_to_position;
    for (size_t i = 0, size = sort_description.size(); i < size; ++i)
    {
        const DataBase::SortColumnDescription& descr = sort_description[i];

        std::string name = !descr.column_name.empty()
                           ? descr.column_name
                           : block.safeGetByPosition(descr.column_number).name;

        if (!primary_columns_name_to_position.emplace(name, i).second)
            throw Poco::Exception("Primary key contains duplicate columns", ErrorCodes::BAD_ARGUMENTS);

        primary_columns[i] = !descr.column_name.empty()
                             ? block.getByName(descr.column_name)
                             : block.safeGetByPosition(descr.column_number);

        /// Reorder primary key columns in advance and add them to `primary_columns`.
        if (permutation)
        {
            primary_columns[i].column = primary_columns[i].column->permute(*permutation, 0);
        }
    }


    if(index_columns.empty()) {
        index_columns.resize(sort_description.size());
        for (size_t i = 0, size = sort_description.size(); i < size; ++i)
        {
            index_columns[i] = primary_columns[i].column.get()->cloneEmpty();
        }
    }

    /// Now write the data.
    for (const auto & it : columns_list)
    {
        const DataBase::ColumnWithTypeAndName & column = block.getByName(it.name);

        if (permutation)
        {
            auto primary_column_it = primary_columns_name_to_position.find(it.name);
            if (primary_columns_name_to_position.end() != primary_column_it)
            {
                writeData(column.name, *column.type, *primary_columns[primary_column_it->second].column, offset_columns, 0, false);
            }
            else
            {
                /// We rearrange the columns that are not included in the primary key here; Then the result is released - to save RAM.
                DataBase::ColumnPtr permutted_column = column.column->permute(*permutation, 0);
                writeData(column.name, *column.type, *permutted_column, offset_columns, 0, false);
            }
        }
        else
        {
            writeData(column.name, *column.type, *column.column, offset_columns, 0, false);
        }
    }

    for (size_t i = index_offset; i < rows; i += storage.index_granularity)
    {
        for (size_t j = 0, size = primary_columns.size(); j < size; ++j)
        {
            const DataBase::IColumn & primary_column = *primary_columns[j].column.get();
            index_columns[j].get()->insertFrom(primary_column, i);
            primary_columns[j].type.get()->serializeBinary(primary_column, i, *index_stream);
        }
        ++marks_count;
    }


}

void MergedBlockOutputStream::writeSuffix()
{
}

Storage::MergeTreeDataPart::Checksums MergedBlockOutputStream::writeSuffixAndGetChecksums(const DataBase::NamesAndTypesList& total_column_list, Storage::MergeTreeDataPart::Checksums* additional_column_checksums)
{
    Storage::MergeTreeDataPart::Checksums  checksums;
    if (additional_column_checksums)
    {
        checksums = std::move(*additional_column_checksums);
    }
    {
        index_stream->next();
        checksums.files["primary.idx"].file_size = index_stream->count();
        checksums.files["primary.idx"].file_hash = index_stream->getHash();
        index_stream = nullptr;
    }

    for (ColumnStreams::iterator it = column_streams.begin(); it != column_streams.end(); ++it)
    {
        it->second->finalize();
        it->second->addToChecksums(checksums);
    }

    column_streams.clear();

    if (marks_count == 0)
    {
        /// A part is empty - all records are deleted.
        Poco::File(part_path).remove(true);
        checksums.files.clear();
        return checksums;
    }

    {
        /// Write a file with a description of columns.
        WriteBufferFromFile out(part_path + "columns.txt", 4096);
        total_column_list.writeText(out);
    }

    {
        /// Write file with checksums.
        WriteBufferFromFile out(part_path + "checksums.txt", 4096);
        checksums.write(out);
    }

    return checksums;
}
Storage::MergeTreeDataPart::Checksums MergedBlockOutputStream::writeSuffixAndGetChecksums()
{
    return writeSuffixAndGetChecksums(columns_list, nullptr);
}

void MergedBlockOutputStream::writeWithPermutation(const Block& block, const DataBase::IColumn::Permutation* permutation)
{
    writeImpl(block, permutation);
}

}
