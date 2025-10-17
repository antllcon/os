#ifndef PROCESSSCANNER_HPP
#define PROCESSSCANNER_HPP

#if WIN32
#include "ScopedHandle.hpp"
#include <psapi.h>
#include <tlhelp32.h>
#include <windows.h>
#endif

#include <cstdint>
#include <string>
#include <vector>

struct ProcessInfo
{
	uint32_t pid = 0;
	std::string processName;
	std::string userName;
	uint64_t privateBytes = 0;
	uint64_t sharedMemory = 0;
	uint64_t threadCount = 0;
	double cpuUsage = 0.0;
	std::string commandLine;
};

class ProcessScanner
{
public:
	static std::vector<ProcessInfo> GetProcesses();
	static void SortByMemory(std::vector<ProcessInfo>& processes);
};

#endif // PROCESSSCANNER_HPP
