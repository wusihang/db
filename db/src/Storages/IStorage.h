#pragma once
#include <Poco/Exception.h>

namespace Storage {

class IStorage {
public:
    virtual std::string getName() const = 0;
    virtual void startup() {};
    virtual void shutdown() {};
};

}
