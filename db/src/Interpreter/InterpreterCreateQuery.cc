#include<Interpreter/InterpreterCreateQuery.h>
#include <Interpreter/Context.h>
#include<Parser/ASTCreateQuery.h>
#include <DataBases/DataBaseFactory.h>
#include <Storages/StorageFactory.h>
#include<Ext/typeid_cast.h>
#include<IO/WriteBufferFromFile.h>
#include<IO/WriteBufferHelper.h>
#include<Poco/File.h>
#include<Parser/ASTExpressionList.h>
#include<Parser/ASTColumnDeclaration.h>
#include <Parser/ASTFunction.h>
#include<Storages/ColumnDefault.h>
#include<DataTypes/DataTypeFactory.h>
#include<DataTypes/DataTypeNumber.h>
#include <set>
#include <Ext/typeid_cast.h>

namespace ErrorCodes {
extern const int  INCORRECT_QUERY;
extern const int  DUPLICATE_COLUMN;
extern const int  EMPTY_LIST_OF_COLUMNS_PASSED;
extern const int ENGINE_REQUIRED;
}



namespace DataBase {

using ColumnsAndDefaults = std::pair<NamesAndTypesList, ColumnDefaults>;
static ColumnsAndDefaults parseColumns(
    std::shared_ptr<IAST> expression_list, const Context & context) {
    auto & column_list_ast = typeid_cast<ASTExpressionList &>(*expression_list);
    NamesAndTypesList columns;
    ColumnDefaults defaults;
    std::vector<std::pair<NameAndTypePair *, ASTColumnDeclaration *> > defaulted_columns;
    std::shared_ptr<IAST> default_expr_list = std::make_shared<ASTExpressionList>();
    for (auto & ast : column_list_ast.children)
    {
        ASTColumnDeclaration & col_decl = typeid_cast<ASTColumnDeclaration &>(*ast);
        if (col_decl.type)
        {
            columns.emplace_back(col_decl.name, DataTypeFactory::instance().get(col_decl.type));
        }
        else
        {
            columns.emplace_back(col_decl.name, std::make_shared<DataTypeUInt8>());
        }

        if (col_decl.default_expression)
        {
            defaulted_columns.emplace_back(&columns.back(), &col_decl);
        }
    }
    return {columns,defaults};
}


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
    bool need_write_metadata = !create.attach;
    if(need_write_metadata) {
        create.attach = true;
        std::string statement = "create database " + database_name;
        IO::WriteBufferFromFile write_buf(metadata_file_tmp_path,statement.size(),O_WRONLY | O_CREAT | O_EXCL);
        IO::writeString(statement,write_buf);
        write_buf.next();
        write_buf.sync();
        write_buf.close();
    }

    try {
        context.addDatabase(database_name,database);
        if(need_write_metadata) {
            Poco::File(metadata_file_tmp_path).renameTo(metadata_file_path);
        }
        database->loadTables(context);
    } catch(...) {
        if(need_write_metadata) {
            Poco::File(metadata_file_tmp_path).remove();
        }
        throw;
    }
    return {};
}

IO::BlockIO InterpreterCreateQuery::createTable(ASTCreateQuery& create)
{
    if(!create.storage) {
        throw Poco::Exception("engine required",ErrorCodes::ENGINE_REQUIRED);
    }
    std::string path = context.getPath();
    std::string current_database = context.getCurrentDatabase();
    //如果未指定库名,那么就使用当前上下文的库
    std::string database_name = create.database.empty() ? current_database : create.database;
    std::string  table_name = create.table;
    std::string data_path = path + "data/" + database_name + "/";

    ColumnsInfo columns = setColumns(create);

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
        ASTFunction& engine = typeid_cast<ASTFunction&>( *create.storage);
        storage = StorageFactory::getStorage(engine.name,path,table_name,database_name,context,columns.columns);
        database->createTable(table_name,storage,query_ptr);
    }
    storage->startup();
    return {};
}

InterpreterCreateQuery::ColumnsInfo InterpreterCreateQuery::setColumns(ASTCreateQuery& create) const
{
    ColumnsInfo res;
    if(create.columns) {
        res = getColumnsInfo(create.columns,context);
    } else {
        throw Poco::Exception("Incorrect CREATE query: required list of column descriptions",ErrorCodes::INCORRECT_QUERY);
    }
    //检查列重复项
    std::set<std::string> all_columns;
    auto check_column_already_exists = [&all_columns](const NameAndTypePair& column_name_and_type) {
        if (!all_columns.emplace(column_name_and_type.name).second)
        {
            throw Poco::Exception("Column " + column_name_and_type.name + " already exists", ErrorCodes::DUPLICATE_COLUMN);
        }
    };
    for(const auto& it : *res.columns) {
        check_column_already_exists(it);
    }
    return res;
}


InterpreterCreateQuery::ColumnsInfo InterpreterCreateQuery::getColumnsInfo(const std::shared_ptr< IAST >& columns, const Context& context)
{
    ColumnsInfo res;
    auto && columns_and_defaults = parseColumns(columns, context);
    res.columns = std::make_shared<NamesAndTypesList>(std::move(columns_and_defaults.first));
    if (res.columns->size()  == 0)
    {
        throw Poco::Exception {"Cannot CREATE table without physical columns", ErrorCodes::EMPTY_LIST_OF_COLUMNS_PASSED};
    }
    return res;
}

}






