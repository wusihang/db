#include<DataBases/DataBaseCommon.h>
#include<Interpreter/Context.h>
#include<Parser/ParseCreateQuery.h>
#include<Parser/ParseUtil.h>
#include<Parser/ASTCreateQuery.h>
#include<Ext/typeid_cast.h>
#include<Interpreter/InterpreterCreateQuery.h>
#include<Parser/ASTFunction.h>
#include<Storages/StorageFactory.h>
namespace DataBase {
std::pair< std::string,std::shared_ptr<Storage::IStorage> > createTableFromDefinition(const std::string& definition, const std::string& database_name, const std::string& database_data_path, Context& context, const std::string& description_for_error_message)
{
    ParserCreateQuery parser;
    std::shared_ptr<DataBase::IAST> ast = ParseUtil::parseQuery(parser, definition.data(), definition.data() + definition.size(), description_for_error_message);

    ASTCreateQuery & ast_create_query = typeid_cast<ASTCreateQuery &>(*ast);
    ast_create_query.database = database_name;
	ast_create_query.attach = true;
    InterpreterCreateQuery::ColumnsInfo columns_info = InterpreterCreateQuery::getColumnsInfo(ast_create_query.columns, context);
    std::string storage_name =  typeid_cast<ASTFunction &>(*ast_create_query.storage).name;
    return
    {
        ast_create_query.table,
        StorageFactory::getStorage(
            storage_name, database_data_path, ast_create_query.table, database_name, context, columns_info.columns)
    };
}

}
