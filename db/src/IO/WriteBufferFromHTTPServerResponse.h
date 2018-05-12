#pragma once
#include<IO/BufferWithOwnMemory.h>
#include<IO/WriteBuffer.h>
#include<IO/WriteBufferFromOStream.h>
#include<mutex>

namespace Poco
{
namespace Net
{
class HTTPServerResponse;
}
}

namespace IO {

//继承自BufferWithOwnMemory ,  BufferWithOwnMemory继承自WriteBuffer
class WriteBufferFromHTTPServerResponse: public BufferWithOwnMemory<WriteBuffer> {
private:
    Poco::Net::HTTPServerResponse & response;
    std::mutex mutex;
    WriteBuffer* out = nullptr;
    std::ostream * response_body_ostr = nullptr;
    std::ostream * response_header_ostr = nullptr;

    bool add_cors_header = false;
    bool headers_started_sending = false;
    bool headers_finished_sending = false;
    void nextImpl() override;
    void startSendHeaders();
    void finishSendHeaders();
public:
    WriteBufferFromHTTPServerResponse(Poco::Net::HTTPServerResponse& response_, size_t size = DBMS_DEFAULT_BUFFER_SIZE);

    void finalize();

    ~WriteBufferFromHTTPServerResponse();

};

}
