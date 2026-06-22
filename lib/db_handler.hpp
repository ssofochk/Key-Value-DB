#include "cache.hpp"
#include "thread_pool.hpp"
#include "fileworker.hpp"
#include <future>
#include <memory>
#include <type_traits>
#include <new>
#include <shared_mutex>

namespace db_handler{

template<typename T>
concept IsFilePath = std::is_same_v<std::decay_t<T>, std::filesystem::path>;

template <int ThreadCount, int MutexCount, IsFilePath auto FileWorkerPath, Policies Policy, bool IsMutexShared = false>
class Handler{
public:

    std::future<std::optional<std::string>> Get(const std::string& key);

    std::future<bool> Set(const std::string& key, const std::string& value);

private:
    ThreadPool thread_pool_ = ThreadPool(ThreadCount);
    FileWorker io_worker_ = FileWorker(FileWorkerPath);
    Cache<Policy> cache_module_ = Cache<Policy>();

    using ChosenMutex = std::conditional_t<IsMutexShared, std::shared_mutex, std::mutex>;

    struct alignas(std::hardware_destructive_interference_size) AlignedMutex {
        ChosenMutex mtx;
    };
    AlignedMutex fragments_mutexes[MutexCount];

    std::hash<std::string> hash_func_;

    size_t GetMutexHash(const std::string& key);

};

}; // namespace db_handler



#include "db_handler.tpp"