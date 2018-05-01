#pragma once
#include <Poco/Foundation.h>
#include <Poco/Channel.h>
#include <Poco/Mutex.h>
#include <Poco/Message.h>
#include <vector>

namespace DataBase {

//这个channel只打印比设定level更高的日志
class Foundation_API LevelFilterChannel: public Poco::Channel
{
public:
	//将消息发送给所有关联的channel
    void log(const Poco::Message& msg);

	//设置或改变配置属性，只支持level属性，用于设置要打印的最小level级别
    void setProperty(const std::string& name, const std::string& value);

	//设置目标channel，格式化后的消息将会传递给这个channel
    void setChannel(Poco::Channel* pChannel);
	//获取目标channel
    Poco::Channel* getChannel() const;
	//打开关联的channel
    void open();
	//关闭关联的channel
    void close();
	//设置日志级别
    void setLevel(Poco::Message::Priority);
	//设置日志级别（使用symbolic值）
	void setLevel(const std::string& value);
	//返回日志级别
	Poco::Message::Priority getLevel() const;

protected:
    ~LevelFilterChannel();

private:
    Channel* _channel = nullptr;
    Poco::Message::Priority _priority = Poco::Message::PRIO_ERROR;
};

}
