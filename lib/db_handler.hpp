#include "cache.hpp"
#include "thread_pool.hpp"

namespace db_handler{

template <int ThreadCount, Policies Policy>
class Handler{
public:

    using Type = std::string;
    using Element = Cache<Policy>::Element;

    Type* Get(const std::string& key);

    bool Set(const std::string& key, Type value);

private:
    ThreadPool thread_pool = ThreadPool(ThreadCount);
    Cache<Policy> cache_module;
    std::mutex cache_mutex;                                 // temporarily one mutex, later could be  fragmentation
    // std::mutex fragments_mutexes[ThreadCount];           // commented for now, will test on later versions
};

}; // namespace db_handler



#include "db_handler.tpp"