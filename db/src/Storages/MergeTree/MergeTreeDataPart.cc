#include<Storages/MergeTree/MergeTreeDataPart.h>
#include<Poco/Exception.h>
#include<Poco/File.h>
#include<IO/WriteBufferHelper.h>
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


}
