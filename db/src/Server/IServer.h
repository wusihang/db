#pragma once

#include<Poco/Logger.h>
#include<Poco/Util/LayeredConfiguration.h>
namespace DataBase {
class Context;

class IServer
{
public:
    /// 返回应用配置
    virtual Poco::Util::LayeredConfiguration & config() const = 0;

    /// 返回日志.
    virtual Poco::Logger & logger() const = 0;

    /// 返回上下文信息
    virtual Context & context() const = 0;

    /// 是否取消了
    virtual bool isCancelled() const = 0;

    virtual ~IServer() {}
};

}
