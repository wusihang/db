#pragma once
#include<string>
namespace PidUtil {
class  Pid {
private:
    std::string file;
public:
    Pid() {
    }
    Pid(const std::string & file_) {
        seed(file_);
    }
    void seed(const std::string & file_);
    ~Pid();
};

}
