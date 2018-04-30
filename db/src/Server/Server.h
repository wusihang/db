#pragma once
#include<Daemon/BaseDaemon.h>

namespace DataBase {

//Poco提供了快速搭建一个应用框架的能力，只要继承ServerAplication并实现其几个virtual函数即可
// 部分virtual函数在BaseDaemon中实现
class Server : public BaseDaemon {

protected:
	//继承自ServerApplication的应用必须实现main函数，主要的应用都在该函数中完成
	// 一般会在该函数的实现的最后调用waitForTerminationRequest函数，等待外部命令来停止应用
    int main(const std::vector<std::string> & args) override;
};
}
