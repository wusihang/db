#pragma once
#include<vector>
#include<string>
namespace Poco {
namespace Util {
class AbstractConfiguration;
}
}
namespace ConfigurationUtil {

std::vector<std::string> getMultipleKeysFromConfig(const Poco::Util::AbstractConfiguration & config, const std::string & root, const std::string & name);

std::vector<std::string> getMultipleValuesFromConfig(const Poco::Util::AbstractConfiguration & config, const std::string & root, const std::string & name);
}
