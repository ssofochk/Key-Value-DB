#include <sstream>

namespace db_handler {

#define DB_HANDLER_MACRO \
    Handler<ThreadCount, MutexCount, MaxKeySize, MaxValueSize, Policy, IsMutexShared>

template <size_t ThreadCount, size_t MutexCount, size_t MaxKeySize, size_t MaxValueSize,
          Policies Policy, bool IsMutexShared>
    requires (std::has_single_bit(MutexCount))
std::future<std::optional<std::string>> DB_HANDLER_MACRO::Get(const std::string& key) {
    auto promise_ptr = std::make_shared<std::promise<std::optional<std::string>>>();

    std::future<std::optional<std::string>> future_res = promise_ptr->get_future();

    thread_pool_.addTask(
        [this, promise_ptr](const std::string& key) mutable {
            using LockType =
                std::conditional_t<IsMutexShared, std::shared_lock<std::shared_mutex>, std::lock_guard<std::mutex>>;
            size_t mtx_idx = GetMutexHash(key);
            try {
                {
                    LockType guard{fragments_mutexes[mtx_idx].mtx};
                    if (auto* res_ptr = cache_module_.Get(key)) {
                        promise_ptr->set_value(std::optional<std::string>(*res_ptr));
                        return;
                    }
                }

                auto file_res = io_worker_.read(key);

                if (file_res.has_value()) {
                    LockType guard{fragments_mutexes[mtx_idx].mtx};
                    cache_module_.Put(key, file_res.value());
                }

                promise_ptr->set_value(file_res);
            } catch (...) {
                promise_ptr->set_exception(std::current_exception());
            }
        },
        key);

    return future_res;
}

template <size_t ThreadCount, size_t MutexCount, size_t MaxKeySize, size_t MaxValueSize,
Policies Policy, bool IsMutexShared>
    requires (std::has_single_bit(MutexCount))
std::future<bool> DB_HANDLER_MACRO::Put(const std::string& key, const std::string& value) {
    auto promise_ptr = std::make_shared<std::promise<bool>>();

    std::future<bool> future_res = promise_ptr->get_future();

    using LockType =
        std::conditional_t<IsMutexShared, std::unique_lock<std::shared_mutex>, std::lock_guard<std::mutex>>;
    thread_pool_.addTask(
        [this, promise_ptr](const std::string& key, const std::string& value) mutable {
            size_t mtx_idx = GetMutexHash(key);
            try {
                {
                    LockType guard{fragments_mutexes[mtx_idx].mtx};
                    bool res = cache_module_.Put(key, value);
                    promise_ptr->set_value(res);
                }

                io_worker_.write(key, value);
            } catch (...) {
                promise_ptr->set_exception(std::current_exception());
            }
        },
        key, value);

    return future_res;
}

template <size_t ThreadCount, size_t MutexCount, size_t MaxKeySize, size_t MaxValueSize,
          Policies Policy, bool IsMutexShared>
    requires (std::has_single_bit(MutexCount))
size_t DB_HANDLER_MACRO::GetMutexHash(const std::string& key) {
    return hash_func_(key) & (MutexCount - 1);
}

}  // namespace db_handler