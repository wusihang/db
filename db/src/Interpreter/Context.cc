#include<Interpreter/Context.h>
#include<Poco/Exception.h>
#include<Poco/Mutex.h>
#include<Ext/std_ext.h>
#include<functional>
#include<unordered_map>
#include<vector>
#include<deque>
#include<iostream>
#include<CommonUtil/LoggerUtil.h>
#include<CommonUtil/SystemUtil.h>
namespace ErrorCodes
{
extern const int THERE_IS_NO_SESSION;
extern const int UNKNOWN_DATABASE;
extern const int LOGICAL_ERROR;
extern const int SESSION_IS_LOCKED;
}

namespace DataBase {
class SessionKeyHash
{
public:
    size_t operator()(const Context::SessionKey & key) const
    {
        std::hash<std::string> hash_fn;
        size_t first = hash_fn(key.first);
        size_t second = hash_fn(key.second);
        first ^= second + 0x9e3779b9 + (first<<6) + (first>>2);
        return first;
    }
};

struct ContextShared {
    mutable Poco::Mutex mutex;
    using Sessions = std::unordered_map<Context::SessionKey, std::shared_ptr<Context>, SessionKeyHash>;
    using CloseTimes = std::deque<std::vector<Context::SessionKey> >;
    Sessions sessions;
    CloseTimes close_times;
    std::chrono::steady_clock::duration close_interval = std::chrono::seconds(1);
    Poco::UInt64 close_cycle = 0;
    std::chrono::steady_clock::time_point close_cycle_time = std::chrono::steady_clock::now();
    bool shutdown_called = false;
    void shutdown() {
        if(shutdown_called) {
            return;
        }
        shutdown_called  = true;
    }

    ContextShared() {
        static std::atomic<size_t> num_calls {0};
        //确保共享Context只被初始化一次
        if (++num_calls > 1)
        {
            std::cerr << "Attempting to create multiple ContextShared instances. \n";
            std::cerr.flush();
            std::terminate();
        }
    }

    ~ContextShared() {
        shutdown();
    }
};
}

static std::string resolveDatabase(const std::string & database_name, const std::string & current_database)
{
    std::string res = database_name.empty() ? current_database : database_name;
    if (res.empty())
    {
        throw Poco::Exception("Default database is not selected", ErrorCodes::UNKNOWN_DATABASE);
    }
    return res;
}

DataBase::Context::Context() = default;

DataBase::Context::~Context() = default;

//创建一个Context,并且将shared初始化
DataBase::Context DataBase::Context::createGlobal()
{
    DataBase::Context context;
    context.shared = std::make_shared<ContextShared>();
    return context;
}

//获取session上下文
const DataBase::Context & DataBase::Context::getSessionContext() const
{
    if (!session_context)
        throw Poco::Exception("There is no session",ErrorCodes::THERE_IS_NO_SESSION);
    return *session_context;
}

DataBase::Context & DataBase::Context::getSessionContext()
{
    if (!session_context)
        throw Poco::Exception("There is no session",ErrorCodes::THERE_IS_NO_SESSION);
    return *session_context;
}

void DataBase::Context::setCurrentDatabase(const std::string& name)
{
    //作用域锁,离开作用域后自动释放
    auto lock = getLock();
    assertDatabaseExists(name);
    current_database = name;
}

std::string DataBase::Context::getCurrentDatabase() const
{
    auto lock = getLock();
    return current_database;
}

void DataBase::Context::assertDatabaseExists(const std::string& database_name, bool /*check_database_acccess_rights*/) const
{
    auto lock = getLock();
    //判断待设置的库名和当前库名是否同时为空,如果是,那么就抛出异常
    std::string db = resolveDatabase(database_name,current_database);
    if(db.empty()) {
        throw Poco::Exception("database "+db+" not exist",ErrorCodes::UNKNOWN_DATABASE);
    }
}


std::shared_ptr< DataBase::Context > DataBase::Context::acquireSession(const std::string& session_id, std::chrono::steady_clock::duration timeout, bool session_check) const
{
    auto lock = getLock();
    //根据session_id获得一个key,这个key是由session_id + 用户名组成
    const auto & key = getSessionKey(session_id);
    //从shared已保存的session中查找这个session
    auto it = shared->sessions.find(key);
    //如果未找到
    if(it == shared->sessions.end())
    {
        //如果获取需求是要求要找到,那么抛出异常
        if(session_check)
        {
            throw Poco::Exception("seesion not exist", ErrorCodes::THERE_IS_NO_SESSION);
        }
        //从全局context拷贝一个Context副本
        auto new_session = std::make_shared<Context>(*global_context);
        //新的context计算调度信息
        new_session->scheduleCloseSession(key,timeout);
        //it = key   , unordered_map插入后,返回这个插入的元素
        it =  shared->sessions.insert(std::make_pair(key,std::move(new_session))).first;
    } else if(it->second->client_info.current_user != client_info.current_user)
    {
        throw Poco::Exception("Session belongs to a different user",ErrorCodes::LOGICAL_ERROR);
    }
    //sessionContext
    const auto & session = it->second;
    if (session->session_is_used)
    {
        throw Poco::Exception("Session is locked by a concurrent client.", ErrorCodes::SESSION_IS_LOCKED);
    }
    session->session_is_used = true;
    session->client_info = client_info;
    return session;
}


