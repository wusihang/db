#include<Server/HttpHandler.h>
#include<Server/HTMLForm.h>
#include<Server/IServer.h>
#include<Poco/Logger.h>
#include<CommonUtil/LoggerUtil.h>
#include<Interpreter/Context.h>
#include<Interpreter/ExecuteQuery.h>
#include<Poco/Net/HTTPServerRequest.h>
#include<Poco/Net/HTTPServerResponse.h>
#include<istream>

DataBase::HttpHandler::HttpHandler(IServer& server_)
    : server(server_), log(&Poco::Logger::get("HTTPHandler")) {

}

void DataBase::HttpHandler::handleRequest(Poco::Net::HTTPServerRequest & request, Poco::Net::HTTPServerResponse & response) {
    try {
        response.setContentType("text/plain; charset=UTF-8");
        /// keep-alive
        if (request.getVersion() == Poco::Net::HTTPServerRequest::HTTP_1_1) {
            response.setChunkedTransferEncoding(true);
        }

        DataBase::HTMLForm params(request);
        processQuery(request,params,response);

        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);
        response.send();
        LOG_INFO(log, "Done processing query");
    } catch (...) {
        /** If exception is received from remote server, then stack trace is embedded in message.
         * If exception is thrown on local server, then stack trace is in separate field.
         */
    }
}


void DataBase::HttpHandler::processQuery(Poco::Net::HTTPServerRequest & request, DataBase::HTMLForm & params, Poco::Net::HTTPServerResponse & response) {
    std::istream & istr = request.stream();
    std::string query_param = params.get("query", "");
    if (!query_param.empty()) {
        query_param += '\n';
    }
    
}

