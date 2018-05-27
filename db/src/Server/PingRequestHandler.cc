#include<Server/PingRequestHandler.h>
#include<Poco/Net/HTTPServerRequest.h>
#include<Poco/Net/HTTPServerResponse.h>
#include<CommonUtil/HttpUtil.h>
#include<CommonUtil/LoggerUtil.h>
#include<string>

void PingRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
    try
    {
        HttpUtil::setResponseDefaultHeaders(response);
        response.setContentType("text/html; charset=UTF-8");
        const std::string data = "Ok.\n";
        response.sendBuffer(data.data(), data.size());
    }
    catch (...)
    {
		DataBase::currentExceptionLog();
    }
}


