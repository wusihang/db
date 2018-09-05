#pragma once
#include<Interpreter/IInterpreter.h>
#include<Parser/IAST.h>
#include<Interpreter/Context.h>
namespace Storage{
	class IStorage;
}
namespace DataBase {
class InterpreterInsertQuery:public IInterpreter {
public:
    InterpreterInsertQuery(const  std::shared_ptr<IAST> & query_ptr_, const Context & context_);

    /** Prepare a request for execution. Return block streams
      * - the stream into which you can write data to execute the query, if INSERT;
      * - the stream from which you can read the result of the query, if SELECT and similar;
      * Or nothing if the request INSERT SELECT (self-sufficient query - does not accept the input data, does not return the result).
      */
    IO::BlockIO execute() override;

private:
    std::shared_ptr<Storage::IStorage> getTable();
    std::shared_ptr<IAST> query_ptr;
    Context context;
};

}
