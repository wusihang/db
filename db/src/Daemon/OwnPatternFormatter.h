#pragma once
#include <Poco/PatternFormatter.h>

class BaseDaemon;

namespace DataBase {

//自定义日志格式，要自定义格式，只要实现PatternFormatter的format虚函数即可
class OwnPatternFormatter: public Poco::PatternFormatter {
public:
    enum Options {
        ADD_NOTHING = 0, ADD_LAYER_TAG = 1 << 0
    };

    OwnPatternFormatter(const BaseDaemon * daemon_, Options options_ = ADD_NOTHING)
        : Poco::PatternFormatter(""), daemon(daemon_), options(options_) {
    }

    void format(const Poco::Message & msg, std::string & text) override;

    ~OwnPatternFormatter() = default;

private:
    const BaseDaemon * daemon;
    Options options; 
};

}
