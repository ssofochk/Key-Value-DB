#include <benchmark\\benchmark.h>
#include "cache.hpp"
#include <string>
#include <mutex>


const int kCacheSize = 100;


template <Policies Policy>
struct CacheFixture : benchmark::Fixture {
    Cache<Policy, int> cache;
    void SetUp(const benchmark::State&) override {
        for (auto i = 0; i < kCacheSize; ++i) {
            cache.Put(std::to_string(i), i);
        }
    }
};


const int kLoopCountOfReps = 10;

using Cache_NoEviction = CacheFixture<Policies::NOEVICTION>;
BENCHMARK_F(Cache_NoEviction, SeqLoop) 
(benchmark::State& state) {
    for (auto _ : state) {
        for (int i = 0; i < kCacheSize * kLoopCountOfReps; ++i) {
            cache.Get(std::to_string(i % kCacheSize));
        }
    }
    state.counters["ops"] = benchmark::Counter(kCacheSize * kLoopCountOfReps,
        benchmark::Counter::kIsIterationInvariantRate);
}

using Cache_Random = CacheFixture<Policies::RANDOM>;
BENCHMARK_F(Cache_Random, SeqLoop) 
(benchmark::State& state) {
    for (auto _ : state) {
        for (int i = 0; i < kCacheSize * kLoopCountOfReps; ++i) {
            cache.Get(std::to_string(i % kCacheSize));
        }
    }
    state.counters["ops"] = benchmark::Counter(kCacheSize * kLoopCountOfReps,
        benchmark::Counter::kIsIterationInvariantRate);
}


using Cache_LRU = CacheFixture<Policies::LRU>;
BENCHMARK_F(Cache_LRU, SeqLoop) 
(benchmark::State& state) {
    for (auto _ : state) {
        for (int i = 0; i < kCacheSize * kLoopCountOfReps; ++i) {
            cache.Get(std::to_string(i % kCacheSize));
        }
    }
    state.counters["ops"] = benchmark::Counter(kCacheSize * kLoopCountOfReps,
        benchmark::Counter::kIsIterationInvariantRate);
}

using Cache_LFU = CacheFixture<Policies::LFU>;
BENCHMARK_F(Cache_LFU, SeqLoop) 
(benchmark::State& state) {
    for (auto _ : state) {
        for (int i = 0; i < kCacheSize * kLoopCountOfReps; ++i) {
            cache.Get(std::to_string(i % kCacheSize));
        }
    }
    state.counters["ops"] = benchmark::Counter(kCacheSize * kLoopCountOfReps,
        benchmark::Counter::kIsIterationInvariantRate);
}

BENCHMARK_MAIN();
