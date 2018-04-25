#pragma once
#include <Poco/Util/Application.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/OptionSet.h>

class BaseDaemon:public Poco::Util::ServerApplication {
public:
    BaseDaemon();
    ~BaseDaemon();
    void initialize(Poco::Util::Application& app) override;
    void defineOptions(Poco::Util::OptionSet& _options) override;
};





