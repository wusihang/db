#pragma once
#include<Poco/ErrorHandler.h>
#include<Poco/Logger.h>
#include<CommonUtil/LoggerUtil.h>
#include<Poco/Exception.h>
#include<IO/WriteBufferHelper.h>


class ServerErrorHandler:public Poco::ErrorHandler {
public:
    void exception(const Poco::Exception & e)     {
        logException("errorCode:" + IO::toString<int>(e.code()) + ",  " + e.displayText());
    }
    void exception(const std::exception & e)    {
        logException(std::string(e.what()));
    }
    void exception() {
        logException("unknow exception");
    }

private:
    Poco::Logger * log = &Poco::Logger::get("ServerErrorHandler");
    void logException(std::string&& msg)
    {
        LOG_ERROR(log, "some exception ocrured !   " + msg);
    }
};