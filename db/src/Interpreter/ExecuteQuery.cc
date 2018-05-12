#include<Interpreter/ExecuteQuery.h>

void DataBase::executeQuery(IO::ReadBuffer& ibuf, IO::WriteBuffer& wbuf)
{
	const char* begin;
	const char* end;
	//如果buffer的工作区,无内容,那么执行next,从输入源读取内容到buffer
	if(ibuf.buffer().size()==0){
		ibuf.next();
	}
	begin = ibuf.position();
	end=ibuf.buffer().end();
	ibuf.position() += (end - begin);
	
	//暂时是: 请求参数query后的值是什么就返回什么
	wbuf.write(begin, (end - begin));
	//...数据库处理逻辑待续...
}
