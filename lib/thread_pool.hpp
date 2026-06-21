#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <stdexcept>


class ThreadPool {
    using TTask = std::function<void()>;
public:
    ThreadPool(size_t threadCount) {
        for(size_t i = 0; i < threadCount; ++i) {
            threads_.emplace_back(
                [this]() {
                    while(true) {
                        TTask task;
                        std::unique_lock<std::mutex> lock{mutex_};
                        cv_.wait(lock, [this] {
                            return !taskList_.empty() || stop_;
                        });

                        if(taskList_.empty()){
                            return;
                        }

                        task = std::move(taskList_.front());
                        taskList_.pop();
                        lock.unlock();

                        task();
                    }
                }
            );
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;


    ~ThreadPool() {
        mutex_.lock();
        stop_ = true;
        mutex_.unlock();

        cv_.notify_all();

        for(auto& tr : threads_){
            if(tr.joinable()){
                tr.join();
            }
        }
    }


    template<typename TFunc, typename... TArgs>
    void addTask(TFunc&& func, TArgs&&... args) {
        auto bound = [
            func = std::forward<TFunc>(func),
            ...args = std::forward<TArgs>(args)
        ]() mutable {
            func(args...);
        };

        mutex_.lock();
        if(stop_)
            throw std::runtime_error{"addTask on stopped ThreadPool"};
        taskList_.push(std::move(bound));
        mutex_.unlock();
        cv_.notify_one();
    }

private:
    std::queue<TTask> taskList_;
    std::vector<std::thread> threads_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_ = false;
};

