#pragma once
#include <random>

class BurstWorkLoad {
private:
    double main_segment = 0.2;
    double probability = 0.9;
    double start_point = 0;
    size_t counter = 0;
public:
    template <Policies Policy>
    uint64_t execute(DataBase<Policy>& cache, const std::vector<std::string>& keys) {
        int res = 0;
        if (Random() % 100 > probability * 100) {
            int idx = Random() % static_cast<int>(keys.size() * (1 - main_segment));
            const std::string& key = keys[idx];

            auto start = std::chrono::steady_clock::now();

            cache.Get(key).get();

            auto end = std::chrono::steady_clock::now();

            res = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
        } else {
            int idx = Random() % static_cast<int>(keys.size() * main_segment);
            auto start = std::chrono::steady_clock::now();
            cache.Get(keys[start_point + idx]).get();
            auto end = std::chrono::steady_clock::now();
            res = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
        }


        ++counter;
        if (counter % (kOperationsCount / 3) == 0) {
            start_point = Random() % static_cast<int>((1 - main_segment) * keys.size() - 1);
        }
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
