#include<Daemon/LevelFilterChannel.h>


#include <Poco/LoggingRegistry.h>
#include <Poco/String.h>

namespace DataBase {

LevelFilterChannel::~LevelFilterChannel() {
    if (_channel) {
        _channel->release();
    }
}

void LevelFilterChannel::setChannel(Channel* channel) {
    if (_channel) {
        _channel->release();
    }
    _channel = channel;
    if (_channel) {
        _channel->duplicate();
    }
}

Poco::Channel* LevelFilterChannel::getChannel() const {
    return _channel;
}

void LevelFilterChannel::open() {
    if (_channel) {
        _channel->open();
    }
}

void LevelFilterChannel::close() {
    if (_channel) {
        _channel->close();
    }
}

void LevelFilterChannel::setLevel(Poco::Message::Priority priority) {
    _priority = priority;
}

void LevelFilterChannel::setLevel(const std::string& value) {
    if (Poco::icompare(value, "fatal") == 0) {
        setLevel(Poco::Message::PRIO_FATAL);
    } else if (Poco::icompare(value, "critical") == 0) {
        setLevel(Poco::Message::PRIO_CRITICAL);
    } else if (Poco::icompare(value, "error") == 0) {
        setLevel(Poco::Message::PRIO_ERROR);
    } else if (Poco::icompare(value, "warning") == 0) {
        setLevel(Poco::Message::PRIO_WARNING);
    } else if (Poco::icompare(value, "notice") == 0) {
        setLevel(Poco::Message::PRIO_NOTICE);
    } else if (Poco::icompare(value, "information") == 0) {
        setLevel(Poco::Message::PRIO_INFORMATION);
    } else if (Poco::icompare(value, "debug") == 0) {
        setLevel(Poco::Message::PRIO_DEBUG);
    } else if (Poco::icompare(value, "trace") == 0) {
        setLevel(Poco::Message::PRIO_TRACE);
    } else {
        throw Poco::InvalidArgumentException("Not a valid log value", value);
    }
}

Poco::Message::Priority LevelFilterChannel::getLevel() const {
    return _priority;
}

void LevelFilterChannel::setProperty(const std::string& name, const std::string& value) {
    if (Poco::icompare(name, "level") == 0) {
        setLevel(value);
    } else if (Poco::icompare(name, "channel") == 0) {
        setChannel(Poco::LoggingRegistry::defaultRegistry().channelForName(value));
    } else {
        Channel::setProperty(name, value);
    }
}

void LevelFilterChannel::log(const Poco::Message& msg) {
    if ((_priority >= msg.getPriority()) && _channel) {
        _channel->log(msg);
    }
}

}
