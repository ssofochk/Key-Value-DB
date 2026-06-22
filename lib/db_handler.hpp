#include "cache.hpp"
#include "thread_pool.hpp"

namespace db_handler{

template <int ThreadCount, Policies Policy, typename... Types>
class Handler{
public:

    using Variant = Cache<Policy, Types...>::Variant;
    using Element = Cache<Policy, Types...>::Element;

    Variant* Get(const std::string& key);

    bool Set(const std::string& key, Variant value);

private:
    ThreadPool thread_pool = ThreadPool(ThreadCount);
    Cache<Policy, Types...> cache_module;
    std::mutex cache_mutex;                                 // temporarily one mutex, later could be  fragmentation
    // std::mutex fragments_mutexes[ThreadCount];           // commented for now, will test on later versions
};

}; // namespace db_handler



#include "db_handler.tpp"