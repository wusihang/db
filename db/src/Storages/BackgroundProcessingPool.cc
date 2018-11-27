#include<Storages/BackgroundProcessingPool.h>
#include<CommonUtil/LoggerUtil.h>
#include<Exception/ExceptionHelper.h>
#include<CommonUtil/SystemUtil.h>
#include<random>
Storage::BackgroundProcessingPool::BackgroundProcessingPool(int size_)
    :size(size_)
{
    LOG_INFO(&Logger::get("BackgroundProcessingPool"), "Create BackgroundProcessingPool with " << size << " threads");
    threads.resize(size);
    for (auto & thread : threads)
    {
        thread = std::thread([this] { threadFunction(); });
    }
}

Storage::BackgroundProcessingPool::TaskHandle Storage::BackgroundProcessingPool::addTask(const Task& task)
{
    //创建一个任务处理句柄
    TaskHandle res = std::make_shared<TaskInfo>(*this, task);
    Poco::Timestamp current_time;
    {
        //添加任务时,加锁
        std::unique_lock<std::mutex> lock(tasks_mutex);
        //当前时间作为key,任务列表中,加入任务处理句柄
        res->iterator = tasks.emplace(current_time, res);
    }
    wake_event.notify_all();
    return res;
}


void Storage::BackgroundProcessingPool::removeTask(const Storage::BackgroundProcessingPool::TaskHandle& task)
{
    // 将removed置为true,并且返回原来的状态,如果原来就已经removed,那么就直接返回
    if (task->removed.exchange(true))
        return;

    /// Wait for all executions of this task
    {
        //独享锁, 如果当前任务已经在执行,那么阻塞,否则获取独享锁,然后释放
        //这是因为shared_mutex是写优先的,因此,这里尝试获取写锁,那么会阻塞在这之后尝试获取读锁的线程.
        std::unique_lock<std::shared_mutex> wlock(task->rwlock);
    }

    {
        //移除任务时加锁
        std::unique_lock<std::mutex> lock(tasks_mutex);
        tasks.erase(task->iterator);
    }
}

void Storage::BackgroundProcessingPool::TaskInfo::wake()
{
    //如果任务已经被移除,那么就直接返回
    if (removed)
        return;

    Poco::Timestamp current_time;
    {
        std::unique_lock<std::mutex> lock(pool.tasks_mutex);

        //获取要执行的时间
        auto next_time_to_execute = iterator->first;
        TaskHandle this_task_handle = iterator->second;

        /// If this task was done nothing at previous time and it has to sleep, then cancel sleep time.
        if (next_time_to_execute > current_time)
            next_time_to_execute = current_time;

        pool.tasks.erase(iterator);
        iterator = pool.tasks.emplace(next_time_to_execute, this_task_handle);
    }

    /// Note that if all threads are currently do some work, this call will not wakeup any thread.
    pool.wake_event.notify_one();
}


void Storage::BackgroundProcessingPool::threadFunction()
{
    SystemUtil::setThreadName("BackgrProcPool");
    std::mt19937 rng(reinterpret_cast<intptr_t>(&rng));
    //均匀分布,0~1,线程随机等待
    std::this_thread::sleep_for(std::chrono::duration<double>(std::uniform_real_distribution<double>(0, sleep_seconds_random_part)(rng)));

    while (!shutdown)
    {
        bool done_work = false;
        TaskHandle task;

        try
        {
            Poco::Timestamp min_time;
            {
                //排它锁
                std::unique_lock<std::mutex> lock(tasks_mutex);
                if (!tasks.empty())
                {
                    for (const auto & time_handle : tasks)
                    {
                        if (!time_handle.second->removed)
                        {
                            min_time = time_handle.first;
                            task = time_handle.second;
                            break;
                        }
                    }
                }
            }

            if (shutdown)
                break;

            if (!task)
            {
                std::unique_lock<std::mutex> lock(tasks_mutex);
                wake_event.wait_for(lock,
                                    std::chrono::duration<double>(sleep_seconds
                                            + std::uniform_real_distribution<double>(0, sleep_seconds_random_part)(rng)));
                continue;
            }

            /// No tasks ready for execution.
            Poco::Timestamp current_time;
            //任务的执行时间还没到
            if (min_time > current_time)
            {
                std::unique_lock<std::mutex> lock(tasks_mutex);
                wake_event.wait_for(lock, std::chrono::microseconds(
                                        min_time - current_time + std::uniform_int_distribution<uint64_t>(0, sleep_seconds_random_part * 1000000)(rng)));
            }

            std::shared_lock<std::shared_mutex> rlock(task->rwlock);
            if (task->removed)
                continue;

            {
                done_work = task->function();
            }
        }
        catch (...)
        {
            DataBase::currentExceptionLog();
        }

        if (shutdown)
            break;

        /// If task has done work, it could be executed again immediately.
        /// If not, add delay before next run.
        Poco::Timestamp next_time_to_execute = Poco::Timestamp() + (done_work ? 0 : sleep_seconds * 1000000);

        {
            std::unique_lock<std::mutex> lock(tasks_mutex);

            if (task->removed)
                continue;

            tasks.erase(task->iterator);
            task->iterator = tasks.emplace(next_time_to_execute, task);
        }
    }
}

Storage::BackgroundProcessingPool::~BackgroundProcessingPool()
{
    try
    {
        //后台线程池释放时, 唤醒所有阻塞的任务,并且等待执行完成
        shutdown = true;
        wake_event.notify_all();
        for (std::thread & thread : threads)
        {
            thread.join();
        }
    }
    catch (...)
    {
        DataBase::currentExceptionLog();
    }
}
