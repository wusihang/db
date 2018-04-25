#pragma once
#include<Daemon/BaseDaemon.h>

namespace DataBase {
class Server : public BaseDaemon {

protected:
    int main(const std::vector<std::string> & args) override;
};
}
