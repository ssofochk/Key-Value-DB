#include "thread_pool.hpp"

ThreadPool::ThreadPool(size_t threadCount) {
    for (size_t i = 0; i < threadCount; ++i) {
        threads_.emplace_back([this]() {
            while (true) {
                TTask task;
                {
                    std::unique_lock<std::mutex> lock{mutex_};
                    cv_.wait(lock, [this] { return !taskList_.empty() || stop_; });

                    if (taskList_.empty()) {
                        return;
                    }

                    task = std::move(taskList_.front());
                    taskList_.pop();
                }

                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> guard{mutex_};
        stop_ = true;
    }
    cv_.notify_all();

    for (auto& tr : threads_) {
        if (tr.joinable()) {
            tr.join();
        }
    }
}
