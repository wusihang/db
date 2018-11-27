#include<Exception/ExceptionHelper.h>
#include<Poco/Exception.h>
#include<Poco/ErrorHandler.h>
#include<IO/WriteBufferHelper.h>
namespace DataBase {

std::string currentExceptionLog()
{
    try {
        throw;
    } catch(const Poco::Exception& e) {
        Poco::ErrorHandler::handle(e);
        return "errorCode:" + IO::toString<int>(e.code()) + ",  " + e.displayText();
    } catch(const std::exception & e1) {
        Poco::ErrorHandler::handle(e1);
        return std::string(e1.what());
    }
    catch(...) {
        Poco::ErrorHandler::handle();
		 return "unknown exception";
    }
}

}
