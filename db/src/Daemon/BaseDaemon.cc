#include <Daemon/BaseDaemon.h>
#include<Common/ConfigProcessor.h>
#include<CommonUtil/FileUtil.h>
#include<Daemon/OwnPatternFormatter.h>
#include<Daemon/LevelFilterChannel.h>
#include<Common/DBVersion.h>

#include<Poco/Util/Option.h>
#include<Poco/Util/Application.h>
#include<Poco/Util/OptionSet.h>
#include<Poco/Exception.h>
#include<Poco/File.h>
#include<Poco/SplitterChannel.h>
#include<Poco/FileChannel.h>
#include<Poco/SyslogChannel.h>
#include<Poco/Message.h>
#include<Poco/FormattingChannel.h>
#include<Poco/ConsoleChannel.h>

#include<ctime>
#include <sstream>
#include<iostream>
#include<cstdio>

#include<unistd.h>
#include<sys/resource.h>
#include<sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>

BaseDaemon::BaseDaemon() = default;

BaseDaemon::~BaseDaemon() = default;

void BaseDaemon::defineOptions(Poco::Util::OptionSet& _options) {
    //调用默认选项定义
    Poco::Util::ServerApplication::defineOptions(_options);
    //定义配置文件选项
    _options.addOption(
        Poco::Util::Option("config-file", "C", "load configuration from a given file").required(false).repeatable(
            false).argument("<file>").binding("config-file"));
    //定义日志文件选项
    _options.addOption(
        Poco::Util::Option("log-file", "L", "use given log file").required(false).repeatable(false).argument(
            "<file>").binding("logger.log"));
    //错误日志
    _options.addOption(
        Poco::Util::Option("errorlog-file", "E", "use given log file for errors only").required(false).repeatable(
            false).argument("<file>").binding("logger.errorlog"));
    //pid文件
    _options.addOption(
        Poco::Util::Option("pid-file", "P", "use given pidfile").required(false).repeatable(false).argument(
            "<file>").binding("pid"));
}

void BaseDaemon::initialize(Poco::Util::Application& app) {
    Poco::Util::ServerApplication::initialize(app);
    //是否是后台启动
    bool is_daemon = config().getBool("application.runAsDaemon", false);
    if (is_daemon) {
        std::string path = Poco::Path(config().getString("application.path")).setFileName("").toString();
        //改变当前工作目录
        if (chdir(path.c_str())) {
            //如果失败就直接抛出异常
            throw Poco::Exception("Cannot change directory to " + path);
        }
    }
    //重新加载配置（从配置文件config.xml中读取）
    reloadConfiguration();
    //重新设置core dump文件大小
    resetCoreDumpSize();
    //设置时区
    resetTimeZone();
    //重新设置文件默认umask
    resetDefaultFileMask();

    std::string log_path = config().getString("logger.log", "");
    if (!log_path.empty()) {
        log_path = Poco::Path(log_path).setFileName("").toString();
    }
    if(is_daemon) {
        //IO重定向，Pid文件
        daemonIORedirectAndPidFile(log_path);
    }

    //构建日志基本框架
    buildLoggers();

    if (is_daemon) {
        generateCorePath(log_path);
    }

    logVersion();
}


void BaseDaemon::reloadConfiguration()
{
    //从原有配置选项中获取config.xml的路径
    config_path = config().getString("config-file", "config.xml");
    //加载该配置文件的内容
    loaded_config = DataBase::ConfigProcessor().loadConfig(config_path);
    //如果上次的配置存在，那么从配置中移除上次配置
    if (last_configuration != nullptr) {
        config().removeConfiguration(last_configuration);
    }
    //将当前配置作为上一次配置
    last_configuration = loaded_config.duplicate();
    config().add(last_configuration, PRIO_DEFAULT, false);
    std::string log_command_line_option = config().getString("logger.log", "");
    // 如果不是以后台运行，并且logger.log未指定日志文件，那么就打印到控制台
    log_to_console = !(config().getBool("application.runAsDaemon", false)) && log_command_line_option.empty();
}

