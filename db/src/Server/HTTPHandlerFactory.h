#pragma once

#include<Poco/Net/HTTPRequestHandlerFactory.h>
#include<Poco/Net/HTTPServerRequest.h>
#include<Poco/Net/HTTPRequestHandler.h>
#include<Server/HttpHandler.h>
#include<Server/RootRequestHandler.h>
#include<Server/PingRequestHandler.h>
#include<Server/NotFoundHandler.h>
#include<Server/IServer.h>
#include<CommonUtil/LoggerUtil.h>
#include<Poco/Logger.h>
#include<string>
namespace DataBase {

template<typename HandlerType>
class HTTPRequestHandlerFactory: public Poco::Net::HTTPRequestHandlerFactory {

private:
    IServer & server;
    Poco::Logger* log;
    std::string name;
public:
    HTTPRequestHandlerFactory(IServer & server_, const std::string & name_)
        : server(server_), log(&Poco::Logger::get(name_)), name(name_) {
    }

    //构造工厂，通过请求的不同类型来返回不同的处理对象指针
    Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request) override {
        LOG_TRACE(log,
                  "HTTP Request for " << name << ". " << "Method: " << request.getMethod() << ", Address: " << request.clientAddress().toString() << ", User-Agent: " << (
                      request.has("User-Agent") ? request.get("User-Agent") : "none"));

        const auto & uri = request.getURI();

        if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET || request.getMethod() == Poco::Net::HTTPRequest::HTTP_HEAD) {
            if (uri == "/") {
                return new RootRequestHandler(server);
            }
            if (uri == "/ping")
            {
                return new PingRequestHandler;
            }
        }

        if (uri.find('?') != std::string::npos || request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST) {
            return new HandlerType(server);
        }

        if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET || request.getMethod() == Poco::Net::HTTPRequest::HTTP_HEAD || request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST) {
            return new NotFoundHandler;
        }
        return nullptr;
    }

};

using HTTPHandlerFactory = HTTPRequestHandlerFactory<HttpHandler>;

}
