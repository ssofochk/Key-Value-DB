#include <bit>
#include <future>
#include <memory>
#include <new>
#include <shared_mutex>
#include <type_traits>

#include "cache/cache.hpp"
#include "fileworker/fileworker.hpp"
#include "thread_pool/thread_pool.hpp"

namespace db_handler {


template <size_t ThreadCount, size_t MutexCount, size_t MaxKeySize, size_t MaxValueSize,
          Policies Policy, bool IsMutexShared = false>
    requires (std::has_single_bit(MutexCount))
class Handler {
public:
    Handler() : io_worker_(FileWorker<MaxKeySize, MaxValueSize>("data_base.txt")), thread_pool_(ThreadCount) {}

    Handler(std::filesystem::path path) : io_worker_(FileWorker<MaxKeySize, MaxValueSize>(path)), thread_pool_(ThreadCount) {}

    std::future<std::optional<std::string>> Get(const std::string& key);

    std::future<bool> Put(const std::string& key, const std::string& value);

    void SetMaxMemory(std::size_t max_memory) {
        cache_module_.SetMaxMemory(max_memory);
    }

    std::uint64_t GetEvictionCount() const {
        return cache_module_.GetEvictionCount();
    }

    std::size_t EstimateMemoryBytes() const {
        return cache_module_.EstimateMemoryBytes();
    }

    uint64_t GetHits() const {
        return cache_module_.GetHits();
    }

    uint64_t GetMisses() const {
        return cache_module_.GetMisses();
    }

private:
    ThreadPool thread_pool_;
    FileWorker<MaxKeySize, MaxValueSize> io_worker_;
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