#include<Server/Server.h>
#include<Common/ServerApplicationExt.h>

int DataBase::Server::main(const std::vector< std::string >& args)
{
    return  Poco::Util::Application::EXIT_OK;
}

//这里注册主函数入口，该宏定义在Common/ServerApplicationExt.h
APP_SERVER_MAIN_FUNC(DataBase::Server,mainEntryDBServer) ;


