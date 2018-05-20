#include<CommonUtil/LoggerUtil.h>
#include<Poco/Exception.h>
#include<Poco/ErrorHandler.h>

void currentExceptionLog()
{
    try {
        throw;
    } catch(const Poco::Exception& e) {
        Poco::ErrorHandler::handle(e);
    } catch(const std::exception & e1) {
        Poco::ErrorHandler::handle(e1);
    }
    catch(...) {
        Poco::ErrorHandler::handle();
    }
}