void BaseDaemon::resetCoreDumpSize() const
{
    struct rlimit rlim;  // 包括资源的软限制和硬限制
    // 获取CORE DUMP的限制
    if (getrlimit(RLIMIT_CORE, &rlim)) {
        throw Poco::Exception("Cannot getrlimit");
    }

    /// 默认设置coredump大小为1GB
    rlim.rlim_cur = config().getUInt64("core_dump.size_limit", 1024 * 1024 * 1024);

    //设置
    if (setrlimit(RLIMIT_CORE, &rlim)) {
        std::string message = "Cannot set max size of core file to " + std::to_string(rlim.rlim_cur);
#if !defined(ADDRESS_SANITIZER) && !defined(THREAD_SANITIZER)
        throw Poco::Exception(message);
#else
        /// address/thread sanitizer. http://lists.llvm.org/pipermail/llvm-bugs/2013-April/027880.html
        std::cerr << message << std::endl;
#endif
    }
}

void BaseDaemon::resetTimeZone() const
{
    if (config().has("timezone")) {
        // 例子 ===>  setenv("TZ","GMT-8",1) 代表中国东八区
        if (0 != setenv("TZ", config().getString("timezone").data(), 1)) {
            throw Poco::Exception("Cannot setenv TZ variable");
        }
        //set unix timezone
        tzset();
    }
}

void BaseDaemon::resetDefaultFileMask() const
{
    if (config().has("umask")) {
        // 例如 022
        std::string umask_str = config().getString("umask");
        mode_t umask_num = 0;
        std::stringstream stream;
        stream << umask_str;
        // 假设 umask_str = "022" 那么 umask_num=18
        stream >> std::oct >> umask_num;
        umask(umask_num);
    }
}

void BaseDaemon::daemonIORedirectAndPidFile(const std::string& log_path)
{
    if (!log_path.empty()) {
        std::string stdout_path = log_path + "/stdout";
        // redirect standard ouput stream to ${path}/stdout by append mode
        if (!freopen(stdout_path.c_str(), "a+", stdout)) {
            throw Poco::OpenFileException("Cannot attach stdout to " + stdout_path);
        }

        std::string stderr_path = log_path + "/stderr";
        // redirect standard error stream to ${path}/stderr by append mode
        if (!freopen(stderr_path.c_str(), "a+", stderr)) {
            throw Poco::OpenFileException("Cannot attach stderr to " + stderr_path);
        }
    }
    // 如果配置项有pid，那么就创建一个pid文件
    if (config().has("pid")) {
        std::string  pidFilePath = config().getString("pid");
        pid.seed(pidFilePath);
    }
}

