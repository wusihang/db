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
//不直接将value放在WriteBufferFromOwnString中作为成员变量的原因是为了保证value比buffer优先初始化
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








