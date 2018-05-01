#include<CommonUtil/ThreadNumber.h>

#include <pthread.h>
#include<Ext/likely.h>
static __thread unsigned thread_number = 0;
static unsigned threads = 0;

unsigned ThreadUtil::ThreadNumber::get() {
    if (unlikely(thread_number == 0))
        thread_number = __sync_add_and_fetch(&threads, 1);
    return thread_number;
}
