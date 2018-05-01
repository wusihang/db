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

}
