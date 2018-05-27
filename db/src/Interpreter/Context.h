#pragma once
#include<string>
#include<mutex>
#include<memory>
#include<condition_variable>
#include<thread>
#include<atomic>
#include<chrono>
#include<utility>
#include<Interpreter/ClientInfo.h>
#include<Poco/Types.h>
namespace Poco {
class Mutex;
}
namespace DataBase {

struct ContextShared;
class SessionKeyHash;

class Context {
public:
    using SessionKey = std::pair<std::string, std::string>;
    ~Context();
	//创建全局的Context
    static Context createGlobal();

    void setGlobalContext(Context & context_) {
        global_context = &context_;
    }

    //设置session上下文
    void setSessionContext(Context & context_) {
        session_context = &context_;
    }
    const Context & getSessionContext() const;
    Context & getSessionContext();

	//请求获得一个session,一个session就是一个Context
    std::shared_ptr<Context> acquireSession(const std::string & session_id, std::chrono::steady_clock::duration timeout, bool session_check) const;

	//释放一个session
    void releaseSession(const std::string & session_id, std::chrono::steady_clock::duration timeout);
	
	//关闭过期的session
	std::chrono::steady_clock::duration closeSessions() const;

	//获得全局锁
    std::unique_lock<Poco::Mutex> getLock() const;

	//获取当前数据库
    std::string getCurrentDatabase() const;
    void setCurrentDatabase(const std::string & name);
	//判断数据库是否存在,如果不存在,那么抛出异常
    void assertDatabaseExists(const std::string & database_name, bool check_database_acccess_rights = true) const;

	//获取客户端信息
    ClientInfo& getClientInfo() {
        return client_info;
    }
    const ClientInfo& getClientInfo() const {
        return client_info;
    }
    
    void shutdown();

    enum ApplicationType {
        SERVER,         /// The program is run as wsdb-server daemon (default behavior)
        CLIENT,         /// wsdb-client
        LOCAL           /// wsdb-local
    };
private:
    Context(); //禁止直接构造,可以使用createGlobal或拷贝构造函数构造
	//获得SessionKey,实际是session_id + 用户名的一个对
    SessionKey getSessionKey(const std::string & session_id) const;
	//调度session,准备close
    void scheduleCloseSession(const SessionKey & key, std::chrono::steady_clock::duration timeout);

	//全局context指针
    Context * global_context = nullptr;
	//session指针
    Context * session_context = nullptr;
	//当前数据库
    std::string current_database;
	
	//共享Context的指针
    std::shared_ptr<ContextShared> shared;
	//客户端信息
    ClientInfo client_info;
	//session是否正在使用
    bool session_is_used = false;
	//session关闭周期
    Poco::UInt64 session_close_cycle = 0;
};


//session清理线程
class SessionCleaner
{
public:
    SessionCleaner(Context & context_)
        : context {context_}
    {
    }
    ~SessionCleaner();

private:
    void run();

    Context & context;

    std::mutex mutex;
    std::condition_variable cond;
    std::atomic<bool> quit {false};
	//清理线程,线程方法就是当前对象的run , mutex + condition_variable + quit标志  实际上就是一个信号量
    std::thread thread {&SessionCleaner::run, this};
};

}
