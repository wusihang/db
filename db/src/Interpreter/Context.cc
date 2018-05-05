#include<Interpreter/Context.h>


DataBase::Context::Context() = default;

DataBase::Context::~Context() = default;

DataBase::Context DataBase::Context::createGlobal()
{
	DataBase::Context context;
	return context;
}
