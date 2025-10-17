#include "ProcessScanner.hpp"
#include "ScopedResource.hpp"

#include <algorithm>
#include <dirent.h>
#include <fstream>
#include <pwd.h>
#include <sstream>
#include <string>
#include <vector>

namespace
{
bool IsPid(const char* name)
{
	for (int i = 0; name[i] != '\0'; i++)
	{
		if (!isdigit(name[i]))
		{
			return false;
		}
	}

	return true;
}

std::string GetUserName(uint32_t pid)
{
	std::ifstream statusFile("/proc/" + std::to_string(pid) + "/status");
	if (!statusFile)
	{
		return "unknown";
	}

	std::string line;
	while (std::getline(statusFile, line))
	{
		if (line.rfind("Uid:", 0) == 0)
		{
			std::istringstream iss(line);
			std::string label;
			uid_t uid;

			iss >> label >> uid;

			passwd* pw = getpwuid(uid);
			return pw ? pw->pw_name : "User_" + std::to_string(uid);
		}
	}

	return "unknown";
}

bool GetMemoryInfo(uint32_t pid, uint64_t& privateBytes, uint64_t& sharedMemory)
{
	privateBytes = 0;
	sharedMemory = 0;

	std::ifstream smapsFile("/proc/" + std::to_string(pid) + "/smaps_rollup");
	if (!smapsFile)
	{
		return false;
	}

	std::string line;
	while (std::getline(smapsFile, line))
	{
		std::istringstream iss(line);
		std::string key;
		uint64_t value;

		iss >> key >> value;

		if (key == "Private_Clean:" || key == "Private_Dirty:")
		{
			privateBytes += value * 1024;
		}
		else if (key == "Shared_Clean:" || key == "Shared_Dirty:")
		{
			sharedMemory += value * 1024;
		}
	}

	return true;
}

std::string GetCommandLine(uint32_t pid)
{
	std::string cmdline;
	std::ifstream cmdlineFile("/proc/" + std::to_string(pid) + "/cmdline");

	if (cmdlineFile)
	{
		std::getline(cmdlineFile, cmdline);
		std::ranges::replace(cmdline, '\0', ' ');
	}

	return cmdline;
}

uint64_t GetThreadCount(uint32_t pid)
{
	uint64_t threadCount = 0;
	std::ifstream statusFile("/proc/" + std::to_string(pid) + "/status");

	if (!statusFile)
	{
		return 0;
	}

	std::string line;
	while (std::getline(statusFile, line))
	{
		if (line.rfind("Threads:", 0) == 0)
		{
			std::istringstream iss(line);
			std::string label;

			iss >> label >> threadCount;
			break;
		}
	}

	return threadCount;
}

std::string GetProcessName(uint32_t pid)
{
	std::string processName;
	std::ifstream commFile("/proc/" + std::to_string(pid) + "/comm");

	if (commFile)
	{
		std::getline(commFile, processName);
	}

	return processName;
}


std::vector<ProcessInfo> ScanProcesses()
{
	std::vector<ProcessInfo> processes;
	ScopedResource procDir("/proc");

	if (!procDir.get())
	{
		return processes;
	}

	dirent* entry;
	while ((entry = readdir(procDir.get())) != nullptr)
	{
		if (entry->d_type != DT_DIR || !IsPid(entry->d_name))
		{
			continue;
		}

		uint32_t pid = static_cast<uint32_t>(std::stoul(entry->d_name));

		ProcessInfo info;
		info.pid = pid;

		info.processName = GetProcessName(pid);
		info.threadCount = GetThreadCount(pid);
		info.userName = GetUserName(pid);
		info.commandLine = GetCommandLine(pid);
		GetMemoryInfo(pid, info.privateBytes, info.sharedMemory);

		processes.emplace_back(info);
	}

	return processes;
}
} // namespace

std::vector<ProcessInfo> ProcessScanner::GetProcesses()
{
	return ScanProcesses();
}

void ProcessScanner::SortByMemory(std::vector<ProcessInfo>& processes)
{
	std::ranges::sort(processes, [](const ProcessInfo& a, const ProcessInfo& b) {
		return a.privateBytes + a.sharedMemory > b.privateBytes + b.sharedMemory;
	});
}
