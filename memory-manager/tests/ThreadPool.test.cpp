#include "ThreadPool.h"
#include <atomic>
#include <chrono>
#include <gtest/gtest.h>

using namespace std::chrono_literals;

namespace
{
int Multiplication(int x)
{
	std::this_thread::sleep_for(2ms);
	return x * x;
}
} // namespace

TEST(ThreadPoolTest, CreateAndDestroy)
{
	try
	{
		ThreadPool pool(4);
	}
	catch (const std::exception& e)
	{
		FAIL() << e.what();
	}

	SUCCEED();
}

TEST(ThreadPoolTest, DispatchWithReturnValue)
{
	ThreadPool pool(4);

	auto future = pool.Dispatch(Multiplication, 8);
	const int result = future.get();

	EXPECT_EQ(result, 64);
}

TEST(ThreadPoolTest, DispatchManyTasks)
{
	ThreadPool pool(4);
	std::atomic<int> counter{0};
	constexpr int numTasks = 100;

	auto task = [&counter]() {
		std::this_thread::sleep_for(1ms);
		counter.fetch_add(1);
	};

	for (int i = 0; i < numTasks; ++i)
	{
		pool.Dispatch(task);
	}

	pool.Wait();
	EXPECT_EQ(counter.load(), numTasks);
}

TEST(ThreadPoolTest, DispatchFromWorker)
{
	ThreadPool pool(2);

	auto innerTask = []() {
		std::this_thread::sleep_for(5ms);
		return 42;
	};

	auto outerTask = [&pool, &innerTask]() {
		std::future<int> innerFuture = pool.Dispatch(innerTask);
		int innerResult = innerFuture.get();
		return innerResult + 1;
	};

	auto outerFuture = pool.Dispatch(outerTask);
	int finalResult = outerFuture.get();

	EXPECT_EQ(finalResult, 43);
}

TEST(ThreadPoolTest, TaskWithException)
{
	ThreadPool pool(1);

	auto failingTask = [] { throw std::runtime_error("Task failed successfully!"); };
	auto normalTask = [] { return 123; };

	auto future = pool.Dispatch(failingTask);
	EXPECT_THROW(future.get(), std::runtime_error);

	auto normalFuture = pool.Dispatch(normalTask);
	EXPECT_EQ(normalFuture.get(), 123);
}