//释放session
void DataBase::Context::releaseSession(const std::string& session_id, std::chrono::steady_clock::duration timeout)
{
    auto lock = getLock();
    //清除使用标志
    session_is_used = false;
    //计算调度时间
    scheduleCloseSession(getSessionKey(session_id), timeout);
}

std::chrono::steady_clock::duration DataBase::Context::closeSessions() const
{
    auto lock = getLock();
    //当前时间
    const auto now = std::chrono::steady_clock::now();
    //如果当前时间 < 关闭session的时间点, 那么返回剩余的过期时间
    if (now < shared->close_cycle_time)
        return shared->close_cycle_time - now;
    //当前关闭周期 = 共享区的周期
    const auto current_cycle = shared->close_cycle;
    //共享区周期 + 1
    ++shared->close_cycle;
    //关闭时间点 = 当前时间 + 关闭周期(1s)
    shared->close_cycle_time = now + shared->close_interval;
    //如果关闭队列为空,返回关闭周期(1s)
    if (shared->close_times.empty())
        return shared->close_interval;
    //获得close_times队列的第一个元素,也就是一个sessionKey列表
    auto & sessions_to_close = shared->close_times.front();
    for (const auto & key : sessions_to_close)
    {
        //在session中查找对应的key
        const auto session = shared->sessions.find(key);
        //如果找到了对应的session,并且该session的session关闭周期 <=  当前周期
        if (session != shared->sessions.end() && session->second->session_close_cycle <= current_cycle)
        {
            //如果session正在被使用,那么重新调度该session, 否则就从共享区的sessions里头抹去该session
            if (session->second->session_is_used)
                session->second->scheduleCloseSession(key, std::chrono::seconds(0));
            else
                shared->sessions.erase(session);
        }
    }
    //第一个元素出列
    shared->close_times.pop_front();
    //返回共享区的关闭周期(1s)
    return shared->close_interval;
}


void DataBase::Context::scheduleCloseSession(const DataBase::Context::SessionKey& key, std::chrono::steady_clock::duration timeout)
{
    //关闭的index = 超时时间 / 关闭周期   +1
    const Poco::UInt64 close_index = timeout / shared->close_interval + 1;
    //新的关闭周期 =  共享关闭周期  + close_index
    const auto new_close_cycle = shared->close_cycle + close_index;

    //如果当前上下文的session关闭周期不等于计算后对应的周期,那么设置当前上下文的session关闭周期
    if (session_close_cycle != new_close_cycle)
    {
        session_close_cycle = new_close_cycle;
        //如果共享区的close_times队列大小<关闭index,那么重新分配close_times的大小
        if (shared->close_times.size() < close_index + 1)
        {
            shared->close_times.resize(close_index + 1);
        }
        //将对应的sessionKey加入到队列指定index的列表中
        shared->close_times[close_index].emplace_back(key);
    }
}


DataBase::Context::SessionKey DataBase::Context::getSessionKey(const std::string& session_id) const
{
    auto & user_name = client_info.current_user;
    if (user_name.empty())
    {
        throw Poco::Exception("Empty user name.", ErrorCodes::LOGICAL_ERROR);
    }
    return SessionKey(user_name, session_id);
}

//unique_lock是作用域范围内的锁
std::unique_lock< Poco::Mutex > DataBase::Context::getLock() const
{
    return std::unique_lock<Poco::Mutex>(shared->mutex);
}

void DataBase::Context::shutdown()
{
    shared->shutdown();
}




//session定时清理任务
void DataBase::SessionCleaner::run()
{
    //设置线程名称
    SystemUtil::setThreadName("HTTPSessionCleaner");
    //unique锁
    std::unique_lock<std::mutex> lock {mutex};
    while (true)
    {
        //关闭sessions并获取间隔
        auto interval = context.closeSessions();
        //当前线程在条件变量cond上等待至多interval毫秒,超时或被其他线程唤醒时,执行回调,返回是否quit
        //条件变量等待interval后执行回调函数,回调函数中返回指示变量quit
        if (cond.wait_for(lock, interval, [this]() -> bool { return quit; }))
            break;
    }
}


DataBase::SessionCleaner::~SessionCleaner()
{
    try
    {
        {
            std::lock_guard<std::mutex> lock {mutex};
            quit = true;
        }
        cond.notify_one();
        thread.join();
    }
    catch (...)
    {
        DataBase::currentExceptionLog();
    }
}






