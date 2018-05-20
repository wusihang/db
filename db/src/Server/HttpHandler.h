#pragma once
#include<Server/IServer.h>
#include<Poco/Net/HTTPRequestHandler.h>
#include<memory>

namespace Poco {
class Logger;
namespace Net {
class HTTPServerRequest;
class HTTPServerResponse;
}
}

namespace IO{
	class WriteBufferFromHTTPServerResponse;
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
	 struct Output
    {
        std::shared_ptr<IO::WriteBufferFromHTTPServerResponse> out;
	};
	
    explicit HttpHandler(IServer & server_);

    //自定义请求处理逻辑只要重新实现该方法即可
    void handleRequest(Poco::Net::HTTPServerRequest & request, Poco::Net::HTTPServerResponse & response) override;

private:
    void processQuery(Poco::Net::HTTPServerRequest & request,DataBase::HTMLForm & params,Poco::Net::HTTPServerResponse & response,Output& output);

	//向客户端发送异常信息
    void trySendExceptionToClient(const std::string & s, int exception_code,
                                  Poco::Net::HTTPServerRequest & request, Poco::Net::HTTPServerResponse & response,Output& output);
};

}






