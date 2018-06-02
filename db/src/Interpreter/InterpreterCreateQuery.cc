#include<Interpreter/InterpreterCreateQuery.h>
#include <Interpreter/Context.h>
#include<Parser/ASTCreateQuery.h>
#include <DataBases/DataBaseFactory.h>
#include <Storages/StorageFactory.h>
#include<Ext/typeid_cast.h>
#include<IO/WriteBufferFromFile.h>
#include<IO/WriteBufferHelper.h>
#include<Poco/File.h>
namespace DataBase {

IO::BlockIO InterpreterCreateQuery::execute()
{
    ASTCreateQuery & create = typeid_cast<ASTCreateQuery &>(*query_ptr);
    /// 如果库名不为空,但是表名为空,那么就是创建数据库,CREATE DATABASE
    if (!create.database.empty() && create.table.empty())
    {
        return createDatabase(create);
    } else {
        return createTable(create);
    }
}

IO::BlockIO InterpreterCreateQuery::createDatabase(ASTCreateQuery& create)
{
    std::string database_name = create.database;
    std::string path = context.getPath();
    std::string data_path = path + "data/" + database_name + "/";
    std::string metadata_path = path + "metadata/" + database_name + "/";
    Poco::File(metadata_path).createDirectory();
    Poco::File(data_path).createDirectory();

    std::string engine_name = "Ordinary";
    std::shared_ptr<IDataBase>  database = DatabaseFactory::get(engine_name,database_name,metadata_path,context);
    std::string metadata_file_tmp_path = path + "metadata/" + database_name + ".sql.tmp";
    std::string metadata_file_path = path + "metadata/" + database_name + ".sql";

    std::string statement = "create database " + database_name;
    IO::WriteBufferFromFile write_buf(metadata_file_tmp_path,statement.size(),O_WRONLY | O_CREAT | O_EXCL);
    IO::writeString(statement,write_buf);
    write_buf.sync();
    write_buf.next();
    try {
        context.addDatabase(database_name,database);
        Poco::File(metadata_file_tmp_path).renameTo(metadata_file_path);
    } catch(...) {
        Poco::File(metadata_file_tmp_path).remove();
        throw;
    }
    return {};
}

IO::BlockIO InterpreterCreateQuery::createTable(ASTCreateQuery& create)
{
    std::string path = context.getPath();
    std::string current_database = context.getCurrentDatabase();
    //如果未指定库名,那么就使用当前上下文的库
    std::string database_name = create.database.empty() ? current_database : create.database;
    std::string  table_name = create.table;
    std::string data_path = path + "data/" + database_name + "/";
    std::shared_ptr<Storage::IStorage> storage;
    //ddl语句级锁
    {
        std::unique_ptr<DDLGuard> guard;
        context.assertDatabaseExists(database_name);
        guard = context.getDDLGuardIfTableDoesntExist(database_name,table_name,"Table "+database_name+"."+table_name +"is creating now");
        if(!guard) {
            throw Poco::Exception("Table " + database_name + "." + table_name + " is already exists");
        }
        std::shared_ptr< DataBase::IDataBase > database = context.getDatabase(database_name);
        storage = StorageFactory::getStorage("MergeTree");
        database->createTable(table_name,storage,query_ptr);
    }
    storage->startup();
    return {};
}


}
