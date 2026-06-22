#pragma once
#include <random>

class UniformWorkLoad {
public:
    template <Policies Policy>
    void execute(Cache<Policy>& cache, const std::vector<std::string>& keys) {
        cache.Get(keys[rand() % keys.size()]);
    }
};

using Cache_NoEviction = CacheFixture<Policies::NOEVICTION>;
BENCHMARK_F(Cache_NoEviction, Uniform) 
(benchmark::State& state) {
    bm_impl<Policies::NOEVICTION, UniformWorkLoad>(state, cache, keys, UniformWorkLoad{});
}

using Cache_Random = CacheFixture<Policies::RANDOM>;
BENCHMARK_F(Cache_Random, Uniform) 
(benchmark::State& state) {
    bm_impl<Policies::RANDOM, UniformWorkLoad>(state, cache, keys, UniformWorkLoad{});
}


using Cache_LRU = CacheFixture<Policies::LRU>;
BENCHMARK_F(Cache_LRU, Uniform) 
(benchmark::State& state) {
    bm_impl<Policies::LRU, UniformWorkLoad>(state, cache, keys, UniformWorkLoad{});
}

using Cache_LFU = CacheFixture<Policies::LFU>;
BENCHMARK_F(Cache_LFU, Uniform) 
(benchmark::State& state) {
    bm_impl<Policies::LFU, UniformWorkLoad>(state, cache, keys, UniformWorkLoad{});
}
