#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool final
{
public:
	using Task = std::function<void()>;

	explicit ThreadPool(unsigned numThreads);
	~ThreadPool() noexcept;

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

	void Dispatch(Task task);
	void Wait();

	template <typename F, typename... Args>
	auto Dispatch(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>
	{
		using ReturnType = std::invoke_result_t<F, Args...>;

		auto taskPtr = std::make_shared<std::packaged_task<ReturnType()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...));

		std::future<ReturnType> future = taskPtr->get_future();

		Task voidTask = [taskPtr]() {
			(*taskPtr)();
		};

		Dispatch(std::move(voidTask));
		return future;
	}

private:
	void ThreadMainLoop();
	bool ShouldWakeUp() const;

	std::vector<std::thread> m_threads;
	std::queue<Task> m_tasks;
	std::condition_variable m_condition;
	std::mutex m_mutex;
	std::atomic<bool> m_stop{false};
};