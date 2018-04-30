#pragma once
#include<Poco/Util/Application.h>
class ServerAppHandler {
public:
    template<typename App>
    static void init(App& app, int _argc, char* _argv[]) {
    }
    template<typename App>
    static int run(App& app, int _argc, char* _argv[]) {
        return app.run(_argc, _argv);
    }
};

//typename or class
template<typename App, typename AppHandler>
class AppMainFuncImpl {
public:
    static int main(int _argc, char * _argv[]) {
        App app;
        try {
            AppHandler::init(app, _argc, _argv);
            return AppHandler::run(app, _argc, _argv);
        } catch (...) {
            //待填写
        }
        return Poco::Util::Application::EXIT_CONFIG;
    }
};

// 类名 + 声明的主函数名称,实际调用AppMainFuncImpl模板的main函数
#define APP_SERVER_MAIN_FUNC(AppServerClassName, main_func) int main_func(int _argc, char * _argv[]) { return AppMainFuncImpl<AppServerClassName, ServerAppHandler>::main(_argc, _argv); }
