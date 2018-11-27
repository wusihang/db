#include<DataBases/DataBaseOrdinary.h>
#include <IO/WriteBufferFromFile.h>
#include<IO/WriteBufferHelper.h>
#include<IO/ReadBufferFromFile.h>
#include<IO/ReadHelper.h>
#include<CommonUtil/StringUtils.h>
#include<Poco/File.h>
#include<Interpreter/Context.h>
#include<Poco/DirectoryIterator.h>
#include<Poco/Logger.h>
#include<CommonUtil/LoggerUtil.h>
#include<iomanip>
#include<functional>
#include<DataBases/DataBaseCommon.h>
namespace ErrorCodes
{
extern const int TABLE_ALREADY_EXISTS;
extern const int INCORRECT_FILE_NAME;
extern const int CANNOT_CREATE_TABLE_FROM_METADATA;
}
static constexpr size_t METADATA_FILE_BUFFER_SIZE = 32768;
static constexpr size_t PRINT_MESSAGE_EACH_N_TABLES = 256;
static constexpr size_t TABLES_PARALLEL_LOAD_BUNCH_SIZE = 100;


static std::string getTableMetadataPath(const std::string & base_path, const std::string & table_name)
{
    return base_path + (StringUtils::endsWith(base_path, "/") ? "" : "/") + table_name + ".sql";
}

static void loadTable(DataBase::Context & context,    const std::string & database_metadata_path,    DataBase::DataBaseOrdinary & database,   const std::string & database_name,    const std::string & database_data_path,    const std::string & file_name)
{
    Poco::Logger* log = &Poco::Logger::get("loadTable");
    const std::string table_metadata_path = database_metadata_path + "/" + file_name;
    std::string s;
    {
        char in_buf[METADATA_FILE_BUFFER_SIZE];
        IO::ReadBufferFromFile in(table_metadata_path, METADATA_FILE_BUFFER_SIZE, -1, in_buf);
        IO:: readStringUntilEOF(s, in);
    }
    /** Empty files with metadata are generated after a rough restart of the server.
    * Remove these files to slightly reduce the work of the admins on startup.
    */
    if (s.empty())
    {
        LOG_ERROR(log, "File " << table_metadata_path << " is empty. Removing.");
        Poco::File(table_metadata_path).remove();
    } else {
        try
        {
            std::string table_name;
            std::shared_ptr<Storage::IStorage> table;
            std::tie(table_name, table) = DataBase::createTableFromDefinition(
                                              s, database_name, database_data_path, context, "in file " + table_metadata_path);
            database.attachTable(table_name, table);
        }
        catch (const Poco::Exception & e)
        {
            throw Poco::Exception("Cannot create table from metadata file " + table_metadata_path + ", error: " + e.displayText(),
                            ErrorCodes::CANNOT_CREATE_TABLE_FROM_METADATA);
        }
    }
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

void DataBase::DataBaseOrdinary::loadTables(DataBase::Context& context)
{
    using FileNames = std::vector<std::string>;
    FileNames file_names;
    log = &Poco::Logger::get("DatabaseOridinary(" + name + ")");

    Poco::DirectoryIterator dir_end;
    for (Poco::DirectoryIterator dir_it(path); dir_it != dir_end; ++dir_it)
    {
        /// For '.svn', '.gitignore' directory and similar.
        if (dir_it.name().at(0) == '.')
            continue;

        /// There are .sql.bak files - skip them.
        if (StringUtils::endsWith(dir_it.name(), ".sql.bak"))
            continue;

        /// There are files .sql.tmp - delete.
        if (StringUtils::endsWith(dir_it.name(), ".sql.tmp"))
        {
            LOG_INFO(log, "Removing file " << dir_it->path());
            Poco::File(dir_it->path()).remove();
            continue;
        }

        /// The required files have names like `table_name.sql`
        if (StringUtils::endsWith(dir_it.name(), ".sql"))
            file_names.push_back(dir_it.name());
        else
            throw Poco::Exception("Incorrect file extension: " + dir_it.name() + " in metadata directory " + path,
                                  ErrorCodes::INCORRECT_FILE_NAME);
    }
    /** Tables load faster if they are loaded in sorted (by name) order.
    * Otherwise (for the ext4 filesystem), `DirectoryIterator` iterates through them in some order,
    *  which does not correspond to order tables creation and does not correspond to order of their location on disk.
    */
    std::sort(file_names.begin(), file_names.end());
    size_t total_tables = file_names.size();
    LOG_INFO(log, "Total " << total_tables << " tables.");

    String data_path = context.getPath() + "data/" + name + "/";

    std::atomic<size_t> tables_processed {0};

    auto task_function = [&](FileNames::const_iterator begin, FileNames::const_iterator end)
    {
        for (FileNames::const_iterator it = begin; it != end; ++it)
        {
            const String & table = *it;
            /// Messages, so that it's not boring to wait for the server to load for a long time.
            if ((++tables_processed) % PRINT_MESSAGE_EACH_N_TABLES == 0)
            {
                LOG_INFO(log, std::fixed << std::setprecision(2) << tables_processed * 100.0 / total_tables << "%");
            }
            loadTable(context, path, *this, name, data_path, table);
        }
    };

    const size_t bunch_size = TABLES_PARALLEL_LOAD_BUNCH_SIZE;
    size_t num_bunches = (total_tables + bunch_size - 1) / bunch_size;

    for (size_t i = 0; i < num_bunches; ++i)
    {
        auto begin = file_names.begin() + i * bunch_size;
        auto end = (i + 1 == num_bunches)
                   ? file_names.end()
                   : (file_names.begin() + (i + 1) * bunch_size);
        auto task = std::bind(task_function, begin, end);
        task();
    }
    startupTables();
}


void DataBase::DataBaseOrdinary::startupTables()
{
    LOG_INFO(log, "Starting up tables.");
    std::atomic<size_t> tables_processed {0};
    size_t total_tables = tables.size();

    auto task_function = [&]( std::map<std::string,std::shared_ptr<Storage::IStorage> >::iterator begin,  std::map<std::string,std::shared_ptr<Storage::IStorage> >::iterator end)
    {
        for (std::map<std::string,std::shared_ptr<Storage::IStorage> >::iterator it = begin; it != end; ++it)
        {
            if ((++tables_processed) % PRINT_MESSAGE_EACH_N_TABLES == 0)
            {
                LOG_INFO(log, std::fixed << std::setprecision(2) << tables_processed * 100.0 / total_tables << "%");
            }
            it->second->startup();
        }
    };

    const size_t bunch_size = TABLES_PARALLEL_LOAD_BUNCH_SIZE;
    size_t num_bunches = (total_tables + bunch_size - 1) / bunch_size;

    std::map<std::string,std::shared_ptr<Storage::IStorage> >::iterator begin = tables.begin();
    for (size_t i = 0; i < num_bunches; ++i)
    {
        auto end = begin;
        if (i + 1 == num_bunches)
            end = tables.end();
        else
            std::advance(end, bunch_size);
        auto task = std::bind(task_function, begin, end);
        task();
        begin = end;
    }
}

