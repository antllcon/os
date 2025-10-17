#include "ProcessPrinter.hpp"
#include <iomanip>
#include <iostream>
#include <string>

namespace
{
constexpr size_t MEGABYTE = 1024 * 1024;
constexpr size_t TABLE_WIDTH = 120;
constexpr size_t PID_WIDTH = 8;
constexpr size_t NAME_WIDTH = 25;
constexpr size_t USER_WIDTH = 20;
constexpr size_t MEMORY_WIDTH = 12;
constexpr size_t THREADS_WIDTH = 8;
constexpr size_t CPU_WIDTH = 10;
constexpr size_t NAME_MAX_LENGTH = 24;
constexpr size_t USER_MAX_LENGTH = 19;
constexpr size_t CMD_MAX_LENGTH = 25;

std::string FormatMemorySize(size_t bytes)
{
	std::string result = std::to_string(static_cast<double>(bytes) / MEGABYTE);
	return result.substr(0, result.find('.') + 2) + " MB";
}

std::string TruncateString(const std::string& str, size_t maxLength)
{
	if (str.length() > maxLength)
	{
		return str.substr(0, maxLength - 2) + "..";
	}

	return str;
}
} // namespace

namespace ProcessPrinter
{
void PrintTable(const std::vector<ProcessInfo>& processes)
{
	std::cout << std::string(TABLE_WIDTH, '-') << std::endl;

	std::cout << std::left
			  << std::setw(PID_WIDTH) << "PID"
			  << std::setw(NAME_WIDTH) << "Process Name"
			  << std::setw(USER_WIDTH) << "User"
			  << std::setw(MEMORY_WIDTH) << "Private"
			  << std::setw(MEMORY_WIDTH) << "Shared"
			  << std::setw(THREADS_WIDTH) << "Threads"
			  << std::setw(CPU_WIDTH) << "CPU %"
			  << "Command Line" << std::endl;

	std::cout << std::string(TABLE_WIDTH, '-') << std::endl;

	for (const auto& process : processes)
	{
		std::cout << std::left
				  << std::setw(PID_WIDTH) << process.pid
				  << std::setw(NAME_WIDTH) << TruncateString(process.processName, NAME_MAX_LENGTH)
				  << std::setw(USER_WIDTH) << TruncateString(process.userName, USER_MAX_LENGTH)
				  << std::setw(MEMORY_WIDTH) << FormatMemorySize(process.privateBytes)
				  << std::setw(MEMORY_WIDTH) << FormatMemorySize(process.sharedMemory)
				  << std::setw(THREADS_WIDTH) << process.threadCount
				  << std::setw(CPU_WIDTH) << process.cpuUsage
				  << TruncateString(process.commandLine, CMD_MAX_LENGTH)
				  << std::endl;
	}

	std::cout << std::string(TABLE_WIDTH, '-') << std::endl;
}

void PrintSummary(const std::vector<ProcessInfo>& processes)
{
	size_t totalPrivate = 0;
	size_t totalShared = 0;

	for (const auto& process : processes)
	{
		totalPrivate += process.privateBytes;
		totalShared += process.sharedMemory;
	}

	std::cout << "Found " << processes.size() << " processes" << std::endl;
	std::cout << "Total private: " << FormatMemorySize(totalPrivate) << std::endl;
	std::cout << "Total shared: " << FormatMemorySize(totalShared) << std::endl;
	std::cout << "Total: " << FormatMemorySize(totalPrivate + totalShared) << std::endl;
}
} // namespace ProcessPrinter
