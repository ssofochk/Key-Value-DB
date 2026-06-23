#include <benchmark/benchmark.h>
#include "cache.hpp"
#include <string>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <random>

const int kCacheSize = 100;
const int kOperationsCount = 1000;


int Random(int l, int r) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(l, r);
    return dist(rng);
}

int Random() {
    return Random(0, 1e9);
}

template <Policies Policy>
struct CacheFixture : benchmark::Fixture {
    Cache<Policy> cache;
    std::vector <std::string> keys;
    void SetUp(const benchmark::State&) override {
        cache.SetMaxMemory(10'000); // in bytes
        keys.resize(kOperationsCount);
        for (auto i = 0; i < kOperationsCount; ++i) {
            keys[i] = std::to_string(i);
        }
        for (auto i = 0; i < 2 * kCacheSize; ++i) {
            cache.Put(keys[i], keys[i]);
        }
    }
};


uint64_t GetPercentile(const std::vector<uint64_t>& v, double x) {
    size_t idx = static_cast<size_t>(x * (v.size() - 1));
    return v[idx];
}


const int K = 100;


template <Policies Policy, typename Workload>
void bm_impl(benchmark::State& state, Cache<Policy>& cache, std::vector<std::string>& keys, Workload workload) {
    
    std::vector <uint64_t> latencies(kOperationsCount);
    
    const auto evictions_before = cache.GetEvictionCount();
    benchmark::DoNotOptimize(workload);
    for (auto _ : state) {
        for (int i = 0; i < kOperationsCount; ++i) {

            auto start = std::chrono::steady_clock::now();
            int t = K;
            while(t--) {
                workload.template execute<Policy>(cache, keys);
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

    std::uint64_t hits = cache.GetHits();
    std::uint64_t misses = cache.GetMisses();
    const uint64_t total_gets = hits + misses;

    if (total_gets == 0) {
        state.counters["hit_ratio_percent"] = 0.0;
    } else {
        state.counters["hit_ratio_percent"] = 100 * static_cast<double>(hits) / static_cast<double>(total_gets);
    }

    const auto evictions_after =
    cache.GetEvictionCount();

    state.counters["eviction_count"] = 
    static_cast<double>(evictions_after - evictions_before) / state.iterations() / K;

    state.counters["memory_bytes"] = 
    static_cast<double>(cache.EstimateMemoryBytes());


    state.SetItemsProcessed(
        state.iterations() * kOperationsCount * K
    );

}




#include "sequential_loop_bm.hpp"
#include "uniform_bm.hpp"
#include "zipf_bm.hpp"
#include "read_heavy_bm.hpp"
#include "write_heavy_bm.hpp"
#include "brust_bm.hpp"

BENCHMARK_MAIN();
