#include<Interpreter/InterpreterFactory.h>
#include<Interpreter/InterpreterUseQuery.h>
#include<Parser/ASTUseQuery.h>
#include<Ext/typeid_cast.h>
#include<Ext/std_ext.h>
#include<Poco/Exception.h>

namespace ErrorCodes {
extern const int UNKNOWN_TYPE_OF_QUERY;
}

std::unique_ptr< DataBase::IInterpreter > DataBase::InterpreterFactory::get(std::shared_ptr< DataBase::IAST >& query, DataBase::Context& context)
{
	//根据传入的语法树类型查找对应的语法解释器, 比如当前传入的是USE查询语法树,那么就选用USE查询解释器来执行
    if(typeid_cast<ASTUseQuery*>(query.get())) {
        return std_ext::make_unique<InterpreterUseQuery>(query, context);
    }
    throw Poco::Exception("Unknown type of query: " + query->getId(), ErrorCodes::UNKNOWN_TYPE_OF_QUERY);
}
