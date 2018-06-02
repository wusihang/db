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
#include<Ext/scope_guard.h>
#include<Poco/Util/Application.h>
#include<Server/HTMLForm.h>
#include<IO/ReadBufferFromString.h>
#include<IO/WriteBufferHelper.h>
#include<IO/ReadHelper.h>

namespace ErrorCodes
{
extern const int INVALID_SESSION_TIMEOUT;
}

static std::chrono::steady_clock::duration parseSessionTimeout(const DataBase::HTMLForm & params)
{
    const auto & config = Poco::Util::Application::instance().config();
    unsigned session_timeout = config.getInt("default_session_timeout", 60);
    if (params.has("session_timeout"))
    {
        unsigned max_session_timeout = config.getUInt("max_session_timeout", 3600);
        std::string session_timeout_str = params.get("session_timeout");

        IO::ReadBufferFromString buf(session_timeout_str);
        if (!IO::tryReadIntText<unsigned>(session_timeout, buf) || !buf.eof())
            throw Poco::Exception("Invalid session timeout: '" + session_timeout_str + "'", ErrorCodes::INVALID_SESSION_TIMEOUT);

        if (session_timeout > max_session_timeout)
            throw Poco::Exception("Session timeout '" + session_timeout_str + "' is larger than max_session_timeout: " + IO::toString(max_session_timeout)
                                  + ". Maximum session timeout could be modified in configuration file.",
                                  ErrorCodes::INVALID_SESSION_TIMEOUT);
    }

    return std::chrono::seconds(session_timeout);
}

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
        DataBase::currentExceptionLog();
        trySendExceptionToClient("cannot executeQuery",500,request,response,output);
    }
}


void DataBase::HttpHandler::processQuery(Poco::Net::HTTPServerRequest & request, DataBase::HTMLForm & params, Poco::Net::HTTPServerResponse & response,Output& output) {
//     std::istream & istr = request.stream();
    std::string query_param = params.get("query", "");
    if (!query_param.empty()) {
        query_param += '\n';
    }
    //构造查询输入buffer和响应输出buffer
    std::unique_ptr<IO::ReadBuffer> in_param = std_ext::make_unique<IO::ReadBufferFromString>(query_param);
    output.out = std::make_shared<IO::WriteBufferFromHTTPServerResponse>(response);


    //获取用户名
    std::string user = request.get("X-DB-User", params.get("user", "default"));
    //获取服务器上下文
    DataBase::Context context = server.context();
    //设置全局Context为服务器Context
    context.setGlobalContext(server.context());
    //获取客户端对象引用
    DataBase::ClientInfo & client_info = context.getClientInfo();
    //设置用户名
    client_info.current_user = user;
    std::shared_ptr<DataBase::Context> session;
    std::string session_id;
    std::chrono::steady_clock::duration session_timeout;
    bool session_is_set = params.has("session_id");
    //如果客户端配置了session_id
    if(session_is_set) {
        session_id = params.get("session_id");
        //解析超时时间
        session_timeout = parseSessionTimeout(params);
        std::string session_check = params.get("session_check", "");
        //获取一个session
        session = context.acquireSession(session_id,session_timeout,session_check == "1");
        //将上下文切换到session
        context = *session;
        context.setSessionContext(*session);
    }

    //退出当前作用域时执行,如果设置了生成了session,那么就释放掉
    SCOPE_EXIT( {
        if(session_is_set)
        {
            session->releaseSession(session_id,session_timeout);
        }
    });
    DataBase::executeQuery(*in_param,*output.out,context);
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
        bool dataSent = output.out->count() != output.out->offset();
        if(!dataSent) {
            output.out->position() = output.out->buffer().begin();
        }
        IO::writeString(s,*output.out);
        output.out->next();
        output.out->finalize();
    } catch(...) {
        DataBase::currentExceptionLog();
    }
}



