#include<Interpreter/ExecuteQuery.h>
#include<Parser/ParseQuery.h>
#include<Parser/ParseUtil.h>
#include<CommonUtil/LoggerUtil.h>
#include<Interpreter/InterpreterFactory.h>
#include<Interpreter/Context.h>
#include<Interpreter/Context.h>
#include<string>
#include<Streams/BlockIO.h>
#include<Streams/CopyData.h>
#include<Streams/InputStreamFromASTInsertQuery.h>


static std::tuple< std::shared_ptr<DataBase::IAST>, IO::BlockIO> executeQueryImpl(const char * begin,   const char * end,   DataBase::Context & context, bool internal) {
    DataBase::ParseQuery parser(end);
    std::shared_ptr<DataBase::IAST> ast;
    try {
        //sql分析为抽象语法树
        ast =  ParseUtil::parseQuery(parser,begin,end,"");
        if(ast->range.first < begin || ast->range.second > end) {
            throw Poco::Exception("Unexpected behavior: AST chars range is not inside source range");
        }
    } catch(...) {
        DataBase::currentExceptionLog();
        throw;
    }
    auto interpreter  = DataBase::InterpreterFactory::get(ast,context);
    IO::BlockIO block =  interpreter -> execute();
    return std::make_tuple(ast,block);
}

void DataBase::executeQuery(IO::ReadBuffer& ibuf, IO::WriteBuffer& wbuf,DataBase::Context& context)
{
    const char* begin;
    const char* end;
    //如果buffer的工作区,无内容,那么执行next,从输入源读取内容到buffer
    if(ibuf.buffer().size()==0) {
        ibuf.next();
    }
    begin = ibuf.position();
    end=ibuf.buffer().end();
    ibuf.position() += (end - begin);


    std::shared_ptr<DataBase::IAST> ast;
    IO::BlockIO streams;
    std::tie(ast, streams) = executeQueryImpl(begin, end, context, false);
    try
    {
        if (streams.out)
        {
            //入库操作实际在此处发生
            IO::InputStreamFromASTInsertQuery in(ast, ibuf, streams, context);
            CopyStreamDataUtil::copyData(in, *streams.out);
        }
    } catch(...) {
        DataBase::currentExceptionLog();
        throw;
    }
}



