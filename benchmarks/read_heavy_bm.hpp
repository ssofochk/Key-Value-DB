#pragma once
#include <random>

class ReadHeavyWorkLoad {
private:
    double read_percent = 0.95;
    double write_percent = 0.05;
public:
    template <Policies Policy>
    uint64_t execute(DataBase<Policy>& cache, const std::vector<std::string>& keys) {
        if (Random() % 100 < read_percent * 100) {
            auto start = std::chrono::steady_clock::now();
            cache.Get(keys[Random() % keys.size()]).get();
            auto end = std::chrono::steady_clock::now();
            return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
        } else {
            auto start = std::chrono::steady_clock::now();
            cache.Put(keys[Random() % keys.size()], "value").get();
            auto end = std::chrono::steady_clock::now();
            return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
        }
    }
};

using Cache_NoEviction = CacheFixture<Policies::NOEVICTION>;
BENCHMARK_F(Cache_NoEviction, ReadHeavy) 
(benchmark::State& state) {
    bm_impl<Policies::NOEVICTION, ReadHeavyWorkLoad>(state, cache, keys, ReadHeavyWorkLoad{});
}

using Cache_Random = CacheFixture<Policies::RANDOM>;
BENCHMARK_F(Cache_Random, ReadHeavy) 
(benchmark::State& state) {
    bm_impl<Policies::RANDOM, ReadHeavyWorkLoad>(state, cache, keys, ReadHeavyWorkLoad{});
}


using Cache_LRU = CacheFixture<Policies::LRU>;
BENCHMARK_F(Cache_LRU, ReadHeavy) 
(benchmark::State& state) {
    bm_impl<Policies::LRU, ReadHeavyWorkLoad>(state, cache, keys, ReadHeavyWorkLoad{});
}

using Cache_LFU = CacheFixture<Policies::LFU>;
BENCHMARK_F(Cache_LFU, ReadHeavy) 
(benchmark::State& state) {
    bm_impl<Policies::LFU, ReadHeavyWorkLoad>(state, cache, keys, ReadHeavyWorkLoad{});
}
