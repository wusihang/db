#pragma once
#include<string>

namespace SystemUtil {
const std::string & getFQDNOrHostName();

void setThreadName(const std::string& name);

unsigned getThreadNumber();
}
