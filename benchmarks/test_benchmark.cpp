#include <benchmark\\benchmark.h>
#include "cache.hpp"
#include <string>
#include <mutex>


// static Cache<Policies::NOEVICTION, std::string> db;

// static void BM_Empty(benchmark::State& state) {

//     const int N = state.range(0);    
//     for (auto _ : state) {

//         for (int i = 0; i < N; ++i) {
//             db.Put("key" + std::to_string(i), std::string("value"));
//         }

//         benchmark::DoNotOptimize(db);
//     }

//     state.SetItemsProcessed(state.iterations() * N);

//     state.counters["ops1"] = benchmark::Counter(N,
//         benchmark::Counter::kIsIterationInvariantRate);

//     state.counters["ops2"] = state.iterations() * N;

//     state.counters["ops3"] = benchmark::Counter(
//         state.iterations() * N,
//         benchmark::Counter::kIsRate
//     );
// }

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
}

using Cache_Random = CacheFixture<Policies::RANDOM>;
BENCHMARK_F(Cache_Random, SeqLoop) 
(benchmark::State& state) {
    for (auto _ : state) {
        for (int i = 0; i < kCacheSize * kLoopCountOfReps; ++i) {
            cache.Get(std::to_string(i % kCacheSize));
        }
    }
}


using Cache_LRU = CacheFixture<Policies::LRU>;
BENCHMARK_F(Cache_LRU, SeqLoop) 
(benchmark::State& state) {
    for (auto _ : state) {
        for (int i = 0; i < kCacheSize * kLoopCountOfReps; ++i) {
            cache.Get(std::to_string(i % kCacheSize));
        }
    }
}

using Cache_LFU = CacheFixture<Policies::LFU>;
BENCHMARK_F(Cache_LFU, SeqLoop) 
(benchmark::State& state) {
    for (auto _ : state) {
        for (int i = 0; i < kCacheSize * kLoopCountOfReps; ++i) {
            cache.Get(std::to_string(i % kCacheSize));
        }
    }
}


// BENCHMARK(BM_Empty) -> Arg(256) -> Threads(1) -> Threads(2) -> Threads(4) -> Threads(8);

BENCHMARK_MAIN();
