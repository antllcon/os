#include "ThreadPool.h"

#include <iostream>
#include <stdexcept>
#include <utility>

ThreadPool::ThreadPool(unsigned numThreads)
{
	if (numThreads == 0)
	{
		throw std::invalid_argument("ThreadPool must have at least one thread");
	}

	m_threads.reserve(numThreads);
	for (unsigned i = 0; i < numThreads; ++i)
	{
		m_threads.emplace_back([this] {
			ThreadMainLoop();
		});
	}
}

ThreadPool::~ThreadPool() noexcept
{
	Wait();
}

void ThreadPool::Dispatch(Task task)
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		if (m_stop)
		{
			throw std::runtime_error("Dispatch on stopped ThreadPool");
		}

		m_tasks.push(std::move(task));
	}

	m_condition.notify_one();
}

void ThreadPool::Wait()
{
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_stop = true;
	}

	m_condition.notify_all();

	for (auto& thread : m_threads)
	{
		if (thread.joinable())
		{
			try
			{
				thread.join();
			}
			catch (const std::system_error& e)
			{
				std::cerr << "Error join threads (" << e.code() << ")" << std::endl;
			}
		}
	}
}

void ThreadPool::ThreadMainLoop()
{
	while (true)
	{
		Task task;

		{
			std::unique_lock<std::mutex> lock(m_mutex);

			m_condition.wait(lock, [this] {
				return this->ShouldWakeUp();
			});

			if (m_stop && this->m_tasks.empty())
			{
				return;
			}

			task = std::move(m_tasks.front());
			m_tasks.pop();
		}

		try
		{
			task();
		}
		catch (const std::exception& e)
		{
			// Передавать указатель на функцию для логирования ошибок
			std::cerr << "Task process failed: " << e.what() << std::endl;
		}
		catch (...)
		{
			std::cerr << "Task process failed" << std::endl;
		}
	}
}

bool ThreadPool::ShouldWakeUp() const
{
	return m_stop || !m_tasks.empty();
}
