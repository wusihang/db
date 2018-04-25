#include <Daemon/BaseDaemon.h>

BaseDaemon::BaseDaemon() = default;

BaseDaemon::~BaseDaemon() = default;

void BaseDaemon::initialize(Poco::Util::Application& app) {
    Poco::Util::ServerApplication::initialize(app);
}

void BaseDaemon::defineOptions(Poco::Util::OptionSet& _options) {

}
