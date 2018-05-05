#include<Server/NotFoundHandler.h>
#include<Poco/Net/HTTPServerRequest.h>
#include<Poco/Net/HTTPServerResponse.h>

void DataBase::NotFoundHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
    try
    {
        response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
        response.send() << "There is no handle " << request.getURI() << "\n"
                        << "Use / or /ping for health checks.\n"
                        << "Send queries from your program with POST method or GET /?query=...\n";
    }
    catch (...)
    {
    }
}


