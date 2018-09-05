#include<Streams/FormatFactory.h>
#include<IO/ReadBuffer.h>
#include<IO/WriteBuffer.h>
#include<Interpreter/Context.h>
#include<Streams/ValuesRowInputStream.h>
#include<Streams/BlockInputStreamFromRowInputStream.h>
namespace ErrorCodes {
extern const int UNKNOWN_FORMAT;
}

IO::BlockInputStreamPtr IO::FormatFactory::getInput(const std::string& name, IO::ReadBuffer& buf,const Block & sample, const DataBase::Context& context) const
{
    auto wrap_row_stream = [&](auto&& row_stream) {
        return std::make_shared<IO::BlockInputStreamFromRowInputStream>(std::move(row_stream), sample,10000);
    };

    if(name == "Values") {
        return wrap_row_stream(std::make_shared<IO::ValuesRowInputStream>(buf, context,false));
    } else
    {
        throw Poco::Exception("Unknown format " + name, ErrorCodes::UNKNOWN_FORMAT);
    }
}


IO::BlockOutputStreamPtr IO::FormatFactory::getOutput(const std::string& name, IO::WriteBuffer& buf,const Block & sample, const DataBase::Context& context) const
{
    return nullptr;
}
