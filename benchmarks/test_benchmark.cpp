#include <benchmark\\benchmark.h>
#include "data_base.hpp"
#include <string>

static void BM_Empty(benchmark::State& state)
{
    for (auto _ : state)
    {
        state.PauseTiming();
        DataBase db;
        state.ResumeTiming();

        db.PutElement("sf", std::string("sds"));
        db.IsContain("sf");
        db.IsContain("sa");
        db.IsContain("asfdg");

        benchmark::DoNotOptimize(db);
    }
}

BENCHMARK(BM_Empty);

BENCHMARK_MAIN();