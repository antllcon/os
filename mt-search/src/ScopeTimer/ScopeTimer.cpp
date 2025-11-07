#include "ScopeTimer.h"
#include <iostream>

ScopeTimer::ScopeTimer(std::string_view message) noexcept
	: m_message(message)
	, m_startTime(std::chrono::high_resolution_clock::now())
{
}

ScopeTimer::~ScopeTimer() noexcept
{
	try
	{
		const auto endTime = std::chrono::high_resolution_clock::now();
		const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_startTime);
		std::cout << m_message << ": " << duration.count() << "ms" << std::endl;
	}
	catch (...)
	{
	}
}