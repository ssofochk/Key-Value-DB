#include "cache.hpp"
#include "thread_pool.hpp"
#include "fileworker.hpp"
#include <future>
#include <memory>
#include <type_traits>

namespace db_handler{

template<typename T>
concept IsFilePath = std::is_same_v<std::decay_t<T>, std::filesystem::path>;

template <int ThreadCount, int MutexCount, IsFilePath FileWorkerPath, Policies Policy, typename... Types>
class Handler{
public:

    using Variant = Cache<Policy, Types...>::Variant;
    using Element = Cache<Policy, Types...>::Element;


    std::future<Variant*> Get(const std::string& key);

    std::future<bool> Set(const std::string& key, Variant value);

private:
    ThreadPool thread_pool_ = ThreadPool(ThreadCount);
    // FileWorker io_worker_ = FileWorker(FileWorkerPath);
    Cache<Policy, Types...> cache_module_ = Cache<Policy, Types...>();
    std::mutex cache_mutex_;                                                    // temporarily one mutex, later could be  fragmentation
    // std::mutex fragments_mutexes[MutexCount];                                // commented for now, will test on later versions
};

}; // namespace db_handler



#include "db_handler.tpp"