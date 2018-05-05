#include<Interpreter/ExecuteQuery.h>

void DataBase::executeQuery(IO::ReadBuffer& ibuf, IO::WriteBuffer& wbuf)
{
	const char* begin;
	const char* end;
	if(ibuf.buffer().size()==0){
		ibuf.next();
	}
	begin = ibuf.position();
	end=ibuf.buffer().end();
	ibuf.position() += (end - begin);
	
}
