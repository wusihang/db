#include<Server/HttpHandler.h>
#include<Server/HTMLForm.h>
#include<Server/IServer.h>
#include<Poco/Logger.h>
#include<CommonUtil/LoggerUtil.h>
#include<Interpreter/Context.h>
#include<Interpreter/ExecuteQuery.h>
#include<Poco/Net/HTTPServerRequest.h>
#include<Poco/Net/HTTPServerResponse.h>
#include<IO/ReadBuffer.h>
#include <IO/ReadBufferFromString.h>
#include <IO/WriteBufferFromHTTPServerResponse.h>
#include<istream>
#include<Ext/std_ext.h>

DataBase::HttpHandler::HttpHandler(IServer& server_)
    : server(server_), log(&Poco::Logger::get("HTTPHandler")) {

}

void DataBase::HttpHandler::handleRequest(Poco::Net::HTTPServerRequest & request, Poco::Net::HTTPServerResponse & response) {
    Output output;
    try {
        response.setContentType("text/plain; charset=UTF-8");
        /// keep-alive
        if (request.getVersion() == Poco::Net::HTTPServerRequest::HTTP_1_1) {
            response.setChunkedTransferEncoding(true);
        }

        DataBase::HTMLForm params(request);
        processQuery(request,params,response,output);
        LOG_INFO(log, "Done processing query");
    } catch (...) {
        trySendExceptionToClient("cannot executeQuery",500,request,response,output);
    }
}


void DataBase::HttpHandler::processQuery(Poco::Net::HTTPServerRequest & request, DataBase::HTMLForm & params, Poco::Net::HTTPServerResponse & response,Output& output) {
    std::istream & istr = request.stream();
    std::string query_param = params.get("query", "");
    if (!query_param.empty()) {
        query_param += '\n';
    }
    std::unique_ptr<IO::ReadBuffer> in_param = std_ext::make_unique<IO::ReadBufferFromString>(query_param);
    output.out
        = std::make_shared<IO::WriteBufferFromHTTPServerResponse>(response);
    DataBase::executeQuery(*in_param,*output.out);
    output.out->finalize();
}


void DataBase::HttpHandler::trySendExceptionToClient(const std::string& s, int exception_code, Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response,Output& output)
{
    try {
        //如果是POST请求,KEEP-ALIVE开启,那么我们需要读取整个请求体
        // 以防止下个请求读取到当前请求的部分内容
        if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST&& response.getKeepAlive()&& !request.stream().eof())
        {
            request.stream().ignore(std::numeric_limits<std::streamsize>::max());
        }
        response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        if (!response.sent())
        {
            ///如果未发送任何数据
            response.send() << s << std::endl;
        } else {
            bool dataSent = output.out->count() != output.out->offset();
            if(!dataSent) {
                output.out->position() = output.out->buffer().begin();
            }
            output.out->next();
            output.out->finalize();
        }
    } catch(...) {
        currentExceptionLog();
    }
}

