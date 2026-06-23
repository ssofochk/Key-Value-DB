#pragma once

class SeqLoopWorkLoad {
private:
    size_t cur_idx = 0;
public:
    template <Policies Policy>
    void execute(Cache<Policy>& cache, const std::vector<std::string>& keys) {
        cache.Get(keys[cur_idx % keys.size()]);
        ++cur_idx;
    }
};

using Cache_NoEviction = CacheFixture<Policies::NOEVICTION>;
BENCHMARK_F(Cache_NoEviction, SeqLoop) 
(benchmark::State& state) {
    bm_impl<Policies::NOEVICTION, SeqLoopWorkLoad>(state, cache, keys, SeqLoopWorkLoad{});
}

using Cache_Random = CacheFixture<Policies::RANDOM>;
BENCHMARK_F(Cache_Random, SeqLoop) 
(benchmark::State& state) {
    bm_impl<Policies::RANDOM, SeqLoopWorkLoad>(state, cache, keys, SeqLoopWorkLoad{});
}


using Cache_LRU = CacheFixture<Policies::LRU>;
BENCHMARK_F(Cache_LRU, SeqLoop) 
(benchmark::State& state) {
    bm_impl<Policies::LRU, SeqLoopWorkLoad>(state, cache, keys, SeqLoopWorkLoad{});
}

using Cache_LFU = CacheFixture<Policies::LFU>;
BENCHMARK_F(Cache_LFU, SeqLoop) 
(benchmark::State& state) {
    bm_impl<Policies::LFU, SeqLoopWorkLoad>(state, cache, keys, SeqLoopWorkLoad{});
}
