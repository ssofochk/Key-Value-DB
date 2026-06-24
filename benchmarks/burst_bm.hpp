#pragma once
#include <random>

class BurstWorkLoad {
private:
    size_t counter = 0;
    int value = 0;

public:
    template <Policies Policy>
    uint64_t execute(DataBase<Policy>& cache,
                 const std::vector<std::string>& keys) {
        bool burst = (counter % 1000) < 200;
        uint64_t res;
        if (burst) {
            auto start = std::chrono::steady_clock::now();
            cache.Get(keys[Random() % keys.size()]).get();
            cache.Get(keys[Random() % keys.size()]).get();
            cache.Get(keys[Random() % keys.size()]).get();
            cache.Get(keys[Random() % keys.size()]).get();
            if (Policy != Policies::NOEVICTION) extra_evics += 3;
            auto end = std::chrono::steady_clock::now();

            res = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()) / 4;

        } else {
            auto start = std::chrono::steady_clock::now();
            cache.Get(keys[Random() % keys.size()]).get();
            auto end = std::chrono::steady_clock::now();

            res = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
        }
        ++counter;
        return res;
    }
};

using Cache_NoEviction = CacheFixture<Policies::NOEVICTION>;
BENCHMARK_F(Cache_NoEviction, Burst) 
(benchmark::State& state) {
    bm_impl<Policies::NOEVICTION, BurstWorkLoad>(state, cache, keys, BurstWorkLoad{});
}

using Cache_Random = CacheFixture<Policies::RANDOM>;
BENCHMARK_F(Cache_Random, Burst) 
(benchmark::State& state) {
    bm_impl<Policies::RANDOM, BurstWorkLoad>(state, cache, keys, BurstWorkLoad{});
}


using Cache_LRU = CacheFixture<Policies::LRU>;
BENCHMARK_F(Cache_LRU, Burst) 
(benchmark::State& state) {
    bm_impl<Policies::LRU, BurstWorkLoad>(state, cache, keys, BurstWorkLoad{});
}

using Cache_LFU = CacheFixture<Policies::LFU>;
BENCHMARK_F(Cache_LFU, Burst) 
(benchmark::State& state) {
    bm_impl<Policies::LFU, BurstWorkLoad>(state, cache, keys, BurstWorkLoad{});
}
