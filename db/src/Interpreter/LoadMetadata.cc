#include<Interpreter/LoadMetadata.h>
#include<Interpreter/Context.h>
#include<Poco/DirectoryIterator.h>
#include <Ext/ThreadPool.h>
#include <Parser/ParseUtil.h>
#include<Ext/typeid_cast.h>
#include<Parser/ASTCreateQuery.h>
#include<Interpreter/InterpreterCreateQuery.h>
#include<IO/ReadHelper.h>
#include<IO/ReadBufferFromFile.h>
#include<Parser/ParseCreateQuery.h>

namespace DataBase {
static void executeCreateQuery(
    const std::string & query,
    Context & context,
    const std::string & database,
    const std::string & file_name,
    ThreadPool * pool,
    bool has_force_restore_data_flag)
{
    DataBase::ParserCreateQuery parser;
    std::shared_ptr<IAST> ast = ParseUtil::parseQuery(parser, query.data(), query.data() + query.size(), "in file " + file_name);
    DataBase::ASTCreateQuery & ast_create_query = typeid_cast< DataBase::ASTCreateQuery &>(*ast);
    ast_create_query.database = database;
    DataBase::InterpreterCreateQuery interpreter(ast, context);
    if(pool)
    {
        interpreter.setDatabaseLoadingThreadpool(*pool);
    }
    interpreter.execute();
}


static void loadDatabase(
    Context & context,
    const std::string & database,
    const std::string & database_path,
    ThreadPool * thread_pool,
    bool force_restore_data)
{
    /// There may exist .sql file with database creation statement.
    /// Or, if it is absent, then database with default engine is created.

    std::string database_attach_query;
    std::string database_metadata_file = database_path + ".sql";

    if (Poco::File(database_metadata_file).exists())
    {
        IO::ReadBufferFromFile in(database_metadata_file, 1024);
        IO::readStringUntilEOF(database_attach_query, in);
    }
    else
    {
        database_attach_query = "CREATE DATABASE " + database;
    }

    executeCreateQuery(database_attach_query, context, database, database_metadata_file, thread_pool, force_restore_data);
}


void loadMetadata(Context& context)
{
    std::string path = context.getPath() + "metadata";
    Poco::File force_restore_data_flag_file(context.getFlagsPath() + "force_restore_data");
    bool has_force_restore_data_flag = force_restore_data_flag_file.exists();
    std::map<std::string, std::string> databases;
//     ThreadPool thread_pool(5);
    Poco::DirectoryIterator dir_end;
    for (Poco::DirectoryIterator it(path); it != dir_end; ++it)
    {
        if (it->isDirectory() && it.name().at(0) != '.' && it.name() != "system") {
            databases.emplace(it.name(), it.path().toString());
        }
    }
    for (const auto & elem : databases)
    {
        loadDatabase(context, elem.first, elem.second, nullptr/*&thread_pool*/, has_force_restore_data_flag);
    }
//     thread_pool.wait();
    if (has_force_restore_data_flag)
    {
        force_restore_data_flag_file.remove();
    }
}

}

