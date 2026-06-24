#pragma once
#include <random>

class WriteHeavyWorkLoad {
private:
    double read_percent = 0.2;
    double write_percent = 0.8;
public:
    template <Policies Policy>
    void execute(DataBase<Policy>& cache, const std::vector<std::string>& keys) {
        if (Random() % 100 < read_percent * 100) {
            auto k = cache.Get(keys[Random() % keys.size()]);
            k.get();
        } else {
            cache.Put(keys[Random() % keys.size()], "value");
        }
    }
};

using Cache_NoEviction = CacheFixture<Policies::NOEVICTION>;
BENCHMARK_F(Cache_NoEviction, WriteHeavy) 
(benchmark::State& state) {
    bm_impl<Policies::NOEVICTION, WriteHeavyWorkLoad>(state, cache, keys, WriteHeavyWorkLoad{});
}

using Cache_Random = CacheFixture<Policies::RANDOM>;
BENCHMARK_F(Cache_Random, WriteHeavy) 
(benchmark::State& state) {
    bm_impl<Policies::RANDOM, WriteHeavyWorkLoad>(state, cache, keys, WriteHeavyWorkLoad{});
}


using Cache_LRU = CacheFixture<Policies::LRU>;
BENCHMARK_F(Cache_LRU, WriteHeavy) 
(benchmark::State& state) {
    bm_impl<Policies::LRU, WriteHeavyWorkLoad>(state, cache, keys, WriteHeavyWorkLoad{});
}

using Cache_LFU = CacheFixture<Policies::LFU>;
BENCHMARK_F(Cache_LFU, WriteHeavy) 
(benchmark::State& state) {
    bm_impl<Policies::LFU, WriteHeavyWorkLoad>(state, cache, keys, WriteHeavyWorkLoad{});
}
