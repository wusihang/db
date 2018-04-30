#pragma once
#include <Poco/Util/Application.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/OptionSet.h>

class BaseDaemon:public Poco::Util::ServerApplication {
public:
    BaseDaemon();
    ~BaseDaemon();
	//自定义初始化应用
    void initialize(Poco::Util::Application& app) override;
	//自定义程序启动选项
    void defineOptions(Poco::Util::OptionSet& _options) override;
};





