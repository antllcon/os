#include "ThreadPool.h"
#include <benchmark/benchmark.h>
#include <mutex>
#include <vector>
#include <future>
#include <chrono>

using namespace std::chrono_literals;

static void DoWork_CPU()
{
	double val = 0.0;
	for (int i = 0; i < 500; ++i)
	{
		val += std::sin(static_cast<double>(i));
	}

	benchmark::DoNotOptimize(val);
}

static void DoWork_Sleep()
{
	std::this_thread::sleep_for(10us);
}

template<void (*TaskFunction)()>
static void BM_Scalability(benchmark::State& state)
{
	constexpr int numTasks = 1000;
	const int numThreads = state.range(0);

	std::vector<std::future<void>> futures;
	futures.reserve(numTasks);

	for (auto _ : state)
	{
		ThreadPool pool(numThreads);

		for (int i = 0; i < numTasks; ++i)
		{
			futures.push_back(pool.Dispatch(TaskFunction));
		}

		for (auto& f : futures)
		{
			f.get();
		}

		futures.clear();
	}
}

BENCHMARK_TEMPLATE(BM_Scalability, DoWork_CPU)
	->DenseRange(1, std::thread::hardware_concurrency() * 2, 1)
	->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(BM_Scalability, DoWork_Sleep)
	->DenseRange(1, std::thread::hardware_concurrency() * 2, 1)
	->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();