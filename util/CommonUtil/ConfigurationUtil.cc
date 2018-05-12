#include<CommonUtil/ConfigurationUtil.h>
#include<CommonUtil/StringUtils.h>
#include<Poco/Util/AbstractConfiguration.h>

std::vector< std::string > ConfigurationUtil::getMultipleKeysFromConfig(const Poco::Util::AbstractConfiguration& config, const std::string& root, const std::string& name)
{
    std::vector<std::string> values;
    Poco::Util::AbstractConfiguration::Keys config_keys;
    config.keys(root, config_keys);
    for (const auto & key : config_keys)
    {
        if (key != name && !(StringUtils::startsWith(key.data(), name + "[") && StringUtils::endsWith(key.data(), "]")))
            continue;
        values.emplace_back(key);
    }
    return values;
}

std::vector<std::string> ConfigurationUtil::getMultipleValuesFromConfig(const Poco::Util::AbstractConfiguration & config, const std::string & root, const std::string & name)
{
    std::vector<std::string> values;
    for (const auto & key : ConfigurationUtil::getMultipleKeysFromConfig(config, root, name))
    {
        values.emplace_back(config.getString(key));
    }
    return values;
}
