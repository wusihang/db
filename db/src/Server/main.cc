#include <vector>
#include <algorithm>
#include <CommonUtil/StringUtils.h>
#include <Server/Server.h>
//关于mainEntryDBServer函数声明
//该函数的实际定义位于server.cc
int mainEntryDBServer(int, char**);

//根据名称或者传递的参数判断应用类型
//例如wsdb-server  或者 wsdb --server
static bool isDBApp(const std::string& app_suffix, std::vector<char*>& argv) {
	std::string arg_mode_app = "--" + app_suffix;
	//lambda expression
	auto arg_it = std::find_if(argv.begin(), argv.end(), [&](const char* arg) {return !arg_mode_app.compare(arg);});
	if (arg_it != argv.end()) {
		argv.erase(arg_it);
		return true;
	}
	std::string app_name = "db-" + app_suffix;
	if (!argv.empty() && (!app_name.compare(argv[0]) || StringUtils::endsWith(argv[0], "/" + app_name))) {
		return true;
	}
	return false;
}

int main(int argc_, char** argv_){
	
	std::vector<char*> argv(argv_, argv_ + argc_);
	
	if (isDBApp("server", argv)) {
		auto main_func = mainEntryDBServer;
		return main_func(static_cast<int>(argv.size()), argv.data());
	}
	
	if(isDBApp("client",argv)){
		// 待完善
	}
	
	return -1;
}