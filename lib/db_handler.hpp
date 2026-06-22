#include "cache.hpp"
#include "thread_pool.hpp"

namespace db_handler{

template <int ThreadCount, int MutexCount, Policies Policy, typename... Types>
class Handler{
public:

    using Variant = Cache<Policy, Types...>::Variant;
    using Element = Cache<Policy, Types...>::Element;

    Variant* Get(const std::string& key);

    bool Set(const std::string& key, Variant value);

private:
    ThreadPool thread_pool_ = ThreadPool(ThreadCount);
    Cache<Policy, Types...> cache_module_ = Cache<Policy, Types...>();
    std::mutex cache_mutex_;                                                    // temporarily one mutex, later could be  fragmentation
    // std::mutex fragments_mutexes[MutexCount];                                // commented for now, will test on later versions
};

}; // namespace db_handler



#include "db_handler.tpp"