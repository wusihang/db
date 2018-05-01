#pragma once

#include<string>
#include<IO/WriteBuffer.h>

namespace IO {
class WriteBufferFromString: public WriteBuffer {

private:
    std::string & s;
    void nextImpl() override;
protected:
    void finish();
public:
    WriteBufferFromString(std::string & s_);

    ~WriteBufferFromString() override;
};

namespace detail {
/// For correct order of initialization.
class StringHolder {
protected:
    std::string value;
};
}

class WriteBufferFromOwnString: public detail::StringHolder, public WriteBufferFromString {
public:
    WriteBufferFromOwnString();
    std::string & str();
};

}








