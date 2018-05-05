#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
namespace Poco {
namespace Net {
class HTTPServerRequest;
class HTTPServerResponse;
}
}

namespace DataBase
{
/// 返回404
class NotFoundHandler : public Poco::Net::HTTPRequestHandler
{
public:
    void handleRequest( Poco::Net::HTTPServerRequest & request,Poco::Net::HTTPServerResponse & response) override;
};

}
