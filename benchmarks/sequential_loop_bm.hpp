#pragma once

class SeqLoopWorkLoad {
private:
    size_t cur_idx = 0;
public:
    template <Policies Policy>
    uint64_t execute(DataBase<Policy>& cache, const std::vector<std::string>& keys) {
        const std::string& key = keys[cur_idx % keys.size()];
        auto start = std::chrono::steady_clock::now();
        cache.Get(key).get();
        auto end = std::chrono::steady_clock::now();
        ++cur_idx;
        return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
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
