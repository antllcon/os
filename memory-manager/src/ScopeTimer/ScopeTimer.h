#pragma once

#include <chrono>
#include <string>
#include <string_view>

class ScopeTimer
{
public:
	explicit ScopeTimer(std::string_view message = "Время выполнения") noexcept;

	~ScopeTimer() noexcept;

	ScopeTimer(const ScopeTimer&) = delete;
	ScopeTimer& operator=(const ScopeTimer&) = delete;
	ScopeTimer(ScopeTimer&&) = delete;
	ScopeTimer& operator=(ScopeTimer&&) = delete;

private:
	std::string m_message;
	std::chrono::high_resolution_clock::time_point m_startTime;
};