#pragma once
#include<Interpreter/IInterpreter.h>
#include<memory>

namespace DataBase {

class IAST;
class Context;

class InterpreterFactory {
public:
    static std::unique_ptr<IInterpreter> get(std::shared_ptr<IAST> & query,Context & context);
};

}
