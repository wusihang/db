#pragma once
#include<Interpreter/IInterpreter.h>
#include<memory>
#include<Core/NamesAndTypes.h>

class ThreadPool;
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

    struct ColumnsInfo
    {
        std::shared_ptr<NamesAndTypesList> columns = std::make_shared<NamesAndTypesList>();
    };

    static ColumnsInfo getColumnsInfo(const std::shared_ptr<IAST> & columns, const Context & context) ;

    void setDatabaseLoadingThreadpool(ThreadPool & thread_pool_)
    {
        thread_pool = &thread_pool_;
    }

    void setForceRestoreData(bool has_force_restore_data_flag_)
    {
        has_force_restore_data_flag = has_force_restore_data_flag_;
    }

private:
    ColumnsInfo setColumns(ASTCreateQuery & create) const;
    IO::BlockIO createDatabase(ASTCreateQuery & create);
    IO::BlockIO createTable(ASTCreateQuery & create);

    std::shared_ptr<IAST> query_ptr;
    Context & context;

    ThreadPool * thread_pool = nullptr;
    bool has_force_restore_data_flag = false;
};

}
