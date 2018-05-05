#include<Server/RootRequestHandler.h>
#include<Server/IServer.h>
#include<Poco/Net/HTTPServerRequest.h>
#include<Poco/Net/HTTPServerResponse.h>
#include<CommonUtil/HttpUtil.h>
#include<CommonUtil/LoggerUtil.h>

void DataBase::RootRequestHandler::handleRequest(Poco::Net::HTTPServerRequest & request,Poco::Net::HTTPServerResponse & response)
{
    try
    {
        HttpUtil::setResponseDefaultHeaders(response);
        response.setContentType("text/html; charset=UTF-8");
        const std::string data = server.config().getString("http_server_default_response", "Ok.\n");
        response.sendBuffer(data.data(), data.size());
    }
    catch (...)
    {
        LOG_ERROR(&server.logger(),"RootRequestHandler ERROR");
    }
}
