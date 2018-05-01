#include <Common/ConfigProcessor.h>
#include <Poco/Util/XMLConfiguration.h>

DataBase::ConfigProcessor::ConfigProcessor() =default;

DataBase::ConfigProcessor::~ConfigProcessor()  = default;

Poco::AutoPtr<Poco::Util::AbstractConfiguration> DataBase::ConfigProcessor::loadConfig(const std::string& path) {
    Poco::AutoPtr<Poco::XML::Document> config_xml = processConfig(path);
    Poco::AutoPtr<Poco::Util::AbstractConfiguration> configuration(new Poco::Util::XMLConfiguration(config_xml));
    return configuration;
}

Poco::AutoPtr<Poco::XML::Document> DataBase::ConfigProcessor::processConfig(const std::string& path) {
    return dom_parser.parse(path);
}



