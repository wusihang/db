#pragma once
#include<Server/IServer.h>
#include<Poco/Net/HTTPRequestHandler.h>

namespace Poco {
class Logger;
namespace Net {
class HTTPServerRequest;
class HTTPServerResponse;
}
}

namespace DataBase {
class IServer;
class HTMLForm;	

//HTTP请求处理
class HttpHandler: public Poco::Net::HTTPRequestHandler {

private:
    IServer& server;
    Poco::Logger * log;

public:
    explicit HttpHandler(IServer & server_);

    //自定义请求处理逻辑只要重新实现该方法即可
    void handleRequest(Poco::Net::HTTPServerRequest & request, Poco::Net::HTTPServerResponse & response) override;

private:
    void processQuery(Poco::Net::HTTPServerRequest & request,DataBase::HTMLForm & params,Poco::Net::HTTPServerResponse & response);

};

}
