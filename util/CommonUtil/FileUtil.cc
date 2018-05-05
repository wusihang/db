#include<CommonUtil/FileUtil.h>
#include<Poco/File.h>
#include<Poco/Path.h>
#include<Poco/String.h>
#include<Poco/Exception.h>
#include<CommonUtil/LoggerUtil.h>
#include<Poco/Logger.h>
#include<sys/stat.h>

bool FileUtil::tryCreateDirectories(Poco::Logger * logger, const std::string & path) {
    try {
        Poco::File(path).createDirectories();
        return true;
    } catch (...) {
        LOG_WARNING(logger, __PRETTY_FUNCTION__ << ": when creating " << path << ", ");
    }
    return false;
}


std::string FileUtil::createDirectoryRecusively(const std::string & _path) {
    Poco::Path path(_path);
    std::string str;
    for (int j = 0; j < path.depth(); ++j) {
        str += "/";
        str += path[j];

        int res = ::mkdir(str.c_str(), 0700);
        if (res && (errno != EEXIST)) {
            throw std::runtime_error(std::string("Can't create dir - ") + str + " - " + strerror(errno));
        }
    }

    return str;
}

std::string FileUtil::getCanonicalPath(std::string&& path)
{
    Poco::trimInPlace(path);
    if (path.empty()) {
        throw Poco::Exception("path configuration parameter is empty");
    }
    if (path.back() != '/') {
        path += '/';
    }
    return path;

}

