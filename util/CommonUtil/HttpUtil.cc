#include<CommonUtil/HttpUtil.h>
#include<Poco/Net/HTTPServerResponse.h>
#include<Poco/Util/Application.h>
#include<string>


void HttpUtil::setResponseDefaultHeaders(Poco::Net::HTTPServerResponse& response)
{
    if (response.getKeepAlive()) {
        Poco::Timespan keep_alive_timeout(Poco::Util::Application::instance().config().getInt("keep_alive_timeout", 10), 0);
        if (keep_alive_timeout.totalSeconds())
        {
            response.set("Keep-Alive", "timeout=" + std::to_string(keep_alive_timeout.totalSeconds()));
        }
    }

}

