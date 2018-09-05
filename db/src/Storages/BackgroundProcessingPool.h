#pragma once
#include<functional>
#include<memory>
#include<atomic>
#include<map>
#include<condition_variable>
#include<vector>
#include<thread>
#include<mutex>
#include<shared_mutex>
#include<Poco/Timestamp.h>
namespace Storage {

class BackgroundProcessingPool {

public:
    using Task = std::function<bool()>;

    class TaskInfo
    {
    public:
        /// Wake up any thread.
        void wake();

        TaskInfo(BackgroundProcessingPool & pool_, const Task & function_) : pool(pool_), function(function_) {}

    private:
        friend class BackgroundProcessingPool;

        BackgroundProcessingPool & pool;
        Task function;

        /// Read lock is hold when task is executed.
		//since C++17 
        std::shared_mutex rwlock;
        std::atomic<bool> removed {false};

        std::multimap<Poco::Timestamp, std::shared_ptr<TaskInfo>>::iterator iterator;
    };

    using TaskHandle = std::shared_ptr<TaskInfo>;
    BackgroundProcessingPool(int size_);

    size_t getNumberOfThreads() const
    {
        return size;
    }

    TaskHandle addTask(const Task & task);
    void removeTask(const TaskHandle & task);

    ~BackgroundProcessingPool();
private:
    using Tasks = std::multimap<Poco::Timestamp, TaskHandle>;    /// key is desired next time to execute (priority).
    using Threads = std::vector<std::thread>;

    const size_t size;
    static constexpr double sleep_seconds = 10;
    static constexpr double sleep_seconds_random_part = 1.0;

    Tasks tasks;         /// Ordered in priority.
    std::mutex tasks_mutex;

    Threads threads;

    std::atomic<bool> shutdown {false};
    std::condition_variable wake_event;
    void threadFunction();

};

using BackgroundProcessingPoolPtr = std::shared_ptr<BackgroundProcessingPool>;

}
