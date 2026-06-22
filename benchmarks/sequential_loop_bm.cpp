#include <benchmark/benchmark.h>
#include "cache.hpp"
#include <string>
#include <mutex>
#include <chrono>
#include <algorithm>

const int kCacheSize = 100;


template <Policies Policy>
struct CacheFixture : benchmark::Fixture {
    Cache<Policy, int> cache;
    std::vector <std::string> keys;
    void SetUp(const benchmark::State&) override {
        keys.resize(kCacheSize);

        for (auto i = 0; i < kCacheSize; ++i) {
            keys[i] = std::to_string(i);
            cache.Put(keys[i], i);
        }
    }
};


uint64_t GetPercentile(const std::vector<uint64_t>& v, double x) {
    size_t idx = static_cast<size_t>(x * (v.size() - 1));
    return v[idx];
}

const int kLoopCountOfReps = 10;
const int K = 1000;

template <Policies Policy>
void bm_impl(benchmark::State& state, Cache<Policy, int>& cache, std::vector<std::string>& keys) {
    std::vector <uint64_t> latencies(kCacheSize * kLoopCountOfReps);
    for (auto _ : state) {
        for (int i = 0; i < kCacheSize * kLoopCountOfReps; ++i) {

            auto start = std::chrono::steady_clock::now();
            int t = K;
            while(--t) {
                benchmark::DoNotOptimize(
                    cache.Get(keys[i % kCacheSize])
                );
            }

            auto end = std::chrono::steady_clock::now();

            latencies[i] = static_cast<uint64_t>
                (std::chrono::duration_cast
                    <std::chrono::nanoseconds>
                        (end - start).count()
                ) / K;
        }
    }

    std::sort(latencies.begin(), latencies.end());

    state.counters["p50"] = GetPercentile(latencies, 0.5);
    state.counters["p99"] = GetPercentile(latencies, 0.99);

    // state.counters["ops"] = benchmark::Counter(kCacheSize * kLoopCountOfReps,
    //     benchmark::Counter::kIsIterationInvariantRate) / K;

    state.counters["ops"] =
    benchmark::Counter(
        kCacheSize * kLoopCountOfReps * 1000,
        benchmark::Counter::kIsRate
    );
}


using Cache_NoEviction = CacheFixture<Policies::NOEVICTION>;
BENCHMARK_F(Cache_NoEviction, SeqLoop) 
(benchmark::State& state) {
    bm_impl<Policies::NOEVICTION>(state, cache, keys);
}

using Cache_Random = CacheFixture<Policies::RANDOM>;
BENCHMARK_F(Cache_Random, SeqLoop) 
(benchmark::State& state) {
    bm_impl<Policies::RANDOM>(state, cache, keys);
}


using Cache_LRU = CacheFixture<Policies::LRU>;
BENCHMARK_F(Cache_LRU, SeqLoop) 
(benchmark::State& state) {
    bm_impl<Policies::LRU>(state, cache, keys);
}

using Cache_LFU = CacheFixture<Policies::LFU>;
BENCHMARK_F(Cache_LFU, SeqLoop) 
(benchmark::State& state) {
    bm_impl<Policies::LFU>(state, cache, keys);
}

BENCHMARK_MAIN();
