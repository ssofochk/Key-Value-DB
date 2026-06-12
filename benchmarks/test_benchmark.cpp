#include <benchmark/benchmark.h>
#include "cache.hpp"
#include <string>

static void BM_Empty(benchmark::State& state)
{
    for (auto _ : state)
    {
        state.PauseTiming();
        Cache<Policies::NOEVICTION, std::string> db;
        state.ResumeTiming();

        db.Put("sf", std::string("sds"));
        db.IsContain("sf");
        db.IsContain("sa");
        db.IsContain("asfdg");

        benchmark::DoNotOptimize(db);
    }
}

BENCHMARK(BM_Empty);

BENCHMARK_MAIN();