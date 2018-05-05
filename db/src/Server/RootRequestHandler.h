#pragma once
#include<Poco/Net/HTTPRequestHandler.h>

namespace Poco{
	namespace Net{
		class HTTPServerRequest;
		class HTTPServerResponse;
	}
}

namespace DataBase {

class IServer;
	
class RootRequestHandler: public Poco::Net::HTTPRequestHandler {

private:
    IServer & server;

public:
    RootRequestHandler(IServer & server_) : server(server_)
    {
    }

    void handleRequest( Poco::Net::HTTPServerRequest & request, Poco::Net::HTTPServerResponse & response) override;

};

}
