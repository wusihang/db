#pragma once
#include<vector>
#include<string>
#include<utility>
#include<Poco/AutoPtr.h>
#include<Poco/Util/AbstractConfiguration.h>
#include<Poco/DOM/Document.h>
#include<Poco/DOM/DOMParser.h>
namespace DataBase {
class ConfigProcessor {
public:
    ConfigProcessor();
    ~ConfigProcessor();
    Poco::AutoPtr<Poco::Util::AbstractConfiguration> loadConfig(const std::string& path);

    Poco::AutoPtr<Poco::XML::Document> processConfig(const std::string & path);

private:
    Poco::XML::DOMParser dom_parser;
};
}
