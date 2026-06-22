#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>

class ThreadPool {
   public:
    ThreadPool(size_t threadCount);

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    ~ThreadPool();

    template <typename TFunc, typename... TArgs>
    void addTask(TFunc&& func, TArgs&&... args);

   private:
    using TTask = std::function<void()>;
    std::queue<TTask> taskList_;
    std::vector<std::thread> threads_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_ = false;
};

#include "thread_pool.ipp"