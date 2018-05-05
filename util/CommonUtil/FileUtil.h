#pragma once
#include<string>
namespace Poco {
class Logger;
}

namespace FileUtil {

//创建目录
bool tryCreateDirectories(Poco::Logger * logger, const std::string & path);

//递归创建目录
std::string createDirectoryRecusively(const std::string& _path);

//获取canoical路径，参数中std::string&&是表示右值引用
//允许外部传入一个非const临时变量
// +++++++++++++++++++++++++++++
//如果是左值引用（即std::string&），那么外部传入必须是非临时变量，因为临时变量是瞬间就丢失的
//如果是左值引用const(即const std::string&)，那么外部传入可以是临时变量，但是函数内部不能修改该变量
std::string getCanonicalPath(std::string && path) ;

}
