#pragma once
#include <random>

class ReadHeavyWorkLoad {
private:
    double read_percent = 0.95;
    double write_percent = 0.05;
public:
    template <Policies Policy>
    void execute(Cache<Policy>& cache, const std::vector<std::string>& keys) {
        if (rand() % 100 < read_percent * 100) {
            cache.Get(keys[rand() % keys.size()]);
        } else {
            cache.Put(keys[rand() % keys.size()], "value");
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
