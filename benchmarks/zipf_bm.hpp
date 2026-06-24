#pragma once
#include <random>

class ZipfWorkLoad {
private:
    double main_segment = 0.2;
    double probability = 0.9;
public:
    template <Policies Policy>
    void execute(DataBase<Policy>& cache, const std::vector<std::string>& keys) {
        if (Random() % 100 > probability * 100) {
            int idx = Random() % static_cast<int>(keys.size() * (1 - main_segment));
            const std::string& key = keys[main_segment * keys.size() + idx];
            auto k = cache.Get(key);
            k.get();
        } else {
            int idx = Random() % static_cast<int>(keys.size() * main_segment);
            cache.Get(keys[idx]);
        }
    }
};

using Cache_NoEviction = CacheFixture<Policies::NOEVICTION>;
BENCHMARK_F(Cache_NoEviction, Zipf) 
(benchmark::State& state) {
    bm_impl<Policies::NOEVICTION, ZipfWorkLoad>(state, cache, keys, ZipfWorkLoad{});
}

using Cache_Random = CacheFixture<Policies::RANDOM>;
BENCHMARK_F(Cache_Random, Zipf) 
(benchmark::State& state) {
    bm_impl<Policies::RANDOM, ZipfWorkLoad>(state, cache, keys, ZipfWorkLoad{});
}


using Cache_LRU = CacheFixture<Policies::LRU>;
BENCHMARK_F(Cache_LRU, Zipf) 
(benchmark::State& state) {
    bm_impl<Policies::LRU, ZipfWorkLoad>(state, cache, keys, ZipfWorkLoad{});
}

using Cache_LFU = CacheFixture<Policies::LFU>;
BENCHMARK_F(Cache_LFU, Zipf) 
(benchmark::State& state) {
    bm_impl<Policies::LFU, ZipfWorkLoad>(state, cache, keys, ZipfWorkLoad{});
}
