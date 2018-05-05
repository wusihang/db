#pragma once

namespace Poco {
namespace Net {
class HTTPServerResponse;
}
}

namespace HttpUtil {

void setResponseDefaultHeaders(Poco::Net::HTTPServerResponse & response);

}
