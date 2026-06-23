#include <bit>
#include <future>
#include <memory>
#include <new>
#include <shared_mutex>
#include <type_traits>

#include "cache.hpp"
#include "fileworker.hpp"
#include "thread_pool.hpp"

namespace db_handler {

template <auto S>
concept PathStringType = requires { std::filesystem::path(S); };

template <size_t ThreadCount, size_t MutexCount, size_t MaxKeySize, size_t MaxValueSize, auto FileWorkerPath,
          Policies Policy, bool IsMutexShared = false>
    requires PathStringType<FileWorkerPath> && (std::has_single_bit(MutexCount))
class Handler {
   public:
    std::future<std::optional<std::string>> Get(const std::string& key);

    std::future<bool> Set(const std::string& key, const std::string& value);

   private:
    ThreadPool thread_pool_ = ThreadPool(ThreadCount);
    FileWorker<MaxKeySize, MaxValueSize> io_worker_{FileWorkerPath};
    Cache<Policy> cache_module_ = Cache<Policy>();

    using ChosenMutex = std::conditional_t<IsMutexShared, std::shared_mutex, std::mutex>;

    struct alignas(std::hardware_destructive_interference_size) AlignedMutex {
        ChosenMutex mtx;
    };
    AlignedMutex fragments_mutexes[MutexCount];

    std::hash<std::string> hash_func_;

    size_t GetMutexHash(const std::string& key);
};

}  // namespace db_handler

#include "db_handler.tpp"