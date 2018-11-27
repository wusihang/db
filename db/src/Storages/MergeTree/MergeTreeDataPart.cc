#include<Storages/MergeTree/MergeTreeDataPart.h>
#include<Poco/Exception.h>
#include<Poco/File.h>
#include<IO/WriteBufferHelper.h>
#include<Storages/MergeTree/MergeTreeData.h>
#include<IO/CompressedWriteBuffer.h>
#include<CommonUtil/LoggerUtil.h>
namespace ErrorCodes
{
extern const int CHECKSUM_DOESNT_MATCH;
extern const int FILE_DOESNT_EXIST;
extern const int NO_FILE_IN_DATA_PART;
extern const int EXPECTED_END_OF_FILE;
extern const int BAD_SIZE_OF_FILE_IN_DATA_PART;
extern const int FORMAT_VERSION_TOO_OLD;
extern const int UNKNOWN_FORMAT;
extern const int UNEXPECTED_FILE_IN_DATA_PART;
extern const int LOGICAL_ERROR;
extern const int DIRECTORY_ALREADY_EXISTS;
}

namespace Storage {

void MergeTreeDataPartChecksum::checkEqual(const MergeTreeDataPartChecksum& rhs, bool have_uncompressed, const std::string& name) const
{
    if (is_compressed && have_uncompressed)
    {
        if (!rhs.is_compressed)
            throw Poco::Exception("No uncompressed checksum for file " + name, ErrorCodes::CHECKSUM_DOESNT_MATCH);
        if (rhs.uncompressed_size != uncompressed_size)
            throw Poco::Exception("Unexpected uncompressed size of file " + name + " in data part", ErrorCodes::BAD_SIZE_OF_FILE_IN_DATA_PART);
        if (rhs.uncompressed_hash != uncompressed_hash)
            throw Poco::Exception("Checksum mismatch for uncompressed file " + name + " in data part", ErrorCodes::CHECKSUM_DOESNT_MATCH);
        return;
    }
    if (rhs.file_size != file_size)
        throw Poco::Exception("Unexpected size of file " + name + " in data part", ErrorCodes::BAD_SIZE_OF_FILE_IN_DATA_PART);
    if (rhs.file_hash != file_hash)
        throw Poco::Exception("Checksum mismatch for file " + name + " in data part", ErrorCodes::CHECKSUM_DOESNT_MATCH);
}

void MergeTreeDataPartChecksum::checkSize(const std::string& path) const
{
    Poco::File file(path);
    if (!file.exists())
        throw Poco::Exception(path + " doesn't exist", ErrorCodes::FILE_DOESNT_EXIST);
    size_t size = file.getSize();
    if (size != file_size)
        throw Poco::Exception(path + " has unexpected size: " + IO::toString(size) + " instead of " + IO::toString(file_size),
                              ErrorCodes::BAD_SIZE_OF_FILE_IN_DATA_PART);
}


std::string MergeTreeDataPart::getFullPath() const
{
    if (relative_path.empty())
        throw Poco::Exception("Part relative_path cannot be empty. This is bug.", ErrorCodes::LOGICAL_ERROR);
    return storage.full_path + relative_path + "/";
}

size_t MergeTreeDataPart::calcTotalSize(const std::string& from)
{
    Poco::File cur(from);
    if (cur.isFile())
        return cur.getSize();
    std::vector<std::string> files;
    cur.list(files);
    size_t res = 0;
    for (size_t i = 0; i < files.size(); ++i)
        res += calcTotalSize(from + files[i]);
    return res;
}

void MergeTreeDataPartChecksums::write(IO::WriteBuffer& to) const
{
    IO::writeString("checksums format version: 4\n", to);

    IO::CompressedWriteBuffer out(to, IO::CompressionMethod::NONE, 1 << 16);
    IO::writeVarUInt(files.size(), out);

    for (const auto & it : files)
    {
        const std::string & name = it.first;
        const Checksum & sum = it.second;

        IO::writeBinary(name, out);
        IO::writeVarUInt(sum.file_size, out);
        IO::writePODBinary(sum.file_hash, out);
        IO::writeBinary(sum.is_compressed, out);

        if (sum.is_compressed)
        {
            IO::writeVarUInt(sum.uncompressed_size, out);
            IO::writePODBinary(sum.uncompressed_hash, out);
        }
    }
}


void MergeTreeDataPart::renameTo(const std::string& new_relative_path, bool remove_new_dir_if_exists) const
{
    std::string from = getFullPath();
    std::string to = storage.full_path + new_relative_path + "/";

    Poco::File from_file(from);
    if (!from_file.exists())
        throw Poco::Exception("Part directory " + from + " doesn't exists. Most likely it is logical error.", ErrorCodes::FILE_DOESNT_EXIST);

    Poco::File to_file(to);
    if (to_file.exists())
    {
        if (remove_new_dir_if_exists)
        {
            std::vector<std::string> files;
            Poco::File(from).list(files);

            LOG_WARNING(storage.log, "Part directory " << to << " already exists"
                        << " and contains " << files.size() << " files. Removing it.");

            to_file.remove(true);
        }
        else
        {
            throw Poco::Exception("part directory " + to + " already exists", ErrorCodes::DIRECTORY_ALREADY_EXISTS);
        }
    }
    from_file.setLastModified(Poco::Timestamp::fromEpochTime(time(0)));
    from_file.renameTo(to);
    relative_path = new_relative_path;
}


}
