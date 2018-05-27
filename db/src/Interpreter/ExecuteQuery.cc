#include<Interpreter/ExecuteQuery.h>
#include<Parser/ParseQuery.h>
#include<Parser/ParseUtil.h>
#include<CommonUtil/LoggerUtil.h>
#include<Interpreter/InterpreterFactory.h>
#include<Interpreter/Context.h>
#include<Interpreter/Context.h>
#include<string>
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

    //暂时是: 请求参数query后的值是什么就返回什么
    // 	wbuf.write(begin, (end - begin));
    //...数据库处理逻辑待续...
    DataBase::ParseQuery parser;
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
//     std::string query(begin,(ast->range.second - begin));
    auto interpreter  = DataBase::InterpreterFactory::get(ast,context);
    interpreter -> execute();
}



