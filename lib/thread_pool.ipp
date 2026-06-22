template <typename TFunc, typename... TArgs>
inline void ThreadPool::addTask(TFunc&& func, TArgs&&... args) {
    auto bound = [func = std::forward<TFunc>(func), ... args = std::forward<TArgs>(args)]() mutable { func(args...); };

    {
        std::lock_guard<std::mutex> guard{mutex_};
        if (stop_) {
            throw std::runtime_error{"addTask on stopped ThreadPool"};
        }
        taskList_.push(std::move(bound));
    }
    cv_.notify_one();
}
