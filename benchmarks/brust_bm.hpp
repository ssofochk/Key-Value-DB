#pragma once
#include <random>

class BurstWorkLoad {
private:
    size_t counter = 0;
    int value = 0;

public:
    template <Policies Policy>
    void execute(Cache<Policy>& cache,
                 const std::vector<std::string>& keys) {
        bool burst = (counter % 1000) < 200;

        if (burst) {
            cache.Put(
                keys[rand() % keys.size()],
                std::to_string(value++)
            );
        } else {
            cache.Get(
                keys[rand() % keys.size()]
            );
        }

        ++counter;
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
