#include<CommonUtil/SystemUtil.h>
#include <Poco/Net/DNS.h>
#include<Poco/Exception.h>
#if defined(__APPLE__)
#include <pthread.h>
#elif defined(__FreeBSD__)
#include <pthread.h>
#include <pthread_np.h>
#else
#include <sys/prctl.h>
#endif
#include <pthread.h>
#include<Ext/likely.h>
static __thread unsigned thread_number = 0;
static unsigned threads = 0;

namespace {
std::string getFQDNOrHostNameImpl() {
    try {
        return Poco::Net::DNS::thisHost().name();
    } catch (...) {
        return Poco::Net::DNS::hostName();
    }
}
}
const std::string & SystemUtil::getFQDNOrHostName() {
    static std::string result = getFQDNOrHostNameImpl();
    return result;
}


void SystemUtil::setThreadName(const std::string& name)
{
#if defined(__FreeBSD__)
    pthread_set_name_np(pthread_self(), name);
    return;

#elif defined(__APPLE__)
    if (0 != pthread_setname_np(name))
#else
    if (0 != prctl(PR_SET_NAME, name.data(), 0, 0, 0))
#endif
    throw Poco::Exception("Cannot set thread name with prctl(PR_SET_NAME...)");
}

unsigned int SystemUtil::getThreadNumber()
{
    if (unlikely(thread_number == 0))
        thread_number = __sync_add_and_fetch(&threads, 1);
    return thread_number;
}
