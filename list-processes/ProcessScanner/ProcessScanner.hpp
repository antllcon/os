#ifndef PROCESSSCANNER_HPP
#define PROCESSSCANNER_HPP

#include "ScopedHandle.hpp"
#include <algorithm>
#include <iostream>
#include <psapi.h>
#include <string>
#include <tlhelp32.h>
#include <vector>
#include <windows.h>

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
	std::vector<ProcessInfo> GetProcesses();
	static void SortByMemory(std::vector<ProcessInfo>& processes);

private:
	std::vector<ProcessInfo> ScanProcesses();
	std::string GetUserName(uint32_t pid);
	bool GetMemoryInfo(uint32_t pid, uint64_t& privateBytes, uint64_t& sharedMemory);
	bool GetCpuUsage(uint32_t pid, double& usage);
	std::string GetCommandLine(uint32_t pid);
};

#endif // PROCESSSCANNER_HPP
