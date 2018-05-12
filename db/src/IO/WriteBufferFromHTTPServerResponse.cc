#include<IO/WriteBufferFromHTTPServerResponse.h>
#include<Poco/Net/HTTPServerResponse.h>
#include<CommonUtil/HttpUtil.h>

IO::WriteBufferFromHTTPServerResponse::WriteBufferFromHTTPServerResponse(Poco::Net::HTTPServerResponse& response_, size_t size)
    : BufferWithOwnMemory< IO::WriteBuffer >(size),response(response_) {

}


void IO::WriteBufferFromHTTPServerResponse::nextImpl()
{
    {
        //对当前作用域内加锁,离开作用域后自动释放
        std::lock_guard<std::mutex> lock(mutex);
        //设置响应头
        startSendHeaders();
        if(!out) {
            //响应体的流指针
            response_body_ostr = &(response.send());
            //构造ostream的流buffer, 流buffer的大小和当前的workinig_buffer一致,流buffer内存区域也和当前一致
            out = new WriteBufferFromOStream(*response_body_ostr,working_buffer.size(),working_buffer.begin());
        }
        finishSendHeaders();
    }
    out->position() = position();
    out->next();
}


void IO::WriteBufferFromHTTPServerResponse::startSendHeaders()
{
    if (!headers_started_sending)
    {
        headers_started_sending = true;
        if (add_cors_header)
        {
            response.set("Access-Control-Allow-Origin", "*");
        }
        HttpUtil::setResponseDefaultHeaders(response);
    }
}

void IO::WriteBufferFromHTTPServerResponse::finishSendHeaders()
{
    if (!headers_finished_sending)
    {
        headers_finished_sending = true;
        if (!response_body_ostr)
        {
            response_body_ostr = &(response.send());
        }
    }
}

void IO::WriteBufferFromHTTPServerResponse::finalize()
{
    if (offset())
    {
        next();
    }
    else
    {
        std::lock_guard<std::mutex> lock(mutex);
        startSendHeaders();
        finishSendHeaders();
    }
}

IO::WriteBufferFromHTTPServerResponse::~WriteBufferFromHTTPServerResponse()
{
    try {
        finalize();
    } catch(...) {

    }
}


