#include<Interpreter/InterpreterInsertQuery.h>
#include<Interpreter/Context.h>
#include<Storages/IStorage.h>
#include<Parser/ASTInsertQuery.h>
#include<Ext/typeid_cast.h>
#include<Streams/BlockIO.h>
namespace DataBase {

std::shared_ptr< Storage::IStorage > InterpreterInsertQuery::getTable()
{
    ASTInsertQuery & query = typeid_cast<ASTInsertQuery &>(*query_ptr);
    /// In what table to write.
    return context.getTable(query.database, query.table);
}
IO::BlockIO InterpreterInsertQuery::execute()
{
//     ASTInsertQuery & query = typeid_cast<ASTInsertQuery &>(*query_ptr);
// 	query.columns
    std::shared_ptr< Storage::IStorage > table = getTable();
    auto table_lock = table->lockStructure(true);
	auto stream = table->write(query_ptr);
	 IO::BlockIO res;
	 res.out_sample = table->getSampleBlock();
	 res.out = stream;
	return res;
}


InterpreterInsertQuery::InterpreterInsertQuery(const std::shared_ptr< IAST >& query_ptr_, const Context& context_)
    : query_ptr(query_ptr_), context(context_)
{

}

}
