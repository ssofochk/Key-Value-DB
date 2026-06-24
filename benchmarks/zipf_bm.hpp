#pragma once
#include <random>

class ZipfWorkLoad {
private:
    double main_segment = 0.2;
    double probability = 0.9;
public:
    template <Policies Policy>
    uint64_t execute(DataBase<Policy>& cache, const std::vector<std::string>& keys) {
        if (Random() % 100 > probability * 100) {
            int idx = Random() % static_cast<int>(keys.size() * (1 - main_segment));
            const std::string& key = keys[idx];
            auto start = std::chrono::steady_clock::now();
            cache.Get(key).get();
            auto end = std::chrono::steady_clock::now();
            return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
        } else {
            int idx = Random() % static_cast<int>(keys.size() * main_segment);
            auto start = std::chrono::steady_clock::now();
            cache.Get(keys[keys.size() - idx - 1]).get();
            auto end = std::chrono::steady_clock::now();
            return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
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
