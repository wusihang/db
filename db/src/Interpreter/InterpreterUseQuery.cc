#include<Interpreter/InterpreterUseQuery.h>
#include<Interpreter/Context.h>
#include<Ext/typeid_cast.h>
#include<Parser/ASTUseQuery.h>

IO::BlockIO DataBase::InterpreterUseQuery::execute()
{
	//use查询就是把当前上下文的数据库切换为新库
    const std::string & new_database = typeid_cast<const ASTUseQuery &>(*query_ptr).database;
    context.setCurrentDatabase(new_database);
    return {};
}
