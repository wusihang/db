#pragma once
#include<Interpreter/IInterpreter.h>
#include<memory>

namespace DataBase {

class Context;
class IAST;

class InterpreterUseQuery : public IInterpreter {

public:
    InterpreterUseQuery(const std::shared_ptr<IAST> query_ptr_, Context& context_)
        :query_ptr(query_ptr_),context(context_) {
    }
    IO::BlockIO execute() override;

private:
    std::shared_ptr<IAST> query_ptr;
    Context & context;
};

}
