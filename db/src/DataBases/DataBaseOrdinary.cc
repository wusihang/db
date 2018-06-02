#include<DataBases/DataBaseOrdinary.h>
#include <IO/WriteBufferFromFile.h>
#include<IO/WriteBufferHelper.h>
#include<CommonUtil/StringUtils.h>
#include<Poco/File.h>
namespace ErrorCodes
{
extern const int TABLE_ALREADY_EXISTS;
}



static std::string getTableMetadataPath(const std::string & base_path, const std::string & table_name)
{
    return base_path + (StringUtils::endsWith(base_path, "/") ? "" : "/") + table_name + ".sql";
}

void DataBase::DataBaseOrdinary::createTable(const std::string& table_name, std::shared_ptr< Storage::IStorage >& storage,const std::shared_ptr<IAST>& query)
{
    //锁检查,如果同时建表,那么会产生竞争,但是DDLGuard已经做了相关保护
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (tables.count(table_name)) {
            throw Poco::Exception("Table " + name + "." + table_name + " already exists.", ErrorCodes::TABLE_ALREADY_EXISTS);
        }
    }
    std::string table_meta_path = getTableMetadataPath(path,table_name);
    std::string table_meta_tmp_path = table_meta_path + ".tmp";
	 size_t size = query->range.second - query->range.first;
	//可写,如果不存在就创建, 如果已经存在就报错
    IO::WriteBufferFromFile wbuf(table_meta_tmp_path,size,O_WRONLY | O_CREAT | O_EXCL);
    IO::writeString(query->range.first,size,wbuf);
    wbuf.next();
    wbuf.sync();
    wbuf.close();
    try {
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (!tables.emplace(table_name, storage).second)
                throw Poco::Exception("Table " + name + "." + table_name + " already exists.", ErrorCodes::TABLE_ALREADY_EXISTS);
        }
        Poco::File(table_meta_tmp_path).renameTo(table_meta_path);
    } catch(...) {
        Poco::File(table_meta_tmp_path).remove();
        throw;
    }
}
