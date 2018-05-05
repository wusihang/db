#pragma once
#include<Poco/Net/HTTPRequestHandler.h>

namespace Poco {
namespace Net {
class HTTPServerRequest;
class HTTPServerResponse;
}
}

class PingRequestHandler : public Poco::Net::HTTPRequestHandler
{
public:
    void handleRequest(
        Poco::Net::HTTPServerRequest & request,
        Poco::Net::HTTPServerResponse & response) override;
};