void BaseDaemon::buildLoggers()
{
    //如果包含logger.log的文件配置，并且不是打印到console的，那么就创建对应的日志文件
    if (config().hasProperty("logger.log") && !log_to_console) {
        //创建logger日志文件
        std::string path = FileUtil::createDirectoryRecusively(config().getString("logger.log"));
        // 如果是以后台模式启动，那么尝试进入刚创建的目录
        if (config().getBool("application.runAsDaemon", false) && chdir(path.c_str()) != 0) {
            throw Poco::Exception("Cannot change directory to " + path);
        }
        std::string loggerPath = config().getString("logger.log");
        std::cerr << "Should logs to " << loggerPath << std::endl;
        //SplitterChannel可以将日志输出到不同的目标
        Poco::AutoPtr<Poco::SplitterChannel> split = new Poco::SplitterChannel;
        Poco::AutoPtr<DataBase::OwnPatternFormatter> pf = new DataBase::OwnPatternFormatter(this);
        pf.get()->setProperty("times","local");
        Poco::AutoPtr<Poco::FormattingChannel> log = new Poco::FormattingChannel();
        log_file  = new Poco::FileChannel;
        log_file->setProperty("path", Poco::Path(loggerPath).absolute().toString());
        log_file->setProperty("rotation", config().getRawString("logger.size", "100M"));
        log_file->setProperty("archive", "number");
        log_file->setProperty("compress", config().getRawString("logger.compress", "true"));
        log_file->setProperty("purgeCount", config().getRawString("logger.count", "1"));
        log->setChannel(log_file);
        split->addChannel(log);
        log_file->open();
        if (config().hasProperty("logger.errorlog")) {
            //创建error日志文件
            FileUtil::createDirectoryRecusively(config().getString("logger.errorlog"));
            std::cerr << "Should error logs to " << config().getString("logger.errorlog") << std::endl;
            Poco::AutoPtr<DataBase::LevelFilterChannel> level = new DataBase::LevelFilterChannel;
            level->setLevel(Poco::Message::PRIO_NOTICE);
            Poco::AutoPtr<DataBase::OwnPatternFormatter> pf = new DataBase::OwnPatternFormatter(this);
            pf->setProperty("times", "local");
            Poco::AutoPtr<Poco::FormattingChannel> errorlog = new Poco::FormattingChannel(pf);
            error_log_file = new Poco::FileChannel;
            error_log_file->setProperty("path",
                                        Poco::Path(config().getString("logger.errorlog")).absolute().toString());
            error_log_file->setProperty("rotation", config().getRawString("logger.size", "100M"));
            error_log_file->setProperty("archive", "number");
            error_log_file->setProperty("compress", config().getRawString("logger.compress", "true"));
            error_log_file->setProperty("purgeCount", config().getRawString("logger.count", "1"));
            errorlog->setChannel(error_log_file);
            level->setChannel(errorlog);
            split->addChannel(level);
            errorlog->open();
        }

        if (config().getBool("logger.use_syslog", false) || config().getBool("dynamic_layer_selection", false)) {
            Poco::AutoPtr<DataBase::OwnPatternFormatter> pf = new DataBase::OwnPatternFormatter(this,
                    DataBase::OwnPatternFormatter::ADD_LAYER_TAG);
            pf->setProperty("times", "local");
            Poco::AutoPtr<Poco::FormattingChannel> log = new Poco::FormattingChannel(pf);
            syslog_channel = new Poco::SyslogChannel(commandName(),
                    Poco::SyslogChannel::SYSLOG_CONS | Poco::SyslogChannel::SYSLOG_PID,
                    Poco::SyslogChannel::SYSLOG_DAEMON);
            log->setChannel(syslog_channel);
            split->addChannel(log);
            syslog_channel->open();
        }
        split->open();
        logger().close();
        logger().setChannel(split);
    }
    else {
        if (config().getBool("application.runAsDaemon", false) && chdir("/tmp") != 0) {
            throw Poco::Exception("Cannot change directory to /tmp");
        }
        Poco::AutoPtr<Poco::ConsoleChannel> file = new Poco::ConsoleChannel;
        Poco::AutoPtr<DataBase::OwnPatternFormatter> pf = new DataBase::OwnPatternFormatter(this);
        pf->setProperty("times", "local");
        Poco::AutoPtr<Poco::FormattingChannel> log = new Poco::FormattingChannel(pf);
        log->setChannel(file);
        logger().close();
        logger().setChannel(log);
        logger().warning("Logging to console");
    }

    logger().setLevel(config().getString("logger.level", "trace"));
    Poco::Logger::root().setLevel(logger().getLevel());
    Poco::Logger::root().setChannel(logger().getChannel());
    //例如: logger.levels.error  logger.levels.info
    Poco::Util::AbstractConfiguration::Keys levels;
    config().keys("logger.levels", levels);
    if (!levels.empty()) {
        for (Poco::Util::AbstractConfiguration::Keys::iterator it = levels.begin(); it != levels.end(); ++it) {
            Poco::Logger::get(*it).setLevel(config().getString("logger.levels." + *it, "trace"));
        }
    }

}

void BaseDaemon::generateCorePath(const std::string log_path) const
{
    std::string core_path = config().getString("core_path", "");
    if (core_path.empty()) {
        core_path = getDefaultCorePath();
    }
    FileUtil::tryCreateDirectories(&logger(), core_path);
    Poco::File cores = core_path;
    if (!(cores.exists() && cores.isDirectory())) {
        core_path = !log_path.empty() ? log_path : "/opt/";
        FileUtil::tryCreateDirectories(&logger(), core_path);
    }
    if (0 != chdir(core_path.c_str())) {
        throw Poco::Exception("Cannot change directory to " + core_path);
    }
}


std::string BaseDaemon::getDefaultCorePath() const
{
    return "/opt/cores";
}

void BaseDaemon::logVersion() const {
    Poco::Logger::root().information("Starting daemon with revision " + DataBase::get_version());
}

