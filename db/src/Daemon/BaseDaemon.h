#pragma once
#include <Poco/Util/ServerApplication.h>
#include <CommonUtil/PidUtil.h>
#include <Poco/AutoPtr.h>
namespace Poco {
class FileChannel;
class SyslogChannel;
namespace Util {
class AbstractConfiguration;
class Application;
class OptionSet;
}
}
class BaseDaemon:public Poco::Util::ServerApplication {
public:
    BaseDaemon();
    ~BaseDaemon();
    //自定义初始化应用
    void initialize(Poco::Util::Application& app) override;
    //自定义程序启动选项
    void defineOptions(Poco::Util::OptionSet& _options) override;
    //重新从配置文件中加载配置
    void reloadConfiguration();
    //加载日志
    void buildLoggers();

    bool isCancelled() const {
        return is_cancelled;
    }

protected:
    virtual std::string getDefaultCorePath() const;
    virtual void logVersion() const;

    PidUtil::Pid pid;
	bool is_cancelled = false;
	
private:
    void resetCoreDumpSize() const;
    void resetTimeZone() const;
    void resetDefaultFileMask() const;
    void daemonIORedirectAndPidFile(const std::string& path );
    void generateCorePath(const std::string log_path) const;

    std::string config_path;
    bool log_to_console = false;
    bool is_daemon = false;
    Poco::Util::AbstractConfiguration* last_configuration = nullptr;
    Poco::AutoPtr<Poco::Util::AbstractConfiguration> loaded_config = nullptr;
    Poco::AutoPtr<Poco::FileChannel> log_file;
    Poco::AutoPtr<Poco::FileChannel> error_log_file;
    Poco::AutoPtr<Poco::SyslogChannel> syslog_channel;
};





