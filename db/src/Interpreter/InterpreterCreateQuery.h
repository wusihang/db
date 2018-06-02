#pragma once
#include<Interpreter/IInterpreter.h>
#include<memory>

namespace DataBase {

class Context;
class IAST;
class ASTCreateQuery;

class InterpreterCreateQuery : public IInterpreter {

public:
    InterpreterCreateQuery(const std::shared_ptr<IAST> query_ptr_, Context& context_)
        :query_ptr(query_ptr_),context(context_) {
    }
    IO::BlockIO execute() override;

private:
    IO::BlockIO createDatabase(ASTCreateQuery & create);
    IO::BlockIO createTable(ASTCreateQuery & create);

    std::shared_ptr<IAST> query_ptr;
    Context & context;
};

}
